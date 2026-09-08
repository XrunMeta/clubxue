

#pragma once

#include "CoreMinimal.h"
#include "eos_base.h"
#include "eos_connect_types.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_CreateDeviceId_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateDeviceIdComplete, const TEnumAsByte<EEIK_Result>&, Result);
UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_CreateDeviceId_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CreateDeviceId")
	static UEIK_CreateDeviceId_AsyncFunction* CreateDeviceId(FString DeviceModel);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit")
	FOnCreateDeviceIdComplete OnCallback;

private:
	virtual void Activate() override;
	FString Var_DeviceModel;
	static void EOS_CALL OnCreateDeviceIdCallback(const EOS_Connect_CreateDeviceIdCallbackInfo* Data);

};
