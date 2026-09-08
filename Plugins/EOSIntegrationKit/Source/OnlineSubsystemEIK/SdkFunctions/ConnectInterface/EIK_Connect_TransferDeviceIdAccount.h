

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Connect_TransferDeviceIdAccount.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEIK_Connect_TransferDeviceIdAccount_Delegate, const FEIK_ProductUserId&, LocalUserId, TEnumAsByte<EEIK_Result>, Result);
UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Connect_TransferDeviceIdAccount : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_TransferDeviceIdAccount")
	static UEIK_Connect_TransferDeviceIdAccount* EIK_Connect_TransferDeviceIdAccount(FEIK_ProductUserId PrimaryLocalUserId, FEIK_ProductUserId LocalUserId, FEIK_ProductUserId ProductUserIdToPreserve);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit")
	FEIK_Connect_TransferDeviceIdAccount_Delegate OnCallback;
private:
	static void OnTransferDeviceIdAccountCallback(const EOS_Connect_TransferDeviceIdAccountCallbackInfo* Data);
	virtual void Activate() override;
	FEIK_ProductUserId Var_PrimaryLocalUserId;
	FEIK_ProductUserId Var_LocalUserId;
	FEIK_ProductUserId Var_ProductUserIdToPreserve;
};
