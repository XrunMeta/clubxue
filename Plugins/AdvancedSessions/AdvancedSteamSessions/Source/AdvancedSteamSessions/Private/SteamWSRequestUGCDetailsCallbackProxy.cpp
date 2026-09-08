

#include "SteamWSRequestUGCDetailsCallbackProxy.h"
#include "OnlineSubSystemHeader.h"
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
#include "steam/isteamugc.h"
#endif

USteamWSRequestUGCDetailsCallbackProxy::USteamWSRequestUGCDetailsCallbackProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

USteamWSRequestUGCDetailsCallbackProxy* USteamWSRequestUGCDetailsCallbackProxy::GetWorkshopItemDetails(UObject* WorldContextObject, FBPSteamWorkshopID WorkShopID)
{
	USteamWSRequestUGCDetailsCallbackProxy* Proxy = NewObject<USteamWSRequestUGCDetailsCallbackProxy>();

	Proxy->WorkShopID = WorkShopID;
	return Proxy;
}

void USteamWSRequestUGCDetailsCallbackProxy::Activate()
{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (SteamAPI_Init())
	{

		UGCQueryHandle_t hQueryHandle = SteamUGC()->CreateQueryUGCDetailsRequest((PublishedFileId_t *)&WorkShopID.SteamWorkshopID, 1);

		SteamAPICall_t hSteamAPICall = SteamUGC()->SendQueryUGCRequest(hQueryHandle);

		SteamUGC()->ReleaseQueryUGCRequest(hQueryHandle);

		if (hSteamAPICall == k_uAPICallInvalid)
		{
			OnFailure.Broadcast(FBPSteamWorkshopItemDetails());
			return;
		}

		m_callResultUGCRequestDetails.Set(hSteamAPICall, this, &USteamWSRequestUGCDetailsCallbackProxy::OnUGCRequestUGCDetails);
		return;
	}
#endif
	OnFailure.Broadcast(FBPSteamWorkshopItemDetails());
}

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
void USteamWSRequestUGCDetailsCallbackProxy::OnUGCRequestUGCDetails(SteamUGCQueryCompleted_t *pResult, bool bIOFailure)
{	

	if (bIOFailure || !pResult || pResult->m_unNumResultsReturned <= 0)
	{

		{

				OnFailure.Broadcast(FBPSteamWorkshopItemDetails());

		}

		return;
	}
	if (SteamAPI_Init())
	{
		SteamUGCDetails_t Details;
		if (SteamUGC()->GetQueryUGCResult(pResult->m_handle, 0, &Details))
		{

			{

					OnSuccess.Broadcast(FBPSteamWorkshopItemDetails(Details));

			}

			return;
		}
	}
	else
	{

		{

				OnFailure.Broadcast(FBPSteamWorkshopItemDetails());

		}
	}

}
#endif

