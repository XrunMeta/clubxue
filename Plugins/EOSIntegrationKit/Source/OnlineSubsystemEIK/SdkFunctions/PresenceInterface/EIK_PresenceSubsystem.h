

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_PresenceSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_Presence_JoinGameAcceptedCallbackInfo, const FString&, JoinInfo, const FEIK_EpicAccountId&, LocalUserId, const FEIK_EpicAccountId&, TargetUserId, const FEIK_UI_EventId&, UiEventId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_Presence_OnPresenceChangedCallbackInfo, const FEIK_EpicAccountId&, LocalUserId, const FEIK_EpicAccountId&, PresenceUserId);

UCLASS(DisplayName="Presense Interface", DisplayName="Presense Interface")
class ONLINESUBSYSTEMEIK_API UEIK_PresenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	FEIK_Presence_JoinGameAcceptedCallbackInfo JoinGameAcceptedCallbackInfo;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_Presence_AddNotifyJoinGameAccepted")
	FEIK_NotificationId EIK_Presence_AddNotifyJoinGameAccepted(const FEIK_Presence_JoinGameAcceptedCallbackInfo& Callback);

	FEIK_Presence_OnPresenceChangedCallbackInfo PresenceChangedCallbackInfo;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_Presence_AddNotifyOnPresenceChanged")
	FEIK_NotificationId EIK_Presence_AddNotifyOnPresenceChanged(const FEIK_Presence_OnPresenceChangedCallbackInfo& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_Presence_CopyPresence")
	TEnumAsByte<EEIK_Result> EIK_Presence_CopyPresence(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId, FEIK_Presence_Info& OutPresence);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_Presence_CreatePresenceModification")
	TEnumAsByte<EEIK_Result> EIK_Presence_CreatePresenceModification(FEIK_EpicAccountId LocalUserId, FEIK_HPresenceModification& OutPresenceModificationHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_Presence_GetJoinInfo")
	TEnumAsByte<EEIK_Result> EIK_Presence_GetJoinInfo(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId, FString& OutBuffer);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_Presence_HasPresence")
	bool EIK_Presence_HasPresence(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_Presence_Info_Release")
	void EIK_Presence_Info_Release(const FEIK_Presence_Info& PresenceInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_Presence_RemoveNotifyJoinGameAccepted")
	void EIK_Presence_RemoveNotifyJoinGameAccepted(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_Presence_RemoveNotifyOnPresenceChanged")
	void EIK_Presence_RemoveNotifyOnPresenceChanged(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_DeleteData")
	TEnumAsByte<EEIK_Result> EIK_PresenceModification_DeleteData(FEIK_HPresenceModification PresenceModificationHandle, const FEIK_PresenceModification_DataRecordId& Data, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_Release")
	void EIK_PresenceModification_Release(FEIK_HPresenceModification PresenceModificationHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_SetData")
	TEnumAsByte<EEIK_Result> EIK_PresenceModification_SetData(FEIK_HPresenceModification PresenceModificationHandle, const FEIK_Presence_DataRecord& Data, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_SetJoinInfo")
	TEnumAsByte<EEIK_Result> EIK_PresenceModification_SetJoinInfo(FEIK_HPresenceModification PresenceModificationHandle, const FString& JoinInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_SetRawRichText")
	TEnumAsByte<EEIK_Result> EIK_PresenceModification_SetRawRichText(FEIK_HPresenceModification PresenceModificationHandle, const FString& RichText);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_SetStatus")
	TEnumAsByte<EEIK_Result> EIK_PresenceModification_SetStatus(FEIK_HPresenceModification PresenceModificationHandle, TEnumAsByte<EEIK_Presence_EStatus> Status);
};
