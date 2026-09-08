
#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Engine/LocalPlayer.h"
#include "LoginUserCallbackProxy.generated.h"

UCLASS(MinimalAPI)
class ULoginUserCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject", AdvancedDisplay = "AuthType"), Category = "Online|AdvancedIdentity")
	static ULoginUserCallbackProxy* LoginUser(UObject* WorldContextObject, class APlayerController* PlayerController, FString UserID, FString UserToken, FString AuthType);

	virtual void Activate() override;

private:

	void OnCompleted(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorVal);

private:

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FString UserID;

	FString UserToken;

	FString AuthType;

	FOnLoginCompleteDelegate Delegate;

	FDelegateHandle DelegateHandle;

	TWeakObjectPtr<UObject> WorldContextObject;
};
