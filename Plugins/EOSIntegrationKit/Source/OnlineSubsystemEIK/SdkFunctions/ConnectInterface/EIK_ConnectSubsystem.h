

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/AsyncFunctions/Extra/EIK_BlueprintFunctions.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_ConnectSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAuthExpirationCallback, FEIK_ProductUserId, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnLoginStatusChangedCallback, FEIK_ProductUserId, LocalUserId, const TEnumAsByte<EIK_ELoginStatus>&, LoginStatus);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_ConnectSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	FOnAuthExpirationCallback OnAuthExpiration;
	FOnLoginStatusChangedCallback OnLoginStatusChanged;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_AddNotifyAuthExpiration")
	FEIK_NotificationId EIK_Connect_AddNotifyAuthExpiration(const FOnAuthExpirationCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_AddNotifyLoginStatusChanged")
	FEIK_NotificationId EIK_Connect_AddNotifyLoginStatusChanged(const FOnLoginStatusChangedCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyIdToken")
	TEnumAsByte<EEIK_Result> EIK_Connect_CopyIdToken(FEIK_ProductUserId LocalUserId, FEIK_Connect_IdToken& OutIdToken);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyProductUserExternalAccountByAccountId")
	TEnumAsByte<EEIK_Result> EIK_Connect_CopyProductUserExternalAccountByAccountId(FEIK_ProductUserId LocalUserId, FString AccountId, FEIK_Connect_ExternalAccountInfo& OutExternalAccountInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyProductUserExternalAccountByAccountType")
	TEnumAsByte<EEIK_Result> EIK_Connect_CopyProductUserExternalAccountByAccountType(FEIK_ProductUserId LocalUserId, TEnumAsByte<EEIK_EExternalAccountType> AccountType, FEIK_Connect_ExternalAccountInfo& OutExternalAccountInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyProductUserExternalAccountByIndex")
	TEnumAsByte<EEIK_Result> EIK_Connect_CopyProductUserExternalAccountByIndex(FEIK_ProductUserId LocalUserId, int32 Index, FEIK_Connect_ExternalAccountInfo& OutExternalAccountInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyProductUserInfo")
	TEnumAsByte<EEIK_Result> EIK_Connect_CopyProductUserInfo(FEIK_ProductUserId LocalUserId, FEIK_Connect_ExternalAccountInfo& OutProductUserInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_ExternalAccountInfo_Release")
	void EIK_Connect_ExternalAccountInfo_Release(FEIK_Connect_ExternalAccountInfo ExternalAccountInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetExternalAccountMapping")
	FEIK_ProductUserId EIK_Connect_GetExternalAccountMapping(FEIK_ProductUserId LocalUserId, TEnumAsByte<EEIK_EExternalAccountType> AccountIdType, FString TargetExternalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetLoggedInUserByIndex")
	FEIK_ProductUserId EIK_Connect_GetLoggedInUserByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetLoggedInUsersCount")
	int32 EIK_Connect_GetLoggedInUsersCount();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetLoginStatus")
	TEnumAsByte<EEIK_LoginStatus> EIK_Connect_GetLoginStatus(FEIK_ProductUserId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetProductUserExternalAccountCount")
	int32 EIK_Connect_GetProductUserExternalAccountCount(FEIK_ProductUserId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetProductUserIdMapping")
	TEnumAsByte<EEIK_Result> EIK_Connect_GetProductUserIdMapping(FEIK_ProductUserId LocalUserId, TEnumAsByte<EEIK_EExternalAccountType> AccountIdType, FEIK_ProductUserId TargetUserId, FString& OutBuffer);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_IdToken_Release")
	void EIK_Connect_IdToken_Release(FEIK_Connect_IdToken IdToken);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_RemoveNotifyAuthExpiration")
	void EIK_Connect_RemoveNotifyAuthExpiration(FEIK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_RemoveNotifyLoginStatusChanged")
	void EIK_Connect_RemoveNotifyLoginStatusChanged(FEIK_NotificationId InId);

};
