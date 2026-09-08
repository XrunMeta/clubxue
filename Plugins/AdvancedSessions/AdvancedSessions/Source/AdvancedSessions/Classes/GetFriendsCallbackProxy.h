
#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Engine/LocalPlayer.h"
#include "GetFriendsCallbackProxy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedGetFriendsLog, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintGetFriendsListDelegate, const TArray<FBPFriendInfo>&, Results);

UCLASS(MinimalAPI)
class UGetFriendsCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FBlueprintGetFriendsListDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FBlueprintGetFriendsListDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedFriends")
	static UGetFriendsCallbackProxy* GetAndStoreFriendsList(UObject* WorldContextObject, class APlayerController* PlayerController);

	virtual void Activate() override;

private:

	void OnReadFriendsListCompleted(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorString);

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FOnReadFriendsListComplete FriendListReadCompleteDelegate;

	TWeakObjectPtr<UObject> WorldContextObject;
};

