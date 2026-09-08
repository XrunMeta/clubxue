
#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BlueprintDataDefinitions.h"
#include "CancelFindSessionsCallbackProxy.generated.h"

UCLASS(MinimalAPI)
class UCancelFindSessionsCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedSessions")
	static UCancelFindSessionsCallbackProxy* CancelFindSessions(UObject* WorldContextObject, class APlayerController* PlayerController);

	virtual void Activate() override;

private:

	void OnCompleted(bool bWasSuccessful);

private:

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FOnCancelFindSessionsCompleteDelegate Delegate;

	FDelegateHandle DelegateHandle;

	TWeakObjectPtr<UObject> WorldContextObject;
};
