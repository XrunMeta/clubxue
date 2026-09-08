

#include "Misc/OptionalRepSkeletalMeshActor.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(OptionalRepSkeletalMeshActor)

#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsReplication.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

UNoRepSphereComponent::UNoRepSphereComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SetIsReplicatedByDefault(true);
	this->PrimaryComponentTick.bCanEverTick = false;
	SphereRadius = 4.0f;
	SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);

	BodyInstance.bOverrideMass = true; 
	BodyInstance.SetMassOverride(0.f);
}

void UNoRepSphereComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UNoRepSphereComponent, bReplicateMovement, PushModelParams);

	RESET_REPLIFETIME_CONDITION_PRIVATE_PROPERTY(USceneComponent, AttachParent, COND_InitialOnly);
	RESET_REPLIFETIME_CONDITION_PRIVATE_PROPERTY(USceneComponent, AttachSocketName, COND_InitialOnly);
	RESET_REPLIFETIME_CONDITION_PRIVATE_PROPERTY(USceneComponent, AttachChildren, COND_InitialOnly);
	RESET_REPLIFETIME_CONDITION_PRIVATE_PROPERTY(USceneComponent, RelativeLocation, COND_InitialOnly);
	RESET_REPLIFETIME_CONDITION_PRIVATE_PROPERTY(USceneComponent, RelativeRotation, COND_InitialOnly);
	RESET_REPLIFETIME_CONDITION_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D, COND_InitialOnly);

}

void UNoRepSphereComponent::SetReplicateMovement(bool bNewReplicateMovement)
{
	bReplicateMovement = bNewReplicateMovement;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UNoRepSphereComponent, bReplicateMovement, this);
#endif
}

void UNoRepSphereComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

}

UInversePhysicsSkeletalMeshComponent::UInversePhysicsSkeletalMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicateMovement = true;

	bReplicatePhysicsToAutonomousProxy = false;

}

void UInversePhysicsSkeletalMeshComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (!IRISNetReplication::IsIris(this))
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeLocation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeRotation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeScale3D, bReplicateMovement);
	}
}

void UInversePhysicsSkeletalMeshComponent::SetReplicateMovement(bool bNewReplicateMovement)
{
	bReplicateMovement = bNewReplicateMovement;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UInversePhysicsSkeletalMeshComponent, bReplicateMovement, this);
#endif
}

void UInversePhysicsSkeletalMeshComponent::GetWeldedBodies(TArray<FBodyInstance*>& OutWeldedBodies, TArray<FName>& OutLabels, bool bIncludingAutoWeld)
{
	UPhysicsAsset* PhysicsAsset = GetPhysicsAsset();

	for (int32 BodyIdx = 0; BodyIdx < Bodies.Num(); ++BodyIdx)
	{
		FBodyInstance* BI = Bodies[BodyIdx];
		if (BI && (BI->WeldParent != nullptr || (bIncludingAutoWeld && BI->bAutoWeld)))
		{
			OutWeldedBodies.Add(BI);
			if (PhysicsAsset)
			{
				if (UBodySetup* PhysicsAssetBodySetup = PhysicsAsset->SkeletalBodySetups[BodyIdx].Get())
				{
					OutLabels.Add(PhysicsAssetBodySetup->BoneName);
				}
				else
				{
					OutLabels.Add(NAME_None);
				}
			}
			else
			{
				OutLabels.Add(NAME_None);
			}

		}
	}

	for (USceneComponent* Child : GetAttachChildren())
	{
		if (UPrimitiveComponent* PrimChild = Cast<UPrimitiveComponent>(Child))
		{
			PrimChild->GetWeldedBodies(OutWeldedBodies, OutLabels, bIncludingAutoWeld);
		}
	}
}

FBodyInstance* UInversePhysicsSkeletalMeshComponent::GetBodyInstance(FName BoneName, bool bGetWelded, int32) const
{
	UPhysicsAsset* const PhysicsAsset = GetPhysicsAsset();
	FBodyInstance* BodyInst = NULL;

	if (PhysicsAsset != NULL)
	{

		if (BoneName == NAME_None)
		{
			if (Bodies.IsValidIndex(RootBodyData.BodyIndex))
			{
				BodyInst = Bodies[RootBodyData.BodyIndex];
			}
		}

		else
		{
			int32 BodyIndex = PhysicsAsset->FindBodyIndex(BoneName);
			if (Bodies.IsValidIndex(BodyIndex))
			{
				BodyInst = Bodies[BodyIndex];
			}
		}

		BodyInst = (bGetWelded && BodyInstance.WeldParent) ? BodyInstance.WeldParent : BodyInst;
	}

	return BodyInst;
}

void UInversePhysicsSkeletalMeshComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UInversePhysicsSkeletalMeshComponent, bReplicateMovement, PushModelParams);
}

AOptionalRepGrippableSkeletalMeshActor::AOptionalRepGrippableSkeletalMeshActor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UInversePhysicsSkeletalMeshComponent>(TEXT("SkeletalMeshComponent0")))
{
	bIgnoreAttachmentReplication = false;
	bIgnorePhysicsReplication = false;
}

void AOptionalRepGrippableSkeletalMeshActor::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_InitialOnly, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(AOptionalRepGrippableSkeletalMeshActor, bIgnoreAttachmentReplication, PushModelParamsWithCondition);
	DOREPLIFETIME_WITH_PARAMS_FAST(AOptionalRepGrippableSkeletalMeshActor, bIgnorePhysicsReplication, PushModelParamsWithCondition);

	if (bIgnoreAttachmentReplication)
	{
		RESET_REPLIFETIME_CONDITION_PRIVATE_PROPERTY(AActor, AttachmentReplication, COND_InitialOnly);
	}

}

void AOptionalRepGrippableSkeletalMeshActor::OnRep_ReplicateMovement()
{
	if (bIgnorePhysicsReplication)
	{
		return;
	}

	Super::OnRep_ReplicateMovement();
}

void AOptionalRepGrippableSkeletalMeshActor::PostNetReceivePhysicState()
{
	if (bIgnorePhysicsReplication)
	{
		return;
	}

	Super::PostNetReceivePhysicState();
}