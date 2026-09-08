

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_friends.h"
#include "eos_friends_types.h"
THIRD_PARTY_INCLUDES_END
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_FriendsSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_Friends_OnBlockedUsersUpdateCallback, const FEIK_EpicAccountId&, LocalUserId, const FEIK_EpicAccountId&, TargetUserId, bool, bBlocked);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_Friends_OnFriendsUpdateCallback, const FEIK_EpicAccountId&, LocalUserId, const FEIK_EpicAccountId&, TargetUserId, const TEnumAsByte<EEIK_EFriendsStatus>&, PreviousStatus, const TEnumAsByte<EEIK_EFriendsStatus>&, CurrentStatus);
UCLASS(DisplayName="Friends Interface",meta=(DisplayName="Friends Interface"))
class ONLINESUBSYSTEMEIK_API UEIK_FriendsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FEIK_Friends_OnBlockedUsersUpdateCallback OnBlockedUserUpdate;
	static void EOS_CALL OnBlockedUserUpdateCallback(const EOS_Friends_OnBlockedUsersUpdateInfo* Data);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Friends Interface", DisplayName="EOS_Friends_AddNotifyBlockedUsersUpdate")
	FEIK_NotificationId EIK_Friends_AddNotifyBlockedUsersUpdate(FEIK_Friends_OnBlockedUsersUpdateCallback Callback);

	FEIK_Friends_OnFriendsUpdateCallback OnFriendsUpdate;
	static void EOS_CALL OnFriendsUpdateCallback(const EOS_Friends_OnFriendsUpdateInfo* Data);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Friends Interface", DisplayName="EOS_Friends_AddNotifyFriendsUpdate")
	FEIK_NotificationId EIK_Friends_AddNotifyFriendsUpdate(FEIK_Friends_OnFriendsUpdateCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetBlockedUserAtIndex")
	FEIK_EpicAccountId EIK_Friends_GetBlockedUserAtIndex(FEIK_EpicAccountId LocalUserId, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetBlockedUsersCount")
	int32 EIK_Friends_GetBlockedUsersCount(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetFriendAtIndex")
	FEIK_EpicAccountId EIK_Friends_GetFriendAtIndex(FEIK_EpicAccountId LocalUserId, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetFriendsCount")
	int32 EIK_Friends_GetFriendsCount(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetStatus")
	TEnumAsByte<EEIK_EFriendsStatus> EIK_Friends_GetStatus(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Friends Interface", DisplayName="EOS_Friends_RemoveNotifyBlockedUsersUpdate")
	void EIK_Friends_RemoveNotifyBlockedUsersUpdate(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Friends Interface", DisplayName="EOS_Friends_RemoveNotifyFriendsUpdate")
	void EIK_Friends_RemoveNotifyFriendsUpdate(FEIK_NotificationId NotificationId);

};

