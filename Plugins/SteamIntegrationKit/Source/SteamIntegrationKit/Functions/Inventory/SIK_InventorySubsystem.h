

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SIK_SharedFile.h"
#include "SIK_InventorySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSteamInventoryDefinitionUpdateDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSteamInventoryEligiblePromoItemDefIDsDelegate, TEnumAsByte<ESIK_Result>, Result, FSIK_SteamId, SteamID, int32, ItemDefs, bool, CachedData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSteamInventoryFullUpdateDelegate, FSIK_SteamInventoryResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSteamInventoryResultReadyDelegate, FSIK_SteamInventoryResult, Result, TEnumAsByte<ESIK_Result>, ResultType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSteamInventoryStartPurchaseResultDelegate, TEnumAsByte<ESIK_Result>, Result, int64, OrderID, int64, TransID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSteamInventoryRequestPricesResultDelegate, TEnumAsByte<ESIK_Result>, Result, FString, Currency);
UCLASS()
class STEAMINTEGRATIONKIT_API USIK_InventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	USIK_InventorySubsystem();
	~USIK_InventorySubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit || Inventory || Callbacks")
	FSteamInventoryDefinitionUpdateDelegate OnSteamInventoryDefinitionUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit || Inventory || Callbacks")
	FSteamInventoryEligiblePromoItemDefIDsDelegate OnSteamInventoryEligiblePromoItemDefIDs;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit || Inventory || Callbacks")
	FSteamInventoryFullUpdateDelegate OnSteamInventoryFullUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit || Inventory || Callbacks")
	FSteamInventoryResultReadyDelegate OnSteamInventoryResultReady;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit || Inventory || Callbacks")
	FSteamInventoryStartPurchaseResultDelegate OnSteamInventoryStartPurchaseResult;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit || Inventory || Callbacks")
	FSteamInventoryRequestPricesResultDelegate OnSteamInventoryRequestPricesResult;

private:
#if (WITH_ENGINE_STEAM && ONLINESUBSYSTEMSTEAM_PACKAGE) || (WITH_STEAMKIT && !WITH_ENGINE_STEAM)
	STEAM_CALLBACK_MANUAL(USIK_InventorySubsystem, OnSteamInventoryDefinitionUpdateCallback, SteamInventoryDefinitionUpdate_t, m_CallbackSteamInventoryDefinitionUpdate);
	STEAM_CALLBACK_MANUAL(USIK_InventorySubsystem, OnSteamInventoryEligiblePromoItemDefIDsCallback, SteamInventoryEligiblePromoItemDefIDs_t, m_CallbackSteamInventoryEligiblePromoItemDefIDs);
	STEAM_CALLBACK_MANUAL(USIK_InventorySubsystem, OnSteamInventoryFullUpdateCallback, SteamInventoryFullUpdate_t, m_CallbackSteamInventoryFullUpdate);
	STEAM_CALLBACK_MANUAL(USIK_InventorySubsystem, OnSteamInventoryResultReadyCallback, SteamInventoryResultReady_t, m_CallbackSteamInventoryResultReady);
	STEAM_CALLBACK_MANUAL(USIK_InventorySubsystem, OnSteamInventoryStartPurchaseResultCallback, SteamInventoryStartPurchaseResult_t, m_CallbackSteamInventoryStartPurchaseResult);
	STEAM_CALLBACK_MANUAL(USIK_InventorySubsystem, OnSteamInventoryRequestPricesResultCallback, SteamInventoryRequestPricesResult_t, m_CallbackSteamInventoryRequestPricesResult);
#endif	
};
