

#pragma once

#include "CoreMinimal.h"
#include "EIK_BaseWebApi.h"
#include "EIK_API_RemoveVoiceParticipant.generated.h"

UCLASS()
class EIKWEB_API UEIK_API_RemoveVoiceParticipant : public UEIK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Web")
	static UEIK_API_RemoveVoiceParticipant* RemoveVoiceParticipant(FString Authorization, FString DeploymentId, FString RoomId, FString ProductUserId);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_RoomId;
	FString Var_ProductUserId;
};
