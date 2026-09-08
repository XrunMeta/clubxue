

#include "Grippables/GrippableStaticMeshActor.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GrippableStaticMeshActor)

#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GripMotionControllerComponent.h"
#include "VRExpansionFunctionLibrary.h"
#include "Misc/BucketUpdateSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsReplication.h"
#include "GripScripts/VRGripScriptBase.h"
#include "DrawDebugHelpers.h"
#include "Physics/Experimental/PhysScene_Chaos.h"

#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

UOptionalRepStaticMeshComponent::UOptionalRepStaticMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicateMovement = true;
}

void UOptionalRepStaticMeshComponent::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (!IRISNetReplication::IsIris(this))
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeLocation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeRotation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeScale3D, bReplicateMovement);
	}

}

void UOptionalRepStaticMeshComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOptionalRepStaticMeshComponent, bReplicateMovement);
}

AGrippableStaticMeshActor::AGrippableStaticMeshActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UOptionalRepStaticMeshComponent>(TEXT("StaticMeshComponent0")))
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

	this->SetMobility(EComponentMobility::Movable);

	SetReplicatingMovement(true);
	this->bReplicates = true;

	bRepGripSettingsAndGameplayTags = true;
	bReplicateGripScripts = false;

	bReplicateUsingRegisteredSubObjectList = true;

	bAllowIgnoringAttachOnOwner = true;

	SetMinNetUpdateFrequency(30.0f);
}

void AGrippableStaticMeshActor::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableStaticMeshActor, bReplicateGripScripts, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableStaticMeshActor, bRepGripSettingsAndGameplayTags, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableStaticMeshActor, bAllowIgnoringAttachOnOwner, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableStaticMeshActor, ClientAuthReplicationData, PushModelParams);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_Custom, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableStaticMeshActor, GripLogicScripts, PushModelParamsWithCondition);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableStaticMeshActor, VRGripInterfaceSettings, PushModelParamsWithCondition);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableStaticMeshActor, GameplayTags, PushModelParamsWithCondition);

	DISABLE_REPLICATED_PRIVATE_PROPERTY(AActor, AttachmentReplication);

	FDoRepLifetimeParams AttachmentReplicationParams{ COND_Custom, REPNOTIFY_Always, true };
	DOREPLIFETIME_WITH_PARAMS_FAST(AGrippableStaticMeshActor, AttachmentWeldReplication, AttachmentReplicationParams);
}

void AGrippableStaticMeshActor::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{

	if (!IRISNetReplication::IsIris(this))
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(AGrippableStaticMeshActor, VRGripInterfaceSettings, bRepGripSettingsAndGameplayTags);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(AGrippableStaticMeshActor, GameplayTags, bRepGripSettingsAndGameplayTags);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(AGrippableStaticMeshActor, GripLogicScripts, bReplicateGripScripts);
	}

#if WITH_PUSH_MODEL
	const AActor* const OldAttachParent = AttachmentWeldReplication.AttachParent;
	const UActorComponent* const OldAttachComponent = AttachmentWeldReplication.AttachComponent;
#endif

	AttachmentWeldReplication.AttachParent = nullptr;
	AttachmentWeldReplication.AttachComponent = nullptr;

	GatherCurrentMovement();

	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(AActor, ReplicatedMovement, IsReplicatingMovement());

	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(AGrippableStaticMeshActor, AttachmentWeldReplication, RootComponent && !RootComponent->GetIsReplicated());

#if WITH_PUSH_MODEL
	if (UNLIKELY(OldAttachParent != AttachmentWeldReplication.AttachParent || OldAttachComponent != AttachmentWeldReplication.AttachComponent))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableStaticMeshActor, AttachmentWeldReplication, this);
	}
#endif

}

void AGrippableStaticMeshActor::GatherCurrentMovement()
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
				MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableStaticMeshActor, AttachmentWeldReplication, this);
#endif
			}
		}
	}
}

bool AGrippableStaticMeshActor::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (bReplicateGripScripts && !IsUsingRegisteredSubObjectList())
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

AGrippableStaticMeshActor::~AGrippableStaticMeshActor()
{
}

void AGrippableStaticMeshActor::BeginPlay()
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

void AGrippableStaticMeshActor::SetDenyGripping(bool bDenyGripping)
{
	VRGripInterfaceSettings.bDenyGripping = bDenyGripping;
}

void AGrippableStaticMeshActor::SetGripPriority(int NewGripPriority)
{
	VRGripInterfaceSettings.AdvancedGripSettings.GripPriority = NewGripPriority;
}

void AGrippableStaticMeshActor::TickGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation, float DeltaTime) {}
void AGrippableStaticMeshActor::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) { }
void AGrippableStaticMeshActor::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) { }
void AGrippableStaticMeshActor::OnChildGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) {}
void AGrippableStaticMeshActor::OnChildGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) {}
void AGrippableStaticMeshActor::OnSecondaryGrip_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* SecondaryGripComponent, const FBPActorGripInformation& GripInformation) { OnSecondaryGripAdded.Broadcast(GripOwningController, GripInformation); }
void AGrippableStaticMeshActor::OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* ReleasingSecondaryGripComponent, const FBPActorGripInformation& GripInformation) { OnSecondaryGripRemoved.Broadcast(GripOwningController, GripInformation); }
void AGrippableStaticMeshActor::OnUsed_Implementation() {}
void AGrippableStaticMeshActor::OnEndUsed_Implementation() {}
void AGrippableStaticMeshActor::OnSecondaryUsed_Implementation() {}
void AGrippableStaticMeshActor::OnEndSecondaryUsed_Implementation() {}
void AGrippableStaticMeshActor::OnInput_Implementation(FKey Key, EInputEvent KeyEvent) {}
bool AGrippableStaticMeshActor::RequestsSocketing_Implementation(USceneComponent *& ParentToSocketTo, FName & OptionalSocketName, FTransform_NetQuantize & RelativeTransform) { return false; }

bool AGrippableStaticMeshActor::DenyGripping_Implementation(UGripMotionControllerComponent * GripInitiator)
{
	return VRGripInterfaceSettings.bDenyGripping;
}

EGripInterfaceTeleportBehavior AGrippableStaticMeshActor::TeleportBehavior_Implementation()
{
	return VRGripInterfaceSettings.OnTeleportBehavior;
}

bool AGrippableStaticMeshActor::SimulateOnDrop_Implementation()
{
	return VRGripInterfaceSettings.bSimulateOnDrop;
}

EGripCollisionType AGrippableStaticMeshActor::GetPrimaryGripType_Implementation(bool bIsSlot)
{
	return bIsSlot ? VRGripInterfaceSettings.SlotDefaultGripType : VRGripInterfaceSettings.FreeDefaultGripType;
}

ESecondaryGripType AGrippableStaticMeshActor::SecondaryGripType_Implementation()
{
	return VRGripInterfaceSettings.SecondaryGripType;
}

EGripMovementReplicationSettings AGrippableStaticMeshActor::GripMovementReplicationType_Implementation()
{
	return VRGripInterfaceSettings.MovementReplicationType;
}

EGripLateUpdateSettings AGrippableStaticMeshActor::GripLateUpdateSetting_Implementation()
{
	return VRGripInterfaceSettings.LateUpdateSetting;
}

void AGrippableStaticMeshActor::GetGripStiffnessAndDamping_Implementation(float &GripStiffnessOut, float &GripDampingOut)
{
	GripStiffnessOut = VRGripInterfaceSettings.ConstraintStiffness;
	GripDampingOut = VRGripInterfaceSettings.ConstraintDamping;
}

FBPAdvGripSettings AGrippableStaticMeshActor::AdvancedGripSettings_Implementation()
{
	return VRGripInterfaceSettings.AdvancedGripSettings;
}

float AGrippableStaticMeshActor::GripBreakDistance_Implementation()
{
	return VRGripInterfaceSettings.ConstraintBreakDistance;
}

void AGrippableStaticMeshActor::ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool & bHadSlotInRange, FTransform & SlotWorldTransform, FName & SlotName, UGripMotionControllerComponent * CallingController, FName OverridePrefix)
{
	if (OverridePrefix.IsNone())
		bSecondarySlot ? OverridePrefix = "VRGripS" : OverridePrefix = "VRGripP";

	UVRExpansionFunctionLibrary::GetGripSlotInRangeByTypeName(OverridePrefix, this, WorldLocation, bSecondarySlot ? VRGripInterfaceSettings.SecondarySlotRange : VRGripInterfaceSettings.PrimarySlotRange, bHadSlotInRange, SlotWorldTransform, SlotName, CallingController);
}

bool AGrippableStaticMeshActor::AllowsMultipleGrips_Implementation()
{
	return VRGripInterfaceSettings.bAllowMultipleGrips;
}

void AGrippableStaticMeshActor::IsHeld_Implementation(TArray<FBPGripPair> & HoldingControllers, bool & bIsHeld)
{
	HoldingControllers = VRGripInterfaceSettings.HoldingControllers;
	bIsHeld = VRGripInterfaceSettings.bIsHeld;
}

bool AGrippableStaticMeshActor::AddToClientReplicationBucket()
{
	if (ShouldWeSkipAttachmentReplication(false))
	{

		GetWorld()->GetSubsystem<UBucketUpdateSubsystem>()->AddObjectToBucket(ClientAuthReplicationData.UpdateRate, this, FName(TEXT("PollReplicationEvent")));
		ClientAuthReplicationData.bIsCurrentlyClientAuth = true;

		if (UWorld * World = GetWorld())
			ClientAuthReplicationData.TimeAtInitialThrow = World->GetTimeSeconds();

		return true;
	}

	return false;
}

bool AGrippableStaticMeshActor::RemoveFromClientReplicationBucket()
{
	if (ClientAuthReplicationData.bIsCurrentlyClientAuth)
	{
		GetWorld()->GetSubsystem<UBucketUpdateSubsystem>()->RemoveObjectFromBucketByFunctionName(this, FName(TEXT("PollReplicationEvent")));
		CeaseReplicationBlocking();
		return true;
	}

	return false;
}

void AGrippableStaticMeshActor::Native_NotifyThrowGripDelegates(UGripMotionControllerComponent* Controller, bool bGripped, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
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

void AGrippableStaticMeshActor::SetHeld_Implementation(UGripMotionControllerComponent* HoldingController, uint8 GripID, bool bIsHeld)
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

bool AGrippableStaticMeshActor::GetGripScripts_Implementation(TArray<UVRGripScriptBase*> & ArrayReference)
{
	ArrayReference = GripLogicScripts;
	return GripLogicScripts.Num() > 0;
}

bool AGrippableStaticMeshActor::PollReplicationEvent()
{
	if (!ClientAuthReplicationData.bIsCurrentlyClientAuth || !this->HasLocalNetOwner() || VRGripInterfaceSettings.bIsHeld)
		return false; 

	UWorld *OurWorld = GetWorld();
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

			if (UPrimitiveComponent * PrimComp = Cast<UPrimitiveComponent>(RootComponent))
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
		AActor * tempOwner = TopOwner->GetOwner();

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
				OurWorld->GetTimerManager().SetTimer(ClientAuthReplicationData.ResetReplicationHandle, this, &AGrippableStaticMeshActor::CeaseReplicationBlocking, clampedPing, false);
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

void AGrippableStaticMeshActor::CeaseReplicationBlocking()
{
	if(ClientAuthReplicationData.bIsCurrentlyClientAuth)
		ClientAuthReplicationData.bIsCurrentlyClientAuth = false;

	ClientAuthReplicationData.LastActorTransform = FTransform::Identity;

	if (ClientAuthReplicationData.ResetReplicationHandle.IsValid())
	{
		if (UWorld * OurWorld = GetWorld())
		{
			OurWorld->GetTimerManager().ClearTimer(ClientAuthReplicationData.ResetReplicationHandle);
		}
	}
}

void AGrippableStaticMeshActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

bool AGrippableStaticMeshActor::Server_EndClientAuthReplication_Validate()
{
	return true;
}

void AGrippableStaticMeshActor::Server_EndClientAuthReplication_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (FPhysScene* PhysScene = World->GetPhysicsScene())
		{
			if (IPhysicsReplication* PhysicsReplication = PhysScene->GetPhysicsReplication())
			{
				PhysicsReplication->RemoveReplicatedTarget(this->GetStaticMeshComponent());
			}
		}
	}
}

bool AGrippableStaticMeshActor::Server_GetClientAuthReplication_Validate(const FRepMovementVR & newMovement)
{
	return true;
}

void AGrippableStaticMeshActor::Server_GetClientAuthReplication_Implementation(const FRepMovementVR & newMovement)
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

bool AGrippableStaticMeshActor::ShouldWeSkipAttachmentReplication(bool bConsiderHeld) const
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

void AGrippableStaticMeshActor::OnRep_AttachmentReplication()
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

void AGrippableStaticMeshActor::OnRep_ReplicateMovement()
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

void AGrippableStaticMeshActor::OnRep_ReplicatedMovement()
{
	if (bAllowIgnoringAttachOnOwner && (ClientAuthReplicationData.bIsCurrentlyClientAuth || ShouldWeSkipAttachmentReplication()))

	{
		return;
	}

	Super::OnRep_ReplicatedMovement();
}

void AGrippableStaticMeshActor::PostNetReceivePhysicState()
{
	if (bAllowIgnoringAttachOnOwner && (ClientAuthReplicationData.bIsCurrentlyClientAuth || ShouldWeSkipAttachmentReplication()))

	{
		return;
	}

	Super::PostNetReceivePhysicState();
}

void AGrippableStaticMeshActor::MarkComponentsAsGarbage(bool bModify)
{
	Super::MarkComponentsAsGarbage(bModify);

	for (int32 i = 0; i < GripLogicScripts.Num(); ++i)
	{
		if (UObject *SubObject = GripLogicScripts[i])
		{
			SubObject->MarkAsGarbage();
		}
	}

	GripLogicScripts.Empty();
}

void AGrippableStaticMeshActor::PreDestroyFromReplication()
{
	Super::PreDestroyFromReplication();

	for (int32 i = 0; i < GripLogicScripts.Num(); ++i)
	{
		if (UObject *SubObject = GripLogicScripts[i])
		{
			OnSubobjectDestroyFromReplication(SubObject); 
			SubObject->PreDestroyFromReplication();
			SubObject->MarkAsGarbage();
		}
	}

	for (UActorComponent * ActorComp : GetComponents())
	{

		if (ActorComp && IsValid(ActorComp) && ActorComp->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			ActorComp->PreDestroyFromReplication();
	}

	GripLogicScripts.Empty();
}

void AGrippableStaticMeshActor::BeginDestroy()
{
	Super::BeginDestroy();

	for (int32 i = 0; i < GripLogicScripts.Num(); i++)
	{
		if (UObject *SubObject = GripLogicScripts[i])
		{
			SubObject->MarkAsGarbage();
		}
	}

	GripLogicScripts.Empty();
}

void AGrippableStaticMeshActor::SetReplicateGripScripts(bool bNewReplicateGripScripts)
{
	bReplicateGripScripts = bNewReplicateGripScripts;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableStaticMeshActor, bReplicateGripScripts, this);
#endif
}

TArray<TObjectPtr<UVRGripScriptBase>>& AGrippableStaticMeshActor::GetGripLogicScripts()
{
#if WITH_PUSH_MODEL
	if (bReplicateGripScripts)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableStaticMeshActor, GripLogicScripts, this);
	}
#endif

	return GripLogicScripts;
}

void AGrippableStaticMeshActor::SetRepGripSettingsAndGameplayTags(bool bNewRepGripSettingsAndGameplayTags)
{
	bRepGripSettingsAndGameplayTags = bNewRepGripSettingsAndGameplayTags;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableStaticMeshActor, bRepGripSettingsAndGameplayTags, this);
#endif
}

void AGrippableStaticMeshActor::SetAllowIgnoringAttachOnOwner(bool bNewAllowIgnoringAttachOnOwner)
{
	bAllowIgnoringAttachOnOwner = bNewAllowIgnoringAttachOnOwner;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableStaticMeshActor, bAllowIgnoringAttachOnOwner, this);
#endif
}

FVRClientAuthReplicationData& AGrippableStaticMeshActor::GetClientAuthReplicationData(FVRClientAuthReplicationData& ClientAuthData)
{
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableStaticMeshActor, ClientAuthReplicationData, this);
#endif
	return ClientAuthReplicationData;
}

FBPInterfaceProperties& AGrippableStaticMeshActor::GetVRGripInterfaceSettings(bool bMarkDirty)
{
#if WITH_PUSH_MODEL
	if (bMarkDirty && bRepGripSettingsAndGameplayTags)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableStaticMeshActor, VRGripInterfaceSettings, this);
	}
#endif

	return VRGripInterfaceSettings;
}

FGameplayTagContainer& AGrippableStaticMeshActor::GetGameplayTags()
{
#if WITH_PUSH_MODEL
	if (bRepGripSettingsAndGameplayTags)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AGrippableStaticMeshActor, GameplayTags, this);
	}
#endif

	return GameplayTags;
}

