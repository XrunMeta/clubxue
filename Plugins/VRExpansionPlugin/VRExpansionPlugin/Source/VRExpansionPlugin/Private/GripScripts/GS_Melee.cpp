

#include "GripScripts/GS_Melee.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GS_Melee)

#include "VRGripInterface.h"
#include "GameFramework/WorldSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "VRExpansionFunctionLibrary.h"
#include "GripMotionControllerComponent.h"
#include "VRGlobalSettings.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GripMotionControllerComponent.h"

UGS_Melee::UGS_Melee(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	bIsActive = true;
	WorldTransformOverrideType = EGSTransformOverrideType::ModifiesWorldTransform;
	bDenyLateUpdates = true;

	bInjectPrePhysicsHandle = true;
	bInjectPostPhysicsHandle = true;
	WeaponRootOrientationComponent = NAME_None;
	OrientationComponentRelativeFacing = FTransform::Identity;

	bAutoSetPrimaryAndSecondaryHands = true;
	PrimaryHandSelectionType = EVRMeleePrimaryHandType::VRPHAND_Rear;
	bHasValidPrimaryHand = false;

	bIsLodged = false;

	bCheckLodge = false;
	bIsHeld = false;
	bCanEverTick = false;
	bAlwaysTickPenetration = false;
	bUsePrimaryHandSettingsWithOneHand = false;
	COMType = EVRMeleeComType::VRPMELEECOM_BetweenHands;
	bOnlyPenetrateWithTwoHands = false;
}

void UGS_Melee::SetIsLodged(bool IsLodged, UPrimitiveComponent* LodgeComponent)
{
	bIsLodged = IsLodged;
	LodgedComponent = LodgeComponent;
}

void UGS_Melee::UpdateDualHandInfo()
{
	TArray<FBPGripPair> HoldingControllers;

	bool bIsHeldOther;
	IVRGripInterface::Execute_IsHeld(GetParent(), HoldingControllers, bIsHeldOther);

	float PHand = 0.0f;
	float SHand = 0.0f;
	bHasValidPrimaryHand = false;

	FBPActorGripInformation* FrontHandGrip = nullptr;
	FBPActorGripInformation* RearHandGrip = nullptr;

	SecondaryHand = FBPGripPair();
	PrimaryHand = FBPGripPair();

	int NumControllers = HoldingControllers.Num();

	for (FBPGripPair& Grip : HoldingControllers)
	{
		if (NumControllers > 1)
		{
			if (!Grip.IsValid())
				continue;

			FBPActorGripInformation* GripInfo = Grip.HoldingController->GetGripPtrByID(Grip.GripID);
			if (GripInfo)
			{
				float GripDistanceOnPrimaryAxis = 0.f;
				FTransform relTransform(GripInfo->RelativeTransform.ToInverseMatrixWithScale());
				relTransform = relTransform.GetRelativeTransform(OrientationComponentRelativeFacing);

				FVector localLoc = relTransform.GetTranslation();

				switch (PrimaryHandSelectionType)
				{
				case EVRMeleePrimaryHandType::VRPHAND_Slotted:
				{
					if (GripInfo->bIsSlotGrip)
					{
						PrimaryHand = Grip;
						bHasValidPrimaryHand = true;
					}
					else
					{
						if (!PrimaryHand.IsValid())
						{
							PrimaryHand = Grip;
						}

						SecondaryHand = Grip;
					}
				}break;
				case EVRMeleePrimaryHandType::VRPHAND_Front:
				case EVRMeleePrimaryHandType::VRPHAND_Rear:
				{

					if (((PrimaryHandSelectionType == EVRMeleePrimaryHandType::VRPHAND_Rear) ? localLoc.X < PHand : localLoc.X > PHand) || !PrimaryHand.HoldingController)
					{
						PrimaryHand = Grip;
						PHand = localLoc.X; 
						bHasValidPrimaryHand = true;
					}

					if ((((PrimaryHandSelectionType == EVRMeleePrimaryHandType::VRPHAND_Rear) ? localLoc.X > SHand : localLoc.X < SHand) || !SecondaryHand.HoldingController || SecondaryHand.HoldingController == PrimaryHand.HoldingController))
					{
						SecondaryHand = Grip;
						SHand = localLoc.X;
					}	

				}break;
				default:break;
				}
			}
		}
		else
		{
			PrimaryHand = Grip;
			SecondaryHand = FBPGripPair();
		}
	}

	if (PrimaryHand.IsValid() && (COMType == EVRMeleeComType::VRPMELEECOM_BetweenHands || COMType == EVRMeleeComType::VRPMELEECOM_PrimaryHand))
	{
		FBPActorGripInformation* GripInfo = PrimaryHand.HoldingController->GetGripPtrByID(PrimaryHand.GripID);

		if (SecondaryHand.IsValid())
		{
			FBPActorGripInformation* GripInfoS = SecondaryHand.HoldingController->GetGripPtrByID(SecondaryHand.GripID);

			if (GripInfo && GripInfoS)
			{
				FVector Primary = GripInfo->RelativeTransform.InverseTransformPositionNoScale(FVector::ZeroVector);
				FVector Secondary = GripInfoS->RelativeTransform.InverseTransformPositionNoScale(FVector::ZeroVector);

				FVector Final = (COMType == EVRMeleeComType::VRPMELEECOM_PrimaryHand) ? Primary : ((Primary + Secondary) / 2.f);
				ObjectRelativeGripCenter.SetLocation(Final);
			}
		}
		else
		{
			if (GripInfo)
			{

				if (GripInfo->SecondaryGripInfo.bHasSecondaryAttachment)
				{
					FVector gripLoc = GripInfo->RelativeTransform.InverseTransformPositionNoScale(FVector::ZeroVector);
					FVector secGripLoc = GripInfo->SecondaryGripInfo.SecondaryRelativeTransform.GetLocation();
					FVector finalloc = (COMType == EVRMeleeComType::VRPMELEECOM_PrimaryHand) ? gripLoc : (gripLoc + secGripLoc) / 2.f;
					FVector finalScaled = finalloc * GripInfo->RelativeTransform.GetScale3D();

					FTransform ownerTrans = GetOwner()->GetActorTransform();

					ObjectRelativeGripCenter.SetLocation(finalScaled);
					PrimaryHand.HoldingController->ReCreateGrip(*GripInfo);
				}
				else
				{
					ObjectRelativeGripCenter = FTransform::Identity;
				}
			}
		}
	}
}

void UGS_Melee::UpdateHandPositionAndRotation(FBPGripPair HandPair, FTransform HandWorldTransform, FVector& LocDifference, float& RotDifference, bool bUpdateLocation, bool bUpdateRotation)
{
	LocDifference = FVector::ZeroVector;

	if (HandPair.IsValid())
	{
		FBPActorGripInformation* GripInfo = HandPair.HoldingController->GetGripPtrByID(HandPair.GripID);

		if (GripInfo)
		{

			FTransform RelativeTrans = GripInfo->RelativeTransform.Inverse();
			FVector OriginalLoc = RelativeTrans.GetLocation();
			FQuat OriginalRot = RelativeTrans.GetRotation();

			FTransform ParentTransform = GetParentTransform();

			FQuat orientationRot = OrientationComponentRelativeFacing.GetRotation();

			if (bUpdateLocation)
			{
				FVector currentRelVec = orientationRot.RotateVector(ParentTransform.InverseTransformPosition(HandWorldTransform.GetLocation()));
				FVector currentLoc = orientationRot.RotateVector(RelativeTrans.GetLocation());
				currentLoc.X = currentRelVec.X;
				RelativeTrans.SetLocation(orientationRot.UnrotateVector(currentLoc));
			}

			if (bUpdateRotation)
			{
				FRotator currentRelRot = (orientationRot * (ParentTransform.GetRotation().Inverse() * HandWorldTransform.GetRotation())).Rotator();
				FRotator currentRot = (orientationRot * RelativeTrans.GetRotation()).Rotator();
				currentRot.Roll = currentRelRot.Roll;
				RelativeTrans.SetRotation(orientationRot.Inverse() * currentRot.Quaternion());
			}

			GripInfo->RelativeTransform = RelativeTrans.Inverse();
			HandPair.HoldingController->UpdatePhysicsHandle(*GripInfo, true);
			HandPair.HoldingController->NotifyGripTransformChanged(*GripInfo);

			LocDifference = RelativeTrans.GetLocation() - OriginalLoc;
			RotDifference = RelativeTrans.GetRotation().Rotator().Roll - OriginalRot.Rotator().Roll;

			FBPGripPair SecHand = SecondaryHand;
			UpdateDualHandInfo();

			if (SecondaryHand.IsValid() && !(SecHand == SecondaryHand))
			{

				GripInfo = SecondaryHand.HoldingController->GetGripPtrByID(SecondaryHand.GripID);
				GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = EPhysicsGripCOMType::COM_GripAtControllerLoc;

				FBPActorPhysicsHandleInformation* HandleInfo = SecondaryHand.HoldingController->GetPhysicsGrip(SecondaryHand.GripID);
				if (HandleInfo)
				{
					SecondaryHandPhysicsSettings.FillTo(HandleInfo);
					SecondaryHand.HoldingController->UpdatePhysicsHandle(SecondaryHand.GripID, true);
				}

				GripInfo = PrimaryHand.HoldingController->GetGripPtrByID(PrimaryHand.GripID);

				switch (COMType)
				{
				case EVRMeleeComType::VRPMELEECOM_Normal:
				case EVRMeleeComType::VRPMELEECOM_BetweenHands:
				{
					GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = EPhysicsGripCOMType::COM_GripAtControllerLoc;
				}break;

				case EVRMeleeComType::VRPMELEECOM_PrimaryHand:
				{
					GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = EPhysicsGripCOMType::COM_SetAndGripAt;
				}
				}

				HandleInfo = PrimaryHand.HoldingController->GetPhysicsGrip(PrimaryHand.GripID);
				if (HandleInfo)
				{
					if (bHasValidPrimaryHand)
					{
						PrimaryHandPhysicsSettings.FillTo(HandleInfo);
					}
					else
					{
						SecondaryHandPhysicsSettings.FillTo(HandleInfo);
					}
					PrimaryHand.HoldingController->UpdatePhysicsHandle(PrimaryHand.GripID, true);
				}
			}

			if (COMType != EVRMeleeComType::VRPMELEECOM_Normal)
				SetComBetweenHands(HandPair.HoldingController, HandPair.HoldingController->GetPhysicsGrip(HandPair.GripID));
		}
	}
}

void UGS_Melee::UpdateHandPosition(FBPGripPair HandPair, FVector HandWorldPosition, FVector& LocDifference)
{
	LocDifference = FVector::ZeroVector;

	if (HandPair.IsValid())
	{
		FBPActorGripInformation* GripInfo = HandPair.HoldingController->GetGripPtrByID(HandPair.GripID);

		if (GripInfo)
		{

			FTransform RelativeTrans = GripInfo->RelativeTransform.Inverse();
			FVector OriginalLoc = RelativeTrans.GetLocation();

			FTransform ParentTransform = GetParentTransform();

			FQuat orientationRot = OrientationComponentRelativeFacing.GetRotation();
			FVector currentRelVec = orientationRot.RotateVector(ParentTransform.InverseTransformPosition(HandWorldPosition));

			FVector currentLoc = orientationRot.RotateVector(RelativeTrans.GetLocation());
			currentLoc.X = currentRelVec.X;

			RelativeTrans.SetLocation(orientationRot.UnrotateVector(currentLoc));
			GripInfo->RelativeTransform = RelativeTrans.Inverse();
			HandPair.HoldingController->UpdatePhysicsHandle(*GripInfo, true);
			HandPair.HoldingController->NotifyGripTransformChanged(*GripInfo);

			LocDifference = RelativeTrans.GetLocation() - OriginalLoc;

			FBPGripPair SecHand = SecondaryHand;
			UpdateDualHandInfo();

			if (SecondaryHand.IsValid() && !(SecHand == SecondaryHand))
			{

				GripInfo = SecondaryHand.HoldingController->GetGripPtrByID(SecondaryHand.GripID);
				GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = EPhysicsGripCOMType::COM_GripAtControllerLoc;

				FBPActorPhysicsHandleInformation* HandleInfo = SecondaryHand.HoldingController->GetPhysicsGrip(SecondaryHand.GripID);
				if (HandleInfo)
				{
					SecondaryHandPhysicsSettings.FillTo(HandleInfo);
					SecondaryHand.HoldingController->UpdatePhysicsHandle(SecondaryHand.GripID, true);
				}

				GripInfo = PrimaryHand.HoldingController->GetGripPtrByID(PrimaryHand.GripID);

				switch (COMType)
				{
				case EVRMeleeComType::VRPMELEECOM_Normal:
				case EVRMeleeComType::VRPMELEECOM_BetweenHands:
				{
					GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = EPhysicsGripCOMType::COM_GripAtControllerLoc;
				}break;

				case EVRMeleeComType::VRPMELEECOM_PrimaryHand:
				{
					GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = EPhysicsGripCOMType::COM_SetAndGripAt;
				}
				}

				HandleInfo = PrimaryHand.HoldingController->GetPhysicsGrip(PrimaryHand.GripID);
				if (HandleInfo)
				{
					if (bHasValidPrimaryHand)
					{
						PrimaryHandPhysicsSettings.FillTo(HandleInfo);
					}
					else
					{
						SecondaryHandPhysicsSettings.FillTo(HandleInfo);
					}
					PrimaryHand.HoldingController->UpdatePhysicsHandle(PrimaryHand.GripID, true);
				}
			}

			if (COMType != EVRMeleeComType::VRPMELEECOM_Normal)
				SetComBetweenHands(HandPair.HoldingController, HandPair.HoldingController->GetPhysicsGrip(HandPair.GripID));
		}
	}
}

void UGS_Melee::SetPrimaryAndSecondaryHands(FBPGripPair& PrimaryGrip, FBPGripPair& SecondaryGrip)
{
	PrimaryHand = PrimaryGrip;
	SecondaryHand = SecondaryGrip;
}

void UGS_Melee::OnSecondaryGrip_Implementation(UGripMotionControllerComponent* Controller, USceneComponent* SecondaryGripComponent, const FBPActorGripInformation& GripInformation)
{
	if (!bIsActive)
		return;

	UpdateDualHandInfo();
}

void UGS_Melee::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation)
{
	if (!bIsActive)
		return;

	bIsHeld = true;

	{
		UpdateDualHandInfo();

		if (SecondaryHand.IsValid())
		{
			FBPActorGripInformation * GripInfo = SecondaryHand.HoldingController->GetGripPtrByID(SecondaryHand.GripID);
			GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = EPhysicsGripCOMType::COM_GripAtControllerLoc;

			FBPActorPhysicsHandleInformation* HandleInfo = SecondaryHand.HoldingController->GetPhysicsGrip(SecondaryHand.GripID);
			if (HandleInfo)
			{
				SecondaryHandPhysicsSettings.FillTo(HandleInfo);
				SecondaryHand.HoldingController->UpdatePhysicsHandle(SecondaryHand.GripID, true);
			}

			GripInfo = PrimaryHand.HoldingController->GetGripPtrByID(PrimaryHand.GripID);

			switch (COMType)
			{
			case EVRMeleeComType::VRPMELEECOM_Normal:
			case EVRMeleeComType::VRPMELEECOM_BetweenHands:
			{
				GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = EPhysicsGripCOMType::COM_GripAtControllerLoc;
			}break;

			case EVRMeleeComType::VRPMELEECOM_PrimaryHand:
			{
				GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = EPhysicsGripCOMType::COM_SetAndGripAt;
			}
			}

			HandleInfo = PrimaryHand.HoldingController->GetPhysicsGrip(PrimaryHand.GripID);
			if (HandleInfo)
			{
				if (bHasValidPrimaryHand)
				{
					PrimaryHandPhysicsSettings.FillTo(HandleInfo);
				}
				else
				{
					SecondaryHandPhysicsSettings.FillTo(HandleInfo);
				}
				PrimaryHand.HoldingController->UpdatePhysicsHandle(PrimaryHand.GripID, true);
			}
		}
	}
}

void UGS_Melee::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{

	if (!bIsActive)
		return;

	TArray<FBPGripPair> HoldingControllers;
	IVRGripInterface::Execute_IsHeld(GetParent(), HoldingControllers, bIsHeld);

	if (SecondaryHand.IsValid() && SecondaryHand.HoldingController == ReleasingController && SecondaryHand.GripID == GripInformation.GripID)
	{
		SecondaryHand = FBPGripPair();
	}
	else if (PrimaryHand.IsValid() && PrimaryHand.HoldingController == ReleasingController && PrimaryHand.GripID == GripInformation.GripID)
	{
		if (SecondaryHand.IsValid())
		{
			PrimaryHand = SecondaryHand;
			SecondaryHand = FBPGripPair();
		}
		else
		{
			PrimaryHand = FBPGripPair();
		}
	}

	if (COMType != EVRMeleeComType::VRPMELEECOM_Normal)
	{
		if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GetParentSceneComp()))
		{
			if (FBodyInstance* rBodyInstance = PrimComp->GetBodyInstance())
			{
				if (rBodyInstance->IsValidBodyInstance() && rBodyInstance->BodySetup.IsValid())
				{

					bool bWasBound = false;
					if (PrimaryHand.IsValid())
					{
						if (rBodyInstance->OnRecalculatedMassProperties().IsBoundToObject(PrimaryHand.HoldingController))
						{
							rBodyInstance->OnRecalculatedMassProperties().RemoveAll(this);
							bWasBound = true;
						}
					}

					rBodyInstance->UpdateMassProperties();

					if (bWasBound)
					{
						rBodyInstance->OnRecalculatedMassProperties().AddUObject(PrimaryHand.HoldingController, &UGripMotionControllerComponent::OnGripMassUpdated);
					}
				}
			}
		}
	}

	if (PrimaryHand.IsValid())
	{

		FBPActorPhysicsHandleInformation* HandleInfo = PrimaryHand.HoldingController->GetPhysicsGrip(PrimaryHand.GripID);
		if (HandleInfo)
		{
			FBPActorGripInformation * GripInfo = PrimaryHand.HoldingController->GetGripPtrByID(PrimaryHand.GripID);

			if (!SecondaryHand.IsValid())
			{
				HandleInfo->bSkipResettingCom = false;
				UpdateDualHandInfo();
			}

			if (GripInfo)
			{

				HandleInfo->LinConstraint.XDrive.bEnablePositionDrive = true;
				HandleInfo->LinConstraint.XDrive.bEnableVelocityDrive = true;
				HandleInfo->LinConstraint.XDrive.Stiffness = GripInfo->Stiffness;
				HandleInfo->LinConstraint.XDrive.Damping = GripInfo->Damping;

				HandleInfo->LinConstraint.YDrive = HandleInfo->LinConstraint.XDrive;
				HandleInfo->LinConstraint.ZDrive = HandleInfo->LinConstraint.XDrive;

				HandleInfo->AngConstraint.SwingDrive.bEnablePositionDrive = false;
				HandleInfo->AngConstraint.SwingDrive.bEnableVelocityDrive = false;
				HandleInfo->AngConstraint.TwistDrive.bEnablePositionDrive = false;
				HandleInfo->AngConstraint.TwistDrive.bEnableVelocityDrive = false;
				HandleInfo->AngConstraint.AngularDriveMode = EAngularDriveMode::SLERP;
				HandleInfo->AngConstraint.SlerpDrive.bEnablePositionDrive = true;
				HandleInfo->AngConstraint.SlerpDrive.bEnableVelocityDrive = true;

				if (GripInfo->AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && GripInfo->AdvancedGripSettings.PhysicsSettings.bUseCustomAngularValues)
				{
					HandleInfo->AngConstraint.SlerpDrive.Damping = GripInfo->AdvancedGripSettings.PhysicsSettings.AngularDamping;
					HandleInfo->AngConstraint.SlerpDrive.Stiffness = GripInfo->AdvancedGripSettings.PhysicsSettings.AngularStiffness;
				}
				else
				{
					HandleInfo->AngConstraint.SlerpDrive.Damping = GripInfo->Damping * 1.4f;
					HandleInfo->AngConstraint.SlerpDrive.Stiffness = GripInfo->Stiffness * 1.5f;
				}

				FBPAdvGripSettings AdvSettings = IVRGripInterface::Execute_AdvancedGripSettings(GripInfo->GrippedObject);
				GripInfo->AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings = AdvSettings.PhysicsSettings.PhysicsGripLocationSettings;

				PrimaryHand.HoldingController->UpdatePhysicsHandle(PrimaryHand.GripID, true);
			}
		}
	}

}

void UGS_Melee::OnBeginPlay_Implementation(UObject * CallingOwner)
{

	if (AActor * Owner = GetOwner())
	{
		FName CurrentCompName = NAME_None;
		bool bSearchRootComp = WeaponRootOrientationComponent.IsValid();
		int RemainingCount = PenetrationNotifierComponents.Num();
		for (UActorComponent* ChildComp : Owner->GetComponents())
		{
			CurrentCompName = ChildComp->GetFName();
			if (CurrentCompName == NAME_None)
				continue;

			if (bSearchRootComp && CurrentCompName == WeaponRootOrientationComponent)
			{
				bSearchRootComp = false;
				if (USceneComponent * SceneComp = Cast<USceneComponent>(ChildComp))
				{
					OrientationComponentRelativeFacing = SceneComp->GetRelativeTransform();
				}
			}

			if (FBPLodgeComponentInfo * Found = PenetrationNotifierComponents.FindByKey(CurrentCompName))
			{
				if (UPrimitiveComponent * PrimComp = Cast<UPrimitiveComponent>(ChildComp))
				{
					Found->TargetComponent = TObjectPtr<UPrimitiveComponent>(PrimComp);

				}

				RemainingCount--;
			}

			if (!bSearchRootComp && RemainingCount < 1)
			{
				break;
			}
		}

		if (RemainingCount < PenetrationNotifierComponents.Num())
		{
			Owner->OnActorHit.AddDynamic(this, &UGS_Melee::OnLodgeHitCallback);
			bCheckLodge = true;
		}
	}
}

void UGS_Melee::OnEndPlay_Implementation(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor * Owner = GetOwner())
	{
		Owner->OnActorHit.RemoveDynamic(this, &UGS_Melee::OnLodgeHitCallback);
	}
}

void UGS_Melee::OnLodgeHitCallback(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!Hit.GetComponent())
		return;

	if (!bCheckLodge || !bIsActive || bIsLodged || OtherActor == SelfActor)
	{
		if (bAlwaysTickPenetration || bIsHeld)
		{
			OnMeleeInvalidHit.Broadcast(OtherActor, Hit.GetComponent(), NormalImpulse, Hit);
		}
		return;
	}

	if (!bAlwaysTickPenetration && !bIsHeld)
		return;

	TArray<FBPHitSurfaceProperties> AllowedPenetrationSurfaceTypes;

	if (OverrideMeleeSurfaceSettings.Num() > 0)
	{

		AllowedPenetrationSurfaceTypes = OverrideMeleeSurfaceSettings;
	}
	else
	{

		UVRGlobalSettings::GetMeleeSurfaceGlobalSettings(AllowedPenetrationSurfaceTypes);
	}

	FBPHitSurfaceProperties HitSurfaceProperties;
	if (Hit.PhysMaterial.IsValid())
	{
		HitSurfaceProperties.SurfaceType = Hit.PhysMaterial->SurfaceType;
	}

	if (AllowedPenetrationSurfaceTypes.Num())
	{

		if (!Hit.PhysMaterial.IsValid())
		{
			OnMeleeInvalidHit.Broadcast(OtherActor, Hit.GetComponent(), NormalImpulse, Hit);
			return;
		}

		EPhysicalSurface PhysSurfaceType = Hit.PhysMaterial->SurfaceType;
		int32 IndexOfSurface = AllowedPenetrationSurfaceTypes.IndexOfByPredicate([&PhysSurfaceType](const FBPHitSurfaceProperties& Entry) { return Entry.SurfaceType == PhysSurfaceType; });

		if (IndexOfSurface != INDEX_NONE)
		{
			HitSurfaceProperties = AllowedPenetrationSurfaceTypes[IndexOfSurface];
		}
		else
		{

			HitSurfaceProperties.bSurfaceAllowsPenetration = false;

		}
	}

	bool bHadFirstHit = false;
	FBPLodgeComponentInfo FirstHitComp;

	float HitNormalImpulse = NormalImpulse.SizeSquared();

	for(FBPLodgeComponentInfo &LodgeData : PenetrationNotifierComponents)
	{
		if (!IsValid(LodgeData.TargetComponent))
			continue;

		FBox LodgeLocalBox = LodgeData.TargetComponent->CalcLocalBounds().GetBox();
		FVector LocalHit = LodgeData.TargetComponent->GetComponentTransform().InverseTransformPosition(Hit.ImpactPoint);

		if (IsValid(LodgeData.TargetComponent) && LodgeLocalBox.IsInsideOrOn(LocalHit))
		{
			FVector ForwardVec = LodgeData.TargetComponent->GetForwardVector();

			float DotValue = FMath::Abs(FVector::DotProduct(Hit.Normal, ForwardVec));
			float Velocity = NormalImpulse.ProjectOnToNormal(ForwardVec).SizeSquared();

			if (HitSurfaceProperties.bSurfaceAllowsPenetration && (!bOnlyPenetrateWithTwoHands || SecondaryHand.IsValid()))
			{
				if (LodgeData.ZoneType != EVRMeleeZoneType::VRPMELLE_ZONETYPE_Hit && DotValue >= (1.0f - LodgeData.AcceptableForwardProductRange) && (Velocity * HitSurfaceProperties.StabVelocityScaler) >= FMath::Square(LodgeData.PenetrationVelocity))
				{
					OnShouldLodgeInObject.Broadcast(LodgeData, OtherActor, Hit.GetComponent(), Hit.GetComponent()->GetCollisionObjectType(), HitSurfaceProperties, NormalImpulse, Hit);
					return;

				}
			}

			float HitImpulse = LodgeData.bIgnoreForwardVectorForHitImpulse ? HitNormalImpulse : Velocity;

			if (!bHadFirstHit && LodgeData.ZoneType > EVRMeleeZoneType::VRPMELLE_ZONETYPE_Stab && DotValue >= (1.0f - LodgeData.AcceptableForwardProductRangeForHits) && HitImpulse >= FMath::Square(LodgeData.MinimumHitVelocity))
			{
				bHadFirstHit = true;
				FirstHitComp = LodgeData;
			}
		}
	}

	if (bHadFirstHit)
	{
		OnMeleeHit.Broadcast(FirstHitComp, OtherActor, Hit.GetComponent(), Hit.GetComponent()->GetCollisionObjectType(), HitSurfaceProperties, NormalImpulse, Hit);
	}
	else
	{
		OnMeleeInvalidHit.Broadcast(OtherActor, Hit.GetComponent(), NormalImpulse, Hit);
	}
}

void UGS_Melee::HandlePrePhysicsHandle(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation &GripInfo, FBPActorPhysicsHandleInformation * HandleInfo, FTransform & KinPose)
{
	if (!bIsActive)
		return;

	if (WeaponRootOrientationComponent != NAME_None)
	{

		FQuat DeltaQuat = OrientationComponentRelativeFacing.GetRotation();

		KinPose.SetRotation(KinPose.GetRotation() * (HandleInfo->RootBoneRotation.GetRotation().Inverse() * DeltaQuat));
		HandleInfo->COMPosition.SetRotation(HandleInfo->COMPosition.GetRotation() * (HandleInfo->RootBoneRotation.GetRotation().Inverse() * DeltaQuat));
	}
}

void UGS_Melee::HandlePostPhysicsHandle(UGripMotionControllerComponent* GrippingController, FBPActorPhysicsHandleInformation * HandleInfo)
{
	if (!bIsActive)
		return;

	if (SecondaryHand.IsValid() )
	{
		if (GrippingController == SecondaryHand.HoldingController && HandleInfo->GripID == SecondaryHand.GripID)
		{
			SecondaryHandPhysicsSettings.FillTo(HandleInfo);
		}
		else if (GrippingController == PrimaryHand.HoldingController && HandleInfo->GripID == PrimaryHand.GripID)
		{
			if (bHasValidPrimaryHand)
			{
				PrimaryHandPhysicsSettings.FillTo(HandleInfo);
			}
			else
			{
				SecondaryHandPhysicsSettings.FillTo(HandleInfo);
			}
		}

		if (COMType != EVRMeleeComType::VRPMELEECOM_Normal)
			SetComBetweenHands(GrippingController, HandleInfo);
	}
	else
	{
		if (bUsePrimaryHandSettingsWithOneHand)
		{
			PrimaryHandPhysicsSettings.FillTo(HandleInfo);
		}

		if (COMType != EVRMeleeComType::VRPMELEECOM_Normal)
			SetComBetweenHands(GrippingController, HandleInfo);
	}
}

void UGS_Melee::SetComBetweenHands(UGripMotionControllerComponent* GrippingController, FBPActorPhysicsHandleInformation * HandleInfo)
{

	if (!GrippingController || !HandleInfo)
		return;

	if (COMType != EVRMeleeComType::VRPMELEECOM_Normal && SecondaryHand.IsValid())
	{

		{
			if (UPrimitiveComponent * PrimComp = Cast<UPrimitiveComponent>(GetParentSceneComp()))
			{
				if (FBodyInstance * rBodyInstance = PrimComp->GetBodyInstance())
				{
					FPhysicsCommand::ExecuteWrite(rBodyInstance->ActorHandle, [&](const FPhysicsActorHandle& Actor)
					{
						FTransform localCom = FPhysicsInterface::GetComTransformLocal_AssumesLocked(Actor);
						localCom.SetLocation((HandleInfo->RootBoneRotation * ObjectRelativeGripCenter).GetLocation());
						FPhysicsInterface::SetComLocalPose_AssumesLocked(Actor, localCom);

					});
				}
			}
		}

		HandleInfo->bSetCOM = true; 
		HandleInfo->bSkipResettingCom = true;
	}
	else
	{
		HandleInfo->bSkipResettingCom = false;
	}
}

bool UGS_Melee::Wants_DenyTeleport_Implementation(UGripMotionControllerComponent* Controller)
{

	return false;
}

bool UGS_Melee::GetWorldTransform_Implementation
(
	UGripMotionControllerComponent* GrippingController, 
	float DeltaTime, FTransform & WorldTransform, 
	const FTransform &ParentTransform, 
	FBPActorGripInformation &Grip, 
	AActor * actor, 
	UPrimitiveComponent * root, 
	bool bRootHasInterface, 
	bool bActorHasInterface, 
	bool bIsForTeleport
) 
{
	if (!GrippingController)
		return false;

	WorldTransform = Grip.RelativeTransform * Grip.AdditionTransform * ParentTransform;
	if (Grip.SecondaryGripInfo.bHasSecondaryAttachment)
	{
		WorldTransform.SetLocation((GrippingController->GetPivotLocation() + Grip.SecondaryGripInfo.SecondaryAttachment->GetComponentLocation()) / 2.f);
	}

	if ((Grip.SecondaryGripInfo.bHasSecondaryAttachment && Grip.SecondaryGripInfo.SecondaryAttachment) || Grip.SecondaryGripInfo.GripLerpState == EGripLerpState::EndLerp)
	{
		switch (Grip.SecondaryGripInfo.GripLerpState)
		{
		case EGripLerpState::StartLerp:
		case EGripLerpState::EndLerp:
		{
			if (Grip.SecondaryGripInfo.curLerp > 0.01f)
				Grip.SecondaryGripInfo.curLerp -= DeltaTime;
			else
			{

				Grip.SecondaryGripInfo.GripLerpState = EGripLerpState::NotLerping;
			}

		}break;

		case EGripLerpState::NotLerping:
		default:break;
		}
	}

	if ((Grip.SecondaryGripInfo.bHasSecondaryAttachment && Grip.SecondaryGripInfo.SecondaryAttachment) || Grip.SecondaryGripInfo.GripLerpState == EGripLerpState::EndLerp)
	{
		FTransform SecondaryTransform = Grip.RelativeTransform * ParentTransform;

		ESecondaryGripType SecondaryType = ESecondaryGripType::SG_None;

		if (bRootHasInterface)
			SecondaryType = IVRGripInterface::Execute_SecondaryGripType(root);
		else if (bActorHasInterface)
			SecondaryType = IVRGripInterface::Execute_SecondaryGripType(actor);

		if (SecondaryType != ESecondaryGripType::SG_Custom)
		{

			FVector BasePoint = ParentTransform.GetLocation(); 
			const FTransform PivotToWorld = FTransform(FQuat::Identity, BasePoint);
			const FTransform WorldToPivot = FTransform(FQuat::Identity, -BasePoint);

			FVector frontLocOrig;
			FVector frontLoc;

			if (Grip.SecondaryGripInfo.GripLerpState == EGripLerpState::EndLerp)
			{
				frontLocOrig = (SecondaryTransform.TransformPosition(Grip.SecondaryGripInfo.SecondaryRelativeTransform.GetLocation())) - BasePoint;
				frontLoc = Grip.SecondaryGripInfo.LastRelativeLocation;

				frontLocOrig = FMath::Lerp(frontLoc, frontLocOrig, FMath::Clamp(Grip.SecondaryGripInfo.curLerp / Grip.SecondaryGripInfo.LerpToRate, 0.0f, 1.0f));
			}
			else 
			{

				CalculateSecondaryLocation(frontLoc, BasePoint, Grip, GrippingController);

				frontLocOrig = (SecondaryTransform.TransformPosition(Grip.SecondaryGripInfo.SecondaryRelativeTransform.GetLocation())) - BasePoint;

				ApplySmoothingAndLerp(Grip, frontLoc, frontLocOrig, DeltaTime);

				Grip.SecondaryGripInfo.LastRelativeLocation = frontLoc;
			}

			FVector Scaler = FVector(1.0f);
			if (SecondaryType == ESecondaryGripType::SG_FreeWithScaling_Retain || SecondaryType == ESecondaryGripType::SG_SlotOnlyWithScaling_Retain || SecondaryType == ESecondaryGripType::SG_ScalingOnly)
			{
				GetAnyScaling(Scaler, Grip, frontLoc, frontLocOrig, SecondaryType, SecondaryTransform);
			}

			Grip.SecondaryGripInfo.SecondaryGripDistance = FVector::Dist(frontLocOrig, frontLoc);

			if (SecondaryType != ESecondaryGripType::SG_ScalingOnly)
			{

				FQuat rotVal = FQuat::FindBetweenVectors(frontLocOrig, frontLoc);

				WorldTransform = WorldTransform * WorldToPivot * FTransform(rotVal, FVector::ZeroVector, Scaler) * PivotToWorld;
			}
			else
			{

				WorldTransform = WorldTransform * WorldToPivot * FTransform(FQuat::Identity, FVector::ZeroVector, Scaler) * PivotToWorld;
			}
		}

		FTransform InverseTrans(Grip.RelativeTransform.ToInverseMatrixWithScale());

		FVector origLocation = InverseTrans.GetLocation();

		FVector orientedvector = FVector::VectorPlaneProject(origLocation, -OrientationComponentRelativeFacing.GetRotation().GetForwardVector());
		FVector newLocation = FVector::VectorPlaneProject(WorldTransform.InverseTransformPosition(GrippingController->GetPivotLocation()), OrientationComponentRelativeFacing.GetRotation().GetForwardVector());

		FQuat DeltaQuat = FQuat::FindBetweenVectors(orientedvector, newLocation);

		WorldTransform.SetRotation(DeltaQuat * WorldTransform.GetRotation());
	}

	return true;
}
