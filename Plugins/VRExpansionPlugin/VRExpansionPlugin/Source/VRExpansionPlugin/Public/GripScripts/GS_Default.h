#pragma once

#include "CoreMinimal.h"
#include "VRGripScriptBase.h"
#include "GS_Default.generated.h"

UCLASS(NotBlueprintable, ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UGS_Default : public UVRGripScriptBase
{
	GENERATED_BODY()
public:

	UGS_Default(const FObjectInitializer& ObjectInitializer);

	virtual bool GetWorldTransform_Implementation(UGripMotionControllerComponent * GrippingController, float DeltaTime, FTransform & WorldTransform, const FTransform &ParentTransform, FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport) override;

	virtual void GetAnyScaling(FVector& Scaler, FBPActorGripInformation& Grip, FVector& frontLoc, FVector& frontLocOrig, ESecondaryGripType SecondaryType, FTransform& SecondaryTransform);	
	virtual void ApplySmoothingAndLerp(FBPActorGripInformation& Grip, FVector& frontLoc, FVector& frontLocOrig, float DeltaTime);

	virtual void CalculateSecondaryLocation(FVector & frontLoc, const FVector& BasePoint, FBPActorGripInformation& Grip, UGripMotionControllerComponent * GrippingController);
};

UCLASS(BlueprintType, ClassGroup = (VRExpansionPlugin), hideCategories = TickSettings)
class VREXPANSIONPLUGIN_API UGS_ExtendedDefault : public UGS_Default
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SecondaryGripSettings")
		bool bLimitGripScaling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SecondaryGripSettings", meta = (editcondition = "bLimitGripScaling"))
		FVector_NetQuantize100 MinimumGripScaling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SecondaryGripSettings", meta = (editcondition = "bLimitGripScaling"))
		FVector_NetQuantize100 MaximumGripScaling;

	virtual void GetAnyScaling(FVector& Scaler, FBPActorGripInformation& Grip, FVector& frontLoc, FVector& frontLocOrig, ESecondaryGripType SecondaryType, FTransform& SecondaryTransform) override;
};