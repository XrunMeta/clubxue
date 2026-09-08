

#include "GripScripts/GS_InteractibleSettings.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GS_InteractibleSettings)

#include "Components/PrimitiveComponent.h"
#include "GripMotionControllerComponent.h"
#include "GameFramework/Actor.h"

UGS_InteractibleSettings::UGS_InteractibleSettings(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	bIsActive = true;
	WorldTransformOverrideType = EGSTransformOverrideType::OverridesWorldTransform;
}

void UGS_InteractibleSettings::OnBeginPlay_Implementation(UObject * CallingOwner)
{
	if (InteractionSettings.bGetInitialPositionsOnBeginPlay)
	{
		FTransform parentTrans = GetParentTransform(!InteractionSettings.bLimitsInLocalSpace);

		InteractionSettings.InitialAngularTranslation = parentTrans.Rotator();
		InteractionSettings.InitialLinearTranslation = parentTrans.GetTranslation();
	}
}
void UGS_InteractibleSettings::OnGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) 
{
	if (InteractionSettings.bIgnoreHandRotation && !InteractionSettings.bHasValidBaseTransform)
	{
		RemoveRelativeRotation(GrippingController, GripInformation);
	}

}

void UGS_InteractibleSettings::OnGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed) 
{
	InteractionSettings.bHasValidBaseTransform = false;
}

void UGS_InteractibleSettings::RemoveRelativeRotation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation)
{
	InteractionSettings.BaseTransform = GripInformation.RelativeTransform;

	FTransform compTrans = this->GetParentTransform(true, GripInformation.GrippedBoneName);

	InteractionSettings.BaseTransform = FTransform(InteractionSettings.BaseTransform.ToInverseMatrixWithScale()) * compTrans; 
	InteractionSettings.BaseTransform.SetScale3D(GrippingController->GetPivotTransform().GetScale3D());
	InteractionSettings.BaseTransform.SetRotation(FQuat::Identity); 

	InteractionSettings.BaseTransform = compTrans.GetRelativeTransform(InteractionSettings.BaseTransform); 
	InteractionSettings.bHasValidBaseTransform = true;
}

bool UGS_InteractibleSettings::GetWorldTransform_Implementation
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
	if (!root)
		return false;

	FTransform LocalTransform;

	if (InteractionSettings.bIgnoreHandRotation)
	{
		if (!InteractionSettings.bHasValidBaseTransform)
		{

			RemoveRelativeRotation(GrippingController, Grip);
		}

		FTransform RotationalessTransform = ParentTransform;
		RotationalessTransform.SetRotation(FQuat::Identity);

		WorldTransform = InteractionSettings.BaseTransform * Grip.AdditionTransform * RotationalessTransform;
	}
	else
		WorldTransform = Grip.RelativeTransform * Grip.AdditionTransform * ParentTransform;

	if (InteractionSettings.bLimitsInLocalSpace)
	{
		if (USceneComponent * parent = root->GetAttachParent())
			LocalTransform = parent->GetComponentTransform();
		else
			LocalTransform = FTransform::Identity;

		WorldTransform = WorldTransform.GetRelativeTransform(LocalTransform);
	}

	FVector componentLoc = WorldTransform.GetLocation();

	if (InteractionSettings.bLimitX)
		componentLoc.X = FMath::Clamp(componentLoc.X, InteractionSettings.InitialLinearTranslation.X + InteractionSettings.MinLinearTranslation.X, InteractionSettings.InitialLinearTranslation.X + InteractionSettings.MaxLinearTranslation.X);

	if (InteractionSettings.bLimitY)
		componentLoc.Y = FMath::Clamp(componentLoc.Y, InteractionSettings.InitialLinearTranslation.Y + InteractionSettings.MinLinearTranslation.Y, InteractionSettings.InitialLinearTranslation.Y + InteractionSettings.MaxLinearTranslation.Y);

	if (InteractionSettings.bLimitZ)
		componentLoc.Z = FMath::Clamp(componentLoc.Z, InteractionSettings.InitialLinearTranslation.Z + InteractionSettings.MinLinearTranslation.Z, InteractionSettings.InitialLinearTranslation.Z + InteractionSettings.MaxLinearTranslation.Z);

	WorldTransform.SetLocation(componentLoc);

	FRotator componentRot = WorldTransform.GetRotation().Rotator();

	if (InteractionSettings.bLimitPitch)
		componentRot.Pitch = FMath::Clamp(componentRot.Pitch, InteractionSettings.InitialAngularTranslation.Pitch + InteractionSettings.MinAngularTranslation.Pitch, InteractionSettings.InitialAngularTranslation.Pitch + InteractionSettings.MaxAngularTranslation.Pitch);

	if (InteractionSettings.bLimitYaw)
		componentRot.Yaw = FMath::Clamp(componentRot.Yaw, InteractionSettings.InitialAngularTranslation.Yaw + InteractionSettings.MinAngularTranslation.Yaw, InteractionSettings.InitialAngularTranslation.Yaw + InteractionSettings.MaxAngularTranslation.Yaw);

	if (InteractionSettings.bLimitRoll)
		componentRot.Roll = FMath::Clamp(componentRot.Roll, InteractionSettings.InitialAngularTranslation.Roll + InteractionSettings.MinAngularTranslation.Roll, InteractionSettings.InitialAngularTranslation.Roll + InteractionSettings.MaxAngularTranslation.Roll);

	WorldTransform.SetRotation(componentRot.Quaternion());

	if (InteractionSettings.bLimitsInLocalSpace)
	{
		WorldTransform = WorldTransform * LocalTransform;
	}

	return true;
}