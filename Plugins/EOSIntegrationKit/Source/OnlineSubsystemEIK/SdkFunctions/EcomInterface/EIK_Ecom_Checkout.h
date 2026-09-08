

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_ecom.h"
#include "eos_ecom_types.h"
THIRD_PARTY_INCLUDES_END
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EIK_Ecom_Checkout.generated.h"

USTRUCT(BlueprintType)
struct FEIK_Ecom_CheckoutOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	FEIK_EpicAccountId LocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	FString OverrideCatalogNamespace;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	int32 EntryCount;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	TArray<FEIK_Ecom_CheckoutEntry> Entries;

	FEIK_Ecom_CheckoutOptions()
	{
		LocalUserId = FEIK_EpicAccountId();
		OverrideCatalogNamespace = "";
		EntryCount = 0;
		Entries = TArray<FEIK_Ecom_CheckoutEntry>();
	}
	EOS_Ecom_CheckoutOptions ToEOS_Ecom_CheckoutOptions()
	{
		EOS_Ecom_CheckoutOptions Options;
		Options.ApiVersion = EOS_ECOM_CHECKOUT_API_LATEST;
		Options.LocalUserId = LocalUserId.GetValueAsEosType();
		auto OverrideCatalogNamespaceAnsi = StringCast<ANSICHAR>(*OverrideCatalogNamespace);
		CachedOverrideCatalogNamespaceAnsi.SetNumUninitialized(OverrideCatalogNamespaceAnsi.Length() + 1);
		FMemory::Memcpy(CachedOverrideCatalogNamespaceAnsi.GetData(), OverrideCatalogNamespaceAnsi.Get(), OverrideCatalogNamespaceAnsi.Length() + 1);
		Options.OverrideCatalogNamespace = CachedOverrideCatalogNamespaceAnsi.GetData();
		Options.EntryCount = EntryCount;
		EOS_Ecom_CheckoutEntry* EntriesArray = new EOS_Ecom_CheckoutEntry[EntryCount];
		for (int i = 0; i < EntryCount; i++)
		{
			EntriesArray[i] = Entries[i].EOS_Ecom_CheckoutEntry_FromStruct();
		}
		Options.Entries = EntriesArray;
		return Options;
	}

	TArray<ANSICHAR> CachedOverrideCatalogNamespaceAnsi;
};

USTRUCT(BlueprintType)
struct FEIK_Ecom_CheckoutCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	TEnumAsByte<EEIK_Result> ResultCode;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	FString TransactionId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	FEIK_EpicAccountId LocalUserId;

	FEIK_Ecom_CheckoutCallbackInfo()
	{
		ResultCode = EEIK_Result::EOS_NotFound;
		TransactionId = "";
		LocalUserId = FEIK_EpicAccountId();
	}

	FEIK_Ecom_CheckoutCallbackInfo(const EOS_Ecom_CheckoutCallbackInfo* Data)
	{
		ResultCode = static_cast<EEIK_Result>(Data->ResultCode);
		TransactionId = Data->TransactionId;
		LocalUserId = Data->LocalUserId;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEIK_Ecom_CheckoutCallback, const FEIK_Ecom_CheckoutCallbackInfo&, Data);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Ecom_Checkout : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName = "EOS_Ecom_Checkout")
	static UEIK_Ecom_Checkout* EIK_Ecom_Checkout(FEIK_Ecom_CheckoutOptions CheckoutOptions);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	FEIK_Ecom_CheckoutCallback OnCallback;

private:
	FEIK_Ecom_CheckoutOptions Local_CheckoutOptions;
	static void EOS_CALL OnCheckoutCallback(const EOS_Ecom_CheckoutCallbackInfo* Data);
	virtual void Activate() override;
};
