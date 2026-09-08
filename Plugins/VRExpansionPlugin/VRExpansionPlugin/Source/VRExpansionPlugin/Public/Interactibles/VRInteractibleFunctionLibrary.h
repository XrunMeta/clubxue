

#pragma once
#include "CoreMinimal.h"
#include "VRBPDatatypes.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"

#include "VRInteractibleFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(VRInteractibleFunctionLibraryLog, Log, All);

UENUM(Blueprintable)
enum class EVRInteractibleAxis : uint8
{
	Axis_X,
	Axis_Y,
	Axis_Z
};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPVRInteractibleBaseData
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "InteractibleData")
	FTransform InitialRelativeTransform;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "InteractibleData")
	FVector InitialInteractorLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "InteractibleData")
	FVector InitialGripLoc;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "InteractibleData")
	FVector InitialDropLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "InteractibleData")
	FTransform ReversedRelativeTransform;

	FBPVRInteractibleBaseData()
	{
		InitialInteractorLocation = FVector::ZeroVector;
		InitialGripLoc = FVector::ZeroVector;
		InitialDropLocation = FVector::ZeroVector;
	}
};

UCLASS()
class VREXPANSIONPLUGIN_API UVRInteractibleFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static float GetAtan2Angle(EVRInteractibleAxis AxisToCalc, FVector CurInteractorLocation, float OptionalInitialRotation = 0.0f)
	{
		switch (AxisToCalc)
		{
		case EVRInteractibleAxis::Axis_X:
		{
			return FMath::RadiansToDegrees(FMath::Atan2(CurInteractorLocation.Y, CurInteractorLocation.Z)) - OptionalInitialRotation;
		}break;
		case EVRInteractibleAxis::Axis_Y:
		{
			return FMath::RadiansToDegrees(FMath::Atan2(CurInteractorLocation.Z, CurInteractorLocation.X)) - OptionalInitialRotation;
		}break;
		case EVRInteractibleAxis::Axis_Z:
		{
			return FMath::RadiansToDegrees(FMath::Atan2(CurInteractorLocation.Y, CurInteractorLocation.X)) - OptionalInitialRotation;
		}break;
		default:
		{}break;
		}

		return 0.0f;
	}

	static float GetDeltaAngleFromTransforms(EVRInteractibleAxis RotAxis, FTransform & InitialRelativeTransform, FTransform &CurrentRelativeTransform)
	{
		return GetDeltaAngle(RotAxis, (CurrentRelativeTransform.GetRelativeTransform(InitialRelativeTransform).GetRotation()).GetNormalized());
	}

	static float GetDeltaAngle(EVRInteractibleAxis RotAxis, FQuat DeltaQuat)
	{
		FVector Axis;
		float Angle;
		DeltaQuat.ToAxisAndAngle(Axis, Angle);

		if (RotAxis == EVRInteractibleAxis::Axis_Z)
			return FRotator::NormalizeAxis(FMath::RadiansToDegrees(Angle)) * (FMath::Sign(GetAxisValue(RotAxis, Axis)));
		else
			return FRotator::NormalizeAxis(FMath::RadiansToDegrees(Angle)) * (-FMath::Sign(GetAxisValue(RotAxis, Axis)));
	}

	static float GetAxisValue(EVRInteractibleAxis RotAxis, FRotator CheckRotation)
	{
		switch (RotAxis)
		{
		case EVRInteractibleAxis::Axis_X:
			return CheckRotation.Roll; break;
		case EVRInteractibleAxis::Axis_Y:
			return CheckRotation.Pitch; break;
		case EVRInteractibleAxis::Axis_Z:
			return CheckRotation.Yaw; break;
		default:return 0.0f; break;
		}
	}

	static float GetAxisValue(EVRInteractibleAxis RotAxis, FVector CheckAxis)
	{
		switch (RotAxis)
		{
		case EVRInteractibleAxis::Axis_X:
			return CheckAxis.X; break;
		case EVRInteractibleAxis::Axis_Y:
			return CheckAxis.Y; break;
		case EVRInteractibleAxis::Axis_Z:
			return CheckAxis.Z; break;
		default:return 0.0f; break;
		}
	}

	static FVector SetAxisValueVec(EVRInteractibleAxis RotAxis, float SetValue)
	{
		FVector vec = FVector::ZeroVector;

		switch (RotAxis)
		{
		case EVRInteractibleAxis::Axis_X:
			vec.X = SetValue; break;
		case EVRInteractibleAxis::Axis_Y:
			vec.Y = SetValue; break;
		case EVRInteractibleAxis::Axis_Z:
			vec.Z = SetValue; break;
		default:break;
		}

		return vec;
	}

	static FRotator SetAxisValueRot(EVRInteractibleAxis RotAxis, float SetValue)
	{
		FRotator vec = FRotator::ZeroRotator;

		switch (RotAxis)
		{
		case EVRInteractibleAxis::Axis_X:
			vec.Roll = SetValue; break;
		case EVRInteractibleAxis::Axis_Y:
			vec.Pitch = SetValue; break;
		case EVRInteractibleAxis::Axis_Z:
			vec.Yaw = SetValue; break;
		default:break;
		}

		return vec;
	}

	static FRotator SetAxisValueRot(EVRInteractibleAxis RotAxis, float SetValue, FRotator Var)
	{
		FRotator vec = Var;
		switch (RotAxis)
		{
		case EVRInteractibleAxis::Axis_X:
			vec.Roll = SetValue; break;
		case EVRInteractibleAxis::Axis_Y:
			vec.Pitch = SetValue; break;
		case EVRInteractibleAxis::Axis_Z:
			vec.Yaw = SetValue; break;
		default:break;
		}

		return vec;
	}

	UFUNCTION(BlueprintPure, Category = "VRInteractibleFunctions", meta = (bIgnoreSelf = "true"))
	static FTransform Interactible_GetCurrentParentTransform(USceneComponent * SceneComponentToCheck)
	{
		if (SceneComponentToCheck)
		{
			if (USceneComponent * AttachParent = SceneComponentToCheck->GetAttachParent())
			{
				return AttachParent->GetComponentTransform();
			}
		}

		return FTransform::Identity;
	}

	UFUNCTION(BlueprintPure, Category = "VRInteractibleFunctions", meta = (bIgnoreSelf = "true"))
	static FTransform Interactible_GetCurrentRelativeTransform(USceneComponent * SceneComponentToCheck, UPARAM(ref)FBPVRInteractibleBaseData & BaseData)
	{
		FTransform ParentTransform = FTransform::Identity;
		if (SceneComponentToCheck)
		{
			if (USceneComponent * AttachParent = SceneComponentToCheck->GetAttachParent())
			{

				ParentTransform = AttachParent->GetComponentTransform();
			}
		}

		return BaseData.InitialRelativeTransform * ParentTransform;
	}

	UFUNCTION(BlueprintCallable, Category = "VRInteractibleFunctions", meta = (bIgnoreSelf = "true"))
		static void Interactible_BeginPlayInit(USceneComponent * InteractibleComp, UPARAM(ref) FBPVRInteractibleBaseData & BaseDataToInit)
	{
		if (!InteractibleComp)
			return;

		BaseDataToInit.InitialRelativeTransform = InteractibleComp->GetRelativeTransform();
	}

	UFUNCTION(BlueprintCallable, Category = "VRInteractibleFunctions", meta = (bIgnoreSelf = "true"))
		static void Interactible_OnGripInit(USceneComponent * InteractibleComp, UPARAM(ref) FBPActorGripInformation& GripInformation, UPARAM(ref) FBPVRInteractibleBaseData & BaseDataToInit)
	{
		if (!InteractibleComp)
			return;

		BaseDataToInit.ReversedRelativeTransform = FTransform(GripInformation.RelativeTransform.ToInverseMatrixWithScale());
		BaseDataToInit.InitialDropLocation = BaseDataToInit.ReversedRelativeTransform.GetTranslation(); 

		FTransform RelativeToGripTransform = BaseDataToInit.ReversedRelativeTransform * InteractibleComp->GetComponentTransform();
		FTransform CurrentRelativeTransform = BaseDataToInit.InitialRelativeTransform * UVRInteractibleFunctionLibrary::Interactible_GetCurrentParentTransform(InteractibleComp);
		BaseDataToInit.InitialInteractorLocation = CurrentRelativeTransform.InverseTransformPosition(RelativeToGripTransform.GetTranslation());

		BaseDataToInit.InitialGripLoc = BaseDataToInit.InitialRelativeTransform.InverseTransformPosition(InteractibleComp->GetRelativeLocation());
	}

	UFUNCTION(BlueprintPure, Category = "VRInteractibleFunctions", meta = (bIgnoreSelf = "true"))
	static float Interactible_GetAngleAroundAxis(EVRInteractibleAxis AxisToCalc, FVector CurInteractorLocation)
	{
		float ReturnAxis = 0.0f;

		switch (AxisToCalc)
		{
		case EVRInteractibleAxis::Axis_X:
		{
			ReturnAxis = FMath::RadiansToDegrees(FMath::Atan2(CurInteractorLocation.Y, CurInteractorLocation.Z));
		}break;
		case EVRInteractibleAxis::Axis_Y:
		{
			ReturnAxis = FMath::RadiansToDegrees(FMath::Atan2(CurInteractorLocation.Z, CurInteractorLocation.X));
		}break;
		case EVRInteractibleAxis::Axis_Z:
		{
			ReturnAxis = FMath::RadiansToDegrees(FMath::Atan2(CurInteractorLocation.Y, CurInteractorLocation.X));
		}break;
		default:
		{}break;
		}

		return ReturnAxis;
	}

	UFUNCTION(BlueprintPure, Category = "VRInteractibleFunctions", meta = (bIgnoreSelf = "true"))	
	static float Interactible_GetAngleAroundAxisDelta(EVRInteractibleAxis AxisToCalc, FVector CurInteractorLocation, float InitialAngle)
	{
		return FRotator::NormalizeAxis(Interactible_GetAngleAroundAxis(AxisToCalc, CurInteractorLocation) - InitialAngle);
	}

	UFUNCTION(BlueprintPure, Category = "VRInteractibleFunctions", meta = (bIgnoreSelf = "true"))
		static float Interactible_GetThresholdSnappedValue(float ValueToSnap, float SnapIncrement, float SnapThreshold)
	{
		if (SnapIncrement > 0.f && FMath::Fmod(ValueToSnap, SnapIncrement) <= FMath::Min(SnapIncrement, SnapThreshold))
		{
			return FMath::GridSnap(ValueToSnap, SnapIncrement);
		}

		return ValueToSnap;
	}

};	

