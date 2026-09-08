

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_LobbySubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_Lobby_OnJoinLobbyAcceptedCallback, FEIK_ProductUserId, LocalUserId, const FEIK_UI_EventId&, UiEventId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_Lobby_OnLeaveLobbyRequestedCallback, FEIK_ProductUserId, LocalUserId, const FEIK_LobbyId&, LobbyId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_Lobby_OnLobbyInviteAcceptedCallback, FEIK_ProductUserId, LocalUserId, FEIK_ProductUserId, TargetUserId, const FEIK_LobbyId&, LobbyId, const FString&, InviteId);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_Lobby_OnLobbyInviteReceivedCallback, FEIK_ProductUserId, LocalUserId, FEIK_ProductUserId, TargetUserId, const FString&, InviteId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_Lobby_OnLobbyInviteRejectedCallback, FEIK_ProductUserId, LocalUserId, FEIK_ProductUserId, TargetUserId, const FEIK_LobbyId&, LobbyId, const FString&, InviteId);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_Lobby_OnLobbyMemberStatusReceivedCallback, FEIK_ProductUserId, TargetUserId, const FEIK_LobbyId&, LobbyId, const TEnumAsByte<EEIK_ELobbyMemberStatus>&, CurrentStatus);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_Lobby_OnLobbyMemberUpdateReceivedCallback, FEIK_ProductUserId, TargetUserId, const FEIK_LobbyId&, LobbyId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEIK_Lobby_OnLobbyUpdateReceivedCallback, const FEIK_LobbyId&, LobbyId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_Lobby_OnRTCRoomConnectionChangedCallback, const FEIK_LobbyId&, LobbyId, const FEIK_ProductUserId&, LocalUserId, bool, bIsConnected, const TEnumAsByte<EEIK_Result>&, DisconnectReason);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEIK_Lobby_OnSendLobbyNativeInviteCallback, FEIK_UI_EventId, UiEventId, FEIK_ProductUserId, LocalUserId, const FString&, TargetNativeAccountType, const FString&, TargetUserNativeAccountId, const FEIK_LobbyId&, LobbyId);

UCLASS(DisplayName="Lobby Interface", meta=(DisplayName="Lobby Interface"))
class ONLINESUBSYSTEMEIK_API UEIK_LobbySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FEIK_Lobby_OnJoinLobbyAcceptedCallback OnJoinLobbyAccepted;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyJoinLobbyAccepted")
	FEIK_NotificationId EIK_Lobby_AddNotifyJoinLobbyAccepted(FEIK_Lobby_OnJoinLobbyAcceptedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyJoinLobbyAccepted")
	void EIK_Lobby_RemoveNotifyJoinLobbyAccepted(FEIK_NotificationId InId);

	FEIK_Lobby_OnLeaveLobbyRequestedCallback OnLeaveLobbyRequested;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLeaveLobbyRequested")
	FEIK_NotificationId EIK_Lobby_AddNotifyLeaveLobbyRequested(FEIK_Lobby_OnLeaveLobbyRequestedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLeaveLobbyRequested")
	void EIK_Lobby_RemoveNotifyLeaveLobbyRequested(FEIK_NotificationId InId);

	FEIK_Lobby_OnLobbyInviteAcceptedCallback OnLobbyInviteAccepted;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyInviteAccepted")
	FEIK_NotificationId EIK_Lobby_AddNotifyLobbyInviteAccepted(FEIK_Lobby_OnLobbyInviteAcceptedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyInviteAccepted")
	void EIK_Lobby_RemoveNotifyLobbyInviteAccepted(FEIK_NotificationId InId);

	FEIK_Lobby_OnLobbyInviteReceivedCallback OnLobbyInviteReceived;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyInviteReceived")
	FEIK_NotificationId EIK_Lobby_AddNotifyLobbyInviteReceived(FEIK_Lobby_OnLobbyInviteReceivedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyInviteReceived")
	void EIK_Lobby_RemoveNotifyLobbyInviteReceived(FEIK_NotificationId InId);

	FEIK_Lobby_OnLobbyInviteRejectedCallback OnLobbyInviteRejected;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyInviteRejected")
	FEIK_NotificationId EIK_Lobby_AddNotifyLobbyInviteRejected(FEIK_Lobby_OnLobbyInviteRejectedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyInviteRejected")
	void EIK_Lobby_RemoveNotifyLobbyInviteRejected(FEIK_NotificationId InId);

	FEIK_Lobby_OnLobbyMemberStatusReceivedCallback OnLobbyMemberStatusReceived;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyMemberStatusReceived")
	FEIK_NotificationId EIK_Lobby_AddNotifyLobbyMemberStatusReceived(FEIK_Lobby_OnLobbyMemberStatusReceivedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived")
	void EIK_Lobby_RemoveNotifyLobbyMemberStatusReceived(FEIK_NotificationId InId);

	FEIK_Lobby_OnLobbyMemberUpdateReceivedCallback OnLobbyMemberUpdateReceived;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyMemberUpdateReceived")
	FEIK_NotificationId EIK_Lobby_AddNotifyLobbyMemberUpdateReceived(FEIK_Lobby_OnLobbyMemberUpdateReceivedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived")
	void EIK_Lobby_RemoveNotifyLobbyMemberUpdateReceived(FEIK_NotificationId InId);

	FEIK_Lobby_OnLobbyUpdateReceivedCallback OnLobbyUpdateReceived;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyUpdateReceived")
	FEIK_NotificationId EIK_Lobby_AddNotifyLobbyUpdateReceived(FEIK_Lobby_OnLobbyUpdateReceivedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyUpdateReceived")
	void EIK_Lobby_RemoveNotifyLobbyUpdateReceived(FEIK_NotificationId InId);

	FEIK_Lobby_OnRTCRoomConnectionChangedCallback OnRTCRoomConnectionChanged;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyRTCRoomConnectionChanged")
	FEIK_NotificationId EIK_Lobby_AddNotifyRTCRoomConnectionChanged(FEIK_Lobby_OnRTCRoomConnectionChangedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyRTCRoomConnectionChanged")
	void EIK_Lobby_RemoveNotifyRTCRoomConnectionChanged(FEIK_NotificationId InId);

	FEIK_Lobby_OnSendLobbyNativeInviteCallback OnSendLobbyNativeInvite;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifySendLobbyNativeInviteRequested")
	FEIK_NotificationId EIK_Lobby_AddNotifySendLobbyNativeInviteRequested(FEIK_Lobby_OnSendLobbyNativeInviteCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifySendLobbyNativeInviteRequested")
	void EIK_Lobby_RemoveNotifySendLobbyNativeInviteRequested(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_Attribute_Release")
	void EIK_Lobby_Attribute_Release(const FEIK_Lobby_Attribute& Attribute);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_CopyLobbyDetailsHandle")
	TEnumAsByte<EEIK_Result> EIK_Lobby_CopyLobbyDetailsHandle(FEIK_LobbyId LobbyId, FEIK_ProductUserId LocalUserId, FEIK_HLobbyDetails& OutLobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_CopyLobbyDetailsHandleByInviteId")
	TEnumAsByte<EEIK_Result> EIK_Lobby_CopyLobbyDetailsHandleByInviteId(FString InviteId, FEIK_HLobbyDetails& OutLobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_CopyLobbyDetailsHandleByUiEventId")
	TEnumAsByte<EEIK_Result> EIK_Lobby_CopyLobbyDetailsHandleByUiEventId(const FEIK_UI_EventId& UiEventId, FEIK_HLobbyDetails& OutLobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_CreateLobbySearch")
	TEnumAsByte<EEIK_Result> EIK_Lobby_CreateLobbySearch(int32 MaxResults, FEIK_HLobbySearch& OutLobbySearchHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_GetConnectString")
	TEnumAsByte<EEIK_Result> EIK_Lobby_GetConnectString(FEIK_ProductUserId LocalUserId, FEIK_LobbyId LobbyId, FString& OutConnectString);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_GetInviteCount")
	int32 EIK_Lobby_GetInviteCount(FEIK_ProductUserId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_GetInviteIdByIndex")
	TEnumAsByte<EEIK_Result> EIK_Lobby_GetInviteIdByIndex(FEIK_ProductUserId LocalUserId, int32 Index, FString& OutInviteId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_GetRTCRoomName")
	TEnumAsByte<EEIK_Result> EIK_Lobby_GetRTCRoomName(FEIK_ProductUserId LocalUserId, FEIK_LobbyId LobbyId, FString& OutRTCRoomName);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_IsRTCRoomConnected")
	TEnumAsByte<EEIK_Result> EIK_Lobby_IsRTCRoomConnected(FEIK_ProductUserId LocalUserId, FEIK_LobbyId LobbyId, bool& bOutIsConnected);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_ParseLobbyIdFromConnectString")
	TEnumAsByte<EEIK_Result> EIK_Lobby_ParseLobbyIdFromConnectString(FString ConnectString, FEIK_LobbyId& OutLobbyId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_UpdateLobbyModification")
	TEnumAsByte<EEIK_Result> EIK_Lobby_UpdateLobbyModification(FEIK_ProductUserId LocalUserId, FEIK_LobbyId LobbyId, FEIK_HLobbyModification& OutLobbyModificationHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyAttributeByIndex")
	TEnumAsByte<EEIK_Result> EIK_LobbyDetails_CopyAttributeByIndex(FEIK_HLobbyDetails LobbyDetailsHandle, int32 AttrIndex, FEIK_Lobby_Attribute& OutAttribute);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyAttributeByKey")
	TEnumAsByte<EEIK_Result> EIK_LobbyDetails_CopyAttributeByKey(FEIK_HLobbyDetails LobbyDetailsHandle, const FString& AttrKey, FEIK_Lobby_Attribute& OutAttribute);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyInfo")
	TEnumAsByte<EEIK_Result> EIK_LobbyDetails_CopyInfo(FEIK_HLobbyDetails LobbyDetailsHandle, FEIK_LobbyDetailsInfo& OutLobbyDetailsInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyMemberAttributeByIndex")
	TEnumAsByte<EEIK_Result> EIK_LobbyDetails_CopyMemberAttributeByIndex(FEIK_HLobbyDetails LobbyDetailsHandle, FEIK_ProductUserId TargetUserId, int32 AttrIndex, FEIK_Lobby_Attribute& OutAttribute);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyMemberAttributeByKey")
	TEnumAsByte<EEIK_Result> EIK_LobbyDetails_CopyMemberAttributeByKey(FEIK_HLobbyDetails LobbyDetailsHandle, FEIK_ProductUserId TargetUserId, const FString& AttrKey, FEIK_Lobby_Attribute& OutAttribute);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyMemberInfo")
	TEnumAsByte<EEIK_Result> EIK_LobbyDetails_CopyMemberInfo(FEIK_HLobbyDetails LobbyDetailsHandle, FEIK_ProductUserId TargetUserId, FEIK_LobbyDetails_MemberInfo& OutMemberInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetAttributeCount")
	int32 EIK_LobbyDetails_GetAttributeCount(FEIK_HLobbyDetails LobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetLobbyOwner")
	FEIK_ProductUserId EIK_LobbyDetails_GetLobbyOwner(FEIK_HLobbyDetails LobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetMemberAttributeCount")
	int32 EIK_LobbyDetails_GetMemberAttributeCount(FEIK_HLobbyDetails LobbyDetailsHandle, FEIK_ProductUserId TargetUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetMemberByIndex")
	FEIK_ProductUserId EIK_LobbyDetails_GetMemberByIndex(FEIK_HLobbyDetails LobbyDetailsHandle, int32 MemberIndex);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetMemberCount")
	int32 EIK_LobbyDetails_GetMemberCount(FEIK_HLobbyDetails LobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_Info_Release")
	void EIK_LobbyDetails_Info_Release(FEIK_LobbyDetailsInfo& LobbyDetailsInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_MemberInfo_Release")
	void EIK_LobbyDetails_MemberInfo_Release(FEIK_LobbyDetails_MemberInfo& MemberInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_Release")
	void EIK_LobbyDetails_Release(FEIK_HLobbyDetails LobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_AddAttribute")
	TEnumAsByte<EEIK_Result> EIK_LobbyModification_AddAttribute(FEIK_HLobbyModification LobbyModificationHandle, const FEIK_Lobby_AttributeData& Attribute, const TEnumAsByte<EEIK_ELobbyAttributeVisibility>& Visibility);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_AddMemberAttribute")
	TEnumAsByte<EEIK_Result> EIK_LobbyModification_AddMemberAttribute(FEIK_HLobbyModification LobbyModificationHandle, const FEIK_Lobby_AttributeData& Attribute, const TEnumAsByte<EEIK_ELobbyAttributeVisibility>& Visibility);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_Release")
	void EIK_LobbyModification_Release(FEIK_HLobbyModification LobbyModificationHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_RemoveAttribute")
	TEnumAsByte<EEIK_Result> EIK_LobbyModification_RemoveAttribute(FEIK_HLobbyModification LobbyModificationHandle, const FString& Options);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_RemoveMemberAttribute")
	TEnumAsByte<EEIK_Result> EIK_LobbyModification_RemoveMemberAttribute(FEIK_HLobbyModification LobbyModificationHandle, const FString& Options);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetAllowedPlatformIds")
	TEnumAsByte<EEIK_Result> EIK_LobbyModification_SetAllowedPlatformIds(FEIK_HLobbyModification LobbyModificationHandle, const TArray<int32>& Options);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetBucketId")
	TEnumAsByte<EEIK_Result> EIK_LobbyModification_SetBucketId(FEIK_HLobbyModification LobbyModificationHandle, const FString& Options);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetInvitesAllowed")
	TEnumAsByte<EEIK_Result> EIK_LobbyModification_SetInvitesAllowed(FEIK_HLobbyModification LobbyModificationHandle, const bool& Options);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetMaxMembers")
	TEnumAsByte<EEIK_Result> EIK_LobbyModification_SetMaxMembers(FEIK_HLobbyModification LobbyModificationHandle, const int32& Options);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetPermissionLevel")
	TEnumAsByte<EEIK_Result> EIK_LobbyModification_SetPermissionLevel(FEIK_HLobbyModification LobbyModificationHandle, const TEnumAsByte<EEIK_ELobbyPermissionLevel>& Options);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_CopySearchResultByIndex")
	TEnumAsByte<EEIK_Result> EIK_LobbySearch_CopySearchResultByIndex(FEIK_HLobbySearch LobbySearchHandle, int32 LobbyIndex, FEIK_HLobbyDetails& OutLobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_GetSearchResultCount")
	int32 EIK_LobbySearch_GetSearchResultCount(FEIK_HLobbySearch LobbySearchHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_Release")
	void EIK_LobbySearch_Release(FEIK_HLobbySearch LobbySearchHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_RemoveParameter")
	TEnumAsByte<EEIK_Result> EIK_LobbySearch_RemoveParameter(FEIK_HLobbySearch LobbySearchHandle, const FString& Key, const TEnumAsByte<EEIK_EComparisonOp>& ComparisonOp);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_SetLobbyId")
	TEnumAsByte<EEIK_Result> EIK_LobbySearch_SetLobbyId(FEIK_HLobbySearch LobbySearchHandle, const FEIK_LobbyId& Options);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_SetMaxResults")
	TEnumAsByte<EEIK_Result> EIK_LobbySearch_SetMaxResults(FEIK_HLobbySearch LobbySearchHandle, const int32 MaxResults);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_SetParameter")
	TEnumAsByte<EEIK_Result> EIK_LobbySearch_SetParameter(FEIK_HLobbySearch LobbySearchHandle, const FEIK_Lobby_AttributeData& Parameter, const TEnumAsByte<EEIK_EComparisonOp>& ComparisonOp);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_SetTargetUserId")
	TEnumAsByte<EEIK_Result> EIK_LobbySearch_SetTargetUserId(FEIK_HLobbySearch LobbySearchHandle, FEIK_ProductUserId Options);
};
