

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "VRGripInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Interactibles/VRInteractibleFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "VRSliderComponent.generated.h"

class UGripMotionControllerComponent;
class USplineComponent;

UENUM(Blueprintable)
enum class EVRInteractibleSliderLerpType : uint8
{
	Lerp_None,
	Lerp_Interp,
	Lerp_InterpConstantTo
};

UENUM(Blueprintable)
enum class EVRInteractibleSliderDropBehavior : uint8
{

	Stay,

	RetainMomentum
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRSliderHitPointSignature, float, SliderProgressPoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRSliderFinishedLerpingSignature, float, FinalProgress);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRSliderComponent : public UStaticMeshComponent, public IVRGripInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UVRSliderComponent(const FObjectInitializer& ObjectInitializer);

	~UVRSliderComponent();

	UPROPERTY(BlueprintAssignable, Category = "VRSliderComponent")
		FVRSliderHitPointSignature OnSliderHitPoint;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Slider State Changed"))
		void ReceiveSliderHitPoint(float SliderProgressPoint);

	UPROPERTY(BlueprintAssignable, Category = "VRSliderComponent")
		FVRSliderFinishedLerpingSignature OnSliderFinishedLerping;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Slider Finished Lerping"))
		void ReceiveSliderFinishedLerping(float FinalProgress);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bUpdateInTick;
	bool bPassThrough;

	float LastSliderProgressState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
	FVector MaxSlideDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
	FVector MinSlideDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		EVRInteractibleSliderDropBehavior SliderBehaviorWhenReleased;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent|Momentum Settings", meta = (ClampMin = "0", ClampMax = "12", UIMin = "0", UIMax = "12"))
		int FramesToAverage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent|Momentum Settings", meta = (ClampMin = "0.0", ClampMax = "10.0", UIMin = "0.0", UIMax = "10.0"))
		float SliderMomentumFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent|Momentum Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float SliderRestitution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent|Momentum Settings", meta = (ClampMin = "0.0", UIMin = "0.0"))
		float MaxSliderMomentum;

	UPROPERTY(BlueprintReadOnly, Category = "VRSliderComponent")
		bool bIsLerping;

	FVector MomentumAtDrop;
	FVector LastSliderProgress;
	float SplineMomentumAtDrop;
	float SplineLastSliderProgress;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VRSliderComponent")
	float CurrentSliderProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
	bool bSlideDistanceIsInParentSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bIsLocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bAutoDropWhenLocked;

	UFUNCTION(BlueprintCallable, Category = "GripSettings")
		void SetIsLocked(bool bNewLockedState);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bUseLegacyLogic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float EventThrowThreshold;
	bool bHitEventThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		float PrimarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
		float SecondarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripSettings")
	int GripPriority;

	UFUNCTION(BlueprintCallable, Category = "GripSettings")
		void SetGripPriority(int NewGripPriority);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "VRSliderComponent")
	TObjectPtr<USplineComponent> SplineComponentToFollow;
public:

	TObjectPtr<USplineComponent> GetSplineComponentToFollow() { return SplineComponentToFollow; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
	bool bFollowSplineRotationAndScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bEnforceSplineLinearity;
	float LastInputKey;
	float LerpedKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		EVRInteractibleSliderLerpType SplineLerpType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent", meta = (ClampMin = "0", UIMin = "0"))
		float SplineLerpValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bSliderUsesSnapPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent", meta = (editcondition = "bSliderUsesSnapPoints", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float SnapIncrement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent", meta = (editcondition = "bSliderUsesSnapPoints", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float SnapThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent", meta = (editcondition = "bSliderUsesSnapPoints") )
		bool bIncrementProgressBetweenSnapPoints;

	virtual void OnRegister() override;

protected:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InitialRelativeTransform, Category = "VRSliderComponent")
		FTransform_NetQuantize InitialRelativeTransform;
public:

	FTransform GetInitialRelativeTransform() { return InitialRelativeTransform; }

	UFUNCTION()
	virtual void OnRep_InitialRelativeTransform()
	{
		CalculateSliderProgress();
	}

	FVector InitialInteractorLocation;
	FVector InitialGripLoc;
	FVector InitialDropLocation;

	void CheckSliderProgress();

	float GetDistanceAlongSplineAtSplineInputKey(float InKey) const;

	UFUNCTION(BlueprintCallable, Category = "VRSliderComponent")
		float CalculateSliderProgress();

	UFUNCTION(BlueprintCallable, Category = "VRSliderComponent")
		void SetSliderProgress(float NewSliderProgress);

	UFUNCTION(BlueprintCallable, Category = "VRSliderComponent")
		void ResetInitialSliderLocation();

	UFUNCTION(BlueprintCallable, Category = "VRSliderComponent")
		void SetSplineComponentToFollow(USplineComponent * SplineToFollow);

	void ResetToParentSplineLocation();

	void GetLerpedKey(float &ClosestKey, float DeltaTime);
	float GetCurrentSliderProgress(FVector CurLocation, bool bUseKeyInstead = false, float CurKey = 0.f);

	UFUNCTION(BlueprintCallable, Category = "VRSliderComponent")
		FVector GetPerAxisSliderProgress();

	FVector ClampSlideVector(FVector ValueToClamp);

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
		float BreakDistance;

	bool CheckAutoDrop(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface", meta = (ScriptName = "IsDenyGripping"))
		bool bDenyGripping;

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface", meta = (ScriptName = "IsCurrentlyHeld"))
		bool bIsHeld; 

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface")
		FBPGripPair HoldingGrip; 
	bool bOriginalReplicatesMovement;

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