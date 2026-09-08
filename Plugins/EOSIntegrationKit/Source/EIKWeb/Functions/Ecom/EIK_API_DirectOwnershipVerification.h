

#pragma once

#include "CoreMinimal.h"
#include "EIK_BaseWebApi.h"
#include "EIK_API_DirectOwnershipVerification.generated.h"

UCLASS()
class EIKWEB_API UEIK_API_DirectOwnershipVerification : public UEIK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Web API|AntiCheat")
	static UEIK_API_DirectOwnershipVerification* DirectOwnershipVerification(FString Authorization, FString CurrentAccountId, TArray<FString> NsCatalogItemIds, FString SandboxId);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_CurrentAccountId;
	TArray<FString> Var_NsCatalogItemIds;
	FString Var_SandboxId;
};
