

#pragma once
#include "CoreMinimal.h"

#include "CharacterMovementCompTypes.h"
#include "Interfaces/MovementBaseInterface.h"
#include "VRBaseCharacterMovementComponent.h"
#include "VRCharacterMovementComponent.generated.h"

class FDebugDisplayInfo;
class ACharacter;
class AVRCharacter;
class UVRRootComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogVRCharacterMovement, Log, All);

UCLASS()
class VREXPANSIONPLUGIN_API UVRCharacterMovementComponent : public UVRBaseCharacterMovementComponent
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Transient, Category = VRMovement)
		TObjectPtr<UVRRootComponent> VRRootCapsule;

	virtual void RegenerateOffset() override;

	static const float CLIMB_SWEEP_EDGE_REJECT_DISTANCE;
	virtual bool IsWithinClimbingEdgeTolerance(const FVector& CapsuleLocation, const FVector& TestImpactPoint, const float CapsuleRadius) const;
	virtual bool VRClimbStepUp(const FVector& GravDir, const FVector& Delta, const FHitResult &InHit, FStepDownResult* OutStepDownResult = nullptr) override;
	virtual FVector GetActorFeetLocation() const override;

	virtual bool IsWithinEdgeTolerance(const FVector& CapsuleLocation, const FVector& TestImpactPoint, const float CapsuleRadius) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VRCharacterMovementComponent")
	bool bAllowMovementMerging;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VRCharacterMovementComponent")
	bool bRunClientCorrectionToHMD;

	virtual void Crouch(bool bClientSimulation = false) override;

	virtual void UnCrouch(bool bClientSimulation = false) override;

	virtual FBasedPosition GetActorFeetLocationBased() const override;

	virtual bool TryToLeaveNavWalking() override;

	virtual void PhysNavWalking(float deltaTime, int32 Iterations) override;
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;

	void PostPhysicsTickComponent(float DeltaTime, FCharacterMovementComponentPostPhysicsTickFunction& ThisTickFunction) override;
	void SimulateMovement(float DeltaSeconds) override;
	void MoveSmooth(const FVector& InVelocity, const float DeltaSeconds, FStepDownResult* OutStepDownResult) override;

	TWeakObjectPtr<UPrimitiveComponent> LastServerMovementBaseVR = nullptr;
	FMovementBaseInterfaceData LastMovementBaseInterfaceDataVR;

	virtual bool ShouldCorrectRotation() const override { return !bUseClientControlRotation; }
	virtual void ClientHandleMoveResponse(const FCharacterMoveResponseDataContainer& MoveResponse) override;

	virtual void ServerMoveHandleClientErrorVR(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& RelativeClientLocation, FRotator ClientRot, FMovementBaseInterfaceData* ClientMovementBaseInterfaceData, FName ClientBaseBoneName, uint8 ClientMovementMode);

	virtual bool ServerCheckClientErrorVR(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, FRotator ClientRot, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	virtual bool ServerCheckClientErrorVR(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, FRotator ClientRot, const FVector& RelativeClientLocation, FMovementBaseInterfaceData* ClientMovementBaseInterfaceData, FName ClientBaseBoneName, uint8 ClientMovementMode);

	virtual void ClientAdjustPositionVR_Implementation(float TimeStamp, FVector NewLoc,  FVector NewVel, UPrimitiveComponent* ClientMovementBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, TOptional<FRotator> OptionalRotation = TOptional<FRotator>(), TOptional<FVector> OptionalGravityDirection = TOptional<FVector>());
	virtual void ClientAdjustPositionVR_Implementation(float TimeStamp, FVector NewLoc,  FVector NewVel, FMovementBaseInterfaceData* ClientMovementBaseInterfaceData, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, TOptional<FRotator> OptionalRotation = TOptional<FRotator>(), TOptional<FVector> OptionalGravityDirection = TOptional<FVector>());

	virtual bool ClientUpdatePositionAfterServerUpdate() override;

	virtual void ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData) override;

	FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	FNetworkPredictionData_Server* GetPredictionData_Server() const override;

	UVRCharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnRegister() override;

	float ImmersionDepth() const override;
	bool CanCrouch();

	bool SafeMoveUpdatedComponent(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult& OutHit, ETeleportType Teleport = ETeleportType::None);
	bool SafeMoveUpdatedComponent(const FVector& Delta, const FRotator& NewRotation, bool bSweep, FHitResult& OutHit, ETeleportType Teleport = ETeleportType::None);

	virtual void MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult) override;

	virtual void ApplyRepulsionForce(float DeltaSeconds) override;

	virtual void UpdateBasedMovement(float DeltaSeconds) override;

	virtual FVector GetImpartedMovementBaseVelocity() const override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction);

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

	virtual void ReplicateMoveToServer(float DeltaTime, const FVector& NewAcceleration) override;

	virtual void FindFloor(const FVector& CapsuleLocation, FFindFloorResult& OutFloorResult, bool bCanUseCachedLocation, const FHitResult* DownwardSweepResult = NULL) const;

	bool StepUp(const FVector& GravDir, const FVector& Delta, const FHitResult &InHit, FStepDownResult* OutStepDownResult = NULL) override;

	virtual FVector GetPenetrationAdjustment(const FHitResult& Hit) const override;

	virtual void PhysWalking(float deltaTime, int32 Iterations) override;

	virtual void PhysFlying(float deltaTime, int32 Iterations) override;

	virtual bool ShouldCheckForValidLandingSpot(float DeltaTime, const FVector& Delta, const FHitResult& Hit) const override;

	virtual void PhysFalling(float deltaTime, int32 Iterations) override;

	virtual void PhysSwimming(float deltaTime, int32 Iterations) override;

	void StartSwimmingVR(FVector OldLocation, FVector OldVelocity, float timeTick, float remainingTime, int32 Iterations);

	float SwimVR(FVector Delta, FHitResult& Hit);

	virtual bool CheckWaterJump(FVector CheckPoint, FVector& WallNormal) override;

	virtual void CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void StoreSetTrackingPaused(bool bNewTrackingPaused) override;
};

class VREXPANSIONPLUGIN_API FSavedMove_VRCharacter : public FSavedMove_VRBaseCharacter
{

public:

	virtual void SetInitialPosition(ACharacter* C);
	virtual void PrepMoveFor(ACharacter* Character) override;

	FSavedMove_VRCharacter() : FSavedMove_VRBaseCharacter()
	{}

};

class VREXPANSIONPLUGIN_API FNetworkPredictionData_Client_VRCharacter : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_VRCharacter(const UCharacterMovementComponent& ClientMovement)
		: FNetworkPredictionData_Client_Character(ClientMovement)
	{

	}

	FSavedMovePtr AllocateNewMove()
	{
		return FSavedMovePtr(new FSavedMove_VRCharacter());
	}
};

class VREXPANSIONPLUGIN_API FNetworkPredictionData_Server_VRCharacter : public FNetworkPredictionData_Server_Character
{
public:

	FNetworkPredictionData_Server_VRCharacter(const UCharacterMovementComponent& ClientMovement)
		: FNetworkPredictionData_Server_Character(ClientMovement)
	{
	}

	FSavedMovePtr AllocateNewMove()
	{
		return FSavedMovePtr(new FSavedMove_VRCharacter());
	}
};