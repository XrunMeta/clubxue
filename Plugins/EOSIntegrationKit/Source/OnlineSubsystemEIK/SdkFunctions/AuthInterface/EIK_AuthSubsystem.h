

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_auth.h"
THIRD_PARTY_INCLUDES_END
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_AuthSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_Auth_OnLoginStatusChangedCallback, FEIK_EpicAccountId, LocalUserId, const TEnumAsByte<EIK_ELoginStatus>&, PrevStatus, const TEnumAsByte<EIK_ELoginStatus>&, CurrentStatus);
UCLASS(meta=(DisplayName="Auth Interface"), Category="EOS Integration Kit", DisplayName="Auth Interface")
class ONLINESUBSYSTEMEIK_API UEIK_AuthSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	FEIK_Auth_OnLoginStatusChangedCallback OnLoginStatusChanged;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_AddNotifyLoginStatusChanged")
	FEIK_NotificationId EIK_Auth_AddNotifyLoginStatusChanged(const FEIK_Auth_OnLoginStatusChangedCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_CopyIdToken")
	TEnumAsByte<EEIK_Result> EIK_Auth_CopyIdToken(FEIK_EpicAccountId AccountId, FEIK_Auth_IdToken& OutToken);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_CopyUserAuthToken")
	TEnumAsByte<EEIK_Result> EIK_Auth_CopyUserAuthToken(FEIK_EpicAccountId LocalUserId, FEIK_Auth_Token& OutToken);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetLoggedInAccountByIndex")
	FEIK_EpicAccountId EIK_Auth_GetLoggedInAccountByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetLoggedInAccountsCount")
	int32 EIK_Auth_GetLoggedInAccountsCount();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetLoginStatus")
	TEnumAsByte<EIK_ELoginStatus> EIK_Auth_GetLoginStatus(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetMergedAccountByIndex")
	FEIK_EpicAccountId EIK_Auth_GetMergedAccountByIndex(FEIK_EpicAccountId LocalUserId, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetMergedAccountsCount")
	int32 EIK_Auth_GetMergedAccountsCount(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetSelectedAccountId")
	TEnumAsByte<EEIK_Result> EIK_Auth_GetSelectedAccountId(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId& OutSelectedAccountId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_IdToken_Release")
	void EIK_Auth_IdToken_Release(FEIK_Auth_IdToken& Token);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_RemoveNotifyLoginStatusChanged")
	void EIK_Auth_RemoveNotifyLoginStatusChanged(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Auth Interface", DisplayName="EOS_Auth_Token_Release")
	void EIK_Auth_Token_Release(FEIK_Auth_Token& Token);
};
