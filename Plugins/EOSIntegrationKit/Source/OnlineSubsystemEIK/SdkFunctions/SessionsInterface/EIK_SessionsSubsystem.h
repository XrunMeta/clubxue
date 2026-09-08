

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_sessions.h"
#include "eos_sessions_types.h"
THIRD_PARTY_INCLUDES_END
#include "EIK_SessionsSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_Sessions_OnJoinSessionAcceptedCallback, const FEIK_ProductUserId&, LocalUserId, const FEIK_UI_EventId&, UIEventId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_Sessions_OnLeaveSessionRequestedCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, SessionName);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEIK_Sessions_OnSendSessionInviteCallback, const FEIK_ProductUserId&, LocalUserId, const FEIK_UI_EventId&, UIEventId, const FString&, TargetNativeAccountType, const FString&, TargetUserNativeAccountId, const FString&, SessionId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_Sessions_OnSessionInviteAcceptedCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, SessionId, const FEIK_ProductUserId&, TargetUserId, const FString&, InviteId);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_Sessions_OnSessionInviteReceivedCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, InviteId, const FEIK_ProductUserId&, TargetUserId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_Sessions_OnSessionInviteRejectedCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, SessionId, const FEIK_ProductUserId&, TargetUserId, const FString&, InviteId);

USTRUCT(BlueprintType)
struct FEIK_Sessions_CreateSessionModificationOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | Sessions Interface")
	FString SessionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | Sessions Interface")
	FString BucketId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | Sessions Interface")
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | Sessions Interface")
	FEIK_ProductUserId LocalUserId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | Sessions Interface")
	bool bPresenceEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | Sessions Interface")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | Sessions Interface")
	bool bSanctionsEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | Sessions Interface")
	TArray<int32> AllowedPlatformIds;

	FEIK_Sessions_CreateSessionModificationOptions()
	{
		SessionName = "";
		BucketId = "";
		MaxPlayers = 0;
		LocalUserId = FEIK_ProductUserId();
		bPresenceEnabled = false;
		SessionId = "";
		bSanctionsEnabled = false;
	}
	EOS_Sessions_CreateSessionModificationOptions ToEosStruct()
	{
		EOS_Sessions_CreateSessionModificationOptions EosStruct;
		EosStruct.ApiVersion = EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST;
		auto SessionNameAnsi = StringCast<ANSICHAR>(*SessionName);
		CachedSessionNameAnsi.SetNumUninitialized(SessionNameAnsi.Length() + 1);
		FMemory::Memcpy(CachedSessionNameAnsi.GetData(), SessionNameAnsi.Get(), SessionNameAnsi.Length() + 1);
		EosStruct.SessionName = CachedSessionNameAnsi.GetData();
		auto BucketIdAnsi = StringCast<ANSICHAR>(*BucketId);
		CachedBucketIdAnsi.SetNumUninitialized(BucketIdAnsi.Length() + 1);
		FMemory::Memcpy(CachedBucketIdAnsi.GetData(), BucketIdAnsi.Get(), BucketIdAnsi.Length() + 1);
		EosStruct.BucketId = CachedBucketIdAnsi.GetData();
		EosStruct.MaxPlayers = MaxPlayers;
		EosStruct.LocalUserId = LocalUserId.GetValueAsEosType();
		EosStruct.bPresenceEnabled = bPresenceEnabled;
		auto SessionIdAnsi = StringCast<ANSICHAR>(*SessionId);
		CachedSessionIdAnsi.SetNumUninitialized(SessionIdAnsi.Length() + 1);
		FMemory::Memcpy(CachedSessionIdAnsi.GetData(), SessionIdAnsi.Get(), SessionIdAnsi.Length() + 1);
		EosStruct.SessionId = CachedSessionIdAnsi.GetData();
		EosStruct.bSanctionsEnabled = bSanctionsEnabled;
		EosStruct.AllowedPlatformIdsCount = AllowedPlatformIds.Num();
		uint32_t* AllowedPlatformIdsArray = new uint32_t[AllowedPlatformIds.Num()];
		for (int i = 0; i < AllowedPlatformIds.Num(); i++)
		{
			AllowedPlatformIdsArray[i] = AllowedPlatformIds[i];
		}
		EosStruct.AllowedPlatformIds = AllowedPlatformIdsArray;
		return EosStruct;
	}

	TArray<ANSICHAR> CachedSessionNameAnsi;
	TArray<ANSICHAR> CachedBucketIdAnsi;
	TArray<ANSICHAR> CachedSessionIdAnsi;
};
UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_SessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_ActiveSession_CopyInfo")
	static TEnumAsByte<EEIK_Result> EIK_ActiveSession_CopyInfo(FEIK_HActiveSession Handle, FEIK_ActiveSession_Info& OutActiveSessionInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_ActiveSession_GetRegisteredPlayerByIndex")
	static FEIK_ProductUserId EIK_ActiveSession_GetRegisteredPlayerByIndex(FEIK_HActiveSession Handle, int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_ActiveSession_Release")
	static void EIK_ActiveSession_Release(FEIK_HActiveSession Handle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionDetails_CopyInfo")
	static TEnumAsByte<EEIK_Result> EIK_SessionDetails_CopyInfo(FEIK_HSessionDetails Handle, FEIK_SessionDetails_Info& OutSessionInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionDetails_CopySessionAttributeByIndex")
	static TEnumAsByte<EEIK_Result> EIK_SessionDetails_CopySessionAttributeByIndex(FEIK_HSessionDetails Handle, int32 AttrIndex, FEIK_SessionDetails_Attribute& OutSessionAttribute);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionDetails_CopySessionAttributeByKey")
	static TEnumAsByte<EEIK_Result> EIK_SessionDetails_CopySessionAttributeByKey(FEIK_HSessionDetails Handle, const FString& AttrKey, FEIK_SessionDetails_Attribute& OutSessionAttribute);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionDetails_GetSessionAttributeCount")
	static int32 EIK_SessionDetails_GetSessionAttributeCount(FEIK_HSessionDetails Handle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_AddAttribute")
	static TEnumAsByte<EEIK_Result> EIK_SessionModification_AddAttribute(FEIK_HSessionModification Handle, FEIK_Sessions_AttributeData AttrData, TEnumAsByte<EIK_ESessionAttributeAdvertisementType> AdvertisementType);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_Release")
	static void EIK_SessionModification_Release(FEIK_HSessionModification Handle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_RemoveAttribute")
	static TEnumAsByte<EEIK_Result> EIK_SessionModification_RemoveAttribute(FEIK_HSessionModification Handle, const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetAllowedPlatformIds")
	static TEnumAsByte<EEIK_Result> EIK_SessionModification_SetAllowedPlatformIds(FEIK_HSessionModification Handle, const TArray<int32>& PlatformIds);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetBucketId")
	static TEnumAsByte<EEIK_Result> EIK_SessionModification_SetBucketId(FEIK_HSessionModification Handle, const FString& BucketId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetHostAddress")
	static TEnumAsByte<EEIK_Result> EIK_SessionModification_SetHostAddress(FEIK_HSessionModification Handle, const FString& HostAddress);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetInvitesAllowed")
	static TEnumAsByte<EEIK_Result> EIK_SessionModification_SetInvitesAllowed(FEIK_HSessionModification Handle, bool bInvitesAllowed);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetJoinInProgressAllowed")
	static TEnumAsByte<EEIK_Result> EIK_SessionModification_SetJoinInProgressAllowed(FEIK_HSessionModification Handle, bool bAllowJoinInProgress);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetMaxPlayers")
	static TEnumAsByte<EEIK_Result> EIK_SessionModification_SetMaxPlayers(FEIK_HSessionModification Handle, int32 MaxPlayers);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetPermissionLevel")
	static TEnumAsByte<EEIK_Result> EIK_SessionModification_SetPermissionLevel(FEIK_HSessionModification Handle, TEnumAsByte<EEIK_EOnlineSessionPermissionLevel> PermissionLevel);

	FEIK_Sessions_OnJoinSessionAcceptedCallback OnJoinSessionAcceptedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifyJoinSessionAccepted")
	FEIK_NotificationId EIK_Sessions_AddNotifyJoinSessionAccepted( const FEIK_Sessions_OnJoinSessionAcceptedCallback& Callback);

	FEIK_Sessions_OnLeaveSessionRequestedCallback OnLeaveSessionRequestedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifyLeaveSessionRequested")
	FEIK_NotificationId EIK_Sessions_AddNotifyLeaveSessionRequested( const FEIK_Sessions_OnLeaveSessionRequestedCallback& Callback);

	FEIK_Sessions_OnSendSessionInviteCallback OnSendSessionInviteCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifySendSessionNativeInviteRequested")
	FEIK_NotificationId EIK_Sessions_AddNotifySendSessionNativeInviteRequested( const FEIK_Sessions_OnSendSessionInviteCallback& Callback);

	FEIK_Sessions_OnSessionInviteAcceptedCallback OnSessionInviteAcceptedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifySessionInviteAccepted")
	FEIK_NotificationId EIK_Sessions_AddNotifySessionInviteAccepted( const FEIK_Sessions_OnSessionInviteAcceptedCallback& Callback);

	FEIK_Sessions_OnSessionInviteReceivedCallback OnSessionInviteReceivedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifySessionInviteReceived")
	FEIK_NotificationId EIK_Sessions_AddNotifySessionInviteReceived( const FEIK_Sessions_OnSessionInviteReceivedCallback& Callback);

	FEIK_Sessions_OnSessionInviteRejectedCallback OnSessionInviteRejectedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifySessionInviteRejected")
	FEIK_NotificationId EIK_Sessions_AddNotifySessionInviteRejected( const FEIK_Sessions_OnSessionInviteRejectedCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CopyActiveSessionHandle")
	static TEnumAsByte<EEIK_Result> EIK_Sessions_CopyActiveSessionHandle(FString SessionName, FEIK_HActiveSession& OutActiveSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CopySessionHandleByInviteId")
	static TEnumAsByte<EEIK_Result> EIK_Sessions_CopySessionHandleByInviteId(FString InviteId, FEIK_HSessionDetails& OutSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CopySessionHandleByUiEventId")
	static TEnumAsByte<EEIK_Result> EIK_Sessions_CopySessionHandleByUiEventId(FEIK_UI_EventId UiEventId, FEIK_HSessionDetails& OutSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CopySessionHandleForPresence")
	static TEnumAsByte<EEIK_Result> EIK_Sessions_CopySessionHandleForPresence(FEIK_ProductUserId LocalUserId, FEIK_HSessionDetails& OutSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CreateSessionModification")
	static TEnumAsByte<EEIK_Result> EIK_Sessions_CreateSessionModification(FEIK_Sessions_CreateSessionModificationOptions Options, FEIK_HSessionModification& OutSessionModificationHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CreateSessionSearch")
	static TEnumAsByte<EEIK_Result> EIK_Sessions_CreateSessionSearch(int32 MaxSearchResults, FEIK_HSessionSearch& OutSessionSearchHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_DumpSessionState")
	static void EIK_Sessions_DumpSessionState(FString SessionName);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_GetInviteCount")
	static int32 EIK_Sessions_GetInviteCount(FEIK_ProductUserId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_GetInviteIdByIndex")
	static FString EIK_Sessions_GetInviteIdByIndex(FEIK_ProductUserId LocalUserId, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_IsUserInSession")
	static TEnumAsByte<EEIK_Result> EIK_Sessions_IsUserInSession(FEIK_ProductUserId TargetUserId, FString SessionName);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifyJoinSessionAccepted")
	static void EIK_Sessions_RemoveNotifyJoinSessionAccepted(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifyLeaveSessionRequested")
	static void EIK_Sessions_RemoveNotifyLeaveSessionRequested(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifySendSessionNativeInviteRequested")
	static void EIK_Sessions_RemoveNotifySendSessionNativeInviteRequested(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifySessionInviteAccepted")
	static void EIK_Sessions_RemoveNotifySessionInviteAccepted(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifySessionInviteReceived")
	static void EIK_Sessions_RemoveNotifySessionInviteReceived(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifySessionInviteRejected")
	static void EIK_Sessions_RemoveNotifySessionInviteRejected(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_UpdateSessionModification")
	static TEnumAsByte<EEIK_Result> EIK_Sessions_UpdateSessionModification(FString SessionName, FEIK_HSessionModification& OutSessionModificationHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_CopySearchResultByIndex")
	static TEnumAsByte<EEIK_Result> EIK_SessionSearch_CopySearchResultByIndex(FEIK_HSessionSearch Handle, int32 SessionIndex, FEIK_HSessionDetails& OutSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_GetSearchResultCount")
	static int32 EIK_SessionSearch_GetSearchResultCount(FEIK_HSessionSearch Handle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_Release")
	static void EIK_SessionSearch_Release(FEIK_HSessionSearch Handle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_RemoveParameter")
	static TEnumAsByte<EEIK_Result> EIK_SessionSearch_RemoveParameter(FEIK_HSessionSearch Handle, const FString& Key, const TEnumAsByte<EEIK_EComparisonOp>& ComparisonOp);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_SetMaxResults")
	static TEnumAsByte<EEIK_Result> EIK_SessionSearch_SetMaxResults(FEIK_HSessionSearch Handle, int32 MaxSearchResults);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_SetParameter")
	static TEnumAsByte<EEIK_Result> EIK_SessionSearch_SetParameter(FEIK_HSessionSearch Handle, FEIK_Sessions_AttributeData Parameter, const TEnumAsByte<EEIK_EComparisonOp>& ComparisonOp);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_SetSessionId")
	static TEnumAsByte<EEIK_Result> EIK_SessionSearch_SetSessionId(FEIK_HSessionSearch Handle, const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_SetTargetUserId")
	static TEnumAsByte<EEIK_Result> EIK_SessionSearch_SetTargetUserId(FEIK_HSessionSearch Handle, FEIK_ProductUserId TargetUserId);

};
