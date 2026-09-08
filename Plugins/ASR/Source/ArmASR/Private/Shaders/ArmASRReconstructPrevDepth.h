

#include "ArmASRShaderParameters.h"

#include "RenderGraphFwd.h"
#include "ShaderCompilerCore.h"
#include "ShaderParameterStruct.h"

class FArmASRReconstructPrevDepthPS : public FGlobalShader
{
public:
	using FPermutationDomain = TShaderPermutationDomain<FArmASR_ApplyUltraPerfOpt>;

	DECLARE_GLOBAL_SHADER(FArmASRReconstructPrevDepthPS);
	SHADER_USE_PARAMETER_STRUCT(FArmASRReconstructPrevDepthPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FArmASRPassParameters, cbArmASR)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_input_motion_vectors)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_input_depth)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_input_color_jittered)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_input_exposure)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, rw_reconstructed_previous_nearest_depth)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return FArmASRGlobalShader::ShouldCompilePermutation(Parameters);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{

		FArmASRGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

inline void SetReconstructPrevDepthParameters(
	bool bIsUltraPerformance,
	FArmASRReconstructPrevDepthPS::FParameters* RpdShaderParameters,
	TUniformBufferRef<FArmASRPassParameters> ArmASRPassParameters,
	const FRDGTextureRef MotionVectorTexture,
	const FRDGTextureSRVRef DepthTexture,
	const FRDGTextureSRVRef SceneColorTexture,
	const FRDGTextureSRVRef AutoExposureTexture, 
	const FIntPoint& InputExtents,
	const FScreenPassTextureViewport& Viewport,
	FRDGBuilder& GraphBuilder)
{
	FRDGTextureSRVDesc MotionVectorSRVDesc = FRDGTextureSRVDesc::Create(MotionVectorTexture);
	FRDGTextureSRVRef MotionVectorSRVTexture = GraphBuilder.CreateSRV(MotionVectorSRVDesc);
	RpdShaderParameters->r_input_motion_vectors = MotionVectorSRVTexture;

	RpdShaderParameters->r_input_depth = DepthTexture;
	RpdShaderParameters->r_input_color_jittered = SceneColorTexture;

	RpdShaderParameters->r_input_exposure = AutoExposureTexture;

	FRDGTextureDesc NearestDepthDesc = FRDGTextureDesc::Create2D(InputExtents, PF_R32_UINT, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable, 1, 1);
	FRDGTextureRef NearestDepthTexture = GraphBuilder.CreateTexture(NearestDepthDesc, TEXT("ReconstructedPreviousNearestDepthTexture"));
	RpdShaderParameters->rw_reconstructed_previous_nearest_depth = GraphBuilder.CreateUAV(NearestDepthTexture);

	AddClearRenderTargetPass(GraphBuilder, NearestDepthTexture);

	if (bIsUltraPerformance)
	{
		FRDGTextureDesc DilatedDepthVelocityLumaDesc = FRDGTextureDesc::Create2D(InputExtents, PF_FloatRGBA, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_RenderTargetable, 1, 1);
		FRDGTextureRef DilatedDepthVelocityLumaTexture = GraphBuilder.CreateTexture(DilatedDepthVelocityLumaDesc, TEXT("DilatedDepthVelocityLumaTexture"));

		const FScreenPassRenderTarget DilatedDepthVelocityLumaRT(DilatedDepthVelocityLumaTexture, Viewport.Rect, ERenderTargetLoadAction::ENoAction);
		RpdShaderParameters->RenderTargets[0] = DilatedDepthVelocityLumaRT.GetRenderTargetBinding();
	}
	else
	{

		FRDGTextureDesc DilatedDepthDesc = FRDGTextureDesc::Create2D(InputExtents, PF_R32_FLOAT, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_RenderTargetable, 1, 1);
		FRDGTextureRef DilatedDepthTexture = GraphBuilder.CreateTexture(DilatedDepthDesc, TEXT("DilatedDepthTexture"));

		FRDGTextureDesc DilatedVelocityDesc = FRDGTextureDesc::Create2D(InputExtents, PF_G16R16F, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_RenderTargetable, 1, 1);
		FRDGTextureRef DilatedVelocityTexture = GraphBuilder.CreateTexture(DilatedVelocityDesc, TEXT("DilatedVelocityTexture"));

		FRDGTextureDesc LockLumaDesc = FRDGTextureDesc::Create2D(InputExtents, PF_R16F, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_RenderTargetable, 1, 1);
		FRDGTextureRef LockLumaTexture = GraphBuilder.CreateTexture(LockLumaDesc, TEXT("LockLumaTexture"));

		const FScreenPassRenderTarget DilatedDepthRT(DilatedDepthTexture, Viewport.Rect, ERenderTargetLoadAction::ENoAction);
		RpdShaderParameters->RenderTargets[0] = DilatedDepthRT.GetRenderTargetBinding();

		const FScreenPassRenderTarget DilatedVelocityRT(DilatedVelocityTexture, Viewport.Rect, ERenderTargetLoadAction::ENoAction);
		RpdShaderParameters->RenderTargets[1] = DilatedVelocityRT.GetRenderTargetBinding();

		const FScreenPassRenderTarget LockLumaRT(LockLumaTexture, Viewport.Rect, ERenderTargetLoadAction::ENoAction);
		RpdShaderParameters->RenderTargets[2] = LockLumaRT.GetRenderTargetBinding();
	}

	RpdShaderParameters->cbArmASR = ArmASRPassParameters;
}
