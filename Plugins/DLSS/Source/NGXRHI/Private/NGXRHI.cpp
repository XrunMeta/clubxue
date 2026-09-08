

#include "NGXRHI.h"

#include "Misc/EngineVersionComparison.h"
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Interfaces/IPluginManager.h"

#include "nvsdk_ngx.h"
#include "nvsdk_ngx_params.h"
#include "nvsdk_ngx_helpers.h"

#ifndef NGX_PLATFORM_DIR
#error "You are not supposed to get to this point, check your build configuration."
#endif

static const FString PlatformDir = NGX_PLATFORM_DIR;

DEFINE_LOG_CATEGORY_STATIC(LogDLSSNGX, Log, All);

DEFINE_LOG_CATEGORY_STATIC(LogDLSSNGXRHI, Log, All);

DECLARE_STATS_GROUP(TEXT("DLSS"), STATGROUP_DLSS, STATCAT_Advanced);
DECLARE_MEMORY_STAT_POOL(TEXT("DLSS: Video memory"), STAT_DLSSInternalGPUMemory, STATGROUP_DLSS, FPlatformMemory::MCR_GPU);
DECLARE_DWORD_COUNTER_STAT(TEXT("DLSS: Num DLSS features"), STAT_DLSSNumFeatures, STATGROUP_DLSS);

#define LOCTEXT_NAMESPACE "NGXRHI"

static TAutoConsoleVariable<int32> CVarNGXLogLevel(
	TEXT("r.NGX.LogLevel"), 1,
	TEXT("Determines the minimal amount of logging the NGX implementation pipes into LogDLSSNGX. Can be overridden by the -NGXLogLevel= command line option\n")
	TEXT("Please refer to the DLSS plugin documentation on other ways to change the logging level.\n")
	TEXT("0: off \n")
	TEXT("1: on (default)\n")
	TEXT("2: verbose "),
	ECVF_ReadOnly);

static TAutoConsoleVariable<int32> CVarNGXFramesUntilFeatureDestruction(
	TEXT("r.NGX.FramesUntilFeatureDestruction"), 3,
	TEXT("Number of frames until an unused NGX feature gets destroyed. (default=3)"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarNGXRenameLogSeverities(
	TEXT("r.NGX.RenameNGXLogSeverities"), 1,
	TEXT("Renames 'error' and 'warning' in messages returned by the NGX log callback to 'e_rror' and 'w_arning' before passing them to the UE log system\n")
	TEXT("0: off \n")
	TEXT("1: on, for select messages during initalization (default)\n")
	TEXT("2: on, for all messages\n"),
	ECVF_Default);

void FRHIDLSSArguments::Validate() const
{

	auto ValidateExtents = [](const FIntRect& InRect, const FRHITexture* InTexture) 
	{
		if (InTexture)
		{
			const FString TextureName = InTexture->GetName().ToString();

			checkf(InRect.Max.X <= InTexture->GetDesc().Extent.X, TEXT("Texture rect validation failed: Rect.Max.X (%d) exceeds texture width (%d) for texture '%s'"),
				InRect.Max.X, InTexture->GetDesc().Extent.X, *TextureName);

			checkf(InRect.Max.Y <= InTexture->GetDesc().Extent.Y, TEXT("Texture rect validation failed: Rect.Max.Y (%d) exceeds texture height (%d) for texture '%s'"),
				InRect.Max.Y, InTexture->GetDesc().Extent.Y, *TextureName);
		}
	};

	check(OutputColor);
	ValidateExtents(DestRect, OutputColor);

	const FIntRect LowResWithOffset = SrcRect;

	const FIntRect LowResTopLeft = FIntRect(0, 0, SrcRect.Width(), SrcRect.Height());

	check(InputColor);
	ValidateExtents(LowResWithOffset, InputColor);

	check(InputDepth);
	static_assert (int(ENGXDLSSDenoiserMode::MaxValue) == 1, "dear DLSS plugin NVIDIA developer, please update this code to handle the new ENGXDLSSDenoiserMode enum values");
	ValidateExtents(DenoiserMode == ENGXDLSSDenoiserMode::DLSSRR ? LowResTopLeft : LowResWithOffset, InputDepth);

	check(InputMotionVectors);
	ValidateExtents(LowResTopLeft, InputMotionVectors);

	check(InputExposure);

	ValidateExtents(LowResWithOffset, InputBiasCurrentColorMask);

	ValidateExtents(LowResTopLeft, InputDiffuseAlbedo);
	ValidateExtents(LowResTopLeft, InputSpecularAlbedo);
	ValidateExtents(LowResTopLeft, InputNormals);
	ValidateExtents(LowResTopLeft, InputRoughness);

#if SUPPORT_GUIDE_GBUFFER
	ValidateExtents(LowResTopLeft, InputReflectionHitDistance);
#endif
}

NGXDLSSFeature::~NGXDLSSFeature()
{
	check(!IsRunningRHIInSeparateThread() || IsInRHIThread());
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Destroying NGX DLSS Feature %s "), *Desc.GetDebugDescription());
}

void NVSDK_CONV NGXLogSink(const char* InNGXMessage, NVSDK_NGX_Logging_Level InLoggingLevel, NVSDK_NGX_Feature InSourceComponent)
{
#if !NO_LOGGING
	FString Message(FString(UTF8_TO_TCHAR(InNGXMessage)).TrimEnd());
	FString NGXComponent = FString::Printf(TEXT("Unknown(%d)"),InSourceComponent) ;

	switch (InSourceComponent)
	{
		case NVSDK_NGX_Feature_SuperSampling    : NGXComponent = TEXT("DLSS-SR"); break;
		case NVSDK_NGX_Feature_RayReconstruction: NGXComponent = TEXT("DLSS-RR"); break;
		case NVSDK_NGX_Feature_FrameGeneration:   NGXComponent = TEXT("DLSS-FG"); break;
		case NVSDK_NGX_Feature_DeepDVC:           NGXComponent = TEXT("DeepDVC"); break;
		case NVSDK_NGX_Feature_Reserved_SDK     : NGXComponent = TEXT("SDK"); break;
		case NVSDK_NGX_Feature_Reserved_Core    : NGXComponent = TEXT("Core"); break;
	}

	const bool bIsOurNGXBinary = Message.Contains(TEXT("nvngx_dlss.dll")) || Message.Contains(TEXT("nvngx_dlssd.dll"));
	const bool bIsVerboseStartupMessage =

		(Message.Contains(TEXT("doesn't exist in any of the search paths")) && !bIsOurNGXBinary)
		|| Message.Contains(TEXT("warning: UWP compliant mode enabled"))
		|| Message.Contains(TEXT("error: failed to load NGXCore"))
		;

	if ((CVarNGXRenameLogSeverities.GetValueOnAnyThread() == 2) || ((CVarNGXRenameLogSeverities.GetValueOnAnyThread() == 1) && bIsVerboseStartupMessage))
	{
		Message = Message.Replace(TEXT("error:"), TEXT("e_rror:"));
		Message = Message.Replace(TEXT("Warning:"), TEXT("w_arning:"));
		UE_LOG(LogDLSSNGX, Verbose, TEXT("[%s]: %s"), *NGXComponent, *Message);
	}
	else
	{

		FModuleStatus Status;
		if (FModuleManager::Get().QueryModule(UE_MODULE_NAME, Status) && Status.bIsLoaded)
		{
			UE_LOG(LogDLSSNGX, Log, TEXT("[%s]: %s"), *NGXComponent, *Message);
		}
	}
#endif
}

bool NGXRHI::bNGXInitialized = false;

bool NGXRHI::bIsIncompatibleAPICaptureToolActive = false;

static void RemoveDuplicateSlashesFromPath(FString& Path)
{
	if (Path.StartsWith(FString("//")))
	{

		FString NewString = FString("/");
		NewString += FPaths::RemoveDuplicateSlashes(Path.RightChop(1));
		Path = NewString;
	}
	else
	{
		FPaths::RemoveDuplicateSlashes(Path);
	}
}

NGXRHI::NGXRHI(const FNGXRHICreateArguments& Arguments)
	: DynamicRHI(Arguments.DynamicRHI)
{
	FString PluginNGXProductionBinariesDir = FPaths::Combine(Arguments.PluginBaseDir, TEXT("Binaries/ThirdParty"), PlatformDir);
	FString PluginNGXDevelopmentBinariesDir = FPaths::Combine(Arguments.PluginBaseDir, TEXT("Binaries/ThirdParty"), PlatformDir, TEXT("/Development/"));
	FString PluginNGXBinariesDir = PluginNGXProductionBinariesDir;

	FString ProjectNGXBinariesDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/ThirdParty/NVIDIA/NGX"), PlatformDir);
	FString LaunchNGXBinariesDir = FPaths::Combine(FPaths::LaunchDir(), TEXT("Binaries/ThirdParty/NVIDIA/NGX"), PlatformDir);

	switch (Arguments.NGXBinariesSearchOrder)
	{
		default:
		case ENGXBinariesSearchOrder::CustomThenGeneric:
		{
			UE_LOG(LogDLSSNGXRHI, Log, TEXT("Searching for custom and generic DLSS binaries"));
			NGXDLLSearchPaths.Append({ ProjectNGXBinariesDir, LaunchNGXBinariesDir, PluginNGXProductionBinariesDir });
			break;
		}
 		case  ENGXBinariesSearchOrder::ForceGeneric:
		{
			UE_LOG(LogDLSSNGXRHI, Log, TEXT("Searching only for generic binaries from the plugin folder"));
			NGXDLLSearchPaths.Append({ PluginNGXProductionBinariesDir });
			break;
		}
		case ENGXBinariesSearchOrder::ForceCustom:
		{
			UE_LOG(LogDLSSNGXRHI, Log, TEXT("Searching only for custom DLSS binaries from the DLSS plugin"));
			NGXDLLSearchPaths.Append({ ProjectNGXBinariesDir, LaunchNGXBinariesDir });
			break;
		}
		case ENGXBinariesSearchOrder::ForceDevelopmentGeneric:
		{
			UE_LOG(LogDLSSNGXRHI, Log, TEXT("Searching only for generic development DLSS binaries from the DLSS plugin. This binary is only packaged for non-shipping build configurations"));
			NGXDLLSearchPaths.Append({ PluginNGXDevelopmentBinariesDir });
			PluginNGXBinariesDir = PluginNGXDevelopmentBinariesDir;
			break;
		}
	}

	for (int32 i = 0; i < NGXDLLSearchPaths.Num(); ++i)
	{
		NGXDLLSearchPaths[i] = FPaths::ConvertRelativePathToFull(NGXDLLSearchPaths[i]);
		RemoveDuplicateSlashesFromPath(NGXDLLSearchPaths[i]);
		FPaths::MakePlatformFilename(NGXDLLSearchPaths[i]);

		NGXDLLSearchPathRawStrings.Add(*NGXDLLSearchPaths[i]);
		const bool bHasDLSSSRBinary = IPlatformFile::GetPlatformPhysical().FileExists(*FPaths::Combine(NGXDLLSearchPaths[i], NGX_DLSS_SR_BINARY_NAME));
		UE_LOG(LogDLSSNGXRHI, Log, TEXT("NVIDIA NGX DLSS-SR binary %s %s in search path %s"), NGX_DLSS_SR_BINARY_NAME, bHasDLSSSRBinary ? TEXT("found") : TEXT("not found"), *NGXDLLSearchPaths[i]);

		const bool bHasDLSSRRBinary = IPlatformFile::GetPlatformPhysical().FileExists(*FPaths::Combine(NGXDLLSearchPaths[i], NGX_DLSS_RR_BINARY_NAME));
		UE_LOG(LogDLSSNGXRHI, Log, TEXT("NVIDIA NGX DLSS-RR binary %s %s in search path %s"), NGX_DLSS_RR_BINARY_NAME, bHasDLSSRRBinary ? TEXT("found") : TEXT("not found"), *NGXDLLSearchPaths[i]);

	}

	DLSSSRGenericBinaryInfo.Get<0>() = FPaths::Combine(PluginNGXBinariesDir, NGX_DLSS_SR_BINARY_NAME);
	DLSSSRGenericBinaryInfo.Get<1>() = IPlatformFile::GetPlatformPhysical().FileExists(*DLSSSRGenericBinaryInfo.Get<0>());

	DLSSSRCustomBinaryInfo.Get<0>() = FPaths::Combine(ProjectNGXBinariesDir, NGX_DLSS_SR_BINARY_NAME);
	DLSSSRCustomBinaryInfo.Get<1>() = IPlatformFile::GetPlatformPhysical().FileExists(*DLSSSRCustomBinaryInfo.Get<0>());

	DLSSRRGenericBinaryInfo.Get<0>() = FPaths::Combine(PluginNGXBinariesDir, NGX_DLSS_RR_BINARY_NAME);
	DLSSRRGenericBinaryInfo.Get<1>() = IPlatformFile::GetPlatformPhysical().FileExists(*DLSSRRGenericBinaryInfo.Get<0>());

	DLSSRRCustomBinaryInfo.Get<0>() = FPaths::Combine(ProjectNGXBinariesDir, NGX_DLSS_RR_BINARY_NAME);
	DLSSRRCustomBinaryInfo.Get<1>() = IPlatformFile::GetPlatformPhysical().FileExists(*DLSSRRCustomBinaryInfo.Get<0>());

	FeatureInfo.PathListInfo.Path = const_cast<wchar_t**>(NGXDLLSearchPathRawStrings.GetData());
	FeatureInfo.PathListInfo.Length = NGXDLLSearchPathRawStrings.Num();

	{
		FeatureInfo.LoggingInfo.DisableOtherLoggingSinks = 1; 
		FeatureInfo.LoggingInfo.LoggingCallback = &NGXLogSink;

		switch (CVarNGXLogLevel.GetValueOnAnyThread())
		{
			case 0:
				FeatureInfo.LoggingInfo.MinimumLoggingLevel = NVSDK_NGX_LOGGING_LEVEL_OFF;
				break;

			default:
			case 1:
				FeatureInfo.LoggingInfo.MinimumLoggingLevel = NVSDK_NGX_LOGGING_LEVEL_ON;
				break;
			case 2:
				FeatureInfo.LoggingInfo.MinimumLoggingLevel = NVSDK_NGX_LOGGING_LEVEL_VERBOSE;
				break;
		}
	}

	if (Arguments.bAllowOTAUpdate)
	{
		UE_LOG(LogDLSSNGXRHI, Log, TEXT("DLSS model OTA update enabled"));
		if (Arguments.InitializeNGXWithNGXApplicationID())
		{
			NVSDK_NGX_Application_Identifier NGXAppIdentifier;
			NGXAppIdentifier.IdentifierType = NVSDK_NGX_Application_Identifier_Type::NVSDK_NGX_Application_Identifier_Type_Application_Id;
			NGXAppIdentifier.v.ApplicationId = Arguments.NGXAppId;
			NVSDK_NGX_UpdateFeature(&NGXAppIdentifier, NVSDK_NGX_Feature::NVSDK_NGX_Feature_SuperSampling);
		}
		else
		{
			FTCHARToUTF8 ProjectIdUTF8(*Arguments.UnrealProjectID);
			FTCHARToUTF8 EngineVersionUTF8(*Arguments.UnrealEngineVersion);
			NVSDK_NGX_Application_Identifier NGXAppIdentifier;
			NGXAppIdentifier.IdentifierType = NVSDK_NGX_Application_Identifier_Type::NVSDK_NGX_Application_Identifier_Type_Project_Id;
			NGXAppIdentifier.v.ProjectDesc.ProjectId = ProjectIdUTF8.Get();
			NGXAppIdentifier.v.ProjectDesc.EngineType = NVSDK_NGX_EngineType::NVSDK_NGX_ENGINE_TYPE_UNREAL;
			NGXAppIdentifier.v.ProjectDesc.EngineVersion = EngineVersionUTF8.Get();
			NVSDK_NGX_UpdateFeature(&NGXAppIdentifier, NVSDK_NGX_Feature::NVSDK_NGX_Feature_SuperSampling);
		}
	}
	else
	{
		UE_LOG(LogDLSSNGXRHI, Log, TEXT("DLSS model OTA update disabled"));
	}
}

TTuple<FString, bool> NGXRHI::GetDLSSSRGenericBinaryInfo() const
{
	return DLSSSRGenericBinaryInfo;
}

TTuple<FString, bool> NGXRHI::GetDLSSSRCustomBinaryInfo() const
{
	return DLSSSRCustomBinaryInfo;
}

TTuple<FString, bool> NGXRHI::GetDLSSRRGenericBinaryInfo() const
{
	return DLSSRRGenericBinaryInfo;
}

TTuple<FString, bool> NGXRHI::GetDLSSRRCustomBinaryInfo() const
{
	return DLSSRRCustomBinaryInfo;
}

#if !ENGINE_PROVIDES_UE_5_6_ID3D12DYNAMICRHI_METHODS
bool NGXRHI::NeedExtraPassesForDebugLayerCompatibility()
{
	return false;
}
#endif

NGXRHI::~NGXRHI()
{
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
}

void NGXRHI::FDLSSQueryFeature::QueryDLSSSupport()
{
	int32 bNeedsUpdatedDriverSR = 1;
	int32 MinDriverVersionMajorSR = 0;
	int32 MinDriverVersionMinorSR = 0;

	int32 bNeedsUpdatedDriverDenoise = 1;
	int32 MinDriverVersionMajorDenoise = 0;
	int32 MinDriverVersionMinorDenoise = 0;

	if (!CapabilityParameters)
	{
		UE_LOG(LogDLSSNGXRHI, Log, TEXT("NVIDIA NGX DLSS cannot be loaded possibly due to issues initializing NGX."));
		NGXInitResult = NVSDK_NGX_Result_Fail;
		bIsDlssSRAvailable = false;
		bIsDlssRRAvailable = false;
		return;
	}

	check(CapabilityParameters);

	NVSDK_NGX_Result ResultUpdatedDriver = CapabilityParameters->Get(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver, &bNeedsUpdatedDriverSR);
	NVSDK_NGX_Result ResultMinDriverVersionMajor = CapabilityParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor, &MinDriverVersionMajorSR);
	NVSDK_NGX_Result ResultMinDriverVersionMinor = CapabilityParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor, &MinDriverVersionMinorSR);

	NVSDK_NGX_Result ResultUpdatedDriverDenoise = CapabilityParameters->Get(NVSDK_NGX_Parameter_SuperSamplingDenoising_NeedsUpdatedDriver, &bNeedsUpdatedDriverDenoise);
	NVSDK_NGX_Result ResultMinDriverVersionMajorDenoise = CapabilityParameters->Get(NVSDK_NGX_Parameter_SuperSamplingDenoising_MinDriverVersionMajor, &MinDriverVersionMajorDenoise);
	NVSDK_NGX_Result ResultMinDriverVersionMinorDenoise = CapabilityParameters->Get(NVSDK_NGX_Parameter_SuperSamplingDenoising_MinDriverVersionMinor, &MinDriverVersionMinorDenoise);

	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver -> (%u %s), bNeedsUpdatedDriver = %d"), ResultUpdatedDriver, GetNGXResultAsString(ResultUpdatedDriver), bNeedsUpdatedDriverSR);
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor -> (%u %s), MinDriverVersionMajor = %d"), ResultMinDriverVersionMajor, GetNGXResultAsString(ResultMinDriverVersionMajor), MinDriverVersionMajorSR);
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor -> (%u %s), MinDriverVersionMinor = %d"), ResultMinDriverVersionMinor, GetNGXResultAsString(ResultMinDriverVersionMinor), MinDriverVersionMinorSR);

	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_Parameter_SuperSamplingDenoising_NeedsUpdatedDriver -> (%u %s), bNeedsUpdatedDriver = %d"), ResultUpdatedDriverDenoise, GetNGXResultAsString(ResultUpdatedDriverDenoise), bNeedsUpdatedDriverDenoise);
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_Parameter_SuperSamplingDenoising_MinDriverVersionMajor -> (%u %s), MinDriverVersionMajor = %d"), ResultMinDriverVersionMajorDenoise, GetNGXResultAsString(ResultMinDriverVersionMajorDenoise), MinDriverVersionMajorDenoise);
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_Parameter_SuperSamplingDenoising_MinDriverVersionMinor -> (%u %s), MinDriverVersionMinor = %d"), ResultMinDriverVersionMinorDenoise, GetNGXResultAsString(ResultMinDriverVersionMinorDenoise), MinDriverVersionMinorDenoise);

	if (NVSDK_NGX_SUCCEED(ResultUpdatedDriver))
	{
		NGXDLSSSRDriverRequirements.DriverUpdateRequired = bNeedsUpdatedDriverSR != 0;

		if (NVSDK_NGX_SUCCEED(ResultMinDriverVersionMajor) && NVSDK_NGX_SUCCEED(ResultMinDriverVersionMinor) && MinDriverVersionMajorSR != 0)
		{
			NGXDLSSSRDriverRequirements.MinDriverVersionMajor = MinDriverVersionMajorSR;
			NGXDLSSSRDriverRequirements.MinDriverVersionMinor = MinDriverVersionMinorSR;
		}

		if (bNeedsUpdatedDriverSR)
		{
			UE_LOG(LogDLSSNGXRHI, Log, TEXT("NVIDIA NGX DLSS cannot be loaded due to an outdated driver. Minimum Driver Version required : %u.%u"), MinDriverVersionMajorSR, MinDriverVersionMinorSR);
		}
		else
		{
			UE_LOG(LogDLSSNGXRHI, Log, TEXT("NVIDIA NGX DLSS is supported by the currently installed driver. Minimum driver version was reported as: %u.%u"), MinDriverVersionMajorSR, MinDriverVersionMinorSR);
		}
	}
	else
	{
		UE_LOG(LogDLSSNGXRHI, Log, TEXT("NVIDIA NGX DLSS Minimum driver version was not reported"));
	}

	NGXDLSSRRDriverRequirements.DriverUpdateRequired = true;
	NGXDLSSRRDriverRequirements.MinDriverVersionMajor = 537;
	NGXDLSSRRDriverRequirements.MinDriverVersionMinor = 2;
	if (NVSDK_NGX_SUCCEED(ResultUpdatedDriverDenoise))
	{
		NGXDLSSRRDriverRequirements.DriverUpdateRequired = bNeedsUpdatedDriverDenoise != 0;

		if (NVSDK_NGX_SUCCEED(ResultMinDriverVersionMajorDenoise) && NVSDK_NGX_SUCCEED(ResultMinDriverVersionMinorDenoise) && MinDriverVersionMajorDenoise != 0)
		{
			NGXDLSSRRDriverRequirements.MinDriverVersionMajor = MinDriverVersionMajorDenoise;
			NGXDLSSRRDriverRequirements.MinDriverVersionMinor = MinDriverVersionMinorDenoise;
		}

		if (bNeedsUpdatedDriverDenoise)
		{
			UE_LOG(LogDLSSNGXRHI, Log, TEXT("NVIDIA NGX DLSS-RR cannot be loaded due to an outdated driver. Minimum Driver Version required : %u.%u"), MinDriverVersionMajorDenoise, MinDriverVersionMinorDenoise);
		}
		else
		{
			UE_LOG(LogDLSSNGXRHI, Log, TEXT("NVIDIA NGX DLSS-RR is supported by the currently installed driver. Minimum driver version was reported as: %u.%u"), MinDriverVersionMajorDenoise, MinDriverVersionMinorDenoise);
		}
	}
	else
	{
		UE_LOG(LogDLSSNGXRHI, Log, TEXT("NVIDIA NGX DLSS-RR Minimum driver version was not reported, driver likely does not support DLSS-RR"));
	}

	int DlssSRAvailable = 0;
	NVSDK_NGX_Result ResultAvailable = CapabilityParameters->Get(NVSDK_NGX_EParameter_SuperSampling_Available, &DlssSRAvailable);
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_EParameter_SuperSampling_Available -> (%u %s), DlssAvailable = %d"), ResultAvailable, GetNGXResultAsString(ResultAvailable), DlssSRAvailable);
	if (NVSDK_NGX_SUCCEED(ResultAvailable) && DlssSRAvailable)
	{
		bIsDlssSRAvailable = true;

		NGXDLSSSRInitResult = ResultAvailable;
	}

	int DlssRRAvailable = 0;
	ResultAvailable = CapabilityParameters->Get(NVSDK_NGX_Parameter_SuperSamplingDenoising_Available, &DlssRRAvailable);
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_Parameter_SuperSamplingDenoising_Available -> (%u %s), DlssRRAvailable = %d"), ResultAvailable, GetNGXResultAsString(ResultAvailable), DlssRRAvailable);
	if (NVSDK_NGX_SUCCEED(ResultAvailable) && DlssRRAvailable)
	{

		bIsDlssRRAvailable = bIsDlssSRAvailable;

		NGXDLSSRRInitResult = ResultAvailable;
	}

	if (!bIsDlssSRAvailable)
	{

		NVSDK_NGX_Result DlssFeatureInitResult = NVSDK_NGX_Result_Fail;
		NVSDK_NGX_Result ResultDlssFeatureInitResult = CapabilityParameters->Get(NVSDK_NGX_Parameter_SuperSampling_FeatureInitResult, (int*)&DlssFeatureInitResult);
		UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_Parameter_SuperSampling_FeatureInitResult -> (%u %s), NVSDK_NGX_Parameter_SuperSampling_FeatureInitResult = (%u %s)"), ResultDlssFeatureInitResult, GetNGXResultAsString(ResultDlssFeatureInitResult), DlssFeatureInitResult, GetNGXResultAsString(DlssFeatureInitResult));

		NGXDLSSSRInitResult = NVSDK_NGX_SUCCEED(ResultDlssFeatureInitResult) ? DlssFeatureInitResult : NVSDK_NGX_Result_Fail;
	}
	if (!bIsDlssRRAvailable)
	{
		NVSDK_NGX_Result DlssRRFeatureInitResult = NVSDK_NGX_Result_Fail;
		NVSDK_NGX_Result ResultDlssRRFeatureInitResult = CapabilityParameters->Get(NVSDK_NGX_Parameter_SuperSamplingDenoising_FeatureInitResult, (int*)&DlssRRFeatureInitResult);
		UE_LOG(LogDLSSNGXRHI, Log, TEXT("Get NVSDK_NGX_Parameter_SuperSamplingDenoising_FeatureInitResult -> (%u %s), NVSDK_NGX_Parameter_SuperSamplingDenoising_FeatureInitResult = (%u %s)"), ResultDlssRRFeatureInitResult, GetNGXResultAsString(ResultDlssRRFeatureInitResult), DlssRRFeatureInitResult, GetNGXResultAsString(DlssRRFeatureInitResult));

		NGXDLSSRRInitResult = NVSDK_NGX_SUCCEED(ResultDlssRRFeatureInitResult) ? DlssRRFeatureInitResult : NVSDK_NGX_Result_Fail;
	}
}

FDLSSOptimalSettings NGXRHI::FDLSSQueryFeature::GetDLSSOptimalSettings(const FDLSSResolutionParameters& InResolution) const
{
	check(CapabilityParameters);

	FDLSSOptimalSettings OptimalSettings;

	float SharpnessIsDeprecatedButWeStillNeedAFunctionArgument;

	const NVSDK_NGX_Result ResultGetOptimalSettings = NGX_DLSS_GET_OPTIMAL_SETTINGS(
		CapabilityParameters,
		InResolution.Width,
		InResolution.Height,
		InResolution.PerfQuality,
		reinterpret_cast<unsigned int*>(&OptimalSettings.RenderSize.X),
		reinterpret_cast<unsigned int*>(&OptimalSettings.RenderSize.Y),
		reinterpret_cast<unsigned int*>(&OptimalSettings.RenderSizeMax.X),
		reinterpret_cast<unsigned int*>(&OptimalSettings.RenderSizeMax.Y),
		reinterpret_cast<unsigned int*>(&OptimalSettings.RenderSizeMin.X),
		reinterpret_cast<unsigned int*>(&OptimalSettings.RenderSizeMin.Y),
		&SharpnessIsDeprecatedButWeStillNeedAFunctionArgument
		);
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("NGX_DLSS_GET_OPTIMAL_SETTINGS -> (%u %s)"), ResultGetOptimalSettings, GetNGXResultAsString(ResultGetOptimalSettings));
	checkf(NVSDK_NGX_SUCCEED(ResultGetOptimalSettings), TEXT("failed to query supported DLSS modes"));

	OptimalSettings.bIsSupported = (OptimalSettings.RenderSize.X > 0) && (OptimalSettings.RenderSize.Y > 0);
	auto ComputeResolutionFraction = [&InResolution](int32 RenderSizeX, int32 RenderSizeY)
	{
		float XScale = float(RenderSizeX) / float(InResolution.Width);
		float YScale = float(RenderSizeY) / float(InResolution.Height);
		return FMath::Min(XScale, YScale);
	};

	OptimalSettings.MinResolutionFraction = ComputeResolutionFraction(OptimalSettings.RenderSizeMin.X, OptimalSettings.RenderSizeMin.Y);
	OptimalSettings.MaxResolutionFraction = ComputeResolutionFraction(OptimalSettings.RenderSizeMax.X, OptimalSettings.RenderSizeMax.Y);

	OptimalSettings.OptimalResolutionFraction = FMath::Clamp<float>(ComputeResolutionFraction(OptimalSettings.RenderSize.X, OptimalSettings.RenderSize.Y), OptimalSettings.MinResolutionFraction, OptimalSettings.MaxResolutionFraction);

	return OptimalSettings;
}

FString NGXRHI::GetNGXLogDirectory()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir());
}

bool NGXRHI::IsSafeToShutdownNGX() const
{

	TSharedPtr<IPlugin> StreamlinePlugin = IPluginManager::Get().FindPlugin(TEXT("StreamlineCore"));

	if (StreamlinePlugin.IsValid())
	{	
		return !StreamlinePlugin->IsEnabled();
	}
	else
	{

		StreamlinePlugin = IPluginManager::Get().FindPlugin(TEXT("Streamline"));
		return !StreamlinePlugin.IsValid() || !StreamlinePlugin->IsEnabled();
	}
}

uint32 FRHIDLSSArguments::GetNGXCommonDLSSFeatureFlags() const
{
	check(!IsRunningRHIInSeparateThread() || IsInRHIThread());
	uint32 DLSSFeatureFlags = NVSDK_NGX_DLSS_Feature_Flags_None;
	DLSSFeatureFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;

	static_assert (int(ENGXDLSSDenoiserMode::MaxValue) == 1, "dear DLSS plugin NVIDIA developer, please update this code to handle the new ENGXDLSSDenoiserMode enum values");

	if (DenoiserMode == ENGXDLSSDenoiserMode::Off)
	{
#if !UE_VERSION_OLDER_THAN(5,8,0)
		DLSSFeatureFlags |= NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
#else
		DLSSFeatureFlags |= bool(ERHIZBuffer::IsInverted) ? NVSDK_NGX_DLSS_Feature_Flags_DepthInverted : 0;
#endif
	}
	DLSSFeatureFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
	DLSSFeatureFlags |= bUseAutoExposure ? NVSDK_NGX_DLSS_Feature_Flags_AutoExposure : 0;
	DLSSFeatureFlags |= bEnableAlphaUpscaling ? NVSDK_NGX_DLSS_Feature_Flags_AlphaUpscaling : 0;
	return DLSSFeatureFlags;
}

NVSDK_NGX_DLSS_Create_Params FRHIDLSSArguments::GetNGXDLSSCreateParams() const
{
	check(!IsRunningRHIInSeparateThread() || IsInRHIThread());
	NVSDK_NGX_DLSS_Create_Params Result;
	FMemory::Memzero(Result);
	Result.Feature.InWidth = SrcRect.Width();
	Result.Feature.InHeight = SrcRect.Height();
	Result.Feature.InTargetWidth = DestRect.Width();
	Result.Feature.InTargetHeight = DestRect.Height();
	Result.Feature.InPerfQualityValue = static_cast<NVSDK_NGX_PerfQuality_Value>(PerfQuality);
	check((Result.Feature.InPerfQualityValue >= NVSDK_NGX_PerfQuality_Value_MaxPerf) && (Result.Feature.InPerfQualityValue <= NVSDK_NGX_PerfQuality_Value_DLAA));

	Result.InFeatureCreateFlags = GetNGXCommonDLSSFeatureFlags();
	Result.InEnableOutputSubrects = OutputColor->GetTexture2D()->GetSizeXY() != DestRect.Size();
	return Result;
}

NVSDK_NGX_DLSSD_Create_Params FRHIDLSSArguments::GetNGXDLSSRRCreateParams() const
{
	check(!IsRunningRHIInSeparateThread() || IsInRHIThread());
	NVSDK_NGX_DLSSD_Create_Params Result;
	FMemory::Memzero(Result);
	Result.InWidth = SrcRect.Width();
	Result.InHeight = SrcRect.Height();
	Result.InTargetWidth = DestRect.Width();
	Result.InTargetHeight = DestRect.Height();
	Result.InPerfQualityValue = static_cast<NVSDK_NGX_PerfQuality_Value>(PerfQuality);
	check((Result.InPerfQualityValue >= NVSDK_NGX_PerfQuality_Value_MaxPerf) && (Result.InPerfQualityValue <= NVSDK_NGX_PerfQuality_Value_DLAA));
	Result.InFeatureCreateFlags = GetNGXCommonDLSSFeatureFlags();
	Result.InEnableOutputSubrects = OutputColor->GetTexture2D()->GetSizeXY() != DestRect.Size();

	Result.InDenoiseMode = NVSDK_NGX_DLSS_Denoise_Mode_DLUnified;

	return Result;
}

bool FDLSSState::RequiresFeatureRecreation(const FRHIDLSSArguments& InArguments)
{
	check(!IsRunningRHIInSeparateThread() || IsInRHIThread());
	float NewUpscaleRatio = static_cast<float>(InArguments.SrcRect.Width()) / static_cast<float>(InArguments.DestRect.Width());
	if (!DLSSFeature || DLSSFeature->Desc != InArguments.GetFeatureDesc())
	{
		return true;
	}

	return false;
}

void NGXRHI::RegisterFeature(TSharedPtr<NGXDLSSFeature> InFeature)
{ 
	check(!IsRunningRHIInSeparateThread() || IsInRHIThread());
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("Creating   NGX DLSS Feature  %s "), *InFeature->Desc.GetDebugDescription());
	AllocatedDLSSFeatures.Add(InFeature);
}

TSharedPtr<NGXDLSSFeature> NGXRHI::FindFreeFeature(const FRHIDLSSArguments& InArguments)
{
	check(!IsRunningRHIInSeparateThread() || IsInRHIThread());
	TSharedPtr<NGXDLSSFeature> OutFeature;
	for (int FeatureIndex = 0; FeatureIndex < AllocatedDLSSFeatures.Num(); ++FeatureIndex)
	{

		if (AllocatedDLSSFeatures[FeatureIndex].GetSharedReferenceCount() > 1)
		{
			continue;
		}

		if (AllocatedDLSSFeatures[FeatureIndex]->Desc == InArguments.GetFeatureDesc())
		{
			OutFeature = AllocatedDLSSFeatures[FeatureIndex];
			OutFeature->LastUsedFrame = FrameCounter;
			break;

		}
	}
	return OutFeature;
}

void NGXRHI::ReleaseAllocatedFeatures()
{
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));

	for (int FeatureIndex = 0; FeatureIndex < AllocatedDLSSFeatures.Num(); ++FeatureIndex)
	{
		checkf(AllocatedDLSSFeatures[FeatureIndex].GetSharedReferenceCount() == 1,TEXT("There should be no FDLSSState::DLSSFeature references elsewhere."));
	}

	AllocatedDLSSFeatures.Empty();
	SET_DWORD_STAT(STAT_DLSSNumFeatures, AllocatedDLSSFeatures.Num());
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
}

void NGXRHI::ApplyCommonNGXParameterSettings(NVSDK_NGX_Parameter* InOutParameter, const FRHIDLSSArguments& InArguments)
{
	NVSDK_NGX_Parameter_SetI(InOutParameter, NVSDK_NGX_Parameter_FreeMemOnReleaseFeature, InArguments.bReleaseMemoryOnDelete ? 1 : 0);

	NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, static_cast<uint32>(InArguments.DLSSPreset));
	NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality, static_cast<uint32>(InArguments.DLSSPreset));
	NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, static_cast<uint32>(InArguments.DLSSPreset));
	NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, static_cast<uint32>(InArguments.DLSSPreset));
	NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, static_cast<uint32>(InArguments.DLSSPreset));
	NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, static_cast<uint32>(InArguments.DLSSPreset));

	static_assert (int(ENGXDLSSDenoiserMode::MaxValue) == 1, "dear DLSS plugin NVIDIA developer, please update this code to handle the new ENGXDLSSDenoiserMode enum values");
	if (NGXQueryFeature.bIsDlssRRAvailable && InArguments.DenoiserMode == ENGXDLSSDenoiserMode::DLSSRR)
	{
		NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_RayReconstruction_Hint_Render_Preset_DLAA, static_cast<uint32>(InArguments.DLSSRRPreset));
		NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_RayReconstruction_Hint_Render_Preset_UltraQuality, static_cast<uint32>(InArguments.DLSSRRPreset));
		NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_RayReconstruction_Hint_Render_Preset_Quality, static_cast<uint32>(InArguments.DLSSRRPreset));
		NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_RayReconstruction_Hint_Render_Preset_Balanced, static_cast<uint32>(InArguments.DLSSRRPreset));
		NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_RayReconstruction_Hint_Render_Preset_Performance, static_cast<uint32>(InArguments.DLSSRRPreset));
		NVSDK_NGX_Parameter_SetUI(InOutParameter, NVSDK_NGX_Parameter_RayReconstruction_Hint_Render_Preset_UltraPerformance, static_cast<uint32>(InArguments.DLSSRRPreset));
	}
}

void NGXRHI::TickPoolElements()
{
	check(!IsRunningRHIInSeparateThread() || IsInRHIThread());
	const uint32 kFramesUntilRelease = CVarNGXFramesUntilFeatureDestruction.GetValueOnAnyThread();

	int32 FeatureIndex = 0;

	while (FeatureIndex < AllocatedDLSSFeatures.Num())
	{
		TSharedPtr<NGXDLSSFeature>& Feature = AllocatedDLSSFeatures[FeatureIndex];

		const bool bIsUnused = Feature.GetSharedReferenceCount() == 1;
		const bool bNotRequestedRecently = (FrameCounter - Feature->LastUsedFrame) > kFramesUntilRelease;

		if (bIsUnused && bNotRequestedRecently)
		{
			Swap(Feature, AllocatedDLSSFeatures.Last());
			AllocatedDLSSFeatures.Pop();
		}
		else
		{
			++FeatureIndex;
		}
	}

	SET_DWORD_STAT(STAT_DLSSNumFeatures, AllocatedDLSSFeatures.Num());

	if(NGXQueryFeature.CapabilityParameters)
	{
		unsigned long long VRAM = 0;

		NVSDK_NGX_Result ResultGetStats = NGX_DLSS_GET_STATS(NGXQueryFeature.CapabilityParameters, &VRAM);

		checkf(NVSDK_NGX_SUCCEED(ResultGetStats), TEXT("Failed to retrieve DLSS memory statistics via NGX_DLSS_GET_STATS -> (%u %s)"), ResultGetStats, GetNGXResultAsString(ResultGetStats));
		if (NVSDK_NGX_SUCCEED(ResultGetStats))
		{
			SET_DWORD_STAT(STAT_DLSSInternalGPUMemory, VRAM);
		}
	}

	++FrameCounter;
}

IMPLEMENT_MODULE(FNGXRHIModule, NGXRHI)

#undef LOCTEXT_NAMESPACE

void FNGXRHIModule::StartupModule()
{
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));

	int32 NGXLogLevel = CVarNGXLogLevel.GetValueOnAnyThread();
	if (FParse::Value(FCommandLine::Get(), TEXT("ngxloglevel="), NGXLogLevel))
	{
		CVarNGXLogLevel->Set(NGXLogLevel, ECVF_SetByCommandline);
	}

	UE_LOG(LogDLSSNGXRHI, Log, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
}

void FNGXRHIModule::ShutdownModule()
{
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));
	UE_LOG(LogDLSSNGXRHI, Log, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
}

