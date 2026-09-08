#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "StartSessionCallbackProxyAdvanced.generated.h"

UCLASS(MinimalAPI)
class UStartSessionCallbackProxyAdvanced : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(
		BlueprintCallable
		, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject")
		, Category = "Online|AdvancedSessions"
	)
	static UStartSessionCallbackProxyAdvanced* StartAdvancedSession(UObject* WorldContextObject);

	virtual void Activate() override;

private:

	void OnStartCompleted(FName SessionName, bool bWasSuccessful);

	FOnStartSessionCompleteDelegate StartCompleteDelegate;

	FDelegateHandle StartCompleteDelegateHandle;

	TWeakObjectPtr<UObject> WorldContextObject;
};