

#pragma once

#include "CoreMinimal.h"
#include "VRBPDatatypes.h"
#include "VRGripInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Components/CapsuleComponent.h"
#include "Engine/ActorChannel.h"
#include "GrippableCapsuleComponent.generated.h"

class UVRGripScriptBase;
class UGripMotionControllerComponent;

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent, ChildCanTick), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UGrippableCapsuleComponent : public UCapsuleComponent, public IVRGripInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UGrippableCapsuleComponent(const FObjectInitializer& ObjectInitializer);

	~UGrippableCapsuleComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	virtual void PreDestroyFromReplication() override;

	virtual void GetSubobjectsWithStableNamesForNetworking(TArray<UObject*> &ObjList) override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface|Replication")
		bool bRepGripSettingsAndGameplayTags;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface|Replication")
		bool bReplicateMovement;

	bool bOriginalReplicatesMovement;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface")
		FBPInterfaceProperties VRGripInterfaceSettings;

public:

	void SetRepGripSettingsAndGameplayTags(bool bNewRepGripSettingsAndGameplayTags);
	inline bool GetRepGripSettingsAndGameplayTags() { return bRepGripSettingsAndGameplayTags; };

	FBPInterfaceProperties& GetVRGripInterfaceSettings(bool bMarkDirty);

	void SetReplicateMovement(bool bNewReplicateMovement);
	inline bool GetReplicateMovement() { return bReplicateMovement; };

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