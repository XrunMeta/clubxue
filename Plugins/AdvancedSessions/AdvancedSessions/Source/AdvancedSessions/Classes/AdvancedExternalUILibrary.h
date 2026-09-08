

#pragma once
#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Online.h"
#include "Engine/LocalPlayer.h"
#include "OnlineSubsystem.h"
#include "BlueprintDataDefinitions.h"

#include "Interfaces/OnlineSessionInterface.h"

#include "AdvancedExternalUILibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedExternalUILog, Log, All);

UCLASS()
class UAdvancedExternalUILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedExternalUI", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
	static void ShowFriendsUI(UObject* WorldContextObject, APlayerController *PlayerController, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedExternalUI", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
	static void ShowInviteUI(UObject* WorldContextObject, APlayerController *PlayerController, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedExternalUI", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
	static void ShowLeaderBoardUI(UObject* WorldContextObject, FString LeaderboardName, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedExternalUI", meta = (ExpandEnumAsExecs = "Result", AutoCreateRefTerm = "AllowedDomains", WorldContext = "WorldContextObject"))
	static void ShowWebURLUI(UObject* WorldContextObject, FString URLToShow, EBlueprintResultSwitch &Result, TArray<FString>& AllowedDomains, bool bEmbedded = false , bool bShowBackground = false, bool bShowCloseButton = false, int32 OffsetX = 0, int32 OffsetY = 0, int32 SizeX = 0, int32 SizeY = 0);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedExternalUI", meta = (WorldContext = "WorldContextObject"))
	static void CloseWebURLUI(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedExternalUI", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
	static void ShowProfileUI(UObject* WorldContextObject, const FBPUniqueNetId PlayerViewingProfile, const FBPUniqueNetId PlayerToViewProfileOf, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedExternalUI", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
	static void ShowAccountUpgradeUI(UObject* WorldContextObject, const FBPUniqueNetId PlayerRequestingAccountUpgradeUI, EBlueprintResultSwitch &Result);

};	
