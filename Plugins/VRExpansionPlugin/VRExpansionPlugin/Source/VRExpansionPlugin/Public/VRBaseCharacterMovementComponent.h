

#pragma once
#include "CoreMinimal.h"
#include "CharacterMovementCompTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "VRBaseCharacterMovementComponent.generated.h"

class AVRBaseCharacter;
class AVRCharacter;
struct FAIRequestID;
struct FPathFollowingResult;

DECLARE_LOG_CATEGORY_EXTERN(LogVRBaseCharacterMovement, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVROnPerformClimbingStepUp, FVector, FinalStepUpLocation);

UCLASS()
class VREXPANSIONPLUGIN_API UVRBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:

	UVRBaseCharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FVRCharacterNetworkMoveDataContainer VRNetworkMoveDataContainer;
	FVRCharacterMoveResponseDataContainer VRMoveResponseDataContainer;

	bool bNotifyTeleported;

	UPROPERTY(Transient, DuplicateTransient)
		TObjectPtr<AVRCharacter> BaseVRCharacterOwner;

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent);

	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel) override;
	virtual void PerformMovement(float DeltaSeconds) override;

	virtual bool ClientUpdatePositionAfterServerUpdate() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual bool ForcePositionUpdate(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRBaseCharacterMovementComponent")
		bool bUseClientControlRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRBaseCharacterMovementComponent|Smoothing")
		bool bDisableSimulatedTickWhenSmoothingMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement")
		bool bCapHMDMovementToMaxMovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Wall Walking")
		bool bAutoOrientToFloorNormal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Wall Walking")
		bool bBlendGravityFloorChanges = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Wall Walking")
		float FloorOrientationChangeBlendRate = 25.0f;

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacterMovementComponent|Wall Walking")
		void SetAutoOrientToFloorNormal(bool bAutoOrient, bool bRevertGravityWhenDisabled = true);

	bool bIsBlendingOrientation = false;

	void AutoTraceAndSetCharacterToNewGravity(FHitResult & TargetFloor, float DeltaTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacterMovementComponent|Wall Walking")
	bool SetCharacterToNewGravity(FVector NewGravityDirection, bool bOrientToNewGravity = true, float Deltatime = 1.0f);

	void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UPROPERTY(BlueprintAssignable, Category = "VRMovement")
		FVROnPerformClimbingStepUp OnPerformClimbingStepUp;

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

	FVector GetActorFeetLocationVR() const;

	FORCEINLINE bool HasRequestedVelocity()
	{
		return bHasRequestedVelocity;
	}

	void SetHasRequestedVelocity(bool bNewHasRequestedVelocity);
	bool IsClimbing() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement", meta = (ClampMin = "0.0", UIMin = "0", ClampMax = "5.0", UIMax = "5"))
		float VRWallSlideScaler;

	virtual float SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact) override;

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacterMovementComponent|VRLocations")
		void AddCustomReplicatedMovement(FVector Movement);

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacterMovementComponent|VRLocations")
		void ClearCustomReplicatedMovement();

	void CheckServerAuthedMoveAction();

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		void PerformMoveAction_SetTrackingPaused(bool bNewTrackingPaused);
	virtual void StoreSetTrackingPaused(bool bNewTrackingPaused);

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		void PerformMoveAction_SnapTurn(float SnapTurnDeltaYaw, EVRMoveActionVelocityRetention VelocityRetention = EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_None, bool bFlagGripTeleport = false, bool bFlagCharacterTeleport = false, bool bRotateAroundCapsule = true);

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		void PerformMoveAction_SetRotation(float NewYaw, EVRMoveActionVelocityRetention VelocityRetention = EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_None, bool bFlagGripTeleport = false, bool bFlagCharacterTeleport = false, bool bRotateAroundCapsule = true);

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		void PerformMoveAction_Teleport(FVector TeleportLocation, FRotator TeleportRotation, EVRMoveActionVelocityRetention VelocityRetention = EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_None, bool bSkipEncroachmentCheck = false);

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		void PerformMoveAction_StopAllMovement();

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		void PerformMoveAction_SetGravityDirection(FVector NewGravityDirection, bool bOrientToNewGravity);

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		void PerformMoveAction_Custom(EVRMoveAction MoveActionToPerform, EVRMoveActionDataReq DataRequirementsForMoveAction, FVector MoveActionVector, FRotator MoveActionRotator, uint8 MoveActionFlags = 0);

	FVRMoveActionArray MoveActionArray;

	virtual void RegenerateOffset() {};

	bool CheckForMoveAction();
	virtual bool DoMASnapTurn(FVRMoveActionContainer& MoveAction);
	virtual bool DoMASetRotation(FVRMoveActionContainer& MoveAction);
	virtual bool DoMATeleport(FVRMoveActionContainer& MoveAction);
	virtual bool DoMAStopAllMovement(FVRMoveActionContainer& MoveAction);
	virtual bool DoMASetGravityDirection(FVRMoveActionContainer& MoveAction);
	virtual bool DoMAPauseTracking(FVRMoveActionContainer& MoveAction);

	FVector CustomVRInputVector;
	FVector AdditionalVRInputVector;
	FVector LastPreAdditiveVRVelocity;
	bool bHadExtremeInput;
	bool bApplyAdditionalVRInputVectorAsNegative;

	void RewindVRRelativeMovement();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement", meta = (ClampMin = "0.0", UIMin = "0"))
		float TrackingLossThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement")
		bool bHoldPositionOnTrackingLossThresholdHit;

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		FVector RewindVRMovement();

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		FVector GetCustomInputVector();

	bool bWasInPushBack;
	bool bIsInPushBack;
	void StartPushBackNotification(FHitResult HitResult);
	void EndPushBackNotification();

	bool bJustUnseated;

	virtual bool VerifyClientTimeStamp(float TimeStamp, FNetworkPredictionData_Server_Character & ServerData) override;

	inline void ApplyVRMotionToVelocity(float deltaTime)
	{
		bHadExtremeInput = false;

		if (AdditionalVRInputVector.IsNearlyZero() && CustomVRInputVector.IsNearlyZero())
		{
			LastPreAdditiveVRVelocity = FVector::ZeroVector;
			return;
		}

		LastPreAdditiveVRVelocity = (AdditionalVRInputVector / deltaTime); 

		if (LastPreAdditiveVRVelocity.SizeSquared() > FMath::Square(TrackingLossThreshold))
		{
			bHadExtremeInput = true;
			if (bHoldPositionOnTrackingLossThresholdHit)
			{
				LastPreAdditiveVRVelocity = FVector::ZeroVector;
			}
		}

		LastPreAdditiveVRVelocity += (CustomVRInputVector / deltaTime);

		Velocity += LastPreAdditiveVRVelocity;

		if (bCapHMDMovementToMaxMovementSpeed && GetReplicatedMovementMode() != EVRConjoinedMovementModes::C_MOVE_Falling)
		{
			if (IsExceedingMaxSpeed(GetMaxSpeed()))
			{

				Velocity = Velocity.GetSafeNormal() * GetMaxSpeed();
			}
		}
	}

	inline void RestorePreAdditiveVRMotionVelocity()
	{
		if (!LastPreAdditiveVRVelocity.IsNearlyZero())
		{
			if (bHadExtremeInput)
			{

				Velocity = FVector::ZeroVector;
			}
			else
			{

				Velocity -= LastPreAdditiveVRVelocity;
			}
		}

		LastPreAdditiveVRVelocity = FVector::ZeroVector;
	}

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void PhysCustom_Climbing(float deltaTime, int32 Iterations);
	virtual void PhysCustom_LowGrav(float deltaTime, int32 Iterations);

	virtual void OnClientCorrectionReceived(class FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, FMovementBaseInterfaceData* ClientMovementBaseInterfaceData, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection) override;

	virtual void SimulatedTick(float DeltaSeconds) override;

	virtual void SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation, const FVector& NewLocation, const FQuat& NewRotation) override;

	virtual void SmoothClientPosition(float DeltaSeconds) override;

	void SmoothClientPosition_UpdateVRVisuals();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement")
		bool bIgnoreSimulatingComponentsInFloorCheck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement")
		bool bRunControlRotationInMovementComponent;

	virtual bool FloorSweepTest(
		struct FHitResult& OutHit,
		const FVector& Start,
		const FVector& End,
		ECollisionChannel TraceChannel,
		const struct FCollisionShape& CollisionShape,
		const struct FCollisionQueryParams& Params,
		const struct FCollisionResponseParams& ResponseParam
	) const override;

	virtual void ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult, float SweepRadius, const FHitResult* DownwardSweepResult = NULL) const override;

	virtual bool VRClimbStepUp(const FVector& GravDir, const FVector& Delta, const FHitResult &InHit, FStepDownResult* OutStepDownResult = nullptr);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Climbing")
		float VRClimbingStepHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Climbing")
		float VRClimbingEdgeRejectDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Climbing")
		float VRClimbingStepUpMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Climbing")
		bool bClampClimbingStepUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Climbing")
		float VRClimbingStepUpMaxSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Climbing")
		bool SetDefaultPostClimbMovementOnStepUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Climbing")
		float VRClimbingMaxReleaseVelocitySize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement")
		float VREdgeRejectDistance;

	UFUNCTION(BlueprintCallable, Category = "VRMovement|Climbing")
		void SetClimbingMode(bool bIsClimbing);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|Climbing")
		EVRConjoinedMovementModes DefaultPostClimbMovement;

	virtual void ApplyNetworkMovementMode(const uint8 ReceivedMode) override;

	UFUNCTION(BlueprintCallable, Category = "VRMovement")
		void SetReplicatedMovementMode(EVRConjoinedMovementModes NewMovementMode);

	UFUNCTION(BlueprintPure, Category = "VRMovement")
		EVRConjoinedMovementModes GetReplicatedMovementMode();

	EVRConjoinedMovementModes VRReplicatedMovementMode;

	FORCEINLINE void ApplyReplicatedMovementMode(EVRConjoinedMovementModes &NewMovementMode, bool bClearMovementMode = false)
	{
		if (NewMovementMode != EVRConjoinedMovementModes::C_MOVE_MAX)
		{
			if (NewMovementMode <= EVRConjoinedMovementModes::C_MOVE_MAX)
			{

				SetMovementMode((EMovementMode)NewMovementMode);
			}
			else 
			{

				SetMovementMode(EMovementMode::MOVE_Custom, (((int8)NewMovementMode - (uint8)EVRConjoinedMovementModes::C_VRMOVE_Climbing)));
			}

			if(bClearMovementMode)
				NewMovementMode = EVRConjoinedMovementModes::C_MOVE_MAX;
		}
	}

	void UpdateFromCompressedFlags(uint8 Flags) override;

	FVector RoundDirectMovement(FVector InMovement) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|LowGrav", meta = (ClampMin = "0.0", UIMin = "0", ClampMax = "5.0", UIMax = "5"))
		float VRLowGravWallFrictionScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMovement|LowGrav")
		bool VRLowGravIgnoresDefaultFluidFriction;
};

