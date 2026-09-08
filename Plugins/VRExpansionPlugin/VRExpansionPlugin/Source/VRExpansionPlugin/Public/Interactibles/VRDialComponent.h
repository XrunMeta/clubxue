

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "VRGripInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Interactibles/VRInteractibleFunctionLibrary.h"
#include "VRDialComponent.generated.h"

class UGripMotionControllerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRDialStateChangedSignature, float, DialMilestoneAngle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVRDialFinishedLerpingSignature);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRDialComponent : public UStaticMeshComponent, public IVRGripInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UVRDialComponent(const FObjectInitializer& ObjectInitializer);

	~UVRDialComponent();

	UPROPERTY(BlueprintAssignable, Category = "VRDialComponent")
		FVRDialStateChangedSignature OnDialHitSnapAngle;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Dial Hit Snap Angle"))
		void ReceiveDialHitSnapAngle(float DialMilestoneAngle);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent|Lerping")
		bool bLerpBackOnRelease;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent|Lerping")
		bool bSendDialEventsDuringLerp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent|Lerping")
		float DialReturnSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "VRDialComponent|Lerping")
		bool bIsLerping;

	UPROPERTY(BlueprintAssignable, Category = "VRDialComponent|Lerping")
		FVRDialFinishedLerpingSignature OnDialFinishedLerping;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Dial Finished Lerping"))
		void ReceiveDialFinishedLerping();

	UPROPERTY(BlueprintReadOnly, Category = "VRDialComponent")
	float CurrentDialAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
	float ClockwiseMaximumDialAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
	float CClockwiseMaximumDialAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
		bool bUseRollover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
	bool bDialUsesAngleSnap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
	bool bDialUseSnapAngleList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (editcondition = "bDialUseSnapAngleList"))
		TArray<float> DialSnapAngleList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (editcondition = "!bDialUseSnapAngleList", ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
	float SnapAngleIncrement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (editcondition = "!bDialUseSnapAngleList", ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
	float SnapAngleThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
	float RotationScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
	EVRInteractibleAxis DialRotationAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
		bool bDialUseDirectHandRotation;

	float LastGripRot;
	float InitialGripRot;
	float InitialRotBackEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (editcondition = "bDialUseDirectHandRotation"))
	EVRInteractibleAxis InteractorRotationAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		float PrimarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		float SecondarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		int GripPriority;

	UFUNCTION(BlueprintCallable, Category = "GripSettings")
		void SetGripPriority(int NewGripPriority);

	virtual void OnRegister() override;

protected:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InitialRelativeTransform, Category = "VRDialComponent")
	FTransform_NetQuantize InitialRelativeTransform;
public:

	FTransform GetInitialRelativeTransform() { return InitialRelativeTransform; }

	UFUNCTION()
		virtual void OnRep_InitialRelativeTransform()
	{
		CalculateDialProgress();
	}

	void CalculateDialProgress();

	FVector InitialInteractorLocation;
	FVector InitialDropLocation;
	float CurRotBackEnd;
	FRotator LastRotation;
	float LastSnapAngle;

	UFUNCTION(BlueprintCallable, Category = "VRDialComponent")
		void ResetInitialDialLocation();

	UFUNCTION(BlueprintCallable, Category = "VRDialComponent")
		void AddDialAngle(float DialAngleDelta, bool bCallEvents = false, bool bSkipSettingRot = false);

	UFUNCTION(BlueprintCallable, Category = "VRDialComponent")
		void SetDialAngle(float DialAngle, bool bCallEvents = false);

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

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface", meta = (ScriptName = "IsCurrentlyHeld"))
		bool bIsHeld; 

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface")
		FBPGripPair HoldingGrip; 
	bool bOriginalReplicatesMovement;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnGripSignature OnGripped;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnDropSignature OnDropped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float BreakDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface", meta = (ScriptName = "IsDenyGripping"))
		bool bDenyGripping;

	bool DenyGripping_Implementation(UGripMotionControllerComponent * GripInitiator = nullptr) override;

	EGripInterfaceTeleportBehavior TeleportBehavior_Implementation() override;

	bool SimulateOnDrop_Implementation() override;

	EGripCollisionType GetPrimaryGripType_Implementation(bool bIsSlot) override;

		ESecondaryGripType SecondaryGripType_Implementation() override;

		EGripMovementReplicationSettings GripMovementReplicationType_Implementation() override;

		EGripLateUpdateSettings GripLateUpdateSetting_Implementation() override;

		void GetGripStiffnessAndDamping_Implementation(float &GripStiffnessOut, float &GripDampingOut) override;

		FBPAdvGripSettings AdvancedGripSettings_Implementation() override;

		float GripBreakDistance_Implementation() override;

		void ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool & bHadSlotInRange, FTransform & SlotWorldTransform, FName & SlotName,  UGripMotionControllerComponent * CallingController = nullptr, FName OverridePrefix = NAME_None) override;

		bool AllowsMultipleGrips_Implementation() override;

		void IsHeld_Implementation(TArray<FBPGripPair>& CurHoldingControllers, bool & bCurIsHeld) override;

		virtual void Native_NotifyThrowGripDelegates(UGripMotionControllerComponent* Controller, bool bGripped, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;

		void SetHeld_Implementation(UGripMotionControllerComponent * NewHoldingController, uint8 GripID, bool bNewIsHeld) override;

		bool RequestsSocketing_Implementation(USceneComponent *& ParentToSocketTo, FName & OptionalSocketName, FTransform_NetQuantize & RelativeTransform) override;

		bool GetGripScripts_Implementation(TArray<UVRGripScriptBase*> & ArrayReference) override;

		void TickGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation, float DeltaTime) override;

		void OnGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) override;

		void OnGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed = false) override;

		void OnChildGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) override;

		void OnChildGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed = false) override;

		void OnSecondaryGrip_Implementation(UGripMotionControllerComponent * GripOwningController, USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation) override;

		void OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent * GripOwningController, USceneComponent * ReleasingSecondaryGripComponent, const FBPActorGripInformation & GripInformation) override;

		void OnUsed_Implementation() override;

		void OnEndUsed_Implementation() override;

		void OnSecondaryUsed_Implementation() override;

		void OnEndSecondaryUsed_Implementation() override;

		void OnInput_Implementation(FKey Key, EInputEvent KeyEvent) override;

	protected:

};