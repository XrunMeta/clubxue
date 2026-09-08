

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SIK_GameServerLibrary.generated.h"

UENUM(BlueprintType)
enum ESIK_UserHasLicenseForAppResult
{
	UserHasLicenseResultHasLicense = 0 UMETA(DisplayName = "Has License"),
	UserHasLicenseResultDoesNotHaveLicense = 1 UMETA(DisplayName = "Does Not Have License"),
	UserHasLicenseResultNoAuth = 2 UMETA(DisplayName = "No Auth"),
};

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_GameServerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, DisplayName = "Begin Steam Auth Session", meta=(Keywords="BeginAuthSession"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static TEnumAsByte<ESIK_BeginAuthSessionResult> BeginAuthSession(const TArray<uint8>& Token, FSIK_SteamId SteamId);

	UFUNCTION(BlueprintCallable, DisplayName = "Logged On", meta=(Keywords="BLoggedOn"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static bool LoggedOn();

	UFUNCTION(BlueprintCallable, DisplayName = "Secure", meta=(Keywords="BSecure"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static bool Secure();

	UFUNCTION(BlueprintCallable, DisplayName = "Update User Data", meta=(Keywords="BUpdateUserData"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static bool UpdateUserData(FSIK_SteamId SteamId, const FString& PlayerName, int32 Score);

	UFUNCTION(BlueprintCallable, DisplayName = "Cancel Steam Auth Ticket", meta=(Keywords="CancelAuthTicket"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void CancelAuthTicket(FSIK_AuthTicket AuthTicket);

	UFUNCTION(BlueprintCallable, DisplayName = "Clear All Key Values", meta=(Keywords="ClearAllKeyValues"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void ClearAllKeyValues();

	UFUNCTION(BlueprintCallable, DisplayName = "Create Unauthenticated User Connection", meta=(Keywords="CreateUnauthenticatedUserConnection"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static FSIK_SteamId CreateUnauthenticatedUserConnection();

	UFUNCTION(BlueprintCallable, DisplayName = "End Auth Session", meta=(Keywords="EndAuthSession"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void EndAuthSession(FSIK_SteamId SteamId);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Auth Session Ticket", meta=(Keywords="GetAuthSessionTicket"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static TArray<uint8> GetAuthSessionTicket();

	UFUNCTION(BlueprintCallable, DisplayName = "Get Public IP", meta=(Keywords="GetPublicIP"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static FSIK_SteamIPAddress GetPublicIP();

	UFUNCTION(BlueprintCallable, DisplayName = "Get Server Steam ID", meta=(Keywords="GetSteamID"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static FSIK_SteamId GetSteamID();

	UFUNCTION(BlueprintCallable, DisplayName = "Log Server Off", meta=(Keywords="LogOff"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void LogOff();

	UFUNCTION(BlueprintCallable, DisplayName = "Log Server On", meta=(Keywords="LogOn"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void LogOn(const FString& Token);

	UFUNCTION(BlueprintCallable, DisplayName = "Log Server On Anonymous", meta=(Keywords="LogOnAnonymous"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void LogOnAnonymous();

	UFUNCTION(BlueprintCallable, DisplayName = "Request User Group Status", meta=(Keywords="RequestUserGroupStatus"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static bool RequestUserGroupStatus(FSIK_SteamId SteamId, FSIK_SteamId GroupId);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Bot Player Count", meta=(Keywords="SetBotPlayerCount"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetBotPlayerCount(int32 BotPlayerCount);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Dedicated Server", meta=(Keywords="SetDedicatedServer"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetDedicatedServer(bool bDedicated);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Game Data", meta=(Keywords="SetGameData"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetGameData(const FString& GameData);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Game Description", meta=(Keywords="SetGameDescription"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetGameDescription(const FString& GameDescription);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Game Tags", meta=(Keywords="SetGameTags"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetGameTags(const FString& GameTags);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Key Value", meta=(Keywords="SetKeyValue"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetKeyValue(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Map Name", meta=(Keywords="SetMapName"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetMapName(const FString& MapName);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Max Player Count", meta=(Keywords="SetMaxPlayerCount"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetMaxPlayerCount(int32 MaxPlayerCount);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Mod Dir", meta=(Keywords="SetModDir"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetModDir(const FString& ModDir);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Password Protected", meta=(Keywords="SetPasswordProtected"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetPasswordProtected(bool bPasswordProtected);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Product", meta=(Keywords="SetProduct"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetProduct(const FString& Product);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Region", meta=(Keywords="SetRegion"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetRegion(const FString& Region);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Server Name", meta=(Keywords="SetServerName"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetServerName(const FString& ServerName);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Spectator Port", meta=(Keywords="SetSpectatorPort"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetSpectatorPort(int32 SpectatorPort);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Spectator Server Name", meta=(Keywords="SetSpectatorServerName"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static void SetSpectatorServerName(const FString& SpectatorServerName);

	UFUNCTION(BlueprintCallable, DisplayName = "User Has License For App", meta=(Keywords="UserHasLicenseForApp"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static TEnumAsByte<ESIK_UserHasLicenseForAppResult> UserHasLicenseForApp(FSIK_SteamId SteamId, int32 AppId);

	UFUNCTION(BlueprintCallable, DisplayName = "Was Restart Requested", meta=(Keywords="WasRestartRequested"), Category = "Steam Integration Kit || SDK Functions || Game Server")
	static bool WasRestartRequested();
};
