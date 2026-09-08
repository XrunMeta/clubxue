

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintDataDefinitions.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "Interfaces/VoiceInterface.h"

#include "Engine/GameInstance.h"

#include "UObject/UObjectIterator.h"

#include "AdvancedVoiceLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedVoiceLog, Log, All);

UCLASS()
class UAdvancedVoiceLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedVoice|VoiceInfo", meta = (WorldContext = "WorldContextObject"))
	static void IsHeadsetPresent(UObject* WorldContextObject, bool & bHasHeadset, uint8 LocalPlayerNum = 0);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static void StartNetworkedVoice(UObject* WorldContextObject, uint8 LocalPlayerNum = 0);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static void StopNetworkedVoice(UObject* WorldContextObject, uint8 LocalPlayerNum = 0);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static bool RegisterLocalTalker(UObject* WorldContextObject, uint8 LocalPlayerNum = 0);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static void RegisterAllLocalTalkers(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static void UnRegisterLocalTalker(UObject* WorldContextObject, uint8 LocalPlayerNum = 0);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static void UnRegisterAllLocalTalkers(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static bool RegisterRemoteTalker(UObject* WorldContextObject, const FBPUniqueNetId& UniqueNetId);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static bool UnRegisterRemoteTalker(UObject* WorldContextObject, const FBPUniqueNetId& UniqueNetId);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static void RemoveAllRemoteTalkers(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedVoice|VoiceInfo", meta = (WorldContext = "WorldContextObject"))
	static bool IsLocalPlayerTalking(UObject* WorldContextObject, uint8 LocalPlayerNum);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedVoice|VoiceInfo", meta = (WorldContext = "WorldContextObject"))
	static bool IsRemotePlayerTalking(UObject* WorldContextObject, const FBPUniqueNetId& UniqueNetId);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedVoice|VoiceInfo", meta = (WorldContext = "WorldContextObject"))
	static bool IsPlayerMuted(UObject* WorldContextObject, uint8 LocalUserNumChecking, const FBPUniqueNetId& UniqueNetId);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static bool MuteRemoteTalker(UObject* WorldContextObject, uint8 LocalUserNum, const FBPUniqueNetId& UniqueNetId, bool bIsSystemWide = false);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedVoice", meta = (WorldContext = "WorldContextObject"))
	static bool UnMuteRemoteTalker(UObject* WorldContextObject, uint8 LocalUserNum, const FBPUniqueNetId& UniqueNetId, bool bIsSystemWide = false);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedVoice|VoiceInfo", meta = (WorldContext = "WorldContextObject"))
	static void GetNumLocalTalkers(UObject* WorldContextObject, int32 & NumLocalTalkers);
};	
