

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Connect_UnlinkAccount.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEIK_Connect_UnlinkAccount_Delegate, const FEIK_ProductUserId&, LocalUserId, TEnumAsByte<EEIK_Result>, Result);
UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Connect_UnlinkAccount : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_UnlinkAccount")
	static UEIK_Connect_UnlinkAccount* EIK_Connect_UnlinkAccount(FEIK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit")
	FEIK_Connect_UnlinkAccount_Delegate OnCallback;
private:
	static void OnUnlinkAccountCallback(const EOS_Connect_UnlinkAccountCallbackInfo* Data);
	virtual void Activate() override;
	FEIK_ProductUserId Var_LocalUserId;
};
