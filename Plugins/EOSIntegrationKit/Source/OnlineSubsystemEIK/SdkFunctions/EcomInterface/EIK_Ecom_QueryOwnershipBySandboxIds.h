

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Ecom_QueryOwnershipBySandboxIds.generated.h"

USTRUCT(BlueprintType)
struct FEIK_Ecom_QueryOwnershipBySandboxIdsOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	FEIK_EpicAccountId LocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	TArray<FEIK_Ecom_SandboxId> SandboxIds;

	FEIK_Ecom_QueryOwnershipBySandboxIdsOptions()
	{
		LocalUserId = FEIK_EpicAccountId();
		SandboxIds = TArray<FEIK_Ecom_SandboxId>();
	}
	EOS_Ecom_QueryOwnershipBySandboxIdsOptions ToEOS_Ecom_QueryOwnershipBySandboxIdsOptions()
	{
		EOS_Ecom_QueryOwnershipBySandboxIdsOptions Options;
		Options.ApiVersion = EOS_ECOM_QUERYOWNERSHIPBYSANDBOXIDSOPTIONS_API_LATEST;
		Options.LocalUserId = LocalUserId.GetValueAsEosType();
		Options.SandboxIdsCount = SandboxIds.Num();
		Options.SandboxIds = new const char*[SandboxIds.Num()];
		for (int i = 0; i < SandboxIds.Num(); i++)
		{
			Options.SandboxIds[i] = SandboxIds[i].Ref;
		}
		return Options;
	}
};

USTRUCT(BlueprintType)
struct FEIK_Ecom_QueryOwnershipBySandboxIdsCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	TEnumAsByte<EEIK_Result> ResultCode;

	UPROPERTY(BlueprintReadOnly, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	FEIK_EpicAccountId LocalUserId;

	UPROPERTY(BlueprintReadOnly, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	TArray<FEIK_Ecom_SandboxIdItemOwnership> SandboxIdItemOwnerships;

	FEIK_Ecom_QueryOwnershipBySandboxIdsCallbackInfo()
	{
		ResultCode = EEIK_Result::EOS_ServiceFailure;
		LocalUserId = FEIK_EpicAccountId();
		SandboxIdItemOwnerships = TArray<FEIK_Ecom_SandboxIdItemOwnership>();
	}
	FEIK_Ecom_QueryOwnershipBySandboxIdsCallbackInfo(const EOS_Ecom_QueryOwnershipBySandboxIdsCallbackInfo& Data)
	{
		ResultCode = static_cast<EEIK_Result>(Data.ResultCode);
		LocalUserId = Data.LocalUserId;
		int32 SandboxIdItemOwnershipsCount = Data.SandboxIdItemOwnershipsCount;
		for (int i = 0; i < SandboxIdItemOwnershipsCount; i++)
		{
			SandboxIdItemOwnerships.Add(Data.SandboxIdItemOwnerships[i]);
		}
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEIK_Ecom_QueryOwnershipBySandboxIdsCallback, const FEIK_Ecom_QueryOwnershipBySandboxIdsCallbackInfo&, Data);
UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Ecom_QueryOwnershipBySandboxIds : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface", DisplayName = "EOS_Ecom_QueryOwnershipBySandboxIds")
	static UEIK_Ecom_QueryOwnershipBySandboxIds* EIK_Ecom_QueryOwnershipBySandboxIds(FEIK_Ecom_QueryOwnershipBySandboxIdsOptions QueryOwnershipBySandboxIdsOptions);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit | SDK Functions | Ecom Interface")
	FEIK_Ecom_QueryOwnershipBySandboxIdsCallback OnCallback;

private:
	static void EOS_CALL OnQueryOwnershipBySandboxIdsCallback(const EOS_Ecom_QueryOwnershipBySandboxIdsCallbackInfo* Data);
	virtual void Activate() override;
	FEIK_Ecom_QueryOwnershipBySandboxIdsOptions Var_QueryOwnershipBySandboxIdsOptions;
};
