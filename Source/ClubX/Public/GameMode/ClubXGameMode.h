

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ClubXGameMode.generated.h"

class UPlayerInfoSubsystem;

UCLASS()
class CLUBX_API AClubXGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess))
	TArray<APlayerState*> ListPlayerState;
};
