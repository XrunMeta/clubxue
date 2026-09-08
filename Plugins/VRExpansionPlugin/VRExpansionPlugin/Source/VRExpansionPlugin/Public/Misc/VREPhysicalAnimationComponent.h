#pragma once

#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "EngineDefines.h"
#if UE_ENABLE_DEBUG_DRAWING

#endif
#include "VREPhysicalAnimationComponent.generated.h"

struct FReferenceSkeleton;

USTRUCT()
struct VREXPANSIONPLUGIN_API FWeldedBoneDriverData
{
	GENERATED_BODY()
public:
	FTransform RelativeTransform;
	FName BoneName;

	FTransform LastLocal;

	FWeldedBoneDriverData() :
		RelativeTransform(FTransform::Identity),
		BoneName(NAME_None)
	{
	}

	FORCEINLINE bool operator==(const FName& Other) const
	{
		return (BoneName == Other);
	}
};

UCLASS(meta = (BlueprintSpawnableComponent), ClassGroup = Physics)
class VREXPANSIONPLUGIN_API UVREPhysicalAnimationComponent : public UPhysicalAnimationComponent
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeldedBoneDriver)
		bool bIsPaused;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeldedBoneDriver)
		bool bAutoSetPhysicsSleepSensitivity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeldedBoneDriver)
		float SleepThresholdMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeldedBoneDriver)
		TArray<FName> BaseWeldedBoneDriverNames;

	UPROPERTY()
		TArray<FWeldedBoneDriverData> BoneDriverMap;

	UFUNCTION(BlueprintCallable, Category = PhysicalAnimation)
	void SetupWeldedBoneDriver(TArray<FName> BaseBoneNames);

	UFUNCTION(BlueprintCallable, Category = PhysicalAnimation)
		void RefreshWeldedBoneDriver();

	UFUNCTION(BlueprintCallable, Category = PhysicalAnimation)
		void SetWeldedBoneDriverPaused(bool bPaused);

	UFUNCTION(BlueprintPure, Category = PhysicalAnimation)
		bool IsWeldedBoneDriverPaused();

	void SetupWeldedBoneDriver_Implementation(bool bReInit = false);

	void UpdateWeldedBoneDriver(float DeltaTime);

#if UE_ENABLE_DEBUG_DRAWING

#endif

	FTransform GetWorldSpaceRefBoneTransform(FReferenceSkeleton& RefSkel, int32 BoneIndex, int32 ParentBoneIndex);
	FTransform GetRefPoseBoneRelativeTransform(USkeletalMeshComponent* SkeleMesh, FName BoneName, FName ParentBoneName);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
