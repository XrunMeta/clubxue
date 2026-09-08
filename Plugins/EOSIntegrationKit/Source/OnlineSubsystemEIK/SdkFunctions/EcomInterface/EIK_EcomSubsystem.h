

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_EcomSubsystem.generated.h"

UCLASS(meta=(DisplayName="Ecom Interface"), Category="EOS Integration Kit", DisplayName="Ecom Interface")
class ONLINESUBSYSTEMEIK_API UEIK_EcomSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CatalogItem_Release")
	void EIK_Ecom_CatalogItem_Release(FEIK_Ecom_CatalogItem CatalogItem);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CatalogOffer_Release")
	void EIK_Ecom_CatalogOffer_Release(FEIK_Ecom_CatalogOffer CatalogOffer);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CatalogRelease_Release")
	void EIK_Ecom_CatalogRelease_Release(FEIK_Ecom_CatalogRelease CatalogRelease);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyEntitlementById")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyEntitlementById(FEIK_EpicAccountId LocalUserId, const FString& EntitlementId, FEIK_Ecom_Entitlement& OutEntitlement);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyEntitlementByIndex")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyEntitlementByIndex(FEIK_EpicAccountId LocalUserId, int32 EntitlementIndex, FEIK_Ecom_Entitlement& OutEntitlement);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyEntitlementByNameAndIndex")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyEntitlementByNameAndIndex(FEIK_EpicAccountId LocalUserId, const FString& EntitlementName, int32 Index, FEIK_Ecom_Entitlement& OutEntitlement);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyItemById")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyItemById(FEIK_EpicAccountId LocalUserId, const FString& ItemId, FEIK_Ecom_CatalogItem& OutCatalogItem);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyItemImageInfoByIndex")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyItemImageInfoByIndex(FEIK_EpicAccountId LocalUserId, FEIK_Ecom_CatalogItemId ItemId, int32 ImageInfoIndex, FEIK_Ecom_KeyImageInfo& OutKeyImageInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyItemReleaseByIndex")
	TEnumAsByte<EEIK_Result>  EIK_Ecom_CopyItemReleaseByIndex(FEIK_EpicAccountId LocalUserId, FEIK_Ecom_CatalogItemId ItemId, int32 ReleaseIndex, FEIK_Ecom_CatalogRelease& OutRelease);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyLastRedeemedEntitlementByIndex")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyLastRedeemedEntitlementByIndex(FEIK_EpicAccountId LocalUserId, int32 RedeemedEntitlementIndex, FString& OutRedeemedEntitlementId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyOfferById")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyOfferById(FEIK_EpicAccountId LocalUserId, const FEIK_Ecom_CatalogOfferId & OfferId, FEIK_Ecom_CatalogOffer& OutCatalogOffer);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyOfferImageInfoByIndex")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyOfferImageInfoByIndex(FEIK_EpicAccountId LocalUserId, const FEIK_Ecom_CatalogOfferId & OfferId, int32 ImageInfoIndex, FEIK_Ecom_KeyImageInfo& OutKeyImageInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyOfferItemByIndex")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyOfferItemByIndex(FEIK_EpicAccountId LocalUserId, const FEIK_Ecom_CatalogOfferId & OfferId, int32 ItemIndex, FEIK_Ecom_CatalogItem& OutCatalogItem);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyOfferByIndex")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyOfferByIndex(FEIK_EpicAccountId LocalUserId, int32 OfferIndex, FEIK_Ecom_CatalogOffer& OutCatalogOffer);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyTransactionById")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyTransactionById(FEIK_EpicAccountId LocalUserId, const FString& TransactionId, FEIK_Ecom_HTransaction& OutTransaction);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyTransactionByIndex")
	TEnumAsByte<EEIK_Result> EIK_Ecom_CopyTransactionByIndex(FEIK_EpicAccountId LocalUserId, int32 TransactionIndex, FEIK_Ecom_HTransaction& OutTransaction);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_Entitlement_Release")
	void EIK_Ecom_Entitlement_Release(FEIK_Ecom_Entitlement Entitlement);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetEntitlementsByNameCount")
	int32 EIK_Ecom_GetEntitlementsByNameCount(FEIK_EpicAccountId LocalUserId, const FEIK_Ecom_EntitlementName& EntitlementName);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetEntitlementsCount")
	int32 EIK_Ecom_GetEntitlementsCount(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetItemImageInfoCount")
	int32 EIK_Ecom_GetItemImageInfoCount(FEIK_EpicAccountId LocalUserId, FEIK_Ecom_CatalogItemId ItemId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetItemReleaseCount")
	int32 EIK_Ecom_GetItemReleaseCount(FEIK_EpicAccountId LocalUserId, FEIK_Ecom_CatalogItemId ItemId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetLastRedeemedEntitlementsCount")
	int32 EIK_Ecom_GetLastRedeemedEntitlementsCount(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetOfferCount")
	int32 EIK_Ecom_GetOfferCount(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetOfferImageInfoCount")
	int32 EIK_Ecom_GetOfferImageInfoCount(FEIK_EpicAccountId LocalUserId, const FEIK_Ecom_CatalogOfferId & OfferId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetOfferItemCount")
	int32 EIK_Ecom_GetOfferItemCount(FEIK_EpicAccountId LocalUserId, const FEIK_Ecom_CatalogOfferId & OfferId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetTransactionCount")
	int32 EIK_Ecom_GetTransactionCount(FEIK_EpicAccountId LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_KeyImageInfo_Release")
	void EIK_Ecom_KeyImageInfo_Release(FEIK_Ecom_KeyImageInfo KeyImageInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_Transaction_CopyEntitlementByIndex")
	TEnumAsByte<EEIK_Result> EIK_Ecom_Transaction_CopyEntitlementByIndex(int32 EntitlementIndex, FEIK_Ecom_HTransaction Transaction, FEIK_Ecom_Entitlement& OutEntitlement);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_Transaction_GetEntitlementsCount")
	int32 EIK_Ecom_Transaction_GetEntitlementsCount(FEIK_Ecom_HTransaction Transaction);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_Transaction_GetTransactionId")
	FString EIK_Ecom_Transaction_GetTransactionId(FEIK_Ecom_HTransaction Transaction);
};
