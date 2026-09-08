

#include "OnlineSubsystemEIKModule.h"
#include "OnlineSubsystemEOSPrivate.h"
#include "OnlineSubsystemModule.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOSTypes.h"
#include "EIKSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Features/IModularFeature.h"
#include "Features/IModularFeatures.h"

#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/LazySingleton.h"
#include "Misc/MessageDialog.h"

#if WITH_EDITOR
	#include "ISettingsModule.h"
	#include "ISettingsSection.h"
	#include "ToolMenus.h"
#endif

#define LOCTEXT_NAMESPACE "EIK"

IMPLEMENT_MODULE(FOnlineSubsystemEIKModule, OnlineSubsystemEIK);

class FOnlineFactoryEOS :
	public IOnlineFactory
{
public:

	FOnlineFactoryEOS() {}
	virtual ~FOnlineFactoryEOS() {}

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName)
	{
		FOnlineSubsystemEOSPtr OnlineSub = MakeShared<FOnlineSubsystemEOS, ESPMode::ThreadSafe>(InstanceName);
		if (!OnlineSub->Init())
		{
			UE_LOG_ONLINE(Warning, TEXT("EOS API failed to initialize!"));
			OnlineSub->Shutdown();
			OnlineSub = nullptr;
		}

		return OnlineSub;
	}
};

void FOnlineSubsystemEIKModule::StartupModule()
{
	if (IsRunningCommandlet())
	{
		return;
	}

	EOSFactory = new FOnlineFactoryEOS();

	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.RegisterPlatformService("EIK", EOSFactory);

#if WITH_EOS_SDK

	FOnlineSubsystemEOS::ModuleInit();
#endif

#if WITH_EDITOR
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FOnlineSubsystemEIKModule::RegisterMenus));
#endif
#if WITH_EDITOR
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FOnlineSubsystemEIKModule::OnPostEngineInit);
	FCoreDelegates::OnPreExit.AddRaw(this, &FOnlineSubsystemEIKModule::OnPreExit);
#endif
}

#if WITH_EDITOR
void FOnlineSubsystemEIKModule::OnPostEngineInit()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Game", "Online Subsystem EIK",
			LOCTEXT("OSSEOSSettingsName", "EOS Integration Kit"),
			LOCTEXT("OSSEOSSettingsDescription", "Take control of your EOS Integration Kit settings here. Explore the options available or refer to the documentation for further information on how to customize your settings."),
			GetMutableDefault<UEIKSettings>());
	}
}
#endif

#if WITH_EDITOR
void FOnlineSubsystemEIKModule::OnPreExit()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Game", "Online Subsystem EIK");
	}
}
#endif

void FOnlineSubsystemEIKModule::ShutdownModule()
{
	if (IsRunningCommandlet())
	{
		return;
	}

#if WITH_EDITOR

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);
#endif
#if WITH_EDITOR
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	FCoreDelegates::OnPreExit.RemoveAll(this);
#endif

#if WITH_EOS_SDK
	FOnlineSubsystemEOS::ModuleShutdown();
#endif

	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.UnregisterPlatformService("EIK");

	delete EOSFactory;
	EOSFactory = nullptr;

	TLazySingleton<FUniqueNetIdEOSRegistry>::TearDown();
}

void FOnlineSubsystemEIKModule::PluginButtonClicked()
{
#if WITH_EDITOR
	FEOSSettings EOSSettings = UEIKSettings::GetSettings();
	UEIKSettings* UeikSettings = GetMutableDefault<UEIKSettings>();
	FString ProductName = UeikSettings->ProductName;
	FString OrganizationId = UeikSettings->OrganizationName;

	if (!ProductName.IsEmpty() && !OrganizationId.IsEmpty())
	{

		FString URL = FString::Printf(TEXT("https://dev.epicgames.com/portal/en-US/%s/products/%s/"), *OrganizationId, *ProductName);
		FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
	}
	else
	{

		FString ErrorMessage;
		if (ProductName.IsEmpty())
		{
			ErrorMessage = TEXT("Product name");
		}
		else if (OrganizationId.IsEmpty())
		{
			ErrorMessage = TEXT("Organization Name");
		}
		else if(OrganizationId.IsEmpty() && ProductName.IsEmpty() )
		{
			ErrorMessage = TEXT("Organization Name and Product Name");
		}

		FString SettingsText = FString::Printf(TEXT("Make sure to set the value of %s in the project settings."), *ErrorMessage);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(SettingsText));
	}
#endif
}

void FOnlineSubsystemEIKModule::RegisterMenus()
{
#if WITH_EDITOR

	FToolMenuOwnerScoped OwnerScoped(this);

	{
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
			}
		}
	}
#endif
}

void FOnlineSubsystemEIKModule::ConfigureOnlineSubsystemEIK()
{
    FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
    FString EngineIniText;
    if (FFileHelper::LoadFileToString(EngineIniText, *EngineIniPath))
    {
        bool bConfigChanged = false;

        if (!EngineIniText.Contains(TEXT("[OnlineSubsystemEIK]")))
        {
            EngineIniText += TEXT("\n[OnlineSubsystemEIK]\nbEnabled=true\n");
            bConfigChanged = true;
        }

        if (!EngineIniText.Contains(TEXT("[OnlineSubsystem]")))
        {
            EngineIniText += TEXT("\n[OnlineSubsystem]\nDefaultPlatformService=EIK\n");
            bConfigChanged = true;
        }

        if (!EngineIniText.Contains(TEXT("\n[/Script/OnlineSubsystemEIK.NetDriverEIK]")))
        {
        	FString Comment = TEXT(";EIK Comment: You do not need to worry about this setting as we dynamically set it in Travel URL depending upon if we are using Listen Server or Dedicated Server\n");
        	EngineIniText += Comment;
            EngineIniText += TEXT("[/Script/OnlineSubsystemEIK.NetDriverEIK]\nbIsUsingP2PSockets=true\n");
            bConfigChanged = true;
        }

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6

        if (!EngineIniText.Contains(TEXT("[/Script/Engine.Engine]")))
        {
            EngineIniText += TEXT("\n[/Script/Engine.Engine]\n");

            FString NetDriverDefinitions = FString::Printf(
                TEXT("!NetDriverDefinitions=ClearArray\n+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEIK.NetDriverEIK\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n+NetDriverDefinitions=(DefName=\"BeaconNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEIK.NetDriverEIK\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n")
            );
            EngineIniText += NetDriverDefinitions;

            bConfigChanged = true;
        }
#else

        if (!EngineIniText.Contains(TEXT("[/Script/Engine.GameEngine]")))
        {
            EngineIniText += TEXT("\n[/Script/Engine.GameEngine]\n");

            FString NetDriverDefinitions = FString::Printf(
                TEXT("!NetDriverDefinitions=ClearArray\n+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"OnlineSubsystemEIK.NetDriverEIK\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n")
            );
            EngineIniText += NetDriverDefinitions;

            bConfigChanged = true;
        }
#endif

        if (bConfigChanged)
        {
            if (FFileHelper::SaveStringToFile(EngineIniText, *EngineIniPath))
            {
                UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystemEIK configuration added/updated in DefaultEngine.ini"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to save modified DefaultEngine.ini"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystemEIK configuration already exists in DefaultEngine.ini"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DefaultEngine.ini"));
    }
}

#undef LOCTEXT_NAMESPACE
