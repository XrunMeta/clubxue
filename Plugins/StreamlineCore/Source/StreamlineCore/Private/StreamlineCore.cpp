

#include "StreamlineCore.h"
#include "StreamlineCorePrivate.h"
#include "CoreMinimal.h"

#include "StreamlineSettings.h"
#include "StreamlineViewExtension.h"
#include "StreamlineReflex.h"
#include "StreamlineDLSSG.h"
#include "StreamlineDeepDVC.h"

#include "StreamlineRHI.h"
#include "sl_helpers.h"

#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#include "GeneralProjectSettings.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif
#include "SceneViewExtension.h"
#include "SceneView.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "FStreamlineModule"
DEFINE_LOG_CATEGORY(LogStreamline);

Streamline::EStreamlineFeatureSupport TranslateStreamlineResult(sl::Result Result)
{
	switch (Result)
	{

	case sl::Result::eOk:					return Streamline::EStreamlineFeatureSupport::Supported;
	case sl::Result::eErrorOSDisabledHWS:   return Streamline::EStreamlineFeatureSupport::NotSupportedHardwareSchedulingDisabled;
	case sl::Result::eErrorOSOutOfDate: return Streamline::EStreamlineFeatureSupport::NotSupportedOperatingSystemOutOfDate;
	case sl::Result::eErrorDriverOutOfDate: return Streamline::EStreamlineFeatureSupport::NotSupportedDriverOutOfDate;
	case sl::Result::eErrorNoSupportedAdapterFound: return Streamline::EStreamlineFeatureSupport::NotSupportedIncompatibleHardware;
	case sl::Result::eErrorAdapterNotSupported: return Streamline::EStreamlineFeatureSupport::NotSupportedIncompatibleHardware;
	case sl::Result::eErrorMissingOrInvalidAPI: return Streamline::EStreamlineFeatureSupport::NotSupportedIncompatibleRHI;

	default:

		return Streamline::EStreamlineFeatureSupport::NotSupported;
	}
}

void FStreamlineCoreModule::StartupModule()
{
	auto CVarInitializePlugin = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streamline.InitializePlugin"));
	if (CVarInitializePlugin && !CVarInitializePlugin->GetBool())
	{
		UE_LOG(LogStreamline, Log, TEXT("Initialization of StreamlineCore is disabled."));
		return;
	}

	UE_LOG(LogStreamline, Log, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));

	if (GetPlatformStreamlineSupport() == EStreamlineSupport::Supported)
	{

		bool bShouldCreateViewExtension = IsStreamlineDLSSGSupported() || IsStreamlineDeepDVCSupported();
		if (FParse::Param(FCommandLine::Get(), TEXT("slviewextension")))
		{
			bShouldCreateViewExtension = true;
		}
		if (FParse::Param(FCommandLine::Get(), TEXT("slnoviewextension")))
		{
			bShouldCreateViewExtension = false;
		}
		if (bShouldCreateViewExtension)
		{
			StreamlineViewExtension = FSceneViewExtensions::NewExtension<FStreamlineViewExtension>(GetStreamlineRHI());
		}
		else
		{
			StreamlineViewExtension = nullptr;
		}

		RegisterStreamlineReflexHooks();

		if (ForceTagStreamlineBuffers() || IsStreamlineDLSSGSupported())
		{
			RegisterStreamlineDLSSGHooks(GetStreamlineRHI());
		}

		LogStreamlineFeatureSupport(sl::kFeatureImGUI, *GetStreamlineRHI()->GetAdapterInfo());
	}

	UE_LOG(LogStreamline, Log, TEXT("NVIDIA Streamline supported %u"), QueryStreamlineSupport() == EStreamlineSupport::Supported);

#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		UStreamlineSettings* Settings = GetMutableDefault<UStreamlineSettings>();
		SettingsModule->RegisterSettings("Project", "Plugins", "Streamline",
			LOCTEXT("StreamlineSettingsName", "NVIDIA Streamline"),
			LOCTEXT("StreamlineSettingsDecription", "Configure the NVIDIA Streamline plugins"),
			Settings
		);
		UStreamlineOverrideSettings* OverrideSettings = GetMutableDefault<UStreamlineOverrideSettings>();
		SettingsModule->RegisterSettings("Project", "Plugins", "StreamlineOverride",
			LOCTEXT("StreamlineOverrideSettingsName", "NVIDIA Streamline Overrides (Local)"),
			LOCTEXT("StreamlineOverrideSettingsDescription", "Configure the local settings for the NVIDIA Streamline plugins"),
			OverrideSettings);
	}
#endif

	UE_LOG(LogStreamline, Log, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
}

void FStreamlineCoreModule::ShutdownModule()
{
	auto CVarInitializePlugin = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streamline.InitializePlugin"));
	if (CVarInitializePlugin && !CVarInitializePlugin->GetBool())
	{
		return;
	}

	UE_LOG(LogStreamline, Log, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));

	{
		StreamlineViewExtension = nullptr;
	}

	if (GetPlatformStreamlineSupport() == EStreamlineSupport::Supported)
	{
		if (IsStreamlineDLSSGSupported())
		{
			UnregisterStreamlineDLSSGHooks();
		}

		UnregisterStreamlineReflexHooks();
	}

#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Streamline");
		SettingsModule->UnregisterSettings("Project", "Plugins", "StreamlineOverride");
	}
#endif

	UE_LOG(LogStreamline, Log, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
}

EStreamlineSupport FStreamlineCoreModule::QueryStreamlineSupport() const
{
	return GetPlatformStreamlineSupport();
}

Streamline::EStreamlineFeatureSupport FStreamlineCoreModule::QueryDLSSGSupport() const
{
	return QueryStreamlineDLSSGSupport();
}

Streamline::EStreamlineFeatureSupport FStreamlineCoreModule::QueryDeepDVCSupport() const
{
	return QueryStreamlineDeepDVCSupport();
}

Streamline::EStreamlineFeatureSupport FStreamlineCoreModule::QueryReflexSupport() const
{
	return QueryStreamlineReflexSupport();
}

FStreamlineRHI* FStreamlineCoreModule::GetStreamlineRHI()
{
	return ::GetPlatformStreamlineRHI();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStreamlineCoreModule, StreamlineCore)

