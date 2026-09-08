
#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Engine/LocalPlayer.h"
#include "FindFriendSessionCallbackProxy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedFindFriendSessionLog, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintFindFriendSessionDelegate, const TArray<FBlueprintSessionResult> &, SessionInfo);

UCLASS(MinimalAPI)
class UFindFriendSessionCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FBlueprintFindFriendSessionDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FBlueprintFindFriendSessionDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedFriends")
	static UFindFriendSessionCallbackProxy* FindFriendSession(UObject* WorldContextObject, APlayerController *PlayerController, const FBPUniqueNetId &FriendUniqueNetId);

	virtual void Activate() override;

private:

	void OnFindFriendSessionCompleted(int32 LocalPlayer, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionInfo);

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FBPUniqueNetId cUniqueNetId;

	FOnFindFriendSessionCompleteDelegate OnFindFriendSessionCompleteDelegate;

	FDelegateHandle FindFriendSessionCompleteDelegateHandle;

	TWeakObjectPtr<UObject> WorldContextObject;
};

