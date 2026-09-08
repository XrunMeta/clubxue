

#include "EIKSharedModule.h"

#include "Features/IModularFeatures.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "CoreGlobals.h"
#include "EOSShared.h"

#include COMPILED_PLATFORM_HEADER(EOSSDKManager.h)

#define LOCTEXT_NAMESPACE "EOS"

IMPLEMENT_MODULE(FEIKSharedModule, EIKShared);

void FEIKSharedModule::StartupModule()
{
#if WITH_EOS_SDK	
	if (IsRunningCommandlet())
	{

		UE_LOG(LogEIKSDK, Log, TEXT("IsRunningCommandlet=true, skipping EOSSDK initialization."))
		return;
	}

	SDKManager = MakeUnique<FPlatformEOSSDKManager>();
	check(SDKManager);

	IModularFeatures::Get().RegisterModularFeature(IEOSSDKManager::GetModularFeatureName(), SDKManager.Get());

	TArray<FString> ModulesToLoad;
	GConfig->GetArray(TEXT("EIKShared"), TEXT("ModulesToLoad"), ModulesToLoad, GEngineIni);
	for (const FString& ModuleToLoad : ModulesToLoad)
	{
		if (FModuleManager::Get().ModuleExists(*ModuleToLoad))
		{
			FModuleManager::Get().LoadModule(*ModuleToLoad);
		}
	}
#endif 
}

void FEIKSharedModule::ShutdownModule()
{
#if WITH_EOS_SDK
	if(SDKManager.IsValid())
	{
		IModularFeatures::Get().UnregisterModularFeature(IEOSSDKManager::GetModularFeatureName(), SDKManager.Get());
		SDKManager->Shutdown();
		SDKManager.Reset();
	}
#endif 
}

#undef LOCTEXT_NAMESPACE