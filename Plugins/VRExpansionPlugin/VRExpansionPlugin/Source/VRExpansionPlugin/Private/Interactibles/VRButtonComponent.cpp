

#include "Interactibles/VRButtonComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRButtonComponent)

#include "Net/UnrealNetwork.h"

#include "GripMotionControllerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

UVRButtonComponent::UVRButtonComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SetGenerateOverlapEvents(true);
	this->PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;

	LastToggleTime = 0.0f;
	DepressDistance = 8.0f;
	ButtonEngageDepth = 8.0f;
	DepressSpeed = 50.0f;

	ButtonAxis = EVRInteractibleAxis::Axis_Z;
	ButtonType = EVRButtonType::Btn_Toggle_Return;

	MinTimeBetweenEngaging = 0.1f;

	bIsEnabled = true;
	StateChangeAuthorityType = EVRStateChangeAuthorityType::CanChangeState_All;
	bButtonState = false;

	this->SetCollisionResponseToAllChannels(ECR_Overlap);

	bSkipOverlapFiltering = false;
	InitialRelativeTransform = FTransform::Identity;

	bReplicateMovement = false;
}

UVRButtonComponent::~UVRButtonComponent()
{
}

void UVRButtonComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRButtonComponent, InitialRelativeTransform, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UVRButtonComponent, bReplicateMovement, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UVRButtonComponent, StateChangeAuthorityType, PushModelParams);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_InitialOnly, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRButtonComponent, bButtonState, PushModelParamsWithCondition);
}

void UVRButtonComponent::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (!IRISNetReplication::IsIris(this))
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeLocation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeRotation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeScale3D, bReplicateMovement);
	}
}

void UVRButtonComponent::OnRegister()
{
	Super::OnRegister();
	ResetInitialButtonLocation();
}

void UVRButtonComponent::BeginPlay()
{

	Super::BeginPlay();

	SetButtonToRestingPosition();

	OnComponentBeginOverlap.AddUniqueDynamic(this, &UVRButtonComponent::OnOverlapBegin);
	OnComponentEndOverlap.AddUniqueDynamic(this, &UVRButtonComponent::OnOverlapEnd);
}

void UVRButtonComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float WorldTime = GetWorld()->GetRealTimeSeconds();

	if (IsValid(LocalInteractingComponent))
	{

		if (!bIsEnabled)
		{

			LocalInteractingComponent = nullptr;
			return;
		}

		FTransform OriginalBaseTransform = CalcNewComponentToWorld(InitialRelativeTransform);

		float CheckDepth = FMath::Clamp(GetAxisValue(InitialLocation) - GetAxisValue(OriginalBaseTransform.InverseTransformPosition(LocalInteractingComponent->GetComponentLocation())), 0.0f, DepressDistance);

		if (CheckDepth > 0.0f)
		{

			float ClampMinDepth = 0.0f;

			if (ButtonType == EVRButtonType::Btn_Toggle_Stay && bButtonState)
				ClampMinDepth = -(ButtonEngageDepth + (1.e-2f)); 

			float NewDepth = FMath::Clamp(GetAxisValue(InitialComponentLoc) + (-CheckDepth), -DepressDistance, ClampMinDepth);
			this->SetRelativeLocation(InitialRelativeTransform.TransformPosition(SetAxisValue(NewDepth)), false);

			if (ButtonType == EVRButtonType::Btn_Toggle_Return || ButtonType == EVRButtonType::Btn_Toggle_Stay)
			{
				if ((StateChangeAuthorityType == EVRStateChangeAuthorityType::CanChangeState_All) ||
					(StateChangeAuthorityType == EVRStateChangeAuthorityType::CanChangeState_Server && GetNetMode() < ENetMode::NM_Client) ||
					(StateChangeAuthorityType == EVRStateChangeAuthorityType::CanChangeState_Owner && IsValid(LocalLastInteractingActor) && LocalLastInteractingActor->HasLocalNetOwner()))
				{
					if (!bToggledThisTouch && NewDepth <= (-ButtonEngageDepth) + UE_KINDA_SMALL_NUMBER && (WorldTime - LastToggleTime) >= MinTimeBetweenEngaging)
					{
						LastToggleTime = WorldTime;
						bToggledThisTouch = true;
						bButtonState = !bButtonState;
						ReceiveButtonStateChanged(bButtonState, LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());
						OnButtonStateChanged.Broadcast(bButtonState, LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());
					}
				}
			}
		}
	}
	else
	{

		if (this->GetRelativeLocation().Equals(GetTargetRelativeLocation()))
		{
			this->SetComponentTickEnabled(false);

			OnButtonEndInteraction.Broadcast(LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());
			ReceiveButtonEndInteraction(LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());

			LocalInteractingComponent = nullptr; 
			LocalLastInteractingComponent = nullptr;
		}
		else
			this->SetRelativeLocation(FMath::VInterpConstantTo(this->GetRelativeLocation(), GetTargetRelativeLocation(), DeltaTime, DepressSpeed), false);
	}

	if (ButtonType == EVRButtonType::Btn_Press)
	{
		if ((StateChangeAuthorityType == EVRStateChangeAuthorityType::CanChangeState_All) ||
			(StateChangeAuthorityType == EVRStateChangeAuthorityType::CanChangeState_Server && GetNetMode() < ENetMode::NM_Client) ||
			(StateChangeAuthorityType == EVRStateChangeAuthorityType::CanChangeState_Owner && IsValid(LocalLastInteractingActor) && LocalLastInteractingActor->HasLocalNetOwner()))
		{

			bool bCheckState = (GetAxisValue(InitialRelativeTransform.InverseTransformPosition(this->GetRelativeLocation())) <= (-ButtonEngageDepth) + UE_KINDA_SMALL_NUMBER);
			if (bButtonState != bCheckState && (WorldTime - LastToggleTime) >= MinTimeBetweenEngaging)

			{
				LastToggleTime = WorldTime;
				bButtonState = bCheckState;
				ReceiveButtonStateChanged(bButtonState, LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());
				OnButtonStateChanged.Broadcast(bButtonState, LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());
			}
		}
	}

}

bool UVRButtonComponent::IsValidOverlap_Implementation(UPrimitiveComponent * OverlapComponent)
{

	if (!OverlapComponent || OverlapComponent == GetAttachParent() || OverlapComponent->GetAttachParent() == GetAttachParent())
		return false;

	AActor * OverlapOwner = OverlapComponent->GetOwner();

	if (IsValid(OverlapOwner))
	{
		if (OverlapOwner->IsA(ACharacter::StaticClass()))
			return true;

		const AActor* OverlapNetOwner = OverlapOwner->GetNetOwner();
		if (IsValid(OverlapNetOwner) && (OverlapNetOwner->IsA(APlayerController::StaticClass()) || OverlapNetOwner->IsA(ACharacter::StaticClass())))
			return true;
	}

	USceneComponent * OurAttachParent = OverlapComponent->GetAttachParent();
	if (OurAttachParent && OurAttachParent->IsA(UMotionControllerComponent::StaticClass()))
		return true;

	if (OverlapComponent->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		TArray<FBPGripPair> Controllers;
		bool bIsHeld;
		IVRGripInterface::Execute_IsHeld(OverlapComponent, Controllers, bIsHeld);

		if (bIsHeld)
			return true;
	}
	else if(OverlapOwner && OverlapOwner->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		TArray<FBPGripPair> Controllers;
		bool bIsHeld;
		IVRGripInterface::Execute_IsHeld(OverlapOwner, Controllers, bIsHeld);

		if (bIsHeld)
			return true;
	}

	return false;
}

void UVRButtonComponent::SetLastInteractingActor()
{

	if (!IsValid(LocalInteractingComponent) || LocalInteractingComponent == GetAttachParent() || LocalInteractingComponent->GetAttachParent() == GetAttachParent())
	{
		LocalLastInteractingActor = nullptr;
		LocalLastInteractingComponent = nullptr;
		return;
	}

	LocalLastInteractingComponent = LocalInteractingComponent;

	AActor * OverlapOwner = LocalInteractingComponent->GetOwner();
	if (OverlapOwner && OverlapOwner->IsA(ACharacter::StaticClass()))
	{
		LocalLastInteractingActor = OverlapOwner;
		return;
	}

	if (LocalInteractingComponent->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		TArray<FBPGripPair> Controllers;
		bool bIsHeld;
		IVRGripInterface::Execute_IsHeld(LocalLastInteractingComponent.Get(), Controllers, bIsHeld);

		if (bIsHeld && Controllers.Num())
		{
			AActor * ControllerOwner = Controllers[0].HoldingController != nullptr ? Controllers[0].HoldingController->GetOwner() : nullptr;
			if (ControllerOwner)
			{
				LocalLastInteractingActor = ControllerOwner;
				return;
			}
		}
	}
	else if (OverlapOwner && OverlapOwner->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		TArray<FBPGripPair> Controllers;
		bool bIsHeld;
		IVRGripInterface::Execute_IsHeld(OverlapOwner, Controllers, bIsHeld);

		if (bIsHeld && Controllers.Num())
		{
			AActor * ControllerOwner = Controllers[0].HoldingController != nullptr ? Controllers[0].HoldingController->GetOwner() : nullptr;
			if (ControllerOwner)
			{
				LocalLastInteractingActor = ControllerOwner;
				return;
			}
		}
	}

	if (OverlapOwner)
	{
		LocalLastInteractingActor = OverlapOwner;
		return;
	}

	LocalLastInteractingActor = nullptr;
	return;
}

void UVRButtonComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (bIsEnabled && !IsValid(LocalInteractingComponent) && (bSkipOverlapFiltering || IsValidOverlap(OtherComp)))
	{
		LocalInteractingComponent = OtherComp;

		FTransform OriginalBaseTransform = CalcNewComponentToWorld(InitialRelativeTransform);
		FVector loc = LocalInteractingComponent->GetComponentLocation();
		InitialLocation = OriginalBaseTransform.InverseTransformPosition(LocalInteractingComponent->GetComponentLocation());
		InitialComponentLoc = OriginalBaseTransform.InverseTransformPosition(this->GetComponentLocation());
		bToggledThisTouch = false;

		this->SetComponentTickEnabled(true);

		if (LocalInteractingComponent != LocalLastInteractingComponent.Get())
		{
			SetLastInteractingActor();
			OnButtonBeginInteraction.Broadcast(LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());
			ReceiveButtonBeginInteraction(LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());
		}
	}
}

void UVRButtonComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsValid(LocalInteractingComponent) && OtherComp == LocalInteractingComponent)
	{
		LocalInteractingComponent = nullptr;
	}
}

FVector UVRButtonComponent::GetTargetRelativeLocation()
{

	if (ButtonType == EVRButtonType::Btn_Toggle_Stay && bButtonState)
	{

		return InitialRelativeTransform.TransformPosition(SetAxisValue(-(ButtonEngageDepth + (1.e-2f))));
	}

	return InitialRelativeTransform.GetTranslation();

}

void UVRButtonComponent::SetButtonToRestingPosition(bool bLerpToPosition)
{
	switch (ButtonType)
	{
	case EVRButtonType::Btn_Press:
	{
	}break;
	case EVRButtonType::Btn_Toggle_Return:
	{}break;
	case EVRButtonType::Btn_Toggle_Stay:
	{
		if (!bLerpToPosition)
		{
			float ClampMinDepth = 0.0f;

			if (bButtonState)
				ClampMinDepth = -(ButtonEngageDepth + (1.e-2f)); 

			float NewDepth = FMath::Clamp(ClampMinDepth, -DepressDistance, ClampMinDepth);
			this->SetRelativeLocation(InitialRelativeTransform.TransformPosition(SetAxisValue(NewDepth)), false);
		}
		else
			this->SetComponentTickEnabled(true); 

	}break;
	default:break;
	}
}

void UVRButtonComponent::SetButtonState(bool bNewButtonState, bool bCallButtonChangedEvent, bool bSnapIntoPosition)
{

	if (bButtonState == bNewButtonState)
		return;

	bButtonState = bNewButtonState;
	SetButtonToRestingPosition(!bSnapIntoPosition);
	LastToggleTime = GetWorld()->GetRealTimeSeconds();

	if (bCallButtonChangedEvent)
	{
		ReceiveButtonStateChanged(bButtonState, LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());
		OnButtonStateChanged.Broadcast(bButtonState, LocalLastInteractingActor.Get(), LocalLastInteractingComponent.Get());
	}
}

void UVRButtonComponent::ResetInitialButtonLocation()
{

	InitialRelativeTransform = this->GetRelativeTransform();
}

bool UVRButtonComponent::IsButtonInUse()
{
	return IsValid(LocalInteractingComponent);
}

float UVRButtonComponent::GetAxisValue(FVector CheckLocation)
{
	switch (ButtonAxis)
	{
	case EVRInteractibleAxis::Axis_X:
		return CheckLocation.X; break;
	case EVRInteractibleAxis::Axis_Y:
		return CheckLocation.Y; break;
	case EVRInteractibleAxis::Axis_Z:
		return CheckLocation.Z; break;
	default:return 0.0f; break;
	}
}

FVector UVRButtonComponent::SetAxisValue(float SetValue)
{
	FVector vec = FVector::ZeroVector;

	switch (ButtonAxis)
	{
	case EVRInteractibleAxis::Axis_X:
		vec.X = SetValue; break;
	case EVRInteractibleAxis::Axis_Y:
		vec.Y = SetValue; break;
	case EVRInteractibleAxis::Axis_Z:
		vec.Z = SetValue; break;
	}

	return vec;
}

void UVRButtonComponent::SetReplicateMovement(bool bNewReplicateMovement)
{
	bReplicateMovement = bNewReplicateMovement;
#if WITH_PUSH_MODEL
		MARK_PROPERTY_DIRTY_FROM_NAME(UVRButtonComponent, bReplicateMovement, this);
#endif
}

void UVRButtonComponent::SetStateChangeAuthorityType(EVRStateChangeAuthorityType NewStateChangeAuthorityType)
{
	StateChangeAuthorityType = NewStateChangeAuthorityType;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRButtonComponent, StateChangeAuthorityType, this);
#endif
}

void UVRButtonComponent::SetButtonState(bool bNewButtonState)
{
	bButtonState = bNewButtonState;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRButtonComponent, bButtonState, this);
#endif
}