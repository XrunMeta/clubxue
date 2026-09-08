
#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Engine/LocalPlayer.h"
#include "AutoLoginUserCallbackProxy.generated.h"

UCLASS(MinimalAPI)
class UAutoLoginUserCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedIdentity")
	static UAutoLoginUserCallbackProxy* AutoLoginUser(UObject* WorldContextObject, int32 LocalUserNum);

	virtual void Activate() override;

private:

	void OnCompleted(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorVal);

private:

	int32 LocalUserNumber;

	FOnLoginCompleteDelegate Delegate;

	FDelegateHandle DelegateHandle;

	TWeakObjectPtr<UObject> WorldContextObject;
};
