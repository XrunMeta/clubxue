

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "Serialization/BulkData.h"
#include "Runtime/Core/Public/PixelFormat.h"
#include "Engine/Texture.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Async/Async.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SIK_FriendsLibrary.generated.h"

USTRUCT(BlueprintType)
struct FSIK_FriendGameInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Steam Integration Kit || SDK Functions || Friends")
	FSIK_SteamId LobbySteamId;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Integration Kit || SDK Functions || Friends")
	FSIK_GameID GameId;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Integration Kit || SDK Functions || Friends")
	int32 IP;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Integration Kit || SDK Functions || Friends")
	int32 GamePort;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Integration Kit || SDK Functions || Friends")
	int32 QueryPort;

	FSIK_FriendGameInfo()
	{
		LobbySteamId = 0;
		GameId = FSIK_GameID();
		IP = 0;
		GamePort = 0;
		QueryPort = 0;
	};
#if (WITH_ENGINE_STEAM && ONLINESUBSYSTEMSTEAM_PACKAGE) || (WITH_STEAMKIT && !WITH_ENGINE_STEAM)
	FSIK_FriendGameInfo(FriendGameInfo_t FriendGameInfo)
	{
		LobbySteamId = FriendGameInfo.m_steamIDLobby;
		GameId = FriendGameInfo.m_gameID;
		IP = FriendGameInfo.m_unGameIP;
		GamePort = FriendGameInfo.m_usGamePort;
		QueryPort = FriendGameInfo.m_usQueryPort;
	};
#endif

};

UENUM(BlueprintType)
enum ESIK_PersonaState
{
	PersonaStateOffline = 0 UMETA(DisplayName = "Offline"),
	PersonaStateOnline = 1 UMETA(DisplayName = "Online"),
	PersonaStateBusy = 2 UMETA(DisplayName = "Busy"),
	PersonaStateAway = 3 UMETA(DisplayName = "Away"),
	PersonaStateSnooze = 4 UMETA(DisplayName = "Snooze"),
	PersonaStateLookingToTrade = 5 UMETA(DisplayName = "Looking To Trade"),
	PersonaStateLookingToPlay = 6 UMETA(DisplayName = "Looking To Play"),
	PersonaStateMax = 7 UMETA(DisplayName = "Max")
};

UENUM(BlueprintType)
enum ESIK_FriendRelationship
{
	FriendRelationshipNone = 0 UMETA(DisplayName = "None"),
	FriendRelationshipBlocked = 1 UMETA(DisplayName = "Blocked"),
	FriendRelationshipRequestRecipient = 2 UMETA(DisplayName = "Request Recipient"),
	FriendRelationshipFriend = 3 UMETA(DisplayName = "Friend"),
	FriendRelationshipRequestInitiator = 4 UMETA(DisplayName = "Request Initiator"),
	FriendRelationshipIgnored = 5 UMETA(DisplayName = "Ignored"),
	FriendRelationshipIgnoredFriend = 6 UMETA(DisplayName = "Ignored Friend"),
	FriendRelationshipSuggested = 7 UMETA(DisplayName = "Suggested"),
	FriendRelationshipMax = 8 UMETA(DisplayName = "Max")
};
UCLASS()
class STEAMINTEGRATIONKIT_API USIK_FriendsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, DisplayName = "Activate Steam Game Overlay", meta=(Keywords="ActivateGameOverlay"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void ActivateGameOverlay(const FString& DialogToOpen);

	UFUNCTION(BlueprintCallable, DisplayName = "Activate Steam Game Overlay Invite Dialog", meta=(Keywords="ActivateGameOverlayInviteDialog"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void ActivateGameOverlayInviteDialog(const int64& SteamIdLobby);

	UFUNCTION(BlueprintCallable, DisplayName = "Activate Steam Game Overlay To Store", meta=(Keywords="ActivateGameOverlayToStore"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void ActivateGameOverlayToStore(const FSIK_AppId& AppID, bool bAddToCartAndShow = true);

	UFUNCTION(BlueprintCallable, DisplayName = "Activate Steam Game Overlay To User", meta=(Keywords="ActivateGameOverlayToUser"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void ActivateGameOverlayToUser(const FString& Dialog, int64 SteamIdUser);

	UFUNCTION(BlueprintCallable, DisplayName = "Activate Steam Game Overlay To WebPage", meta=(Keywords="ActivateGameOverlayToWebPage"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void ActivateGameOverlayToWebPage(const FString& URL, bool bUseModal = false);

	UFUNCTION(BlueprintCallable, DisplayName = "Clear Steam Rich Presence", meta=(Keywords="ClearRichPresence"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void ClearRichPresence();

	UFUNCTION(BlueprintCallable, DisplayName = "Close Steam Clan Chat Window In Steam", meta=(Keywords="CloseClanChatWindow"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool CloseClanChatWindowInSteam(int64 SteamIdClanChat);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Chat Member By Index", meta=(Keywords="GetChatMemberByIndex"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FSIK_SteamId GetChatMemberByIndex(FSIK_SteamId SteamIdClan, int32 MemberIndex);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan Activity Counts", meta=(Keywords="GetClanActivityCounts"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool GetClanActivityCounts(FSIK_SteamId SteamIdClan, int32& Online, int32& InGame, int32& Chatting);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan By Index", meta=(Keywords="GetClanByIndex"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FSIK_SteamId GetClanByIndex(int32 ClanIndex);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan Chat Member Count", meta=(Keywords="GetClanChatMemberCount"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetClanChatMemberCount(FSIK_SteamId SteamIdClan);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan Chat Message", meta=(Keywords="GetClanChatMessage"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetClanChatMessage(FSIK_SteamId SteamIdClan, int32 MessageIndex, const TArray<uint8>& Text, FSIK_SteamId& SteamIdUser, FString& ChatEntryType);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan Count", meta=(Keywords="GetClanCount"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetClanCount();

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan Name", meta=(Keywords="GetClanName"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetClanName(FSIK_SteamId SteamIdClan);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan Officer By Index", meta=(Keywords="GetClanOfficerByIndex"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FSIK_SteamId GetClanOfficerByIndex(FSIK_SteamId SteamIdClan, int32 OfficerIndex);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan Officer Count", meta=(Keywords="GetClanOfficerCount"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetClanOfficerCount(FSIK_SteamId SteamIdClan);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan Owner", meta=(Keywords="GetClanOwner"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FSIK_SteamId GetClanOwner(FSIK_SteamId SteamIdClan);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Clan Tag", meta=(Keywords="GetClanTag"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetClanTag(FSIK_SteamId SteamIdClan);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Coplay Friend", meta=(Keywords="GetCoplayFriend"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FSIK_SteamId GetCoplayFriend(int32 CoplayFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Coplay Friend Count", meta=(Keywords="GetCoplayFriendCount"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetCoplayFriendCount();

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend By Index", meta=(Keywords="GetFriendByIndex"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FSIK_SteamId GetFriendByIndex(int32 FriendIndex, TArray<TEnumAsByte<ESIK_FriendFlags>> FriendFlags);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Coplay Game", meta=(Keywords="GetFriendCoplayGame"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FSIK_AppId GetFriendCoplayGame(FSIK_SteamId SteamIdFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Coplay Time", meta=(Keywords="GetFriendCoplayTime"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FDateTime GetFriendCoplayTime(FSIK_SteamId SteamIdFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Count", meta=(Keywords="GetFriendCount"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetFriendCount(TArray<TEnumAsByte<ESIK_FriendFlags>> FriendFlags);

	UFUNCTION(BlueprintCallable, DisplayName ="Get Steam Friend Count From Source", meta=(Keywords="GetFriendCountFromSource"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetFriendCountFromSource(FSIK_SteamId SteamIdSource);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend From Source By Index", meta=(Keywords="GetFriendFromSourceByIndex"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FSIK_SteamId GetFriendFromSourceByIndex(FSIK_SteamId SteamIdSource, int32 FriendIndex);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Game Played", meta=(Keywords="GetFriendGamePlayed"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool GetFriendGamePlayed(FSIK_SteamId SteamIdFriend, FSIK_FriendGameInfo& GameInfo);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Message", meta=(Keywords="GetFriendMessage"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetFriendMessage(FSIK_SteamId SteamIdFriend, int32 MessageIndex, TArray<uint8>& Text, FString& ChatEntryType);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Persona Name", meta=(Keywords="GetFriendPersonaName"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetFriendPersonaName(FSIK_SteamId SteamIdFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Persona Name History", meta=(Keywords="GetFriendPersonaNameHistory"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetFriendPersonaNameHistory(FSIK_SteamId SteamIdFriend, int32 PersonaNameIndex);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Persona State", meta=(Keywords="GetFriendPersonaState"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static TEnumAsByte<ESIK_PersonaState> GetFriendPersonaState(FSIK_SteamId SteamIdFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Relationship", meta=(Keywords="GetFriendRelationship"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static TEnumAsByte<ESIK_FriendRelationship> GetFriendRelationship(FSIK_SteamId SteamIdFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Rich Presence", meta=(Keywords="GetFriendRichPresence"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetFriendRichPresence(FSIK_SteamId SteamIdFriend, const FString& Key);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Rich Presence Key By Index", meta=(Keywords="GetFriendRichPresenceKeyByIndex"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetFriendRichPresenceKeyByIndex(FSIK_SteamId SteamIdFriend, int32 KeyIndex);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Rich Presence Key Count", meta=(Keywords="GetFriendRichPresenceKeyCount"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetFriendRichPresenceKeyCount(FSIK_SteamId SteamIdFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friends Group Count", meta=(Keywords="GetFriendsGroupCount"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetFriendGroupCount();

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friends Group ID By Index", meta=(Keywords="GetFriendsGroupIDByIndex"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FSIK_FriendsGroupID GetFriendGroupIDByIndex(int32 GroupIndex);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friends Group Members Count", meta=(Keywords="GetFriendsGroupMembersCount"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetFriendGroupMembersCount(FSIK_FriendsGroupID FriendsGroupID);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friends Group Members List", meta=(Keywords="GetFriendsGroupMembersList"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static TArray<FSIK_SteamId> GetFriendsGroupMembersList(FSIK_FriendsGroupID FriendsGroupID, int32 MembersCount);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friends Group Name", meta=(Keywords="GetFriendsGroupName"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetFriendsGroupName(FSIK_FriendsGroupID FriendsGroupID);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Friend Steam Level", meta=(Keywords="GetFriendSteamLevel"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetFriendSteamLevel(FSIK_SteamId SteamIdFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Large Friend Avatar", meta=(Keywords="GetLargeFriendAvatar"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static UTexture2D* GetLargeFriendAvatar(FSIK_SteamId SteamIdFriend, int32& Avatar);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Medium Friend Avatar", meta=(Keywords="GetMediumFriendAvatar"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static UTexture2D* GetMediumFriendAvatar(FSIK_SteamId SteamIdFriend, int32& Avatar);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Persona Name", meta=(Keywords="GetPersonaName"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetPersonaName();

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Persona State", meta=(Keywords="GetPersonaState"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static TEnumAsByte<ESIK_PersonaState> GetPersonaState();

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Player Nickname", meta=(Keywords="GetPlayerNickname"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetPlayerNickname(FSIK_SteamId SteamIdPlayer);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam Small Friend Avatar", meta=(Keywords="GetSmallFriendAvatar"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static UTexture2D* GetSmallFriendAvatar(FSIK_SteamId SteamIdFriend, int32& Avatar);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Steam User Restrictions", meta=(Keywords="GetUserRestrictions"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetUserRestrictions();

	UFUNCTION(BlueprintCallable, DisplayName = "Has Steam Friend", meta=(Keywords="HasFriend"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool HasFriend(FSIK_SteamId SteamIdFriend, TArray<TEnumAsByte<ESIK_FriendFlags>> FriendFlags);

	UFUNCTION(BlueprintCallable, DisplayName = "Invite Steam User To Game", meta=(Keywords="InviteUserToGame"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool InviteUserToGame(FSIK_SteamId SteamIdFriend, const FString& ConnectString);

	UFUNCTION(BlueprintCallable, DisplayName = "Is Steam Clan Chat Admin", meta=(Keywords="IsClanChatAdmin"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool IsClanChatAdmin(FSIK_SteamId SteamIdClanChat, FSIK_SteamId SteamIdUser);

	UFUNCTION(BlueprintCallable, DisplayName = "Is Steam Clan Public", meta=(Keywords="IsClanPublic"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool IsClanPublic(FSIK_SteamId SteamIdClan);

	UFUNCTION(BlueprintCallable, DisplayName = "Is Steam Clan Official Game Group", meta=(Keywords="IsClanOfficialGameGroup"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool IsClanOfficialGameGroup(FSIK_SteamId SteamIdClan);

	UFUNCTION(BlueprintCallable, DisplayName = "Is Steam Clan Chat Window Open In Steam", meta=(Keywords="IsClanChatWindowOpen"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool IsClanChatWindowOpenInSteam(FSIK_SteamId SteamIdClanChat);

	UFUNCTION(BlueprintCallable, DisplayName = "Is Steam Following", meta=(Keywords="IsFollowing"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool IsFollowing(FSIK_SteamId SteamIdFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Is Steam User In Source", meta=(Keywords="IsUserInSource"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool IsUserInSource(FSIK_SteamId SteamIdFriend, FSIK_SteamId SteamIdSource);

	UFUNCTION(BlueprintCallable, DisplayName = "Leave Steam Clan Chat Room", meta=(Keywords="LeaveClanChatRoom"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool LeaveClanChatRoom(FSIK_SteamId SteamIdClanChat);

	UFUNCTION(BlueprintCallable, DisplayName = "Reply To Steam Friend Message", meta=(Keywords="ReplyToFriendMessage"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool ReplyToFriendMessage(FSIK_SteamId SteamIdFriend, const FString& Text);

	UFUNCTION(BlueprintCallable, DisplayName = "Request Steam Friend Rich Presence", meta=(Keywords="RequestFriendRichPresence"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void RequestFriendRichPresence(FSIK_SteamId SteamIdFriend);

	UFUNCTION(BlueprintCallable, DisplayName = "Request Steam User Information", meta=(Keywords="RequestUserInfo"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool RequestUserInfo(FSIK_SteamId SteamIdUser, bool bRequireNameOnly);

	UFUNCTION(BlueprintCallable, DisplayName = "Send Steam Clan Chat Message", meta=(Keywords="SendClanChatMessage"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool SendClanChatMessage(FSIK_SteamId SteamIdClanChat, const FString& Text);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Steam In Game Voice Speaking", meta=(Keywords="SetInGameVoiceSpeaking"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void SetInGameVoiceSpeaking(FSIK_SteamId SteamIdFriend, bool bSpeaking);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Steam Listen For Friend Message", meta=(Keywords="SetListenForFriendMessage"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void SetListenForFriendMessage(bool bInterceptEnabled = true);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Steam Played With", meta=(Keywords="SetPlayedWith"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static void SetPlayedWith(FSIK_SteamId SteamIdUser);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Steam Rich Presence", meta=(Keywords="SetRichPresence"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool SetRichPresence(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, DisplayName = "B Has Equipped Profile Item", meta=(Keywords="BHasEquippedProfileItem"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static bool BHasEquippedProfileItem(FSIK_SteamId SteamIdUser, TEnumAsByte<ESIK_ECommunityProfileItemType> ItemType);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Profile Item Property String", meta=(Keywords="GetProfileItemPropertyString"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static FString GetProfileItemPropertyString(FSIK_SteamId SteamIdUser, TEnumAsByte<ESIK_ECommunityProfileItemType> ItemType, TEnumAsByte<ESIK_ECommunityProfileItemProperty> Property);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Profile Item Property Uint", meta=(Keywords="GetProfileItemPropertyUint"), Category = "Steam Integration Kit || SDK Functions || Friends")
	static int32 GetProfileItemPropertyUint(FSIK_SteamId SteamIdUser, TEnumAsByte<ESIK_ECommunityProfileItemType> ItemType, TEnumAsByte<ESIK_ECommunityProfileItemProperty> Property);
};
