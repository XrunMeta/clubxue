

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/RuntimeOptionsBase.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/MessageDialog.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/DataAsset.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Engine/DeveloperSettings.h"
#include "SIKSettings.generated.h"

UENUM(BlueprintType)
enum ESIK_BuildConfiguration
{

	UnknownBuild1 = 0 UMETA(DisplayName = "Unknown"),

	Debug1 = 1 UMETA(DisplayName = "Debug"),

	DebugGame1 = 2 UMETA(DisplayName = "DebugGame"),

	Development1 = 3 UMETA(DisplayName = "Development"),

	Shipping1 = 4 UMETA(DisplayName = "Shipping"),

	Test1 = 5 UMETA(DisplayName = "Test"),
};

UCLASS(meta=(DisplayName="Steam Integration Kit"))
class STEAMINTEGRATIONKIT_API USIKSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USIKSettings();

	UPROPERTY(EditAnywhere, Category = "Basic Settings")
	FString GameVersion = "1.0.0.0";

	UPROPERTY(EditAnywhere, Category = "Basic Settings")
	int32 SteamAppId = 480;

	UPROPERTY(EditAnywhere, Category = "Basic Settings")
	int32 SteamDevAppId = 480;

	UPROPERTY(EditAnywhere, Category = "Basic Settings")
	bool bRelaunchInSteam = false;

	UPROPERTY(EditAnywhere, Category = "Basic Settings")
	int32 GameServerPort = 7777;

	UPROPERTY(EditAnywhere, Category = "Basic Settings")
	int32 P2PConnectionTimeout = 120;

	UPROPERTY(EditAnywhere, Category = "Marketplace Version Settings | One Click Deploy")
	TArray<FString> MapsToCook;

	UPROPERTY(EditAnywhere, Category = "Marketplace Version Settings | One Click Deploy")
	TArray<int32> DepotIds;

	UPROPERTY(EditAnywhere, Category = "Marketplace Version Settings | One Click Deploy")
	FString BranchName;

	UPROPERTY(EditAnywhere, Category = "Marketplace Version Settings | Deployer Account Information", meta=(DisplayName="Username", ToolTip="Steam username. Will be stored in SIK_STEAM_USERNAME environment variable."))
	FString Username;

	UPROPERTY(EditAnywhere, Category = "Marketplace Version Settings | Deployer Account Information", meta=(DisplayName="Password", ToolTip="Steam password. Will be stored in SIK_STEAM_PASSWORD environment variable."))
	FString Password;

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, Category = "Marketplace Version Settings | Deployer Account Information", meta = (DisplayName = "☐ Save Credentials to System", ToolTip = "Click to save credentials"))
	bool bSaveToSystem = false;

	void SaveCredentialsToSystem();

private:
	bool SaveCredentialsToSystem_Windows();
	bool SaveCredentialsToSystem_Mac();
	bool SaveCredentialsToSystem_Linux();
#endif

	UPROPERTY(EditAnywhere, Category = "Marketplace Version Settings | Server Settings")
	FString ServerName;

	UPROPERTY(EditAnywhere, Category = "Marketplace Version Settings | Server Settings")
	FString ServerDescription;

	UPROPERTY(EditAnywhere, Category = "Marketplace Version Settings | Server Settings")
	FString ServerGameDir;

private:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostInitProperties() override;
#endif
	UPROPERTY()
	bool bUseEnvironmentVariables = false;
	UPROPERTY()
	bool bEngineInitialized = false;	
};
