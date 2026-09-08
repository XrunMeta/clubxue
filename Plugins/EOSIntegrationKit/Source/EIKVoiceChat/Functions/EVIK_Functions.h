

#pragma once

#include "CoreMinimal.h"
#include "EIKVoiceChat/Subsystem/EIK_Voice_Subsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "eos_common.h"

#include "EVIK_Functions.generated.h"

UCLASS()

class EIKVOICECHAT_API UEVIK_Functions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, DisplayName="Initialize EOS Voice Chat", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
	static bool InitializeEOSVoiceChat(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, DisplayName="Connect to EOS Voice Chat", Category="EOS Integration Kit|Voice Chat", meta = (AutoCreateRefTerm = "Result", WorldContext="WorldContextObject"))
	static void ConnectVoiceChat(const UObject* WorldContextObject, const FEIKResultDelegate& Result);

	UFUNCTION(BlueprintPure, DisplayName = "Is EOS Voice Chat Connected?", Category = "EOS Integration Kit|Voice Chat", meta = (WorldContext = "WorldContextObject"))
	static bool IsVoiceChatConnected(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, DisplayName="Login to EOS Voice Chat", Category="EOS Integration Kit|Voice Chat", meta = (AutoCreateRefTerm = "Result", WorldContext="WorldContextObject"))
	static void LoginEOSVoiceChat(const UObject* WorldContextObject, FString PlayerName, const FEIKResultDelegate& Result);

	UFUNCTION(BlueprintCallable, DisplayName="Logout from EOS Voice Chat", Category="EOS Integration Kit|Voice Chat", meta = (AutoCreateRefTerm = "Result", WorldContext="WorldContextObject"))
	static void LogoutEOSVoiceChat(const UObject* WorldContextObject, FString PlayerName, const FEIKResultDelegate& Result);

	UFUNCTION(BlueprintPure, DisplayName = "Is EOS Voice Chat Logging-In?", Category = "EOS Integration Kit|Voice Chat", meta = (WorldContext = "WorldContextObject"))
	static bool IsEOSVoiceChatLoggingIn(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, DisplayName = "Is EOS Voice Chat Logged-In?", Category = "EOS Integration Kit|Voice Chat", meta = (WorldContext = "WorldContextObject"))
	static bool IsEOSVoiceChatLoggedIn(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, DisplayName="Get Logged in EOS Voice Chat User", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
	static FString LoggedInUser(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, DisplayName="Get EOS Voice Room Token", Category="EOS Integration Kit|Voice Chat", meta = (AutoCreateRefTerm = "Result", WorldContext="WorldContextObject"))
	static void EOSRoomToken(FString VoiceRoomName, FString PlayerName, FString ClientIP, const FEIKRoomTokenResultDelegate& Result);

	UFUNCTION(BlueprintCallable, DisplayName="Join EOS Voice Room", Category="EOS Integration Kit|Voice Chat", meta = (AutoCreateRefTerm = "Result", WorldContext="WorldContextObject"))
	static void JoinEOSRoom(const UObject* WorldContextObject, FString VoiceRoomName, FString RoomData, bool bEnableEcho, const FEIKResultDelegate& Result);

	UFUNCTION(BlueprintCallable, DisplayName="Leave EOS Voice Room", Category="EOS Integration Kit|Voice Chat", meta = (AutoCreateRefTerm = "Result", WorldContext="WorldContextObject"))
	static void LeaveEOSRoom(const UObject* WorldContextObject, FString VoiceRoomName, const FEIKResultDelegate& Result);

	UFUNCTION(BlueprintCallable, DisplayName="Get Players in EOS Voice Room", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
	static TArray<FString> GetPlayersInRoom(const UObject* WorldContextObject, FString VoiceRoomName);

	UFUNCTION(BlueprintCallable, DisplayName="Get Joined EOS Voice Rooms", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
	static TArray<FString> GetAllRooms(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, DisplayName="Get EOS Voice Player Volume", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
	static float GetPlayerVolume(const UObject* WorldContextObject, const FString& PlayerName);

	static char* GetProductUserID(const FString& PlayerName);

	UFUNCTION(BlueprintCallable, DisplayName="Set EOS Voice Player Volume", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
	static bool SetPlayerVolume(const UObject* WorldContextObject, const FString& PlayerName, float NewVolume);

    UFUNCTION(BlueprintPure, DisplayName="Is EOS Voice Player Muted", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
    static bool IsPlayerMuted(const UObject* WorldContextObject, const FString& PlayerName);

    UFUNCTION(BlueprintCallable, DisplayName="Set EOS Voice Player Muted", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
    static bool SetPlayerMuted(const UObject* WorldContextObject, const FString& PlayerName, bool MutePlayer);

    UFUNCTION(BlueprintCallable, DisplayName="Transmit Voice To All EOS Voice Rooms", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
    static bool TransmitToAllRooms(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, DisplayName="Transmit Voice To Selected EOS Voice Room", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
    static bool TransmitToSelectedRoom(const UObject* WorldContextObject, FString RoomName);

    UFUNCTION(BlueprintCallable, DisplayName="Transmit Voice To No EOS Voice Room", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
    static bool TransmitToNoRoom(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, DisplayName="Get All EOS Voice Input Methods", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
    static TArray<FDeviceEVIKSettings> GetInputMethods(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, DisplayName="Get All EOS Voice Output Methods", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
    static TArray<FDeviceEVIKSettings> GetOutputMethods(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, DisplayName="Set EOS Voice Output Method", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
    static bool SetOutputMethods(const UObject* WorldContextObject, FString MethodID);

    UFUNCTION(BlueprintCallable, DisplayName="Set EOS Voice Input Method", Category="EOS Integration Kit|Voice Chat", meta=(WorldContext="WorldContextObject"))
    static bool SetInputMethods(const UObject* WorldContextObject, FString MethodID);

	UFUNCTION(BlueprintPure, DisplayName = "Is EOS Player Talking", Category = "EOS Integration Kit|Voice Chat", meta = (WorldContext = "WorldContextObject"))
	static bool IsPlayerTalking(const UObject* WorldContextObject, FString PlayerName);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Is EOS Voice Input Method Muted", Category = "EOS Integration Kit|Voice Chat", meta = (WorldContext = "WorldContextObject"))
	static void MuteInputDevice(const UObject* WorldContextObject, bool Mute, bool& bWasSuccess);
};