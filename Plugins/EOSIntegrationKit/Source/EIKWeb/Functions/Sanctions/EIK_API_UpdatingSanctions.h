

#pragma once

#include "CoreMinimal.h"
#include "EIK_BaseWebApi.h"
#include "EIK_API_UpdatingSanctions.generated.h"

USTRUCT(BlueprintType)
struct FEIK_UpdatableFields
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	TArray<FString> Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	TMap<FString, FString> Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString Justification;

};

USTRUCT(BlueprintType)
struct FEIK_SanctionPatchPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString ReferenceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	TArray<FEIK_UpdatableFields> Updates;
};
UCLASS()
class EIKWEB_API UEIK_API_UpdatingSanctions : public UEIK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Web")
	static UEIK_API_UpdatingSanctions* UpdatingSanctions(FString Authorization, FString DeploymentId, TArray<FEIK_SanctionPatchPayload> SanctionPatchPayload);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	TArray<FEIK_SanctionPatchPayload> Var_SanctionPatchPayload;
};
