

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEIK/SdkFunctions/ConnectInterface/EIK_ConnectSubsystem.h"
#include "EIK_PlayerDataStorage_QueryFile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQueryFileComplete, const TEnumAsByte<EEIK_Result>&, Result, const FEIK_ProductUserId&, LocalUserId);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_PlayerDataStorage_QueryFile : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_QueryFile")
	static UEIK_PlayerDataStorage_QueryFile* EIK_PlayerDataStorage_QueryFile(FEIK_ProductUserId LocalUserId, FString Filename);

	UPROPERTY(BlueprintAssignable)
	FOnQueryFileComplete OnCallback;

private:
	virtual void Activate() override;
	static void EOS_CALL EOS_PlayerDataStorage_OnQueryFileComplete(const EOS_PlayerDataStorage_QueryFileCallbackInfo* Data);
	FEIK_ProductUserId Var_LocalUserId;
	FString Var_Filename;
};
