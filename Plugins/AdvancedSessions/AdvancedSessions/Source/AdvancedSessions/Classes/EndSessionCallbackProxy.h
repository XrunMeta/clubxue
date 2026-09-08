
#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BlueprintDataDefinitions.h"
#include "EndSessionCallbackProxy.generated.h"

UCLASS(MinimalAPI)
class UEndSessionCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedSessions")
	static UEndSessionCallbackProxy* EndSession(UObject* WorldContextObject, class APlayerController* PlayerController);

	virtual void Activate() override;

private:

	void OnCompleted(FName SessionName, bool bWasSuccessful);

private:

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FOnEndSessionCompleteDelegate Delegate;

	FDelegateHandle DelegateHandle;

	TWeakObjectPtr<UObject> WorldContextObject;
};
