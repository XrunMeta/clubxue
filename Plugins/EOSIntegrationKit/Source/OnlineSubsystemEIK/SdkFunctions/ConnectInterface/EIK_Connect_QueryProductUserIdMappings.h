

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Connect_QueryProductUserIdMappings.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEIK_Connect_QueryProductUserIdMappings_Delegate, const FEIK_ProductUserId&, LocalUserId, TEnumAsByte<EEIK_Result>, Result);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Connect_QueryProductUserIdMappings : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_QueryProductUserIdMappings")
	static UEIK_Connect_QueryProductUserIdMappings* EIK_Connect_QueryProductUserIdMappings(FEIK_ProductUserId LocalUserId, const TArray<FEIK_ProductUserId>& TargetProductUserIds);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit")
	FEIK_Connect_QueryProductUserIdMappings_Delegate OnCallback;

private:
	FEIK_ProductUserId Var_LocalUserId;
	TArray<FEIK_ProductUserId> Var_TargetProductUserIds;
	static void OnQueryProductUserIdMappingsCallback(const EOS_Connect_QueryProductUserIdMappingsCallbackInfo* Data);
	virtual void Activate() override;

};
