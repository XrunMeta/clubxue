

#pragma once

#include "CoreMinimal.h"
#include "eos_common.h"
#include "eos_sdk.h"
#include "eos_anticheatserver.h"
#include "eos_anticheatcommon_types.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AntiCheatServer.generated.h"

UENUM(BlueprintType)
enum EUserPlatform
{

	EOS_Unknown UMETA(DisplayName = "Unknown"),

	EOS_Windows UMETA(DisplayName = "Windows"),

	EOS_Mac UMETA(DisplayName = "Mac"),

	EOS_Linux UMETA(DisplayName = "Linux"),

	EOS_Xbox UMETA(DisplayName = "Xbox"),

	EOS_PlayStation UMETA(DisplayName = "PlayStation"),

	EOS_Nintendo UMETA(DisplayName = "Nintendo"),

	EOS_IOS UMETA(DisplayName = "IOS"),

	EOS_Android UMETA(DisplayName = "Android"),	
};

UENUM(BlueprintType)
enum EEOS_ClientType
{

	EOS_ProtectedClient UMETA(DisplayName = "ProtectedClient"),

	EOS_UnprotectedClient UMETA(DisplayName = "UnprotectedClient"),

	EOS_AIBot UMETA(DisplayName = "AIBot"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAntiCheatActionRequired, APlayerController*, ControllerRef, bool, bRemoveFromSession);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAntiCheatRegisterClient, APlayerController*, ControllerRef, const TArray<uint8>&, ClientData);
UCLASS()
class ONLINESUBSYSTEMEIK_API UAntiCheatServer : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "EOS Integration Kit|AntiCheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsAntiCheatServerAvailable(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|AntiCheat")
	bool RegisterAntiCheatServer(FString ServerName, FString ClientProductID );

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|AntiCheat")
	bool UnregisterAntiCheatServer();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|AntiCheat")
	bool RegisterClientForAntiCheat(FString ClientProductID, APlayerController* ControllerRef, TEnumAsByte<EUserPlatform> UserPlatform, TEnumAsByte<EEOS_ClientType> ClientType);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|AntiCheat")
	bool UnregisterClientFromAntiCheat(APlayerController* ControllerRef);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|AntiCheat")
	bool RecievedMessageFromClient(APlayerController* Controller, const TArray<uint8>& Message);

	EOS_NotificationId MessageToClientId;

	EOS_NotificationId ClientActionRequiredId;

	static void EOS_CALL OnMessageToClientCb(const EOS_AntiCheatCommon_OnMessageToClientCallbackInfo* Data);
	static void EOS_CALL OnClientActionRequiredCb(const EOS_AntiCheatCommon_OnClientActionRequiredCallbackInfo* Data);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit|AntiCheat")
	FAntiCheatActionRequired OnAntiCheatActionRequired;

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit|AntiCheat")
	FAntiCheatRegisterClient OnAntiCheatRegisterClient;
};
