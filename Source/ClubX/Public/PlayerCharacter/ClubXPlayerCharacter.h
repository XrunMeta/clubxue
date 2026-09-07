

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ClubXPlayerCharacter.generated.h"

struct FInputActionValue;
class UInputAction;
class AClubXPlayerController;
class AClubXPlayerState;

UCLASS()
class CLUBX_API AClubXPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	AClubXPlayerCharacter();

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputConfiguration")
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputConfiguration|Key")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputConfiguration|Key")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputConfiguration|Key")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputConfiguration|Key")
	TObjectPtr<UInputAction> InteractAction;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();

private:
	UPROPERTY()
	AClubXPlayerController* PlayerController;

	UPROPERTY()
	TObjectPtr<AClubXPlayerState> MainPlayerState;
};
