

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SIK_SharedFile.h"
#include "SIK_MatchmakingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFavoritesListAccountsUpdated, TEnumAsByte<ESIK_Result>, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(FOnFavoritesListChanged, FString, IP, int32, QueryPort, int32, ConnPort, FSIK_AppId, AppID, int32, Flags, bool, Add, int32, AccountId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnLobbyMessageDel, FSIK_SteamId, LobbyId, FSIK_SteamId, UserId, TEnumAsByte<ESIK_LobbyChatEntryType>, ChatEntryType, int32, ChatID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnLobbyChatUpdateDel, FSIK_SteamId, LobbyId, FSIK_SteamId, UserId, FSIK_SteamId, MemberIDMakingChange, TEnumAsByte<ESIK_LobbyChatMemberStateChange>, ChatMemberStateChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLobbyCreatedDel, FSIK_SteamId, LobbyId, TEnumAsByte<ESIK_Result>, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLobbyDataUpdateDel, FSIK_SteamId, LobbyId, FSIK_SteamId, UserId, bool, Success);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLobbyEnterDel, FSIK_SteamId, LobbyId, bool, bLocked,TEnumAsByte<ESIK_ChatRoomEnterResponse>, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnLobbyGameCreatedDel, FSIK_SteamId, LobbyId, FSIK_SteamId, GameServerId, FString, GameServerIP, int32, GameServerPort);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLobbyInviteDel, FSIK_SteamId, LobbyId, FSIK_SteamId, SenderUserId, FSIK_AppId, AppId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLobbyKickedDel, FSIK_SteamId, LobbyId, FSIK_SteamId, AdminId, bool, bKickedDueToDisconnect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyMatchListDel, int32, LobbyCount);

UCLASS(Category = "Steam Integration Kit | Matchmaking")
class STEAMINTEGRATIONKIT_API USIK_MatchmakingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	USIK_MatchmakingSubsystem();
	~USIK_MatchmakingSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnFavoritesListAccountsUpdated OnFavoritesListAccountsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnFavoritesListChanged OnFavoritesListChanged;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnLobbyMessageDel OnLobbyChatMsg;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnLobbyChatUpdateDel OnLobbyChatUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnLobbyCreatedDel OnLobbyCreated;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnLobbyDataUpdateDel OnLobbyDataUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnLobbyEnterDel OnLobbyEnter;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnLobbyGameCreatedDel OnLobbyGameCreated;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnLobbyInviteDel OnLobbyInvite;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnLobbyKickedDel OnLobbyKicked;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Matchmaking | Callbacks")
	FOnLobbyMatchListDel OnLobbyMatchList;

private:
#if (WITH_ENGINE_STEAM && ONLINESUBSYSTEMSTEAM_PACKAGE) || (WITH_STEAMKIT && !WITH_ENGINE_STEAM)
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnFavoritesListAccountsCallbck, FavoritesListAccountsUpdated_t, m_CallbackFavoritesListAccountsUpdated);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnFavoritesListChangedCallback, FavoritesListChanged_t, m_CallbackFavoritesListChanged);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnLobbyChatMsgCallback, LobbyChatMsg_t, m_CallbackLobbyChatMsg);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnLobbyChatUpdateCallback, LobbyChatUpdate_t, m_CallbackLobbyChatUpdate);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnLobbyCreatedCallback, LobbyCreated_t, m_CallbackLobbyCreated);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnLobbyDataUpdateCallback, LobbyDataUpdate_t, m_CallbackLobbyDataUpdate);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnLobbyEnterCallback, LobbyEnter_t, m_CallbackLobbyEnter);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnLobbyGameCreatedCallback, LobbyGameCreated_t, m_CallbackLobbyGameCreated);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnLobbyInviteCallback, LobbyInvite_t, m_CallbackLobbyInvite);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnLobbyKickedCallback, LobbyKicked_t, m_CallbackLobbyKicked);
	STEAM_CALLBACK_MANUAL(USIK_MatchmakingSubsystem, OnLobbyMatchListCallback, LobbyMatchList_t, m_CallbackLobbyMatchList);
#endif
};
