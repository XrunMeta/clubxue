
#include "SendFriendInviteCallbackProxy.h"

#include "Online.h"

DEFINE_LOG_CATEGORY(AdvancedSendFriendInviteLog);

USendFriendInviteCallbackProxy::USendFriendInviteCallbackProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, OnSendInviteCompleteDelegate(FOnSendInviteComplete::CreateUObject(this, &ThisClass::OnSendInviteComplete))
{
}

USendFriendInviteCallbackProxy* USendFriendInviteCallbackProxy::SendFriendInvite(UObject* WorldContextObject, APlayerController *PlayerController, const FBPUniqueNetId &UniqueNetIDInvited)
{
	USendFriendInviteCallbackProxy* Proxy = NewObject<USendFriendInviteCallbackProxy>();
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->cUniqueNetId = UniqueNetIDInvited;
	Proxy->WorldContextObject = WorldContextObject;
	return Proxy;
}

void USendFriendInviteCallbackProxy::Activate()
{
	if (!cUniqueNetId.IsValid())
	{

		UE_LOGF(AdvancedSendFriendInviteLog, Warning, "SendFriendInvite Failed received a bad UniqueNetId!");
		OnFailure.Broadcast();
		return;
	}

	if (!PlayerControllerWeakPtr.IsValid())
	{

		UE_LOGF(AdvancedSendFriendInviteLog, Warning, "SendFriendInvite Failed received a bad playercontroller!");
		OnFailure.Broadcast();
		return;
	}

	FOnlineSubsystemBPCallHelperAdvanced Helper(TEXT("SendFriendInvite"), GEngine->GetWorldFromContextObject(WorldContextObject.Get(), EGetWorldErrorMode::LogAndReturnNull));

	if (!Helper.OnlineSub)
	{
		OnFailure.Broadcast();
		return;
	}

	auto Friends = Helper.OnlineSub->GetFriendsInterface();
	if (Friends.IsValid())
	{	
		ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerControllerWeakPtr->Player);

		if (!Player)
		{

			UE_LOGF(AdvancedSendFriendInviteLog, Warning, "SendFriendInvite Failed couldn't cast to ULocalPlayer!");
			OnFailure.Broadcast();
			return;
		}

		Friends->SendInvite(Player->GetControllerId(), *cUniqueNetId.GetUniqueNetId(), EFriendsLists::ToString((EFriendsLists::Default)), OnSendInviteCompleteDelegate);
		return;
	}

	OnFailure.Broadcast();
}

void USendFriendInviteCallbackProxy::OnSendInviteComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FUniqueNetId &InvitedPlayer, const FString &ListName, const FString &ErrorString)
{
	if ( bWasSuccessful )
	{ 
		OnSuccess.Broadcast();
	}
	else
	{
		UE_LOGF(AdvancedSendFriendInviteLog, Warning, "SendFriendInvite Failed with error: %ls", *ErrorString);
		OnFailure.Broadcast();
	}
}
