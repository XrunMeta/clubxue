

#pragma once

#include "CoreMinimal.h"
#include "EIK_BaseWebApi.h"
#include "EIK_API_CreateRoomTokens.generated.h"

USTRUCT(BlueprintType)
struct FEWebApi_EosRoomParticipant
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString ProductUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	FString ClientIp;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit|Web")
	bool bHardMuted = false;
};

UCLASS()
class EIKWEB_API UEIK_API_CreateRoomTokens : public UEIK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Web")
	static UEIK_API_CreateRoomTokens* CreateVoiceRoomToken(FString Authorization, FString DeploymentId, FString RoomId, TArray<FEWebApi_EosRoomParticipant> Participants);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_RoomId;
	TArray<FEWebApi_EosRoomParticipant> Var_Participants;
};
