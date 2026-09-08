

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SIK_MatchmakingLibrary.generated.h"

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_MatchmakingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, DisplayName = "Add Favorite Game", meta=(Keywords="AddFavoriteGame"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static int32 AddFavoriteGame(FSIK_AppId AppID, FString IP, int32 ConnPort, int32 QueryPort, TArray<int32> Flags, int32 Time32LastPlayedOnServer);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Request Lobby List Compatible Members Filter", meta=(Keywords="AddRequestLobbyListCompatibleMembersFilter"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void AddRequestLobbyListCompatibleMembersFilter(FSIK_SteamId SteamID);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Request Lobby List Distance Filter", meta=(Keywords="AddRequestLobbyListDistanceFilter"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void AddRequestLobbyListDistanceFilter(TEnumAsByte<ESIK_LobbyDistanceFilter> LobbyDistanceFilter);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Request Lobby List Filter Slots Available", meta=(Keywords="AddRequestLobbyListFilterSlotsAvailable"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void AddRequestLobbyListFilterSlotsAvailable(int32 SlotsAvailable);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Request Lobby List Near Value Filter", meta=(Keywords="AddRequestLobbyListNearValueFilter"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void AddRequestLobbyListNearValueFilter(FString KeyToMatch, int32 ValueToBeCloseTo);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Request Lobby List Numerical Filter", meta=(Keywords="AddRequestLobbyListNumericalFilter"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void AddRequestLobbyListNumericalFilter(FString KeyToMatch, int32 ValueToMatch, TEnumAsByte<ESIK_LobbyComparisonType> ComparisonType);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Request Lobby List Result Count Filter", meta=(Keywords="AddRequestLobbyListResultCountFilter"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void AddRequestLobbyListResultCountFilter(int32 MaxResults);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Request Lobby List String Filter", meta=(Keywords="AddRequestLobbyListStringFilter"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void AddRequestLobbyListStringFilter(FString KeyToMatch, FString ValueToMatch, TEnumAsByte<ESIK_LobbyComparisonType> ComparisonType);

	UFUNCTION(BlueprintCallable, DisplayName = "Delete Lobby Data", meta=(Keywords="DeleteLobbyData"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool DeleteLobbyData(FSIK_SteamId SteamID, FString Key);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Favorite Game", meta=(Keywords="GetFavoriteGame"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool GetFavoriteGame(int32 GameIndex, FSIK_AppId& AppID, FString& IP, int32& ConnPort, int32& QueryPort, TArray<int32>& Flags, int32& Time32LastPlayedOnServer);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Favorite Game Count", meta=(Keywords="GetFavoriteGameCount"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static int32 GetFavoriteGameCount();

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby By Index", meta=(Keywords="GetLobbyByIndex"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static FSIK_SteamId GetLobbyByIndex(int32 LobbyIndex);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby Chat Entry", meta=(Keywords="GetLobbyChatEntry"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void GetLobbyChatEntry(FSIK_SteamId SteamID, int32 ChatID, FSIK_SteamId& SteamIDUser, FString& ChatEntry, TEnumAsByte<ESIK_LobbyChatEntryType>& ChatEntryType);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby Data", meta=(Keywords="GetLobbyData"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static FString GetLobbyData(FSIK_SteamId LobbyID, FString Key);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby Data By Index", meta=(Keywords="GetLobbyDataByIndex"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool GetLobbyDataByIndex(FSIK_SteamId LobbyID, int32 DataIndex, FString& Key, FString& Value);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby Data Count", meta=(Keywords="GetLobbyDataCount"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static int32 GetLobbyDataCount(FSIK_SteamId LobbyID);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby Game Server", meta=(Keywords="GetLobbyGameServer"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool GetLobbyGameServer(FSIK_SteamId LobbyID, FString& ServerIP, int32& ServerPort, FSIK_SteamId& SteamID);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby Member By Index", meta=(Keywords="GetLobbyMemberByIndex"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static FSIK_SteamId GetLobbyMemberByIndex(FSIK_SteamId LobbyID, int32 MemberIndex);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby Member Data", meta=(Keywords="GetLobbyMemberData"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static FString GetLobbyMemberData(FSIK_SteamId LobbyID, FSIK_SteamId UserID, FString Key);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby Member Limit", meta=(Keywords="GetLobbyMemberLimit"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static int32 GetLobbyMemberLimit(FSIK_SteamId LobbyID);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Lobby Owner", meta=(Keywords="GetLobbyOwner"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static FSIK_SteamId GetLobbyOwner(FSIK_SteamId LobbyID);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Num Lobby Members", meta=(Keywords="GetNumLobbyMembers"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static int32 GetNumLobbyMembers(FSIK_SteamId LobbyID);

	UFUNCTION(BlueprintCallable, DisplayName = "Invite User To Lobby", meta=(Keywords="InviteUserToLobby"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool InviteUserToLobby(FSIK_SteamId LobbyID, FSIK_SteamId SteamID);

	UFUNCTION(BlueprintCallable, DisplayName = "Leave Lobby", meta=(Keywords="LeaveLobby"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void LeaveLobby(FSIK_SteamId LobbyID);

	UFUNCTION(BlueprintCallable, DisplayName = "Remove Favorite Game", meta=(Keywords="RemoveFavoriteGame"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool RemoveFavoriteGame(FSIK_AppId AppID, FString IP, int32 ConnPort, int32 QueryPort, TArray<int32> Flags);

	UFUNCTION(BlueprintCallable, DisplayName = "Request Lobby Data", meta=(Keywords="RequestLobbyData"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool RequestLobbyData(FSIK_SteamId LobbyID);

	UFUNCTION(BlueprintCallable, DisplayName = "Send Lobby Chat Message", meta=(Keywords="SendLobbyChatMessage"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool SendLobbyChatMessage(FSIK_SteamId LobbyID, FString Message);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Linked Lobby", meta=(Keywords="SetLinkedLobby"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void SetLinkedLobby(FSIK_SteamId LobbyID, FSIK_SteamId DependentLobbyID);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Lobby Data", meta=(Keywords="SetLobbyData"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool SetLobbyData(FSIK_SteamId LobbyID, FString Key, FString Value);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Lobby Game Server", meta=(Keywords="SetLobbyGameServer"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void SetLobbyGameServer(FSIK_SteamId LobbyID, FString ServerIP, int32 ServerPort, FSIK_SteamId SteamID);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Lobby Joinable", meta=(Keywords="SetLobbyJoinable"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool SetLobbyJoinable(FSIK_SteamId LobbyID, bool bJoinable);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Lobby Member Data", meta=(Keywords="SetLobbyMemberData"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void SetLobbyMemberData(FSIK_SteamId LobbyID, FString Key, FString Value);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Lobby Member Limit", meta=(Keywords="SetLobbyMemberLimit"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void SetLobbyMemberLimit(FSIK_SteamId LobbyID, int32 MemberLimit);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Lobby Owner", meta=(Keywords="SetLobbyOwner"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static void SetLobbyOwner(FSIK_SteamId LobbyID, FSIK_SteamId SteamID);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Lobby Type", meta=(Keywords="SetLobbyType"), Category="Steam Integration Kit || SDK Functions || Matchmaking")
	static bool SetLobbyType(FSIK_SteamId LobbyID, TEnumAsByte<ESIK_LobbyType> LobbyType);
};
