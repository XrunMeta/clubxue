

#pragma once
#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Online.h"
#include "Engine/LocalPlayer.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Interfaces/OnlineMessageInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "UObject/UObjectIterator.h"

#include "AdvancedFriendsLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedFriendsLog, Log, All);

UCLASS()
class UAdvancedFriendsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|FriendsList", meta = (ExpandEnumAsExecs = "Result"))
	static void SendSessionInviteToFriends(APlayerController *PlayerController, const TArray<FBPUniqueNetId> &Friends, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|FriendsList", meta = (ExpandEnumAsExecs = "Result"))
	static void SendSessionInviteToFriend(APlayerController *PlayerController, const FBPUniqueNetId &FriendUniqueNetId, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|FriendsList")
	static void GetFriend(APlayerController *PlayerController, const FBPUniqueNetId FriendUniqueNetId, FBPFriendInfo &Friend);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|FriendsList")
	static void GetStoredFriendsList(APlayerController *PlayerController, TArray<FBPFriendInfo> &FriendsList);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|RecentPlayersList")
	static void GetStoredRecentPlayersList(FBPUniqueNetId UniqueNetId, TArray<FBPOnlineRecentPlayer> &PlayersList);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedFriends|FriendsList")
	static void IsAFriend(APlayerController *PlayerController, const FBPUniqueNetId UniqueNetId, bool &IsFriend);
};	
