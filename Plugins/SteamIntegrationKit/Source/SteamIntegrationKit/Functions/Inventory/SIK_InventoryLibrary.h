

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SIK_InventoryLibrary.generated.h"

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_InventoryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool AddPromoItem(FSIK_SteamInventoryResult& InventoryResult, FSIK_SteamItemDef ItemDef);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool AddPromoItems(FSIK_SteamInventoryResult& InventoryResult, const TArray<FSIK_SteamItemDef>& ItemDefs);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool CheckResultSteamID(FSIK_SteamInventoryResult InventoryResult, FSIK_SteamId SteamID);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool ConsumeItem(FSIK_SteamInventoryResult& InventoryResult, FSIK_SteamItemInstanceID ItemInstance, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool DeserializeResult(FSIK_SteamInventoryResult& InventoryResult, const TArray<uint8>& Buffer);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static void DestroyResult(FSIK_SteamInventoryResult InventoryResult);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool ExchangeItems(FSIK_SteamInventoryResult& InventoryResult, const TArray<FSIK_SteamItemDef>& ItemDefsToCreate,
		const TArray<int32>& ArrayGenerateQuantity, const TArray<FSIK_SteamItemInstanceID>& ItemDefsToDestroy,
		const TArray<int32>& DestroyQuantity);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GenerateItems(FSIK_SteamInventoryResult& InventoryResult, const TArray<FSIK_SteamItemDef>& ItemDefs,
		const TArray<int32>& Quantity);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GetAllItems(FSIK_SteamInventoryResult& InventoryResult);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GetEligiblePromoItemDefinitionIDs(FSIK_SteamId SteamID, TArray<FSIK_SteamItemDef>& ItemDefs);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GetItemDefinitionIDs(TArray<FSIK_SteamItemDef>& ItemDefs);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GetItemDefinitionProperty(FSIK_SteamItemDef ItemDef, const FString& PropertyName, FString& Value);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GetItemsByID(FSIK_SteamInventoryResult& InventoryResult, const TArray<FSIK_SteamItemInstanceID>& ItemInstances);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GetItemPrice(FSIK_SteamItemDef ItemDef, int64& CurrentPrice, int64& BasePrice);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GetItemsWithPrices(TArray<FSIK_SteamItemDef>& ItemDefs, TArray<int64>& CurrentPrices, TArray<int64>& BasePrices, int32 ArraySize = 0);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static int32 GetNumItemsWithPrices();

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GetResultItemProperty(FSIK_SteamInventoryResult InventoryResult, int32 ItemIndex, const FString& PropertyName, FString& Value);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GetResultItems(FSIK_SteamInventoryResult InventoryResult, TArray<FSIK_SteamItemDetails>& ItemInstances);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static TEnumAsByte<ESIK_Result> GetResultStatus(FSIK_SteamInventoryResult InventoryResult);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static FDateTime GetResultTimestamp(FSIK_SteamInventoryResult InventoryResult, int32& Timestamp);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool GrantPromoItems(FSIK_SteamInventoryResult& InventoryResult);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool LoadItemDefinitions();

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool SerializeResult(FSIK_SteamInventoryResult InventoryResult, TArray<uint8>& Buffer);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool TransferItemQuantity(FSIK_SteamInventoryResult& InventoryResult, FSIK_SteamItemInstanceID ItemInstance, int32 Quantity, FSIK_SteamItemInstanceID itemIdDest);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool TriggerItemDrop(FSIK_SteamInventoryResult& InventoryResult, FSIK_SteamItemDef ItemDef);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static FSIK_SteamInventoryUpdateHandle StartUpdateProperties();

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool SubmitUpdateProperties(FSIK_SteamInventoryUpdateHandle UpdateHandle, FSIK_SteamInventoryResult& InventoryResult);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool RemoveProperty(FSIK_SteamInventoryUpdateHandle UpdateHandle, FSIK_SteamItemInstanceID ItemInstance, const FString& PropertyName);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Inventory")
	static bool SetProperty(FSIK_SteamInventoryUpdateHandle UpdateHandle, FSIK_SteamItemInstanceID ItemInstance, const FString& PropertyName, const FString& Value);
};
