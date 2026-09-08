

#pragma once
#include "CoreMinimal.h"
#include "FindSessionsCallbackProxy.h"
#include "OnlineLeaderboardsEOS.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/OnlineSessionNames.h"
#endif
#ifdef PLAYFAB_PLUGIN_INSTALLED
#include "PlayFab.h"
#include "Core/PlayFabError.h"
#include "Core/PlayFabClientDataModels.h"
#endif
#include "EIK_Subsystem.generated.h"

UENUM(BlueprintType)
enum EEIKAttributeType
{
	String,
	Bool,
	Integer
};

USTRUCT(BlueprintType)
struct FEIKAttribute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit")
	TEnumAsByte<EEIKAttributeType> AttributeType = EEIKAttributeType::String;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit")
	FString StringValue = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit")
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit")
	int32 IntValue = 0;

	FEIKAttribute()
	{
		AttributeType = EEIKAttributeType::String;
		StringValue = "";
		BoolValue = false;
		IntValue = 0;
	}
	FVariantData GetVariantData() const
	{
		FVariantData VariantData;
		switch (AttributeType)
		{
		case EEIKAttributeType::String:
			VariantData.SetValue(StringValue);
			break;
		case EEIKAttributeType::Bool:
			VariantData.SetValue(BoolValue);
			break;
		case EEIKAttributeType::Integer:
			VariantData.SetValue(IntValue);
			break;
		default:
			VariantData.SetValue(StringValue);
			break;
		}
		return VariantData;
	}

	FEIKAttribute(FVariantData VariantData)
	{		
		switch (VariantData.GetType())
		{
		case EOnlineKeyValuePairDataType::String:
			AttributeType = EEIKAttributeType::String;
			VariantData.GetValue(StringValue);
			break;
		case EOnlineKeyValuePairDataType::Bool:
			AttributeType = EEIKAttributeType::Bool;
			VariantData.GetValue(BoolValue);
			break;
		case EOnlineKeyValuePairDataType::Int32:
			AttributeType = EEIKAttributeType::Integer;
			VariantData.GetValue(IntValue);
			break;
		case EOnlineKeyValuePairDataType::Int64:
			AttributeType = EEIKAttributeType::Integer;
			int64 Int64Value;
			VariantData.GetValue(Int64Value);
			IntValue = Int64Value;
			break;
		default:
			AttributeType = EEIKAttributeType::String;
			VariantData.GetValue(StringValue);
			break;
		}
	}

};

USTRUCT(BlueprintType)
struct FOffersStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	FText LongDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	FText RegularPriceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	int64 RegularPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	FText PriceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	int64 NumericPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	FDateTime ReleaseDate = FDateTime::MinValue();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EIK Nodes")
	FDateTime ExpirationDate = FDateTime::MinValue();
};

USTRUCT(BlueprintType)
struct FSessionFindStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	FBlueprintSessionResult SessionResult = FBlueprintSessionResult();

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	TMap<FString, FEIKAttribute> SessionSettings = TMap<FString, FEIKAttribute>();

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	FString SessionName = FString();

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	int32 CurrentNumberOfPlayers = 0;

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	int32 MaxNumberOfPlayers = 0;

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	bool bIsDedicatedServer = false;
};

USTRUCT(BlueprintType)
struct FFileListStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	FString Hash = FString();

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	FName HashType = FName();

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	FString DLName = FString();

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	FString FileName = FString();

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	int32 FileSize = 0;

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	FString URL = FString();

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	int32 iChunkID = 0;

	UPROPERTY(BlueprintReadWrite, Category="EOS Struct")
	TMap<FString, FString> ExternalStorageIds = TMap<FString, FString>();
};

UENUM(BlueprintType)
enum class ERegionInfo : uint8 {
	RE_NoSelection       UMETA(DisplayName="No Selection"),
	RE_Asia               UMETA(DisplayName="Asia"),
	RE_NorthAmerica        UMETA(DisplayName="North America"),
	RE_SouthAmerica        UMETA(DisplayName="South America"),
	RE_Africa              UMETA(DisplayName="Africa"),
	RE_Europe              UMETA(DisplayName="Europe"),
	RE_Australia           UMETA(DisplayName="Australia"),

};

UENUM(BlueprintType)
enum class EMatchType : uint8 {
	MT_MatchMakingSession       UMETA(DisplayName="Matchmaking Session"),
	MT_Lobby					UMETA(DisplayName="Lobby Session"),
};

USTRUCT(BlueprintType)
struct FEIKUniqueNetId
{
	GENERATED_USTRUCT_BODY()

private:
	bool bUseDirectPointer;

public:
	TSharedPtr<const FUniqueNetId> UniqueNetId;
	const FUniqueNetId * UniqueNetIdPtr;

	void SetUniqueNetId(const TSharedPtr<const FUniqueNetId> &ID)
	{
		bUseDirectPointer = false;
		UniqueNetIdPtr = nullptr;
		UniqueNetId = ID;
	}

	void SetUniqueNetId(const FUniqueNetId *ID)
	{
		bUseDirectPointer = true;
		UniqueNetIdPtr = ID;
	}

	bool IsValid() const
	{
		if (bUseDirectPointer && UniqueNetIdPtr != nullptr && UniqueNetIdPtr->IsValid())
		{
			return true;
		}
		else if (UniqueNetId.IsValid())
		{
			return true;
		}
		else
			return false;

	}

	const FUniqueNetId* GetUniqueNetId() const
	{
		if (bUseDirectPointer && UniqueNetIdPtr != nullptr)
		{

			return (UniqueNetIdPtr);
		}
		else if (UniqueNetId.IsValid())
		{
			return UniqueNetId.Get();
		}
		else
			return nullptr;
	}

	FORCEINLINE bool operator==(const FEIKUniqueNetId& Other) const
	{
		return (IsValid() && Other.IsValid() && (*GetUniqueNetId() == *Other.GetUniqueNetId()));
	}

	FORCEINLINE bool operator!=(const FEIKUniqueNetId& Other) const
	{
		return !(IsValid() && Other.IsValid() && (*GetUniqueNetId() == *Other.GetUniqueNetId()));
	}

	FEIKUniqueNetId()
	{
		bUseDirectPointer = false;
		UniqueNetIdPtr = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FEIK_Stats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="EIK Struct")
	FString StatsName;

	UPROPERTY(BlueprintReadWrite, Category="EIK Struct")
	FString StatsValue;

};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_Login_Callback, bool, bWasSuccess,const FString&,Error);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_Logout_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_CreateSession_Callback, bool, bWasSuccess, const FName&,SessionName);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_CreateLobby_Callback, bool, bWasSuccess, const FName&,SessionName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_DestroySession_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_PurchaseOffer_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_JoinSession_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_GetTitleFile_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_UpdateStat_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_GetStats_Callback, bool, bWasSuccess, const TArray<FEIK_Stats>&,Stats);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_FindSession_Callback, bool, bWasSuccess, const TArray<FSessionFindStruct>&, SessionResults);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_WriteFile_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_GetOffers_Callback, bool, bWasSuccess, const TArray<FOffersStruct>&, Offers);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_GetOwnedItems_Callback, bool, bWasSuccess, const TArray<FString>&, OwnedItemNames);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_GetFile_Callback, bool, bWasSuccess, USaveGame*,SaveGame);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_ConnectEOSAndPlayFab_Callback, bool, bWasSuccess, const FString&, Error);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_TitleFileList_Callback, bool, bWasSuccess, const FString&, Error);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FBP_HostMigration_Callback, bool, bLocalHost, const FString&, PromotedMember, const FString&, JoinAddress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBP_FriendInviteRecievedDelegate, const FEIKUniqueNetId&, LocalUserId, const FEIKUniqueNetId&, InvitedUserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBP_SessionInviteRecievedDelegate, const FString&, SessionInfo, const FString&, LocalProductId, const FString&, InvitedProductId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEIK_OnSessionUserInviteAccepted, bool, bWasSuccesfull, const FBlueprintSessionResult&, AcceptedSession);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Subsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEIK_Subsystem();

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Login")
	void LoginWithDeviceID(int32 LocalUserNum, FString DisplayName, FString DeviceName, const FBP_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Login")
	void LoginWithAccountPortal(int32 LocalUserNum, const FBP_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Login")
	void LoginWithSteam(int32 LocalUserNum, const FBP_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Login")
	void LoginWithPersistantAuth(int32 LocalUserNum, const FBP_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Login")
	void LoginWithDeveloperTool(int32 LocalUserNum, FString LocalIP, FString Credential, const FBP_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Login")
	void LoginWithEpicLauncher(int32 LocalUserNum, const FBP_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Login")
	void Logout(int32 LocalUserNum, const FBP_Logout_Callback& Result);

	UFUNCTION(BlueprintPure, Category="EOS Integration Kit || Extra")
	static FString GetPlayerNickname(const int32 LocalUserNum);

	UFUNCTION(BlueprintPure, Category="EOS Integration Kit || Extra")
	static bool GetLoginStatus(const int32 LocalUserNum);

	UFUNCTION(BlueprintCallable,DisplayName="Create EOS Session", Category="EOS Integration Kit || Sessions")
	void CreateEOSSession( const FBP_CreateSession_Callback& Result, TMap<FString, FString> Custom_Settings, FString SessionName = "Modified_EOS_Session", 
								bool bIsDedicatedServer = false, 
								bool bIsLan = false,	
								int32 NumberOfPublicConnections = 4, 
								ERegionInfo Region = ERegionInfo::RE_NoSelection);

	UFUNCTION(BlueprintCallable,DisplayName="Create EOS Lobby", Category="EOS Integration Kit || Sessions")
	void CreateEOSLobby( const FBP_CreateLobby_Callback& Result, TMap<FString, FString> Custom_Settings, FString SessionName = "Modified_EOS_Session",
							  bool bUseVoiceChat = true,
							  bool bUsePresence = true,
							  bool bAllowInvites = true,
							  bool bAdvertise = true,
							  bool bAllowJoinInProgress = true, 
							  bool bIsLan = false, 
							  int32 NumberOfPublicConnections = 4,
							  int32 NumberOfPrivateConnections = 4);

	UFUNCTION(BlueprintCallable, DisplayName="Find EOS Session", Category="EOS Integration Kit || Sessions")
	void FindEOSSession(const FBP_FindSession_Callback& Result, TMap<FString, FString> Search_Settings, EMatchType MatchType = EMatchType::MT_Lobby, ERegionInfo RegionToSearch = ERegionInfo::RE_NoSelection);

	UFUNCTION(BlueprintCallable, DisplayName="Destroy EOS Session", Category="EOS Integration Kit || Sessions")
	void DestroyEosSession(const FBP_DestroySession_Callback& Result, FName SessionName);

	UFUNCTION(BlueprintCallable, DisplayName="Join EOS Session", Category="EOS Integration Kit || Sessions")
	void JoinEosSession(const FBP_JoinSession_Callback& Result, FName SessionName, bool bIsDedicatedServerSession, FBlueprintSessionResult SessionResult);

	UFUNCTION(BlueprintPure, DisplayName="Get User Unique NetID", Category="EOS Integration Kit || Extra")
	FEIKUniqueNetId GetUserUniqueID() const;

	UFUNCTION(BlueprintPure, DisplayName="Get Product UserID", Category="EOS Integration Kit || Extra")
	static FString GetProductUserID(const FEIKUniqueNetId& UniqueNetId);

	UFUNCTION(BlueprintPure, DisplayName="Get Epic ID", Category="EOS Integration Kit || Extra")
	static FString GetEpicID(const FEIKUniqueNetId& UniqueNetId);

	UFUNCTION(BlueprintCallable, DisplayName="Unregister Players", Category="EOS Integration Kit || Sessions")
	void UnRegisterPlayer(FName SessionName);

	UFUNCTION(BlueprintCallable, DisplayName="Register Players", Category="EOS Integration Kit || Sessions")
	void RegisterPlayer(FName SessionName, bool bWasInvited);

	UFUNCTION(BlueprintCallable, DisplayName="Start EOS Session", Category="EOS Integration Kit || Sessions")
	void StartSession(FName SessionName);

	UFUNCTION(BlueprintCallable, DisplayName="End EOS Session", Category="EOS Integration Kit || Sessions")
	void EndSession(FName SessionName);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Friend")
	bool ShowFriendUserInterface();

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Statistics")
	void UpdateStats(const FBP_UpdateStat_Callback& Result, FString StatName, int32 Amount);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Statistics")
	void GetStats(const FBP_GetStats_Callback& Result, TArray<FString> StatName);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Data")
	void SetPlayerData(const FBP_WriteFile_Callback& Result, FString FileName, USaveGame* SavedGame);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Data")
	void GetPlayerData(const FBP_GetFile_Callback& Result, FString FileName);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Data")
	void EnumerateTitleFiles(const FBP_TitleFileList_Callback& Result);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Data")
	TArray<FFileListStruct> GetTitleFileList();

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Data")
	void GetTitleFile(const FBP_GetTitleFile_Callback& Result, FString FileName);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Data")
	TArray<uint8> GetTitleFileContent(FString FileName);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Leaderboard")
	void GetLeaderboard(const FBP_GetFile_Callback& Result, FName LeaderboardName, int32 Rank, int32 Range);

	UFUNCTION(BlueprintCallable,DisplayName="Connect EOS And PlayFab", Category="EOS Integration Kit || PlayFab")
	void ConnectEosAndPlayFab( const FBP_ConnectEOSAndPlayFab_Callback& Result);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Store")
	void PurchaseItem(const FBP_PurchaseOffer_Callback& Result, FString ItemID);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Store")
	void QueryOffers(const FBP_GetOffers_Callback& Result);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Store")
	void GetOwnedItems(const FBP_GetOwnedItems_Callback& Result);

	UFUNCTION(BlueprintPure, Category="EOS Integration Kit || Extra")
	FString GenerateSessionCode(int32 CodeLength = 9) const;

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit || Sessions")
	static bool OnHostMigrated(const FBP_HostMigration_Callback& Result);

	void Login(int32 LocalUserNum, FString ID, FString Token, FString Type, const FBP_Login_Callback& Result);

	void LoginCallback(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error) const;
	void LogoutCallback(int32 LocalUserNum, bool bWasSuccess) const;
	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful) const;
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful) const;
	void OnFindSessionCompleted(bool bWasSuccess) const;
	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccess) const;
	void OnUpdateStatsCompleted(const FOnlineError& Result) const;
	void OnGetStatsCompleted(const FOnlineError &ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult) const;
	void OnWriteFileComplete(bool bSuccess, const FUniqueNetId& UserID, const FString& FileName) const;
	void OnGetFileComplete(bool bSuccess, const FUniqueNetId& UserID, const FString& FileName) const;
	void OnTitleFileListComplete(bool bSuccess, const FString& Error) const;
	void OnTitleFileComplete(bool bSuccess, const FString& FileName) const;
	void OnLeaderboardListCompleted(bool bWasSuccess) const;

	FOnlineLeaderboardReadRef ReadRef = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();

#ifdef PLAYFAB_PLUGIN_INSTALLED

	void OnLoginWithEpicIDPFSuccess(const PlayFab::ClientModels::FLoginResult& Result) const
	{
		UE_LOG(LogTemp, Log, TEXT("Congratulations, you made your first successful API call!"));
		ConnectEosAndPlayFab_CallbackBP.ExecuteIfBound(true, "");
	}

	void OnLoginWithEpicIDPFFailure(const PlayFab::FPlayFabCppError& ErrorResult) const
	{
		UE_LOG(LogTemp, Error, TEXT("Something went wrong with your first API call.\nHere's some debug information:\n%s"), *ErrorResult.GenerateErrorReport());
		ConnectEosAndPlayFab_CallbackBP.ExecuteIfBound(false, TEXT("Something went wrong."));
	}
#endif

	FBP_Login_Callback LoginCallBackBP;
	FBP_Logout_Callback LogoutCallbackBP;
	FBP_CreateSession_Callback CreateSession_CallbackBP;
	FBP_CreateLobby_Callback CreateLobby_CallbackBP;
	FBP_JoinSession_Callback JoinSession_CallbackBP;
	FBP_FindSession_Callback FindSession_CallbackBP;
	FBP_DestroySession_Callback DestroySession_CallbackBP;
	FBP_ConnectEOSAndPlayFab_Callback ConnectEosAndPlayFab_CallbackBP;
	FBP_UpdateStat_Callback UpdateStat_CallbackBP;
	FBP_GetStats_Callback GetStats_CallbackBP;
	FBP_GetFile_Callback GetFile_CallbackBP;
	FBP_GetOwnedItems_Callback GetOwnedItems_CallbackBP;
	FBP_PurchaseOffer_Callback PurchaseOffer_CallbackBP;
	FBP_GetOffers_Callback GetOffers_CallbackBP;
	FBP_WriteFile_Callback WriteFile_CallbackBP;
	FBP_TitleFileList_Callback TitleFileList_CallbackBP;
	FBP_GetTitleFile_Callback GetTitleFile_CallbackBP;

	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;

	UPROPERTY(BlueprintAssignable, DisplayName="On Session User Invite Accepted")
	FEIK_OnSessionUserInviteAccepted OnSessionUserInviteAccepted;

	void OnFriendInviteAcceptedDestroySession(FName Name, bool bArg);
	void OnSessionUserInviteAccepted12(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	void FuncEK_OnSessionUserInviteAccepted(bool bArg, int I, TSharedPtr<const FUniqueNetId> UniqueNetId, const FOnlineSessionSearchResult& OnlineSessionSearchResult);

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FString LocalPortInfo;
	bool Local_bIsDedicatedServerSession = false;

};

