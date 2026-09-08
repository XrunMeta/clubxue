
#include "AdvancedSteamWorkshopLibrary.h"
#include "OnlineSubSystemHeader.h"

DEFINE_LOG_CATEGORY(AdvancedSteamWorkshopLog);

void UAdvancedSteamWorkshopLibrary::GetNumSubscribedWorkshopItems(int32 & NumberOfItems)
{
	NumberOfItems = 0;
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

	if (SteamAPI_Init())
	{
		NumberOfItems = SteamUGC()->GetNumSubscribedItems();
		return;
	}
	else
	{
		UE_LOGF(AdvancedSteamWorkshopLog, Warning, "Error in GetNumSubscribedWorkshopItemCount : SteamAPI is not Inited!");
		return;
	}
#else
	UE_LOGF(AdvancedSteamWorkshopLog, Warning, "Error in GetNumSubscribedWorkshopItemCount : Called on an incompatible platform");
	return;
#endif
}

TArray<FBPSteamWorkshopID> UAdvancedSteamWorkshopLibrary::GetSubscribedWorkshopItems(int32 & NumberOfItems)
{
	TArray<FBPSteamWorkshopID> outArray;
	NumberOfItems = 0;

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

	if (SteamAPI_Init())
	{
		uint32 NumItems = SteamUGC()->GetNumSubscribedItems();

		if (NumItems == 0)
			return outArray;

		NumberOfItems = NumItems;

		PublishedFileId_t *fileIds = new PublishedFileId_t[NumItems];

		uint32 subItems = SteamUGC()->GetSubscribedItems(fileIds, NumItems);

		for (uint32 i = 0; i < subItems; ++i)
		{
			outArray.Add(FBPSteamWorkshopID(fileIds[i]));
		}

		delete[] fileIds;

		return outArray;
	}
	else
	{
		UE_LOGF(AdvancedSteamWorkshopLog, Warning, "Error in GetSubscribedWorkshopItemCount : SteamAPI is not Inited!");
		return outArray;
	}
#else
	UE_LOGF(AdvancedSteamWorkshopLog, Warning, "Error in GetSubscribedWorkshopItemCount : Called on an incompatible platform");
	return outArray;
#endif
}
