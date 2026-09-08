

#pragma once
#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

#include "UObject/UObjectIterator.h"

#include "AdvancedIdentityLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedIdentityLog, Log, All);

UCLASS()
class UAdvancedIdentityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedIdentity", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
	static void GetLoginStatus(UObject* WorldContextObject, const FBPUniqueNetId & UniqueNetID, EBPLoginStatus & LoginStatus, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedIdentity", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
	static void GetPlayerAuthToken(UObject* WorldContextObject, APlayerController * PlayerController, FString & AuthToken, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedIdentity", meta = (WorldContext = "WorldContextObject"))
	static void GetPlayerNickname(UObject* WorldContextObject, const FBPUniqueNetId & UniqueNetID, FString & PlayerNickname);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedIdentity|UserAccount", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
	static void GetUserAccount(UObject* WorldContextObject, const FBPUniqueNetId & UniqueNetId, FBPUserOnlineAccount & AccountInfo, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedIdentity|UserAccount", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
	static void GetAllUserAccounts(UObject* WorldContextObject, TArray<FBPUserOnlineAccount> & AccountInfos, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedIdentity|UserAccount")
	static void GetUserAccountAccessToken(const FBPUserOnlineAccount & AccountInfo, FString & AccessToken);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedIdentity|UserAccount", meta = (ExpandEnumAsExecs = "Result"))
	static void GetUserAccountAuthAttribute(const FBPUserOnlineAccount & AccountInfo, const FString & AttributeName, FString & AuthAttribute, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedIdentity|UserAccount", meta = (ExpandEnumAsExecs = "Result"))
	static void SetUserAccountAttribute(const FBPUserOnlineAccount & AccountInfo, const FString & AttributeName, const FString & NewAttributeValue, EBlueprintResultSwitch &Result);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedIdentity|UserAccount")
	static void GetUserID(const FBPUserOnlineAccount & AccountInfo, FBPUniqueNetId & UniqueNetID);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedIdentity|UserAccount")
	static void GetUserAccountRealName(const FBPUserOnlineAccount & AccountInfo, FString & UserName);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedIdentity|UserAccount")
	static void GetUserAccountDisplayName(const FBPUserOnlineAccount & AccountInfo, FString & DisplayName);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedIdentity|UserAccount", meta = (ExpandEnumAsExecs = "Result"))
	static void GetUserAccountAttribute(const FBPUserOnlineAccount & AccountInfo, const FString & AttributeName, FString & AttributeValue, EBlueprintResultSwitch &Result);

};	
