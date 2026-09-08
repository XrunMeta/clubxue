

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Ecom_QueryEntitlements.generated.h"

USTRUCT(BlueprintType)
struct FEIK_Ecom_QueryEntitlementsOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	FEIK_EpicAccountId LocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	TArray<FString> EntitlementNames;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	bool bIncludeRedeemed;

	FEIK_Ecom_QueryEntitlementsOptions()
	{
		LocalUserId = FEIK_EpicAccountId();
		EntitlementNames = TArray<FString>();
		bIncludeRedeemed = false;
	}
	EOS_Ecom_QueryEntitlementsOptions ToEOS_Ecom_QueryEntitlementsOptions()
	{
		EOS_Ecom_QueryEntitlementsOptions Options;
		Options.ApiVersion = EOS_ECOM_QUERYENTITLEMENTS_API_LATEST;
		Options.LocalUserId = LocalUserId.GetValueAsEosType();
		Options.EntitlementNameCount = EntitlementNames.Num();
		CachedEntitlementNamesAnsi.Reset();
		CachedEntitlementNamePtrs.SetNum(EntitlementNames.Num());
		for (int i = 0; i < EntitlementNames.Num(); i++)
		{
			auto EntitlementNameAnsi = StringCast<ANSICHAR>(*EntitlementNames[i]);
			CachedEntitlementNamesAnsi.AddDefaulted();
			TArray<ANSICHAR>& EntitlementNameBuffer = CachedEntitlementNamesAnsi.Last();
			EntitlementNameBuffer.SetNumUninitialized(EntitlementNameAnsi.Length() + 1);
			FMemory::Memcpy(EntitlementNameBuffer.GetData(), EntitlementNameAnsi.Get(), EntitlementNameAnsi.Length() + 1);
			CachedEntitlementNamePtrs[i] = EntitlementNameBuffer.GetData();
		}
		Options.EntitlementNames = CachedEntitlementNamePtrs.GetData();
		Options.bIncludeRedeemed = bIncludeRedeemed;
		return Options;
	}

	TArray<TArray<ANSICHAR>> CachedEntitlementNamesAnsi;
	TArray<const char*> CachedEntitlementNamePtrs;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEIK_Ecom_QueryEntitlementsCallback, const FEIK_EpicAccountId&, LocalUserId, const TEnumAsByte<EEIK_Result>&, Result);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Ecom_QueryEntitlements : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_QueryEntitlements")
	static UEIK_Ecom_QueryEntitlements* EIK_Ecom_QueryEntitlements(FEIK_Ecom_QueryEntitlementsOptions QueryEntitlementsOptions);

	UPROPERTY(BlueprintAssignable)
	FEIK_Ecom_QueryEntitlementsCallback OnCallback;
private:
	static void EOS_CALL OnQueryEntitlementsCallback(const EOS_Ecom_QueryEntitlementsCallbackInfo* Data);
	virtual void Activate() override;
	FEIK_Ecom_QueryEntitlementsOptions Local_QueryEntitlementsOptions;
};
