

#include "VRBaseCharacterMovementComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRBaseCharacterMovementComponent)

#include "VRBPDatatypes.h"
#include "ParentRelativeAttachmentComponent.h"
#include "VRBaseCharacter.h"
#include "VRCharacter.h"
#include "VRRootComponent.h"
#include "AITypes.h"
#include "GripMotionControllerComponent.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "VRPlayerController.h"
#include "GameFramework/PhysicsVolume.h"
#include "Animation/AnimInstance.h"

DEFINE_LOG_CATEGORY(LogVRBaseCharacterMovement);

UVRBaseCharacterMovementComponent::UVRBaseCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	AdditionalVRInputVector = FVector::ZeroVector;	
	CustomVRInputVector = FVector::ZeroVector;
	TrackingLossThreshold = 6000.f;
	bApplyAdditionalVRInputVectorAsNegative = true;
	bHadExtremeInput = false;
	bHoldPositionOnTrackingLossThresholdHit = false;

	VRClimbingStepHeight = 96.0f;
	VRClimbingEdgeRejectDistance = 5.0f;
	VRClimbingStepUpMultiplier = 1.0f;
	bClampClimbingStepUp = false;
	VRClimbingStepUpMaxSize = 20.0f;

	VRClimbingMaxReleaseVelocitySize = 800.0f;
	SetDefaultPostClimbMovementOnStepUp = true;
	DefaultPostClimbMovement = EVRConjoinedMovementModes::C_MOVE_Falling;

	bIgnoreSimulatingComponentsInFloorCheck = true;

	VRWallSlideScaler = 1.0f;
	VRLowGravWallFrictionScaler = 1.0f;
	VRLowGravIgnoresDefaultFluidFriction = true;

	VREdgeRejectDistance = 0.01f; 

	VRReplicatedMovementMode = EVRConjoinedMovementModes::C_MOVE_MAX;

	NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;

	bWasInPushBack = false;
	bIsInPushBack = false;

	bRunControlRotationInMovementComponent = true;

	bEnableServerDualMoveScopedMovementUpdates = true;

	bNotifyTeleported = false;

	bJustUnseated = false;

	bUseClientControlRotation = true;
	bDisableSimulatedTickWhenSmoothingMovement = true;
	bCapHMDMovementToMaxMovementSpeed = false;

	SetNetworkMoveDataContainer(VRNetworkMoveDataContainer);
	SetMoveResponseDataContainer(VRMoveResponseDataContainer);
}

void UVRBaseCharacterMovementComponent::RewindVRRelativeMovement()
{
	if (bApplyAdditionalVRInputVectorAsNegative && (BaseVRCharacterOwner && BaseVRCharacterOwner->bRetainRoomscale))
	{

		MoveUpdatedComponent(-AdditionalVRInputVector, UpdatedComponent->GetComponentQuat(), false);

	}
}

void UVRBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (!HasValidData())
	{
		return;
	}

	CustomVRInputVector = FVector::ZeroVector;

	if (PreviousMovementMode == EMovementMode::MOVE_Custom && PreviousCustomMode == (uint8)EVRCustomMovementMode::VRMOVE_Seated)
	{
		if (MovementMode != EMovementMode::MOVE_Custom || CustomMovementMode != (uint8)EVRCustomMovementMode::VRMOVE_Seated)
		{
			if (AVRBaseCharacter * BaseOwner = Cast<AVRBaseCharacter>(CharacterOwner))
			{
				BaseOwner->InitSeatedModeTransition();
			}
		}
	}

	if (MovementMode == EMovementMode::MOVE_Custom)
	{
		if (CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Climbing || CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Seated)
		{

			StopMovementKeepPathing();
			CharacterOwner->ResetJumpState();
			ClearAccumulatedForces();

			if (CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Seated)
			{
				if (AVRBaseCharacter * BaseOwner = Cast<AVRBaseCharacter>(CharacterOwner))
				{
					BaseOwner->InitSeatedModeTransition();
				}
			}
		}
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

bool UVRBaseCharacterMovementComponent::ForcePositionUpdate(float DeltaTime)
{

	if ((MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Seated))
	{
		return false;
	}

	return Super::ForcePositionUpdate(DeltaTime);
}

bool UVRBaseCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate()
{

	if (!HasValidData())
	{
		return false;
	}

	FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
	check(ClientData);

	if (!ClientData->bUpdatePosition)
	{
		return false;
	}

	ClientData->bUpdatePosition = false;

	if (CharacterOwner->GetRootComponent() && CharacterOwner->GetRootComponent()->IsSimulatingPhysics())
	{
		return false;
	}

	if (ClientData->SavedMoves.Num() == 0)
	{
		UE_LOGF(LogNetPlayerMovement, Verbose, "ClientUpdatePositionAfterServerUpdate No saved moves to replay");

		CharacterOwner->bClientResimulateRootMotion = false;
		if (CharacterOwner->bClientResimulateRootMotionSources)
		{

			UE_LOGF(LogRootMotion, VeryVerbose, "CurrentRootMotion getting updated to ServerUpdate state: %ls", *CharacterOwner->GetName());
			CurrentRootMotion.UpdateStateFrom(CharacterOwner->SavedRootMotion);
			CharacterOwner->bClientResimulateRootMotionSources = false;
		}
		CharacterOwner->SavedRootMotion.Clear();

		return false;
	}

	const float SavedAnalogInputModifier = AnalogInputModifier;
	const FRootMotionMovementParams BackupRootMotionParams = RootMotionParams; 
	const FRootMotionSourceGroup BackupRootMotion = CurrentRootMotion;
	const bool bRealPressedJump = CharacterOwner->bPressedJump;
	const float RealJumpMaxHoldTime = CharacterOwner->JumpMaxHoldTime;
	const int32 RealJumpMaxCount = CharacterOwner->JumpMaxCount;
	const bool bRealCrouch = bWantsToCrouch;
	const bool bRealForceMaxAccel = bForceMaxAccel;
	CharacterOwner->bClientWasFalling = (MovementMode == MOVE_Falling);
	CharacterOwner->bClientUpdating = true;
	bForceNextFloorCheck = true;

	const FVRMoveActionArray Orig_MoveActions = MoveActionArray;
	const FVector Orig_CustomInput = CustomVRInputVector;
	const EVRConjoinedMovementModes Orig_VRReplicatedMovementMode = VRReplicatedMovementMode;
	const FVector Orig_RequestedVelocity = RequestedVelocity;
	const bool Orig_HasRequestedVelocity = HasRequestedVelocity();

	UE_LOGF(LogNetPlayerMovement, Verbose, "ClientUpdatePositionAfterServerUpdate Replaying %d Moves, starting at Timestamp %f", ClientData->SavedMoves.Num(), ClientData->SavedMoves[0]->TimeStamp);
	for (int32 i = 0; i < ClientData->SavedMoves.Num(); i++)
	{
		FSavedMove_Character* const CurrentMove = ClientData->SavedMoves[i].Get();
		checkSlow(CurrentMove != nullptr);

		SetCurrentReplayedSavedMove(CurrentMove);

		CurrentMove->PrepMoveFor(CharacterOwner);

		if (ShouldUsePackedMovementRPCs())
		{

			if (FCharacterNetworkMoveData* NewMove = GetNetworkMoveDataContainer().GetNewMoveData())
			{
				SetCurrentNetworkMoveData(NewMove);
				NewMove->ClientFillNetworkMoveData(*CurrentMove, FCharacterNetworkMoveData::ENetworkMoveType::NewMove);
			}
		}

		MoveAutonomous(CurrentMove->TimeStamp, CurrentMove->DeltaTime, CurrentMove->GetCompressedFlags(), CurrentMove->Acceleration);

		CurrentMove->PostUpdate(CharacterOwner, FSavedMove_Character::PostUpdate_Replay);
		SetCurrentNetworkMoveData(nullptr);
		SetCurrentReplayedSavedMove(nullptr);
	}
	const bool bPostReplayPressedJump = CharacterOwner->bPressedJump;

	if (FSavedMove_Character* const PendingMove = ClientData->PendingMove.Get())
	{
		PendingMove->bForceNoCombine = true;
	}

	AnalogInputModifier = SavedAnalogInputModifier;
	RootMotionParams = BackupRootMotionParams;
	CurrentRootMotion = BackupRootMotion;
	if (CharacterOwner->bClientResimulateRootMotionSources)
	{

		UE_LOGF(LogRootMotion, VeryVerbose, "CurrentRootMotion getting updated after ServerUpdate replays: %ls", *CharacterOwner->GetName());
		CurrentRootMotion.UpdateStateFrom(CharacterOwner->SavedRootMotion);
		CharacterOwner->bClientResimulateRootMotionSources = false;
	}
	CharacterOwner->SavedRootMotion.Clear();
	CharacterOwner->bClientResimulateRootMotion = false;
	CharacterOwner->bClientUpdating = false;
	CharacterOwner->bPressedJump = bRealPressedJump || bPostReplayPressedJump;
	CharacterOwner->JumpMaxHoldTime = RealJumpMaxHoldTime;
	CharacterOwner->JumpMaxCount = RealJumpMaxCount;
	bWantsToCrouch = bRealCrouch;
	bForceMaxAccel = bRealForceMaxAccel;
	bForceNextFloorCheck = true;

	MoveActionArray = Orig_MoveActions;
	CustomVRInputVector = Orig_CustomInput;
	VRReplicatedMovementMode = Orig_VRReplicatedMovementMode;
	RequestedVelocity = Orig_RequestedVelocity;
	SetHasRequestedVelocity(Orig_HasRequestedVelocity);

	return (ClientData->SavedMoves.Num() > 0);
}

void UVRBaseCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{

	if (CharacterOwner->IsLocallyControlled() && GetReplicatedMovementMode() == EVRConjoinedMovementModes::C_VRMOVE_Climbing)
	{

		if (BaseVRCharacterOwner)
		{
			BaseVRCharacterOwner->UpdateClimbingMovement(DeltaTime);
		}
	}

	{
		UParentRelativeAttachmentComponent* OuterScopePRC = nullptr;
		if (BaseVRCharacterOwner && BaseVRCharacterOwner->ParentRelativeAttachment && BaseVRCharacterOwner->ParentRelativeAttachment->bUpdateInCharacterMovement)
		{
			OuterScopePRC = BaseVRCharacterOwner->ParentRelativeAttachment;
		}

		FScopedMovementUpdate ScopedPRCMovementUpdate(OuterScopePRC, EScopedUpdate::DeferredUpdates);

		{
			UReplicatedVRCameraComponent* OuterScopeCamera = nullptr;
			if (BaseVRCharacterOwner && BaseVRCharacterOwner->VRReplicatedCamera)
			{
				OuterScopeCamera = BaseVRCharacterOwner->VRReplicatedCamera;
			}

			FScopedMovementUpdate ScopedCameraMovementUpdate(OuterScopeCamera, EScopedUpdate::DeferredUpdates);

			{

				FVRCharacterScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);

				if (MovementMode == MOVE_Custom && CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Seated)
				{

					if (BaseVRCharacterOwner)
					{
						if (
							!bNetworkSmoothingComplete ||
							!FVector2D(BaseVRCharacterOwner->NetSmoother->GetRelativeLocation()).Equals(FVector2D::ZeroVector) ||
							!BaseVRCharacterOwner->NetSmoother->GetRelativeRotation().IsZero()
							)
						{

							BaseVRCharacterOwner->ZeroToSeatInformation();
							bNetworkSmoothingComplete = true;
						}
					}

					const FVector InputVector = ConsumeInputVector();
					if (!HasValidData() || ShouldSkipUpdate(DeltaTime))
					{
						return;
					}

					Super::Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

					const bool bIsSimulatingPhysics = UpdatedComponent->IsSimulatingPhysics();
					if (CharacterOwner->GetLocalRole() == ROLE_Authority && (!bCheatFlying || bIsSimulatingPhysics) && !CharacterOwner->CheckStillInWorld())
					{
						return;
					}

					if (CharacterOwner->GetLocalRole() > ROLE_SimulatedProxy)
					{

						if (AVRBaseCharacter* BaseChar = Cast<AVRBaseCharacter>(CharacterOwner))
						{
							BaseChar->TickSeatInformation(DeltaTime);
						}

						CheckForMoveAction();
						MoveActionArray.Clear();

						if (CharacterOwner && !CharacterOwner->IsLocallyControlled() && DeltaTime > 0.0f)
						{

							if (!CharacterOwner->bClientUpdating && !CharacterOwner->IsPlayingRootMotion() && CharacterOwner->GetMesh())
							{
								TickCharacterPose(DeltaTime);

								CharacterOwner->GetMesh()->ConditionallyDispatchQueuedAnimEvents();
							}
						}

					}
					else
					{
						if (bNetworkUpdateReceived)
						{
							if (bNetworkMovementModeChanged)
							{
								ApplyNetworkMovementMode(CharacterOwner->GetReplicatedMovementMode());
								bNetworkMovementModeChanged = false;
							}
						}
					}
				}
				else
					Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

				if (UVRRootComponent* VRRoot = Cast<UVRRootComponent>(CharacterOwner->GetCapsuleComponent()))
				{

					if (!VRRoot->bCalledUpdateTransform)
						VRRoot->OnUpdateTransform_Public(EUpdateTransformFlags::None, ETeleportType::None);
				}

				AdditionalVRInputVector = FVector::ZeroVector;
				CustomVRInputVector = FVector::ZeroVector;
			}

			if (bRunControlRotationInMovementComponent && CharacterOwner->IsLocallyControlled())
			{
				if (BaseVRCharacterOwner)
				{
					if (BaseVRCharacterOwner->VRReplicatedCamera && BaseVRCharacterOwner->VRReplicatedCamera->bUsePawnControlRotation)
					{
						const AController* OwningController = BaseVRCharacterOwner->GetController();
						if (OwningController)
						{
							const FRotator PawnViewRotation = BaseVRCharacterOwner->GetViewRotation();
							if (!PawnViewRotation.Equals(BaseVRCharacterOwner->VRReplicatedCamera->GetComponentRotation()))
							{
								BaseVRCharacterOwner->VRReplicatedCamera->SetWorldRotation(PawnViewRotation);
							}
						}
					}
				}
			}

			if (OuterScopeCamera)
			{
				OuterScopeCamera->UpdateTracking(DeltaTime);
			}

			if (OuterScopePRC)
			{
				OuterScopePRC->UpdateTracking(DeltaTime);
			}
		}
	}

	if (bNotifyTeleported)
	{
		if (BaseVRCharacterOwner)
		{
			BaseVRCharacterOwner->OnCharacterTeleported_Bind.Broadcast();
			bNotifyTeleported = false;
		}
	}
}

bool UVRBaseCharacterMovementComponent::VerifyClientTimeStamp(float TimeStamp, FNetworkPredictionData_Server_Character & ServerData)
{

	if (MovementMode == MOVE_Custom && CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Seated)
		return false;

	return Super::VerifyClientTimeStamp(TimeStamp, ServerData);
}

void UVRBaseCharacterMovementComponent::StartPushBackNotification(FHitResult HitResult)
{
	bIsInPushBack = true;

	if (bWasInPushBack)
		return;

	bWasInPushBack = true;

	if (AVRBaseCharacter * OwningCharacter = Cast<AVRBaseCharacter>(GetCharacterOwner()))
	{
		OwningCharacter->OnBeginWallPushback(HitResult, !Acceleration.Equals(FVector::ZeroVector), AdditionalVRInputVector);
	}
}

void UVRBaseCharacterMovementComponent::EndPushBackNotification()
{
	if (bIsInPushBack || !bWasInPushBack)
		return;

	bIsInPushBack = false;
	bWasInPushBack = false;

	if (AVRBaseCharacter * OwningCharacter = Cast<AVRBaseCharacter>(GetCharacterOwner()))
	{
		OwningCharacter->OnEndWallPushback();
	}
}

FVector UVRBaseCharacterMovementComponent::GetActorFeetLocationVR() const
{

	const UCapsuleComponent* const CapsuleComponent = CharacterOwner ? CharacterOwner->GetCapsuleComponent() : Cast<UCapsuleComponent>(UpdatedComponent);
	if (CapsuleComponent)
	{
		const float HalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
		if (AVRBaseCharacter* BaseCharacter = Cast<AVRBaseCharacter>(GetCharacterOwner()))
		{
			return BaseCharacter->OffsetComponentToWorld.GetLocation() + HalfHeight * GetGravityDirection();
		}
		else
		{
			return UpdatedComponent->GetComponentLocation() + HalfHeight * GetGravityDirection();
		}
	}

	return Super::GetActorFeetLocation();

}

void UVRBaseCharacterMovementComponent::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (AVRBaseCharacter* vrOwner = Cast<AVRBaseCharacter>(GetCharacterOwner()))
	{
		vrOwner->NavigationMoveCompleted(RequestID, Result);
	}
}

bool UVRBaseCharacterMovementComponent::FloorSweepTest(
	FHitResult& OutHit,
	const FVector& Start,
	const FVector& End,
	ECollisionChannel TraceChannel,
	const struct FCollisionShape& CollisionShape,
	const struct FCollisionQueryParams& Params,
	const struct FCollisionResponseParams& ResponseParam
) const
{
	bool bBlockingHit = false;

	if (!bUseFlatBaseForFloorChecks)
	{
		bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, GetWorldToGravityTransform(), TraceChannel, CollisionShape, Params, ResponseParam);
	}
	else
	{

		const float CapsuleRadius = CollisionShape.GetCapsuleRadius();
		const float CapsuleHeight = CollisionShape.GetCapsuleHalfHeight();
		const FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(CapsuleRadius * 0.707f, CapsuleRadius * 0.707f, CapsuleHeight));

		bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat(GetGravityDirection(), UE_PI * 0.25f), TraceChannel, BoxShape, Params, ResponseParam);

		if (!bBlockingHit)
		{

			OutHit.Reset(1.f, false);
			bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, GetWorldToGravityTransform(), TraceChannel, BoxShape, Params, ResponseParam);
		}
	}

	return bBlockingHit;
}

void UVRBaseCharacterMovementComponent::ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult, float SweepRadius, const FHitResult* DownwardSweepResult) const
{
	UE_LOGF(LogVRBaseCharacterMovement, VeryVerbose, "[Role:%d] ComputeFloorDist: %ls at location %ls", (int32)CharacterOwner->GetLocalRole(), *GetNameSafe(CharacterOwner), *CapsuleLocation.ToString());
	OutFloorResult.Clear();

	float PawnRadius, PawnHalfHeight;
	CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

	bool bSkipSweep = false;
	if (DownwardSweepResult != NULL && DownwardSweepResult->IsValidBlockingHit())
	{

		const bool bIsDownward = GetGravitySpaceZ(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd) > 0;
		const bool bIsVertical = ProjectToGravityFloor(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd).SizeSquared() <= UE_KINDA_SMALL_NUMBER;
		if (bIsDownward && bIsVertical)
		{

			if (IsWithinEdgeTolerance(DownwardSweepResult->Location, DownwardSweepResult->ImpactPoint, PawnRadius))
			{

				bSkipSweep = true;

				const bool bIsWalkable = IsWalkable(*DownwardSweepResult);
				const float FloorDist = GetGravitySpaceZ(CapsuleLocation - DownwardSweepResult->Location);
				OutFloorResult.SetFromSweep(*DownwardSweepResult, FloorDist, bIsWalkable);

				if (bIsWalkable)
				{

					return;
				}
			}
		}
	}

	if (SweepDistance < LineDistance)
	{
		ensure(SweepDistance >= LineDistance);
		return;
	}

	bool bBlockingHit = false;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ComputeFloorDist), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(QueryParams, ResponseParam);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

	if (bIgnoreSimulatingComponentsInFloorCheck)
		ResponseParam.CollisionResponse.PhysicsBody = ECollisionResponse::ECR_Ignore;

	if (!bSkipSweep && SweepDistance > 0.f && SweepRadius > 0.f)
	{

		const float ShrinkScale = 0.9f;
		const float ShrinkScaleOverlap = 0.1f;
		float ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScale);
		float TraceDist = SweepDistance + ShrinkHeight;
		FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(SweepRadius, PawnHalfHeight - ShrinkHeight);

		FHitResult Hit(1.f);
		bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + TraceDist * GetGravityDirection(), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);

		if (bBlockingHit)
		{

			if (Hit.bStartPenetrating || !IsWithinEdgeTolerance(CapsuleLocation, Hit.ImpactPoint, CapsuleShape.Capsule.Radius))
			{

				CapsuleShape.Capsule.Radius = FMath::Max(0.f, CapsuleShape.Capsule.Radius - SWEEP_EDGE_REJECT_DISTANCE - UE_KINDA_SMALL_NUMBER);
				if (!CapsuleShape.IsNearlyZero())
				{
					ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScaleOverlap);
					TraceDist = SweepDistance + ShrinkHeight;
					CapsuleShape.Capsule.HalfHeight = FMath::Max(PawnHalfHeight - ShrinkHeight, CapsuleShape.Capsule.Radius);
					Hit.Reset(1.f, false);

					bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + TraceDist * GetGravityDirection(), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);
				}
			}

			const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
			const float SweepResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

			OutFloorResult.SetFromSweep(Hit, SweepResult, false);
			if (Hit.IsValidBlockingHit() && IsWalkable(Hit))
			{
				if (SweepResult <= SweepDistance)
				{

					OutFloorResult.bWalkableFloor = true;
					return;
				}
			}
		}
	}

	if (!OutFloorResult.bBlockingHit && !OutFloorResult.HitResult.bStartPenetrating)
	{
		OutFloorResult.FloorDist = SweepDistance;
		return;
	}

	if (LineDistance > 0.f)
	{
		const float ShrinkHeight = PawnHalfHeight;
		const FVector LineTraceStart = CapsuleLocation;
		const float TraceDist = LineDistance + ShrinkHeight;
		const FVector Down = TraceDist * GetGravityDirection();
		QueryParams.TraceTag = SCENE_QUERY_STAT_NAME_ONLY(FloorLineTrace);

		FHitResult Hit(1.f);
		bBlockingHit = GetWorld()->LineTraceSingleByChannel(Hit, LineTraceStart, LineTraceStart + Down, CollisionChannel, QueryParams, ResponseParam);

		if (bBlockingHit)
		{
			if (Hit.Time > 0.f)
			{

				const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
				const float LineResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

				OutFloorResult.bBlockingHit = true;
				if (LineResult <= LineDistance && IsWalkable(Hit))
				{
					OutFloorResult.SetFromLineTrace(Hit, OutFloorResult.FloorDist, LineResult, true);
					return;
				}
			}
		}
	}

	OutFloorResult.bWalkableFloor = false;
}

float UVRBaseCharacterMovementComponent::SlideAlongSurface(const FVector& Delta, float Time, const FVector& InNormal, FHitResult& Hit, bool bHandleImpact)
{

	if (!Hit.bBlockingHit)
	{
		return 0.f;
	}

	FVector Normal(InNormal);
	const FVector::FReal NormalZ = GetGravitySpaceZ(Normal);
	if (IsMovingOnGround())
	{

		if (NormalZ > 0.f)
		{
			if (!IsWalkable(Hit))
			{
				Normal = ProjectToGravityFloor(Normal).GetSafeNormal();
			}
		}
		else if (NormalZ < -UE_KINDA_SMALL_NUMBER)
		{

			if (CurrentFloor.FloorDist < MIN_FLOOR_DIST && CurrentFloor.bBlockingHit)
			{
				const FVector FloorNormal = CurrentFloor.HitResult.Normal;
				const bool bFloorOpposedToMovement = (Delta | FloorNormal) < 0.f && (GetGravitySpaceZ(FloorNormal) < 1.f - UE_DELTA);

				if (bFloorOpposedToMovement)
				{
					Normal = FloorNormal;
				}

				Normal = ProjectToGravityFloor(Normal).GetSafeNormal();
			}
		}
	}

	StartPushBackNotification(Hit);

	if (IsMovingOnGround() || (MovementMode == MOVE_Custom && CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Climbing))
		return Super::Super::SlideAlongSurface(Delta * VRWallSlideScaler, Time, Normal, Hit, bHandleImpact);
	else
		return Super::Super::SlideAlongSurface(Delta, Time, Normal, Hit, bHandleImpact);
}

void UVRBaseCharacterMovementComponent::AddCustomReplicatedMovement(FVector Movement)
{

	if (GetNetMode() == NM_Client)
		CustomVRInputVector += RoundDirectMovement(Movement);
	else
		CustomVRInputVector += Movement; 
}

void UVRBaseCharacterMovementComponent::ClearCustomReplicatedMovement()
{
	CustomVRInputVector = FVector::ZeroVector;
}

void UVRBaseCharacterMovementComponent::CheckServerAuthedMoveAction()
{

	if (GetNetMode() < NM_Client)
	{
		ACharacter* OwningChar = GetCharacterOwner();
		if (OwningChar && !OwningChar->IsLocallyControlled())
		{
			CheckForMoveAction();
			MoveActionArray.Clear();
		}
	}
}

void UVRBaseCharacterMovementComponent::PerformMoveAction_SetTrackingPaused(bool bNewTrackingPaused)
{
	StoreSetTrackingPaused(bNewTrackingPaused);
}

void UVRBaseCharacterMovementComponent::StoreSetTrackingPaused(bool bNewTrackingPaused)
{
	FVRMoveActionContainer MoveAction;
	MoveAction.MoveAction = EVRMoveAction::VRMOVEACTION_PauseTracking;
	MoveAction.MoveActionFlags = bNewTrackingPaused;
	MoveActionArray.MoveActions.Add(MoveAction);
	CheckServerAuthedMoveAction();
}

void UVRBaseCharacterMovementComponent::PerformMoveAction_SnapTurn(float DeltaYawAngle, EVRMoveActionVelocityRetention VelocityRetention, bool bFlagGripTeleport, bool bFlagCharacterTeleport, bool bRotateAroundCapsule )
{
	FVRMoveActionContainer MoveAction;
	MoveAction.MoveAction = EVRMoveAction::VRMOVEACTION_SnapTurn; 

	FRotator TargetRotation = (UpdatedComponent->GetComponentQuat() * FRotator(0.f, DeltaYawAngle, 0.f).Quaternion()).Rotator();
	TargetRotation.Yaw = FRotator::DecompressAxisFromShort(FRotator::CompressAxisToShort(TargetRotation.Yaw));
	TargetRotation.Pitch = FRotator::DecompressAxisFromShort(FRotator::CompressAxisToShort(TargetRotation.Pitch));
	TargetRotation.Roll = FRotator::DecompressAxisFromShort(FRotator::CompressAxisToShort(TargetRotation.Roll));
	MoveAction.MoveActionRot = TargetRotation;

	if (bFlagCharacterTeleport)
		MoveAction.MoveActionFlags = 0x02;
	else if(bFlagGripTeleport)
		MoveAction.MoveActionFlags = 0x01;

	if (bRotateAroundCapsule)
	{
		MoveAction.MoveActionFlags |= 0x08;
	}

	if (VelocityRetention == EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn)
	{

		MoveAction.MoveActionDeltaYaw = FRotator::DecompressAxisFromShort(FRotator::CompressAxisToShort(DeltaYawAngle));
	}

	MoveAction.VelRetentionSetting = VelocityRetention;

	MoveActionArray.MoveActions.Add(MoveAction);
	CheckServerAuthedMoveAction();
}

void UVRBaseCharacterMovementComponent::PerformMoveAction_SetRotation(float NewYaw, EVRMoveActionVelocityRetention VelocityRetention, bool bFlagGripTeleport, bool bFlagCharacterTeleport, bool bRotateAroundCapsule)
{
	FVRMoveActionContainer MoveAction;
	MoveAction.MoveAction = EVRMoveAction::VRMOVEACTION_SetRotation;
	MoveAction.MoveActionRot = FRotator(0.0f, FMath::RoundToFloat(NewYaw * 100.f) / 100.f, 0.0f);

	if (bFlagCharacterTeleport)
		MoveAction.MoveActionFlags = 0x02;
	else if (bFlagGripTeleport)
		MoveAction.MoveActionFlags = 0x01;

	if (bRotateAroundCapsule)
	{
		MoveAction.MoveActionFlags |= 0x08;
	}

	if (VelocityRetention == EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn)
	{
		float DeltaYawAngle = FMath::FindDeltaAngleDegrees(UpdatedComponent->GetComponentRotation().Yaw, NewYaw);

		MoveAction.MoveActionRot.Pitch = DeltaYawAngle;
	}

	MoveAction.VelRetentionSetting = VelocityRetention;

	MoveActionArray.MoveActions.Add(MoveAction);
	CheckServerAuthedMoveAction();
}

void UVRBaseCharacterMovementComponent::PerformMoveAction_Teleport(FVector TeleportLocation, FRotator TeleportRotation, EVRMoveActionVelocityRetention VelocityRetention,  bool bSkipEncroachmentCheck)
{
	FVRMoveActionContainer MoveAction;
	MoveAction.MoveAction = EVRMoveAction::VRMOVEACTION_Teleport;
	MoveAction.MoveActionLoc = RoundDirectMovement(TeleportLocation);
	MoveAction.MoveActionRot.Yaw = FMath::RoundToFloat(TeleportRotation.Yaw * 100.f) / 100.f;
	MoveAction.MoveActionFlags |= (uint8)bSkipEncroachmentCheck;

	if (VelocityRetention == EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn)
	{
		float DeltaYawAngle = FMath::FindDeltaAngleDegrees(UpdatedComponent->GetComponentRotation().Yaw, TeleportRotation.Yaw);

		MoveAction.MoveActionRot.Pitch = DeltaYawAngle;
	}

	MoveAction.VelRetentionSetting = VelocityRetention;

	MoveActionArray.MoveActions.Add(MoveAction);
	CheckServerAuthedMoveAction();
}

void UVRBaseCharacterMovementComponent::PerformMoveAction_StopAllMovement()
{
	FVRMoveActionContainer MoveAction;
	MoveAction.MoveAction = EVRMoveAction::VRMOVEACTION_StopAllMovement;
	MoveActionArray.MoveActions.Add(MoveAction);

	CheckServerAuthedMoveAction();
}

void UVRBaseCharacterMovementComponent::PerformMoveAction_SetGravityDirection(FVector NewGravityDirection, bool bOrientToNewGravity)
{
	if (NewGravityDirection.IsNearlyZero())
	{
		return;
	}

	FVRMoveActionContainer MoveAction;
	MoveAction.MoveAction = EVRMoveAction::VRMOVEACTION_SetGravityDirection;
	MoveAction.MoveActionVel = NewGravityDirection.GetSafeNormal();
	MoveAction.MoveActionFlags |= (uint8)bOrientToNewGravity;
	MoveActionArray.MoveActions.Add(MoveAction);

	CheckServerAuthedMoveAction();
}

void UVRBaseCharacterMovementComponent::PerformMoveAction_Custom(EVRMoveAction MoveActionToPerform, EVRMoveActionDataReq DataRequirementsForMoveAction, FVector MoveActionVector, FRotator MoveActionRotator, uint8 MoveActionFlags)
{
	FVRMoveActionContainer MoveAction;
	MoveAction.MoveAction = MoveActionToPerform;

	MoveAction.MoveActionLoc = RoundDirectMovement(MoveActionVector);
	MoveAction.MoveActionRot = MoveActionRotator;
	MoveAction.MoveActionDataReq = DataRequirementsForMoveAction;
	MoveAction.MoveActionFlags = MoveActionFlags;
	MoveActionArray.MoveActions.Add(MoveAction);

	CheckServerAuthedMoveAction();
}

bool UVRBaseCharacterMovementComponent::CheckForMoveAction()
{
	if (!BaseVRCharacterOwner)
		return true;

	for (FVRMoveActionContainer& MoveAction : MoveActionArray.MoveActions)
	{
		switch (MoveAction.MoveAction)
		{
		case EVRMoveAction::VRMOVEACTION_SnapTurn:
		{
			DoMASnapTurn(MoveAction);
		}break;
		case EVRMoveAction::VRMOVEACTION_Teleport:
		{
			if (!BaseVRCharacterOwner->SeatInformation.bSitting)
			{
				DoMATeleport(MoveAction);
			}
		}break;
		case EVRMoveAction::VRMOVEACTION_StopAllMovement:
		{
			DoMAStopAllMovement(MoveAction);
		}break;
		case EVRMoveAction::VRMOVEACTION_SetGravityDirection:
		{
			if (!BaseVRCharacterOwner->SeatInformation.bSitting)
			{
				DoMASetGravityDirection(MoveAction);
			}
		}break;
		case EVRMoveAction::VRMOVEACTION_SetRotation:
		{
			if (!BaseVRCharacterOwner->SeatInformation.bSitting)
			{
				DoMASetRotation(MoveAction);
			}
		}break;
		case EVRMoveAction::VRMOVEACTION_PauseTracking:
		{
			DoMAPauseTracking(MoveAction);
		}break;
		case EVRMoveAction::VRMOVEACTION_None:
		{}break;
		default: 
		{
			if (BaseVRCharacterOwner)
			{
				BaseVRCharacterOwner->OnCustomMoveActionPerformed(MoveAction.MoveAction, MoveAction.MoveActionLoc, MoveAction.MoveActionRot, MoveAction.MoveActionFlags);
			}
		}break;
		}
	}

	return true;
}

bool UVRBaseCharacterMovementComponent::DoMASnapTurn(FVRMoveActionContainer& MoveAction)
{
	if (BaseVRCharacterOwner)
	{	
		FRotator TargetRot = MoveAction.MoveActionRot;
		FQuat OrigRot = BaseVRCharacterOwner->GetActorQuat();

		if (BaseVRCharacterOwner->SeatInformation.bSitting)
		{
			FRotator DeltaRot(0.f, MoveAction.MoveActionDeltaYaw, 0.f);
			TargetRot = ( OrigRot * DeltaRot.Quaternion() ).Rotator();
		}

		FTransform OriginalRelativeTrans = BaseVRCharacterOwner->GetRootComponent()->GetRelativeTransform();

		bool bRotateAroundCapsule = MoveAction.MoveActionFlags & 0x08;

		bIsBlendingOrientation = true;

		if (this->BaseVRCharacterOwner && this->BaseVRCharacterOwner->IsLocallyControlled())
		{
			if (this->bUseClientControlRotation)
			{
				MoveAction.MoveActionLoc = BaseVRCharacterOwner->SetActorRotationVR(TargetRot, false, false, bRotateAroundCapsule);
				MoveAction.MoveActionFlags |= 0x04; 
			}
			else
			{
				BaseVRCharacterOwner->SetActorRotationVR(TargetRot, false, false, bRotateAroundCapsule);
			}
		}
		else
		{
			if (MoveAction.MoveActionFlags & 0x04)
			{
				BaseVRCharacterOwner->SetActorLocation(BaseVRCharacterOwner->GetActorLocation() + MoveAction.MoveActionLoc);
			}
			else
			{
				BaseVRCharacterOwner->SetActorRotationVR(TargetRot, false, false, bRotateAroundCapsule);
			}
		}

		switch (MoveAction.VelRetentionSetting)
		{
		case EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_None:
		{

		}break;
		case EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Clear:
		{
			this->Velocity = FVector::ZeroVector;
		}break;
		case EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn:
		{	
			if (BaseVRCharacterOwner->IsLocallyControlled())
			{
				MoveAction.MoveActionVel = RoundDirectMovement((TargetRot.Quaternion() * OrigRot.Inverse()).RotateVector(this->Velocity));
				this->Velocity = MoveAction.MoveActionVel;
			}
			else
			{
				this->Velocity = MoveAction.MoveActionVel;
			}
		}break;
		}

		if (MoveAction.MoveActionFlags & 0x01 || MoveAction.MoveActionFlags & 0x02)
		{
			BaseVRCharacterOwner->NotifyOfTeleport(MoveAction.MoveActionFlags & 0x02);
		}

		if (BaseVRCharacterOwner->SeatInformation.bSitting)
		{
			BaseVRCharacterOwner->SeatInformation.StoredTargetTransform = (OriginalRelativeTrans.Inverse() * BaseVRCharacterOwner->GetRootComponent()->GetRelativeTransform()) * BaseVRCharacterOwner->SeatInformation.StoredTargetTransform;
			if (BaseVRCharacterOwner->IsLocallyControlled() && GetNetMode() == ENetMode::NM_Client)
			{
				BaseVRCharacterOwner->Server_SeatedSnapTurn(MoveAction.MoveActionDeltaYaw);
			}
		}
	}

	return false;
}

bool UVRBaseCharacterMovementComponent::DoMASetRotation(FVRMoveActionContainer& MoveAction)
{
	bool bRotateAroundCapsule = MoveAction.MoveActionFlags & 0x08;

	if (BaseVRCharacterOwner)
	{
		FTransform OriginalRelativeTrans = BaseVRCharacterOwner->GetRootComponent()->GetRelativeTransform();

		FRotator TargetRot(0.f, MoveAction.MoveActionRot.Yaw, 0.f);

		bIsBlendingOrientation = true;

		if (this->BaseVRCharacterOwner && this->BaseVRCharacterOwner->IsLocallyControlled())
		{
			if (this->bUseClientControlRotation)
			{
				MoveAction.MoveActionLoc = BaseVRCharacterOwner->SetActorRotationVR(TargetRot, true);
				MoveAction.MoveActionFlags |= 0x04; 
			}
			else
			{
				BaseVRCharacterOwner->SetActorRotationVR(TargetRot, true, true, bRotateAroundCapsule);
			}
		}
		else
		{
			if (MoveAction.MoveActionFlags & 0x04)
			{
				BaseVRCharacterOwner->SetActorLocation(BaseVRCharacterOwner->GetActorLocation() + MoveAction.MoveActionLoc);
			}
			else
			{
				BaseVRCharacterOwner->SetActorRotationVR(TargetRot, true, true, bRotateAroundCapsule);
			}
		}

		switch (MoveAction.VelRetentionSetting)
		{
		case EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_None:
		{

		}break;
		case EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Clear:
		{
			this->Velocity = FVector::ZeroVector;
		}break;
		case EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn:
		{
			if (BaseVRCharacterOwner->IsLocallyControlled())
			{
				MoveAction.MoveActionVel = RoundDirectMovement(FRotator(0.f, MoveAction.MoveActionRot.Pitch, 0.f).RotateVector(this->Velocity));
				this->Velocity = MoveAction.MoveActionVel;
			}
			else
			{
				this->Velocity = MoveAction.MoveActionVel;
			}
		}break;
		}

		if (MoveAction.MoveActionFlags & 0x01 || MoveAction.MoveActionFlags & 0x02)
		{
			BaseVRCharacterOwner->NotifyOfTeleport(MoveAction.MoveActionFlags & 0x02);
		}
	}

	return false;
}

bool UVRBaseCharacterMovementComponent::DoMATeleport(FVRMoveActionContainer& MoveAction)
{
	if (BaseVRCharacterOwner)
	{
		AController* OwningController = BaseVRCharacterOwner->GetController();

		if (!OwningController)
		{
			MoveAction.MoveAction = EVRMoveAction::VRMOVEACTION_None;
			return false;
		}

		bool bSkipEncroachmentCheck = MoveAction.MoveActionFlags & 0x01; 
		FRotator TargetRot(0.f, MoveAction.MoveActionRot.Yaw, 0.f);
		BaseVRCharacterOwner->TeleportTo(MoveAction.MoveActionLoc, TargetRot, false, bSkipEncroachmentCheck);

		switch (MoveAction.VelRetentionSetting)
		{
		case EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_None:
		{

		}break;
		case EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Clear:
		{
			this->Velocity = FVector::ZeroVector;
		}break;
		case EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn:
		{
			if (BaseVRCharacterOwner->IsLocallyControlled())
			{
				MoveAction.MoveActionVel = RoundDirectMovement(FRotator(0.f, MoveAction.MoveActionRot.Pitch, 0.f).RotateVector(this->Velocity));
				this->Velocity = MoveAction.MoveActionVel;
			}
			else
			{
				this->Velocity = MoveAction.MoveActionVel;
			}
		}break;
		}

		if (BaseVRCharacterOwner->bUseControllerRotationYaw)
			OwningController->SetControlRotation(TargetRot);

		return true;
	}

	return false;
}

bool UVRBaseCharacterMovementComponent::DoMAStopAllMovement(FVRMoveActionContainer& MoveAction)
{
	if (AVRBaseCharacter * OwningCharacter = Cast<AVRBaseCharacter>(GetCharacterOwner()))
	{
		this->StopMovementImmediately();
		return true;
	}

	return false;
}

bool UVRBaseCharacterMovementComponent::DoMASetGravityDirection(FVRMoveActionContainer& MoveAction)
{
	bool bOrientToNewGravity = MoveAction.MoveActionFlags > 0;
	return SetCharacterToNewGravity(MoveAction.MoveActionVel, bOrientToNewGravity);
}

bool UVRBaseCharacterMovementComponent::DoMAPauseTracking(FVRMoveActionContainer& MoveAction)
{
	if (BaseVRCharacterOwner)
	{
		BaseVRCharacterOwner->bTrackingPaused = MoveAction.MoveActionFlags > 0;
		BaseVRCharacterOwner->PausedTrackingLoc = MoveAction.MoveActionLoc;
		BaseVRCharacterOwner->PausedTrackingRot = MoveAction.MoveActionRot.Yaw;
		return true;
	}
	return false;
}

void UVRBaseCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	switch (static_cast<EVRCustomMovementMode>(CustomMovementMode))
	{
	case EVRCustomMovementMode::VRMOVE_Climbing:
		PhysCustom_Climbing(deltaTime, Iterations);
		break;
	case EVRCustomMovementMode::VRMOVE_LowGrav:
		PhysCustom_LowGrav(deltaTime, Iterations);
		break;
	case EVRCustomMovementMode::VRMOVE_Seated:
		break;
	default:
		Super::PhysCustom(deltaTime, Iterations);
		break;
	}
}

bool UVRBaseCharacterMovementComponent::VRClimbStepUp(const FVector& GravDir, const FVector& Delta, const FHitResult &InHit, FStepDownResult* OutStepDownResult)
{
	return StepUp(GravDir, Delta, InHit, OutStepDownResult);
}

void UVRBaseCharacterMovementComponent::PhysCustom_Climbing(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	Velocity = FVector::ZeroVector;

	RewindVRRelativeMovement();

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = CustomVRInputVector;
	FVector Delta = Adjusted + AdditionalVRInputVector;
	bool bZeroDelta = Delta.IsNearlyZero();

	FStepDownResult StepDownResult;

	float OldMaxStepHeight = MaxStepHeight;
	MaxStepHeight = VRClimbingStepHeight;
	bool bSteppedUp = false;

	if (!bZeroDelta)
	{
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

		if (Hit.Time < 1.f)
		{
			const FVector GravDir = FVector(0.f, 0.f, -1.f);
			const FVector VelDir = (CustomVRInputVector).GetSafeNormal();
			const float UpDown = GravDir | VelDir;

			if ((FMath::Abs(Hit.ImpactNormal.Z) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(Hit))
			{

				FVRCharacterScopedMovementUpdate ScopedStepUpMovement(UpdatedComponent, EScopedUpdate::DeferredUpdates);

				float stepZ = UpdatedComponent->GetComponentLocation().Z;

					if(bClampClimbingStepUp)
						bSteppedUp = VRClimbStepUp(GravDir, ((Adjusted.GetClampedToMaxSize2D(VRClimbingStepUpMaxSize) * VRClimbingStepUpMultiplier) + AdditionalVRInputVector) * (1.f - Hit.Time), Hit, &StepDownResult);
					else
						bSteppedUp = VRClimbStepUp(GravDir, ((Adjusted * VRClimbingStepUpMultiplier) + AdditionalVRInputVector) * (1.f - Hit.Time), Hit, &StepDownResult);

				if (bSteppedUp && OnPerformClimbingStepUp.IsBound())
				{
					FVector finalLoc = UpdatedComponent->GetComponentLocation();

					ScopedStepUpMovement.RevertMove();

					MaxStepHeight = OldMaxStepHeight;

					OnPerformClimbingStepUp.Broadcast(finalLoc);
					return;
				}

				if (bSteppedUp)
				{
					OldLocation.Z = UpdatedComponent->GetComponentLocation().Z + (OldLocation.Z - stepZ);
				}

			}

			if (!bSteppedUp)
			{

				HandleImpact(Hit, deltaTime, Adjusted);
				SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
			}
		}
	}

	MaxStepHeight = OldMaxStepHeight;

	if (bSteppedUp)
	{
		if (AVRBaseCharacter * ownerCharacter = Cast<AVRBaseCharacter>(CharacterOwner))
		{
			if (SetDefaultPostClimbMovementOnStepUp)
			{

				SetReplicatedMovementMode(DefaultPostClimbMovement);

				Velocity = FVector::ZeroVector;
			}

			ownerCharacter->OnClimbingSteppedUp();
		}
	}

	if (StepDownResult.bComputedFloor)
	{
		CurrentFloor = StepDownResult.FloorResult;
	}
	else
	{
		FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
	}

	if (CurrentFloor.IsWalkableFloor())
	{
		if(CurrentFloor.GetDistanceToFloor() < (MIN_FLOOR_DIST + MAX_FLOOR_DIST) / 2)
			AdjustFloorHeight();

	}
	else if (CurrentFloor.HitResult.bStartPenetrating)
	{

		FHitResult Hit(CurrentFloor.HitResult);
		Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
		const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
		ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
		bForceNextFloorCheck = true;
	}

	if (bAutoOrientToFloorNormal && CurrentFloor.IsWalkableFloor())
	{

		AutoTraceAndSetCharacterToNewGravity(CurrentFloor.HitResult, deltaTime);
	}

	if(!bSteppedUp || !SetDefaultPostClimbMovementOnStepUp)
	{
		if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			Velocity = (((UpdatedComponent->GetComponentLocation() - OldLocation) - AdditionalVRInputVector) / deltaTime).GetClampedToMaxSize(VRClimbingMaxReleaseVelocitySize);
		}
	}
}

void UVRBaseCharacterMovementComponent::PhysCustom_LowGrav(float deltaTime, int32 Iterations)
{

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (CharacterOwner->IsLocallyControlled())
	{

		if (AVRBaseCharacter * characterOwner = Cast<AVRBaseCharacter>(CharacterOwner))
		{
			characterOwner->UpdateLowGravMovement(deltaTime);
		}
	}

	float Friction = 0.0f; 

	RewindVRRelativeMovement();

	if(!VRLowGravIgnoresDefaultFluidFriction || GetWorld()->GetDefaultPhysicsVolume() != GetPhysicsVolume())
		Friction = 0.5f * GetPhysicsVolume()->FluidFriction;

	CalcVelocity(deltaTime, Friction, true, 0.0f);

	ApplyVRMotionToVelocity(deltaTime);

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = (Velocity * deltaTime);
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{

		const FVector GravDir = FVector(0.f, 0.f, -1.f);
		const FVector VelDir = Velocity.GetSafeNormal();
		const float UpDown = GravDir | VelDir;

		bool bSteppedUp = false;
		if ((FMath::Abs(Hit.ImpactNormal.Z) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(Hit))
		{
			float stepZ = UpdatedComponent->GetComponentLocation().Z;
			bSteppedUp = StepUp(GravDir, Adjusted * (1.f - Hit.Time), Hit);
			if (bSteppedUp)
			{
				OldLocation.Z = UpdatedComponent->GetComponentLocation().Z + (OldLocation.Z - stepZ);
			}
		}

		if (!bSteppedUp)
		{

			HandleImpact(Hit, deltaTime, Adjusted);
			SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
		}

		if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			Velocity = (((UpdatedComponent->GetComponentLocation() - OldLocation) ) / deltaTime) * VRLowGravWallFrictionScaler;
		}
	}
	else
	{
		if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			Velocity = (((UpdatedComponent->GetComponentLocation() - OldLocation) ) / deltaTime);
		}
	}

	RestorePreAdditiveVRMotionVelocity();
}

void UVRBaseCharacterMovementComponent::SetClimbingMode(bool bIsClimbing)
{
	if (bIsClimbing)
		VRReplicatedMovementMode = EVRConjoinedMovementModes::C_VRMOVE_Climbing;
	else
		VRReplicatedMovementMode = DefaultPostClimbMovement;
}

void UVRBaseCharacterMovementComponent::SetReplicatedMovementMode(EVRConjoinedMovementModes NewMovementMode)
{

	VRReplicatedMovementMode = NewMovementMode;
}

EVRConjoinedMovementModes UVRBaseCharacterMovementComponent::GetReplicatedMovementMode()
{
	if (MovementMode == EMovementMode::MOVE_Custom)
	{
		return (EVRConjoinedMovementModes)((int8)CustomMovementMode + (int8)EVRConjoinedMovementModes::C_VRMOVE_Climbing);
	}
	else
		return (EVRConjoinedMovementModes)MovementMode.GetValue();
}

void UVRBaseCharacterMovementComponent::ApplyNetworkMovementMode(const uint8 ReceivedMode)
{
	if (CharacterOwner->GetLocalRole() != ENetRole::ROLE_SimulatedProxy)
	{
		const uint8 CurrentPackedMovementMode = PackNetworkMovementMode();
		if (CurrentPackedMovementMode != ReceivedMode)
		{
			TEnumAsByte<EMovementMode> NetMovementMode(MOVE_None);
			TEnumAsByte<EMovementMode> NetGroundMode(MOVE_None);
			uint8 NetCustomMode(0);
			UnpackNetworkMovementMode(ReceivedMode, NetMovementMode, NetCustomMode, NetGroundMode);

			if (NetMovementMode == EMovementMode::MOVE_Custom || MovementMode == EMovementMode::MOVE_Custom)
			{
				if (NetCustomMode == (uint8)EVRCustomMovementMode::VRMOVE_Climbing || CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Climbing)
				return; 
			}
		}
	}

	Super::ApplyNetworkMovementMode(ReceivedMode);
}

void  UVRBaseCharacterMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);

	BaseVRCharacterOwner = Cast<AVRCharacter>(CharacterOwner);
}

void UVRBaseCharacterMovementComponent::PerformMovement(float DeltaSeconds)
{

	FVRCharacterScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);

	if (bRunControlRotationInMovementComponent && CharacterOwner->IsLocallyControlled())
	{
		if (BaseVRCharacterOwner && BaseVRCharacterOwner->OwningVRPlayerController)
		{
			BaseVRCharacterOwner->OwningVRPlayerController->RotationInput = BaseVRCharacterOwner->OwningVRPlayerController->LastRotationInput;
			BaseVRCharacterOwner->OwningVRPlayerController->UpdateRotation(DeltaSeconds);
			BaseVRCharacterOwner->OwningVRPlayerController->LastRotationInput = FRotator::ZeroRotator;
			BaseVRCharacterOwner->OwningVRPlayerController->RotationInput = FRotator::ZeroRotator;
		}
	}

	ApplyReplicatedMovementMode(VRReplicatedMovementMode, true);

	CheckForMoveAction();

	bIsInPushBack = false;

	if (BaseVRCharacterOwner->VRRootReference->bUseWalkingCollisionOverride && !BaseVRCharacterOwner->bRetainRoomscale)
	{
		bool bAllowWalkingCollision = false;
		if (MovementMode == EMovementMode::MOVE_Walking || MovementMode == EMovementMode::MOVE_NavWalking)
			bAllowWalkingCollision = true;

		BaseVRCharacterOwner->VRRootReference->SetCollisionOverride(bAllowWalkingCollision && GetCurrentAcceleration().IsNearlyZero());
	}

	Super::PerformMovement(DeltaSeconds);

	EndPushBackNotification(); 

	if (CharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		MoveActionArray.Clear();
	}
}

void UVRBaseCharacterMovementComponent::OnClientCorrectionReceived(class FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, FMovementBaseInterfaceData* ClientMovementBaseInterfaceData, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection)
{
	Super::OnClientCorrectionReceived(ClientData, TimeStamp, NewLocation, NewVelocity, ClientMovementBaseInterfaceData, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode, ServerGravityDirection);

	if (BaseVRCharacterOwner)
	{
		BaseVRCharacterOwner->OnCharacterNetworkCorrected_Bind.Broadcast();

		if (IsValid(BaseVRCharacterOwner->LeftMotionController))
		{
			BaseVRCharacterOwner->LeftMotionController->TeleportMoveGrips(false, false);
			BaseVRCharacterOwner->LeftMotionController->PostTeleportMoveGrippedObjects();
		}

		if (IsValid(BaseVRCharacterOwner->RightMotionController))
		{
			BaseVRCharacterOwner->RightMotionController->TeleportMoveGrips(false, false);
			BaseVRCharacterOwner->RightMotionController->PostTeleportMoveGrippedObjects();
		}

	}
}

void UVRBaseCharacterMovementComponent::SimulatedTick(float DeltaSeconds)
{

	QUICK_SCOPE_CYCLE_COUNTER(STAT_Character_CharacterMovementSimulated);
	checkSlow(CharacterOwner != nullptr);

	if (CharacterOwner->IsPlayingNetworkedRootMotionMontage())
	{
		bWasSimulatingRootMotion = true;
		UE_LOGF(LogRootMotion, Verbose, "UCharacterMovementComponent::SimulatedTick");

		if (CharacterOwner && CharacterOwner->GetMesh())
		{
			TickCharacterPose(DeltaSeconds);

			if (!HasValidData())
			{
				return;
			}
		}

		const FQuat OldRotationQuat = UpdatedComponent->GetComponentQuat();
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();

		USkeletalMeshComponent* Mesh = CharacterOwner->GetMesh();
		const FVector SavedMeshRelativeLocation = Mesh ? Mesh->GetRelativeLocation() : FVector::ZeroVector;

		if (RootMotionParams.bHasRootMotion)
		{
			SimulateRootMotion(DeltaSeconds, RootMotionParams.GetRootMotionTransform());

#if !(UE_BUILD_SHIPPING)

#endif 
		}

		if (CharacterOwner && (CharacterOwner->RootMotionRepMoves.Num() > 0))
		{
			CharacterOwner->SimulatedRootMotionPositionFixup(DeltaSeconds);
		}

		if (!bNetworkSmoothingComplete && (NetworkSmoothingMode == ENetworkSmoothingMode::Linear))
		{

			const FQuat NewCapsuleRotation = UpdatedComponent->GetComponentQuat();
			if (Mesh == CharacterOwner->GetMesh() && !NewCapsuleRotation.Equals(OldRotationQuat, 1e-6f) && ClientPredictionData)
			{

				ClientPredictionData->MeshRotationTarget = NewCapsuleRotation;
				Mesh->SetRelativeLocationAndRotation(SavedMeshRelativeLocation, CharacterOwner->GetBaseRotationOffset());
			}
		}
	}
	else if (CurrentRootMotion.HasActiveRootMotionSources())
	{

		bWasSimulatingRootMotion = true;
		UE_LOGF(LogRootMotion, Verbose, "UCharacterMovementComponent::SimulatedTick");

		bool bCorrectedToServer = false;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FQuat OldRotation = UpdatedComponent->GetComponentQuat();
		if (CharacterOwner->RootMotionRepMoves.Num() > 0)
		{

			FSimulatedRootMotionReplicatedMove& RootMotionRepMove = CharacterOwner->RootMotionRepMoves.Last();
			if (CharacterOwner->RestoreReplicatedMove(RootMotionRepMove))
			{
				bCorrectedToServer = true;
			}
			Acceleration = RootMotionRepMove.RootMotion.Acceleration;

			CharacterOwner->PostNetReceiveVelocity(RootMotionRepMove.RootMotion.LinearVelocity);
			LastUpdateVelocity = RootMotionRepMove.RootMotion.LinearVelocity;

			ConvertRootMotionServerIDsToLocalIDs(CurrentRootMotion, RootMotionRepMove.RootMotion.AuthoritativeRootMotion, RootMotionRepMove.Time);
			RootMotionRepMove.RootMotion.AuthoritativeRootMotion.CullInvalidSources();

			CurrentRootMotion.UpdateStateFrom(RootMotionRepMove.RootMotion.AuthoritativeRootMotion, true);

			UE_LOGF(LogRootMotion, Log, "\tClearing old moves in SimulatedTick (%d)", CharacterOwner->RootMotionRepMoves.Num());
			CharacterOwner->RootMotionRepMoves.Reset();
		}

		if (bNetworkGravityDirectionChanged)
		{
			SetGravityDirection(CharacterOwner->GetReplicatedGravityDirection());
			bNetworkGravityDirectionChanged = false;
		}

		if (bNetworkMovementModeChanged)
		{
			ApplyNetworkMovementMode(CharacterOwner->GetReplicatedMovementMode());
			bNetworkMovementModeChanged = false;
		}

		PerformMovement(DeltaSeconds);

		if (bCorrectedToServer || CurrentRootMotion.NeedsSimulatedSmoothing())
		{
			SmoothCorrection(OldLocation, OldRotation, UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat());
		}
	}

	else
	{

		if (bWasSimulatingRootMotion)
		{
			CharacterOwner->RootMotionRepMoves.Empty();
			CharacterOwner->OnRep_ReplicatedMovement();
			CharacterOwner->OnRep_ReplicatedBasedMovement();
			SetGravityDirection(GetCharacterOwner()->GetReplicatedGravityDirection());
			ApplyNetworkMovementMode(GetCharacterOwner()->GetReplicatedMovementMode());
		}

		if (CharacterOwner->IsReplicatingMovement() && UpdatedComponent)
		{

			const bool bPreventMeshMovement = !bNetworkSmoothingComplete;

			if(NetworkSmoothingMode != ENetworkSmoothingMode::Disabled)
			{
				const FScopedPreventAttachedComponentMove PreventMeshMove(bPreventMeshMovement ? BaseVRCharacterOwner->NetSmoother : nullptr);

				if (CharacterOwner->IsPlayingRootMotion())
				{

					if (bNetworkGravityDirectionChanged)
					{
						SetGravityDirection(CharacterOwner->GetReplicatedGravityDirection());
						bNetworkGravityDirectionChanged = false;
					}

					if (bNetworkMovementModeChanged)
					{
						ApplyNetworkMovementMode(CharacterOwner->GetReplicatedMovementMode());
						bNetworkMovementModeChanged = false;
					}

					PerformMovement(DeltaSeconds);
				}
				else
				{

						SimulateMovement(DeltaSeconds);
				}
			}
			else
			{
				if (CharacterOwner->IsPlayingRootMotion())
				{
					PerformMovement(DeltaSeconds);
				}
				else
				{
					SimulateMovement(DeltaSeconds);
				}
			}

		}

		if (bWasSimulatingRootMotion)
		{
			bWasSimulatingRootMotion = false;
		}
	}

	if (!bNetworkSmoothingComplete)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_Character_CharacterMovementSmoothClientPosition);
		SmoothClientPosition(DeltaSeconds);
	}
	else
	{
		UE_LOGF(LogVRBaseCharacterMovement, Verbose, "Skipping network smoothing for %ls.", *GetNameSafe(CharacterOwner));
	}
}

void UVRBaseCharacterMovementComponent::MoveAutonomous(
	float ClientTimeStamp,
	float DeltaTime,
	uint8 CompressedFlags,
	const FVector& NewAccel
)
{
	if (!HasValidData())
	{
		return;
	}

	UpdateFromCompressedFlags(CompressedFlags);
	CharacterOwner->CheckJumpInput(DeltaTime);

	Acceleration = ConstrainInputAcceleration(NewAccel);
	Acceleration = Acceleration.GetClampedToMaxSize(GetMaxAcceleration());
	AnalogInputModifier = ComputeAnalogInputModifier();

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentQuat();

	if (BaseVRCharacterOwner && NetworkSmoothingMode == ENetworkSmoothingMode::Exponential)
	{
		OldLocation = BaseVRCharacterOwner->OffsetComponentToWorld.GetTranslation();
		OldRotation = BaseVRCharacterOwner->OffsetComponentToWorld.GetRotation();
	}

	const bool bWasPlayingRootMotion = CharacterOwner->IsPlayingRootMotion();

	PerformMovement(DeltaTime);

	if (!HasValidData())
	{
		return;
	}

	if (CharacterOwner && !CharacterOwner->bClientUpdating && !CharacterOwner->IsPlayingRootMotion() && CharacterOwner->GetMesh())
	{
		if (!bWasPlayingRootMotion) 
		{
			TickCharacterPose(DeltaTime);
		}

		USkeletalMeshComponent* OwnerMesh = CharacterOwner->GetMesh();
		check(OwnerMesh != nullptr)

		static const auto CVarEnableQueuedAnimEventsOnServer = IConsoleManager::Get().FindConsoleVariable(TEXT("a.EnableQueuedAnimEventsOnServer"));
		if (!CVarEnableQueuedAnimEventsOnServer->GetInt())

		{

			if (OwnerMesh->ShouldOnlyTickMontages(DeltaTime) || OwnerMesh->ShouldOnlyTickMontagesAndRefreshBones(DeltaTime))
			{
				OwnerMesh->ConditionallyDispatchQueuedAnimEvents();
			}
		}
	}

	if (CharacterOwner && UpdatedComponent)
	{

		static const auto CVarNetEnableListenServerSmoothing = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetEnableListenServerSmoothing"));
		if (CVarNetEnableListenServerSmoothing->GetInt() &&
			CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy &&
			IsNetMode(NM_ListenServer))
		{
			SmoothCorrection(OldLocation, OldRotation, UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat());
		}
	}
}

void UVRBaseCharacterMovementComponent::SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation, const FVector& NewLocation, const FQuat& NewRotation)
{

	if (!HasValidData())
	{
		return;
	}

	if (!BaseVRCharacterOwner)
		Super::SmoothCorrection(OldLocation, OldRotation, NewLocation, NewRotation);

	checkSlow(GetNetMode() != NM_DedicatedServer);

	const bool bIsSimulatedProxy = (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy);
	const bool bIsRemoteAutoProxy = (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy);
	ensure(bIsSimulatedProxy || bIsRemoteAutoProxy);

	bNetworkSmoothingComplete = false;

	if (NetworkSmoothingMode == ENetworkSmoothingMode::Disabled || GetNetMode() == NM_Standalone)
	{
		UpdatedComponent->SetWorldLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
		bNetworkSmoothingComplete = true;
	}
	else if (FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character())
	{
		const UWorld* MyWorld = GetWorld();
		if (!ensure(MyWorld != nullptr))
		{
			return;
		}

		FVector OldWorldLocation = OldLocation;
		FQuat OldWorldRotation = OldRotation;
		FVector NewWorldLocation = NewLocation;
		FQuat NewWorldRotation = NewRotation;

		if (BaseVRCharacterOwner && NetworkSmoothingMode == ENetworkSmoothingMode::Exponential)
		{
			if (GetNetMode() < ENetMode::NM_Client)
			{
				NewWorldLocation = BaseVRCharacterOwner->OffsetComponentToWorld.GetTranslation();
				NewWorldRotation = BaseVRCharacterOwner->OffsetComponentToWorld.GetRotation();
			}
			else
			{
				FTransform NewWorldTransform(NewRotation, NewLocation, UpdatedComponent->GetRelativeScale3D());
				FTransform CurrentRelative = BaseVRCharacterOwner->OffsetComponentToWorld.GetRelativeTransform(UpdatedComponent->GetComponentTransform());
				FTransform NewWorld = CurrentRelative * NewWorldTransform;
				OldWorldLocation = BaseVRCharacterOwner->OffsetComponentToWorld.GetLocation();
				OldWorldRotation = BaseVRCharacterOwner->OffsetComponentToWorld.GetRotation();
				NewWorldLocation = NewWorld.GetLocation();
				NewWorldRotation = NewWorld.GetRotation();
			}
		}

		FVector NewToOldVector = (OldWorldLocation - NewWorldLocation);
		if (bIsNavWalkingOnServer && FMath::Abs(GetGravitySpaceZ(NewToOldVector)) < NavWalkingFloorDistTolerance)
		{

			NewToOldVector = ProjectToGravityFloor(NewToOldVector);
		}

		const float DistSq = NewToOldVector.SizeSquared();
		if (DistSq > FMath::Square(ClientData->MaxSmoothNetUpdateDist))
		{
			ClientData->MeshTranslationOffset = (DistSq > FMath::Square(ClientData->NoSmoothNetUpdateDist))
				? FVector::ZeroVector
				: ClientData->MeshTranslationOffset + ClientData->MaxSmoothNetUpdateDist * NewToOldVector.GetSafeNormal();
		}
		else
		{
			ClientData->MeshTranslationOffset = ClientData->MeshTranslationOffset + NewToOldVector;
		}

		if (NetworkSmoothingMode == ENetworkSmoothingMode::Linear)
		{

			if ((!OldRotation.Equals(NewRotation, 1e-5f)))
			{
				if (BaseVRCharacterOwner->NetSmoother)
				{
					BaseVRCharacterOwner->NetSmoother->SetRelativeLocation(BaseVRCharacterOwner->bRetainRoomscale ? FVector::ZeroVector : BaseVRCharacterOwner->VRRootReference->GetTargetHeightOffset());			

				}
				UpdatedComponent->SetWorldLocationAndRotation(NewLocation, NewRotation, false, nullptr, GetTeleportType());
				ClientData->MeshTranslationOffset = FVector::ZeroVector;
				ClientData->MeshRotationOffset = ClientData->MeshRotationTarget;
				bNetworkSmoothingComplete = true;
			}
			else
			{
				ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;

				ClientData->OriginalMeshRotationOffset = OldRotation;
				ClientData->MeshRotationOffset = OldRotation;
				ClientData->MeshRotationTarget = NewRotation;

				if (NewLocation != OldLocation)
				{
					const FScopedPreventAttachedComponentMove PreventMeshMove(BaseVRCharacterOwner->NetSmoother);
					UpdatedComponent->SetWorldLocation(NewLocation, false, nullptr, GetTeleportType());
				}
			}
		}
		else
		{

			{			

				ClientData->MeshRotationOffset = FQuat::Identity;
				ClientData->MeshRotationTarget = FQuat::Identity;

				const FScopedPreventAttachedComponentMove PreventMeshMove(BaseVRCharacterOwner->NetSmoother);
				UpdatedComponent->SetWorldLocationAndRotation(NewLocation, NewRotation, false, nullptr, GetTeleportType());
			}
		}

		if (ClientData->SmoothingClientTimeStamp > ClientData->SmoothingServerTimeStamp)
		{
			const double OldClientTimeStamp = ClientData->SmoothingClientTimeStamp;
			ClientData->SmoothingClientTimeStamp = FMath::LerpStable(ClientData->SmoothingServerTimeStamp, OldClientTimeStamp, 0.5);

			UE_LOGF(LogVRBaseCharacterMovement, VeryVerbose, "SmoothCorrection: Pull back client from ClientTimeStamp: %.6f to %.6f, ServerTimeStamp: %.6f for %ls",
				OldClientTimeStamp, ClientData->SmoothingClientTimeStamp, ClientData->SmoothingServerTimeStamp, *GetNameSafe(CharacterOwner));

		}

		double OldServerTimeStamp = ClientData->SmoothingServerTimeStamp;

		if (bIsSimulatedProxy)
		{

			ServerLastTransformUpdateTimeStamp = CharacterOwner->GetReplicatedServerLastTransformUpdateTimeStamp();
		}
		ClientData->SmoothingServerTimeStamp = ServerLastTransformUpdateTimeStamp;

		if (ClientData->LastCorrectionTime == 0)
		{
			ClientData->SmoothingClientTimeStamp = ClientData->SmoothingServerTimeStamp;
			OldServerTimeStamp = ClientData->SmoothingServerTimeStamp;
		}

		const double ServerDeltaTime = ClientData->SmoothingServerTimeStamp - OldServerTimeStamp;
		const double MaxOffset = ClientData->MaxClientSmoothingDeltaTime;
		const double MinOffset = FMath::Min(double(ClientData->SmoothNetUpdateTime), MaxOffset);

		const double MaxDelta = FMath::Clamp(ServerDeltaTime * 1.25, MinOffset, MaxOffset);
		ClientData->SmoothingClientTimeStamp = FMath::Clamp(ClientData->SmoothingClientTimeStamp, ClientData->SmoothingServerTimeStamp - MaxDelta, ClientData->SmoothingServerTimeStamp);

		ClientData->LastCorrectionDelta = ClientData->SmoothingServerTimeStamp - ClientData->SmoothingClientTimeStamp;
		ClientData->LastCorrectionTime = MyWorld->GetTimeSeconds();

		UE_LOGF(LogVRBaseCharacterMovement, VeryVerbose, "SmoothCorrection: WorldTime: %.6f, ServerTimeStamp: %.6f, ClientTimeStamp: %.6f, Delta: %.6f for %ls",
			MyWorld->GetTimeSeconds(), ClientData->SmoothingServerTimeStamp, ClientData->SmoothingClientTimeStamp, ClientData->LastCorrectionDelta, *GetNameSafe(CharacterOwner));

	}
}

void UVRBaseCharacterMovementComponent::SmoothClientPosition(float DeltaSeconds)
{
	if (!HasValidData() || NetworkSmoothingMode == ENetworkSmoothingMode::Disabled)
	{
		return;
	}

	checkSlow(GetNetMode() != NM_DedicatedServer);
	checkSlow(GetNetMode() != NM_Standalone);

	const bool bIsSimulatedProxy = (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy);
	const bool bIsRemoteAutoProxy = (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy);
	if (!ensure(bIsSimulatedProxy || bIsRemoteAutoProxy))
	{
		return;
	}

	SmoothClientPosition_Interpolate(DeltaSeconds);

	SmoothClientPosition_UpdateVRVisuals();
}

void UVRBaseCharacterMovementComponent::SmoothClientPosition_UpdateVRVisuals()
{

	FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();

	if (!BaseVRCharacterOwner || !ClientData)
		return;

	if (ClientData)
	{
		if (NetworkSmoothingMode == ENetworkSmoothingMode::Linear && BaseVRCharacterOwner->NetSmoother)
		{

			const USceneComponent* MeshParent = BaseVRCharacterOwner->NetSmoother->GetAttachParent();
			FVector MeshParentScale = MeshParent != nullptr ? MeshParent->GetComponentScale() : FVector(1.0f, 1.0f, 1.0f);

			MeshParentScale.X = FMath::IsNearlyZero(MeshParentScale.X) ? 1.0f : MeshParentScale.X;
			MeshParentScale.Y = FMath::IsNearlyZero(MeshParentScale.Y) ? 1.0f : MeshParentScale.Y;
			MeshParentScale.Z = FMath::IsNearlyZero(MeshParentScale.Z) ? 1.0f : MeshParentScale.Z;

			const FVector NewRelLocation = ClientData->MeshRotationOffset.UnrotateVector(ClientData->MeshTranslationOffset);

			FVector HeightOffset = (BaseVRCharacterOwner->bRetainRoomscale ? FVector::ZeroVector : BaseVRCharacterOwner->VRRootReference->GetTargetHeightOffset());
			BaseVRCharacterOwner->NetSmoother->SetRelativeLocation(NewRelLocation + HeightOffset);
		}
		else if (NetworkSmoothingMode == ENetworkSmoothingMode::Exponential && BaseVRCharacterOwner->NetSmoother)
		{
			const USceneComponent* MeshParent = BaseVRCharacterOwner->NetSmoother->GetAttachParent();
			FVector MeshParentScale = MeshParent != nullptr ? MeshParent->GetComponentScale() : FVector(1.0f, 1.0f, 1.0f);

			MeshParentScale.X = FMath::IsNearlyZero(MeshParentScale.X) ? 1.0f : MeshParentScale.X;
			MeshParentScale.Y = FMath::IsNearlyZero(MeshParentScale.Y) ? 1.0f : MeshParentScale.Y;
			MeshParentScale.Z = FMath::IsNearlyZero(MeshParentScale.Z) ? 1.0f : MeshParentScale.Z;

			const FVector NewRelTranslation = (UpdatedComponent->GetComponentToWorld().InverseTransformVectorNoScale(ClientData->MeshTranslationOffset) / MeshParentScale);
			const FQuat NewRelRotation = ClientData->MeshRotationOffset;

			FVector HeightOffset = BaseVRCharacterOwner->bRetainRoomscale ? FVector::ZeroVector : BaseVRCharacterOwner->VRRootReference->GetTargetHeightOffset();
			BaseVRCharacterOwner->NetSmoother->SetRelativeLocationAndRotation(NewRelTranslation + HeightOffset, NewRelRotation);
		}
		else
		{

		}
	}
}

void UVRBaseCharacterMovementComponent::SetHasRequestedVelocity(bool bNewHasRequestedVelocity)
{
	bHasRequestedVelocity = bNewHasRequestedVelocity;
}

bool UVRBaseCharacterMovementComponent::IsClimbing() const
{
	return ((MovementMode == MOVE_Custom) && (CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Climbing)) && UpdatedComponent;
}

FVector UVRBaseCharacterMovementComponent::RewindVRMovement()
{
	RewindVRRelativeMovement();
	return AdditionalVRInputVector;
}

FVector UVRBaseCharacterMovementComponent::GetCustomInputVector()
{
	return CustomVRInputVector;
}

void UVRBaseCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{

	Super::UpdateFromCompressedFlags(Flags);
}

FVector UVRBaseCharacterMovementComponent::RoundDirectMovement(FVector InMovement) const
{

	UE::Net::QuantizeVector(100, InMovement);

	return InMovement;
}

void UVRBaseCharacterMovementComponent::SetAutoOrientToFloorNormal(bool bAutoOrient, bool bRevertGravityWhenDisabled)
{
	bAutoOrientToFloorNormal = bAutoOrient;

	if (!bAutoOrientToFloorNormal && bRevertGravityWhenDisabled)
	{

		SetCharacterToNewGravity(FVector(0.0f, 0.0f, -1.0f), true);
	}
}

void UVRBaseCharacterMovementComponent::AutoTraceAndSetCharacterToNewGravity(FHitResult & TargetFloor, float DeltaTime)
{
	if (TargetFloor.Component.IsValid())
	{

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AutoTraceFloorNormal), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

		FVector TraceStart = BaseVRCharacterOwner->GetVRLocation_Inline();
		FVector Offset = (-UpdatedComponent->GetComponentQuat().GetUpVector()) * (BaseVRCharacterOwner->VRRootReference->GetScaledCapsuleHalfHeight() + 10.0f);

		FHitResult OutHit;

		bool bDidHit = GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceStart + Offset, CollisionChannel, QueryParams, ResponseParam);

		if (!bDidHit)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Didn't hit!"));
		}
		else
		{
			if (!IsWalkable(OutHit))
			{
				bDidHit = false;
			}
		}

		if (bDidHit)
		{
			FVector NewGravityDir = -OutHit.Normal;

			float AngleOfChange = FMath::Abs(FMath::RadiansToDegrees(acosf(FVector::DotProduct(-OutHit.Normal, GetGravityDirection()))));
			if (AngleOfChange > GetWalkableFloorAngle() || AngleOfChange < 0.01f)
			{
				return;
			}
			else
			{
				if (bBlendGravityFloorChanges)
				{	

				}
			}

			SetCharacterToNewGravity(NewGravityDir, true, DeltaTime);
		}
	}
}

bool UVRBaseCharacterMovementComponent::SetCharacterToNewGravity(FVector NewGravityDirection, bool bOrientToNewGravity, float DeltaTime)
{

	NewGravityDirection.Normalize();

	if (NewGravityDirection.Equals(GetGravityDirection()))
		return false;

	if (bOrientToNewGravity && IsValid(BaseVRCharacterOwner))
	{
		FQuat CurrentRotQ = UpdatedComponent->GetComponentQuat();
		FQuat DeltaRot = FQuat::FindBetweenNormals(-CurrentRotQ.GetUpVector(), NewGravityDirection);

		if (bBlendGravityFloorChanges)
		{

			const float Alpha = FMath::Clamp(DeltaTime * FloorOrientationChangeBlendRate, 0.f, 1.f);
			DeltaRot = FQuat::Slerp(FQuat::Identity, DeltaRot, Alpha);

			bIsBlendingOrientation = true;
		}

		FQuat NewRot = (DeltaRot * CurrentRotQ);
		NewRot.Normalize();

		NewGravityDirection = -NewRot.GetUpVector();

		AController* OwningController = BaseVRCharacterOwner->GetController();

		float PivotZ = BaseVRCharacterOwner->bRetainRoomscale ? 0.0f : -BaseVRCharacterOwner->VRRootReference->GetUnscaledCapsuleHalfHeight();
		FQuat NewRotation = NewRot;

		FTransform BaseTransform = BaseVRCharacterOwner->VRRootReference->GetComponentTransform();
		FVector PivotPoint = BaseTransform.TransformPosition(FVector(0.0f, 0.0f, PivotZ));

		FVector BasePoint = PivotPoint; 
		const FTransform PivotToWorld = FTransform(FQuat::Identity, BasePoint);
		const FTransform WorldToPivot = FTransform(FQuat::Identity, -BasePoint);

		FTransform NewTransform = BaseTransform * WorldToPivot * FTransform(DeltaRot, FVector::ZeroVector, FVector(1.0f)) * PivotToWorld;

		MoveUpdatedComponent(NewTransform.GetLocation() - BaseTransform.GetLocation(), NewRotation,  false);

		if (BaseVRCharacterOwner->bUseControllerRotationYaw && OwningController)
			OwningController->SetControlRotation(NewRotation.Rotator());

		SetGravityDirection(NewGravityDirection);
		return true;
	}
	else
	{
		SetGravityDirection(NewGravityDirection);
		return true;
	}

}