

#pragma once
#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
#include "steam/isteamugc.h"
#include "steam/isteamremotestorage.h"
#endif
#include "Interfaces/OnlineSessionInterface.h"

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

#pragma pop_macro("ARRAY_COUNT")

#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "AdvancedSteamWorkshopLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedSteamWorkshopLog, Log, All);

USTRUCT(BlueprintType)
struct FBPSteamWorkshopID
{
	GENERATED_USTRUCT_BODY()

public:

	uint64 SteamWorkshopID;

	FBPSteamWorkshopID()
	{

	}

	FBPSteamWorkshopID(uint64 ID)
	{
		SteamWorkshopID = ID;
	}
};

UENUM(BlueprintType)
enum class FBPSteamResult : uint8
{
	K_EResultInvalid = 0,
	k_EResultOK = 1,							
	k_EResultFail = 2,							
	k_EResultNoConnection = 3,					

	k_EResultInvalidPassword = 5,				
	k_EResultLoggedInElsewhere = 6,				
	k_EResultInvalidProtocolVer = 7,			
	k_EResultInvalidParam = 8,					
	k_EResultFileNotFound = 9,					
	k_EResultBusy = 10,							
	k_EResultInvalidState = 11,					
	k_EResultInvalidName = 12,					
	k_EResultInvalidEmail = 13,					
	k_EResultDuplicateName = 14,				
	k_EResultAccessDenied = 15,					
	k_EResultTimeout = 16,						
	k_EResultBanned = 17,						
	k_EResultAccountNotFound = 18,				
	k_EResultInvalidSteamID = 19,				
	k_EResultServiceUnavailable = 20,			
	k_EResultNotLoggedOn = 21,					
	k_EResultPending = 22,						
	k_EResultEncryptionFailure = 23,			
	k_EResultInsufficientPrivilege = 24,		
	k_EResultLimitExceeded = 25,				
	k_EResultRevoked = 26,						
	k_EResultExpired = 27,						
	k_EResultAlreadyRedeemed = 28,				
	k_EResultDuplicateRequest = 29,				
	k_EResultAlreadyOwned = 30,					
	k_EResultIPNotFound = 31,					
	k_EResultPersistFailed = 32,				
	k_EResultLockingFailed = 33,				
	k_EResultLogonSessionReplaced = 34,
	k_EResultConnectFailed = 35,
	k_EResultHandshakeFailed = 36,
	k_EResultIOFailure = 37,
	k_EResultRemoteDisconnect = 38,
	k_EResultShoppingCartNotFound = 39,			
	k_EResultBlocked = 40,						
	k_EResultIgnored = 41,						
	k_EResultNoMatch = 42,						
	k_EResultAccountDisabled = 43,
	k_EResultServiceReadOnly = 44,				
	k_EResultAccountNotFeatured = 45,			
	k_EResultAdministratorOK = 46,				
	k_EResultContentVersion = 47,				
	k_EResultTryAnotherCM = 48,					
	k_EResultPasswordRequiredToKickSession = 49,
	k_EResultAlreadyLoggedInElsewhere = 50,		
	k_EResultSuspended = 51,					
	k_EResultCancelled = 52,					
	k_EResultDataCorruption = 53,				
	k_EResultDiskFull = 54,						
	k_EResultRemoteCallFailed = 55,				
	k_EResultPasswordUnset = 56,				
	k_EResultExternalAccountUnlinked = 57,		
	k_EResultPSNTicketInvalid = 58,				
	k_EResultExternalAccountAlreadyLinked = 59,	
	k_EResultRemoteFileConflict = 60,			
	k_EResultIllegalPassword = 61,				
	k_EResultSameAsPreviousValue = 62,			
	k_EResultAccountLogonDenied = 63,			
	k_EResultCannotUseOldPassword = 64,			
	k_EResultInvalidLoginAuthCode = 65,			
	k_EResultAccountLogonDeniedNoMail = 66,		
	k_EResultHardwareNotCapableOfIPT = 67,		
	k_EResultIPTInitError = 68,					
	k_EResultParentalControlRestricted = 69,	
	k_EResultFacebookQueryError = 70,			
	k_EResultExpiredLoginAuthCode = 71,			
	k_EResultIPLoginRestrictionFailed = 72,
	k_EResultAccountLockedDown = 73,
	k_EResultAccountLogonDeniedVerifiedEmailRequired = 74,
	k_EResultNoMatchingURL = 75,
	k_EResultBadResponse = 76,					
	k_EResultRequirePasswordReEntry = 77,		
	k_EResultValueOutOfRange = 78,				
	k_EResultUnexpectedError = 79,				
	k_EResultDisabled = 80,						
	k_EResultInvalidCEGSubmission = 81,			
	k_EResultRestrictedDevice = 82,				
	k_EResultRegionLocked = 83,					
	k_EResultRateLimitExceeded = 84,			
	k_EResultAccountLoginDeniedNeedTwoFactor = 85,	
	k_EResultItemDeleted = 86,					
	k_EResultAccountLoginDeniedThrottle = 87,	
	k_EResultTwoFactorCodeMismatch = 88,		
	k_EResultTwoFactorActivationCodeMismatch = 89,	
	k_EResultAccountAssociatedToMultiplePartners = 90,	
	k_EResultNotModified = 91, 
};

UENUM(BlueprintType)
enum class FBPWorkshopFileType : uint8
{
	k_EWorkshopFileTypeCommunity = 0,
	k_EWorkshopFileTypeMicrotransaction = 1,
	k_EWorkshopFileTypeCollection = 2,
	k_EWorkshopFileTypeArt = 3,
	k_EWorkshopFileTypeVideo = 4,
	k_EWorkshopFileTypeScreenshot = 5,
	k_EWorkshopFileTypeGame = 6,
	k_EWorkshopFileTypeSoftware = 7,
	k_EWorkshopFileTypeConcept = 8,
	k_EWorkshopFileTypeWebGuide = 9,
	k_EWorkshopFileTypeIntegratedGuide = 10,
	k_EWorkshopFileTypeMerch = 11,
	k_EWorkshopFileTypeControllerBinding = 12,
	k_EWorkshopFileTypeSteamworksAccessInvite = 13,
	k_EWorkshopFileTypeSteamVideo = 14,

	k_EWorkshopFileTypeMax = 15
};

USTRUCT(BlueprintType)
struct FBPSteamWorkshopItemDetails
{
	GENERATED_USTRUCT_BODY()

public:

	FBPSteamWorkshopItemDetails()
	{
		ResultOfRequest = FBPSteamResult::k_EResultOK;
		FileType = FBPWorkshopFileType::k_EWorkshopFileTypeMax;
		CreatorAppID = 0;
		ConsumerAppID = 0;
		VotesUp = 0;
		VotesDown = 0;
		CalculatedScore = 0.f;
		bBanned = false;
		bAcceptedForUse = false;
		bTagsTruncated = false;
	}

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	FBPSteamWorkshopItemDetails(SteamUGCDetails_t &hUGCDetails)
	{
		ResultOfRequest = (FBPSteamResult)hUGCDetails.m_eResult;
		FileType = (FBPWorkshopFileType)hUGCDetails.m_eFileType;
		CreatorAppID = (int32)hUGCDetails.m_nCreatorAppID;
		ConsumerAppID = (int32)hUGCDetails.m_nConsumerAppID;
		Title = FString(hUGCDetails.m_rgchTitle, k_cchPublishedDocumentTitleMax);
		Description = FString(hUGCDetails.m_rgchDescription, k_cchPublishedDocumentDescriptionMax);
		ItemUrl = FString(hUGCDetails.m_rgchURL, k_cchPublishedFileURLMax);
		VotesUp = (int32)hUGCDetails.m_unVotesUp;
		VotesDown = (int32)hUGCDetails.m_unVotesDown;
		CalculatedScore = hUGCDetails.m_flScore;
		bBanned = hUGCDetails.m_bBanned;
		bAcceptedForUse = hUGCDetails.m_bAcceptedForUse;
		bTagsTruncated = hUGCDetails.m_bTagsTruncated;

		CreatorSteamID = FString::Printf(TEXT("%llu"), hUGCDetails.m_ulSteamIDOwner);
	}

	FBPSteamWorkshopItemDetails(const SteamUGCDetails_t &hUGCDetails)
	{
		ResultOfRequest = (FBPSteamResult)hUGCDetails.m_eResult;
		FileType = (FBPWorkshopFileType)hUGCDetails.m_eFileType;
		CreatorAppID = (int32)hUGCDetails.m_nCreatorAppID;
		ConsumerAppID = (int32)hUGCDetails.m_nConsumerAppID;
		Title = FString(hUGCDetails.m_rgchTitle, k_cchPublishedDocumentTitleMax);
		Description = FString(hUGCDetails.m_rgchDescription, k_cchPublishedDocumentDescriptionMax);
		ItemUrl = FString(hUGCDetails.m_rgchURL, k_cchPublishedFileURLMax);
		VotesUp = (int32)hUGCDetails.m_unVotesUp;
		VotesDown = (int32)hUGCDetails.m_unVotesDown;
		CalculatedScore = hUGCDetails.m_flScore;
		bBanned = hUGCDetails.m_bBanned;
		bAcceptedForUse = hUGCDetails.m_bAcceptedForUse;
		bTagsTruncated = hUGCDetails.m_bTagsTruncated;

		CreatorSteamID = FString::Printf(TEXT("%llu"), hUGCDetails.m_ulSteamIDOwner);
	}
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
		FBPSteamResult ResultOfRequest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
		FBPWorkshopFileType FileType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
		int32 CreatorAppID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
		int32 ConsumerAppID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
		FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
		FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
		FString ItemUrl;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
	int32 VotesUp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
	int32 VotesDown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
	float CalculatedScore;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
	bool bBanned;													

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
	bool bAcceptedForUse;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
	bool bTagsTruncated;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|AdvancedSteamWorkshop")
	FString CreatorSteamID;

};

UCLASS()
class UAdvancedSteamWorkshopLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSteamWorkshop")
	static TArray<FBPSteamWorkshopID> GetSubscribedWorkshopItems(int32 & NumberOfItems);

	UFUNCTION(BlueprintCallable, Category = "Online|AdvancedSteamWorkshop")
	static void GetNumSubscribedWorkshopItems(int32 & NumberOfItems);

};	
