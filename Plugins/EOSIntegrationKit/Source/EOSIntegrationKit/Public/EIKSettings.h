

#pragma once

#include "UObject/ObjectMacros.h"
#include "Engine/RuntimeOptionsBase.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/MessageDialog.h"
#include "Engine/DataAsset.h"
#include "Runtime/Launch/Resources/Version.h"
#include "EIKSettings.generated.h"

struct FEOSArtifactSettings
{
	FString ArtifactName;
	FString ClientId;
	FString ClientSecret;
	FString ProductId;
	FString SandboxId;
	FString DeploymentId;
	FString EncryptionKey;

	void ParseRawArrayEntry(const FString& RawLine);
};

UCLASS(Deprecated)
class UDEPRECATED_EIKArtifactSettings :
	public UDataAsset
{
	GENERATED_BODY()

public:
	UDEPRECATED_EIKArtifactSettings()
	{
	}
};

USTRUCT(BlueprintType)
struct FEArtifactSettings
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EOS Settings")
	FString ArtifactName;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Artifact Settings")
	FString ClientId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Artifact Settings")
	FString ClientSecret;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Artifact Settings")
	FString ProductId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Artifact Settings")
	FString SandboxId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Artifact Settings")
	FString DeploymentId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Artifact Settings")
	FString EncryptionKey = TEXT("9B86F278E855EE69855A5CAE0B1AD04166A143FD98CF96EC71AA4409F9B301C3");

	FEOSArtifactSettings ToNative() const;
};

struct FEOSSettings
{
	FString ApiKey;
	FString ReturnLevelName;
	FString CacheDir;
	FString DefaultArtifactName;
	FString VoiceArtifactName;
	FString DedicatedServerArtifactName;
	int32 TickBudgetInMilliseconds;
	int32 TitleStorageReadChunkLength;
	bool bEnableOverlay;
	bool bEnableSocialOverlay;
	bool bEnableEditorOverlay;
	bool bUseLauncherChecks;
	bool bUseEAS;
	bool bUseEOSConnect;
	bool bUseEOSSessions;
	bool bMirrorStatsToEOS;
	bool bMirrorAchievementsToEOS;
	bool bMirrorPresenceToEAS;
	TArray<FEOSArtifactSettings> Artifacts;
	TArray<FString> TitleStorageTags;
};

UENUM(BlueprintType)
enum EEIK_AutoLoginType {
	AutoLogin_None 			UMETA(DisplayName="None"),

	AutoLogin_DeveloperTool 	UMETA(DisplayName="Developer Tool"),

	AutoLogin_PersistentAuth 	UMETA(DisplayName="Persistent Auth"),

	AutoLogin_DeviceIdLogin 	UMETA(DisplayName="Device ID Login"),

	AutoLogin_AccountPortalLogin 	UMETA(DisplayName="Account Portal Login"),

	AutoLogin_PlatformLogin 	UMETA(DisplayName="Platform Login"),

	AutoLogin_SteamLogin 	UMETA(DisplayName="Steam Login"),

	AutoLogin_PSNLogin 	UMETA(DisplayName="PSN Login"),

	AutoLogin_GoogleLogin 	UMETA(DisplayName="Google Login"),

	AutoLogin_AppleLogin 	UMETA(DisplayName="Apple Login"),

};

UENUM(BlueprintType)
enum EEIK_FallbackForAutoLoginType
{

	Fallback_None UMETA(DisplayName="None"),

	Fallback_DeviceIdLogin UMETA(DisplayName="Device ID Login"),

	Fallback_AccountPortalLogin UMETA(DisplayName="Account Portal Login")
};

UENUM(BlueprintType)
enum EEIK_LoginFlags_LocalForSettings
{
	T_EOS_AS_NoFlags = 0 UMETA(DisplayName = "No Flags"),

	EOS_AS_BasicProfile = 0x1 UMETA(DisplayName = "Basic Profile"),

	EOS_AS_FriendsList = 0x2 UMETA(DisplayName = "Friends List"),

	EOS_AS_Presence = 0x4 UMETA(DisplayName = "Presence"),

	EOS_AS_FriendsManagement = 0x8 UMETA(DisplayName = "Friends Management"),

	EOS_AS_Email = 0x10 UMETA(DisplayName = "Email"),

	EOS_AS_Country = 0x20 UMETA(DisplayName = "Country"),
};

UENUM(BlueprintType)
enum EEIK_BuildConfiguration
{

	EIK_UnknownBuild = 0 UMETA(DisplayName = "Unknown"),

	EIK_Debug = 1 UMETA(DisplayName = "Debug"),

	EIK_DebugGame = 2 UMETA(DisplayName = "DebugGame"),

	EIK_Development = 3 UMETA(DisplayName = "Development"),

	EIK_Shipping = 4 UMETA(DisplayName = "Shipping"),

	EIK_Test = 5 UMETA(DisplayName = "Test"),
};

UCLASS(Config=Engine, DefaultConfig)
class EOSINTEGRATIONKIT_API UEIKSettings :
	public URuntimeOptionsBase
{
	GENERATED_BODY()

public:

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Integration Kit Settings")
	bool bAutomaticallySetupEIK;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Integration Kit Settings")
	bool bAutoLaunchDevTool = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Integration Kit Settings")
	FString OrganizationName;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Integration Kit Settings")
	FString ProductName;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Integration Kit Settings")
	bool bEnableGoogleOneTap = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Integration Kit Settings")
	bool bEnableGooglePlayGames = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Integration Kit Settings", meta = (EditCondition = "bEnableGooglePlayGames"))
	FString GooglePlayGamesAppID = FString("");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Login Settings|Auto Login")
	TEnumAsByte<EEIK_AutoLoginType> AutoLoginType;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Login Settings|Auto Login")
	TEnumAsByte<EEIK_FallbackForAutoLoginType> FallbackForAutoLoginType;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Login Settings|Auto Login")
	bool bUse_EAS_ForAutoLogin = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Login Settings|Auto Login")
	FString DeveloperToolUrl = TEXT("localhost:6300");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Login Settings")
	TArray<TEnumAsByte<EEIK_LoginFlags_LocalForSettings>> LoginFlags = {EEIK_LoginFlags_LocalForSettings::EOS_AS_BasicProfile, EEIK_LoginFlags_LocalForSettings::EOS_AS_FriendsList, EEIK_LoginFlags_LocalForSettings::EOS_AS_Presence};

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "EOS Settings|Player Ticketing Settings")
	FString ApiKey;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Title Storage Settings")
	FString CacheDir = TEXT("CacheDir");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings")
	int32 TickBudgetInMilliseconds = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Overlay Settings")
	bool bEnableOverlay = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Overlay Settings")
	bool bEnableSocialOverlay = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "EOS Settings|Overlay Settings")
	bool bEnableEditorOverlay = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "EOS Settings|Overlay Settings")
	FString ReturnLevelName;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings", DisplayName="Require Being Launched by the Epic Games Store")
	bool bUseLauncherChecks = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Title Storage Settings")
	TArray<FString> TitleStorageTags;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Title Storage Settings")
	int32 TitleStorageReadChunkLength = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Artifact Settings")
	FString DefaultArtifactName = TEXT("DefaultArtifact");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Artifact Settings")
	FString VoiceArtifactName = TEXT("DefaultArtifact");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Artifact Settings")
	FString DedicatedServerArtifactName = TEXT("DefaultArtifact");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Artifact Settings|Platform Specific")
	FString PlatformSpecificArtifactName = TEXT("DefaultArtifact");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Artifact Settings")
	TArray<FEArtifactSettings> Artifacts;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Artifact Settings|Platform Specific|Android")
	FString ClientId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Artifact Settings|Platform Specific|iOS|GoogleSignIn")
	FString Google_ClientId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="EOS Settings|Artifact Settings|Platform Specific|iOS|GoogleSignIn")
	FString Google_ReverseClientId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy", DisplayName="Build Configuration")
	TEnumAsByte<EEIK_BuildConfiguration> OneClick_BuildConfiguration;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Credentials", DisplayName="Organization Id")
	FString OneClick_OrganizationId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Credentials", DisplayName="Product Id")
	FString OneClick_ProductId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Credentials", DisplayName="Artifact Id")
	FString OneClick_ArtifactId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Credentials", DisplayName="Client Id")
	FString OneClick_ClientId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Credentials", DisplayName="Client Secret")
	FString OneClick_ClientSecret;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Parameters", DisplayName="Cloud Dir")
	FString OneClick_CloudDirOverride;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Parameters", DisplayName="Args Override")
	FString OneClick_ArgsOverride;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Parameters", DisplayName="App Launch Override")
	FString OneClick_AppLaunchOverride;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Parameters", DisplayName="Build Version Override")
	FString OneClick_BuildVersionOverride;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Parameters", DisplayName="App Args Override")
	FString OneClick_AppArgsOverride;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="One Click Deploy|Build Patch Tool Parameters", DisplayName="Build Root Override")
	FString OneClick_BuildRootOverride;

	UPROPERTY()
	bool bUseEAS = false;

	UPROPERTY()
	bool bUseEOSConnect = false;

	UPROPERTY()
	bool bMirrorStatsToEOS = false;

	UPROPERTY()
	bool bMirrorAchievementsToEOS = false;

	UPROPERTY()
	bool bUseEOSSessions = false;

	UPROPERTY()
	bool bMirrorPresenceToEAS = false;

	static bool GetSettingsForArtifact(const FString& ArtifactName, FEOSArtifactSettings& OutSettings);

	static FEOSSettings GetSettings();
	FEOSSettings ToNative() const;

private:
#if WITH_EDITOR
	EAppReturnType::Type ShowRestartWarning(const FText& Title);
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	static bool AutoGetSettingsForArtifact(const FString& ArtifactName, FEOSArtifactSettings& OutSettings);
	static bool ManualGetSettingsForArtifact(const FString& ArtifactName, FEOSArtifactSettings& OutSettings);

	static FEOSSettings AutoGetSettings();
	static const FEOSSettings& ManualGetSettings();
};