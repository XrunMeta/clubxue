
#pragma once

#include "CoreMinimal.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "BlueprintDataDefinitions.h"

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

#include "SteamRequestGroupOfficersCallbackProxy.generated.h"

USTRUCT(BlueprintType, Category = "Online|SteamAPI|SteamGroups")
struct FBPSteamGroupOfficer
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
		FBPUniqueNetId OfficerUniqueNetID; 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
		bool bIsOwner = false;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintGroupOfficerDetailsDelegate, const TArray<FBPSteamGroupOfficer> &, OfficerList);

UCLASS(MinimalAPI)
class USteamRequestGroupOfficersCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	virtual ~USteamRequestGroupOfficersCallbackProxy();

	UPROPERTY(BlueprintAssignable)
	FBlueprintGroupOfficerDetailsDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FBlueprintGroupOfficerDetailsDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|SteamAPI|SteamGroups")
	static USteamRequestGroupOfficersCallbackProxy* GetSteamGroupOfficerList(UObject* WorldContextObject, FBPUniqueNetId GroupUniqueNetID);

	virtual void Activate() override;

private:

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	void OnRequestGroupOfficerDetails( ClanOfficerListResponse_t *pResult, bool bIOFailure);
	CCallResult<USteamRequestGroupOfficersCallbackProxy, ClanOfficerListResponse_t> m_callResultGroupOfficerRequestDetails;

#endif

private:

	FBPUniqueNetId GroupUniqueID;
	UObject* WorldContextObject;
};
