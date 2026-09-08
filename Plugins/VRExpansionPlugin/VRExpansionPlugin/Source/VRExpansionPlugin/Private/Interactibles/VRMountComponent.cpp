

#include "Interactibles/VRMountComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRMountComponent)

#include "VRExpansionFunctionLibrary.h"
#include "GripMotionControllerComponent.h"

#include "Net/UnrealNetwork.h"

#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

UVRMountComponent::UVRMountComponent(const FObjectInitializer& ObjectInitializer)
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

	MountRotationAxis = EVRInteractibleMountAxis::Axis_XZ;

	InitialRelativeTransform = FTransform::Identity;
	InitialInteractorLocation = FVector::ZeroVector;
	InitialGripRot = 0.0f;
	qRotAtGrab = FQuat::Identity;

	bDenyGripping = false;

	PrimarySlotRange = 100.f;
	SecondarySlotRange = 100.f;
	GripPriority = 1;

	FlipingZone = 0.4f;
	FlipReajustYawSpeed = 7.7f;

	this->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

UVRMountComponent::~UVRMountComponent()
{
}

void UVRMountComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRMountComponent, bRepGameplayTags, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UVRMountComponent, bReplicateMovement, PushModelParams);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_Custom, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UVRMountComponent, GameplayTags, PushModelParamsWithCondition);
}

void UVRMountComponent::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(UVRMountComponent, GameplayTags, bRepGameplayTags);

	if (!IRISNetReplication::IsIris(this))
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeLocation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeRotation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeScale3D, bReplicateMovement);
	}
}

void UVRMountComponent::OnRegister()
{
	Super::OnRegister();
	ResetInitialMountLocation(); 
}

void UVRMountComponent::BeginPlay()
{

	Super::BeginPlay();
}

void UVRMountComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UVRMountComponent::OnUnregister()
{
	Super::OnUnregister();
}

void UVRMountComponent::TickGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation, float DeltaTime)
{

	FTransform CurrentRelativeTransform = InitialRelativeTransform * UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this);
	FVector CurInteractorLocation = CurrentRelativeTransform.InverseTransformPosition(GrippingController->GetPivotLocation());

	switch (MountRotationAxis)
	{
	case EVRInteractibleMountAxis::Axis_XZ:
	{

		FVector MountToTarget;

		if (GrippedOnBack)
		{
			CurInteractorLocation = CurInteractorLocation *-1;
		}

		FRotator RelativeRot = GetRelativeRotation();
		FVector CurToForwardAxisVec = FRotator(RelativeRot.Pitch, RelativeRot.Yaw, TwistDiff).RotateVector(InitialGripToForwardVec);

		CurInteractorLocation = (CurInteractorLocation.GetSafeNormal() * InitialInteractorLocation.Size() + CurToForwardAxisVec).GetSafeNormal()*CurInteractorLocation.Size();

		FRotator Rot;

		if (bIsInsideFrontFlipingZone || bIsInsideBackFlipZone)
		{

			if (!bFirstEntryToHalfFlipZone)
			{

				ForwardPullPlane = FPlane(FVector::ZeroVector, FVector(EntryRightVec.X, EntryRightVec.Y, 0).GetSafeNormal());

				bFirstEntryToHalfFlipZone = true;
				bLerpingOutOfFlipZone = false;

				CurInterpGripLoc = CurInteractorLocation;

				CurPointOnForwardPlane = FPlane::PointPlaneProject(CurInteractorLocation, ForwardPullPlane);
			}

			LastPointOnForwardPlane = CurPointOnForwardPlane;

			CurPointOnForwardPlane = FPlane::PointPlaneProject(CurInteractorLocation, ForwardPullPlane);

			FVector ForwardPlainDiffVec = CurPointOnForwardPlane - LastPointOnForwardPlane;

			CurInterpGripLoc += ForwardPlainDiffVec;

			CurInterpGripLoc = FMath::VInterpConstantTo(CurInterpGripLoc, CurPointOnForwardPlane, GetWorld()->GetDeltaSeconds(), 50);

			MountToTarget = FVector(CurInterpGripLoc.X, CurInterpGripLoc.Y, CurInteractorLocation.Z).GetSafeNormal();

			CurInteractorLocation = FVector(CurInterpGripLoc.X, CurInterpGripLoc.Y, CurInteractorLocation.Z);
		}
		else
		{

			if (bLerpingOutOfFlipZone)
			{

				if ((CurInterpGripLoc - CurInteractorLocation).Size() >= 1.0f)
				{

					LerpOutAlpha = FMath::Clamp(LerpOutAlpha + FlipReajustYawSpeed / 100.0f, 0.0f, 1.0f);

					CurInterpGripLoc = CurInterpGripLoc + LerpOutAlpha * (CurInteractorLocation - CurInterpGripLoc);

					MountToTarget = CurInterpGripLoc.GetSafeNormal();

					if (LerpOutAlpha >= 0.97f)
					{
						bLerpingOutOfFlipZone = false;
						bIsInsideBackFlipZone = false;
					}
				}
				else
				{

					bLerpingOutOfFlipZone = false;
					bIsInsideBackFlipZone = false;
					MountToTarget = CurInteractorLocation.GetSafeNormal();
				}
			}
			else
			{

				MountToTarget = CurInteractorLocation.GetSafeNormal();
				bIsInsideBackFlipZone = false;
			}

		}

		Rot = MountToTarget.Rotation();
		this->SetRelativeRotation((FTransform(FRotator(Rot.Pitch, Rot.Yaw, 0))*InitialRelativeTransform).Rotator());

		FVector nAxis;
		float FlipAngle;
		FQuat::FindBetweenVectors(FVector(0, 0, 1), CurInteractorLocation.GetSafeNormal()).ToAxisAndAngle(nAxis, FlipAngle);

		if (FlipAngle < FlipingZone || FlipAngle > PI - FlipingZone)
		{

			if (!bIsInsideFrontFlipingZone && !bIsInsideBackFlipZone)
			{

				bIsInsideFrontFlipingZone = true;

				if (USceneComponent * ParentComp = GetAttachParent())
				{
					EntryUpVec = ParentComp->GetComponentRotation().UnrotateVector(GetUpVector());
					EntryRightVec = ParentComp->GetComponentRotation().UnrotateVector(GetRightVector());
				}
				else
				{
					EntryUpVec = GetUpVector();
					EntryRightVec = GetRightVector();
				}

				EntryUpXYNeg = FVector(EntryUpVec.X, EntryUpVec.Y, 0).GetSafeNormal()*-1;

				if (FlipAngle > PI - FlipingZone)
				{
					EntryUpXYNeg *= -1;
				}

				FlipPlane = FPlane(FVector::ZeroVector, EntryUpXYNeg);
			}

			FVector CurInteractorToFlipPlaneVec = CurInteractorLocation - FPlane::PointPlaneProject(CurInteractorLocation, FlipPlane);

			if (bIsInsideFrontFlipingZone)
			{

				if (FVector::DotProduct(CurInteractorToFlipPlaneVec, EntryUpXYNeg) <= 0)
				{
					bIsInsideFrontFlipingZone = false;
					bIsInsideBackFlipZone = true;

					bIsFlipped = !bIsFlipped;

					if (bIsFlipped)
					{
						TwistDiff = 180;
					}
					else
					{
						TwistDiff = 0;
					}
				}
				else
				{

					FVector RelativeUpVec = GetUpVector();

					if(USceneComponent * ParentComp = GetAttachParent())
						RelativeUpVec = ParentComp->GetComponentRotation().UnrotateVector(RelativeUpVec);

					FVector CurrentUpVec = FVector(RelativeUpVec.X, RelativeUpVec.Y, 0).GetSafeNormal();

					if (FlipAngle < FlipingZone)
					{
						CurrentUpVec *= -1;
					}

					float EntryTwist = FMath::Atan2(EntryUpXYNeg.Y, EntryUpXYNeg.X);
					float CurTwist = FMath::Atan2(CurrentUpVec.Y, CurrentUpVec.X);

					if (bIsFlipped)
					{
						TwistDiff = FMath::RadiansToDegrees(EntryTwist - CurTwist - PI);
					}
					else
					{
						TwistDiff = FMath::RadiansToDegrees(EntryTwist - CurTwist);
					}
				}
			}
			else
			{

				if (bIsInsideBackFlipZone)
				{
					if (FVector::DotProduct(CurInteractorToFlipPlaneVec, EntryUpXYNeg) >= 0)
					{
						bIsInsideFrontFlipingZone = true;
						bIsInsideBackFlipZone = false;

						bIsFlipped = !bIsFlipped;

						if (bIsFlipped)
						{
							TwistDiff = 180;
						}
						else
						{
							TwistDiff = 0;
						}
					}
				}
			}

		}
		else
		{

			bIsInsideFrontFlipingZone = false;
			bIsInsideBackFlipZone = false;

			if (bIsFlipped)
			{
				TwistDiff = 180;
			}
			else
			{
				TwistDiff = 0;
			}

			if (bFirstEntryToHalfFlipZone)
			{

				bFirstEntryToHalfFlipZone = false;
				LerpOutAlpha = 0;

				bLerpingOutOfFlipZone = true;
			}

		}

		this->AddLocalRotation(FRotator(0, 0, -TwistDiff));

	}break;
	default:break;
	}

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
		return;
	}
}

void UVRMountComponent::OnGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation)
{	
	FTransform CurrentRelativeTransform = InitialRelativeTransform * UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(this);

	FTransform ReversedRelativeTransform = FTransform(GripInformation.RelativeTransform.ToInverseMatrixWithScale());
	FTransform RelativeToGripTransform = ReversedRelativeTransform * this->GetComponentTransform();

	InitialInteractorLocation = CurrentRelativeTransform.InverseTransformPosition(RelativeToGripTransform.GetTranslation());
	InitialInteractorDropLocation = ReversedRelativeTransform.GetTranslation();

	switch (MountRotationAxis)
	{
	case EVRInteractibleMountAxis::Axis_XZ:
	{

		qRotAtGrab = this->GetComponentTransform().GetRelativeTransform(CurrentRelativeTransform).GetRotation();

		FVector ForwardVectorToUse = GetForwardVector();

		if (USceneComponent * ParentComp = GetAttachParent())
		{
			ForwardVectorToUse = ParentComp->GetComponentRotation().UnrotateVector(ForwardVectorToUse);
		}

		InitialForwardVector = InitialInteractorLocation.Size() * ForwardVectorToUse;

		if (FVector::DotProduct(InitialInteractorLocation, ForwardVectorToUse) <= 0)
		{
			GrippedOnBack = true;
			InitialGripToForwardVec = (InitialForwardVector + InitialInteractorLocation);
		}
		else
		{
			InitialGripToForwardVec = InitialForwardVector - InitialInteractorLocation;
			GrippedOnBack = false;
		}

		FRotator RelativeRot = GetRelativeRotation();
		InitialGripToForwardVec = FRotator(RelativeRot.Pitch, RelativeRot.Yaw, TwistDiff).UnrotateVector(InitialGripToForwardVec);

	}break;
	default:break;
	}

	this->SetComponentTickEnabled(true);
}

void UVRMountComponent::OnGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed)
{
		this->SetComponentTickEnabled(false);
}

void UVRMountComponent::SetGripPriority(int NewGripPriority)
{
	GripPriority = NewGripPriority;
}

void UVRMountComponent::OnChildGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) {}
void UVRMountComponent::OnChildGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed) {}
void UVRMountComponent::OnSecondaryGrip_Implementation(UGripMotionControllerComponent * GripOwningController, USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation) {}
void UVRMountComponent::OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent * GripOwningController, USceneComponent * ReleasingSecondaryGripComponent, const FBPActorGripInformation & GripInformation) {}
void UVRMountComponent::OnUsed_Implementation() {}
void UVRMountComponent::OnEndUsed_Implementation() {}
void UVRMountComponent::OnSecondaryUsed_Implementation() {}
void UVRMountComponent::OnEndSecondaryUsed_Implementation() {}
void UVRMountComponent::OnInput_Implementation(FKey Key, EInputEvent KeyEvent) {}
bool UVRMountComponent::RequestsSocketing_Implementation(USceneComponent *& ParentToSocketTo, FName & OptionalSocketName, FTransform_NetQuantize & RelativeTransform) { return false; }

bool UVRMountComponent::DenyGripping_Implementation(UGripMotionControllerComponent * GripInitiator)
{
	return bDenyGripping;
}

EGripInterfaceTeleportBehavior UVRMountComponent::TeleportBehavior_Implementation()
{
	return EGripInterfaceTeleportBehavior::DropOnTeleport;
}

bool UVRMountComponent::SimulateOnDrop_Implementation()
{
	return false;
}

EGripCollisionType UVRMountComponent::GetPrimaryGripType_Implementation(bool bIsSlot)
{
		return EGripCollisionType::CustomGrip;
}

ESecondaryGripType UVRMountComponent::SecondaryGripType_Implementation()
{
	return ESecondaryGripType::SG_None;
}

EGripMovementReplicationSettings UVRMountComponent::GripMovementReplicationType_Implementation()
{
	return MovementReplicationSetting;
}

EGripLateUpdateSettings UVRMountComponent::GripLateUpdateSetting_Implementation()
{
	return EGripLateUpdateSettings::LateUpdatesAlwaysOff;
}

void UVRMountComponent::GetGripStiffnessAndDamping_Implementation(float &GripStiffnessOut, float &GripDampingOut)
{
	GripStiffnessOut = Stiffness;
	GripDampingOut = Damping;
}

FBPAdvGripSettings UVRMountComponent::AdvancedGripSettings_Implementation()
{
	FBPAdvGripSettings GripSettings(GripPriority);
	GripSettings.bDisallowSettingPositionOnClientAuthDrop = true;
	return GripSettings;
}

float UVRMountComponent::GripBreakDistance_Implementation()
{
	return BreakDistance;
}

void UVRMountComponent::ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool & bHadSlotInRange, FTransform & SlotWorldTransform, FName & SlotName, UGripMotionControllerComponent * CallingController, FName OverridePrefix)
{
	if (OverridePrefix.IsNone())
		bSecondarySlot ? OverridePrefix = "VRGripS" : OverridePrefix = "VRGripP";

	UVRExpansionFunctionLibrary::GetGripSlotInRangeByTypeName_Component(OverridePrefix, this, WorldLocation, bSecondarySlot ? SecondarySlotRange : PrimarySlotRange, bHadSlotInRange, SlotWorldTransform, SlotName, CallingController);
}

bool UVRMountComponent::AllowsMultipleGrips_Implementation()
{
	return false;
}

void UVRMountComponent::IsHeld_Implementation(TArray<FBPGripPair> & CurHoldingControllers, bool & bCurIsHeld)
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

void UVRMountComponent::SetHeld_Implementation(UGripMotionControllerComponent * NewHoldingController, uint8 GripID, bool bNewIsHeld)
{
	if (bNewIsHeld)
	{
		HoldingGrip = FBPGripPair(NewHoldingController, GripID);
		if (MovementReplicationSetting != EGripMovementReplicationSettings::ForceServerSideMovement)
		{
			if (!bIsHeld)
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

void UVRMountComponent::ResetInitialMountLocation()
{

	InitialRelativeTransform = this->GetRelativeTransform();
}

bool UVRMountComponent::GetGripScripts_Implementation(TArray<UVRGripScriptBase*> & ArrayReference)
{
	return false;
}

void UVRMountComponent::SetRepGameplayTags(bool bNewRepGameplayTags)
{
	bRepGameplayTags = bNewRepGameplayTags;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRMountComponent, bRepGameplayTags, this);
#endif
}

void UVRMountComponent::SetReplicateMovement(bool bNewReplicateMovement)
{
	bReplicateMovement = bNewReplicateMovement;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UVRMountComponent, bReplicateMovement, this);
#endif
}

FGameplayTagContainer& UVRMountComponent::GetGameplayTags()
{
#if WITH_PUSH_MODEL
	if (bRepGameplayTags)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UVRMountComponent, GameplayTags, this);
	}
#endif

	return GameplayTags;
}