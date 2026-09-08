
#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Engine/LocalPlayer.h"
#include "LogoutUserCallbackProxy.generated.h"

UCLASS(MinimalAPI)
class ULogoutUserCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedIdentity")
	static ULogoutUserCallbackProxy* LogoutUser(UObject* WorldContextObject, class APlayerController* PlayerController);

	virtual void Activate() override;

private:

	void OnCompleted(int LocalUserNum, bool bWasSuccessful);

private:

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FOnLogoutCompleteDelegate Delegate;

	FDelegateHandle DelegateHandle;

	TWeakObjectPtr<UObject> WorldContextObject;
};
