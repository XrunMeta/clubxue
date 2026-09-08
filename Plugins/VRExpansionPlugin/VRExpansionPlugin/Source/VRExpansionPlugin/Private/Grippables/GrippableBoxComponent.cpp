

#include "Grippables/GrippableBoxComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GrippableBoxComponent)

#include "GripMotionControllerComponent.h"
#include "VRExpansionFunctionLibrary.h"
#include "GripScripts/VRGripScriptBase.h"
#include "Net/UnrealNetwork.h"

#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

UGrippableBoxComponent::~UGrippableBoxComponent()
{
}

UGrippableBoxComponent::UGrippableBoxComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VRGripInterfaceSettings.bDenyGripping = false;
	VRGripInterfaceSettings.OnTeleportBehavior = EGripInterfaceTeleportBehavior::DropOnTeleport;
	VRGripInterfaceSettings.bSimulateOnDrop = true;
	VRGripInterfaceSettings.SlotDefaultGripType = EGripCollisionType::ManipulationGrip;
	VRGripInterfaceSettings.FreeDefaultGripType = EGripCollisionType::ManipulationGrip;
	VRGripInterfaceSettings.SecondaryGripType = ESecondaryGripType::SG_None;
	VRGripInterfaceSettings.MovementReplicationType = EGripMovementReplicationSettings::ForceClientSideMovement;
	VRGripInterfaceSettings.LateUpdateSetting = EGripLateUpdateSettings::LateUpdatesAlwaysOff;
	VRGripInterfaceSettings.ConstraintStiffness = 1500.0f;
	VRGripInterfaceSettings.ConstraintDamping = 200.0f;
	VRGripInterfaceSettings.ConstraintBreakDistance = 100.0f;
	VRGripInterfaceSettings.SecondarySlotRange = 20.0f;
	VRGripInterfaceSettings.PrimarySlotRange = 20.0f;
	VRGripInterfaceSettings.bIsHeld = false;

	bReplicateMovement = false;

	bRepGripSettingsAndGameplayTags = true;
	bReplicateGripScripts = false;

	bReplicateUsingRegisteredSubObjectList = true;
}

void UGrippableBoxComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	 Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	 FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	 DOREPLIFETIME_WITH_PARAMS_FAST(UGrippableBoxComponent, bReplicateGripScripts, PushModelParams);
	 DOREPLIFETIME_WITH_PARAMS_FAST(UGrippableBoxComponent, bRepGripSettingsAndGameplayTags, PushModelParams);
	 DOREPLIFETIME_WITH_PARAMS_FAST(UGrippableBoxComponent, bReplicateMovement, PushModelParams);

	 FDoRepLifetimeParams PushModelParamsWithCondition{ COND_Custom, REPNOTIFY_OnChanged, true };

	 DOREPLIFETIME_WITH_PARAMS_FAST(UGrippableBoxComponent, GripLogicScripts, PushModelParamsWithCondition);
	 DOREPLIFETIME_WITH_PARAMS_FAST(UGrippableBoxComponent, VRGripInterfaceSettings, PushModelParamsWithCondition);
	 DOREPLIFETIME_WITH_PARAMS_FAST(UGrippableBoxComponent, GameplayTags, PushModelParamsWithCondition);
}

void UGrippableBoxComponent::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(UGrippableBoxComponent, VRGripInterfaceSettings, bRepGripSettingsAndGameplayTags);
	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(UGrippableBoxComponent, GameplayTags, bRepGripSettingsAndGameplayTags);
	DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(UGrippableBoxComponent, GripLogicScripts, bReplicateGripScripts);

	if (!IRISNetReplication::IsIris(this))
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeLocation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeRotation, bReplicateMovement);
		DOREPLIFETIME_ACTIVE_OVERRIDE_FAST(USceneComponent, RelativeScale3D, bReplicateMovement);
	}
}

bool UGrippableBoxComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
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

void UGrippableBoxComponent::BeginPlay()
{

	Super::BeginPlay();

	for (UVRGripScriptBase* Script : GripLogicScripts)
	{
		if (Script)
		{
			Script->BeginPlay(this);
		}
	}

	bOriginalReplicatesMovement = bReplicateMovement;
}

void UGrippableBoxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	Super::EndPlay(EndPlayReason);

	for (UVRGripScriptBase* Script : GripLogicScripts)
	{
		if (Script)
		{
			Script->EndPlay(EndPlayReason);
		}
	}
}

void UGrippableBoxComponent::SetDenyGripping(bool bDenyGripping)
{
	VRGripInterfaceSettings.bDenyGripping = bDenyGripping;
}

void UGrippableBoxComponent::SetGripPriority(int NewGripPriority)
{
	VRGripInterfaceSettings.AdvancedGripSettings.GripPriority = NewGripPriority;
}

void UGrippableBoxComponent::TickGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation, float DeltaTime) {}
void UGrippableBoxComponent::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) { }
void UGrippableBoxComponent::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) { }
void UGrippableBoxComponent::OnChildGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) {}
void UGrippableBoxComponent::OnChildGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) {}
void UGrippableBoxComponent::OnSecondaryGrip_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* SecondaryGripComponent, const FBPActorGripInformation& GripInformation) { OnSecondaryGripAdded.Broadcast(GripOwningController, GripInformation); }
void UGrippableBoxComponent::OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* ReleasingSecondaryGripComponent, const FBPActorGripInformation& GripInformation) { OnSecondaryGripRemoved.Broadcast(GripOwningController, GripInformation); }
void UGrippableBoxComponent::OnUsed_Implementation() {}
void UGrippableBoxComponent::OnEndUsed_Implementation() {}
void UGrippableBoxComponent::OnSecondaryUsed_Implementation() {}
void UGrippableBoxComponent::OnEndSecondaryUsed_Implementation() {}
void UGrippableBoxComponent::OnInput_Implementation(FKey Key, EInputEvent KeyEvent) {}
bool UGrippableBoxComponent::RequestsSocketing_Implementation(USceneComponent *& ParentToSocketTo, FName & OptionalSocketName, FTransform_NetQuantize & RelativeTransform) { return false; }

bool UGrippableBoxComponent::DenyGripping_Implementation(UGripMotionControllerComponent * GripInitiator)
{
	return VRGripInterfaceSettings.bDenyGripping;
}

EGripInterfaceTeleportBehavior UGrippableBoxComponent::TeleportBehavior_Implementation()
{
	return VRGripInterfaceSettings.OnTeleportBehavior;
}

bool UGrippableBoxComponent::SimulateOnDrop_Implementation()
{
	return VRGripInterfaceSettings.bSimulateOnDrop;
}

EGripCollisionType UGrippableBoxComponent::GetPrimaryGripType_Implementation(bool bIsSlot)
{
	return bIsSlot ? VRGripInterfaceSettings.SlotDefaultGripType : VRGripInterfaceSettings.FreeDefaultGripType;
}

ESecondaryGripType UGrippableBoxComponent::SecondaryGripType_Implementation()
{
	return VRGripInterfaceSettings.SecondaryGripType;
}

EGripMovementReplicationSettings UGrippableBoxComponent::GripMovementReplicationType_Implementation()
{
	return VRGripInterfaceSettings.MovementReplicationType;
}

EGripLateUpdateSettings UGrippableBoxComponent::GripLateUpdateSetting_Implementation()
{
	return VRGripInterfaceSettings.LateUpdateSetting;
}

void UGrippableBoxComponent::GetGripStiffnessAndDamping_Implementation(float &GripStiffnessOut, float &GripDampingOut)
{
	GripStiffnessOut = VRGripInterfaceSettings.ConstraintStiffness;
	GripDampingOut = VRGripInterfaceSettings.ConstraintDamping;
}

FBPAdvGripSettings UGrippableBoxComponent::AdvancedGripSettings_Implementation()
{
	return VRGripInterfaceSettings.AdvancedGripSettings;
}

float UGrippableBoxComponent::GripBreakDistance_Implementation()
{
	return VRGripInterfaceSettings.ConstraintBreakDistance;
}

void UGrippableBoxComponent::ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool & bHadSlotInRange, FTransform & SlotWorldTransform, FName & SlotName, UGripMotionControllerComponent * CallingController, FName OverridePrefix)
{
	if (OverridePrefix.IsNone())
		bSecondarySlot ? OverridePrefix = "VRGripS" : OverridePrefix = "VRGripP";

	UVRExpansionFunctionLibrary::GetGripSlotInRangeByTypeName_Component(OverridePrefix, this, WorldLocation, bSecondarySlot ? VRGripInterfaceSettings.SecondarySlotRange : VRGripInterfaceSettings.PrimarySlotRange, bHadSlotInRange, SlotWorldTransform, SlotName, CallingController);
}

bool UGrippableBoxComponent::AllowsMultipleGrips_Implementation()
{
	return VRGripInterfaceSettings.bAllowMultipleGrips;
}

void UGrippableBoxComponent::IsHeld_Implementation(TArray<FBPGripPair> & HoldingControllers, bool & bIsHeld)
{
	HoldingControllers = VRGripInterfaceSettings.HoldingControllers;
	bIsHeld = VRGripInterfaceSettings.bIsHeld;
}

void UGrippableBoxComponent::Native_NotifyThrowGripDelegates(UGripMotionControllerComponent* Controller, bool bGripped, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
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

void UGrippableBoxComponent::SetHeld_Implementation(UGripMotionControllerComponent * HoldingController, uint8 GripID, bool bIsHeld)
{
	if (bIsHeld)
	{
		if (VRGripInterfaceSettings.MovementReplicationType != EGripMovementReplicationSettings::ForceServerSideMovement)
		{
			if (!VRGripInterfaceSettings.bIsHeld)
				bOriginalReplicatesMovement = bReplicateMovement;
			bReplicateMovement = false;
		}

		VRGripInterfaceSettings.bWasHeld = true;
		VRGripInterfaceSettings.HoldingControllers.AddUnique(FBPGripPair(HoldingController, GripID));
	}
	else
	{
		if (VRGripInterfaceSettings.MovementReplicationType != EGripMovementReplicationSettings::ForceServerSideMovement)
		{
			bReplicateMovement = bOriginalReplicatesMovement;
		}

		VRGripInterfaceSettings.HoldingControllers.Remove(FBPGripPair(HoldingController, GripID));
	}

	VRGripInterfaceSettings.bIsHeld = VRGripInterfaceSettings.HoldingControllers.Num() > 0;
}

bool UGrippableBoxComponent::GetGripScripts_Implementation(TArray<UVRGripScriptBase*> & ArrayReference)
{
	ArrayReference = GripLogicScripts;
	return GripLogicScripts.Num() > 0;
}

void UGrippableBoxComponent::PreDestroyFromReplication()
{
	Super::PreDestroyFromReplication();

	for (int32 i = 0; i < GripLogicScripts.Num(); ++i)
	{
		if (UObject *SubObject = GripLogicScripts[i])
		{
			SubObject->PreDestroyFromReplication();
			SubObject->MarkAsGarbage();
		}
	}

	GripLogicScripts.Empty();
}

void UGrippableBoxComponent::GetSubobjectsWithStableNamesForNetworking(TArray<UObject*> &ObjList)
{
	if (bReplicateGripScripts)
	{
		for (int32 i = 0; i < GripLogicScripts.Num(); ++i)
		{
			if (UObject* SubObject = GripLogicScripts[i])
			{
				ObjList.Add(SubObject);
			}
		}
	}
}

void UGrippableBoxComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{

	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (UWorld * World = GetWorld())
	{
		EWorldType::Type WorldType = World->WorldType;
		if (WorldType == EWorldType::Editor || WorldType == EWorldType::EditorPreview)
		{
			return;
		}
	}

	for (int32 i = 0; i < GripLogicScripts.Num(); i++)
	{
		if (UObject *SubObject = GripLogicScripts[i])
		{
			SubObject->MarkAsGarbage();
		}
	}

	GripLogicScripts.Empty();
}

void UGrippableBoxComponent::SetReplicateGripScripts(bool bNewReplicateGripScripts)
{
	bReplicateGripScripts = bNewReplicateGripScripts;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UGrippableBoxComponent, bReplicateGripScripts, this);
#endif
}

TArray<TObjectPtr<UVRGripScriptBase>>& UGrippableBoxComponent::GetGripLogicScripts()
{
#if WITH_PUSH_MODEL
	if (bReplicateGripScripts)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UGrippableBoxComponent, GripLogicScripts, this);
	}
#endif

	return GripLogicScripts;
}

void UGrippableBoxComponent::SetRepGripSettingsAndGameplayTags(bool bNewRepGripSettingsAndGameplayTags)
{
	bRepGripSettingsAndGameplayTags = bNewRepGripSettingsAndGameplayTags;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UGrippableBoxComponent, bRepGripSettingsAndGameplayTags, this);
#endif
}

void UGrippableBoxComponent::SetReplicateMovement(bool bNewReplicateMovement)
{
	bReplicateMovement = bNewReplicateMovement;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UGrippableBoxComponent, bReplicateMovement, this);
#endif
}

FBPInterfaceProperties& UGrippableBoxComponent::GetVRGripInterfaceSettings(bool bMarkDirty)
{
#if WITH_PUSH_MODEL
	if (bMarkDirty && bRepGripSettingsAndGameplayTags)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UGrippableBoxComponent, VRGripInterfaceSettings, this);
	}
#endif

	return VRGripInterfaceSettings;
}

FGameplayTagContainer& UGrippableBoxComponent::GetGameplayTags()
{
#if WITH_PUSH_MODEL
	if (bRepGripSettingsAndGameplayTags)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UGrippableBoxComponent, GameplayTags, this);
	}
#endif

	return GameplayTags;
}

