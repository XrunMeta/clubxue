

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Connect_Logout.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEIK_Connect_Logout_Delegate, const FEIK_ProductUserId&, ProductUserId);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Connect_Logout : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Connect Interface", DisplayName="EOS_Connect_Logout")
	static UEIK_Connect_Logout* EIK_Connect_Logout(FEIK_ProductUserId ProductUserId);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit")
	FEIK_Connect_Logout_Delegate OnCallback;

	static void OnLogoutCallback(const EOS_Connect_LogoutCallbackInfo* Data);

private:
	FEIK_ProductUserId Var_ProductUserId;
	virtual void Activate() override;

};
