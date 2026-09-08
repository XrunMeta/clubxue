

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_ui.h"
#include "eos_ui_types.h"
THIRD_PARTY_INCLUDES_END
#include "EIK_UiSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_OnDisplaySettingsUpdated, bool, bIsVisible, bool, bIsExclusiveInput);
DECLARE_DYNAMIC_DELEGATE(FEIK_OnMemoryMonitor);
UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_UiSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_AcknowledgeEventId")
	static TEnumAsByte<EEIK_Result> EIK_UI_AcknowledgeEventId(const FEIK_UI_EventId& UiEventId, const TEnumAsByte<EEIK_Result>& Result);

	FEIK_OnDisplaySettingsUpdated OnDisplaySettingsUpdated;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_AddNotifyDisplaySettingsUpdated")
	FEIK_NotificationId EIK_UI_AddNotifyDisplaySettingsUpdated(FEIK_OnDisplaySettingsUpdated Callback);

	FEIK_OnMemoryMonitor OnMemoryMonitor;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_AddNotifyMemoryMonitor")
	FEIK_NotificationId EIK_UI_AddNotifyMemoryMonitor(FEIK_OnMemoryMonitor Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_GetFriendsExclusiveInput")
	static bool EIK_UI_GetFriendsExclusiveInput(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_GetFriendsVisible")
	static bool EIK_UI_GetFriendsVisible(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_GetNotificationLocationPreference")
	static TEnumAsByte<EEIK_UI_ENotificationLocation> EIK_UI_GetNotificationLocationPreference();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_PauseSocialOverlay")
	TEnumAsByte<EEIK_Result> EIK_UI_PauseSocialOverlay(bool bIsPaused);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_RemoveNotifyDisplaySettingsUpdated")
	void EIK_UI_RemoveNotifyDisplaySettingsUpdated(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_RemoveNotifyMemoryMonitor")
	void EIK_UI_RemoveNotifyMemoryMonitor(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | UI Interface", DisplayName="EOS_UI_SetDisplayPreference")
	TEnumAsByte<EEIK_Result> EIK_UI_SetDisplayPreference(const EEIK_UI_ENotificationLocation& DisplayPreference);
};
