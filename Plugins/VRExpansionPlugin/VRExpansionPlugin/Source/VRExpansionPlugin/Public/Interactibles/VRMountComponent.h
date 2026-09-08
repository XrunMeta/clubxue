
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Interactibles/VRInteractibleFunctionLibrary.h"
#include "VRGripInterface.h"
#include "VRMountComponent.generated.h"

class UGripMotionControllerComponent;

UENUM(Blueprintable)
enum class EVRInteractibleMountAxis : uint8
{

	Axis_XZ
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRMountComponent : public UStaticMeshComponent, public IVRGripInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UVRMountComponent(const FObjectInitializer& ObjectInitializer);

	~UVRMountComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMountComponent")
		EVRInteractibleMountAxis MountRotationAxis;

	virtual void OnRegister() override;

	FTransform InitialRelativeTransform;
	FVector InitialInteractorLocation;
	FVector InitialInteractorDropLocation;
	float InitialGripRot;
	FQuat qRotAtGrab;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMountComponent")
		float FlipingZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMountComponent")
		float FlipReajustYawSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		int GripPriority;

	UFUNCTION(BlueprintCallable, Category = "GripSettings")
		void SetGripPriority(int NewGripPriority);

	bool GrippedOnBack;

	bool bIsInsideFrontFlipingZone;
	bool bIsInsideBackFlipZone;
	FVector CurInterpGripLoc;

	float TwistDiff;
	FVector InitialGripToForwardVec;
	FVector InitialForwardVector;
	FVector EntryUpXYNeg;
	FVector EntryUpVec;
	FVector EntryRightVec;

	bool bFirstEntryToHalfFlipZone;
	bool bLerpingOutOfFlipZone;
	bool bIsFlipped;

	FPlane FlipPlane;
	FPlane ForwardPullPlane;
	FVector LastPointOnForwardPlane;
	FVector CurPointOnForwardPlane;

	float LerpOutAlpha;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer = GameplayTags;
	}

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "GameplayTags")
		FGameplayTagContainer GameplayTags;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface")
		bool bRepGameplayTags;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface|Replication")
		bool bReplicateMovement;
public:
	FGameplayTagContainer& GetGameplayTags();
	bool GetRepGameplayTags() { return bRepGameplayTags; }
	void SetRepGameplayTags(bool bNewRepGameplayTags);
	bool GetReplicateMovement() { return bReplicateMovement; }
	void SetReplicateMovement(bool bNewReplicateMovement);

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		EGripMovementReplicationSettings MovementReplicationSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float Stiffness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float Damping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float BreakDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		float PrimarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		float SecondarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface", meta = (ScriptName = "IsDenyGripping"))
		bool bDenyGripping;

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface", meta = (ScriptName = "IsCurrentlyHeld"))
		bool bIsHeld; 

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface")
		FBPGripPair HoldingGrip; 
	bool bOriginalReplicatesMovement;

	UFUNCTION(BlueprintCallable, Category = "VRMountComponent")
		void ResetInitialMountLocation();

	virtual void OnUnregister() override;;

	bool DenyGripping_Implementation(UGripMotionControllerComponent * GripInitiator = nullptr) override;

	EGripInterfaceTeleportBehavior TeleportBehavior_Implementation() override;

	bool SimulateOnDrop_Implementation() override;

	EGripCollisionType GetPrimaryGripType_Implementation(bool bIsSlot) override;

	ESecondaryGripType SecondaryGripType_Implementation() override;

	EGripMovementReplicationSettings GripMovementReplicationType_Implementation() override;

	EGripLateUpdateSettings GripLateUpdateSetting_Implementation() override;

	void GetGripStiffnessAndDamping_Implementation(float& GripStiffnessOut, float& GripDampingOut) override;

	FBPAdvGripSettings AdvancedGripSettings_Implementation() override;

	float GripBreakDistance_Implementation() override;

	void ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool& bHadSlotInRange, FTransform& SlotWorldTransform, FName& SlotName, UGripMotionControllerComponent* CallingController = nullptr, FName OverridePrefix = NAME_None) override;

	bool AllowsMultipleGrips_Implementation() override;

	void IsHeld_Implementation(TArray<FBPGripPair>& CurHoldingControllers, bool& bCurIsHeld) override;

	void SetHeld_Implementation(UGripMotionControllerComponent* NewHoldingController, uint8 GripID, bool bNewIsHeld) override;

	bool RequestsSocketing_Implementation(USceneComponent*& ParentToSocketTo, FName& OptionalSocketName, FTransform_NetQuantize& RelativeTransform) override;

	bool GetGripScripts_Implementation(TArray<UVRGripScriptBase*>& ArrayReference) override;

	void TickGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation, float DeltaTime) override;

	void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;

	void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;

	void OnChildGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;

	void OnChildGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;

	void OnSecondaryGrip_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* SecondaryGripComponent, const FBPActorGripInformation& GripInformation) override;

	void OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* ReleasingSecondaryGripComponent, const FBPActorGripInformation& GripInformation) override;

	void OnUsed_Implementation() override;

	void OnEndUsed_Implementation() override;

	void OnSecondaryUsed_Implementation() override;

	void OnEndSecondaryUsed_Implementation() override;

	void OnInput_Implementation(FKey Key, EInputEvent KeyEvent) override;

protected:

};

