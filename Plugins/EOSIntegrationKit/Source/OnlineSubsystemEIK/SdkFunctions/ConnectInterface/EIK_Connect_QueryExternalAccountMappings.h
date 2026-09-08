

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Connect_QueryExternalAccountMappings.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEIK_Connect_QueryExternalAccountMappings_Delegate, const FEIK_ProductUserId&, ProductUserId, TEnumAsByte<EEIK_Result>, Result);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Connect_QueryExternalAccountMappings : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_QueryExternalAccountMappings")
	static UEIK_Connect_QueryExternalAccountMappings* EIK_Connect_QueryExternalAccountMappings(FEIK_ProductUserId ProductUserId, TEnumAsByte<EEIK_EExternalAccountType> AccountType, const TArray<FString>& ExternalAccountIds);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit")
	FEIK_Connect_QueryExternalAccountMappings_Delegate OnCallback;
private:
	static void OnQueryExternalAccountMappingsCallback(const EOS_Connect_QueryExternalAccountMappingsCallbackInfo* Data);
	virtual void Activate() override;

	FEIK_ProductUserId Var_ProductUserId;
	TEnumAsByte<EEIK_EExternalAccountType> Var_AccountType;
	TArray<FString> Var_ExternalAccountIds;

};
