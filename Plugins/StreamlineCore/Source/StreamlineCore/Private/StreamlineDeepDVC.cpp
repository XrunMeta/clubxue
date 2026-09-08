

#include "StreamlineDeepDVC.h"
#include "StreamlineCore.h"
#include "StreamlineCorePrivate.h"
#include "StreamlineAPI.h"
#include "StreamlineRHI.h"
#include "sl_helpers.h"
#include "sl_deepdvc.h"
#include "UIHintExtractionPass.h"

#include "CoreMinimal.h"
#include "Framework/Application/SlateApplication.h"
#include "RenderGraphBuilder.h"
#include "Runtime/Launch/Resources/Version.h"
#include "ScenePrivate.h"
#include "SystemTextures.h"
#include "HAL/PlatformApplicationMisc.h"

static TAutoConsoleVariable<int32> CVarStreamlineDeepDVCEnable(
	TEXT("r.Streamline.DeepDVC.Enable"),
	0,
	TEXT("DeepDVC mode (default = 0)\n")
	TEXT("0: off\n")
	TEXT("1: always on\n"),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarStreamlineDeepDVCIntensity(
	TEXT("r.Streamline.DeepDVC.Intensity"),
	0.5,
	TEXT("DeepDVC Intensity (default = 0.5, range [0..1])\n")
	TEXT("Controls how strong or subtle the filter effect will be on an image.\n")
	TEXT("A low intensity will keep the images closer to the original, while a high intensity will make the filter effect more pronounced.\n")
	TEXT("Note: '0' disables DeepDVC implicitely\n"),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarStreamlineDeepDVCSaturationBoost(
	TEXT("r.Streamline.DeepDVC.SaturationBoost"),
	0.5,
	TEXT("DeepDVC SaturationBoost(default = 0.5) [0..1]\n")
	TEXT("Enhances the colors in them image, making them more vibrant and eye-catching.\n")
	TEXT("This setting will only be active if r.Streamline.DeepDVC.Intensity is relatively high. Once active, colors pop up more, making the image look more lively.\n")
	TEXT("Note: Applied only when r.Streamline.DeepDVC.Intensity > 0\n"),
	ECVF_Default);

static Streamline::EStreamlineFeatureSupport GStreamlineDeepDVCSupport = Streamline::EStreamlineFeatureSupport::NotSupported;

namespace
{
	float GLastDeepDVCVRAMEstimate = 0;
}

STREAMLINECORE_API Streamline::EStreamlineFeatureSupport QueryStreamlineDeepDVCSupport()
{
	static bool bStreamlineDeepDVCSupportedInitialized = false;

	if (!bStreamlineDeepDVCSupportedInitialized)
	{
		if (!FApp::CanEverRender( ))
		{
			GStreamlineDeepDVCSupport = Streamline::EStreamlineFeatureSupport::NotSupported;
		}
		else if (!IsRHIDeviceNVIDIA())
		{
			GStreamlineDeepDVCSupport = Streamline::EStreamlineFeatureSupport::NotSupportedIncompatibleHardware;
		}
		else if(!IsStreamlineSupported())
		{
			GStreamlineDeepDVCSupport = Streamline::EStreamlineFeatureSupport::NotSupported;
		}
		else
		{
			FStreamlineRHI* StreamlineRHI = GetPlatformStreamlineRHI();
			if (StreamlineRHI->IsDeepDVCSupportedByRHI())
			{
				const sl::Feature Feature = sl::kFeatureDeepDVC;
				sl::Result SupportedResult = SLisFeatureSupported(Feature, *StreamlineRHI->GetAdapterInfo());
				LogStreamlineFeatureSupport(Feature, *StreamlineRHI->GetAdapterInfo());

				GStreamlineDeepDVCSupport = TranslateStreamlineResult(SupportedResult);

			}
			else
			{
				GStreamlineDeepDVCSupport = Streamline::EStreamlineFeatureSupport::NotSupportedIncompatibleRHI;
			}
		}

		bStreamlineDeepDVCSupportedInitialized = true;

		if (Streamline::EStreamlineFeatureSupport::Supported == GStreamlineDeepDVCSupport)
		{

			GetDeepDVCStatusFromStreamline();
		}
	}

	return GStreamlineDeepDVCSupport;
}

bool IsStreamlineDeepDVCSupported()
{
	return Streamline::EStreamlineFeatureSupport::Supported == QueryStreamlineDeepDVCSupport();
}

static sl::DeepDVCMode SLDeepDVCModeFromCvar()
{
	int32 DeepDVCMode = CVarStreamlineDeepDVCEnable.GetValueOnAnyThread();
	switch (DeepDVCMode)
	{
	case 0:
		return sl::DeepDVCMode::eOff;
	case 1:
		return sl::DeepDVCMode::eOn;
	default:
		UE_LOG(LogStreamline, Error, TEXT("Invalid r.Streamline.DeepDVC.Enable value %d"), DeepDVCMode);
		return sl::DeepDVCMode::eOff;
	}
}

bool IsDeepDVCActive()
{
	if (!IsStreamlineDeepDVCSupported())
	{
		return false;
	}
	else
	{
		return SLDeepDVCModeFromCvar() != sl::DeepDVCMode::eOff ? true : false;
	}
}

DECLARE_STATS_GROUP(TEXT("DeepDVC"), STATGROUP_DeepDVC, STATCAT_Advanced);
DECLARE_FLOAT_COUNTER_STAT(TEXT("DeepDVC: VRAM Estimate (MiB)"), STAT_DeepDVCVRAMEstimate, STATGROUP_DeepDVC);

void GetDeepDVCStatusFromStreamline()
{
	GLastDeepDVCVRAMEstimate = 0;

	if (IsStreamlineDeepDVCSupported())
	{

		sl::ViewportHandle Viewport(0);

		sl::DeepDVCState State;

		sl::DeepDVCOptions StreamlineConstantsDeepDVC;

		StreamlineConstantsDeepDVC.mode = SLDeepDVCModeFromCvar();

		CALL_SL_FEATURE_FN(sl::kFeatureDeepDVC, slDeepDVCGetState, Viewport, State);

		GLastDeepDVCVRAMEstimate = float(State.estimatedVRAMUsageInBytes) / (1024 * 1024);
		SET_FLOAT_STAT(STAT_DeepDVCVRAMEstimate, GLastDeepDVCVRAMEstimate);
	}

}

namespace
{

BEGIN_SHADER_PARAMETER_STRUCT(FSLDeepDVCShaderParameters, )
RDG_TEXTURE_ACCESS(SceneColorWithoutHUD, ERHIAccess::UAVCompute)

#if !ENGINE_PROVIDES_UE_5_6_ID3D12DYNAMICRHI_METHODS
SHADER_PARAMETER_STRUCT_INCLUDE(FDebugLayerCompatibilityShaderParameters, DebugLayerCompatibility)
#endif
END_SHADER_PARAMETER_STRUCT()
}

void AddStreamlineDeepDVCStateRenderPass(FRDGBuilder& GraphBuilder, uint32 ViewID, const FIntRect& SecondaryViewRect)
{
	AddStreamlineStateRenderPass(TEXT("DeepDVC"), GraphBuilder, ViewID, SecondaryViewRect,

		[](uint32 ViewID, const FIntRect& SecondaryViewRect) ->sl::DeepDVCOptions
		{

			check(IsStreamlineDeepDVCSupported());
			check(IsInRenderingThread());

			sl::DeepDVCOptions SLConstants;

			SLConstants.mode = SLDeepDVCModeFromCvar();
			SLConstants.intensity = CVarStreamlineDeepDVCIntensity.GetValueOnRenderThread();
			SLConstants.saturationBoost = CVarStreamlineDeepDVCSaturationBoost.GetValueOnRenderThread();

			return SLConstants;
		},

		[](FRHICommandListImmediate& RHICmdList, uint32 ViewID, const FIntRect& SecondaryViewRect, const sl::DeepDVCOptions& Options)
		{
			CALL_SL_FEATURE_FN(sl::kFeatureDeepDVC, slDeepDVCSetOptions, sl::ViewportHandle(ViewID), Options);
		}
	);

}

void AddStreamlineDeepDVCEvaluateRenderPass(FStreamlineRHI* StreamlineRHIExtensions, FRDGBuilder& GraphBuilder, uint32 ViewID, const FIntRect& SecondaryViewRect, FRDGTextureRef SLSceneColorWithoutHUD)
{

	FSLDeepDVCShaderParameters* PassParameters = GraphBuilder.AllocParameters<FSLDeepDVCShaderParameters>();
	PassParameters->SceneColorWithoutHUD = SLSceneColorWithoutHUD;

#if !ENGINE_PROVIDES_UE_5_6_ID3D12DYNAMICRHI_METHODS
	if (StreamlineRHIExtensions->NeedExtraPassesForDebugLayerCompatibility())
	{
		AddDebugLayerCompatibilitySetupPasses(GraphBuilder, &PassParameters->DebugLayerCompatibility);
	}
#endif

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("Streamline DeepDVC Evaluate ViewID=%u", ViewID),
		PassParameters,
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION == 25)
		ERDGPassFlags::Compute,
#else
		ERDGPassFlags::Compute | ERDGPassFlags::Raster | ERDGPassFlags::Copy | 
		ERDGPassFlags::SkipRenderPass | ERDGPassFlags::NeverCull,
#endif
		[StreamlineRHIExtensions, PassParameters, ViewID, SecondaryViewRect](FRHICommandListImmediate& RHICmdList) mutable
		{
			check(PassParameters->SceneColorWithoutHUD);
			PassParameters->SceneColorWithoutHUD->MarkResourceAsUsed();
			FRHIStreamlineResource DeeDVCResource = FRHIStreamlineResource::FromRDGTextureAccess(PassParameters->SceneColorWithoutHUD, SecondaryViewRect, EStreamlineResource::ScalingOutputColor);

#if !ENGINE_PROVIDES_UE_5_6_ID3D12DYNAMICRHI_METHODS
			if (StreamlineRHIExtensions->NeedExtraPassesForDebugLayerCompatibility())
			{
				DebugLayerCompatibilityRHISetup(PassParameters->DebugLayerCompatibility, { DeeDVCResource });
			}
#endif
			const uint64 LocalGFrameCounter = GFrameCounterRenderThread;
			RHICmdList.EnqueueLambda(
				[StreamlineRHIExtensions, DeeDVCResource, ViewID, SecondaryViewRect, LocalGFrameCounter](FRHICommandListImmediate& Cmd) mutable
				{
					sl::FrameToken* FrameToken = FStreamlineCoreModule::GetStreamlineRHI()->GetFrameToken(LocalGFrameCounter);
					StreamlineRHIExtensions->StreamlineEvaluateDeepDVC(Cmd, DeeDVCResource, FrameToken, ViewID);
				});
		});
}

