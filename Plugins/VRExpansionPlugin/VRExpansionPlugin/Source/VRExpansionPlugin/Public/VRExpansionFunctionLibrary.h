

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VRBPDatatypes.h"
#include "VRExpansionFunctionLibrary.generated.h"

class USplineComponent;
class UPrimitiveComponent;
class UGripMotioncontroller;
struct FGameplayTag;
struct FGameplayTagContainer;

enum class EControllerHand : uint8;
enum class EBPHMDDeviceType : uint8;

DECLARE_LOG_CATEGORY_EXTERN(VRExpansionFunctionLibraryLog, Log, All);

UENUM(BlueprintType)
enum class EBPHMDWornState : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	Worn UMETA(DisplayName = "Worn"),
	NotWorn UMETA(DisplayName = "Not Worn"),
};

UCLASS()
class VREXPANSIONPLUGIN_API UVRExpansionFunctionLibrary : public UBlueprintFunctionLibrary
{

	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static UGameViewportClient * GetGameViewportClient(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|Collision", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static void SetObjectsIgnoreCollision(UObject* WorldContextObject, UPrimitiveComponent* Prim1 = nullptr, FName OptionalBoneName1 = NAME_None, bool bAddChildBones1 = false, UPrimitiveComponent* Prim2 = nullptr, FName OptionalBoneName2 = NAME_None, bool bAddChildBones2 = false, bool bIgnoreCollision = true);

	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|Collision", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static void SetActorsIgnoreAllCollision(UObject* WorldContextObject, AActor* Actor1 = nullptr, AActor* Actor2 = nullptr, bool bIgnoreCollision = true);

	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|Collision", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static void RemoveObjectCollisionIgnore(UObject* WorldContextObject, UPrimitiveComponent* Prim1);

	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|Collision", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static void RemoveActorCollisionIgnore(UObject* WorldContextObject, AActor* Actor1);

	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|Collision", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static bool IsComponentIgnoringCollision(UObject* WorldContextObject, UPrimitiveComponent* Prim1);

	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|Collision", meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static bool AreComponentsIgnoringCollisions(UObject* WorldContextObject, UPrimitiveComponent* Prim1, UPrimitiveComponent* Prim2);

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetHandFromMotionSourceName"))
	static bool GetHandFromMotionSourceName(FName MotionSource, EControllerHand& Hand);

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetHMDPureYaw"))
	static FRotator GetHMDPureYaw(FRotator HMDRotation);

	inline static FRotator GetHMDPureYaw_I(FRotator HMDRotation)
	{

		FRotationMatrix HeadMat(HMDRotation);
		FVector forward = HeadMat.GetScaledAxis(EAxis::X);
		FVector forwardLeveled = forward;
		forwardLeveled.Z = 0;
		forwardLeveled.Normalize();
		FVector mixedInLocalForward = HeadMat.GetScaledAxis(EAxis::Z);

		if (forward.Z > 0)
		{
			mixedInLocalForward = -mixedInLocalForward;
		}

		mixedInLocalForward.Z = 0;
		mixedInLocalForward.Normalize();
		float dot = FMath::Clamp(FVector::DotProduct(forwardLeveled, forward), 0.0f, 1.0f);
		FVector finalForward = FMath::Lerp(mixedInLocalForward, forwardLeveled, dot * dot);

		return FRotationMatrix::MakeFromXZ(finalForward, FVector::UpVector).Rotator();
	}

	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "RotateAroundPivot"))
	static void RotateAroundPivot(FRotator RotationDelta, FVector OriginalLocation, FRotator OriginalRotation, FVector PivotPoint, FVector & NewLocation, FRotator & NewRotation,bool bUseOriginalYawOnly = true)
	{		
		if (bUseOriginalYawOnly)
		{

			NewRotation.Pitch = OriginalRotation.Pitch;
			NewRotation.Roll = OriginalRotation.Roll;

			OriginalRotation.Roll = 0;
			OriginalRotation.Pitch = 0;

			NewLocation = OriginalLocation + OriginalRotation.RotateVector(PivotPoint);

			OriginalRotation.Yaw = (OriginalRotation.Quaternion() * RotationDelta.Quaternion()).Rotator().Yaw;
			NewRotation.Yaw = OriginalRotation.Yaw;

			NewLocation -= OriginalRotation.RotateVector(PivotPoint);

		}
		else
		{
			NewLocation = OriginalLocation + OriginalRotation.RotateVector(PivotPoint);
			NewRotation = (OriginalRotation.Quaternion() * RotationDelta.Quaternion()).Rotator();
			NewLocation -= NewRotation.RotateVector(PivotPoint);
		}
	}

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "FindBetween"))
		static FRotator BPQuat_FindBetween(FVector Vec1, FVector Vec2)
	{
		return FQuat::FindBetween(Vec1, Vec2).Rotator();
	}

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetIsHMDConnected"))
	static bool GetIsHMDConnected();

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetIsHMDWorn"))
	static EBPHMDWornState GetIsHMDWorn();

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetHMDType"))
	static EBPHMDDeviceType GetHMDType();

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "IsInVREditorPreviewOrGame"))
	static bool IsInVREditorPreviewOrGame();

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "IsInVREditorPreview"))
	static bool IsInVREditorPreview();

	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions", meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext))
	static void NonAuthorityMinimumAreaRectangle(UObject* WorldContextObject, const TArray<FVector>& InVerts, const FVector& SampleSurfaceNormal, FVector& OutRectCenter, FRotator& OutRectRotation, float& OutSideLengthX, float& OutSideLengthY, bool bDebugDraw = false);

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "LowPassFilter_RollingAverage"))
	static void LowPassFilter_RollingAverage(FVector lastAverage, FVector newSample, FVector & newAverage, int32 numSamples = 10);

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "LowPassFilter_Exponential"))
	static void LowPassFilter_Exponential(FVector lastAverage, FVector newSample, FVector & newAverage, float sampleFactor = 0.25f);

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetIsActorMovable"))
	static bool GetIsActorMovable(AActor * ActorToCheck);

	UFUNCTION(BlueprintPure, Category = "VRGrip", meta = (bIgnoreSelf = "true", DisplayName = "GetGripSlotInRangeByTypeName"))
	static void GetGripSlotInRangeByTypeName(FName SlotType, AActor * Actor, FVector WorldLocation, float MaxRange, bool & bHadSlotInRange, FTransform & SlotWorldTransform, FName & SlotName, UGripMotionControllerComponent* QueryController = nullptr);

	UFUNCTION(BlueprintPure, Category = "VRGrip", meta = (bIgnoreSelf = "true", DisplayName = "GetGripSlotInRangeByTypeName_Component"))
	static void GetGripSlotInRangeByTypeName_Component(FName SlotType, USceneComponent * Component, FVector WorldLocation, float MaxRange, bool & bHadSlotInRange, FTransform & SlotWorldTransform, FName & SlotName, UGripMotionControllerComponent* QueryController = nullptr);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal VR Grip", CompactNodeTitle = "==", Keywords = "== equal"), Category = "VRExpansionFunctions")
	static bool EqualEqual_FBPActorGripInformation(const FBPActorGripInformation &A, const FBPActorGripInformation &B);

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions")
		static bool IsActiveGrip(const FBPActorGripInformation& Grip);

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions")
		static UPrimitiveComponent* GetGripPrimitiveTarget(const FBPActorGripInformation& Grip);

	UFUNCTION(BlueprintPure, meta = (Scale = "1,1,1", Keywords = "construct build", NativeMakeFunc), Category = "VRExpansionLibrary|TransformNetQuantize")
		static FTransform_NetQuantize MakeTransform_NetQuantize(FVector Location, FRotator Rotation, FVector Scale);

	UFUNCTION(BlueprintPure, Category = "VRExpansionLibrary|TransformNetQuantize", meta = (NativeBreakFunc))
		static void BreakTransform_NetQuantize(const FTransform_NetQuantize& InTransform, FVector& Location, FRotator& Rotation, FVector& Scale);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTransform_NetQuantize (Transform)", CompactNodeTitle = "->", BlueprintAutocast), Category = "VRExpansionLibrary|TransformNetQuantize")
		static FTransform_NetQuantize Conv_TransformToTransformNetQuantize(const FTransform &InTransform);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToVector_NetQuantize (Vector)", CompactNodeTitle = "->", BlueprintAutocast), Category = "VRExpansionLibrary|FVectorNetQuantize")
		static FVector_NetQuantize Conv_FVectorToFVectorNetQuantize(const FVector &InVector);

	UFUNCTION(BlueprintPure, meta = (Scale = "1,1,1", Keywords = "construct build", NativeMakeFunc), Category = "VRExpansionLibrary|FVectorNetQuantize")
		static FVector_NetQuantize MakeVector_NetQuantize(FVector InVector);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToVector_NetQuantize10 (Vector)", CompactNodeTitle = "->", BlueprintAutocast), Category = "VRExpansionLibrary|FVectorNetQuantize")
		static FVector_NetQuantize10 Conv_FVectorToFVectorNetQuantize10(const FVector &InVector);

	UFUNCTION(BlueprintPure, meta = (Scale = "1,1,1", Keywords = "construct build", NativeMakeFunc), Category = "VRExpansionLibrary|FVectorNetQuantize")
		static FVector_NetQuantize10 MakeVector_NetQuantize10(FVector InVector);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToVector_NetQuantize100 (Vector)", CompactNodeTitle = "->", BlueprintAutocast), Category = "VRExpansionLibrary|FVectorNetQuantize")
		static FVector_NetQuantize100 Conv_FVectorToFVectorNetQuantize100(const FVector &InVector);

	UFUNCTION(BlueprintPure, meta = (Scale = "1,1,1", Keywords = "construct build", NativeMakeFunc), Category = "VRExpansionLibrary|FVectorNetQuantize")
		static FVector_NetQuantize100 MakeVector_NetQuantize100(FVector InVector);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToController (FBPGripPair)", CompactNodeTitle = "->", BlueprintAutocast), Category = "VRExpansionLibrary")
		static UGripMotionControllerComponent * Conv_GripPairToMotionController(const FBPGripPair &GripPair);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToGripID (FBPGripPair)", CompactNodeTitle = "->", BlueprintAutocast), Category = "VRExpansionLibrary")
		static uint8 Conv_GripPairToGripID(const FBPGripPair &GripPair);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Scene Component By Class"), Category = "VRExpansionLibrary")
		static USceneComponent* AddSceneComponentByClass(UObject* Outer, TSubclassOf<USceneComponent> Class, const FTransform & ComponentRelativeTransform);

	UFUNCTION(BlueprintCallable, Category = "LowPassFilter_Peak")
		static void ResetPeakLowPassFilter(UPARAM(ref) FBPLowPassPeakFilter& TargetPeakFilter);

	UFUNCTION(BlueprintCallable, Category = "LowPassFilter_Peak")
		static void UpdatePeakLowPassFilter(UPARAM(ref) FBPLowPassPeakFilter& TargetPeakFilter, FVector NewSample);

	UFUNCTION(BlueprintCallable, Category = "LowPassFilter_Peak")
		static FVector GetPeak_PeakLowPassFilter(UPARAM(ref) FBPLowPassPeakFilter& TargetPeakFilter);

	UFUNCTION(BlueprintCallable, Category = "EuroLowPassFilter")
	static void ResetEuroSmoothingFilter(UPARAM(ref) FBPEuroLowPassFilter& TargetEuroFilter);

	UFUNCTION(BlueprintCallable, Category = "EuroLowPassFilter")
	static void RunEuroSmoothingFilter(UPARAM(ref) FBPEuroLowPassFilter& TargetEuroFilter, FVector InRawValue, const float DeltaTime, FVector & SmoothedValue);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Smooth Update Laser Spline"), Category = "VRExpansionLibrary")
	static void SmoothUpdateLaserSpline(USplineComponent * LaserSplineComponent, TArray<USplineMeshComponent *> LaserSplineMeshComponents, FVector InStartLocation, FVector InEndLocation, FVector InForward, float LaserRadius);

	UFUNCTION(BlueprintPure, Category = "GameplayTags")
	static bool MatchesAnyTagsWithDirectParentTag(FGameplayTag DirectParentTag,const FGameplayTagContainer& BaseContainer, const FGameplayTagContainer& OtherContainer);

	UFUNCTION(BlueprintPure, Category = "GameplayTags")
	static bool GetFirstGameplayTagWithExactParent(FGameplayTag DirectParentTag, const FGameplayTagContainer& BaseContainer, FGameplayTag& FoundTag);

};	

