

#include "GripScripts/GS_Default.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GS_Default)

#include "VRGripInterface.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/WorldSettings.h"
#include "GripMotionControllerComponent.h"

UGS_Default::UGS_Default(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	bIsActive = true;
	WorldTransformOverrideType = EGSTransformOverrideType::OverridesWorldTransform;
}

void UGS_Default::GetAnyScaling(FVector& Scaler, FBPActorGripInformation& Grip, FVector& frontLoc, FVector& frontLocOrig, ESecondaryGripType SecondaryType, FTransform& SecondaryTransform)
{
	if (Grip.SecondaryGripInfo.GripLerpState != EGripLerpState::EndLerp)
	{

		if (SecondaryType == ESecondaryGripType::SG_FreeWithScaling_Retain || SecondaryType == ESecondaryGripType::SG_SlotOnlyWithScaling_Retain || SecondaryType == ESecondaryGripType::SG_ScalingOnly)
		{
			 Scaler = FVector(frontLoc.Size() / frontLocOrig.Size());

		}
	}
}

void UGS_Default::ApplySmoothingAndLerp(FBPActorGripInformation& Grip, FVector& frontLoc, FVector& frontLocOrig, float DeltaTime)
{
	if (Grip.SecondaryGripInfo.GripLerpState == EGripLerpState::StartLerp) 
	{

		frontLocOrig = FMath::Lerp(frontLocOrig, frontLoc, FMath::Clamp(Grip.SecondaryGripInfo.curLerp / Grip.SecondaryGripInfo.LerpToRate, 0.0f, 1.0f));
	}

}

bool UGS_Default::GetWorldTransform_Implementation
(
	UGripMotionControllerComponent* GrippingController,
	float DeltaTime, FTransform& WorldTransform,
	const FTransform& ParentTransform,
	FBPActorGripInformation& Grip,
	AActor* actor,
	UPrimitiveComponent* root,
	bool bRootHasInterface,
	bool bActorHasInterface,
	bool bIsForTeleport
)
{
	if (!GrippingController)
		return false;

	WorldTransform = Grip.RelativeTransform * Grip.AdditionTransform * ParentTransform;

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
	}
	return true;
}

void UGS_Default::CalculateSecondaryLocation(FVector& frontLoc, const FVector& BasePoint, FBPActorGripInformation& Grip, UGripMotionControllerComponent* GrippingController)
{
	bool bPulledControllerLoc = false;
	if (UGripMotionControllerComponent* OtherController = Cast<UGripMotionControllerComponent>(Grip.SecondaryGripInfo.SecondaryAttachment))
	{
		bool bPulledCurrentTransform = false;

		if (IsValid(OtherController->CustomPivotComponent))
		{
			FTransform SecondaryTrans = FTransform::Identity;
			SecondaryTrans = OtherController->GetPivotTransform();
			bPulledControllerLoc = true;
			frontLoc = SecondaryTrans.GetLocation() - BasePoint;
		}
	}

	if (!bPulledControllerLoc)
	{
		frontLoc = Grip.SecondaryGripInfo.SecondaryAttachment->GetComponentLocation() - BasePoint;
	}
}

void UGS_ExtendedDefault::GetAnyScaling(FVector& Scaler, FBPActorGripInformation& Grip, FVector& frontLoc, FVector& frontLocOrig, ESecondaryGripType SecondaryType, FTransform& SecondaryTransform)
{
	if (Grip.SecondaryGripInfo.GripLerpState != EGripLerpState::EndLerp)
	{

		if (SecondaryType == ESecondaryGripType::SG_FreeWithScaling_Retain || SecondaryType == ESecondaryGripType::SG_SlotOnlyWithScaling_Retain || SecondaryType == ESecondaryGripType::SG_ScalingOnly)
		{
			 Scaler = FVector(frontLoc.Size() / frontLocOrig.Size());

			if (bLimitGripScaling)
			{

				FVector WorldScale = SecondaryTransform.GetScale3D();
				FVector CombinedScale = WorldScale * Scaler;

				CombinedScale.X = FMath::Clamp(CombinedScale.X, MinimumGripScaling.X, MaximumGripScaling.X);
				CombinedScale.Y = FMath::Clamp(CombinedScale.Y, MinimumGripScaling.Y, MaximumGripScaling.Y);
				CombinedScale.Z = FMath::Clamp(CombinedScale.Z, MinimumGripScaling.Z, MaximumGripScaling.Z);

				Scaler = CombinedScale / WorldScale;
			}

		}
	}
}
