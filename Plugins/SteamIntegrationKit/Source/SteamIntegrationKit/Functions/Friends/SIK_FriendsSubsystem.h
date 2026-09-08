

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SIK_SharedFile.h"
#include "SIK_FriendsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAvatarImageLoaded, FSIK_SteamId, SteamId, int32, Image, int32, Wide, int32, Tall);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnClanOfficerList, FSIK_SteamId, SteamId, int32, OfficerCount, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFriendRichPresenceUpdate, FSIK_SteamId, SteamId, FSIK_AppId, AppId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDownloadClanActivityCountsResult, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnFriendEnumerateFollowingList, TEnumAsByte<ESIK_Result>, Result, TArray<FSIK_SteamId>, SteamId, int32, ResultsReturned, int32, TotalResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFriendsGetFollowerCount, TEnumAsByte<ESIK_Result>, Result, FSIK_SteamId, SteamId, int32, Count);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFriendsIsFollowing, TEnumAsByte<ESIK_Result>, Result, FSIK_SteamId, SteamId, bool, bIsFollowing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameConnectedChatJoin, FSIK_SteamId, ClanChatId, FSIK_SteamId, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnGameConnectedChatLeave, FSIK_SteamId, ClanChatId, FSIK_SteamId, User, bool, Kicked, bool, Dropped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGameConnectedClanChatMsg, FSIK_SteamId, ClanChatId, FSIK_SteamId, User, int32, MessageID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameConnectedFriendChatMsg, FSIK_SteamId, User, int32, MessageID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameLobbyJoinRequested, FSIK_SteamId, SteamId, FSIK_SteamId, LobbyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOverlayActivated, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameRichPresenceJoinRequested, FSIK_SteamId, SteamId, FString, ConnectString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameServerChangeRequested, FString, Server, FString, Password);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJoinClanChatRoomCompletionResult, FSIK_SteamId, ClanChatId, TEnumAsByte<ESIK_ChatRoomEnterResponse>, ChatRoomEnterResponse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPersonaStateChange, FSIK_SteamId, SteamId, int32, ChangeFlags);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSetPersonaNameResponse, TEnumAsByte<ESIK_Result>, Result, bool, bLocalSuccess, bool, bRemoteSuccess);

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_FriendsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	USIK_FriendsSubsystem();
	~USIK_FriendsSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnClanOfficerList OnClanOfficerList;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnFriendRichPresenceUpdate OnFriendRichPresenceUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnDownloadClanActivityCountsResult OnDownloadClanActivityCountsResult;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnFriendEnumerateFollowingList OnFriendEnumerateFollowingList;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnFriendsGetFollowerCount OnFriendsGetFollowerCount;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnFriendsIsFollowing OnFriendsIsFollowing;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnGameConnectedChatJoin OnGameConnectedChatJoin;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnGameConnectedChatLeave OnGameConnectedChatLeave;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnGameConnectedClanChatMsg OnGameConnectedClanChatMsg;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnGameConnectedFriendChatMsg OnGameConnectedFriendChatMsg;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnGameLobbyJoinRequested OnGameLobbyJoinRequested;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnGameOverlayActivated OnGameOverlayActivated;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnGameRichPresenceJoinRequested OnGameRichPresenceJoinRequested;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnGameServerChangeRequested OnGameServerChangeRequested;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnJoinClanChatRoomCompletionResult OnJoinClanChatRoomCompletionResult;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnPersonaStateChange OnPersonaStateChange;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnSetPersonaNameResponse OnSetPersonaNameResponse;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit | Friends | Response")
	FOnAvatarImageLoaded OnAvatarImageLoaded;
private:

#if (WITH_ENGINE_STEAM && ONLINESUBSYSTEMSTEAM_PACKAGE) || (WITH_STEAMKIT && !WITH_ENGINE_STEAM)
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnAvatarImageLoadedCallback, AvatarImageLoaded_t, m_CallbackAvatarImageLoaded);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnClanOfficerListCallback, ClanOfficerListResponse_t, m_CallbackClanOfficerList);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnFriendRichPresenceUpdateCallback, FriendRichPresenceUpdate_t, m_CallbackFriendRichPresenceUpdate);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnDownloadClanActivityCountsResultCallback, DownloadClanActivityCountsResult_t, m_CallbackDownloadClanActivityCountsResult);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnFriendEnumerateFollowingListCallback, FriendsEnumerateFollowingList_t, m_CallbackFriendEnumerateFollowingList);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnFriendsGetFollowerCountCallback, FriendsGetFollowerCount_t, m_CallbackFriendsGetFollowerCount);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnFriendsIsFollowingCallback, FriendsIsFollowing_t, m_CallbackFriendsIsFollowing);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnGameConnectedChatJoinCallback, GameConnectedChatJoin_t, m_CallbackGameConnectedChatJoin);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnGameConnectedChatLeaveCallback, GameConnectedChatLeave_t, m_CallbackGameConnectedChatLeave);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnGameConnectedClanChatMsgCallback, GameConnectedClanChatMsg_t, m_CallbackGameConnectedClanChatMsg);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnGameConnectedFriendChatMsgCallback, GameConnectedFriendChatMsg_t, m_CallbackGameConnectedFriendChatMsg);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnGameLobbyJoinRequestedCallback, GameLobbyJoinRequested_t, m_CallbackGameLobbyJoinRequested);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnGameOverlayActivatedCallback, GameOverlayActivated_t, m_CallbackGameOverlayActivated);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnGameRichPresenceJoinRequestedCallback, GameRichPresenceJoinRequested_t, m_CallbackGameRichPresenceJoinRequested);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnGameServerChangeRequestedCallback, GameServerChangeRequested_t, m_CallbackGameServerChangeRequested);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnJoinClanChatRoomCompletionResultCallback, JoinClanChatRoomCompletionResult_t, m_CallbackJoinClanChatRoomCompletionResult);
	STEAM_CALLBACK_MANUAL(USIK_FriendsSubsystem, OnPersonaStateChangeCallback, PersonaStateChange_t, m_CallbackPersonaStateChange);

#endif
};

