

#include "Grippables/GrippableSkeletalMeshActor.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GrippableSkeletalMeshActor)

#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GripMotionControllerComponent.h"
#include "VRExpansionFunctionLibrary.h"
#include "Misc/BucketUpdateSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsReplication.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsEngine/PhysicsAsset.h" 
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

UOptionalRepSkeletalMeshComponent::UOptionalRepSkeletalMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UOptionalRepSkeletalMeshComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (!IRISNetReplication::IsIris(this))
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeLocation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeRotation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeScale3D, bReplicateMovement);
	}
}

void UOptionalRepSkeletalMeshComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UOptionalRepSkeletalMeshComponent, bReplicateMovement, PushModelParams);
}

void UOptionalRepSkeletalMeshComponent::SetReplicateMovement(bool bNewReplicateMovement)
{
	bReplicateMovement = bNewReplicateMovement;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UOptionalRepSkeletalMeshComponent, bReplicateMovement, this);
#endif
}

void UOptionalRepSkeletalMeshComponent::GetWeldedBodies(TArray<FBodyInstance*>& OutWeldedBodies, TArray<FName>& OutLabels, bool bIncludingAutoWeld)
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

FBodyInstance* UOptionalRepSkeletalMeshComponent::GetBodyInstance(FName BoneName, bool bGetWelded, int32) const
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

AGrippableSkeletalMeshActor::AGrippableSkeletalMeshActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UOptionalRepSkeletalMeshComponent>(TEXT("SkeletalMeshComponent0")))
{
	VRGripInterfaceSettings.bDenyGripping = false;
	VRGripInterfaceSettings.OnTeleportBehavior = EGripInterfaceTeleportBehavior::TeleportAllComponents;
	VRGripInterfaceSettings.bSimulateOnDrop = true;
	VRGripInterfaceSettings.SlotDefaultGripType = EGripCollisionType::InteractiveCollisionWithPhysics;
	VRGripInterfaceSettings.FreeDefaultGripType = EGripCollisionType::InteractiveCollisionWithPhysics;
	VRGripInterfaceSettings.SecondaryGripType = ESecondaryGripType::SG_None;
	VRGripInterfaceSettings.MovementReplicationType = EGripMovementReplicationSettings::ForceClientSideMovement;
	VRGripInterfaceSettings.LateUpdateSetting = EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping;
	VRGripInterfaceSettings.ConstraintStiffness = 1500.0f;
	VRGripInterfaceSettings.ConstraintDamping = 200.0f;
	VRGripInterfaceSettings.ConstraintBreakDistance = 100.0f;
	VRGripInterfaceSettings.SecondarySlotRange = 20.0f;
	VRGripInterfaceSettings.PrimarySlotRange = 20.0f;

	VRGripInterfaceSettings.bIsHeld = false;

	SetReplicatingMovement(true);
	bReplicates = true;

	bRepGripSettingsAndGameplayTags = true;
	bReplicateGripScripts = false;

	bReplicateUsingRegisteredSubObjectList = true;

	bAllowIgnoringAttachOnOwner = true;

	SetMinNetUpdateFrequency(30.0f);
}

void AGrippableSkeletalMeshActor::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableSkeletalMeshActor, bReplicateGripScripts, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableSkeletalMeshActor, bRepGripSettingsAndGameplayTags, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableSkeletalMeshActor, bAllowIgnoringAttachOnOwner, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableSkeletalMeshActor, ClientAuthReplicationData, PushModelParams);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_Custom, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableSkeletalMeshActor, GripLogicScripts, PushModelParamsWithCondition);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableSkeletalMeshActor, VRGripInterfaceSettings, PushModelParamsWithCondition);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableSkeletalMeshActor, GameplayTags, PushModelParamsWithCondition);

	DISABLE_REPLICATED_PRIVATE_PROPERTY(AActor, AttachmentReplication);

	FDoRepLifetimeParams AttachmentReplicationParams{ COND_Custom, REPNOTIFY_Always, true };
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableSkeletalMeshActor, AttachmentWeldReplication, AttachmentReplicationParams);
}

void AGrippableSkeletalMeshActor::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{

	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(AGrippableSkeletalMeshActor, VRGripInterfaceSettings, bRepGripSettingsAndGameplayTags);
	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(AGrippableSkeletalMeshActor, GameplayTags, bRepGripSettingsAndGameplayTags);
	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(AGrippableSkeletalMeshActor, GripLogicScripts, bReplicateGripScripts);

#if WITH_PUSH_MODEL
	const AActor* const OldAttachParent = AttachmentWeldReplication.AttachParent;
	const UActorComponent* const OldAttachComponent = AttachmentWeldReplication.AttachComponent;
#endif

	AttachmentWeldReplication.AttachParent = nullptr;
	AttachmentWeldReplication.AttachComponent = nullptr;

	GatherCurrentMovement();

	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(AActor, ReplicatedMovement, IsReplicatingMovement());

	DOREPLIFETIME_ACTIVE_OVERRIDE(AGrippableSkeletalMeshActor, AttachmentWeldReplication, RootComponent && !RootComponent->GetIsReplicated());

#if WITH_PUSH_MODEL
	if (UNLIKELY(OldAttachParent != AttachmentWeldReplication.AttachParent || OldAttachComponent != AttachmentWeldReplication.AttachComponent))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableSkeletalMeshActor, AttachmentWeldReplication, this);
	}
#endif

}

void AGrippableSkeletalMeshActor::GatherCurrentMovement()
{
	Super::GatherCurrentMovement();

	FRepMovement RepMovement = GetReplicatedMovement();
	if (RootComponent && (!RepMovement.bRepPhysics || RootComponent->GetAttachParent()))
	{
		UPrimitiveComponent* RootPrimComp = Cast<UPrimitiveComponent>(GetRootComponent());

		if (!RepMovement.bRepPhysics || (!RootPrimComp || !RootPrimComp->IsSimulatingPhysics()))
		{
			bool bWasAttachmentModified = false;

			AActor* OldAttachParent = AttachmentWeldReplication.AttachParent;
			USceneComponent* OldAttachComponent = AttachmentWeldReplication.AttachComponent;

			AttachmentWeldReplication.AttachParent = nullptr;
			AttachmentWeldReplication.AttachComponent = nullptr;

			if (RootComponent->GetAttachParent() != nullptr)
			{

				AttachmentWeldReplication.AttachParent = RootComponent->GetAttachParentActor();
				if (AttachmentWeldReplication.AttachParent != nullptr)
				{
					AttachmentWeldReplication.LocationOffset = RootComponent->GetRelativeLocation();
					AttachmentWeldReplication.RotationOffset = RootComponent->GetRelativeRotation();
					AttachmentWeldReplication.RelativeScale3D = RootComponent->GetRelativeScale3D();
					AttachmentWeldReplication.AttachComponent = RootComponent->GetAttachParent();
					AttachmentWeldReplication.AttachSocket = RootComponent->GetAttachSocketName();
					AttachmentWeldReplication.bIsWelded = RootPrimComp ? RootPrimComp->IsWelded() : false;

					bWasAttachmentModified = true;
				}
			}

			if (bWasAttachmentModified ||
				OldAttachParent != AttachmentWeldReplication.AttachParent ||
				OldAttachComponent != AttachmentWeldReplication.AttachComponent)
			{
#if WITH_PUSH_MODEL
				MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableSkeletalMeshActor, AttachmentWeldReplication, this);
#endif
			}
		}
	}
}

bool AGrippableSkeletalMeshActor::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (bReplicateGripScripts)
	{
		for (UVRGripScriptBase* Script : GripLogicScripts)
		{
			if (Script && IsValid(Script))
			{
				WroteSomething |= Channel->ReplicateSubobject(Script, *Bunch, *RepFlags);
			}
		}
	}

	return WroteSomething;
}

AGrippableSkeletalMeshActor::~AGrippableSkeletalMeshActor()
{
}

void AGrippableSkeletalMeshActor::BeginPlay()
{

	Super::BeginPlay();

	for (UVRGripScriptBase* Script : GripLogicScripts)
	{
		if (Script)
		{
			Script->BeginPlay(this);
		}
	}
}

void AGrippableSkeletalMeshActor::SetDenyGripping(bool bDenyGripping)
{
	VRGripInterfaceSettings.bDenyGripping = bDenyGripping;
}

void AGrippableSkeletalMeshActor::SetGripPriority(int NewGripPriority)
{
	VRGripInterfaceSettings.AdvancedGripSettings.GripPriority = NewGripPriority;
}

void AGrippableSkeletalMeshActor::TickGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation, float DeltaTime) {}
void AGrippableSkeletalMeshActor::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) { }
void AGrippableSkeletalMeshActor::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) { }
void AGrippableSkeletalMeshActor::OnChildGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) {}
void AGrippableSkeletalMeshActor::OnChildGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) {}
void AGrippableSkeletalMeshActor::OnSecondaryGrip_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* SecondaryGripComponent, const FBPActorGripInformation& GripInformation) { OnSecondaryGripAdded.Broadcast(GripOwningController, GripInformation); }
void AGrippableSkeletalMeshActor::OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* ReleasingSecondaryGripComponent, const FBPActorGripInformation& GripInformation) { OnSecondaryGripRemoved.Broadcast(GripOwningController, GripInformation); }
void AGrippableSkeletalMeshActor::OnUsed_Implementation() {}
void AGrippableSkeletalMeshActor::OnEndUsed_Implementation() {}
void AGrippableSkeletalMeshActor::OnSecondaryUsed_Implementation() {}
void AGrippableSkeletalMeshActor::OnEndSecondaryUsed_Implementation() {}
void AGrippableSkeletalMeshActor::OnInput_Implementation(FKey Key, EInputEvent KeyEvent) {}
bool AGrippableSkeletalMeshActor::RequestsSocketing_Implementation(USceneComponent*& ParentToSocketTo, FName& OptionalSocketName, FTransform_NetQuantize& RelativeTransform) { return false; }

bool AGrippableSkeletalMeshActor::DenyGripping_Implementation(UGripMotionControllerComponent * GripInitiator)
{
	return VRGripInterfaceSettings.bDenyGripping;
}

EGripInterfaceTeleportBehavior AGrippableSkeletalMeshActor::TeleportBehavior_Implementation()
{
	return VRGripInterfaceSettings.OnTeleportBehavior;
}

bool AGrippableSkeletalMeshActor::SimulateOnDrop_Implementation()
{
	return VRGripInterfaceSettings.bSimulateOnDrop;
}

EGripCollisionType AGrippableSkeletalMeshActor::GetPrimaryGripType_Implementation(bool bIsSlot)
{
	return bIsSlot ? VRGripInterfaceSettings.SlotDefaultGripType : VRGripInterfaceSettings.FreeDefaultGripType;
}

ESecondaryGripType AGrippableSkeletalMeshActor::SecondaryGripType_Implementation()
{
	return VRGripInterfaceSettings.SecondaryGripType;
}

EGripMovementReplicationSettings AGrippableSkeletalMeshActor::GripMovementReplicationType_Implementation()
{
	return VRGripInterfaceSettings.MovementReplicationType;
}

EGripLateUpdateSettings AGrippableSkeletalMeshActor::GripLateUpdateSetting_Implementation()
{
	return VRGripInterfaceSettings.LateUpdateSetting;
}

void AGrippableSkeletalMeshActor::GetGripStiffnessAndDamping_Implementation(float& GripStiffnessOut, float& GripDampingOut)
{
	GripStiffnessOut = VRGripInterfaceSettings.ConstraintStiffness;
	GripDampingOut = VRGripInterfaceSettings.ConstraintDamping;
}

FBPAdvGripSettings AGrippableSkeletalMeshActor::AdvancedGripSettings_Implementation()
{
	return VRGripInterfaceSettings.AdvancedGripSettings;
}

float AGrippableSkeletalMeshActor::GripBreakDistance_Implementation()
{
	return VRGripInterfaceSettings.ConstraintBreakDistance;
}

void AGrippableSkeletalMeshActor::ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool& bHadSlotInRange, FTransform& SlotWorldTransform, FName& SlotName, UGripMotionControllerComponent* CallingController, FName OverridePrefix)
{
	if (OverridePrefix.IsNone())
		bSecondarySlot ? OverridePrefix = "VRGripS" : OverridePrefix = "VRGripP";

	UVRExpansionFunctionLibrary::GetGripSlotInRangeByTypeName(OverridePrefix, this, WorldLocation, bSecondarySlot ? VRGripInterfaceSettings.SecondarySlotRange : VRGripInterfaceSettings.PrimarySlotRange, bHadSlotInRange, SlotWorldTransform, SlotName, CallingController);
}

bool AGrippableSkeletalMeshActor::AllowsMultipleGrips_Implementation()
{
	return VRGripInterfaceSettings.bAllowMultipleGrips;
}

void AGrippableSkeletalMeshActor::IsHeld_Implementation(TArray<FBPGripPair>& HoldingControllers, bool& bIsHeld)
{
	HoldingControllers = VRGripInterfaceSettings.HoldingControllers;
	bIsHeld = VRGripInterfaceSettings.bIsHeld;
}

bool AGrippableSkeletalMeshActor::AddToClientReplicationBucket()
{
	if (ShouldWeSkipAttachmentReplication(false))
	{

		GetWorld()->GetSubsystem<UBucketUpdateSubsystem>()->AddObjectToBucket(ClientAuthReplicationData.UpdateRate, this, FName(TEXT("PollReplicationEvent")));
		ClientAuthReplicationData.bIsCurrentlyClientAuth = true;

		if (UWorld* World = GetWorld())
			ClientAuthReplicationData.TimeAtInitialThrow = World->GetTimeSeconds();

		return true;
	}

	return false;
}

bool AGrippableSkeletalMeshActor::RemoveFromClientReplicationBucket()
{
	if (ClientAuthReplicationData.bIsCurrentlyClientAuth)
	{
		GetWorld()->GetSubsystem<UBucketUpdateSubsystem>()->RemoveObjectFromBucketByFunctionName(this, FName(TEXT("PollReplicationEvent")));
		CeaseReplicationBlocking();
		return true;
	}

	return false;
}

void AGrippableSkeletalMeshActor::Native_NotifyThrowGripDelegates(UGripMotionControllerComponent* Controller, bool bGripped, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	if (bGripped)
	{
		OnGripped.Broadcast(Controller, GripInformation);
	}
	else
	{
		OnDropped.Broadcast(Controller, GripInformation, bWasSocketed);
	}
}

void AGrippableSkeletalMeshActor::SetHeld_Implementation(UGripMotionControllerComponent* HoldingController, uint8 GripID, bool bIsHeld)
{
	if (bIsHeld)
	{
		VRGripInterfaceSettings.HoldingControllers.AddUnique(FBPGripPair(HoldingController, GripID));
		RemoveFromClientReplicationBucket();

		VRGripInterfaceSettings.bWasHeld = true;
		VRGripInterfaceSettings.bIsHeld = VRGripInterfaceSettings.HoldingControllers.Num() > 0;
	}
	else
	{
		VRGripInterfaceSettings.HoldingControllers.Remove(FBPGripPair(HoldingController, GripID));
		VRGripInterfaceSettings.bIsHeld = VRGripInterfaceSettings.HoldingControllers.Num() > 0;

		if (ClientAuthReplicationData.bUseClientAuthThrowing && !VRGripInterfaceSettings.bIsHeld)
		{
			bool bWasLocallyOwned = HoldingController ? HoldingController->IsLocallyControlled() : false;
			if (bWasLocallyOwned && ShouldWeSkipAttachmentReplication(false))
			{
				if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GetRootComponent()))
				{
					if (PrimComp->IsSimulatingPhysics())
					{
						AddToClientReplicationBucket();
					}
				}
			}
		}
	}
}

bool AGrippableSkeletalMeshActor::GetGripScripts_Implementation(TArray<UVRGripScriptBase*>& ArrayReference)
{
	ArrayReference = GripLogicScripts;
	return GripLogicScripts.Num() > 0;
}

bool AGrippableSkeletalMeshActor::PollReplicationEvent()
{
	if (!ClientAuthReplicationData.bIsCurrentlyClientAuth || !this->HasLocalNetOwner() || VRGripInterfaceSettings.bIsHeld)
		return false; 

	UWorld* OurWorld = GetWorld();
	if (!OurWorld)
		return false; 

	bool bRemoveBlocking = false;

	if ((OurWorld->GetTimeSeconds() - ClientAuthReplicationData.TimeAtInitialThrow) > 10.0f)
	{

		bRemoveBlocking = true;
	}

	FTransform CurTransform = this->GetActorTransform();

	if (!bRemoveBlocking)
	{
		if (!CurTransform.GetRotation().Equals(ClientAuthReplicationData.LastActorTransform.GetRotation()) || !CurTransform.GetLocation().Equals(ClientAuthReplicationData.LastActorTransform.GetLocation()))
		{
			ClientAuthReplicationData.LastActorTransform = CurTransform;

			if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(RootComponent))
			{

				if (PrimComp->IsSimulatingPhysics() && ShouldWeSkipAttachmentReplication(false))
				{
					FRepMovementVR ClientAuthMovementRep;
					if (ClientAuthMovementRep.GatherActorsMovement(this))
					{
						Server_GetClientAuthReplication(ClientAuthMovementRep);

						if (PrimComp->RigidBodyIsAwake())
						{
							return true;
						}
					}
				}
			}
			else
			{
				bRemoveBlocking = true;

			}
		}

	}

	bool TimedBlockingRelease = false;

	AActor* TopOwner = GetOwner();
	if (TopOwner != nullptr)
	{
		AActor* tempOwner = TopOwner->GetOwner();

		while (tempOwner)
		{
			TopOwner = tempOwner;
			tempOwner = TopOwner->GetOwner();
		}

		if (APlayerController* PlayerController = Cast<APlayerController>(TopOwner))
		{
			if (APlayerState* PlayerState = PlayerController->PlayerState)
			{
				if (ClientAuthReplicationData.ResetReplicationHandle.IsValid())
				{
					OurWorld->GetTimerManager().ClearTimer(ClientAuthReplicationData.ResetReplicationHandle);
				}

				float clampedPing = FMath::Clamp(PlayerState->ExactPing * 0.001f, 0.0f, 1000.0f);
				OurWorld->GetTimerManager().SetTimer(ClientAuthReplicationData.ResetReplicationHandle, this, &AGrippableSkeletalMeshActor::CeaseReplicationBlocking, clampedPing, false);
				TimedBlockingRelease = true;
			}
		}
	}

	if (!TimedBlockingRelease)
	{
		CeaseReplicationBlocking();
	}

	Server_EndClientAuthReplication();
	return false; 
}

void AGrippableSkeletalMeshActor::CeaseReplicationBlocking()
{
	if (ClientAuthReplicationData.bIsCurrentlyClientAuth)
		ClientAuthReplicationData.bIsCurrentlyClientAuth = false;

	ClientAuthReplicationData.LastActorTransform = FTransform::Identity;

	if (ClientAuthReplicationData.ResetReplicationHandle.IsValid())
	{
		if (UWorld* OurWorld = GetWorld())
		{
			OurWorld->GetTimerManager().ClearTimer(ClientAuthReplicationData.ResetReplicationHandle);
		}
	}
}

void AGrippableSkeletalMeshActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveFromClientReplicationBucket();

	for (UVRGripScriptBase* Script : GripLogicScripts)
	{
		if (Script)
		{
			Script->EndPlay(EndPlayReason);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool AGrippableSkeletalMeshActor::Server_EndClientAuthReplication_Validate()
{
	return true;
}

void AGrippableSkeletalMeshActor::Server_EndClientAuthReplication_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (FPhysScene* PhysScene = World->GetPhysicsScene())
		{ 
			if (IPhysicsReplication* PhysicsReplication = PhysScene->GetPhysicsReplication())
			{
				PhysicsReplication->RemoveReplicatedTarget(this->GetSkeletalMeshComponent());
			}
		}
	}
}

bool AGrippableSkeletalMeshActor::Server_GetClientAuthReplication_Validate(const FRepMovementVR& newMovement)
{
	return true;
}

void AGrippableSkeletalMeshActor::Server_GetClientAuthReplication_Implementation(const FRepMovementVR& newMovement)
{
	if (!VRGripInterfaceSettings.bIsHeld)
	{
		if (!newMovement.Location.ContainsNaN() && !newMovement.Rotation.ContainsNaN())
		{
			FRepMovement& MovementRep = GetReplicatedMovement_Mutable();
			newMovement.CopyTo(MovementRep);
			OnRep_ReplicatedMovement();
		}
	}
}

bool AGrippableSkeletalMeshActor::ShouldWeSkipAttachmentReplication(bool bConsiderHeld) const
{
	if ((bConsiderHeld && !VRGripInterfaceSettings.bWasHeld) || GetNetMode() < ENetMode::NM_Client)
		return false;

	if (VRGripInterfaceSettings.MovementReplicationType == EGripMovementReplicationSettings::ClientSide_Authoritive ||
		VRGripInterfaceSettings.MovementReplicationType == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep)
	{

		for (const FBPGripPair& Grip : VRGripInterfaceSettings.HoldingControllers)
		{
			if (IsValid(Grip.HoldingController) && Grip.HoldingController->IsLocallyControlled())
			{
				return true;
			}
		}

		return HasLocalNetOwner();
	}
	else
		return false;
}

void AGrippableSkeletalMeshActor::OnRep_AttachmentReplication()
{
	if (bAllowIgnoringAttachOnOwner && (ClientAuthReplicationData.bIsCurrentlyClientAuth || ShouldWeSkipAttachmentReplication()))

	{
		return;
	}

	if (AttachmentWeldReplication.AttachParent)
	{
		if (RootComponent)
		{
			USceneComponent* AttachParentComponent = (AttachmentWeldReplication.AttachComponent ? ToRawPtr(AttachmentWeldReplication.AttachComponent) : AttachmentWeldReplication.AttachParent->GetRootComponent());

			if (AttachParentComponent)
			{
				RootComponent->SetRelativeLocation_Direct(AttachmentWeldReplication.LocationOffset);
				RootComponent->SetRelativeRotation_Direct(AttachmentWeldReplication.RotationOffset);
				RootComponent->SetRelativeScale3D_Direct(AttachmentWeldReplication.RelativeScale3D);

				const bool bAlreadyAttached = (AttachParentComponent == RootComponent->GetAttachParent() && AttachmentWeldReplication.AttachSocket == RootComponent->GetAttachSocketName() && AttachParentComponent->GetAttachChildren().Contains(RootComponent));
				if (bAlreadyAttached)
				{

					RootComponent->UpdateComponentToWorld(EUpdateTransformFlags::SkipPhysicsUpdate, ETeleportType::None);
				}
				else
				{
					FAttachmentTransformRules attachRules = FAttachmentTransformRules::KeepRelativeTransform;
					attachRules.bWeldSimulatedBodies = AttachmentWeldReplication.bIsWelded;
					RootComponent->AttachToComponent(AttachParentComponent, attachRules, AttachmentWeldReplication.AttachSocket);
				}
			}
		}
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		if (IsReplicatingMovement())
		{
			OnRep_ReplicatedMovement();
		}
	}
}

void AGrippableSkeletalMeshActor::OnRep_ReplicateMovement()
{
	if (bAllowIgnoringAttachOnOwner && (ClientAuthReplicationData.bIsCurrentlyClientAuth || ShouldWeSkipAttachmentReplication()))

	{
		return;
	}

	if (RootComponent)
	{
		const FRepAttachment ReplicationAttachment = GetAttachmentReplication();
		if (!ReplicationAttachment.AttachParent)
		{
			const FRepMovement& RepMove = GetReplicatedMovement();

			if (RootComponent->IsSimulatingPhysics() != RepMove.bRepPhysics)
			{

				SyncReplicatedPhysicsSimulation();

			}
		}
	}

	Super::OnRep_ReplicateMovement();
}

void AGrippableSkeletalMeshActor::OnRep_ReplicatedMovement()
{
	if (bAllowIgnoringAttachOnOwner && (ClientAuthReplicationData.bIsCurrentlyClientAuth || ShouldWeSkipAttachmentReplication()))

	{
		return;
	}

	Super::OnRep_ReplicatedMovement();
}

void AGrippableSkeletalMeshActor::PostNetReceivePhysicState()
{
	if (bAllowIgnoringAttachOnOwner && (ClientAuthReplicationData.bIsCurrentlyClientAuth || ShouldWeSkipAttachmentReplication()))

	{
		return;
	}

	Super::PostNetReceivePhysicState();
}

void AGrippableSkeletalMeshActor::MarkComponentsAsGarbage(bool bModify)
{
	Super::MarkComponentsAsGarbage(bModify);

	for (int32 i = 0; i < GripLogicScripts.Num(); ++i)
	{
		if (UObject* SubObject = GripLogicScripts[i])
		{
			SubObject->MarkAsGarbage();
		}
	}

	GripLogicScripts.Empty();
}

void AGrippableSkeletalMeshActor::PreDestroyFromReplication()
{
	Super::PreDestroyFromReplication();

	for (int32 i = 0; i < GripLogicScripts.Num(); ++i)
	{
		if (UObject* SubObject = GripLogicScripts[i])
		{
			OnSubobjectDestroyFromReplication(SubObject); 
			SubObject->PreDestroyFromReplication();
			SubObject->MarkAsGarbage();
		}
	}

	for (UActorComponent* ActorComp : GetComponents())
	{

		if (ActorComp && IsValid(ActorComp) && ActorComp->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			ActorComp->PreDestroyFromReplication();
	}

	GripLogicScripts.Empty();
}

void AGrippableSkeletalMeshActor::BeginDestroy()
{
	Super::BeginDestroy();

	for (int32 i = 0; i < GripLogicScripts.Num(); i++)
	{
		if (UObject* SubObject = GripLogicScripts[i])
		{
			SubObject->MarkAsGarbage();
		}
	}

	GripLogicScripts.Empty();
}

void AGrippableSkeletalMeshActor::SetReplicateGripScripts(bool bNewReplicateGripScripts)
{
	bReplicateGripScripts = bNewReplicateGripScripts;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableSkeletalMeshActor, bReplicateGripScripts, this);
#endif
}

TArray<TObjectPtr<UVRGripScriptBase>>& AGrippableSkeletalMeshActor::GetGripLogicScripts()
{
#if WITH_PUSH_MODEL
	if (bReplicateGripScripts)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableSkeletalMeshActor, GripLogicScripts, this);
	}
#endif

	return GripLogicScripts;
}

void AGrippableSkeletalMeshActor::SetRepGripSettingsAndGameplayTags(bool bNewRepGripSettingsAndGameplayTags)
{
	bRepGripSettingsAndGameplayTags = bNewRepGripSettingsAndGameplayTags;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableSkeletalMeshActor, bRepGripSettingsAndGameplayTags, this);
#endif
}

void AGrippableSkeletalMeshActor::SetAllowIgnoringAttachOnOwner(bool bNewAllowIgnoringAttachOnOwner)
{
	bAllowIgnoringAttachOnOwner = bNewAllowIgnoringAttachOnOwner;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableSkeletalMeshActor, bAllowIgnoringAttachOnOwner, this);
#endif
}

FVRClientAuthReplicationData& AGrippableSkeletalMeshActor::GetClientAuthReplicationData(FVRClientAuthReplicationData& ClientAuthData)
{
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableSkeletalMeshActor, ClientAuthReplicationData, this);
#endif
	return ClientAuthReplicationData;
}

FBPInterfaceProperties& AGrippableSkeletalMeshActor::GetVRGripInterfaceSettings(bool bMarkDirty)
{
#if WITH_PUSH_MODEL
	if (bMarkDirty && bRepGripSettingsAndGameplayTags)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableSkeletalMeshActor, VRGripInterfaceSettings, this);
	}
#endif

	return VRGripInterfaceSettings;
}

FGameplayTagContainer& AGrippableSkeletalMeshActor::GetGameplayTags()
{
#if WITH_PUSH_MODEL
	if (bRepGripSettingsAndGameplayTags)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableSkeletalMeshActor, GameplayTags, this);
	}
#endif

	return GameplayTags;
}

