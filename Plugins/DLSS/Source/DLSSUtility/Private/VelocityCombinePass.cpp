

#include "VelocityCombinePass.h"

#include "Misc/EngineVersionComparison.h"
#include "RenderGraphUtils.h"
#include "SceneView.h"
#include "ScreenPass.h"

#if !UE_VERSION_OLDER_THAN(5,2,0)
#include "DataDrivenShaderPlatformInfo.h"
#endif
#if UE_VERSION_OLDER_THAN(5,3,0)
#include "ScenePrivate.h"
#endif

const int32 kVelocityCombineComputeTileSizeX = FComputeShaderUtils::kGolden2DGroupSize;
const int32 kVelocityCombineComputeTileSizeY = FComputeShaderUtils::kGolden2DGroupSize;

class FDilateMotionVectorsDim : SHADER_PERMUTATION_BOOL("DILATE_MOTION_VECTORS");
class FSupportAlternateMotionVectorDim : SHADER_PERMUTATION_BOOL("SUPPORT_ALTERNATE_MOTION_VECTOR");

class FVelocityCombineCS : public FGlobalShader
{
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{

		return 	IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) &&
				IsPCPlatform(Parameters.Platform) && (
					IsVulkanPlatform(Parameters.Platform) ||
					IsD3DPlatform(Parameters.Platform));
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZEX"), kVelocityCombineComputeTileSizeX);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZEY"), kVelocityCombineComputeTileSizeY);
	}
	using FPermutationDomain = TShaderPermutationDomain<FDilateMotionVectorsDim, FSupportAlternateMotionVectorDim>;

	DECLARE_GLOBAL_SHADER(FVelocityCombineCS);
	SHADER_USE_PARAMETER_STRUCT(FVelocityCombineCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )

		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, VelocityTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, VelocityTextureSampler)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Velocity)

		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, DepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, DepthTextureSampler)

		SHADER_PARAMETER(FVector2f, TemporalJitterPixels)

		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)

		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutVelocityCombinedTexture)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, CombinedVelocity)

		SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float2>, AlternateMotionVectorsTexture)

	END_SHADER_PARAMETER_STRUCT()
};

IMPLEMENT_GLOBAL_SHADER(FVelocityCombineCS, "/Plugin/DLSS/Private/VelocityCombine.usf", "VelocityCombineMain", SF_Compute);

FRDGTextureRef AddVelocityCombinePass(
	FRDGBuilder& GraphBuilder,
#if !UE_VERSION_OLDER_THAN(5,3,0)
	const FSceneView& View,
#else
	const FViewInfo& View,
#endif
	FRDGTextureRef InSceneDepthTexture,
	FRDGTextureRef InVelocityTexture,
	FRDGTextureRef AlternateMotionVectorTexture,
	FIntRect InputViewRect,
	FIntRect DLSSOutputViewRect,
	FVector2f TemporalJitterPixels

)
{

	const  bool bDilateMotionVectors = false;

	const FIntRect OutputViewRect = FIntRect( FIntPoint::ZeroValue, bDilateMotionVectors ? DLSSOutputViewRect.Size() : InputViewRect.Size());

	FRDGTextureDesc CombinedVelocityDesc = FRDGTextureDesc::Create2D(
		OutputViewRect.Size(),
		PF_G16R16F,
		FClearValueBinding::Black,
		TexCreate_ShaderResource | TexCreate_UAV);
	const TCHAR* OutputName = TEXT("DLSSCombinedVelocity");
	FRDGTextureRef CombinedVelocityTexture = GraphBuilder.CreateTexture(
		CombinedVelocityDesc,
		OutputName);

	FVelocityCombineCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FVelocityCombineCS::FParameters>();

	const bool bHasAlternateMotionVectors = AlternateMotionVectorTexture != nullptr;

	{
		PassParameters->VelocityTexture = InVelocityTexture;
		PassParameters->VelocityTextureSampler = TStaticSamplerState<SF_Point>::GetRHI();

		check(InVelocityTexture->Desc.Extent == FIntPoint(1, 1) || InVelocityTexture->Desc.Extent == InSceneDepthTexture->Desc.Extent);
		FScreenPassTextureViewport velocityViewport(InSceneDepthTexture, InputViewRect);
		FScreenPassTextureViewportParameters velocityViewportParameters = GetScreenPassTextureViewportParameters(velocityViewport);
		PassParameters->Velocity = velocityViewportParameters;
	}

	{
		PassParameters->DepthTexture = InSceneDepthTexture;
		PassParameters->DepthTextureSampler = TStaticSamplerState<SF_Point>::GetRHI();
	}

	{
		PassParameters->AlternateMotionVectorsTexture = AlternateMotionVectorTexture;
	}

	{
		PassParameters->OutVelocityCombinedTexture = GraphBuilder.CreateUAV(CombinedVelocityTexture);

		FScreenPassTextureViewport CombinedVelocityViewport(CombinedVelocityTexture, OutputViewRect);
		FScreenPassTextureViewportParameters CombinedVelocityViewportParameters = GetScreenPassTextureViewportParameters(CombinedVelocityViewport);
		PassParameters->CombinedVelocity = CombinedVelocityViewportParameters;
	}

	{
		PassParameters->TemporalJitterPixels = TemporalJitterPixels;
		PassParameters->View = View.ViewUniformBuffer;
	}

	FVelocityCombineCS::FPermutationDomain PermutationVector;
	PermutationVector.Set<FDilateMotionVectorsDim>(bDilateMotionVectors);
	PermutationVector.Set<FSupportAlternateMotionVectorDim>(bHasAlternateMotionVectors);

	const FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());
	TShaderMapRef<FVelocityCombineCS> ComputeShader(ShaderMap, PermutationVector);

	FComputeShaderUtils::AddPass(
		GraphBuilder,
		RDG_EVENT_NAME("Velocity Combine%s%s (%dx%d -> %dx%d)", 
			bDilateMotionVectors ? TEXT(" Dilate") : TEXT(""),
			bHasAlternateMotionVectors ? TEXT(" AlternateMotionVectors") : TEXT("SceneMotionVectors"),
			InputViewRect.Width(), InputViewRect.Height(),
			OutputViewRect.Width(), OutputViewRect.Height()
		),
		ComputeShader,
		PassParameters,
		FComputeShaderUtils::GetGroupCount(OutputViewRect.Size(), FComputeShaderUtils::kGolden2DGroupSize));

	return CombinedVelocityTexture;
}