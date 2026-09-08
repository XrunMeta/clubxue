

#include "Interactibles/VRDialComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRDialComponent)

#include "VRExpansionFunctionLibrary.h"
#include "GripMotionControllerComponent.h"
#include "Net/UnrealNetwork.h"

#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

UVRDialComponent::UVRDialComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SetGenerateOverlapEvents(true);
	this->PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;

	bRepGameplayTags = false;

	bReplicateMovement = true;

	DialRotationAxis = EVRInteractibleAxis::Axis_Z;
	InteractorRotationAxis = EVRInteractibleAxis::Axis_X;

	bDialUsesAngleSnap = false;
	bDialUseSnapAngleList = false;
	SnapAngleThreshold = 45.0f;
	SnapAngleIncrement = 45.0f;
	LastSnapAngle = 0.0f;
	RotationScaler = 1.0f;

	ClockwiseMaximumDialAngle = 180.0f;
	CClockwiseMaximumDialAngle = 180.0f;
	bDenyGripping = false;

	PrimarySlotRange = 100.f;
	SecondarySlotRange = 100.f;
	GripPriority = 1;

	MovementReplicationSetting = EGripMovementReplicationSettings::ForceClientSideMovement;
	BreakDistance = 100.0f;

	bLerpBackOnRelease = false;
	bSendDialEventsDuringLerp = false;
	DialReturnSpeed = 90.0f;
	bIsLerping = false;

	bDialUseDirectHandRotation = false;
	LastGripRot = 0.0f;
	InitialGripRot = 0.f;
	InitialRotBackEnd = 0.f;
	bUseRollover = false;
}

UVRDialComponent::~UVRDialComponent()
{
}

void UVRDialComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRDialComponent, InitialRelativeTransform, PushModelParams);

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRDialComponent, bRepGameplayTags, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UVRDialComponent, bReplicateMovement, PushModelParams);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_Custom, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRDialComponent, GameplayTags, PushModelParamsWithCondition);
}

void UVRDialComponent::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(UVRDialComponent, GameplayTags, bRepGameplayTags);

	if (!IRISNetReplication::IsIris(this))
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeLocation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeRotation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeScale3D, bReplicateMovement);
	}
}

void UVRDialComponent::OnRegister()
{
	Super::OnRegister();
	ResetInitialDialLocation(); 
}

void UVRDialComponent::BeginPlay()
{

	Super::BeginPlay();
	CalculateDialProgress();

	bOriginalReplicatesMovement = bReplicateMovement;
}

void UVRDialComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (bIsLerping)
	{
		if (bUseRollover)
		{
			this->SetDialAngle(FMath::FInterpConstantTo(CurRotBackEnd, 0.f, DeltaTime, DialReturnSpeed), bSendDialEventsDuringLerp);
		}
		else
		{

			if (CurrentDialAngle > ClockwiseMaximumDialAngle)
				this->SetDialAngle(FMath::FInterpConstantTo(CurRotBackEnd, 360.f, DeltaTime, DialReturnSpeed), bSendDialEventsDuringLerp);
			else
				this->SetDialAngle(FMath::FInterpConstantTo(CurRotBackEnd, 0.f, DeltaTime, DialReturnSpeed), bSendDialEventsDuringLerp);
		}

		if (CurRotBackEnd == 0.f)
		{
			this->SetComponentTickEnabled(false);
			bIsLerping = false;
			OnDialFinishedLerping.Broadcast();
			ReceiveDialFinishedLerping();
		}
	}
	else
	{
		this->SetComponentTickEnabled(false); 
	}
}

void UVRDialComponent::TickGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation, float DeltaTime) 
{

	float DeltaRot = 0.0f;

	if (!bDialUseDirectHandRotation)
	{
		FTransform CurrentRelativeTransform = InitialRelativeTransform * UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this);
		FVector CurInteractorLocation = CurrentRelativeTransform.InverseTransformPosition(GrippingController->GetPivotLocation());

		float NewRot = FRotator::ClampAxis(UVRInteractibleFunctionLibrary::GetAtan2Angle(DialRotationAxis, CurInteractorLocation));

		DeltaRot = RotationScaler * FMath::FindDeltaAngleDegrees(LastGripRot, NewRot);

		float LimitTest = FRotator::ClampAxis(((NewRot - InitialGripRot) + InitialRotBackEnd));
		float MaxCheckValue = bUseRollover ? -CClockwiseMaximumDialAngle : 360.0f - CClockwiseMaximumDialAngle;

		if (FMath::IsNearlyZero(CClockwiseMaximumDialAngle))
		{
			if (LimitTest > ClockwiseMaximumDialAngle && (CurRotBackEnd == ClockwiseMaximumDialAngle || CurRotBackEnd == 0.f))
			{
				DeltaRot = 0.f;
			}
		}
		else if (FMath::IsNearlyZero(ClockwiseMaximumDialAngle))
		{
			if (LimitTest < MaxCheckValue && (CurRotBackEnd == MaxCheckValue || CurRotBackEnd == 0.f))
			{
				DeltaRot = 0.f;
			}
		}
		else if (LimitTest > ClockwiseMaximumDialAngle && LimitTest < MaxCheckValue && (CurRotBackEnd == ClockwiseMaximumDialAngle || CurRotBackEnd == MaxCheckValue))
		{
			DeltaRot = 0.f;
		}

		LastGripRot = NewRot;
	}
	else
	{
		FRotator curRotation = GrippingController->GetComponentRotation();
		DeltaRot = RotationScaler * UVRInteractibleFunctionLibrary::GetAxisValue(InteractorRotationAxis, (curRotation - LastRotation).GetNormalized());
		LastRotation = curRotation;
	}

	AddDialAngle(DeltaRot, true);

	if (BreakDistance > 0.f && GrippingController->HasGripAuthority(GripInformation) && FVector::DistSquared(InitialDropLocation, this->GetComponentTransform().InverseTransformPosition(GrippingController->GetPivotLocation())) >= FMath::Square(BreakDistance))
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
		return;
	}
}

void UVRDialComponent::OnGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) 
{
	FTransform CurrentRelativeTransform = InitialRelativeTransform * UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this);

	FTransform ReversedRelativeTransform = FTransform(GripInformation.RelativeTransform.ToInverseMatrixWithScale());
	FTransform CurrentTransform = this->GetComponentTransform();
	FTransform RelativeToGripTransform = ReversedRelativeTransform * CurrentTransform;

	InitialInteractorLocation = CurrentRelativeTransform.InverseTransformPosition(RelativeToGripTransform.GetTranslation());
	InitialDropLocation = ReversedRelativeTransform.GetTranslation();

	if (!bDialUseDirectHandRotation)
	{
		LastGripRot = FRotator::ClampAxis(UVRInteractibleFunctionLibrary::GetAtan2Angle(DialRotationAxis, InitialInteractorLocation));
		InitialGripRot = LastGripRot;
		InitialRotBackEnd = CurRotBackEnd;
	}
	else
	{
		LastRotation = RelativeToGripTransform.GetRotation().Rotator(); 
	}

	bIsLerping = false;

}

void UVRDialComponent::OnGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed) 
{
	if (bDialUsesAngleSnap && bDialUseSnapAngleList)
	{
		float closestAngle = 0.f;
		float closestVal = FMath::Abs(closestAngle - CurRotBackEnd);
		float closestValt = 0.f;
		for (float val : DialSnapAngleList)
		{
			closestValt = FMath::Abs(val - CurRotBackEnd);
			if (closestValt < closestVal)
			{
				closestAngle = val;
				closestVal = closestValt;
			}
		}

		if (closestAngle != LastSnapAngle)
		{
			this->SetRelativeRotation((FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot(DialRotationAxis, FMath::UnwindDegrees(closestAngle), FRotator::ZeroRotator)) * InitialRelativeTransform).Rotator());
			CurrentDialAngle = FMath::RoundToFloat(closestAngle);
			CurRotBackEnd = CurrentDialAngle;

			if (!FMath::IsNearlyEqual(LastSnapAngle, CurrentDialAngle))
			{
				ReceiveDialHitSnapAngle(CurrentDialAngle);
				OnDialHitSnapAngle.Broadcast(CurrentDialAngle);
				LastSnapAngle = CurrentDialAngle;
			}
		}
	}
	else if (bDialUsesAngleSnap && SnapAngleIncrement > 0.f)
	{

		float AngleOffsetCheck = FMath::Abs(FRotator::ClampAxis(CurRotBackEnd) - FRotator::ClampAxis(LastSnapAngle));
		float TargetSnap = FMath::RoundToFloat(FMath::GridSnap(CurRotBackEnd, SnapAngleIncrement));

		if (FMath::Abs(CurRotBackEnd - TargetSnap) <= FMath::Min(SnapAngleIncrement, SnapAngleThreshold))
		{
			if (AngleOffsetCheck >= SnapAngleThreshold)
			{
				this->SetRelativeRotation((FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot(DialRotationAxis, FMath::GridSnap(CurRotBackEnd, SnapAngleIncrement), FRotator::ZeroRotator)) * InitialRelativeTransform).Rotator());
				CurRotBackEnd = FMath::GridSnap(CurRotBackEnd, SnapAngleIncrement);

				if (bUseRollover)
				{
					CurrentDialAngle = FMath::RoundToFloat(CurRotBackEnd);
				}
				else
				{
					CurrentDialAngle = FRotator::ClampAxis(FMath::RoundToFloat(CurRotBackEnd));
				}
			}
			else
			{

				CurRotBackEnd = CurrentDialAngle;
			}
		}
		else
		{

			CurRotBackEnd = CurrentDialAngle;
		}

		if (!FMath::IsNearlyEqual(LastSnapAngle, CurrentDialAngle))
		{
			ReceiveDialHitSnapAngle(CurrentDialAngle);
			OnDialHitSnapAngle.Broadcast(CurrentDialAngle);
			LastSnapAngle = CurrentDialAngle;
		}
	}

	if (bLerpBackOnRelease)
	{
		bIsLerping = true;
		this->SetComponentTickEnabled(true);
	}
	else
		this->SetComponentTickEnabled(false);

}

void UVRDialComponent::SetGripPriority(int NewGripPriority)
{
	GripPriority = NewGripPriority;
}

void UVRDialComponent::OnChildGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) {}
void UVRDialComponent::OnChildGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed) {}
void UVRDialComponent::OnSecondaryGrip_Implementation(UGripMotionControllerComponent * GripOwningController, USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation) {}
void UVRDialComponent::OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent * GripOwningController, USceneComponent * ReleasingSecondaryGripComponent, const FBPActorGripInformation & GripInformation) {}
void UVRDialComponent::OnUsed_Implementation() {}
void UVRDialComponent::OnEndUsed_Implementation() {}
void UVRDialComponent::OnSecondaryUsed_Implementation() {}
void UVRDialComponent::OnEndSecondaryUsed_Implementation() {}
void UVRDialComponent::OnInput_Implementation(FKey Key, EInputEvent KeyEvent) {}
bool UVRDialComponent::RequestsSocketing_Implementation(USceneComponent *& ParentToSocketTo, FName & OptionalSocketName, FTransform_NetQuantize & RelativeTransform) { return false; }

bool UVRDialComponent::DenyGripping_Implementation(UGripMotionControllerComponent * GripInitiator)
{
	return bDenyGripping;
}

EGripInterfaceTeleportBehavior UVRDialComponent::TeleportBehavior_Implementation()
{
	return EGripInterfaceTeleportBehavior::DropOnTeleport;
}

bool UVRDialComponent::SimulateOnDrop_Implementation()
{
	return false;
}

EGripCollisionType UVRDialComponent::GetPrimaryGripType_Implementation(bool bIsSlot)
{
	return EGripCollisionType::CustomGrip;
}

ESecondaryGripType UVRDialComponent::SecondaryGripType_Implementation()
{
	return ESecondaryGripType::SG_None;
}

EGripMovementReplicationSettings UVRDialComponent::GripMovementReplicationType_Implementation()
{
	return MovementReplicationSetting;
}

EGripLateUpdateSettings UVRDialComponent::GripLateUpdateSetting_Implementation()
{
	return EGripLateUpdateSettings::LateUpdatesAlwaysOff;
}

void UVRDialComponent::GetGripStiffnessAndDamping_Implementation(float &GripStiffnessOut, float &GripDampingOut)
{
	GripStiffnessOut = 0.0f;
	GripDampingOut = 0.0f;
}

FBPAdvGripSettings UVRDialComponent::AdvancedGripSettings_Implementation()
{
	FBPAdvGripSettings GripSettings(GripPriority);
	GripSettings.bDisallowSettingPositionOnClientAuthDrop = true;
	return GripSettings;
}

float UVRDialComponent::GripBreakDistance_Implementation()
{
	return BreakDistance;
}

void UVRDialComponent::ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool & bHadSlotInRange, FTransform & SlotWorldTransform, FName & SlotName, UGripMotionControllerComponent * CallingController, FName OverridePrefix)
{
	if (OverridePrefix.IsNone())
		bSecondarySlot ? OverridePrefix = "VRGripS" : OverridePrefix = "VRGripP";

	UVRExpansionFunctionLibrary::GetGripSlotInRangeByTypeName_Component(OverridePrefix, this, WorldLocation, bSecondarySlot ? SecondarySlotRange : PrimarySlotRange, bHadSlotInRange, SlotWorldTransform, SlotName, CallingController);
}

bool UVRDialComponent::AllowsMultipleGrips_Implementation()
{
	return false;
}

void UVRDialComponent::IsHeld_Implementation(TArray<FBPGripPair> & CurHoldingControllers, bool & bCurIsHeld)
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

void UVRDialComponent::Native_NotifyThrowGripDelegates(UGripMotionControllerComponent* Controller, bool bGripped, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
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

void UVRDialComponent::SetHeld_Implementation(UGripMotionControllerComponent * NewHoldingController, uint8 GripID, bool bNewIsHeld)
{
	if (bNewIsHeld)
	{
		HoldingGrip = FBPGripPair(NewHoldingController, GripID);
		if (MovementReplicationSetting != EGripMovementReplicationSettings::ForceServerSideMovement)
		{
			if(!bIsHeld)
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

bool UVRDialComponent::GetGripScripts_Implementation(TArray<UVRGripScriptBase*> & ArrayReference)
{
	return false;
}

void UVRDialComponent::SetDialAngle(float DialAngle, bool bCallEvents)
{
	CurRotBackEnd = DialAngle;
	AddDialAngle(0.0f, bCallEvents);
}

void UVRDialComponent::AddDialAngle(float DialAngleDelta, bool bCallEvents, bool bSkipSettingRot)
{

	float MaxCheckValue = bUseRollover ? -CClockwiseMaximumDialAngle : 360.0f - CClockwiseMaximumDialAngle;

	float DeltaRot = DialAngleDelta;
	float tempCheck = bUseRollover ? CurRotBackEnd + DeltaRot : FRotator::ClampAxis(CurRotBackEnd + DeltaRot);

	if (FMath::IsNearlyZero(CClockwiseMaximumDialAngle))
	{
		CurRotBackEnd = FMath::Clamp(CurRotBackEnd + DeltaRot, 0.0f, ClockwiseMaximumDialAngle);
	}
	else if (FMath::IsNearlyZero(ClockwiseMaximumDialAngle))
	{
		if (bUseRollover)
		{
			CurRotBackEnd = FMath::Clamp(CurRotBackEnd + DeltaRot, -CClockwiseMaximumDialAngle, 0.0f);
		}
		else
		{
			if (CurRotBackEnd < MaxCheckValue)
				CurRotBackEnd = FMath::Clamp(360.0f + DeltaRot, MaxCheckValue, 360.0f);
			else
				CurRotBackEnd = FMath::Clamp(CurRotBackEnd + DeltaRot, MaxCheckValue, 360.0f);
		}
	}
	else if (!bUseRollover && tempCheck > ClockwiseMaximumDialAngle && tempCheck < MaxCheckValue)
	{
		if (CurRotBackEnd < MaxCheckValue)
		{
			CurRotBackEnd = ClockwiseMaximumDialAngle;
		}
		else
		{
			CurRotBackEnd = MaxCheckValue;
		}
	}
	else if (bUseRollover)
	{
		if (tempCheck > ClockwiseMaximumDialAngle)
		{
			CurRotBackEnd = ClockwiseMaximumDialAngle;
		}
		else if (tempCheck < MaxCheckValue)
		{
			CurRotBackEnd = MaxCheckValue;
		}
		else
		{
			CurRotBackEnd = tempCheck;
		}
	}
	else
	{
		CurRotBackEnd = tempCheck;
	}

	if (bDialUsesAngleSnap && bDialUseSnapAngleList)
	{
		float closestAngle = 0.f;

		float closestVal = FMath::Abs(closestAngle - CurRotBackEnd);
		float closestValt = 0.f;
		for (float val : DialSnapAngleList)
		{
			closestValt = FMath::Abs(val - CurRotBackEnd);
			if (closestValt < closestVal)
			{
				closestAngle = val;
				closestVal = closestValt;
			}
		}

		if (closestAngle != LastSnapAngle)
		{
			if (!bSkipSettingRot)
				this->SetRelativeRotation((FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot(DialRotationAxis, FMath::UnwindDegrees(closestAngle), FRotator::ZeroRotator)) * InitialRelativeTransform).Rotator());
			CurrentDialAngle = FMath::RoundToFloat(closestAngle);

			if (bCallEvents && !FMath::IsNearlyEqual(LastSnapAngle, CurrentDialAngle))
			{
				ReceiveDialHitSnapAngle(CurrentDialAngle);
				OnDialHitSnapAngle.Broadcast(CurrentDialAngle);
			}

			LastSnapAngle = CurrentDialAngle;
		}
	}
	else if (bDialUsesAngleSnap && SnapAngleIncrement > 0.f)
	{
		float AngleOffsetCheck = FMath::Abs(FRotator::ClampAxis(CurRotBackEnd) - FRotator::ClampAxis(LastSnapAngle));
		float TargetSnap = FMath::RoundToFloat(FMath::GridSnap(CurRotBackEnd, SnapAngleIncrement));

		if (FMath::Abs(CurRotBackEnd - TargetSnap) <= FMath::Min(SnapAngleIncrement, SnapAngleThreshold))
		{
			if (AngleOffsetCheck >= SnapAngleThreshold)
			{
				if (!bSkipSettingRot)
					this->SetRelativeRotation((FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot(DialRotationAxis, FMath::UnwindDegrees(FMath::GridSnap(CurRotBackEnd, SnapAngleIncrement)), FRotator::ZeroRotator)) * InitialRelativeTransform).Rotator());
				CurrentDialAngle = FMath::RoundToFloat(FMath::GridSnap(CurRotBackEnd, SnapAngleIncrement));

				if (bCallEvents && !FMath::IsNearlyEqual(LastSnapAngle, CurrentDialAngle))
				{
					ReceiveDialHitSnapAngle(CurrentDialAngle);
					OnDialHitSnapAngle.Broadcast(CurrentDialAngle);
				}

				LastSnapAngle = CurrentDialAngle;
			}
		}
		else
		{
			if (!bSkipSettingRot)
				this->SetRelativeRotation((FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot(DialRotationAxis, FMath::UnwindDegrees(CurRotBackEnd), FRotator::ZeroRotator)) * InitialRelativeTransform).Rotator());
			CurrentDialAngle = FMath::RoundToFloat(CurRotBackEnd);
		}
	}
	else
	{
		if (!bSkipSettingRot)
			this->SetRelativeRotation((FTransform(UVRInteractibleFunctionLibrary::SetAxisValueRot(DialRotationAxis, FMath::UnwindDegrees(CurRotBackEnd), FRotator::ZeroRotator)) * InitialRelativeTransform).Rotator());
		CurrentDialAngle = FMath::RoundToFloat(CurRotBackEnd);
	}

}

void UVRDialComponent::ResetInitialDialLocation()
{

	InitialRelativeTransform = this->GetRelativeTransform();

#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRDialComponent, InitialRelativeTransform, this);
#endif

	CurRotBackEnd = 0.0f;
	CalculateDialProgress();
}

void UVRDialComponent::CalculateDialProgress()
{
	FTransform CurRelativeTransform = this->GetComponentTransform().GetRelativeTransform(UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this));
	LastGripRot = UVRInteractibleFunctionLibrary::GetDeltaAngleFromTransforms(DialRotationAxis, InitialRelativeTransform, CurRelativeTransform);
	CurRotBackEnd = LastGripRot;
	AddDialAngle(0.0f, false, true);
}

void UVRDialComponent::SetRepGameplayTags(bool bNewRepGameplayTags)
{
	bRepGameplayTags = bNewRepGameplayTags;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRDialComponent, bRepGameplayTags, this);
#endif
}

void UVRDialComponent::SetReplicateMovement(bool bNewReplicateMovement)
{
	bReplicateMovement = bNewReplicateMovement;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRDialComponent, bReplicateMovement, this);
#endif
}

FGameplayTagContainer& UVRDialComponent::GetGameplayTags()
{
#if WITH_PUSH_MODEL
	if (bRepGameplayTags)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UVRDialComponent, GameplayTags, this);
	}
#endif

	return GameplayTags;
}