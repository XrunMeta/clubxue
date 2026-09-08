

#include "VRTrackedParentInterface.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRTrackedParentInterface)

#include "UObject/Interface.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "VRBPDatatypes.h"

UVRTrackedParentInterface::UVRTrackedParentInterface(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void IVRTrackedParentInterface::Default_SetTrackedParent_Impl(UPrimitiveComponent * NewParentComponent, float WaistRadius, EBPVRWaistTrackingMode WaistTrackingMode, FBPVRWaistTracking_Info & OptionalWaistTrackingParent, USceneComponent * Self)
{

	if (OptionalWaistTrackingParent.IsValid())
	{

		Self->RemoveTickPrerequisiteComponent(OptionalWaistTrackingParent.TrackedDevice);
	}
	if (!NewParentComponent || !Self)
	{
		OptionalWaistTrackingParent.Clear();
		return;
	}

	if (NewParentComponent->PrimaryComponentTick.TickGroup == Self->PrimaryComponentTick.TickGroup)
	{

		NewParentComponent->RemoveTickPrerequisiteComponent(Self);

		Self->AddTickPrerequisiteComponent(NewParentComponent);
	}
	OptionalWaistTrackingParent.TrackedDevice = NewParentComponent;
	OptionalWaistTrackingParent.RestingRotation = NewParentComponent->GetRelativeRotation();
	OptionalWaistTrackingParent.RestingRotation.Yaw = 0.0f;

	OptionalWaistTrackingParent.TrackingMode = WaistTrackingMode;
	OptionalWaistTrackingParent.WaistRadius = WaistRadius;
}
FTransform IVRTrackedParentInterface::Default_GetWaistOrientationAndPosition(FBPVRWaistTracking_Info & WaistTrackingInfo)
{
	if (!WaistTrackingInfo.IsValid())
		return FTransform::Identity;
	FTransform DeviceTransform = WaistTrackingInfo.TrackedDevice->GetRelativeTransform();

	DeviceTransform.ConcatenateRotation(WaistTrackingInfo.RestingRotation.Quaternion().Inverse());
	DeviceTransform.SetScale3D(FVector(1, 1, 1));

	if (WaistTrackingInfo.WaistRadius > 0.0f)
	{
		DeviceTransform.AddToTranslation(DeviceTransform.GetRotation().RotateVector(FVector(-WaistTrackingInfo.WaistRadius, 0, 0)));
	}

	switch (WaistTrackingInfo.TrackingMode)
	{
	case EBPVRWaistTrackingMode::VRWaist_Tracked_Front: DeviceTransform.ConcatenateRotation(FRotator(0, 0, 0).Quaternion()); break;
	case EBPVRWaistTrackingMode::VRWaist_Tracked_Rear: DeviceTransform.ConcatenateRotation(FRotator(0, -180, 0).Quaternion()); break;
	case EBPVRWaistTrackingMode::VRWaist_Tracked_Left: DeviceTransform.ConcatenateRotation(FRotator(0, 90, 0).Quaternion()); break;
	case EBPVRWaistTrackingMode::VRWaist_Tracked_Right:	DeviceTransform.ConcatenateRotation(FRotator(0, -90, 0).Quaternion()); break;
	}

	return DeviceTransform;
}