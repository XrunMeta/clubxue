

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SIK_GameServerStatsLibrary.generated.h"

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_GameServerStatsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, DisplayName = "Clear User Achievement", meta=(Keywords="ClearUserAchievement"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool ClearUserAchievement(FSIK_SteamId SteamId, FString AchievementName);

	UFUNCTION(BlueprintCallable, DisplayName = "Get User Achievement", meta=(Keywords="GetUserAchievement"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool GetUserAchievement(FSIK_SteamId SteamId, FString AchievementName, bool& bAchieved);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Int User Stat", meta=(Keywords="GetUserStat"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool GetIntUserStat(FSIK_SteamId SteamId, FString StatName, int32& Data);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Float User Stat", meta=(Keywords="GetUserStat"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool GetFloatUserStat(FSIK_SteamId SteamId, FString StatName, float& Data);

	UFUNCTION(BlueprintCallable, DisplayName = "Request User Stats", meta=(Keywords="RequestUserStats"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool RequestUserStats(FSIK_SteamId SteamId);

	UFUNCTION(BlueprintCallable, DisplayName = "Set User Achievement", meta=(Keywords="SetUserAchievement"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool SetUserAchievement(FSIK_SteamId SteamId, FString AchievementName);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Int User Stat", meta=(Keywords="SetUserStat"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool SetIntUserStat(FSIK_SteamId SteamId, FString StatName, int32 Data);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Float User Stat", meta=(Keywords="SetUserStat"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool SetFloatUserStat(FSIK_SteamId SteamId, FString StatName, float Data);

	UFUNCTION(BlueprintCallable, DisplayName = "Store User Stats", meta=(Keywords="StoreUserStats"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool StoreUserStats(FSIK_SteamId SteamId);

	UFUNCTION(BlueprintCallable, DisplayName = "Update User Avg Rate Stat", meta=(Keywords="UpdateUserStat"), Category = "Steam Integration Kit || SDK Functions || Game Server Stats")
	static bool UpdateUserAvgRateStat(FSIK_SteamId SteamId, FString StatName, float CountThisSession, float SessionLength);

};
