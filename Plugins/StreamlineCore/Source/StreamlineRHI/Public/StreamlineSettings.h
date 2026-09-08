

#pragma once

#include "Engine/DeveloperSettings.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#define UE_API STREAMLINERHI_API

#include "StreamlineSettings.generated.h"

UENUM()
enum class EStreamlineSettingOverride : uint8
{
	Enabled UMETA(DisplayName = "True"),
	Disabled UMETA(DisplayName = "False"),
	UseProjectSettings UMETA(DisplayName = "Use project settings"),
};

UCLASS(MinimalAPI, Config = Engine, ProjectUserConfig)
class UStreamlineOverrideSettings : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(Config, EditAnywhere, Category = "General Settings (Local)", DisplayName = "Load Debug Overlay", meta = (ConfigRestartRequired = true))
	EStreamlineSettingOverride LoadDebugOverlayOverride = EStreamlineSettingOverride::UseProjectSettings;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings (Local)", DisplayName = "Allow OTA update", meta = (ConfigRestartRequired = true))
	EStreamlineSettingOverride AllowOTAUpdateOverride = EStreamlineSettingOverride::UseProjectSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Editor (Local)", DisplayName = "Enable DLSS-FG in New Editor Window (PIE) mode")
	EStreamlineSettingOverride EnableDLSSFGInPlayInEditorViewportsOverride = EStreamlineSettingOverride::UseProjectSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Compatibility (Local)", DisplayName = "Use slSetTag (deprecated)", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	EStreamlineSettingOverride UseSlSetTagOverride = EStreamlineSettingOverride::UseProjectSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Compatibility (Local)", DisplayName = "Use Slate callbacks for Swapchain tracking (deprecated)", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	EStreamlineSettingOverride UseSlateCallbacksForSwapchainTrackingOverride = EStreamlineSettingOverride::UseProjectSettings;
};

UCLASS(MinimalAPI, Config = Engine, DefaultConfig)
class UStreamlineSettings: public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "Load Debug Overlay", meta = (ConfigRestartRequired = true))
	bool bLoadDebugOverlay = false;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "Allow OTA update", meta = (ConfigRestartRequired = true))
	bool bAllowOTAUpdate = true;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "NVIDIA NGX Application ID", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	int32 NVIDIANGXApplicationId = 0;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "Enable plugin features for the D3D12RHI", meta = (ConfigRestartRequired = true))
	bool bEnableStreamlineD3D12 = PLATFORM_WINDOWS;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "Enable plugin features for the D3D11RHI (Reflex only)", meta = (ConfigRestartRequired = true))
	bool bEnableStreamlineD3D11 = PLATFORM_WINDOWS;

	UPROPERTY(Config, EditAnywhere, Category = "Editor", DisplayName = "Enable DLSS-FG in New Editor Window (PIE) mode")
	bool bEnableDLSSFGInPlayInEditorViewports = true;

	UPROPERTY(Config, EditAnywhere, Category = "Compatibility", DisplayName = "Use slSetTag (deprecated)", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	bool bUseSlSetTag = false;

	UPROPERTY(Config,    EditAnywhere, Category = "Compatibility", DisplayName = "Use Slate callbacks for Swapchain tracking (deprecated)", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	bool bUseSlateCallbacksForSwapchainTracking = false;

	static TObjectPtr <UStreamlineSettings> CppDefaults()
	{

		TObjectPtr<UStreamlineSettings> Result = NewObject<UStreamlineSettings>();
		return Result;

	}
};

#undef UE_API
