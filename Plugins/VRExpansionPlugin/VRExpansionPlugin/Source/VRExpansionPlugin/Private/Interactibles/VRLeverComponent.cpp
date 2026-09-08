

#include "Interactibles/VRLeverComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRLeverComponent)

#include "GripMotionControllerComponent.h"
#include "VRExpansionFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

UVRLeverComponent::UVRLeverComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SetGenerateOverlapEvents(true);
	this->PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;

	bRepGameplayTags = false;

	bReplicateMovement = true;

	MovementReplicationSetting = EGripMovementReplicationSettings::ForceClientSideMovement;
	BreakDistance = 100.0f;
	Stiffness = 1500.0f;
	Damping = 200.0f;

	ParentComponent = nullptr;
	LeverRotationAxis = EVRInteractibleLeverAxis::Axis_X;

	LeverLimitNegative = 0.0f;
	LeverLimitPositive = 90.0f;
	FlightStickYawLimit = 180.0f;
	bLeverState = false;
	LeverTogglePercentage = 0.8f;

	LastDeltaAngle = 0.0f;
	FullCurrentAngle = 0.0f;

	LeverReturnTypeWhenReleased = EVRInteractibleLeverReturnType::ReturnToZero;
	LeverReturnSpeed = 50.0f;

	MomentumAtDrop = 0.0f;
	LeverMomentumFriction = 5.0f;
	MaxLeverMomentum = 180.0f;
	FramesToAverage = 3;

	bBlendAxisValuesByAngleThreshold = false;
	AngleThreshold = 90.0f;

	LastLeverAngle = 0.0f;

	bSendLeverEventsDuringLerp = false;

	InitialRelativeTransform = FTransform::Identity;
	InitialInteractorLocation = FVector::ZeroVector;
	InteractorOffsetTransform = FTransform::Identity;
	AllCurrentLeverAngles = FRotator::ZeroRotator;
	InitialGripRot = 0.0f;
	qRotAtGrab = FQuat::Identity;
	bIsLerping = false;
	bUngripAtTargetRotation = false;
	bDenyGripping = false;

	bIsLocked = false;
	bAutoDropWhenLocked = true;

	PrimarySlotRange = 100.f;
	SecondarySlotRange = 100.f;
	GripPriority = 1;

	this->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

UVRLeverComponent::~UVRLeverComponent()
{
}

void UVRLeverComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRLeverComponent, InitialRelativeTransform, PushModelParams);

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRLeverComponent, bRepGameplayTags, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UVRLeverComponent, bReplicateMovement, PushModelParams);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_Custom, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRLeverComponent, GameplayTags, PushModelParamsWithCondition);
}

void UVRLeverComponent::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(UVRLeverComponent, GameplayTags, bRepGameplayTags);

	if (!IRISNetReplication::IsIris(this))
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeLocation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeRotation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeScale3D, bReplicateMovement);
	}
}

void UVRLeverComponent::OnRegister()
{
	Super::OnRegister();
	ResetInitialLeverLocation(); 
}

void UVRLeverComponent::BeginPlay()
{

	Super::BeginPlay();
	ReCalculateCurrentAngle(true);
	bOriginalReplicatesMovement = bReplicateMovement;
}

void UVRLeverComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bWasLerping = bIsLerping;

	if (bIsLocked)
	{

		if (bWasLerping)
		{
			bIsLerping = false;

			OnLeverFinishedLerping.Broadcast(CurrentLeverAngle);
			ReceiveLeverFinishedLerping(CurrentLeverAngle);
		}

		return;
	}

	if (bIsLerping)
	{
		FTransform CurRelativeTransform = this->GetComponentTransform().GetRelativeTransform(UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this));

		switch (LeverRotationAxis)
		{
		case EVRInteractibleLeverAxis::Axis_X:
		case EVRInteractibleLeverAxis::Axis_Y:
		case EVRInteractibleLeverAxis::Axis_Z:
		{
			LerpAxis(FullCurrentAngle, DeltaTime);
		}break;
		case EVRInteractibleLeverAxis::Axis_XY:
		case EVRInteractibleLeverAxis::FlightStick_XY:
		{

			FQuat LerpedQuat = FMath::QInterpConstantTo(CurRelativeTransform.GetRelativeTransform(InitialRelativeTransform).GetRotation(), FQuat::Identity, DeltaTime, FMath::DegreesToRadians(LeverReturnSpeed));

			if (LerpedQuat.IsIdentity())
			{
				this->SetComponentTickEnabled(false);
				bIsLerping = false;
				bReplicateMovement = bOriginalReplicatesMovement;
				this->SetRelativeRotation(InitialRelativeTransform.Rotator());
			}
			else
			{
				this->SetRelativeRotation((FTransform(LerpedQuat) * InitialRelativeTransform).GetRotation());
			}
		}break;
		default:break;
		}
	}

	FTransform CurrentRelativeTransform = this->GetComponentTransform().GetRelativeTransform(UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this));
	CalculateCurrentAngle(CurrentRelativeTransform);

	if (!bWasLerping && LeverReturnTypeWhenReleased == EVRInteractibleLeverReturnType::RetainMomentum)
	{

		MomentumAtDrop -= MomentumAtDrop / FramesToAverage;
		MomentumAtDrop += ((FullCurrentAngle - LastLeverAngle) / DeltaTime) / FramesToAverage;

		MomentumAtDrop = FMath::Min(MaxLeverMomentum, MomentumAtDrop);

		LastLeverAngle = FullCurrentAngle;
	}

	ProccessCurrentState(bWasLerping, true, true);

	if (bWasLerping && !bIsLerping)
	{
		OnLeverFinishedLerping.Broadcast(CurrentLeverAngle);
		ReceiveLeverFinishedLerping(CurrentLeverAngle);
	}
}

void UVRLeverComponent::ProccessCurrentState(bool bWasLerping, bool bThrowEvents, bool bCheckAutoDrop)
{
	bool bNewLeverState = (!FMath::IsNearlyZero(LeverLimitNegative) && FullCurrentAngle <= -(LeverLimitNegative * LeverTogglePercentage)) || (!FMath::IsNearlyZero(LeverLimitPositive) && FullCurrentAngle >= (LeverLimitPositive * LeverTogglePercentage));

	if (bNewLeverState != bLeverState)
	{
		bLeverState = bNewLeverState;

		if (bThrowEvents && (bSendLeverEventsDuringLerp || !bWasLerping))
		{
			ReceiveLeverStateChanged(bLeverState, FullCurrentAngle >= 0.0f ? EVRInteractibleLeverEventType::LeverPositive : EVRInteractibleLeverEventType::LeverNegative, CurrentLeverAngle, FullCurrentAngle);
			OnLeverStateChanged.Broadcast(bLeverState, FullCurrentAngle >= 0.0f ? EVRInteractibleLeverEventType::LeverPositive : EVRInteractibleLeverEventType::LeverNegative, CurrentLeverAngle, FullCurrentAngle);
		}

		if (bCheckAutoDrop)
		{
			if (!bWasLerping && bUngripAtTargetRotation && bLeverState && HoldingGrip.IsValid())
			{
				FBPActorGripInformation GripInformation;
				EBPVRResultSwitch result;
				HoldingGrip.HoldingController->GetGripByID(GripInformation, HoldingGrip.GripID, result);
				if (result == EBPVRResultSwitch::OnSucceeded && HoldingGrip.HoldingController->HasGripAuthority(GripInformation))
				{
					HoldingGrip.HoldingController->DropObjectByInterface(this, HoldingGrip.GripID);
				}
			}
		}
	}
}

void UVRLeverComponent::OnUnregister()
{
	Super::OnUnregister();
}

bool UVRLeverComponent::CheckAutoDrop(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation)
{

	if (BreakDistance > 0.f && GrippingController->HasGripAuthority(GripInformation) && FVector::DistSquared(InitialInteractorDropLocation, this->GetComponentTransform().InverseTransformPosition(GrippingController->GetPivotLocation())) >= FMath::Square(BreakDistance))
	{
		if (GrippingController->OnGripOutOfRange.IsBound())
		{
			uint8 GripID = GripInformation.GripID;
			GrippingController->OnGripOutOfRange.Broadcast(GripInformation, GripInformation.GripDistance);
		}
		else
		{
			GrippingController->DropObjectByInterface(this, HoldingGrip.GripID);
		}

		return true;
	}

	return false;
}

void UVRLeverComponent::TickGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation, float DeltaTime) 
{
	if (bIsLocked)
	{
		if (bAutoDropWhenLocked)
		{

			CheckAutoDrop(GrippingController, GripInformation);
		}

		return;
	}

	FTransform ParentTransform = UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this);
	FTransform CurrentRelativeTransform = InitialRelativeTransform * ParentTransform;
	FTransform PivotTransform = GrippingController->GetPivotTransform();
	FVector CurInteractorLocation = (InteractorOffsetTransform * PivotTransform).GetRelativeTransform(CurrentRelativeTransform).GetTranslation();

	switch (LeverRotationAxis)
	{
	case EVRInteractibleLeverAxis::Axis_XY:
	case EVRInteractibleLeverAxis::FlightStick_XY:
	{
		FRotator Rot;

		FVector nAxis;
		float nAngle = 0.0f;

		FQuat::FindBetweenVectors(qRotAtGrab.UnrotateVector(InitialInteractorLocation), CurInteractorLocation).ToAxisAndAngle(nAxis, nAngle);
		float MaxAngle = FMath::DegreesToRadians(LeverLimitPositive);

		bool bWasClamped = nAngle > MaxAngle;
		nAngle = FMath::Clamp(nAngle, 0.0f, MaxAngle);
		Rot = FQuat(nAxis, nAngle).Rotator();

		if (LeverRotationAxis == EVRInteractibleLeverAxis::FlightStick_XY)
		{

			FTransform CalcTransform = (FTransform(Rot) * InitialRelativeTransform);

			if (bWasClamped)
			{

				FTransform NewPivTrans = PivotTransform.GetRelativeTransform((CalcTransform * ParentTransform));
				CurInteractorLocation = NewPivTrans.GetTranslation();

				FVector OffsetVal = CurInteractorLocation + NewPivTrans.GetRotation().RotateVector(InteractorOffsetTransform.GetTranslation());
				OffsetVal.Z = 0;

				CurInteractorLocation -= OffsetVal;
			}
			else
			{
				CurInteractorLocation = (CalcTransform * ParentTransform).InverseTransformPosition(GrippingController->GetPivotLocation());
			}

			float CurrentLeverYawAngle = FRotator::NormalizeAxis(UVRInteractibleFunctionLibrary::GetAtan2Angle(EVRInteractibleAxis::Axis_Z, CurInteractorLocation, InitialGripRot));

			if (FlightStickYawLimit < 180.0f)
			{
				CurrentLeverYawAngle = FMath::Clamp(CurrentLeverYawAngle, -FlightStickYawLimit, FlightStickYawLimit);
			}

			FQuat newLocalRot = CalcTransform.GetRotation() * FQuat(FVector::UpVector, FMath::DegreesToRadians(CurrentLeverYawAngle));
			this->SetRelativeRotation(newLocalRot.Rotator());
		}
		else
		{
			this->SetRelativeRotation((FTransform(Rot) * InitialRelativeTransform).Rotator());
		}
	}
	break;
	case EVRInteractibleLeverAxis::Axis_X:
	case EVRInteractibleLeverAxis::Axis_Y:
	case EVRInteractibleLeverAxis::Axis_Z:
	{
		float DeltaAngle = CalcAngle(LeverRotationAxis, CurInteractorLocation);
		LastDeltaAngle = DeltaAngle;
		FTransform CalcTransform = (FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot((EVRInteractibleAxis)LeverRotationAxis, DeltaAngle, FRotator::ZeroRotator)) * InitialRelativeTransform);
		this->SetRelativeRotation(CalcTransform.Rotator());
	}break;
	default:break;
	}

	CurrentRelativeTransform = this->GetComponentTransform().GetRelativeTransform(UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this));
	CalculateCurrentAngle(CurrentRelativeTransform);

	ProccessCurrentState(bIsLerping, true, true);

	CheckAutoDrop(GrippingController, GripInformation);
}

void UVRLeverComponent::OnGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) 
{
	ParentComponent = this->GetAttachParent();

	FTransform CurrentRelativeTransform = InitialRelativeTransform * UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this);

	FTransform ReversedRelativeTransform = FTransform(GripInformation.RelativeTransform.ToInverseMatrixWithScale());
	FTransform CurrentTransform = this->GetComponentTransform();
	FTransform RelativeToGripTransform = FTransform::Identity;

	if (LeverRotationAxis == EVRInteractibleLeverAxis::FlightStick_XY)
	{

		FVector InitialInteractorOffset = ReversedRelativeTransform.GetTranslation();
		FTransform InitTrans = ReversedRelativeTransform;
		InitialInteractorOffset.X = 0;
		InitialInteractorOffset.Y = 0;
		InteractorOffsetTransform = ReversedRelativeTransform;
		InteractorOffsetTransform.AddToTranslation(-InitialInteractorOffset);
		InteractorOffsetTransform = FTransform(InteractorOffsetTransform.ToInverseMatrixWithScale());

		InitialInteractorOffset = ReversedRelativeTransform.GetTranslation();
		InitialInteractorOffset.Z = 0;

		InitTrans.AddToTranslation(-InitialInteractorOffset);
		RelativeToGripTransform = InitTrans * CurrentTransform;
	}
	else
	{
		RelativeToGripTransform = ReversedRelativeTransform * CurrentTransform;
		InteractorOffsetTransform = FTransform::Identity;
	}

	InitialInteractorLocation = CurrentRelativeTransform.InverseTransformPosition(RelativeToGripTransform.GetTranslation());
	InitialInteractorDropLocation = ReversedRelativeTransform.GetTranslation();

	switch (LeverRotationAxis)
	{
	case EVRInteractibleLeverAxis::Axis_XY:
	{
		qRotAtGrab = this->GetComponentTransform().GetRelativeTransform(CurrentRelativeTransform).GetRotation();
	}break;
	case EVRInteractibleLeverAxis::FlightStick_XY:
	{
		qRotAtGrab = this->GetComponentTransform().GetRelativeTransform(CurrentRelativeTransform).GetRotation();
		InitialGripRot = UVRInteractibleFunctionLibrary::GetAtan2Angle(EVRInteractibleAxis::Axis_Z, ReversedRelativeTransform.GetTranslation());
	}break;
	case EVRInteractibleLeverAxis::Axis_X:
	case EVRInteractibleLeverAxis::Axis_Y:
	{

		InitialGripRot = UVRInteractibleFunctionLibrary::GetAtan2Angle((EVRInteractibleAxis)LeverRotationAxis, InitialInteractorLocation);
	}break;

	case EVRInteractibleLeverAxis::Axis_Z:
	{

		InitialGripRot = UVRInteractibleFunctionLibrary::GetAtan2Angle((EVRInteractibleAxis)LeverRotationAxis, InitialInteractorLocation);
	}break;

	default:break;
	}

	RotAtGrab = UVRInteractibleFunctionLibrary::GetDeltaAngleFromTransforms((EVRInteractibleAxis)LeverRotationAxis, CurrentRelativeTransform, CurrentTransform);

	LastLeverAngle = CurrentLeverAngle;
	bIsLerping = false;
	bIsInFirstTick = true;
	MomentumAtDrop = 0.0f;

	this->SetComponentTickEnabled(true);

}

void UVRLeverComponent::OnGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed) 
{
	if (LeverReturnTypeWhenReleased != EVRInteractibleLeverReturnType::Stay)
	{		
		bIsLerping = true;
		this->SetComponentTickEnabled(true);
		if (MovementReplicationSetting != EGripMovementReplicationSettings::ForceServerSideMovement)
			bReplicateMovement = false;
	}
	else
	{
		this->SetComponentTickEnabled(false);
		bReplicateMovement = bOriginalReplicatesMovement;
	}

}

void UVRLeverComponent::SetGripPriority(int NewGripPriority)
{
	GripPriority = NewGripPriority;
}

void UVRLeverComponent::SetIsLocked(bool bNewLockedState)
{
	bIsLocked = bNewLockedState;
}

void UVRLeverComponent::OnChildGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) {}
void UVRLeverComponent::OnChildGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed) {}
void UVRLeverComponent::OnSecondaryGrip_Implementation(UGripMotionControllerComponent * GripOwningController, USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation) {}
void UVRLeverComponent::OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent * GripOwningController, USceneComponent * ReleasingSecondaryGripComponent, const FBPActorGripInformation & GripInformation) {}
void UVRLeverComponent::OnUsed_Implementation() {}
void UVRLeverComponent::OnEndUsed_Implementation() {}
void UVRLeverComponent::OnSecondaryUsed_Implementation() {}
void UVRLeverComponent::OnEndSecondaryUsed_Implementation() {}
void UVRLeverComponent::OnInput_Implementation(FKey Key, EInputEvent KeyEvent) {}
bool UVRLeverComponent::RequestsSocketing_Implementation(USceneComponent *& ParentToSocketTo, FName & OptionalSocketName, FTransform_NetQuantize & RelativeTransform) { return false; }

bool UVRLeverComponent::DenyGripping_Implementation(UGripMotionControllerComponent * GripInitiator)
{
	return bDenyGripping;
}

EGripInterfaceTeleportBehavior UVRLeverComponent::TeleportBehavior_Implementation()
{
	return EGripInterfaceTeleportBehavior::DropOnTeleport;
}

bool UVRLeverComponent::SimulateOnDrop_Implementation()
{
	return false;
}

EGripCollisionType UVRLeverComponent::GetPrimaryGripType_Implementation(bool bIsSlot)
{
	return EGripCollisionType::CustomGrip;
}

ESecondaryGripType UVRLeverComponent::SecondaryGripType_Implementation()
{
	return ESecondaryGripType::SG_None;
}

EGripMovementReplicationSettings UVRLeverComponent::GripMovementReplicationType_Implementation()
{
	return MovementReplicationSetting;
}

EGripLateUpdateSettings UVRLeverComponent::GripLateUpdateSetting_Implementation()
{
	return EGripLateUpdateSettings::LateUpdatesAlwaysOff;
}

void UVRLeverComponent::GetGripStiffnessAndDamping_Implementation(float &GripStiffnessOut, float &GripDampingOut)
{
	GripStiffnessOut = Stiffness;
	GripDampingOut = Damping;
}

FBPAdvGripSettings UVRLeverComponent::AdvancedGripSettings_Implementation()
{
	FBPAdvGripSettings GripSettings(GripPriority);
	GripSettings.bDisallowSettingPositionOnClientAuthDrop = true;
	return GripSettings;
}

float UVRLeverComponent::GripBreakDistance_Implementation()
{
	return BreakDistance;
}

void UVRLeverComponent::ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool & bHadSlotInRange, FTransform & SlotWorldTransform, FName & SlotName, UGripMotionControllerComponent * CallingController, FName OverridePrefix)
{
	if (OverridePrefix.IsNone())
		bSecondarySlot ? OverridePrefix = "VRGripS" : OverridePrefix = "VRGripP";

	UVRExpansionFunctionLibrary::GetGripSlotInRangeByTypeName_Component(OverridePrefix, this, WorldLocation, bSecondarySlot ? SecondarySlotRange : PrimarySlotRange, bHadSlotInRange, SlotWorldTransform, SlotName, CallingController);
}

bool UVRLeverComponent::AllowsMultipleGrips_Implementation()
{
	return false;
}

void UVRLeverComponent::IsHeld_Implementation(TArray<FBPGripPair> & CurHoldingControllers, bool & bCurIsHeld)
{
	CurHoldingControllers.Empty();
	if (HoldingGrip.IsValid())
	{
		CurHoldingControllers.Add(HoldingGrip);
		bCurIsHeld = bIsHeld;
	}
	else
	{
		bCurIsHeld = false;
	}
}

void UVRLeverComponent::Native_NotifyThrowGripDelegates(UGripMotionControllerComponent* Controller, bool bGripped, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	if (bGripped)
	{
		OnGripped.Broadcast(Controller, GripInformation);
	}
	else
	{
		OnDropped.Broadcast(Controller, GripInformation, bWasSocketed);
	}
}

void UVRLeverComponent::SetHeld_Implementation(UGripMotionControllerComponent * NewHoldingController, uint8 GripID, bool bNewIsHeld)
{
	if (bNewIsHeld)
	{
		HoldingGrip = FBPGripPair(NewHoldingController, GripID);
		if (MovementReplicationSetting != EGripMovementReplicationSettings::ForceServerSideMovement)
		{
			if (!bIsHeld && !bIsLerping)
				bOriginalReplicatesMovement = bReplicateMovement;
			bReplicateMovement = false;
		}
	}
	else
	{
		HoldingGrip.Clear();
		if (MovementReplicationSetting != EGripMovementReplicationSettings::ForceServerSideMovement)
		{
			bReplicateMovement = bOriginalReplicatesMovement;
		}
	}

	bIsHeld = bNewIsHeld;
}

bool UVRLeverComponent::GetGripScripts_Implementation(TArray<UVRGripScriptBase*> & ArrayReference)
{
	return false;
}

float UVRLeverComponent::ReCalculateCurrentAngle(bool bAllowThrowingEvents)
{
	FTransform CurRelativeTransform = this->GetComponentTransform().GetRelativeTransform(UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this));
	CalculateCurrentAngle(CurRelativeTransform);
	ProccessCurrentState(bIsLerping, bAllowThrowingEvents, bAllowThrowingEvents);
	return CurrentLeverAngle;
}

void UVRLeverComponent::SetLeverAngle(float NewAngle, FVector DualAxisForwardVector, bool bAllowThrowingEvents)
{
	NewAngle = -NewAngle; 

	FVector ForwardVector = DualAxisForwardVector;
	switch (LeverRotationAxis)
	{
	case EVRInteractibleLeverAxis::Axis_X:
		ForwardVector = FVector(FMath::Sign(NewAngle), 0.0f, 0.0f); break;
	case EVRInteractibleLeverAxis::Axis_Y:
		ForwardVector = FVector(0.0f, FMath::Sign(NewAngle), 0.0f); break;
	case EVRInteractibleLeverAxis::Axis_Z:
		ForwardVector = FVector(0.0f, 0.0f, FMath::Sign(NewAngle)); break;
	default:break;
	}

	FQuat NewLeverRotation(ForwardVector, FMath::DegreesToRadians(FMath::Abs(NewAngle)));

	this->SetRelativeTransform(FTransform(NewLeverRotation) * InitialRelativeTransform);
	ReCalculateCurrentAngle(bAllowThrowingEvents);
}

void UVRLeverComponent::ResetInitialLeverLocation(bool bAllowThrowingEvents)
{

	InitialRelativeTransform = this->GetRelativeTransform();

#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRLeverComponent, InitialRelativeTransform, this);
#endif

	CalculateCurrentAngle(InitialRelativeTransform);
	ProccessCurrentState(bIsLerping, bAllowThrowingEvents, bAllowThrowingEvents);
}

void UVRLeverComponent::CalculateCurrentAngle(FTransform & CurrentTransform)
{
	float Angle;
	switch (LeverRotationAxis)
	{
	case EVRInteractibleLeverAxis::Axis_XY:
	case EVRInteractibleLeverAxis::FlightStick_XY:
	{
		FTransform RelativeToSpace = CurrentTransform.GetRelativeTransform(InitialRelativeTransform);
		FQuat CurrentRelRot = RelativeToSpace.GetRotation();
		FVector UpVec = CurrentRelRot.GetUpVector();

		CurrentLeverForwardVector = FVector::VectorPlaneProject(UpVec, FVector::UpVector);
		CurrentLeverForwardVector.Normalize();

		FullCurrentAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(UpVec, FVector::UpVector)));
		CurrentLeverAngle = FMath::RoundToFloat(FullCurrentAngle);

		AllCurrentLeverAngles.Roll = FMath::Sign(UpVec.Y) * FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector(0.0f, UpVec.Y, UpVec.Z).GetSafeNormal(), FVector::UpVector)));
		AllCurrentLeverAngles.Pitch = FMath::Sign(UpVec.X) * FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector(UpVec.X, 0.0f, UpVec.Z).GetSafeNormal(), FVector::UpVector)));

		if (bBlendAxisValuesByAngleThreshold)
		{
			FVector ProjectedLoc = FVector(UpVec.X, UpVec.Y, 0.0f).GetSafeNormal();
			AllCurrentLeverAngles.Pitch *= FMath::Clamp(1.0f - (FMath::Abs(FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ProjectedLoc, FMath::Sign(UpVec.X) * FVector::ForwardVector)))) / AngleThreshold), 0.0f, 1.0f);
			AllCurrentLeverAngles.Roll *= FMath::Clamp(1.0f - (FMath::Abs(FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ProjectedLoc, FMath::Sign(UpVec.Y) * FVector::RightVector)))) / AngleThreshold), 0.0f, 1.0f);
		}

		AllCurrentLeverAngles.Roll = FMath::RoundToFloat(AllCurrentLeverAngles.Roll);
		AllCurrentLeverAngles.Pitch = FMath::RoundToFloat(AllCurrentLeverAngles.Pitch);

		if (LeverRotationAxis == EVRInteractibleLeverAxis::FlightStick_XY)
		{
			AllCurrentLeverAngles.Yaw = FRotator::NormalizeAxis(FMath::RoundToFloat(UVRExpansionFunctionLibrary::GetHMDPureYaw_I(CurrentRelRot.Rotator()).Yaw));
			if (FlightStickYawLimit < 180.0f)
			{
				AllCurrentLeverAngles.Yaw = FMath::Clamp(AllCurrentLeverAngles.Yaw, -FlightStickYawLimit, FlightStickYawLimit);
			}
		}
		else
			AllCurrentLeverAngles.Yaw = 0.0f;

	}break;
	default:
	{
		Angle = UVRInteractibleFunctionLibrary::GetDeltaAngleFromTransforms((EVRInteractibleAxis)LeverRotationAxis, InitialRelativeTransform, CurrentTransform);
		FullCurrentAngle = Angle;
		CurrentLeverAngle = FMath::RoundToFloat(FullCurrentAngle);
		CurrentLeverForwardVector = UVRInteractibleFunctionLibrary::SetAxisValueVec((EVRInteractibleAxis)LeverRotationAxis, FMath::Sign(Angle));
		AllCurrentLeverAngles = UVRInteractibleFunctionLibrary::SetAxisValueRot((EVRInteractibleAxis)LeverRotationAxis, CurrentLeverAngle, FRotator::ZeroRotator);

	}break;
	}
}

void UVRLeverComponent::LerpAxis(float CurrentAngle, float DeltaTime)
{
	float TargetAngle = 0.0f;
	float FinalReturnSpeed = LeverReturnSpeed;

	switch (LeverReturnTypeWhenReleased)
	{
	case EVRInteractibleLeverReturnType::LerpToMax:
	{
		if (CurrentAngle >= 0)
			TargetAngle = FMath::RoundToFloat(LeverLimitPositive);
		else
			TargetAngle = -FMath::RoundToFloat(LeverLimitNegative);
	}break;
	case EVRInteractibleLeverReturnType::LerpToMaxIfOverThreshold:
	{
		if ((!FMath::IsNearlyZero(LeverLimitPositive) && CurrentAngle >= (LeverLimitPositive * LeverTogglePercentage)))
			TargetAngle = FMath::RoundToFloat(LeverLimitPositive);
		else if ((!FMath::IsNearlyZero(LeverLimitNegative) && CurrentAngle <= -(LeverLimitNegative * LeverTogglePercentage)))
			TargetAngle = -FMath::RoundToFloat(LeverLimitNegative);
	}break;
	case EVRInteractibleLeverReturnType::RetainMomentum:
	{
		if (FMath::IsNearlyZero(MomentumAtDrop * DeltaTime, 0.1f))
		{
			MomentumAtDrop = 0.0f;
			this->SetComponentTickEnabled(false);
			bIsLerping = false;
			bReplicateMovement = bOriginalReplicatesMovement;
			return;
		}
		else
		{
			MomentumAtDrop = FMath::FInterpTo(MomentumAtDrop, 0.0f, DeltaTime, LeverMomentumFriction);

			FinalReturnSpeed = FMath::Abs(MomentumAtDrop);

			if (MomentumAtDrop >= 0.0f)
				TargetAngle = FMath::RoundToFloat(LeverLimitPositive);
			else
				TargetAngle = -FMath::RoundToFloat(LeverLimitNegative);
		}

	}break;
	case EVRInteractibleLeverReturnType::ReturnToZero:
	default:
	{}break;
	}

	float LerpedVal = FMath::FInterpConstantTo(CurrentAngle, TargetAngle, DeltaTime, FinalReturnSpeed);

	if (FMath::IsNearlyEqual(LerpedVal, TargetAngle))
	{
		if (LeverRestitution > 0.0f)
		{
			MomentumAtDrop = -(MomentumAtDrop * LeverRestitution);
			FTransform CalcTransform = (FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot((EVRInteractibleAxis)LeverRotationAxis, TargetAngle, FRotator::ZeroRotator)) * InitialRelativeTransform);
			this->SetRelativeRotation(CalcTransform.Rotator());
		}
		else
		{
			this->SetComponentTickEnabled(false);
			bIsLerping = false;
			bReplicateMovement = bOriginalReplicatesMovement;
			FTransform CalcTransform = (FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot((EVRInteractibleAxis)LeverRotationAxis, TargetAngle, FRotator::ZeroRotator)) * InitialRelativeTransform);
			this->SetRelativeRotation(CalcTransform.Rotator());
		}
	}
	else
	{
		FTransform CalcTransform = (FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot((EVRInteractibleAxis)LeverRotationAxis, LerpedVal, FRotator::ZeroRotator)) * InitialRelativeTransform);
		this->SetRelativeRotation(CalcTransform.Rotator());
	}
}

float UVRLeverComponent::CalcAngle(EVRInteractibleLeverAxis AxisToCalc, FVector CurInteractorLocation, bool bSkipLimits)
{
	float ReturnAxis = 0.0f;

	ReturnAxis = UVRInteractibleFunctionLibrary::GetAtan2Angle((EVRInteractibleAxis)AxisToCalc, CurInteractorLocation, InitialGripRot);

	if (bSkipLimits)
		return ReturnAxis;

	if (LeverLimitPositive > 0.0f && LeverLimitNegative > 0.0f && FMath::IsNearlyEqual(LeverLimitNegative, 180.f, 0.01f) && FMath::IsNearlyEqual(LeverLimitPositive, 180.f, 0.01f))
	{

	}
	else
	{
		ReturnAxis = FMath::ClampAngle(FRotator::NormalizeAxis(RotAtGrab + ReturnAxis), -LeverLimitNegative, LeverLimitPositive);

		if (!bIsInFirstTick && ((LeverLimitPositive > 0.0f && LastDeltaAngle >= LeverLimitPositive) || (LeverLimitNegative > 0.0f && LastDeltaAngle <= -LeverLimitNegative)) && FMath::Sign(LastDeltaAngle) != FMath::Sign(ReturnAxis))
		{
			ReturnAxis = LastDeltaAngle;
		}
	}

	bIsInFirstTick = false;
	return ReturnAxis;
}

void UVRLeverComponent::SetRepGameplayTags(bool bNewRepGameplayTags)
{
	bRepGameplayTags = bNewRepGameplayTags;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRLeverComponent, bRepGameplayTags, this);
#endif
}

void UVRLeverComponent::SetReplicateMovement(bool bNewReplicateMovement)
{
	bReplicateMovement = bNewReplicateMovement;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRLeverComponent, bReplicateMovement, this);
#endif
}

FGameplayTagContainer& UVRLeverComponent::GetGameplayTags()
{
#if WITH_PUSH_MODEL
	if (bRepGameplayTags)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UVRLeverComponent, GameplayTags, this);
	}
#endif

	return GameplayTags;
}