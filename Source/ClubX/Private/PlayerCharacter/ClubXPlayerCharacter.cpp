

#include "PlayerCharacter/ClubXPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerController/ClubXPlayerController.h"
#include "PlayerState/ClubXPlayerState.h"

AClubXPlayerCharacter::AClubXPlayerCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; 
	CameraBoom->bUsePawnControlRotation = true; 

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 

	bReplicates = true;
	bAlwaysRelevant = true;
}

void AClubXPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<AClubXPlayerController>(Controller);
	if (!IsValid(PlayerController)) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AClubXPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	MainPlayerState = Cast<AClubXPlayerState>(GetPlayerState());
	if (IsValid(MainPlayerState))
	{

	}
}

void AClubXPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	MainPlayerState = Cast<AClubXPlayerState>(GetPlayerState());
	if (IsValid(MainPlayerState))
	{

	}
}

void AClubXPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void AClubXPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AClubXPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AClubXPlayerCharacter::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AClubXPlayerCharacter::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AClubXPlayerCharacter::Interact);
	}
}

void AClubXPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AClubXPlayerCharacter::Look(const FInputActionValue& Value)
{

	FVector2D LookAxisVector = Value.Get<FVector2D>();

}

void AClubXPlayerCharacter::Interact()
{

}

