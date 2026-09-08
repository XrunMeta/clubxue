

#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/DeveloperSettings.h"
#include "SceneView.h"

#include "DLSSSettings.generated.h"

#define UE_API DLSS_API

UENUM()
enum class EDLSSSettingOverride : uint8
{
	Enabled UMETA(DisplayName = "True"),
	Disabled UMETA(DisplayName = "False"),
	UseProjectSettings UMETA(DisplayName = "Use project settings"),
};

UENUM(BlueprintType)
enum class EDLSSPreset : uint8
{
	Default=0 UMETA(ToolTip = "default behavior, preset specified per DLSS SDK release"),
	A=1  UMETA(Hidden, ToolTip = "Deprecated, use preset J or K"),
	B=2  UMETA(Hidden, ToolTip = "Deprecated, use preset J or K"),
	C=3  UMETA(Hidden, ToolTip = "Deprecated, use preset J or K"),
	D=4  UMETA(Hidden, ToolTip = "Deprecated, use preset J or K"),
	E=5  UMETA(Hidden, ToolTip = "Deprecated, use preset J or K"),
	F=6  UMETA(ToolTip = "Force preset F. Legacy preset for UltraPerformance/DLAA. Recommended not to use", DisplayName = "Preset F"),
	G=7  UMETA(ToolTip = "Force preset G, Do not use – reverts to default behavior", Hidden),
	H=8  UMETA(ToolTip = "Force preset H, Do not use – reverts to default behavior", Hidden),
	I=9  UMETA(ToolTip = "Force preset I, Do not use – reverts to default behavior", Hidden),
	J=10 UMETA(ToolTip = "Force preset J, Similar to preset K. Preset J might exhibit slightly less ghosting at the cost of extra flickering. Preset K is generally recommended over preset J", DisplayName = "Preset J"),
	K=11 UMETA(ToolTip = "Force preset K, Default preset for DLAA/Balanced/Quality modes. Less expensive performance wise compared to Preset L.", DisplayName= "Preset K"),
	L=12 UMETA(ToolTip = "Force preset L, Default preset for UltraPerformance mode. Delivers a sharper, more stable image with less ghosting than Preset J, K but are more expensive performance wise. Preset L is peak performant on RTX 40 series GPUs and above.", DisplayName = "Preset L"),
	M=13 UMETA(ToolTip = "Force preset M, Default preset for Performance mode. Delivers similar image quality improvements as Preset L but closer in speed to Presets J, K.Preset M is peak performant on RTX 40 series GPUs and above.", DisplayName = "Preset M"),
	N=14 UMETA(ToolTip = "Force preset N, Do not use - Reverts to default behavior", Hidden),
	O=15 UMETA(ToolTip = "Force preset O, Do not use - Reverts to default behavior", Hidden),

	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDLSSRRPreset : uint8
{
	Default=0 UMETA(ToolTip="Default behavior, may or may not change after OTA"),
	A =  1 UMETA(ToolTip = "Force preset A",Hidden),
	B =  2 UMETA(ToolTip = "Force preset B",Hidden),
	C =  3 UMETA(ToolTip = "Force preset C",Hidden),
	D =  4 UMETA(ToolTip = "Force preset D, Default model (transformer)",DisplayName = "Preset D"),
	E =  5 UMETA(ToolTip = "Force preset E, Latest transformer model (must use if DoF guide is needed)",DisplayName = "Preset E"),
	F =  6 UMETA(ToolTip = "Force preset F, Do not use – reverts to default behavior", Hidden),
	G =  7 UMETA(ToolTip = "Force preset G, Do not use – reverts to default behavior", Hidden),
	H =  8 UMETA(ToolTip = "Force preset H, Do not use – reverts to default behavior", Hidden),
	I =  9 UMETA(ToolTip = "Force preset I, Do not use – reverts to default behavior", Hidden),
	J = 10 UMETA(ToolTip = "Force preset J, Do not use – reverts to default behavior", Hidden),
	K = 11 UMETA(ToolTip = "Force preset K, Do not use – reverts to default behavior", Hidden),
	L = 12 UMETA(ToolTip = "Force preset L, Do not use – reverts to default behavior", Hidden),
	M = 13 UMETA(ToolTip = "Force preset M, Do not use – reverts to default behavior", Hidden),
	N = 14 UMETA(ToolTip = "Force preset N, Do not use – reverts to default behavior", Hidden),
	O = 15 UMETA(ToolTip = "Force preset O, Do not use – reverts to default behavior", Hidden),

	MAX UMETA(Hidden)
};

UCLASS(MinimalAPI,Config = Engine, ProjectUserConfig)
class UDLSSOverrideSettings : public UObject
{
public:

	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, Category = "Editor (Local)", DisplayName = "Warn about incompatible plugins and tools")
	bool bShowDLSSIncompatiblePluginsToolsWarnings = true;

	UPROPERTY(Config, EditAnywhere, Category = "Editor (Local)", DisplayName = "Show various DLSS/DLAA on screen debug messages")
	EDLSSSettingOverride ShowDLSSSDebugOnScreenMessages = EDLSSSettingOverride::UseProjectSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Editor (Local)", DisplayName = "Enable DLSS/DLAA to be turned on in Editor viewports")
	EDLSSSettingOverride EnableDLSSInEditorViewportsOverride = EDLSSSettingOverride::UseProjectSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Editor (Local)", DisplayName = "Enable DLSS/DLAA in Play In Editor viewports")
	EDLSSSettingOverride EnableDLSSInPlayInEditorViewportsOverride = EDLSSSettingOverride::UseProjectSettings;
};

UCLASS(MinimalAPI, Config = Engine, DefaultConfig, DisplayName="NVIDIA DLSS")
class UDLSSSettings: public UObject
{
	GENERATED_BODY()

private:

public:

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "Allow OTA update", meta = (ConfigRestartRequired = true))
	bool bAllowOTAUpdate = true;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "NVIDIA NGX Application ID", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	uint32 NVIDIANGXApplicationId;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "Bias Current Color Custom Stencil Value", meta = (UIMin = 1))
	uint8 BiasCurrentColorStencilValue = 8;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "Enable DLSS/DLAA for the D3D12RHI", meta = (ConfigRestartRequired = true))
		bool bEnableDLSSD3D12 = PLATFORM_WINDOWS;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "Enable DLSS/DLAA for the D3D11RHI", meta = (ConfigRestartRequired = true))
		bool bEnableDLSSD3D11 = PLATFORM_WINDOWS;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", DisplayName = "Enable DLSS/DLAA for the VulkanRHI", meta = (ConfigRestartRequired = true))
		bool bEnableDLSSVulkan = PLATFORM_WINDOWS;

	UPROPERTY(Config, EditAnywhere, Category = "Editor", DisplayName = "Warn about incompatible plugins and tools")
	bool bShowDLSSIncompatiblePluginsToolsWarnings = true;

	UPROPERTY(Config, EditAnywhere, Category = "Editor", DisplayName = "Enable DLSS/DLAA to be turned on in Editor viewports")
		bool bEnableDLSSInEditorViewports = false;

	UPROPERTY(Config, EditAnywhere, Category = "Editor", DisplayName = "Enable DLSS/DLAA in Play In Editor viewports")
		bool bEnableDLSSInPlayInEditorViewports = true;

	UPROPERTY(Config, EditAnywhere, Category = "Editor", DisplayName = "Show various DLSS/DLAA on screen debug messages")
		bool bShowDLSSSDebugOnScreenMessages = true;

	UPROPERTY(VisibleAnywhere, Config, Category = "DLSS-SR Settings", DisplayName = "Generic DLSS-SR Binary Path")
	FString GenericDLSSSRBinaryPath;

	UPROPERTY(VisibleAnywhere, Config, Category = "DLSS-SR Settings", DisplayName = "Exists")
	bool bGenericDLSSSRBinaryExists;

	UPROPERTY(VisibleAnywhere, Config, Category = "DLSS-SR Settings", DisplayName = "Custom DLSS-SR Binary Path", AdvancedDisplay)
	FString CustomDLSSSRBinaryPath;

	UPROPERTY(VisibleAnywhere, Config, Category = "DLSS-SR Settings", DisplayName = "Exists", AdvancedDisplay)
	bool bCustomDLSSSRBinaryExists;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-SR Settings", DisplayName = "DLAA Preset", AdvancedDisplay)
		EDLSSPreset DLAAPreset = EDLSSPreset::Default;

		EDLSSPreset DLSSUltraQualityPreset = EDLSSPreset::Default;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-SR Settings", DisplayName = "DLSS Quality Preset", AdvancedDisplay)
		EDLSSPreset DLSSQualityPreset = EDLSSPreset::Default;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-SR Settings", DisplayName = "DLSS Balanced Preset", AdvancedDisplay)
		EDLSSPreset DLSSBalancedPreset = EDLSSPreset::Default;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-SR Settings", DisplayName = "DLSS Performance Preset", AdvancedDisplay)
		EDLSSPreset DLSSPerformancePreset = EDLSSPreset::Default;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-SR Settings", DisplayName = "DLSS Ultra Performance Preset", AdvancedDisplay)
		EDLSSPreset DLSSUltraPerformancePreset = EDLSSPreset::Default;

	UPROPERTY(VisibleAnywhere, Config, Category = "DLSS-RR Settings", DisplayName = "Generic DLSS-RR Binary Path")
	FString GenericDLSSRRBinaryPath;

	UPROPERTY(VisibleAnywhere, Config, Category = "DLSS-RR Settings", DisplayName = "Exists")
	bool bGenericDLSSRRBinaryExists;

	UPROPERTY(VisibleAnywhere, Config, Category = "DLSS-RR Settings", DisplayName = "Custom DLSS-RR Binary Path", AdvancedDisplay)
	FString CustomDLSSRRBinaryPath;

	UPROPERTY(VisibleAnywhere, Config, Category = "DLSS-RR Settings", DisplayName = "Exists", AdvancedDisplay)
	bool bCustomDLSSRRBinaryExists;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-RR Settings", DisplayName = "DLAA-RR Preset", AdvancedDisplay)
		EDLSSRRPreset DLAARRPreset = EDLSSRRPreset::Default;

		EDLSSRRPreset DLSSRRUltraQualityPreset = EDLSSRRPreset::Default;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-RR Settings", DisplayName = "DLSS-RR Quality Preset", AdvancedDisplay)
		EDLSSRRPreset DLSSRRQualityPreset = EDLSSRRPreset::Default;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-RR Settings", DisplayName = "DLSS-RR Balanced Preset", AdvancedDisplay)
		EDLSSRRPreset DLSSRRBalancedPreset = EDLSSRRPreset::Default;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-RR Settings", DisplayName = "DLSS-RR Performance Preset", AdvancedDisplay)
		EDLSSRRPreset DLSSRRPerformancePreset = EDLSSRRPreset::Default;

	UPROPERTY(Config, EditAnywhere, Category = "DLSS-RR Settings", DisplayName = "DLSS-RR Ultra Performance Preset", AdvancedDisplay)
		EDLSSRRPreset DLSSRRUltraPerformancePreset = EDLSSRRPreset::Default;

public:

	virtual void PostInitProperties();

};

#undef UE_API
