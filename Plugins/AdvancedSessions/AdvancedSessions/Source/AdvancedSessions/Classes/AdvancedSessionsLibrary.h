

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
#include "Interfaces/OnlineSessionInterface.h"

#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"

#include "AdvancedSessionsLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedSessionsLog, Log, All);

UCLASS()
class UAdvancedSessionsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions", meta = (WorldContext = "WorldContextObject"))
		static bool KickPlayer(UObject* WorldContextObject, APlayerController* PlayerToKick, FText KickReason);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions", meta = (WorldContext = "WorldContextObject"))
		static bool BanPlayer(UObject* WorldContextObject, APlayerController* PlayerToBan, FText BanReason);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo")
		static void AddOrModifyExtraSettings(UPARAM(ref)  TArray<FSessionPropertyKeyPair> & SettingsArray, UPARAM(ref)  TArray<FSessionPropertyKeyPair> & NewOrChangedSettings, TArray<FSessionPropertyKeyPair> & ModifiedSettingsArray);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo")
		static void GetExtraSettings(FBlueprintSessionResult SessionResult, TArray<FSessionPropertyKeyPair> & ExtraSettings);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (WorldContext = "WorldContextObject"))
		static void GetSessionState(UObject* WorldContextObject, EBPOnlineSessionState &SessionState);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (ExpandEnumAsExecs = "Result", WorldContext = "WorldContextObject"))
		static void GetSessionSettings(UObject* WorldContextObject, int32 &NumConnections, int32 &NumPrivateConnections, bool &bIsLAN, bool &bIsDedicated, bool &bAllowInvites, bool &bAllowJoinInProgress, bool &bIsAnticheatEnabled, int32 &BuildUniqueID, TArray<FSessionPropertyKeyPair> &ExtraSettings, EBlueprintResultSwitch &Result);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (WorldContext = "WorldContextObject"))
		static void IsPlayerInSession(UObject* WorldContextObject, const FBPUniqueNetId &PlayerToCheck, bool &bIsInSession);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo|Literals")
		static FSessionsSearchSetting MakeLiteralSessionSearchProperty(FSessionPropertyKeyPair SessionSearchProperty, EOnlineComparisonOpRedux ComparisonOp);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo")
		static bool IsValidSession(const FBlueprintSessionResult & SessionResult);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo")
		static void GetSessionID_AsString(const FBlueprintSessionResult & SessionResult, FString& SessionID);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo", meta = (WorldContext = "WorldContextObject"))
		static void GetCurrentSessionID_AsString(UObject* WorldContextObject, FString& SessionID);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo")
		static void GetCurrentUniqueBuildID(int32 &UniqueBuildId);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo")
		static void GetUniqueBuildID(FBlueprintSessionResult SessionResult, int32 &UniqueBuildId);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo")
		static FName GetSessionPropertyKey(const FSessionPropertyKeyPair& SessionProperty);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (ExpandEnumAsExecs = "Result"))
		static void FindSessionPropertyByName(const TArray<FSessionPropertyKeyPair>& ExtraSettings, FName SettingsName, EBlueprintResultSwitch &Result, FSessionPropertyKeyPair& OutProperty);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (ExpandEnumAsExecs = "Result"))
		static void FindSessionPropertyIndexByName(const TArray<FSessionPropertyKeyPair>& ExtraSettings, FName SettingName, EBlueprintResultSwitch &Result, int32& OutIndex);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (ExpandEnumAsExecs = "SearchResult"))
		static void GetSessionPropertyByte(const TArray<FSessionPropertyKeyPair> & ExtraSettings, FName SettingName, ESessionSettingSearchResult &SearchResult, uint8 &SettingValue);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (ExpandEnumAsExecs = "SearchResult"))
		static void GetSessionPropertyBool(const TArray<FSessionPropertyKeyPair> & ExtraSettings, FName SettingName, ESessionSettingSearchResult &SearchResult, bool &SettingValue);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (ExpandEnumAsExecs = "SearchResult"))
		static void GetSessionPropertyString(const TArray<FSessionPropertyKeyPair> & ExtraSettings, FName SettingName, ESessionSettingSearchResult &SearchResult, FString &SettingValue);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (ExpandEnumAsExecs = "SearchResult"))
		static void GetSessionPropertyInt(const TArray<FSessionPropertyKeyPair> & ExtraSettings, FName SettingName, ESessionSettingSearchResult &SearchResult, int32 &SettingValue);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|SessionInfo", meta = (ExpandEnumAsExecs = "SearchResult"))
		static void GetSessionPropertyFloat(const TArray<FSessionPropertyKeyPair> & ExtraSettings, FName SettingName, ESessionSettingSearchResult &SearchResult, float &SettingValue);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo|Literals")
		static FSessionPropertyKeyPair MakeLiteralSessionPropertyByte(FName Key, uint8 Value);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo|Literals")
		static FSessionPropertyKeyPair MakeLiteralSessionPropertyBool(FName Key, bool Value);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo|Literals")
		static FSessionPropertyKeyPair MakeLiteralSessionPropertyString(FName Key, FString Value);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo|Literals")
		static FSessionPropertyKeyPair MakeLiteralSessionPropertyInt(FName Key, int32 Value);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|SessionInfo|Literals")
		static FSessionPropertyKeyPair MakeLiteralSessionPropertyFloat(FName Key, float Value);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|PlayerInfo|PlayerID")
		static void GetUniqueNetID(APlayerController *PlayerController, FBPUniqueNetId &UniqueNetId);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|PlayerInfo|PlayerID")
		static void GetUniqueNetIdOfSessionOwner(FBlueprintSessionResult SessionResult, FBPUniqueNetId& UniqueNetId);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|PlayerInfo|PlayerID")
		static void GetUniqueNetIDFromPlayerState(APlayerState *PlayerState, FBPUniqueNetId &UniqueNetId);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|PlayerInfo|PlayerID")
		static bool IsValidUniqueNetID(const FBPUniqueNetId &UniqueNetId);

		UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal Unique Net ID", CompactNodeTitle = "==", Keywords = "== equal"), Category = "Online|AdvancedSessions|PlayerInfo|PlayerID")
		static bool EqualEqual_UNetIDUnetID(const FBPUniqueNetId &A, const FBPUniqueNetId &B);

		UFUNCTION(BlueprintPure, meta = (DisplayName = "ToUniqueNetIDRepl (Unique Net ID)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Online|AdvancedSessions|PlayerInfo|PlayerID")
			static FUniqueNetIdRepl Conv_BPUniqueIDToUniqueNetIDRepl(const FBPUniqueNetId& InUniqueID);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|UniqueNetId")
		static void UniqueNetIdToString(const FBPUniqueNetId &UniqueNetId, FString &String);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|PlayerInfo|PlayerName")
		static void GetPlayerName(APlayerController *PlayerController, FString &PlayerName);

		UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSessions|PlayerInfo|PlayerName")
		static void SetPlayerName(APlayerController *PlayerController, FString PlayerName);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|PlayerInfo|Misc", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", DisplayName = "GetNumNetworkPlayers"))
		static void GetNumberOfNetworkPlayers(UObject* WorldContextObject, int32 &NumNetPlayers);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|PlayerInfo|Misc")
		static void GetNetPlayerIndex(APlayerController *PlayerController, int32 &NetPlayerIndex);

		UFUNCTION(BlueprintPure, Category = "Online|AdvancedSessions|Misc")
		static bool HasOnlineSubsystem(FName SubSystemName);

		UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Online|AdvancedSessions|Seamless", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static bool ServerTravel(UObject* WorldContextObject, const FString& InURL, bool bAbsolute, bool bShouldSkipGameNotify);

};	
