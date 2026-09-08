

#pragma once

#include "CoreMinimal.h"
#include "GripMotionControllerComponent.h"

#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/ActorChannel.h"
#include "OptionalRepSkeletalMeshActor.generated.h"

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent, ChildCanTick), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UNoRepSphereComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UNoRepSphereComponent(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Component Replication")
		bool bReplicateMovement;
public:
	bool GetReplicateMovement() { return bReplicateMovement; }
	void SetReplicateMovement(bool bNewReplicateMovement);

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
};

UCLASS(Blueprintable, meta = (ChildCanTick, BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UInversePhysicsSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UInversePhysicsSkeletalMeshComponent(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Component Replication")
		bool bReplicateMovement;
public:
	bool GetReplicateMovement() { return bReplicateMovement; }
	void SetReplicateMovement(bool bNewReplicateMovement);

	virtual void GetWeldedBodies(TArray<FBodyInstance*>& OutWeldedBodies, TArray<FName>& OutLabels, bool bIncludingAutoWeld) override;
	virtual FBodyInstance* GetBodyInstance(FName BoneName = NAME_None, bool bGetWelded = true, int32 Index = INDEX_NONE) const override;

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions")
	FBoxSphereBounds GetLocalBounds() const
	{
		return this->GetCachedLocalBounds();
	}

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent, ChildCanTick), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API AOptionalRepGrippableSkeletalMeshActor : public ASkeletalMeshActor
{
	GENERATED_BODY()

public:
	AOptionalRepGrippableSkeletalMeshActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Replication")
		bool bIgnoreAttachmentReplication;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Replication")
		bool bIgnorePhysicsReplication;

	virtual void OnRep_ReplicateMovement() override;
	virtual void PostNetReceivePhysicState() override;
};