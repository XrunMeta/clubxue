

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "VRGripInterface.h"
#include "Interactibles/VRInteractibleFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "VRLeverComponent.generated.h"

class UGripMotionControllerComponent;

UENUM(Blueprintable)
enum class EVRInteractibleLeverAxis : uint8
{

	Axis_X,

	Axis_Y,

	Axis_Z,

	Axis_XY,

	FlightStick_XY,
};

UENUM(Blueprintable)
enum class EVRInteractibleLeverEventType : uint8
{
	LeverPositive,
	LeverNegative
};

UENUM(Blueprintable)
enum class EVRInteractibleLeverReturnType : uint8
{

	Stay,

	ReturnToZero,

	LerpToMax,

	LerpToMaxIfOverThreshold,

	RetainMomentum
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FVRLeverStateChangedSignature, bool, LeverStatus, EVRInteractibleLeverEventType, LeverStatusType, float, LeverAngleAtTime, float, FullLeverAngleAtTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRLeverFinishedLerpingSignature, float, FinalAngle);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRLeverComponent : public UStaticMeshComponent, public IVRGripInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UVRLeverComponent(const FObjectInitializer& ObjectInitializer);

	~UVRLeverComponent();

	UPROPERTY(BlueprintAssignable, Category = "VRLeverComponent")
		FVRLeverStateChangedSignature OnLeverStateChanged;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Lever State Changed"))
		void ReceiveLeverStateChanged(bool LeverStatus, EVRInteractibleLeverEventType LeverStatusType, float LeverAngleAtTime, float FullLeverAngleAttime);

	UPROPERTY(BlueprintAssignable, Category = "VRLeverComponent")
		FVRLeverFinishedLerpingSignature OnLeverFinishedLerping;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Lever Finished Lerping"))
		void ReceiveLeverFinishedLerping(float LeverFinalAngle);

	UPROPERTY(BlueprintReadOnly, Category = "VRLeverComponent")
		float CurrentLeverAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent")
		FRotator AllCurrentLeverAngles;

	UPROPERTY(BlueprintReadOnly, Category = "VRLeverComponent")
		FVector CurrentLeverForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent")
		bool bUngripAtTargetRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent")
		EVRInteractibleLeverAxis LeverRotationAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent", meta = (ClampMin = "0.01", ClampMax = "1.0", UIMin = "0.01", UIMax = "1.0"))
		float LeverTogglePercentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent", meta = (ClampMin = "0.0", ClampMax = "179.9", UIMin = "0.0", UIMax = "180.0"))
		float LeverLimitPositive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent", meta = (ClampMin = "0.0", ClampMax = "179.9", UIMin = "0.0", UIMax = "180.0"))
		float LeverLimitNegative;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent|Flight Stick Settings", meta = (ClampMin = "0.0", ClampMax = "179.9", UIMin = "0.0", UIMax = "180.0"))
		float FlightStickYawLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent")
		bool bIsLocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent")
		bool bAutoDropWhenLocked;

	UFUNCTION(BlueprintCallable, Category = "GripSettings")
		void SetIsLocked(bool bNewLockedState);

	bool CheckAutoDrop(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent")
		EVRInteractibleLeverReturnType LeverReturnTypeWhenReleased;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent")
		bool bSendLeverEventsDuringLerp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent")
		float LeverReturnSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent|Flight Stick Settings")
		bool bBlendAxisValuesByAngleThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent|Flight Stick Settings", meta = (ClampMin = "1.0", ClampMax = "360.0", UIMin = "1.0", UIMax = "360.0"))
		float AngleThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent|Momentum Settings", meta = (ClampMin = "0", ClampMax = "12", UIMin = "0", UIMax = "12"))
		int FramesToAverage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent|Momentum Settings", meta = (ClampMin = "0.0", ClampMax = "180", UIMin = "0.0", UIMax = "180.0"))
		float LeverMomentumFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent|Momentum Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float LeverRestitution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRLeverComponent|Momentum Settings", meta = (ClampMin = "0.0", UIMin = "0.0"))
		float MaxLeverMomentum;

	UPROPERTY(BlueprintReadWrite, Category = "VRLeverComponent")
		bool bIsLerping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		float PrimarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		float SecondarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		int GripPriority;

	UFUNCTION(BlueprintCallable, Category = "GripSettings")
		void SetGripPriority(int NewGripPriority);

	float FullCurrentAngle;

	float LastDeltaAngle;

	virtual void OnRegister() override;

protected:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InitialRelativeTransform, Category = "VRLeverComponent")
	FTransform_NetQuantize InitialRelativeTransform;
public:

	FTransform GetInitialRelativeTransform() { return InitialRelativeTransform; }

	UFUNCTION()
	virtual void OnRep_InitialRelativeTransform()
	{
		ReCalculateCurrentAngle();
	}

	FTransform InteractorOffsetTransform;

	FVector InitialInteractorLocation;
	FVector InitialInteractorDropLocation;
	float InitialGripRot;
	float RotAtGrab;
	FQuat qRotAtGrab;
	bool bLeverState;
	bool bIsInFirstTick;

	float MomentumAtDrop;
	float LastLeverAngle;

	float CalcAngle(EVRInteractibleLeverAxis AxisToCalc, FVector CurInteractorLocation, bool bSkipLimits = false);

	void LerpAxis(float CurrentAngle, float DeltaTime);

	void CalculateCurrentAngle(FTransform & CurrentTransform);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface", meta = (ScriptName = "IsDenyGripping"))
		bool bDenyGripping;

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface", meta = (ScriptName = "IsCurrentlyHeld"))
		bool bIsHeld; 

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface")
		FBPGripPair HoldingGrip; 
	bool bOriginalReplicatesMovement;

	TWeakObjectPtr<USceneComponent> ParentComponent;

	UFUNCTION(BlueprintCallable, Category = "VRLeverComponent")
		void ResetInitialLeverLocation(bool bAllowThrowingEvents = true);

	UFUNCTION(BlueprintCallable, Category = "VRLeverComponent")
		void SetLeverAngle(float NewAngle, FVector DualAxisForwardVector, bool bAllowThrowingEvents = true);

	UFUNCTION(BlueprintCallable, Category = "VRLeverComponent")
		float ReCalculateCurrentAngle(bool bAllowThrowingEvents = true);

	void ProccessCurrentState(bool bWasLerping = false, bool bThrowEvents = true, bool bCheckAutoDrop = true);

	virtual void OnUnregister() override;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnGripSignature OnGripped;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnDropSignature OnDropped;

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

	virtual void Native_NotifyThrowGripDelegates(UGripMotionControllerComponent* Controller, bool bGripped, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;

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