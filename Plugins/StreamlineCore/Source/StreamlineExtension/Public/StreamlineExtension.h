

#pragma once

#include "Modules/ModuleManager.h"

#define UE_API STREAMLINEEXTENSION_API

namespace sl
{
	using Feature = uint32_t;
}

struct FSLFeatureDesc
{

	UE_API FSLFeatureDesc(sl::Feature InSLFeature, const FString& InUEPluginName, const FString& InFeatureName);

	FSLFeatureDesc() = default;

	sl::Feature SLFeature;
	FString UEPluginName;
	FString FeatureName;
	FString CommandLineSuffix;
	FString LoadCVar;
	TArray<FString> BinaryDirectories;
	bool bAllowByDefault = true;
};

class FStreamlineExtensionModule: public IModuleInterface
{
public:

	static UE_API FStreamlineExtensionModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FStreamlineExtensionModule>(FName("StreamlineExtension"));
	}

	UE_API bool RegisterFeature(const FSLFeatureDesc& Feature);

	static UE_API FString GetDefaultBinaryBaseDir(const FStringView PluginName);

	UE_API const TArray<FSLFeatureDesc>& GetRegisteredFeatures() const;
	UE_API void SetSLInitialized();

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	bool bIsStreamlineInitialized = false;
	TArray<FSLFeatureDesc> Features;
};

#undef UE_API

