#pragma once
#include "CoreMinimal.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerState.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemUtilsModule.h"
#include "GameFramework/PlayerController.h"
#include "Modules/ModuleManager.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "FindSessionsCallbackProxy.h"

#include "BlueprintDataDefinitions.generated.h"	

UENUM(BlueprintType)
enum class EBPUserPrivileges : uint8
{

	CanPlay,

	CanPlayOnline,

	CanCommunicateOnline,

	CanUseUserGeneratedContent
};

UENUM(BlueprintType)
enum class EBPLoginStatus : uint8
{

	NotLoggedIn,

	UsingLocalProfile,

	LoggedIn
};

USTRUCT(BlueprintType)
struct FBPUserOnlineAccount
{
	GENERATED_USTRUCT_BODY()

public:
	TSharedPtr<FUserOnlineAccount> UserAccountInfo;

	FBPUserOnlineAccount()
	{

	}

	FBPUserOnlineAccount(TSharedPtr<FUserOnlineAccount> UserAccount)
	{
		UserAccountInfo = UserAccount;
	}
};

UENUM()
enum class ESessionSettingSearchResult : uint8
{

	Found,

	NotFound,

	WrongType
};

UENUM()
enum class EBlueprintResultSwitch : uint8
{

	OnSuccess,

	OnFailure
};

UENUM()
enum class EBlueprintAsyncResultSwitch : uint8
{

	OnSuccess,

	AsyncLoading,

	OnFailure
};

UENUM(BlueprintType)
enum class EBPServerPresenceSearchType : uint8
{
	AllServers,
	ClientServersOnly,
	DedicatedServersOnly
};

UENUM(BlueprintType)
enum class EBPOnlinePresenceState : uint8
{
	Online,
	Offline,
	Away,
	ExtendedAway,
	DoNotDisturb,
	Chat
};

UENUM(BlueprintType)
enum class EBPOnlineSessionState : uint8
{

	NoSession,

	Creating,

	Pending,

	Starting,

	InProgress,

	Ending,

	Ended,

	Destroying
};

USTRUCT(BlueprintType)
struct FBPUniqueNetId
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

	FORCEINLINE bool operator==(const FBPUniqueNetId& Other) const
	{
		return (IsValid() && Other.IsValid() && (*GetUniqueNetId() == *Other.GetUniqueNetId()));
	}

	FORCEINLINE bool operator!=(const FBPUniqueNetId& Other) const
	{
		return !(IsValid() && Other.IsValid() && (*GetUniqueNetId() == *Other.GetUniqueNetId()));
	}

	FBPUniqueNetId()
	{
		bUseDirectPointer = false;
		UniqueNetIdPtr = nullptr;
	}
};

USTRUCT(BluePrintType)
struct FBPOnlineUser
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		FBPUniqueNetId UniqueNetId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		FString DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		FString RealName;
};

USTRUCT(BluePrintType)
struct FBPOnlineRecentPlayer : public FBPOnlineUser
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		FString LastSeen;
};

USTRUCT(BlueprintType)
struct FBPFriendPresenceInfo
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		bool bIsOnline = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		bool bIsPlaying = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		bool bIsPlayingThisGame = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		bool bIsJoinable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		bool bHasVoiceSupport = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		EBPOnlinePresenceState PresenceState = EBPOnlinePresenceState::Offline;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
		FString StatusString;

	FBPFriendPresenceInfo()
	{
		bIsOnline = false;
		bIsPlaying = false;
		bIsPlayingThisGame = false;
		bIsJoinable = false;
		bHasVoiceSupport = false;
		PresenceState = EBPOnlinePresenceState::Offline;
	}
};

USTRUCT(BlueprintType)
struct FBPFriendInfo
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	FString DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	FString RealName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	EBPOnlinePresenceState OnlineState = EBPOnlinePresenceState::Offline;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	FBPUniqueNetId UniqueNetId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	bool bIsPlayingSameGame = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	FBPFriendPresenceInfo PresenceInfo;

	FBPFriendInfo()
	{
		OnlineState = EBPOnlinePresenceState::Offline;
		bIsPlayingSameGame = false;
	}
};

UENUM(BlueprintType)
enum class EOnlineComparisonOpRedux : uint8
{
	Equals,
	NotEquals,
	GreaterThan,
	GreaterThanEquals,
	LessThan,
	LessThanEquals,
};

USTRUCT(BlueprintType)
struct FSessionPropertyKeyPair
{
	GENERATED_USTRUCT_BODY()

	FName Key;
	FVariantData Data;
};

USTRUCT(BlueprintType)
struct FSessionsSearchSetting
{
	GENERATED_USTRUCT_BODY()

	EOnlineComparisonOpRedux ComparisonOp;

	FSessionPropertyKeyPair PropertyKeyPair;
};

struct FOnlineSubsystemBPCallHelperAdvanced
{
public:
	FOnlineSubsystemBPCallHelperAdvanced(const TCHAR* CallFunctionContext, UWorld* World, FName SystemName = NAME_None)
		: OnlineSub(Online::GetSubsystem(World, SystemName))
		, FunctionContext(CallFunctionContext)
	{
		if (OnlineSub == nullptr)
		{
			FFrame::KismetExecutionMessage(*FString::Printf(TEXT("%s - Invalid or uninitialized OnlineSubsystem"), FunctionContext), ELogVerbosity::Warning);
		}
	}

	void QueryIDFromPlayerController(APlayerController* PlayerController)
	{
		UserID.Reset();

		if (APlayerState* PlayerState = (PlayerController != NULL) ? PlayerController->PlayerState : NULL)
		{
			UserID = PlayerState->GetUniqueId().GetUniqueNetId();
			if (!UserID.IsValid())
			{
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("%s - Cannot map local player to unique net ID"), FunctionContext), ELogVerbosity::Warning);
			}
		}
		else
		{
			FFrame::KismetExecutionMessage(*FString::Printf(TEXT("%s - Invalid player state"), FunctionContext), ELogVerbosity::Warning);
		}
	}

	bool IsValid() const
	{
		return UserID.IsValid() && (OnlineSub != nullptr);
	}

public:

	TSharedPtr< const FUniqueNetId> UserID;
	IOnlineSubsystem* const OnlineSub;
	const TCHAR* FunctionContext;
};
class FOnlineSearchSettingsEx : public FOnlineSearchSettings
{

public:

	void HardSet(FName Key, const FVariantData& Value, EOnlineComparisonOpRedux CompOp)
	{
		FOnlineSessionSearchParam* SearchParam = SearchParams.Find(Key);

		TEnumAsByte<EOnlineComparisonOp::Type> op;

		switch (CompOp)
		{
		case EOnlineComparisonOpRedux::Equals: op = EOnlineComparisonOp::Equals; break;
		case EOnlineComparisonOpRedux::GreaterThan: op = EOnlineComparisonOp::GreaterThan; break;
		case EOnlineComparisonOpRedux::GreaterThanEquals: op = EOnlineComparisonOp::GreaterThanEquals; break;
		case EOnlineComparisonOpRedux::LessThan: op = EOnlineComparisonOp::LessThan; break;
		case EOnlineComparisonOpRedux::LessThanEquals: op = EOnlineComparisonOp::LessThanEquals; break;
		case EOnlineComparisonOpRedux::NotEquals: op = EOnlineComparisonOp::NotEquals; break;
		default: op = EOnlineComparisonOp::Equals; break;
		}

		if (SearchParam)
		{
			SearchParam->Data = Value;
			SearchParam->ComparisonOp = op;
		}
		else
		{
			FOnlineSessionSearchParam searchSetting((int)0, op);
			searchSetting.Data = Value;
			SearchParams.Add(Key, searchSetting);
		}
	}
};

#define INVALID_INDEX -1