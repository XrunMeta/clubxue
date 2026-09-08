

#pragma once

#include "CoreMinimal.h"

#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "VRBPDatatypes.h"
#include "VRGripInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Engine/ActorChannel.h"
#include "Grippables/GrippableDataTypes.h"
#include "Grippables/GrippablePhysicsReplication.h"
#include "GrippableStaticMeshActor.generated.h"

class UGripMotionControllerComponent;
class UVRGripScriptBase;

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent,ChildCanTick), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UOptionalRepStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UOptionalRepStaticMeshComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Component Replication")
		bool bReplicateMovement;

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent, ChildCanTick), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API AGrippableStaticMeshActor : public AStaticMeshActor, public IVRGripInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	AGrippableStaticMeshActor(const FObjectInitializer& ObjectInitializer);

	~AGrippableStaticMeshActor();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_AttachmentReplication)
		FRepAttachmentWithWeld AttachmentWeldReplication;

	virtual void GatherCurrentMovement() override;

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly, Instanced, Category = "VRGripInterface")
		TArray<TObjectPtr<UVRGripScriptBase>> GripLogicScripts;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface")
		bool bReplicateGripScripts;

public:

	void SetReplicateGripScripts(bool NewReplicateGripScripts);
	inline bool GetReplicateGripScripts() { return bReplicateGripScripts; };

	TArray<TObjectPtr<UVRGripScriptBase>>& GetGripLogicScripts();

	bool ReplicateSubobjects(UActorChannel* Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	UFUNCTION(BlueprintCallable, Category = "VRGripInterface")
		void SetDenyGripping(bool bDenyGripping);

	UFUNCTION(BlueprintCallable, Category = "VRGripInterface")
		void SetGripPriority(int NewGripPriority);

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnGripSignature OnGripped;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnDropSignature OnDropped;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnGripSignature OnSecondaryGripAdded;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnGripSignature OnSecondaryGripRemoved;

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Replication")
		FVRClientAuthReplicationData ClientAuthReplicationData;

public:

	FVRClientAuthReplicationData& GetClientAuthReplicationData(FVRClientAuthReplicationData& ClientAuthData);

	UFUNCTION(BlueprintCallable, Category = "Networking")
		bool AddToClientReplicationBucket();

	UFUNCTION(BlueprintCallable, Category = "Networking")
		bool RemoveFromClientReplicationBucket();

	UFUNCTION()
	bool PollReplicationEvent();

	UFUNCTION(Category = "Networking")
		void CeaseReplicationBlocking();

	UFUNCTION(Reliable, Server, WithValidation, Category = "Networking")
		void Server_EndClientAuthReplication();

	UFUNCTION(UnReliable, Server, WithValidation, Category = "Networking")
		void Server_GetClientAuthReplication(const FRepMovementVR & newMovement);

	UFUNCTION(BlueprintPure, Category = "Networking")
		FORCEINLINE bool IsCurrentlyClientAuthThrowing()
	{
		return ClientAuthReplicationData.bIsCurrentlyClientAuth;
	}

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer = GameplayTags;
	}

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "GameplayTags")
		FGameplayTagContainer GameplayTags;
public:
	FGameplayTagContainer& GetGameplayTags();

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Replication")
		bool bAllowIgnoringAttachOnOwner;

public:
	void SetAllowIgnoringAttachOnOwner(bool bNewAllowIgnoringAttachOnOwner);
	inline bool GetAllowIgnoringAttachOnOwner() { return bAllowIgnoringAttachOnOwner; };

	 bool ShouldWeSkipAttachmentReplication(bool bConsiderHeld = true) const;

	virtual void OnRep_AttachmentReplication() override;
	virtual void OnRep_ReplicateMovement() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void PostNetReceivePhysicState() override;

	virtual void MarkComponentsAsGarbage(bool bModify) override;

	virtual void PreDestroyFromReplication() override;

	virtual void BeginDestroy() override;

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface")
		bool bRepGripSettingsAndGameplayTags;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface")
		FBPInterfaceProperties VRGripInterfaceSettings;

public:

	void SetRepGripSettingsAndGameplayTags(bool bNewRepGripSettingsAndGameplayTags);
	inline bool GetRepGripSettingsAndGameplayTags() { return bRepGripSettingsAndGameplayTags; };

	FBPInterfaceProperties& GetVRGripInterfaceSettings(bool bMarkDirty);

	virtual bool DenyGripping_Implementation(UGripMotionControllerComponent * GripInitiator = nullptr) override;

	virtual EGripInterfaceTeleportBehavior TeleportBehavior_Implementation() override;

	virtual bool SimulateOnDrop_Implementation() override;

	virtual EGripCollisionType GetPrimaryGripType_Implementation(bool bIsSlot) override;

	virtual ESecondaryGripType SecondaryGripType_Implementation() override;

	virtual EGripMovementReplicationSettings GripMovementReplicationType_Implementation() override;

	virtual EGripLateUpdateSettings GripLateUpdateSetting_Implementation() override;

	virtual void GetGripStiffnessAndDamping_Implementation(float& GripStiffnessOut, float& GripDampingOut) override;

	virtual FBPAdvGripSettings AdvancedGripSettings_Implementation() override;

	virtual float GripBreakDistance_Implementation() override;

	virtual  void ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool& bHadSlotInRange, FTransform& SlotWorldTransform, FName& SlotName, UGripMotionControllerComponent* CallingController = nullptr, FName OverridePrefix = NAME_None) override;

	virtual  bool AllowsMultipleGrips_Implementation() override;

	virtual void IsHeld_Implementation(TArray<FBPGripPair>& HoldingControllers, bool& bIsHeld) override;

	virtual void SetHeld_Implementation(UGripMotionControllerComponent* HoldingController, uint8 GripID, bool bIsHeld) override;

	virtual void Native_NotifyThrowGripDelegates(UGripMotionControllerComponent* Controller, bool bGripped, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;

	virtual bool RequestsSocketing_Implementation(USceneComponent*& ParentToSocketTo, FName& OptionalSocketName, FTransform_NetQuantize& RelativeTransform) override;

	virtual bool GetGripScripts_Implementation(TArray<UVRGripScriptBase*>& ArrayReference) override;

	virtual void TickGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation, float DeltaTime) override;

	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;

	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;

	virtual void OnChildGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;

	virtual void OnChildGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;

	virtual void OnSecondaryGrip_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* SecondaryGripComponent, const FBPActorGripInformation& GripInformation) override;

	virtual void OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* ReleasingSecondaryGripComponent, const FBPActorGripInformation& GripInformation) override;

	virtual void OnUsed_Implementation() override;

	virtual void OnEndUsed_Implementation() override;

	virtual void OnSecondaryUsed_Implementation() override;

	virtual void OnEndSecondaryUsed_Implementation() override;

	virtual void OnInput_Implementation(FKey Key, EInputEvent KeyEvent) override;
};