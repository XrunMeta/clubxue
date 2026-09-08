
#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Engine/LocalPlayer.h"
#include "SendFriendInviteCallbackProxy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedSendFriendInviteLog, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBlueprintSendFriendInviteDelegate);

UCLASS(MinimalAPI)
class USendFriendInviteCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FBlueprintSendFriendInviteDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FBlueprintSendFriendInviteDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedFriends")
	static USendFriendInviteCallbackProxy* SendFriendInvite(UObject* WorldContextObject, APlayerController *PlayerController, const FBPUniqueNetId &UniqueNetIDInvited);

	virtual void Activate() override;

private:

	void OnSendInviteComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FUniqueNetId &InvitedPlayer, const FString &ListName, const FString &ErrorString);

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FBPUniqueNetId cUniqueNetId;

	FOnSendInviteComplete OnSendInviteCompleteDelegate;

	TWeakObjectPtr<UObject> WorldContextObject;
};

