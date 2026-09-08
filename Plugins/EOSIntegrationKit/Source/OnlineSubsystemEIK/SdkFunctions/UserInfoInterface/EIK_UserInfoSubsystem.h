

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_UserInfoSubsystem.generated.h"

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_UserInfoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyBestDisplayName")
	static TEnumAsByte<EEIK_Result> EIK_UserInfo_CopyBestDisplayName(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId, FEIK_UserInfo_BestDisplayName& OutBestDisplayName);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyBestDisplayNameWithPlatform")
	static TEnumAsByte<EEIK_Result> EIK_UserInfo_CopyBestDisplayNameWithPlatform(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId, const int32& Platform, FEIK_UserInfo_BestDisplayName& OutBestDisplayName);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyExternalUserInfoByAccountId")
	static TEnumAsByte<EEIK_Result> EIK_UserInfo_CopyExternalUserInfoByAccountId(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId, FString AccountId, FEIK_UserInfo_ExternalUserInfo& OutExternalUserInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyExternalUserInfoByAccountType")
	static TEnumAsByte<EEIK_Result> EIK_UserInfo_CopyExternalUserInfoByAccountType(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId, const TEnumAsByte<EEIK_EExternalAccountType>& AccountType, FEIK_UserInfo_ExternalUserInfo& OutExternalUserInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyExternalUserInfoByIndex")
	static TEnumAsByte<EEIK_Result> EIK_UserInfo_CopyExternalUserInfoByIndex(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId, const int32& Index, FEIK_UserInfo_ExternalUserInfo& OutExternalUserInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyUserInfo")
	static TEnumAsByte<EEIK_Result> EIK_UserInfo_CopyUserInfo(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId, FEIK_UserInfo& OutUserInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_GetExternalUserInfoCount")
	static int32 EIK_UserInfo_GetExternalUserInfoCount(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_GetLocalPlatformType")
	static int32 EIK_UserInfo_GetLocalPlatformType();
};
