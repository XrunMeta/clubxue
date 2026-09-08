

#include "VRPlayerController.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRPlayerController)

#include "AI/NavigationSystemBase.h"
#include "VRBaseCharacterMovementComponent.h"
#include "VRPathFollowingComponent.h"

#include "Engine/Player.h"

AVRPlayerController::AVRPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bDisableServerUpdateCamera = true;
}

void AVRPlayerController::SpawnPlayerCameraManager()
{
	Super::SpawnPlayerCameraManager();

	if(PlayerCameraManager != NULL && bDisableServerUpdateCamera)
		PlayerCameraManager->bUseClientSideCameraUpdates = false;
}

void AVRPlayerController::PlayerTick(float DeltaTime)
{

	if (AVRBaseCharacter * VRChar = Cast<AVRBaseCharacter>(GetPawn()))
	{

		UVRBaseCharacterMovementComponent * BaseCMC = Cast<UVRBaseCharacterMovementComponent>(VRChar->GetMovementComponent());

		if (!BaseCMC || !BaseCMC->bRunControlRotationInMovementComponent)
			return Super::PlayerTick(DeltaTime);

		if (!bShortConnectTimeOut)
		{
			bShortConnectTimeOut = true;
			ServerShortTimeout();
		}

		TickPlayerInput(DeltaTime, DeltaTime == 0.f);
		LastRotationInput = RotationInput;

		if ((Player != NULL) && (Player->PlayerController == this))
		{

			bool bUpdateRotation = false;
			if (IsInState(NAME_Playing))
			{
				if (GetPawn() == NULL)
				{
					ChangeState(NAME_Inactive);
				}
				else if (Player && GetPawn() == AcknowledgedPawn && (!BaseCMC || (BaseCMC && !BaseCMC->IsActive())))
				{
					bUpdateRotation = true;
				}
			}

			if (IsInState(NAME_Inactive))
			{
				if (GetLocalRole() < ROLE_Authority)
				{
					SafeServerCheckClientPossession();
				}

			}
			else if (IsInState(NAME_Spectating))
			{
				if (GetLocalRole() < ROLE_Authority)
				{
					SafeServerUpdateSpectatorState();
				}

				bUpdateRotation = true;
			}

			if (bUpdateRotation)
			{
				UpdateRotation(DeltaTime);
			}
		}
	}
	else
	{

		Super::PlayerTick(DeltaTime);
	}
}

UVRLocalPlayer::UVRLocalPlayer(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
}