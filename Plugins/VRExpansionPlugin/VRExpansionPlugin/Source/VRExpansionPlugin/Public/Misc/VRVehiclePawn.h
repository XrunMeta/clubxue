

#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Pawn.h"
#include "Engine/InputDelegateBinding.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "VRVehiclePawn.generated.h"

UCLASS(config = Game, BlueprintType)
class VREXPANSIONPLUGIN_API AVRVehiclePawn : public APawn
{
	GENERATED_BODY()

public:

		virtual void OnRep_Controller() override
	{
		if ((Controller != NULL) && (Controller->GetPawn() == NULL))
		{

		}

	}

	UFUNCTION(BlueprintCallable, Category = "Pawn")
		virtual bool SetBindToInput(AController * CController, bool bBindToInput)
	{
		APlayerController * playe = Cast<APlayerController>(CController);

		if (playe != NULL)
		{
			if(InputComponent)
				playe->PopInputComponent(InputComponent); 

			if (!bBindToInput)
			{			

				DestroyPlayerInputComponent();
				return true;
			}
			else
			{

				if (InputComponent == NULL)
				{
					InputComponent = CreatePlayerInputComponent();
					if (InputComponent)
					{
						SetupPlayerInputComponent(InputComponent);
						InputComponent->RegisterComponent();

							InputComponent->bBlockInput = bBlockInput;
							UInputDelegateBinding::BindInputDelegates(GetClass(), InputComponent);

					}
				}

				if (InputComponent)
				{
					playe->PushInputComponent(InputComponent); 
					return true;
				}
			}
		}
		else
		{

			DestroyPlayerInputComponent();
			return false;
		}

		return false;
	}

	UFUNCTION(BlueprintCallable, Category = "Pawn")
		virtual bool ForceSecondaryPossession(AController * NewController)
	{
		if (NewController)
		{
			PossessedBy(NewController);
		}
		else
		{
			UnPossessed();
		}

		return false;

	}

};