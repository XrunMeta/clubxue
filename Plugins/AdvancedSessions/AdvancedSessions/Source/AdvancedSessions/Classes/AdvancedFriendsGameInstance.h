

#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "BlueprintDataDefinitions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Interfaces/OnlineMessageInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "UObject/UObjectIterator.h"
#include "AdvancedFriendsInterface.h"

#include "AdvancedFriendsGameInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedFriendsInterfaceLog, Log, All);

UCLASS()
class ADVANCEDSESSIONS_API UAdvancedFriendsGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	UAdvancedFriendsGameInstance(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AdvancedFriendsInterface)
	bool bCallFriendInterfaceEventsOnPlayerControllers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AdvancedFriendsInterface)
	bool bCallIdentityInterfaceEventsOnPlayerControllers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AdvancedFriendsInterface)
	bool bCallVoiceInterfaceEventsOnPlayerControllers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AdvancedVoiceInterface)
	bool bEnableTalkingStatusDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AdvancedFriendsInterface)
	bool bAutoJoinSessionOnAcceptedUserInviteReceived = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AdvancedFriendsInterface)
	bool bAutoTravelOnAcceptedUserInviteReceived = false;

	virtual void Shutdown() override;
	virtual void Init() override;

	FOnSessionInviteReceivedDelegate SessionInviteReceivedDelegate;
	FDelegateHandle SessionInviteReceivedDelegateHandle;

	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void OnSessionInviteReceivedMaster(const FUniqueNetId & PersonInvited, const FUniqueNetId & PersonInviting, const FString & AppId, const FOnlineSessionSearchResult& SessionToJoin);

	UFUNCTION(BlueprintImplementableEvent, Category = "AdvancedFriends")
	void OnSessionInviteReceived(int32 LocalPlayerNum, FBPUniqueNetId PersonInviting, const FString& AppId, const FBlueprintSessionResult& SessionToJoin);

	FOnSessionUserInviteAcceptedDelegate SessionInviteAcceptedDelegate;
	FDelegateHandle SessionInviteAcceptedDelegateHandle;

	void OnSessionInviteAcceptedMaster(const bool bWasSuccessful, int32 LocalPlayer, TSharedPtr<const FUniqueNetId> PersonInviting, const FOnlineSessionSearchResult& SessionToJoin);

	UFUNCTION(BlueprintImplementableEvent, Category = "AdvancedFriends")
	void OnSessionInviteAccepted(int32 LocalPlayerNum, FBPUniqueNetId PersonInvited, const FBlueprintSessionResult& SessionToJoin);

	UFUNCTION(BlueprintImplementableEvent, Category = "AdvancedVoice")
	void OnPlayerTalkingStateChanged(FBPUniqueNetId PlayerId, bool bIsTalking);

	void OnPlayerTalkingStateChangedMaster(TSharedRef<const FUniqueNetId> PlayerId, bool bIsTalking);

	FOnPlayerTalkingStateChangedDelegate PlayerTalkingStateChangedDelegate;
	FDelegateHandle PlayerTalkingStateChangedDelegateHandle;

	UFUNCTION(BlueprintImplementableEvent , Category = "AdvancedIdentity", meta = (DisplayName = "OnPlayerLoginChanged"))
	void OnPlayerLoginChanged(int32 PlayerNum);

	void OnPlayerLoginChangedMaster(int32 PlayerNum);
	FOnLoginChangedDelegate PlayerLoginChangedDelegate;
	FDelegateHandle PlayerLoginChangedDelegateHandle;

	UFUNCTION(BlueprintImplementableEvent, Category = "AdvancedIdentity", meta = (DisplayName = "OnPlayerLoginStatusChanged"))
	void OnPlayerLoginStatusChanged(int32 PlayerNum, EBPLoginStatus PreviousStatus, EBPLoginStatus NewStatus, FBPUniqueNetId NewPlayerUniqueNetID);

	void OnPlayerLoginStatusChangedMaster(int32 PlayerNum, ELoginStatus::Type PreviousStatus, ELoginStatus::Type NewStatus, const FUniqueNetId & NewPlayerUniqueNetID);
	FOnLoginStatusChangedDelegate PlayerLoginStatusChangedDelegate;
	FDelegateHandle PlayerLoginStatusChangedDelegateHandle;

};

