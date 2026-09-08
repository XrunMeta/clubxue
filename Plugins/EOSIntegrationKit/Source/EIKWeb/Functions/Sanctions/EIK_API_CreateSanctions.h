

#pragma once

#include "CoreMinimal.h"
#include "EIK_BaseWebApi.h"
#include "EIK_API_CreateSanctions.generated.h"

USTRUCT(BlueprintType)
struct FEIK_SanctionPostPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString ProductUserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString Justification;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	TArray<FString> Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	bool bPending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	TMap<FString, FString> Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString IdentityProvider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString AccountId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	int32 Duration = 0;

	FEIK_SanctionPostPayload()
	{
		ProductUserId = "";
		Action = "";
		Justification = "";
		Source = "";
		Tags = TArray<FString>();
		bPending = false;
		Metadata = TMap<FString, FString>();
		DisplayName = "";
		IdentityProvider = "";
		AccountId = "";
		Duration = 0;
	}

};
UCLASS()
class EIKWEB_API UEIK_API_CreateSanctions : public UEIK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Web")
	static UEIK_API_CreateSanctions* CreateSanctions(FString Authorization, FString DeploymentId, TArray<FEIK_SanctionPostPayload> Sanctions);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	TArray<FEIK_SanctionPostPayload> Var_Sanctions;
};
