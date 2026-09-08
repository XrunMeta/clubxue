

#include "StreamlineReflex.h"

#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "RHI.h"
#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
#include "Null/NullPlatformApplicationMisc.h"
#endif
#include "sl_helpers.h"
#include "sl_reflex.h"
#include "sl_pcl.h"
#include "StreamlineAPI.h"
#include "StreamlineCore.h"
#include "StreamlineCorePrivate.h"
#include "StreamlineDLSSG.h"
#include "StreamlineRHI.h"

static TAutoConsoleVariable<bool> CVarStreamlineUnregisterReflexPlugin(
	TEXT("r.Streamline.UnregisterReflexPlugin"),
	true,
	TEXT("The existing NVAPI based UE Reflex plugin is incompatible with the DLSS Frame Generation based implementation. This cvar controls whether the Reflex plugin should be unregistered from the engine or not.\n")
	TEXT("0: keep Reflex plugin modular features registered\n")
	TEXT("1: unregister Reflex plugin modular features. The Reflex blueprint library should work with the DLSS Frame Generation plugin modular features 🤞\n"),
	ECVF_ReadOnly);

static TAutoConsoleVariable<int32> CVarStreamlineReflexEnable(
	TEXT("t.Streamline.Reflex.Enable"),
	0,
	TEXT("Enable Streamline Reflex extension. (default = 0)\n")
	TEXT("0: Disabled\n")
	TEXT("1: Enabled)\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarStreamlineReflexEnableLatencyMarkers(
	TEXT("t.Streamline.Reflex.EnableLatencyMarkers"),
	1,
	TEXT("Enable Streamline PC Latency metrics. (default = 1)\n")
	TEXT("0: Disabled\n")
	TEXT("1: Enabled)\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarStreamlineReflexAuto(
	TEXT("t.Streamline.Reflex.Auto"),
	1,
	TEXT("Enable Streamline Reflex extension when other SL features need it. (default = 1)\n")
	TEXT("0: Disabled\n")
	TEXT("1: Enabled)\n"),
	ECVF_ReadOnly);

static TAutoConsoleVariable<bool> CVarStreamlineReflexEnableInEditor(
	TEXT("t.Streamline.Reflex.EnableInEditor"),
	true,
	TEXT("Enable Streamline Reflex and PC Latency in the editor. (default = 1)\n")
	TEXT("0: Disabled\n")
	TEXT("1: Enabled)\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarStreamlineReflexMode(
	TEXT("t.Streamline.Reflex.Mode"),
	1,
	TEXT("Streamline Reflex mode (default = 1)\n")
	TEXT("0: off \n")
	TEXT("1: low latency\n")
	TEXT("2: low latency with boost\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<bool> CVarStreamlineReflexHandleMaxTickRate(
	TEXT("t.Streamline.Reflex.HandleMaxTickRate"),
	true,
	TEXT("Controls whether Streamline Reflex handles frame rate limiting instead of the engine (default = true)"),
	ECVF_Default);

TUniquePtr<FStreamlineMaxTickRateHandler> FStreamlineMaxTickRateHandler::StreamlineMaxTickRateHandler = nullptr;
TUniquePtr<FStreamlineLatencyMarkers> FStreamlineLatencyMarkers::StreamlineLatencyMarkers = nullptr;

bool FStreamlineLatencyBase::bStreamlineReflexSupported = false;
bool FStreamlineLatencyBase::IsStreamlineReflexSupported()
{
	if (!FApp::CanEverRender())
	{
		return false;
	}

	if (!IsStreamlineSupported())
	{
		return false;
	}

#if WITH_EDITOR
	if (GIsEditor && !CVarStreamlineReflexEnableInEditor.GetValueOnAnyThread())
	{
		return false;
	}
#endif

	static bool bStreamlineReflexSupportedInitialized = false;

	if (!bStreamlineReflexSupportedInitialized)
	{
		const sl::AdapterInfo* AdapterInfo = FStreamlineCoreModule::GetStreamlineRHI()->GetAdapterInfo();
		sl::Result Result = SLisFeatureSupported(sl::kFeatureReflex, *AdapterInfo);
		LogStreamlineFeatureSupport(sl::kFeatureReflex, *AdapterInfo);

		if (sl::Result::eOk == Result)
		{
			sl::ReflexState ReflexState{};
			sl::Result ResultReflexGetState = CALL_SL_FEATURE_FN(sl::kFeatureReflex, slReflexGetState, ReflexState);

			checkf(ResultReflexGetState == sl::Result::eOk, TEXT("slReflexGetState failed (%s)"), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));
			if (ResultReflexGetState == sl::Result::eOk)
			{
				UE_LOG(LogStreamline, Log, TEXT("%s sl::ReflexState::lowLatencyAvailable=%u"), ANSI_TO_TCHAR(__FUNCTION__), ReflexState.lowLatencyAvailable);
				bStreamlineReflexSupported = ReflexState.lowLatencyAvailable;
			}
			else
			{
				UE_LOG(LogStreamline, Log, TEXT("%s slReflexGetState failed (%s)"), ANSI_TO_TCHAR(__FUNCTION__), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));
			}
		}

		bStreamlineReflexSupportedInitialized = true;
	}

	return bStreamlineReflexSupported;
}

bool FStreamlineLatencyBase::bStreamlinePCLSupported = false;
bool FStreamlineLatencyBase::IsStreamlinePCLSupported()
{
	if (!FApp::CanEverRender())
	{
		return false;
	}

	if (!IsStreamlineSupported())
	{
		return false;
	}

#if WITH_EDITOR
	if (GIsEditor && !CVarStreamlineReflexEnableInEditor.GetValueOnAnyThread())
	{
		return false;
	}
#endif

	static bool bStreamlinePCLSupportedInitialized = false;

	if (!bStreamlinePCLSupportedInitialized)
	{
		const sl::AdapterInfo* AdapterInfo = FStreamlineCoreModule::GetStreamlineRHI()->GetAdapterInfo();
		sl::Result Result = SLisFeatureSupported(sl::kFeaturePCL, *AdapterInfo);
		LogStreamlineFeatureSupport(sl::kFeaturePCL, *AdapterInfo);

		bStreamlinePCLSupported = (Result == sl::Result::eOk);
		bStreamlinePCLSupportedInitialized = true;
	}

	return bStreamlinePCLSupported;
}

void FStreamlineMaxTickRateHandler::Initialize()
{
	if (IsStreamlineReflexSupported())
	{
		const sl::ReflexOptions MakeSureStreamlineReflexCallsNVSTATS_INITAtLeastOnce;
		CALL_SL_FEATURE_FN(sl::kFeatureReflex, slReflexSetOptions, MakeSureStreamlineReflexCallsNVSTATS_INITAtLeastOnce);
	}
	else
	{
		CVarStreamlineReflexEnable->Set(false, ECVF_SetByCommandline);
	}
}

void FStreamlineMaxTickRateHandler::SetEnabled(bool bInEnabled)
{
	if (!GetAvailable())
	{
		bInEnabled = false;
		UE_LOG(LogStreamline, Log, TEXT("%s Tried to set SL Reflex Low Latency state but SL Reflex is not available"), ANSI_TO_TCHAR(__FUNCTION__));
	}

	CVarStreamlineReflexEnable->Set(bInEnabled, ECVF_SetByCommandline);
}

bool DoActiveStreamlineFeaturesRequireReflex()
{
	return IsDLSSGActive();
}

bool FStreamlineMaxTickRateHandler::GetEnabled()
{
	if (!GetAvailable())
	{
		return false;
	}
#if WITH_EDITOR
	if (GIsEditor && !CVarStreamlineReflexEnableInEditor.GetValueOnAnyThread())
	{
		return false;
	}
#endif

	int32 CVarReflexEnable = CVarStreamlineReflexEnable.GetValueOnAnyThread();
	int32 CVarReflexAuto = CVarStreamlineReflexAuto.GetValueOnAnyThread();
	if ((CVarReflexAuto) != 0 && DoActiveStreamlineFeaturesRequireReflex())
	{
		return true;
	}
	else
	{
		return CVarReflexEnable != 0;
	}

}

bool FStreamlineMaxTickRateHandler::GetAvailable()
{
	if (!IsStreamlineReflexSupported())
	{
		return false;
	}

	sl::ReflexState ReflexState{};
	sl::Result Result = CALL_SL_FEATURE_FN(sl::kFeatureReflex, slReflexGetState, ReflexState);
	checkf(Result == sl::Result::eOk, TEXT("slReflexGetState failed (%s)"), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));

	return ReflexState.lowLatencyAvailable;
}

void FStreamlineMaxTickRateHandler::SetFlags(uint32 Flags)
{
	static_assert(sl::ReflexMode_eCount == 3U, "sl::ReflexMode_eCount enum value mismatch. Dear NVIDIA Streamline plugin developer, please update this code!");

	switch (static_cast<ReflexFlags>(Flags & uint32(ReflexFlags::AllBits)))
	{
	default:
		UE_LOG(LogStreamline, Error, TEXT("invalid FStreamlineMaxTickRateHandler::GetFlags=%u"), GetFlags());

	case ReflexFlags::ReflexOff:  
		CVarStreamlineReflexMode->Set(sl::ReflexMode::eOff, ECVF_SetByCommandline);
		break;

	case ReflexFlags::ReflexOn:   
		CVarStreamlineReflexMode->Set(sl::ReflexMode::eLowLatency, ECVF_SetByCommandline);
		break;

	case ReflexFlags::ReflexBoost:
		CVarStreamlineReflexMode->Set(sl::ReflexMode::eLowLatencyWithBoost, ECVF_SetByCommandline);
		break;
	}
}

uint32 FStreamlineMaxTickRateHandler::GetFlags()
{
	int32 CVarReflexMode = FMath::Clamp(CVarStreamlineReflexMode.GetValueOnAnyThread(), sl::eOff, sl::eLowLatencyWithBoost);
	int32 CVarReflexAuto = CVarStreamlineReflexAuto.GetValueOnAnyThread();

	if ((CVarReflexAuto != 0) && DoActiveStreamlineFeaturesRequireReflex() && CVarReflexMode == sl::eOff)
	{
		return uint32(ReflexFlags::ReflexOn);
	}
	else
	{
		switch (CVarReflexMode)
		{
			default:
			case sl::eOff:                 return uint32(ReflexFlags::ReflexOff);
			case sl::eLowLatency:          return uint32(ReflexFlags::ReflexOn);
			case sl::eLowLatencyWithBoost: return uint32(ReflexFlags::ReflexBoost);
		}
	}
}

FStreamlineMaxTickRateHandler* FStreamlineMaxTickRateHandler::Get(bool bCreateIfInvalid)
{
	if (!StreamlineMaxTickRateHandler && bCreateIfInvalid)
	{
		StreamlineMaxTickRateHandler = MakeUnique<FStreamlineMaxTickRateHandler>();
		StreamlineMaxTickRateHandler->Initialize();
	}

	return StreamlineMaxTickRateHandler.Get();
}

void FStreamlineMaxTickRateHandler::Reset()
{
	StreamlineMaxTickRateHandler.Reset();
}

DECLARE_CYCLE_STAT(TEXT("Game thread wait time (Reflex)"), STAT_GameTickReflexWaitTime, STATGROUP_Threading);

static float CalculateDesiredMinimumIntervalUs(float DesiredMaxTickRate, float DeltaRealTimeMinusSleep)
{
	const float DesiredMinimumInterval = DesiredMaxTickRate > 0 ? (1.0f / DesiredMaxTickRate) : 0.0f;

	if (DesiredMinimumInterval < DeltaRealTimeMinusSleep)
	{
		return 0.0f;
	}

	return 1.0E6f * DesiredMinimumInterval;
}

static bool AreReflexOptionsEquivalent(const sl::ReflexOptions& Opt1, const sl::ReflexOptions& Opt2)
{

	return (Opt1.mode == Opt2.mode)
		&& (Opt1.frameLimitUs == Opt2.frameLimitUs)
		&& (Opt1.useMarkersToOptimize == Opt2.useMarkersToOptimize)
		&& (Opt1.virtualKey == Opt2.virtualKey)
		&& (Opt1.idThread == Opt2.idThread);
}

static void UpdateReflexOptionsIfChanged(const sl::ReflexOptions& ReflexOptions)
{
	static sl::ReflexOptions LastFrameOptions{};
	if (!(StreamlineFilterRedundantSetOptionsCalls() && AreReflexOptionsEquivalent(LastFrameOptions, ReflexOptions)))
	{
		sl::Result Result = CALL_SL_FEATURE_FN(sl::kFeatureReflex, slReflexSetOptions, ReflexOptions);
		checkf(Result == sl::Result::eOk, TEXT("slReflexSetOptions failed (%s)"), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));
		LastFrameOptions = ReflexOptions;
	}
}

bool FStreamlineMaxTickRateHandler::HandleMaxTickRate(float DesiredMaxTickRate)
{
	bool bFrameRateHandled = false;

	if (IsStreamlineReflexSupported())
	{
		SCOPE_CYCLE_COUNTER(STAT_GameTickReflexWaitTime);
		const double CurrentRealTime = FPlatformTime::Seconds();
		static double LastRealTimeAfterSleep = CurrentRealTime - 0.0001;
		const float DeltaRealTimeMinusSleep = static_cast<float>(CurrentRealTime - LastRealTimeAfterSleep);

		sl::ReflexOptions ReflexOptions = {};

		static_assert(sl::ReflexMode_eCount == 3U, "sl::ReflexMode_eCount enum value mismatch. Dear NVIDIA Streamline plugin developer, please update this code!");
		switch (static_cast<ReflexFlags>(GetFlags() & uint32(ReflexFlags::AllBits)))
		{
			case ReflexFlags::ReflexInvalidFlags:
				UE_LOG(LogStreamline, Error, TEXT("invalid FStreamlineMaxTickRateHandler::GetFlags=%u"), GetFlags());

			case ReflexFlags::ReflexOff:
				ReflexOptions.mode = sl::ReflexMode::eOff;

				ReflexOptions.useMarkersToOptimize = false;
				break;
			case ReflexFlags::ReflexOn:
				ReflexOptions.mode = sl::ReflexMode::eLowLatency;
				ReflexOptions.useMarkersToOptimize = true;
				break;
			case ReflexFlags::ReflexBoost:
				ReflexOptions.mode = sl::ReflexMode::eLowLatencyWithBoost;
				ReflexOptions.useMarkersToOptimize = true;
				break;
		}

		if (!CVarStreamlineReflexHandleMaxTickRate.GetValueOnAnyThread() || GIsEditor
#if (ENGINE_MAJOR_VERSION < 5) || (ENGINE_MINOR_VERSION < 2)

			|| GEngine->IsAllowedFramerateSmoothing()
#endif
			)
		{

			ReflexOptions.frameLimitUs = 0;
		}
		else
		{
			const float DesiredMinimumIntervalUs = CalculateDesiredMinimumIntervalUs(DesiredMaxTickRate, DeltaRealTimeMinusSleep);
#if ENGINE_MAJOR_VERSION > 4
			ReflexOptions.frameLimitUs = FMath::TruncToInt32(DesiredMinimumIntervalUs);
#else
			ReflexOptions.frameLimitUs = FMath::TruncToInt(DesiredMinimumIntervalUs);
#endif
			bFrameRateHandled = true;
		}

		UpdateReflexOptionsIfChanged(ReflexOptions);

		TRACE_CPUPROFILER_EVENT_SCOPE(ReflexSleep);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
		UE::Stats::FThreadIdleStats::FScopeIdle Scope;
#else
		FThreadIdleStats::FScopeIdle Scope;
#endif
		sl::FrameToken* FrameToken = FStreamlineCoreModule::GetStreamlineRHI()->GetFrameToken(GFrameCounter);
		sl::Result Result = CALL_SL_FEATURE_FN(sl::kFeatureReflex, slReflexSleep, *FrameToken);
		checkf(Result == sl::Result::eOk, TEXT("slReflexSleep failed (%s)"), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));
		LastRealTimeAfterSleep = FPlatformTime::Seconds();
	}

	return bFrameRateHandled;
}

void FStreamlineLatencyMarkers::Initialize()
{
	if (IsStreamlineReflexSupported())
	{
		sl::ReflexState ReflexState{};
		sl::Result Result = CALL_SL_FEATURE_FN(sl::kFeatureReflex, slReflexGetState, ReflexState);
		checkf(Result == sl::Result::eOk, TEXT("slReflexGetState failed (%s)"), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));

		UE_LOG(LogStreamline, Log, TEXT("%s sl::ReflexState::flashIndicatorDriverControlled=%u"), ANSI_TO_TCHAR(__FUNCTION__), ReflexState.flashIndicatorDriverControlled);
		bFlashIndicatorDriverControlled = ReflexState.flashIndicatorDriverControlled;
	}
}

void FStreamlineLatencyMarkers::Tick(float DeltaTime)
{
	if (IsStreamlineReflexSupported() && GetAvailable())
	{
		sl::ReflexState ReflexState{};
		sl::Result Result = CALL_SL_FEATURE_FN(sl::kFeatureReflex, slReflexGetState, ReflexState);
		checkf(Result == sl::Result::eOk, TEXT("slReflexGetState failed (%s)"), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));

		if (ReflexState.latencyReportAvailable)
		{

			const uint64_t TotalLatencyUs = ReflexState.frameReport[63].gpuRenderEndTime - ReflexState.frameReport[63].simStartTime;

			if (TotalLatencyUs != 0)
			{

				AverageTotalLatencyMs = AverageTotalLatencyMs * 0.75f + TotalLatencyUs / 1000.0f * 0.25f;

				AverageGameLatencyMs = AverageGameLatencyMs * 0.75f + (ReflexState.frameReport[63].driverEndTime - ReflexState.frameReport[63].simStartTime) / 1000.0f * 0.25f;
				AverageRenderLatencyMs = AverageRenderLatencyMs * 0.75f + (ReflexState.frameReport[63].gpuRenderEndTime - ReflexState.frameReport[63].osRenderQueueStartTime) / 1000.0f * 0.25f;

				AverageSimulationLatencyMs = AverageSimulationLatencyMs * 0.75f + (ReflexState.frameReport[63].simEndTime - ReflexState.frameReport[63].simStartTime) / 1000.0f * 0.25f;
				AverageRenderSubmitLatencyMs = AverageRenderSubmitLatencyMs * 0.75f + (ReflexState.frameReport[63].renderSubmitEndTime - ReflexState.frameReport[63].renderSubmitStartTime) / 1000.0f * 0.25f;
				AveragePresentLatencyMs = AveragePresentLatencyMs * 0.75f + (ReflexState.frameReport[63].presentEndTime - ReflexState.frameReport[63].presentStartTime) / 1000.0f * 0.25f;
				AverageDriverLatencyMs = AverageDriverLatencyMs * 0.75f + (ReflexState.frameReport[63].driverEndTime - ReflexState.frameReport[63].driverStartTime) / 1000.0f * 0.25f;
				AverageOSRenderQueueLatencyMs = AverageOSRenderQueueLatencyMs * 0.75f + (ReflexState.frameReport[63].osRenderQueueEndTime - ReflexState.frameReport[63].osRenderQueueStartTime) / 1000.0f * 0.25f;
				AverageGPURenderLatencyMs = AverageGPURenderLatencyMs * 0.75f + (ReflexState.frameReport[63].gpuRenderEndTime - ReflexState.frameReport[63].gpuRenderStartTime) / 1000.0f * 0.25f;

				RenderSubmitOffsetMs = (ReflexState.frameReport[63].renderSubmitStartTime - ReflexState.frameReport[63].simStartTime) / 1000.0f;
				PresentOffsetMs = (ReflexState.frameReport[63].presentStartTime - ReflexState.frameReport[63].simStartTime) / 1000.0f;
				DriverOffsetMs = (ReflexState.frameReport[63].driverStartTime - ReflexState.frameReport[63].simStartTime) / 1000.0f;
				OSRenderQueueOffsetMs = (ReflexState.frameReport[63].osRenderQueueStartTime - ReflexState.frameReport[63].simStartTime) / 1000.0f;
				GPURenderOffsetMs = (ReflexState.frameReport[63].gpuRenderStartTime - ReflexState.frameReport[63].simStartTime) / 1000.0f;

				UE_LOG(LogStreamline, VeryVerbose, TEXT("AverageTotalLatencyMs: %f"), AverageTotalLatencyMs);
				UE_LOG(LogStreamline, VeryVerbose, TEXT("AverageGameLatencyMs: %f"), AverageGameLatencyMs);
				UE_LOG(LogStreamline, VeryVerbose, TEXT("AverageRenderLatencyMs: %f"), AverageRenderLatencyMs);

				UE_LOG(LogStreamline, VeryVerbose, TEXT("AverageSimulationLatencyMs: %f"), AverageSimulationLatencyMs);
				UE_LOG(LogStreamline, VeryVerbose, TEXT("AverageRenderSubmitLatencyMs: %f"), AverageRenderSubmitLatencyMs);
				UE_LOG(LogStreamline, VeryVerbose, TEXT("AveragePresentLatencyMs: %f"), AveragePresentLatencyMs);
				UE_LOG(LogStreamline, VeryVerbose, TEXT("AverageDriverLatencyMs: %f"), AverageDriverLatencyMs);
				UE_LOG(LogStreamline, VeryVerbose, TEXT("AverageOSRenderQueueLatencyMs: %f"), AverageOSRenderQueueLatencyMs);
				UE_LOG(LogStreamline, VeryVerbose, TEXT("AverageGPURenderLatencyMs: %f"), AverageGPURenderLatencyMs);
			}
		}
	}
	else
	{

		AverageTotalLatencyMs = 0.0f;
		AverageGameLatencyMs = 0.0f;
		AverageRenderLatencyMs = 0.0f;

		AverageSimulationLatencyMs = 0.0f;
		AverageRenderSubmitLatencyMs = 0.0f;
		AveragePresentLatencyMs = 0.0f;
		AverageDriverLatencyMs = 0.0f;
		AverageOSRenderQueueLatencyMs = 0.0f;
		AverageGPURenderLatencyMs = 0.0f;

		RenderSubmitOffsetMs = 0.0f;
		PresentOffsetMs = 0.0f;
		DriverOffsetMs = 0.0f;
		OSRenderQueueOffsetMs = 0.0f;
		GPURenderOffsetMs = 0.0f;

		AverageTotalLatencyMs = 0.0f;
		AverageGameLatencyMs = 0.0f;
		AverageRenderLatencyMs = 0.0f;

		AverageSimulationLatencyMs = 0.0f;
		AverageRenderSubmitLatencyMs = 0.0f;
		AveragePresentLatencyMs = 0.0f;
		AverageDriverLatencyMs = 0.0f;
		AverageOSRenderQueueLatencyMs = 0.0f;
		AverageGPURenderLatencyMs = 0.0f;

		RenderSubmitOffsetMs = 0.0f;
		PresentOffsetMs = 0.0f;
		DriverOffsetMs = 0.0f;
		OSRenderQueueOffsetMs = 0.0f;
		GPURenderOffsetMs = 0.0f;
	}
}

void FStreamlineLatencyMarkers::SetInputSampleLatencyMarker(uint64)
{

}

void FStreamlineLatencyMarkers::SetSimulationLatencyMarkerStart(uint64 FrameNumber)
{
	SetCustomLatencyMarker(uint32(sl::PCLMarker::eSimulationStart), FrameNumber);
}

void FStreamlineLatencyMarkers::SetSimulationLatencyMarkerEnd(uint64 FrameNumber)
{
	SetCustomLatencyMarker(uint32(sl::PCLMarker::eSimulationEnd), FrameNumber);
}

void FStreamlineLatencyMarkers::SetRenderSubmitLatencyMarkerStart(uint64 FrameNumber)
{
	SetCustomLatencyMarker(uint32(sl::PCLMarker::eRenderSubmitStart), FrameNumber);
}

void FStreamlineLatencyMarkers::SetRenderSubmitLatencyMarkerEnd(uint64 FrameNumber)
{
	SetCustomLatencyMarker(uint32(sl::PCLMarker::eRenderSubmitEnd), FrameNumber);
}

void FStreamlineLatencyMarkers::SetPresentLatencyMarkerStart(uint64 FrameNumber)
{
	SetCustomLatencyMarker(uint32(sl::PCLMarker::ePresentStart), FrameNumber);
}

void FStreamlineLatencyMarkers::SetPresentLatencyMarkerEnd(uint64 FrameNumber)
{
	SetCustomLatencyMarker(uint32(sl::PCLMarker::ePresentEnd), FrameNumber);

	GetDLSSGStatusFromStreamline();
}

void FStreamlineLatencyMarkers::SetFlashIndicatorLatencyMarker(uint64 FrameNumber)
{
	if (GetFlashIndicatorEnabled())
	{
		SetCustomLatencyMarker(uint32(sl::PCLMarker::eTriggerFlash), FrameNumber );
	}

}

void FStreamlineLatencyMarkers::SetCustomLatencyMarker(uint32 MarkerId, uint64 FrameNumber)
{
	if (IsStreamlinePCLSupported() &&  GetEnabled())
	{
		sl::PCLMarker Marker = static_cast<sl::PCLMarker>(MarkerId);
		sl::FrameToken* FrameToken = FStreamlineCoreModule::GetStreamlineRHI()->GetFrameToken(FrameNumber);
		sl::Result Result = CALL_SL_FEATURE_FN(sl::kFeaturePCL, slPCLSetMarker, Marker, *FrameToken);
		checkf(Result == sl::Result::eOk, TEXT("slPCLSetMarker failed MarkerId=%s (%s)"),
			ANSI_TO_TCHAR(sl::getPCLMarkerAsStr(Marker)), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));
	}
}

bool FStreamlineLatencyMarkers::ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32& OutResult)
{
	if (IsStreamlinePCLSupported() && GetEnabled())
	{
		sl::PCLState LatencySettings{};
		sl::Result Result = CALL_SL_FEATURE_FN(sl::kFeaturePCL, slPCLGetState, LatencySettings);
		checkf(Result == sl::Result::eOk, TEXT("slPCLGetState failed (%s)"), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));

		if(LatencySettings.statsWindowMessage == msg)
		{

			sl::FrameToken* FrameToken = FStreamlineCoreModule::GetStreamlineRHI()->GetFrameToken(GFrameCounter);
			Result = CALL_SL_FEATURE_FN(sl::kFeaturePCL, slPCLSetMarker, sl::PCLMarker::ePCLatencyPing, *FrameToken);
			checkf(Result == sl::Result::eOk, TEXT("slPCLSetMarker ePCLatencyPing failed (%s)"), ANSI_TO_TCHAR(sl::getResultAsStr(Result)));
			return true;
		}
	}
	return false;
}

FStreamlineLatencyMarkers* FStreamlineLatencyMarkers::Get(bool bCreateIfInvalid)
{
	if (!StreamlineLatencyMarkers && bCreateIfInvalid)
	{
		StreamlineLatencyMarkers = MakeUnique<FStreamlineLatencyMarkers>();
		StreamlineLatencyMarkers->Initialize();
	}

	return StreamlineLatencyMarkers.Get();
}

void FStreamlineLatencyMarkers::Reset()
{
	StreamlineLatencyMarkers.Reset();
}

void FStreamlineLatencyMarkers::SetEnabled(bool bInEnabled)
{
	CVarStreamlineReflexEnableLatencyMarkers->Set(bInEnabled, ECVF_SetByCommandline);
}

bool FStreamlineLatencyMarkers::GetEnabled()
{
	if (!GetAvailable())
	{
		return false;
	}

	int32 CVarReflex = CVarStreamlineReflexEnableLatencyMarkers.GetValueOnAnyThread();
	return CVarReflex != 0;
}

bool FStreamlineLatencyMarkers::GetAvailable()
{
	return IsStreamlinePCLSupported();
}

void FStreamlineLatencyMarkers::SetFlashIndicatorEnabled(bool bInEnabled)
{
	UE_LOG(LogStreamline, Log, TEXT("FStreamlineLatencyMarkers::SetFlashIndicatorEnabled is obsolete and non-functional. The Reflex Flash Indicator is configured by the NVIDIA GeForce Experience overlay"));
}

bool FStreamlineLatencyMarkers::GetFlashIndicatorEnabled()
{
	return GetEnabled() && bFlashIndicatorDriverControlled;
}

FStreamlineMaxTickRateHandler* GetStreamlineReflexMaxTickRateHandler()
{
	return FStreamlineMaxTickRateHandler::Get();
}

FStreamlineLatencyMarkers* GetStreamlineReflexLatencyMarkerModule()
{
	return FStreamlineLatencyMarkers::Get();
}

static Streamline::EStreamlineFeatureSupport GStreamlineReflexSupport = Streamline::EStreamlineFeatureSupport::NotSupported;

Streamline::EStreamlineFeatureSupport QueryStreamlineReflexSupport()
{
	static bool bStreamlineDLSSGSupportedInitialized = false;

	if (!bStreamlineDLSSGSupportedInitialized)
	{

		if (!FApp::CanEverRender())
		{
			GStreamlineReflexSupport = Streamline::EStreamlineFeatureSupport::NotSupported;
		}
		else if (!IsRHIDeviceNVIDIA())
		{
			GStreamlineReflexSupport = Streamline::EStreamlineFeatureSupport::NotSupportedIncompatibleHardware;
		}
		else if (!IsStreamlineSupported())
		{
			GStreamlineReflexSupport = Streamline::EStreamlineFeatureSupport::NotSupported;
		}
		else
		{
			FStreamlineRHI* StreamlineRHI = GetPlatformStreamlineRHI();
			if (StreamlineRHI->IsReflexSupportedByRHI())
			{
				const sl::Feature Feature = sl::kFeatureReflex;
				sl::Result SupportedResult = SLisFeatureSupported(Feature, *StreamlineRHI->GetAdapterInfo());
				LogStreamlineFeatureSupport(Feature, *StreamlineRHI->GetAdapterInfo());

				GStreamlineReflexSupport = TranslateStreamlineResult(SupportedResult);

			}
			else
			{
				GStreamlineReflexSupport = Streamline::EStreamlineFeatureSupport::NotSupportedIncompatibleRHI;
			}
		}

		bStreamlineDLSSGSupportedInitialized = true;

		if (Streamline::EStreamlineFeatureSupport::Supported == GStreamlineReflexSupport)
		{

			GetDLSSGStatusFromStreamline(true);
		}
	}

	return GStreamlineReflexSupport;
}

bool IsStreamlineReflexSupported()
{
	return Streamline::EStreamlineFeatureSupport::Supported == QueryStreamlineReflexSupport();
}

void RegisterStreamlineReflexHooks()
{
	UE_LOG(LogStreamline, Log, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));

	TSharedPtr<IPlugin> ReflexPlugin = IPluginManager::Get().FindPlugin(TEXT("Reflex"));
	const bool bIsReflexPluginEnabled = ReflexPlugin && (ReflexPlugin->IsEnabled() || ReflexPlugin->IsEnabledByDefault(false));
	if (bIsReflexPluginEnabled)
	{
		UE_LOG(LogStreamline, Log, TEXT("Reflex plugin enabled, which is incompatible with the Reflex implementation provided by this Streamline UE plugin"));

		if (CVarStreamlineUnregisterReflexPlugin.GetValueOnAnyThread() != 0)
		{
			UE_LOG(LogStreamline, Log, TEXT("Unregistering the Reflex plugin related modular features. The Reflex plugin Blueprint library is expected to continue to work 🤞."));

			auto UnregisterFeatures = [](const FName& FeatureName)
			{
				TArray< IModularFeature*> Features = IModularFeatures::Get().GetModularFeatureImplementations< IModularFeature>(FeatureName);
				for (IModularFeature* Feature : Features)
				{
					UE_LOG(LogStreamline, Log, TEXT("Unregistering %s %p "), *FeatureName.ToString(), Feature);
					IModularFeatures::Get().UnregisterModularFeature(FeatureName, Feature);
				}
			};

			UnregisterFeatures(IMaxTickRateHandlerModule::GetModularFeatureName());
			UnregisterFeatures(ILatencyMarkerModule::GetModularFeatureName());
		}
		else
		{
			UE_LOG(LogStreamline, Log, TEXT("It is recommended to either disable the Reflex plugin or set r.Streamline.UnregisterReflexPlugin to disable the incompatible parts of the Reflex plugin."));
		}
	}

	IModularFeatures::Get().RegisterModularFeature(FStreamlineMaxTickRateHandler::Get()->GetModularFeatureName(), FStreamlineMaxTickRateHandler::Get());

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	if (FNullPlatformApplicationMisc::IsUsingNullApplication())
	{
		UE_LOG(LogStreamline, Log, TEXT("Reflex latency markers unsupported with -RenderOffScreen"));
	}
	else
#endif
	{
		IModularFeatures::Get().RegisterModularFeature(FStreamlineLatencyMarkers::Get()->GetModularFeatureName(), FStreamlineLatencyMarkers::Get());

		check(FSlateApplication::IsInitialized());
		FWindowsApplication* WindowsApplication = (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();
		check(WindowsApplication);
		WindowsApplication->AddMessageHandler(*FStreamlineLatencyMarkers::Get());

		FSlateApplication::Get().OnPreShutdown().AddLambda(
			[]()
		{
			FWindowsApplication* WindowsApplication = (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();
			check(WindowsApplication);
			WindowsApplication->RemoveMessageHandler(*FStreamlineLatencyMarkers::Get());
		}
		);
	}
	UE_LOG(LogStreamline, Log, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
}

void UnregisterStreamlineReflexHooks()
{
	UE_LOG(LogStreamline, Log, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));
	IModularFeatures::Get().UnregisterModularFeature(FStreamlineMaxTickRateHandler::Get()->GetModularFeatureName(), FStreamlineMaxTickRateHandler::Get());
	FStreamlineMaxTickRateHandler::Reset();

	IModularFeatures::Get().UnregisterModularFeature(FStreamlineLatencyMarkers::Get()->GetModularFeatureName(), FStreamlineLatencyMarkers::Get());

	FStreamlineLatencyMarkers::Reset();

	UE_LOG(LogStreamline, Log, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
}
