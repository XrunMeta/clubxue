

#pragma once
#include "CoreMinimal.h"
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
#include "BlueprintDataDefinitions.h"
#include "UObject/UObjectIterator.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)

#pragma warning(disable:4265) 
#endif

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

#pragma push_macro("ARRAY_COUNT")
#undef ARRAY_COUNT

#if USING_CODE_ANALYSIS
MSVC_PRAGMA(warning(push))
MSVC_PRAGMA(warning(disable : ALL_CODE_ANALYSIS_WARNINGS))
#endif	

#include <steam/steam_api.h>

#if USING_CODE_ANALYSIS
MSVC_PRAGMA(warning(pop))
#endif	

#include <steam/isteamapps.h>

#pragma pop_macro("ARRAY_COUNT")

#ifdef _MSC_VER
#pragma warning(pop)
#endif

class FUniqueNetIdSteam2 :
	public FUniqueNetId
{
PACKAGE_SCOPE:

	uint64 UniqueNetId;

	FUniqueNetIdSteam2() :
		UniqueNetId(0)
	{
	}

	explicit FUniqueNetIdSteam2(const FUniqueNetIdSteam2& Src) :
		UniqueNetId(Src.UniqueNetId)
	{
	}

public:

	explicit FUniqueNetIdSteam2(uint64 InUniqueNetId) :
		UniqueNetId(InUniqueNetId)
	{
	}

	explicit FUniqueNetIdSteam2(CSteamID InSteamId) :
		UniqueNetId(InSteamId.ConvertToUint64())
	{
	}

	explicit FUniqueNetIdSteam2(const FString& Str) :
		UniqueNetId(FCString::Atoi64(*Str))
	{
	}

	explicit FUniqueNetIdSteam2(const FUniqueNetId& InUniqueNetId) :
		UniqueNetId(*(uint64*)InUniqueNetId.GetBytes())
	{
	}

	virtual FName GetType() const override
	{
		return STEAM_SUBSYSTEM;
	}

	virtual const uint8* GetBytes() const override
	{
		return (uint8*)&UniqueNetId;
	}

	virtual int32 GetSize() const override
	{
		return sizeof(uint64);
	}

	virtual bool IsValid() const override
	{
		return UniqueNetId != 0 && CSteamID(UniqueNetId).IsValid();
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("%llu"), UniqueNetId);
	}

	virtual FString ToDebugString() const override
	{
		CSteamID SteamID(UniqueNetId);
		if (SteamID.IsLobby())
		{
			return FString::Printf(TEXT("Lobby [0x%llX]"), UniqueNetId);
		}
		else if (SteamID.BAnonGameServerAccount())
		{
			return FString::Printf(TEXT("Server [0x%llX]"), UniqueNetId);
		}
		else if (SteamID.IsValid())
		{
			const FString NickName(SteamFriends() ? UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaName(UniqueNetId)) : TEXT("UNKNOWN"));
			return FString::Printf(TEXT("%s [0x%llX]"), *NickName, UniqueNetId);
		}
		else
		{
			return FString::Printf(TEXT("INVALID [0x%llX]"), UniqueNetId);
		}
	}

	virtual uint32 GetTypeHash() const override
	{
		return ::GetTypeHash(UniqueNetId);
	}

	operator CSteamID()
	{
		return UniqueNetId;
	}

	operator const CSteamID() const
	{
		return UniqueNetId;
	}

	operator CSteamID*()
	{
		return (CSteamID*)&UniqueNetId;
	}

	operator const CSteamID*() const
	{
		return (const CSteamID*)&UniqueNetId;
	}

	friend FArchive& operator<<(FArchive& Ar, FUniqueNetIdSteam2& UserId)
	{
		return Ar << UserId.UniqueNetId;
	}
};

#endif

#include "AdvancedSteamFriendsLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedSteamFriendsLog, Log, All);

UENUM(Blueprintable)
enum class SteamAvatarSize : uint8
{
	SteamAvatar_INVALID = 0,
	SteamAvatar_Small = 1,
	SteamAvatar_Medium = 2,
	SteamAvatar_Large = 3
};

UENUM(Blueprintable)
enum class ESteamUserOverlayType : uint8
{

	steamid,

	chat,

	jointrade,

	stats,

	achievements,

	friendadd,

	friendremove,

	friendrequestaccept,

	friendrequestignore,

	invitetolobby
};

static FString EnumToString(const FString& enumName, uint8 value)
{

	const UEnum* EnumPtr = FindFirstObject<UEnum>(*enumName, EFindFirstObjectOptions::None, ELogVerbosity::Warning, TEXT("EumtoString"));

	if (!EnumPtr)
		return FString();

	FString EnumName = EnumPtr->GetNameStringByIndex(value);
	return EnumName;
}

USTRUCT(BlueprintType, Category = "Online|SteamAPI|SteamGroups")
struct FBPSteamGroupInfo
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
		FBPUniqueNetId GroupID; 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
		FString GroupName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
		FString GroupTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
		int32 numOnline = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
		int32 numInGame = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
		int32 numChatting = 0;

};

UENUM(Blueprintable)
enum class EBPTextFilteringContext : uint8
{

	FContext_Unknown = 0,

	FContext_GameContent = 1,

	FContext_Chat = 2,

	FContext_Name = 3
};

UCLASS()
class UAdvancedSteamFriendsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|SteamAPI", meta = (ExpandEnumAsExecs = "Result"))
	static UTexture2D * GetSteamFriendAvatar(const FBPUniqueNetId UniqueNetId, EBlueprintAsyncResultSwitch &Result, SteamAvatarSize AvatarSize = SteamAvatarSize::SteamAvatar_Medium);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|SteamAPI")
	static bool RequestSteamFriendInfo(const FBPUniqueNetId UniqueNetId, bool bRequireNameOnly = false);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|SteamAPI", meta = (WorldContext = "WorldContextObject"))
		static bool OpenSteamUserOverlay(UObject* WorldContextObject, const FBPUniqueNetId UniqueNetId, ESteamUserOverlayType DialogType);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedFriends|SteamAPI")
		static bool IsOverlayEnabled();

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|SteamAPI")
	static int32 GetFriendSteamLevel(const FBPUniqueNetId UniqueNetId);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|SteamAPI")
	static FString GetSteamPersonaName(const FBPUniqueNetId UniqueNetId);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedFriends|SteamAPI")
	static FBPUniqueNetId CreateSteamIDFromString(const FString SteamID64);

	UFUNCTION(BlueprintPure, Category = "Online|AdvancedFriends|SteamAPI")
		static FBPUniqueNetId GetLocalSteamIDFromSteam();

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedFriends|SteamAPI", meta = (ExpandEnumAsExecs = "Result"))
	static void GetSteamFriendGamePlayed(const FBPUniqueNetId UniqueNetId, EBlueprintResultSwitch &Result, int32 & AppID);

	UFUNCTION(BlueprintCallable, Category = "Online|SteamAPI|SteamGroups")
		static void GetSteamGroups(TArray<FBPSteamGroupInfo> & SteamGroups);

	UFUNCTION(BlueprintCallable, Category = "Online|SteamAPI|TextFiltering")
		static bool InitTextFiltering();

	UFUNCTION(BlueprintCallable, Category = "Online|SteamAPI|TextFiltering")
		static bool FilterText(FString TextToFilter, EBPTextFilteringContext Context, const FBPUniqueNetId TextSourceID, FString& FilteredText);

	UFUNCTION(BlueprintPure, Category = "Online|SteamAPI")
		static bool IsSteamInBigPictureMode();
};	
