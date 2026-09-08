

#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"

#include "AdvancedGameSession.generated.h"

UCLASS(config = Game, notplaceable)
class AAdvancedGameSession : public AGameSession
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(Transient)
	TMap<FUniqueNetIdRepl, FText> BanList;

	virtual bool BanPlayer(class APlayerController* BannedPlayer, const FText& BanReason)
	{

		if (APlayerState* PlayerState = (BannedPlayer != NULL) ? BannedPlayer->PlayerState : NULL)
		{
			FUniqueNetIdRepl UniqueNetID = PlayerState->GetUniqueId();
			bool bWasKicked = KickPlayer(BannedPlayer, BanReason);

			if (bWasKicked)
			{
				BanList.Add(UniqueNetID, BanReason);
			}

			return bWasKicked;
		}

		return false;
	}

	virtual void PostLogin(APlayerController* NewPlayer) override
	{
		if (APlayerState* PlayerState = (NewPlayer != NULL) ? NewPlayer->PlayerState : NULL)
		{
			FUniqueNetIdRepl UniqueNetID = PlayerState->GetUniqueId();

			if (BanList.Contains(UniqueNetID))
			{
				KickPlayer(NewPlayer, BanList[UniqueNetID]);
			}
		}
	}
};

AAdvancedGameSession::AAdvancedGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}