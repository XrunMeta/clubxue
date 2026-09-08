

#include "StreamlineExtension.h"

#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FStreamlineExtensionModule"
DEFINE_LOG_CATEGORY_STATIC(LogStreamlineExtension, Log, All)

FSLFeatureDesc::FSLFeatureDesc(sl::Feature InSLFeature, const FString& InUEPluginName, const FString& InFeatureName)
	: SLFeature(InSLFeature)
	, UEPluginName(InUEPluginName)
	, FeatureName(InFeatureName)
{

	LoadCVar = TEXT("r.Streamline.Load.");
	for (auto Char : InFeatureName)
	{
		if (FChar::IsAlnum(Char))
		{
			LoadCVar += Char;
			CommandLineSuffix += FChar::ToLower(Char);
		}
	}
}

bool FStreamlineExtensionModule::RegisterFeature(const FSLFeatureDesc& Feature)
{
	if (bIsStreamlineInitialized)
	{
		UE_LOG(LogStreamlineExtension, Error, TEXT("Failed attempt to register SL feature %s after SL already initialized"), *Feature.FeatureName);
		return false;
	}

	Features.Add(Feature);
	return true;
}

FString FStreamlineExtensionModule::GetDefaultBinaryBaseDir(const FStringView PluginName)
{
	const FString PluginBaseDir = IPluginManager::Get().FindPlugin(PluginName)->GetBaseDir();
	const FString Platform = FPlatformProcess::GetBinariesSubdirectory();
	return FPaths::Combine(*PluginBaseDir, TEXT("Binaries"), TEXT("ThirdParty"), *Platform);
}

const TArray<FSLFeatureDesc>& FStreamlineExtensionModule::GetRegisteredFeatures() const
{
	return Features;
}

void FStreamlineExtensionModule::SetSLInitialized()
{
	bIsStreamlineInitialized = true;
}

void FStreamlineExtensionModule::StartupModule()
{
}

void FStreamlineExtensionModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStreamlineExtensionModule, StreamlineExtension)

