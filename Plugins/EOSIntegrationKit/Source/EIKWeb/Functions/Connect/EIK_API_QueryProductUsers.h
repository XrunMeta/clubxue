

#pragma once

#include "CoreMinimal.h"
#include "EIK_BaseWebApi.h"
#include "EIK_API_QueryProductUsers.generated.h"

UCLASS()
class EIKWEB_API UEIK_API_QueryProductUsers : public UEIK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Web API|Connect")
	static UEIK_API_QueryProductUsers* QueryProductUsers(FString Authorization, TArray<FString> ProductUserIds);

private:

	virtual void Activate() override;
	FString Var_Authorization;
	TArray<FString> Var_ProductUserIds;
};
