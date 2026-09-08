

#include "ArmASRShaderParameters.h"
#include "ArmASRShaderUtils.h"
#include "ArmASRInfo.h"

#include "RenderGraphFwd.h"
#include "ShaderCompilerCore.h"
#include "ShaderParameterStruct.h"
#include "SystemTextures.h"

class FArmASR_DoSharpening : SHADER_PERMUTATION_BOOL("FFXM_FSR2_OPTION_APPLY_SHARPENING");

class FArmASRAccumulatePS : public FGlobalShader
{
public:
	using FPermutationDomain = TShaderPermutationDomain<FArmASR_DoSharpening, FArmASR_ApplyBalancedOpt, FArmASR_ApplyPerfOpt, FArmASR_ApplyUltraPerfOpt>;

	DECLARE_GLOBAL_SHADER(FArmASRAccumulatePS);
	SHADER_USE_PARAMETER_STRUCT(FArmASRAccumulatePS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FArmASRPassParameters, cbArmASR)
		SHADER_PARAMETER_SAMPLER(SamplerState, s_LinearClamp)
		SHADER_PARAMETER_SAMPLER(SamplerState, s_PointClamp)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_input_exposure)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_dilated_reactive_masks)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_dilated_motion_vectors)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_dilated_depth_motion_vectors_input_luma)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_input_motion_vectors)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_internal_upscaled_color)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_input_color_jittered)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_lock_status)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_prepared_input_color)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_imgMips)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_auto_exposure)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_luma_history)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_internal_temporal_reactive)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, r_new_locks)
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

inline void SetAccumulateParameters(
	FArmASRAccumulatePS::FParameters* AccumulateParameters,
	TUniformBufferRef<FArmASRPassParameters> ArmASRPassParameters,
	const FRDGTextureSRVRef AutoExposureTexture,        
	const FRDGTextureRef ImgMipsTexture,                
	const FRDGTextureRef DilatedMotionVectorTexture,    
	const FRDGTextureRef DilatedDepthMotionVectorsInputLumaTexture, 
	const FRDGTextureRef DilatedReactiveMaskTexture,    
	const FRDGTextureRef PreparedInputColor,            
	const FRDGTextureSRVRef SceneColorTexture,
	const FRDGTextureRef PrevLockStatusTexture,         
	const FRDGTextureRef OutputTexture,
	const FRDGTextureRef MotionVectorTexture,
	const FRDGTextureRef PrevUpscaledColourTexture,     
	const FRDGTextureRef PrevLumaHistoryTexture,        
	const FRDGTextureRef PrevTemporalReactiveTexture,   
	const FRDGTextureRef LockMaskTexture,               
	const float Sharpness,
	const EShaderQualityPreset QualityPreset,
	const FIntPoint& OutputExtents,
	const FIntRect& OutputRect,
	FRDGBuilder& GraphBuilder)
{

	AccumulateParameters->s_LinearClamp = TStaticSamplerState<SF_Bilinear>::GetRHI();
	AccumulateParameters->s_PointClamp = TStaticSamplerState<SF_Point>::GetRHI();

	AccumulateParameters->r_input_exposure = AutoExposureTexture;

	FRDGTextureSRVDesc DilatedReactiveMaskSRVDesc = FRDGTextureSRVDesc::Create(DilatedReactiveMaskTexture);
	FRDGTextureSRVRef DilatedReactiveMaskSRVTexture = GraphBuilder.CreateSRV(DilatedReactiveMaskSRVDesc);
	AccumulateParameters->r_dilated_reactive_masks = DilatedReactiveMaskSRVTexture;

	const bool bIsUltraPerformance = (QualityPreset == EShaderQualityPreset::ULTRA_PERFORMANCE);
	if (bIsUltraPerformance)
	{
		FRDGTextureSRVDesc DilatedDepthMotionVectorsInputLumaSRVDesc = FRDGTextureSRVDesc::Create(DilatedDepthMotionVectorsInputLumaTexture);
		FRDGTextureSRVRef DilatedDepthMotionVectorsInputLumaSRVTexture = GraphBuilder.CreateSRV(DilatedDepthMotionVectorsInputLumaSRVDesc);
		AccumulateParameters->r_dilated_depth_motion_vectors_input_luma = DilatedDepthMotionVectorsInputLumaSRVTexture;

		AccumulateParameters->r_input_color_jittered = SceneColorTexture;
	}
	else
	{
		FRDGTextureSRVDesc DilatedMotionVectorSRVDesc = FRDGTextureSRVDesc::Create(DilatedMotionVectorTexture);
		FRDGTextureSRVRef DilatedMotionVectorSRVTexture = GraphBuilder.CreateSRV(DilatedMotionVectorSRVDesc);
		AccumulateParameters->r_dilated_motion_vectors = DilatedMotionVectorSRVTexture;

		FRDGTextureSRVDesc PreparedInputColorSRVDesc = FRDGTextureSRVDesc::Create(PreparedInputColor);
		FRDGTextureSRVRef PreparedInputSRVTexture = GraphBuilder.CreateSRV(PreparedInputColorSRVDesc);
		AccumulateParameters->r_prepared_input_color = PreparedInputSRVTexture;

		FRDGTextureSRVDesc LumaHistorySRVDesc = FRDGTextureSRVDesc::Create(PrevLumaHistoryTexture);
		AccumulateParameters->r_luma_history = GraphBuilder.CreateSRV(LumaHistorySRVDesc);

		FRDGTextureSRVDesc TemporalReactiveHistorySRVDesc = FRDGTextureSRVDesc::Create(PrevTemporalReactiveTexture);
		AccumulateParameters->r_internal_temporal_reactive = GraphBuilder.CreateSRV(TemporalReactiveHistorySRVDesc);

		FRDGTextureSRVDesc ImgMipsSRVDesc = FRDGTextureSRVDesc::Create(ImgMipsTexture);
		FRDGTextureSRVRef ImgMipsSRVTexture = GraphBuilder.CreateSRV(ImgMipsSRVDesc);
		AccumulateParameters->r_imgMips = GraphBuilder.CreateSRV(ImgMipsSRVDesc);
	}

	FRDGTextureSRVDesc MotionVectorSRVDesc = FRDGTextureSRVDesc::Create(MotionVectorTexture);
	FRDGTextureSRVRef MotionVectorSRVTexture = GraphBuilder.CreateSRV(MotionVectorSRVDesc);
	AccumulateParameters->r_input_motion_vectors = MotionVectorSRVTexture;

	FRDGTextureSRVDesc InternalUpscaledPrevSRVDesc = FRDGTextureSRVDesc::Create(PrevUpscaledColourTexture);
	FRDGTextureSRVRef InternalUpscaledPrevSRVTexture = GraphBuilder.CreateSRV(InternalUpscaledPrevSRVDesc);
	AccumulateParameters->r_internal_upscaled_color = InternalUpscaledPrevSRVTexture;

	FRDGTextureSRVDesc LockStatusSRVDesc = FRDGTextureSRVDesc::Create(PrevLockStatusTexture);
	FRDGTextureSRVRef LockStatusSRVTexture = GraphBuilder.CreateSRV(LockStatusSRVDesc);
	AccumulateParameters->r_lock_status = LockStatusSRVTexture;

	AccumulateParameters->r_auto_exposure = AutoExposureTexture;

	FRDGTextureSRVDesc LumaHistorySRVDesc = FRDGTextureSRVDesc::Create(PrevLumaHistoryTexture);
	AccumulateParameters->r_luma_history = GraphBuilder.CreateSRV(LumaHistorySRVDesc);

	FRDGTextureSRVDesc TemporalReactiveHistorySRVDesc = FRDGTextureSRVDesc::Create(PrevTemporalReactiveTexture);
	AccumulateParameters->r_internal_temporal_reactive = GraphBuilder.CreateSRV(TemporalReactiveHistorySRVDesc);

	FRDGTextureSRVDesc LockMaskSRVDesc = FRDGTextureSRVDesc::Create(LockMaskTexture);
	AccumulateParameters->r_new_locks = GraphBuilder.CreateSRV(LockMaskSRVDesc);

	const bool bIsBalancedOrPerformance = (QualityPreset == EShaderQualityPreset::BALANCED) || (QualityPreset == EShaderQualityPreset::PERFORMANCE);
	const EPixelFormat InternalUpscaledFormat = (bIsUltraPerformance || bIsBalancedOrPerformance) ? PF_FloatR11G11B10 : PF_FloatRGBA;

	FRDGTextureDesc InternalUpscaledOutputColorDesc = FRDGTextureDesc::Create2D(OutputExtents, InternalUpscaledFormat, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_RenderTargetable, 1, 1);
	FRDGTextureRef InternalUpscaledColorOutputTexture = GraphBuilder.CreateTexture(InternalUpscaledOutputColorDesc, TEXT("InternalUpscaledColorOutputTexture"));

	FRDGTextureDesc LockStatusOutputDesc = FRDGTextureDesc::Create2D(OutputExtents, PF_G16R16F, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_RenderTargetable, 1, 1);
	FRDGTextureRef LockStatusOutputTexture = GraphBuilder.CreateTexture(LockStatusOutputDesc, TEXT("LockStatusOutputTexture"));

	const FScreenPassRenderTarget InternalUpscaledColorRT(InternalUpscaledColorOutputTexture, OutputRect, ERenderTargetLoadAction::ENoAction);
	const FScreenPassRenderTarget LockStatusRT(LockStatusOutputTexture, OutputRect, ERenderTargetLoadAction::ENoAction);
	const FScreenPassRenderTarget UpscaledOutput(OutputTexture, OutputRect, ERenderTargetLoadAction::ENoAction);

	AccumulateParameters->RenderTargets[0] = InternalUpscaledColorRT.GetRenderTargetBinding();
	if (bIsUltraPerformance)
	{
		AccumulateParameters->RenderTargets[1] = LockStatusRT.GetRenderTargetBinding();
	}
	else if (!bIsBalancedOrPerformance)
	{
		AccumulateParameters->RenderTargets[1] = LockStatusRT.GetRenderTargetBinding();

		FRDGTextureDesc LumaHistoryOutputDesc = FRDGTextureDesc::Create2D(OutputExtents, PF_R8G8B8A8, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_RenderTargetable, 1, 1);
		FRDGTextureRef LumaHistoryOutputTexture = GraphBuilder.CreateTexture(LumaHistoryOutputDesc, TEXT("LumaHistoryOutputTexture"));
		const FScreenPassRenderTarget LumaHistoryRT(LumaHistoryOutputTexture, OutputRect, ERenderTargetLoadAction::ENoAction);
		AccumulateParameters->RenderTargets[2] = LumaHistoryRT.GetRenderTargetBinding();
	}
	else
	{

		const FRDGTextureDesc TemporalReactiveOutputDesc = FRDGTextureDesc::Create2D(OutputExtents, PF_R16F, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_RenderTargetable, 1, 1);
		const FRDGTextureRef TemporalReactiveOutputTexture = GraphBuilder.CreateTexture(TemporalReactiveOutputDesc, TEXT("InternalReactiveOutput"));
		const FScreenPassRenderTarget TemporalReactiveRT(TemporalReactiveOutputTexture, OutputRect, ERenderTargetLoadAction::ENoAction);
		AccumulateParameters->RenderTargets[1] = TemporalReactiveRT.GetRenderTargetBinding();
		AccumulateParameters->RenderTargets[2] = LockStatusRT.GetRenderTargetBinding();
	}

	const bool bUseRCAS = (Sharpness > 0.0f);
	if (!bUseRCAS)
	{
		const size_t index = bIsUltraPerformance ? 2 : 3;
		AccumulateParameters->RenderTargets[index] = UpscaledOutput.GetRenderTargetBinding();
	}

	AccumulateParameters->cbArmASR = ArmASRPassParameters;
}
