
#pragma once

#include "CoreMinimal.h"
#include "AdvancedSteamWorkshopLibrary.h"
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

#include "SteamWSRequestUGCDetailsCallbackProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintWorkshopDetailsDelegate, const FBPSteamWorkshopItemDetails&, WorkShopDetails);

UCLASS(MinimalAPI)
class USteamWSRequestUGCDetailsCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FBlueprintWorkshopDetailsDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FBlueprintWorkshopDetailsDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedSteamWorkshop")
	static USteamWSRequestUGCDetailsCallbackProxy* GetWorkshopItemDetails(UObject* WorldContextObject, FBPSteamWorkshopID WorkShopID);

	virtual void Activate() override;

private:

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

	void OnUGCRequestUGCDetails(SteamUGCQueryCompleted_t *pResult, bool bIOFailure);
	CCallResult<USteamWSRequestUGCDetailsCallbackProxy, SteamUGCQueryCompleted_t> m_callResultUGCRequestDetails;

#endif

private:

	FBPSteamWorkshopID WorkShopID;
	UObject* WorldContextObject;
};
