

#pragma once

#include "CoreMinimal.h"
#include "VRGripScriptBase.h"
#include "VRBPDatatypes.h"
#include "Curves/CurveFloat.h"
#include "GS_LerpToHand.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVRLerpToHandFinishedSignature);

UCLASS(NotBlueprintable, ClassGroup = (VRExpansionPlugin), hideCategories = TickSettings)
class VREXPANSIONPLUGIN_API UGS_LerpToHand : public UVRGripScriptBase
{
	GENERATED_BODY()
public:

	UGS_LerpToHand(const FObjectInitializer& ObjectInitializer);

	float CurrentLerpTime;
	float LerpSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LerpSettings")
		float MinDistanceForLerp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LerpSettings")
		float LerpDuration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LerpSettings")
		float MinSpeedForLerp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LerpSettings")
		float MaxSpeedForLerp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LerpSettings")
	EVRLerpInterpolationMode LerpInterpolationMode;

	UPROPERTY(BlueprintAssignable, Category = "LerpEvents")
		FVRLerpToHandFinishedSignature OnLerpToHandBegin;

	UPROPERTY(BlueprintAssignable, Category = "LerpEvents")
		FVRLerpToHandFinishedSignature OnLerpToHandFinished;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LerpCurve")
		bool bUseCurve;

	UPROPERTY(Category = "LerpCurve", EditAnywhere, meta = (editcondition = "bUseCurve"))
		FRuntimeFloatCurve OptionalCurveToFollow;

	FTransform OnGripTransform;
	uint8 TargetGrip;

	virtual bool GetWorldTransform_Implementation(UGripMotionControllerComponent * OwningController, float DeltaTime, FTransform & WorldTransform, const FTransform &ParentTransform, FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport) override;
	virtual void OnGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) override;
	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed) override;
};
