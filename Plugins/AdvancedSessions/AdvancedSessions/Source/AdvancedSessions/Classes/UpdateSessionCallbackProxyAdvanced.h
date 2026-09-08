
#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "BlueprintDataDefinitions.h"
#include "UpdateSessionCallbackProxyAdvanced.generated.h"

UCLASS(MinimalAPI)
class UUpdateSessionCallbackProxyAdvanced : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject",AutoCreateRefTerm="ExtraSettings"), Category = "Online|AdvancedSessions")
	static UUpdateSessionCallbackProxyAdvanced* UpdateSession(UObject* WorldContextObject, const TArray<FSessionPropertyKeyPair> &ExtraSettings, int32 PublicConnections = 100, int32 PrivateConnections = 0, bool bUseLAN = false, bool bAllowInvites = false, bool bAllowJoinInProgress = false, bool bRefreshOnlineData = true, bool bIsDedicatedServer = false, bool bShouldAdvertise = true, bool bAllowJoinViaPresence = true, bool bAllowJoinViaPresenceFriendsOnly = false);

	virtual void Activate() override;

private:

	void OnUpdateCompleted(FName SessionName, bool bWasSuccessful);

	FOnUpdateSessionCompleteDelegate OnUpdateSessionCompleteDelegate;

	FDelegateHandle OnUpdateSessionCompleteDelegateHandle;

	int NumPublicConnections = 100;

	int NumPrivateConnections = 0;

	bool bUseLAN = false;

	bool bAllowInvites = true;

	TArray<FSessionPropertyKeyPair> ExtraSettings;

	bool bRefreshOnlineData = true;

	bool bAllowJoinInProgress = true;

	bool bAllowJoinViaPresence = true;

	bool bAllowJoinViaPresenceFriendsOnly = false;

	bool bDedicatedServer = false;

	bool bShouldAdvertise = true;

	TWeakObjectPtr<UObject> WorldContextObject;
};

