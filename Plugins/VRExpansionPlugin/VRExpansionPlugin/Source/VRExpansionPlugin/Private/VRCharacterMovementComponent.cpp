

#include "VRCharacterMovementComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRCharacterMovementComponent)

#include "GameFramework/PhysicsVolume.h"
#include "GameFramework/GameNetworkManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameState.h"
#include "GameFramework/WorldSettings.h"
#include "Components/PrimitiveComponent.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"
#include "ObjectCacheContext.h"

#include "VRCharacter.h"
#include "VRExpansionFunctionLibrary.h"

#include "Chaos/ChaosUserEntity.h"

#include "Navigation/PathFollowingComponent.h"
#include "AI/Navigation/AvoidanceManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/BrushComponent.h"

#include "Engine/DemoNetDriver.h"
#include "Engine/NetworkObjectList.h"
#include "UObject/Package.h"

#include "VRRootComponent.h"
#include "WorldCollision.h"
#include "Runtime/Launch/Resources/Version.h"
#include "GameFramework/CharacterMovementReplication.h"
#include "Interfaces/NetworkPredictionInterface.h"

#include "PhysicsEngine/PhysicsBodyInstanceOwnerInterface.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include "Runtime/Experimental/Chaos/Private/Chaos/PhysicsObjectInternal.h"

#if UE_WITH_REMOTE_OBJECT_HANDLE
#include "UObject/RemoteExecutor.h"
#endif

DEFINE_LOG_CATEGORY(LogVRCharacterMovement);

DECLARE_CYCLE_STAT(TEXT("Char StepUp"), STAT_CharStepUp, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char FindFloor"), STAT_CharFindFloor, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char ReplicateMoveToServer"), STAT_CharacterMovementReplicateMoveToServer, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char CallServerMove"), STAT_CharacterMovementCallServerMove, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char CombineNetMove"), STAT_CharacterMovementCombineNetMove, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PhysWalking"), STAT_CharPhysWalking, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PhysFalling"), STAT_CharPhysFalling, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PhysNavWalking"), STAT_CharPhysNavWalking, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char NavProjectPoint"), STAT_CharNavProjectPoint, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char NavProjectLocation"), STAT_CharNavProjectLocation, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char AdjustFloorHeight"), STAT_CharAdjustFloorHeight, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char ProcessLanded"), STAT_CharProcessLanded, STATGROUP_Character);

namespace CharacterMovementConstants
{

	const float MAX_STEP_SIDE_ZVR = 0.08f;	
	const float SWIMBOBSPEEDVR = -80.f;
	const float VERTICAL_SLOPE_NORMAL_ZVR = 0.001f; 

}

#if DO_CHECK && !UE_BUILD_SHIPPING 
#define devCodeVR( Code )		checkCode( Code )
#else
#define devCodeVR(...)
#endif

namespace CharacterMovementComponentStatics
{
	static const FName CrouchTraceName = FName(TEXT("CrouchTrace"));
	static const FName ImmersionDepthName = FName(TEXT("MovementComp_Character_ImmersionDepth"));

	static float fRotationCorrectionThreshold = 0.02f;
	FAutoConsoleVariableRef CVarRotationCorrectionThreshold(
		TEXT("vre.RotationCorrectionThreshold"),
		fRotationCorrectionThreshold,
		TEXT("Error threshold value before correcting a clients rotation.\n")
		TEXT("Rotation is replicated at 2 decimal precision, so values less than 0.01 won't matter."),
		ECVF_Default);

	static float fRotationChangingCorrectionThreshold = 0.3f;
	FAutoConsoleVariableRef CVarRotationChangingCorrectionThreshold(
		TEXT("vre.RotationChangingCorrectionThreshold"),
		fRotationChangingCorrectionThreshold,
		TEXT("Error threshold value before correcting a clients rotation when actively changing rotation.\n")
		TEXT("Rotation is replicated at 2 decimal precision, so values less than 0.01 won't matter."),
		ECVF_Default);
}

void UVRCharacterMovementComponent::StoreSetTrackingPaused(bool bNewTrackingPaused)
{
	FVRMoveActionContainer MoveAction;
	MoveAction.MoveAction = EVRMoveAction::VRMOVEACTION_PauseTracking;
	MoveAction.MoveActionFlags = bNewTrackingPaused;
	MoveAction.MoveActionLoc = VRRootCapsule->curCameraLoc;
	MoveAction.MoveActionRot = VRRootCapsule->StoredCameraRotOffset;
	MoveActionArray.MoveActions.Add(MoveAction);
	CheckServerAuthedMoveAction();
}

void UVRCharacterMovementComponent::Crouch(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (!bClientSimulation && !CanCrouchInCurrentState())
	{
		return;
	}

	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == GetCrouchedHalfHeight())
	{
		if (!bClientSimulation)
		{
			CharacterOwner->SetIsCrouched(true);
		}
		CharacterOwner->OnStartCrouch(0.f, 0.f);
		return;
	}

	if (bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{

		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		if (VRRootCapsule)
			VRRootCapsule->SetCapsuleSizeVR(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
		else
			CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());

		bShrinkProxyCapsule = true;
	}

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();

	const float ClampedCrouchedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, GetCrouchedHalfHeight());

	if (VRRootCapsule)
		VRRootCapsule->SetCapsuleSizeVR(OldUnscaledRadius, ClampedCrouchedHalfHeight);
	else
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedCrouchedHalfHeight);

	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedCrouchedHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	if (!bClientSimulation)
	{

		if (ClampedCrouchedHalfHeight > OldUnscaledHalfHeight)
		{
			FCollisionQueryParams CapsuleParams(CharacterMovementComponentStatics::CrouchTraceName, false, CharacterOwner);
			FCollisionResponseParams ResponseParam;
			InitCollisionParams(CapsuleParams, ResponseParam);

			FVector capLocation;
			if (VRRootCapsule)
			{
				capLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();
			}
			else
				capLocation = UpdatedComponent->GetComponentLocation();

			const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(capLocation - (ScaledHalfHeightAdjust * GetGravityDirection()), FQuat::Identity,
				UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (VRRootCapsule)
					VRRootCapsule->SetCapsuleSizeVR(OldUnscaledRadius, OldUnscaledHalfHeight);
				else
					CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, OldUnscaledHalfHeight);
				return;
			}
		}

		if (bCrouchMaintainsBaseLocation)
		{

		}

		CharacterOwner->SetIsCrouched(true);
	}

	bForceNextFloorCheck = true;

	const float MeshAdjust = ScaledHalfHeightAdjust;
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedCrouchedHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	CharacterOwner->OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

}

void UVRCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		if (!bClientSimulation)
		{
			CharacterOwner->SetIsCrouched(false);
		}
		CharacterOwner->OnEndCrouch(0.f, 0.f);
		return;
	}

	const float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	 FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	if (VRRootCapsule)
	{
		PawnLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();
	}

	check(CharacterOwner->GetCapsuleComponent());

	if (!bClientSimulation)
	{

		const UWorld* MyWorld = GetWorld();
		const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(CharacterMovementComponentStatics::CrouchTraceName, false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); 
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		if (!bCrouchMaintainsBaseLocation)
		{

			bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{

				if (ScaledHalfHeightAdjust > 0.f)
				{

					float PawnRadius, PawnHalfHeight;
					CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					const FVector Down = TraceDist * GetGravityDirection();

					FHitResult Hit(1.f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
					const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, GetWorldToGravityTransform(), CollisionChannel, ShortCapsuleShape, CapsuleParams);
					if (Hit.bStartPenetrating)
					{
						bEncroached = true;
					}
					else
					{

						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector Adjustment = (-DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f) * -GetGravityDirection();
						const FVector NewLoc = PawnLocation + Adjustment;
						bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{

							UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
		else
		{

			FVector StandingLocation = PawnLocation + (StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight) * -GetGravityDirection();
			bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (IsMovingOnGround())
				{

					const float MinFloorDist = UE_KINDA_SMALL_NUMBER * 10.f;
					if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
					{
						StandingLocation -= (CurrentFloor.FloorDist - MinFloorDist) * -GetGravityDirection();
						bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					}
				}
			}

			if (!bEncroached)
			{

				if (!BaseVRCharacterOwner || !BaseVRCharacterOwner->bRetainRoomscale)
				{

				}
				bForceNextFloorCheck = true;
			}
		}

		if (bEncroached)
		{
			return;
		}

		CharacterOwner->SetIsCrouched(false);
	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	if (VRRootCapsule)
		VRRootCapsule->SetCapsuleSizeVR(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	else
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);

	const float MeshAdjust = ScaledHalfHeightAdjust;
	AdjustProxyCapsuleSize();
	CharacterOwner->OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

}

FNetworkPredictionData_Client* UVRCharacterMovementComponent::GetPredictionData_Client() const
{

	check(CharacterOwner != NULL);
	checkSlow(CharacterOwner->GetLocalRole() < ROLE_Authority || (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy && GetNetMode() == NM_ListenServer));
	checkSlow(GetNetMode() == NM_Client || GetNetMode() == NM_ListenServer);

	if (!ClientPredictionData)
	{
		UVRCharacterMovementComponent* MutableThis = const_cast<UVRCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_VRCharacter(*this);
	}

	return ClientPredictionData;
}

FNetworkPredictionData_Server* UVRCharacterMovementComponent::GetPredictionData_Server() const
{

	check(CharacterOwner != NULL);
	check(CharacterOwner->GetLocalRole() == ROLE_Authority);
	checkSlow(GetNetMode() < NM_Client);

	if (!ServerPredictionData)
	{
		UVRCharacterMovementComponent* MutableThis = const_cast<UVRCharacterMovementComponent*>(this);
		MutableThis->ServerPredictionData = new FNetworkPredictionData_Server_VRCharacter(*this);
	}

	return ServerPredictionData;
}

void FSavedMove_VRCharacter::SetInitialPosition(ACharacter* C)
{

	if (AVRCharacter * VRC = Cast<AVRCharacter>(C))
	{
		UVRCharacterMovementComponent * CharMove = Cast<UVRCharacterMovementComponent>(VRC->GetCharacterMovement());
		if (VRC->VRRootReference)
		{
			VRCapsuleLocation = VRC->VRRootReference->curCameraLoc;
			VRCapsuleRotation = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(VRC->VRRootReference->curCameraRot);

			if (LFDiff.IsZero())
			{
				LFDiff = VRC->VRRootReference->DifferenceFromLastFrame;
			}
		}
		else
		{
			VRCapsuleLocation = FVector::ZeroVector;
			VRCapsuleRotation = FRotator::ZeroRotator;
			LFDiff = FVector::ZeroVector;
		}
	}

	FSavedMove_VRBaseCharacter::SetInitialPosition(C);
}

void FSavedMove_VRCharacter::PrepMoveFor(ACharacter* Character)
{
	UVRCharacterMovementComponent * CharMove = Cast<UVRCharacterMovementComponent>(Character->GetCharacterMovement());

	if (CharMove && CharMove->VRRootCapsule)
	{
		CharMove->VRRootCapsule->curCameraLoc = this->VRCapsuleLocation;
		CharMove->VRRootCapsule->curCameraRot = this->VRCapsuleRotation;
		CharMove->VRRootCapsule->DifferenceFromLastFrame = LFDiff;
		CharMove->AdditionalVRInputVector = CharMove->VRRootCapsule->DifferenceFromLastFrame;

		if (AVRBaseCharacter * BaseChar = Cast<AVRBaseCharacter>(CharMove->GetCharacterOwner()))
		{
			if (BaseChar->GetVRReplicateCapsuleHeight() && this->CapsuleHeight > 0.0f && !FMath::IsNearlyEqual(this->CapsuleHeight, CharMove->VRRootCapsule->GetUnscaledCapsuleHalfHeight()))
			{
				BaseChar->SetCharacterHalfHeightVR(CapsuleHeight, false);

			}
		}

		CharMove->VRRootCapsule->StoredCameraRotOffset = CharMove->VRRootCapsule->curCameraRot;
		CharMove->VRRootCapsule->GenerateOffsetToWorld(false, false);
	}

	FSavedMove_VRBaseCharacter::PrepMoveFor(Character);
}

void UVRCharacterMovementComponent::RegenerateOffset()
{
	if(VRRootCapsule)
		VRRootCapsule->GenerateOffsetToWorld();
}

void UVRCharacterMovementComponent::ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData)
{
	QUICK_SCOPE_CYCLE_COUNTER(VRCharacterMovementServerMove_PerformMovement);

	if (!HasValidData() || !IsActive())
	{
		return;
	}

	bool bAutoAcceptPacket = false;

	FNetworkPredictionData_Server_Character* ServerData = GetPredictionData_Server_Character();
	check(ServerData);

	const FVRCharacterNetworkMoveData* MoveDataVR = (const FVRCharacterNetworkMoveData*)&MoveData;

	if (MovementMode == MOVE_Custom && CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Seated)
	{
		return;
	}
	else if (bJustUnseated && MoveDataVR->ReplicatedMovementMode != EVRConjoinedMovementModes::C_VRMOVE_Seated)
	{	

		ServerData->CurrentClientTimeStamp = MoveData.TimeStamp;
		bAutoAcceptPacket = true;
		bJustUnseated = false;
	}

	const float ClientTimeStamp = MoveData.TimeStamp;
	FVector ClientAccel = MoveData.Acceleration;

	static const auto CVarNetUseBaseRelativeAcceleration = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetUseBaseRelativeAcceleration"));

	FMovementBaseInterfaceData MovementBaseInterfaceData = MovementBaseUtility::GetMovementBaseDataFromPhysicsOwner(MoveData.MovementBasePhysicsObjectOwner);
	if (CVarNetUseBaseRelativeAcceleration->GetInt() && MovementBaseUtility::IsDynamicBase(&MovementBaseInterfaceData))
	{
		MovementBaseUtility::TransformDirectionToWorld(&MovementBaseInterfaceData, MoveData.MovementBaseBoneName, MoveData.Acceleration, ClientAccel);
	}

	const uint8 ClientMoveFlags = MoveData.CompressedMoveFlags;
	const FRotator ClientControlRotation = MoveData.ControlRotation;

	if (!bAutoAcceptPacket && !VerifyClientTimeStamp(ClientTimeStamp, *ServerData))
	{
		const float ServerTimeStamp = ServerData->CurrentClientTimeStamp;

		static const auto CVarNetServerMoveTimestampExpiredWarningThreshold = IConsoleManager::Get().FindConsoleVariable(TEXT("net.NetServerMoveTimestampExpiredWarningThreshold"));
		if (ServerTimeStamp > 1.0f && FMath::Abs(ServerTimeStamp - ClientTimeStamp) > CVarNetServerMoveTimestampExpiredWarningThreshold->GetFloat())
		{
			UE_LOGF(LogNetPlayerMovement, Warning, "ServerMove: TimeStamp expired: %f, CurrentTimeStamp: %f, Character: %ls", ClientTimeStamp, ServerTimeStamp, *GetNameSafe(CharacterOwner));
		}
		else
		{
			UE_LOGF(LogNetPlayerMovement, Log, "ServerMove: TimeStamp expired: %f, CurrentTimeStamp: %f, Character: %ls", ClientTimeStamp, ServerTimeStamp, *GetNameSafe(CharacterOwner));
		}
		return;
	}

	FVRCharacterScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);

	bool bServerReadyForClient = true;
	APlayerController* PC = Cast<APlayerController>(CharacterOwner->GetController());
	if (PC)
	{
		bServerReadyForClient = PC->NotifyServerReceivedClientData(CharacterOwner, ClientTimeStamp);
		if (!bServerReadyForClient)
		{
			ClientAccel = FVector::ZeroVector;
		}
	}

	const UWorld* MyWorld = GetWorld();
	 float DeltaTime = 0.0f;

	if (bAutoAcceptPacket)
	{
		DeltaTime = ServerData->MaxMoveDeltaTime * CharacterOwner->GetActorTimeDilation(*MyWorld);
	}
	else
	{
		DeltaTime = ServerData->GetServerMoveDeltaTime(ClientTimeStamp, CharacterOwner->GetActorTimeDilation(*MyWorld));
	}

	if (DeltaTime > 0.f)
	{
		ServerData->CurrentClientTimeStamp = ClientTimeStamp;
		ServerData->ServerAccumulatedClientTimeStamp += DeltaTime;
		ServerData->ServerTimeStamp = MyWorld->GetTimeSeconds();
		ServerData->ServerTimeStampLastServerMove = ServerData->ServerTimeStamp;

		if (bUseClientControlRotation)
		{
			if (AController* CharacterController = Cast<AController>(CharacterOwner->GetController()))
			{
				CharacterController->SetControlRotation(ClientControlRotation);
			}
		}

		if (!bServerReadyForClient)
		{
			return;
		}

		if ((MyWorld->GetWorldSettings()->GetPauserPlayerState() == NULL))
		{
			if (PC)
			{
				PC->UpdateRotation(DeltaTime);
			}

			if (!MoveDataVR->ConditionalMoveReps.RequestedVelocity.IsZero())
			{
				RequestedVelocity = MoveDataVR->ConditionalMoveReps.RequestedVelocity;
				bHasRequestedVelocity = true;
			}

			CustomVRInputVector = MoveDataVR->ConditionalMoveReps.CustomVRInputVector;
			MoveActionArray = MoveDataVR->ConditionalMoveReps.MoveActionArray;
			VRReplicatedMovementMode = MoveDataVR->ReplicatedMovementMode;

			if (VRRootCapsule)
			{
				VRRootCapsule->curCameraLoc = MoveDataVR->VRCapsuleLocation;
				VRRootCapsule->curCameraRot = FRotator(0.0f, FRotator::DecompressAxisFromShort(MoveDataVR->VRCapsuleRotation), 0.0f);
				VRRootCapsule->DifferenceFromLastFrame = MoveDataVR->LFDiff;
				AdditionalVRInputVector = VRRootCapsule->DifferenceFromLastFrame;

				if (BaseVRCharacterOwner)
				{
					if (BaseVRCharacterOwner->GetVRReplicateCapsuleHeight() && MoveDataVR->CapsuleHeight > 0.0f && !FMath::IsNearlyEqual(MoveDataVR->CapsuleHeight, VRRootCapsule->GetUnscaledCapsuleHalfHeight()))
					{
						BaseVRCharacterOwner->SetCharacterHalfHeightVR(MoveDataVR->CapsuleHeight, false);

					}
				}

				VRRootCapsule->StoredCameraRotOffset = VRRootCapsule->curCameraRot;
				VRRootCapsule->GenerateOffsetToWorld(false, false);
			}

			MoveAutonomous(ClientTimeStamp, DeltaTime, ClientMoveFlags, ClientAccel);
			bHasRequestedVelocity = false;
		}

		UE_CLOGF(CharacterOwner&& UpdatedComponent, LogNetPlayerMovement, VeryVerbose, "ServerMove Time %f Acceleration %ls Velocity %ls Position %ls Rotation %ls GravityDirection %ls DeltaTime %f Mode %ls MovementBase %ls.%ls (Dynamic:%d)",
			ClientTimeStamp, *ClientAccel.ToString(), *Velocity.ToString(), *UpdatedComponent->GetComponentLocation().ToString(), *UpdatedComponent->GetComponentRotation().ToCompactString(), *GetGravityDirection().ToCompactString(), DeltaTime, *GetMovementName(),
			*GetNameSafe(MovementBaseInterfaceData.GetMovementBaseObject()), *CharacterOwner->GetBasedMovement().BoneName.ToString(), MovementBaseUtility::IsDynamicBase(&MovementBaseInterfaceData) ? 1 : 0);
	}

	const uint8 CurrentPackedMovementMode = PackNetworkMovementMode();
	if (CurrentPackedMovementMode != MoveData.MovementMode)
	{
		TEnumAsByte<EMovementMode> NetMovementMode(MOVE_None);
		TEnumAsByte<EMovementMode> NetGroundMode(MOVE_None);
		uint8 NetCustomMode(0);
		UnpackNetworkMovementMode(MoveData.MovementMode, NetMovementMode, NetCustomMode, NetGroundMode);

		if (NetMovementMode == EMovementMode::MOVE_Custom || MovementMode == EMovementMode::MOVE_Custom)
		{
			if (NetCustomMode == (uint8)EVRCustomMovementMode::VRMOVE_Climbing || CustomMovementMode == (uint8)EVRCustomMovementMode::VRMOVE_Climbing)
				SetMovementMode(NetMovementMode, NetCustomMode);
		}
	}

	if (MoveData.NetworkMoveType == FCharacterNetworkMoveData::ENetworkMoveType::NewMove)
	{
		ServerMoveHandleClientErrorVR(ClientTimeStamp, DeltaTime, ClientAccel, MoveData.Location, ClientControlRotation, &MovementBaseInterfaceData, MoveData.MovementBaseBoneName, MoveData.MovementMode);

	}
}

bool UVRCharacterMovementComponent::ShouldCheckForValidLandingSpot(float DeltaTime, const FVector& Delta, const FHitResult& Hit) const
{

	if (GetGravitySpaceZ(Hit.Normal) > UE_KINDA_SMALL_NUMBER && !Hit.Normal.Equals(Hit.ImpactNormal))
	{
		FVector PawnLocation = UpdatedComponent->GetComponentLocation();
		if (VRRootCapsule)
			PawnLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();

		if (IsWithinEdgeTolerance(PawnLocation, Hit.ImpactPoint, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius()))
		{
			return true;
		}
	}

	return false;
}

void UVRCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharPhysWalking);

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->GetController() && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Walking);
		return;
	}

	devCodeVR(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	const EMovementMode StartingMovementMode = MovementMode;
	const uint8 StartingCustomMovementMode = CustomMovementMode;

	RewindVRRelativeMovement();

	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->GetController() || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

#if UE_WITH_REMOTE_OBJECT_HANDLE

		const float LastFrameDt = GetWorld()->GetDeltaSeconds();
		PhysicsForceSubsteppingFactor = timeTick / LastFrameDt;
#endif

		FMovementBaseInterfaceData* OldMovementBaseInterfaceData = GetMovementBaseInterfaceData_Mutable();
		const FVector PreviousBaseLocation = OldMovementBaseInterfaceData && OldMovementBaseInterfaceData->IsValid() ? OldMovementBaseInterfaceData->GetBodyInstanceOwner()->GetPhysicsOwnerTransform().GetLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();

		FVector OldCapsuleLocation = VRRootCapsule ? VRRootCapsule->OffsetComponentToWorld.GetLocation() : OldLocation;

		const FFindFloorResult OldFloor = CurrentFloor;

		RestorePreAdditiveRootMotionVelocity();

		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration = FVector::VectorPlaneProject(Acceleration, -GetGravityDirection());

		static const auto CVarLedgeMovementApplyDirectMove = IConsoleManager::Get().FindConsoleVariable(TEXT("p.LedgeMovement.ApplyDirectMove"));

		const bool bSkipForLedgeMove = bTriedLedgeMove && CVarLedgeMovementApplyDirectMove->GetBool();
		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && !bSkipForLedgeMove)
		{
			CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration());
			devCodeVR(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
		}

		ApplyRootMotionToVelocity(timeTick);
		ApplyVRMotionToVelocity(deltaTime);

		devCodeVR(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
		{

			StartNewPhysics(remainingTime + timeTick, Iterations - 1);
			return;
		}

		const FVector MoveVelocity = Velocity;

		const FVector Delta = timeTick * MoveVelocity;

		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if (bZeroDelta)
		{
			remainingTime = 0.f;

		}
		else
		{

			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsSwimming()) 
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
			else if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
			{

				const float DesiredDist = Delta.Size();
				if (DesiredDist > UE_KINDA_SMALL_NUMBER)
				{
					const float ActualDist = ProjectToGravityFloor(UpdatedComponent->GetComponentLocation() - OldLocation).Size();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				RestorePreAdditiveVRMotionVelocity();
				StartNewPhysics(remainingTime, Iterations);
				return;
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

		const bool bCheckLedges = !CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{

			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, OldFloor);

			if (!NewDelta.IsZero())
			{

				RevertMove(OldLocation, OldMovementBaseInterfaceData, PreviousBaseLocation, OldFloor, false);

				bTriedLedgeMove = true;

				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				Iterations--;
				RestorePreAdditiveVRMotionVelocity();
				continue;
			}
			else
			{

				bCheckedFall = true;

				RevertMove(OldLocation, OldMovementBaseInterfaceData, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				RestorePreAdditiveVRMotionVelocity();
				break;
			}
		}
		else
		{

			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					RestorePreAdditiveVRMotionVelocity();
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{

						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBaseFromFloor(CurrentFloor);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{

				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + MAX_FLOOR_DIST * -GetGravityDirection();
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			if (IsSwimming())
			{
				RestorePreAdditiveVRMotionVelocity();
				StartSwimmingVR(OldCapsuleLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bOldMovementBaseValid = OldMovementBaseInterfaceData && OldMovementBaseInterfaceData->IsValid();
				const bool bMustJump = bJustTeleported || bZeroDelta || (!bOldMovementBaseValid || (!CollisionEnabledHasQuery(OldMovementBaseInterfaceData->GetBodyInstanceOwner()->GetCollisionEnabled()) && MovementBaseUtility::IsDynamicBase(OldMovementBaseInterfaceData)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					RestorePreAdditiveVRMotionVelocity();
					return;
				}
				bCheckedFall = true;
			}

			if (bAutoOrientToFloorNormal && CurrentFloor.IsWalkableFloor())
			{

				AutoTraceAndSetCharacterToNewGravity(CurrentFloor.HitResult, timeTick);
			}
		}

		if (IsMovingOnGround())
		{

			if (!bJustTeleported && timeTick >= MIN_TICK_TIME)
			{
				if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
				{

					Velocity = ((UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick);
					MaintainHorizontalGroundVelocity();
				}

				RestorePreAdditiveVRMotionVelocity();
			}
		}

		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			RestorePreAdditiveVRMotionVelocity();
			remainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
}

void UVRCharacterMovementComponent::CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bEnablePhysicsInteraction)
	{
		return;
	}

	if (OtherComp != NULL && OtherComp->IsAnySimulatingPhysics())
	{
		FVector OtherLoc = OtherComp->GetComponentLocation();
		if (UVRRootComponent * rCap = Cast<UVRRootComponent>(OtherComp))
		{
			OtherLoc = rCap->OffsetComponentToWorld.GetLocation();
		}

		const FVector Loc = VRRootCapsule->OffsetComponentToWorld.GetLocation();

		FVector ImpulseDir = OtherLoc - Loc;
		SetGravitySpaceZ(ImpulseDir, 0.25f);
		ImpulseDir = (ImpulseDir.GetSafeNormal() + ProjectToGravityFloor(Velocity).GetSafeNormal()) * 0.5f;
		ImpulseDir.Normalize();

		FName BoneName = NAME_None;
		if (OtherBodyIndex != INDEX_NONE)
		{
			BoneName = ((USkinnedMeshComponent*)OtherComp)->GetBoneName(OtherBodyIndex);
		}

		float TouchForceFactorModified = TouchForceFactor;

		if (bTouchForceScaledToMass)
		{
			FBodyInstance* BI = OtherComp->GetBodyInstance(BoneName);
			TouchForceFactorModified *= BI ? BI->GetBodyMass() : 1.0f;
		}

		float ImpulseStrength = FMath::Clamp<FVector::FReal>(ProjectToGravityFloor(Velocity).Size() * TouchForceFactorModified,
			MinTouchForce > 0.0f ? MinTouchForce : -FLT_MAX,
			MaxTouchForce > 0.0f ? MaxTouchForce : FLT_MAX);

		FVector Impulse = ImpulseDir * ImpulseStrength;

		OtherComp->AddImpulse(Impulse, BoneName);
	}
}

void UVRCharacterMovementComponent::ReplicateMoveToServer(float DeltaTime, const FVector& NewAcceleration)
{
	SCOPE_CYCLE_COUNTER(STAT_CharacterMovementReplicateMoveToServer);
	check(CharacterOwner != NULL);

	APlayerController* PC = Cast<APlayerController>(CharacterOwner->GetController());
	if (PC && PC->AcknowledgedPawn != CharacterOwner)
	{
		return;
	}

	if (PC && PC->Player == nullptr)
	{
		return;
	}

	FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
	if (!ClientData)
	{
		return;
	}

	DeltaTime = ClientData->UpdateTimeStampAndDeltaTime(DeltaTime, *CharacterOwner, *this);

	FSavedMovePtr OldMove = NULL;
	if (ClientData->LastAckedMove.IsValid())
	{
		for (int32 i = 0; i < ClientData->SavedMoves.Num() - 1; i++)
		{
			const FSavedMovePtr& CurrentMove = ClientData->SavedMoves[i];
			if (CurrentMove->IsImportantMove(ClientData->LastAckedMove))
			{
				OldMove = CurrentMove;
				break;
			}
		}
	}

	FSavedMovePtr NewMovePtr = ClientData->CreateSavedMove();
	FSavedMove_Character* const NewMove = NewMovePtr.Get();
	if (NewMove == nullptr)
	{
		return;
	}

	NewMove->SetMoveFor(CharacterOwner, DeltaTime, NewAcceleration, *ClientData);
	const UWorld* MyWorld = GetWorld();

	if (const FSavedMove_Character* PendingMove = ClientData->PendingMove.Get())
	{
		if (bAllowMovementMerging && PendingMove->CanCombineWith(NewMovePtr, CharacterOwner, ClientData->MaxMoveDeltaTime * CharacterOwner->GetActorTimeDilation(*MyWorld)))
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_VRCharacterMovementComponent_CombineNetMove);

			FVector OldStartLocation = PendingMove->GetRevertedLocation();

			FVector OverlapLocation = OldStartLocation;
			if (VRRootCapsule)
				OverlapLocation += VRRootCapsule->OffsetComponentToWorld.GetLocation() - VRRootCapsule->GetComponentLocation();

			const bool bAttachedToObject = (NewMovePtr->StartAttachParent != nullptr);
			if (bAttachedToObject || !OverlapTest(OverlapLocation, PendingMove->StartRotation.Quaternion(), UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CharacterOwner))
			{

				FScopedMeshBoneUpdateOverrideVR ScopedNoMeshBoneUpdate(CharacterOwner->GetMesh(), EKinematicBonesUpdateToPhysics::SkipAllBones);

				FVRCharacterScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, EScopedUpdate::DeferredUpdates);
				UE_LOGF(LogVRCharacterMovement, VeryVerbose, "CombineMove: add delta %f + %f and revert from %f %f to %f %f", DeltaTime, ClientData->PendingMove->DeltaTime, UpdatedComponent->GetComponentLocation().X, UpdatedComponent->GetComponentLocation().Y, OverlapLocation.X, OverlapLocation.Y);

				NewMove->CombineWith(PendingMove, CharacterOwner, PC, OldStartLocation);

				FSavedMove_VRBaseCharacter * BaseCharMove = ((FSavedMove_VRBaseCharacter*)NewMove);
				AdditionalVRInputVector = BaseCharMove->LFDiff;

				if (PC)
				{

					CharacterOwner->FaceRotation(PC->GetControlRotation(), NewMove->DeltaTime);
				}

				SaveBaseLocation();
				NewMove->SetInitialPosition(CharacterOwner);

				if (ClientData->SavedMoves.Num() > 0 && ClientData->SavedMoves.Last() == ClientData->PendingMove)
				{
					ClientData->SavedMoves.Pop(EAllowShrinking::No);
				}
				ClientData->FreeMove(ClientData->PendingMove);
				ClientData->PendingMove = nullptr;
				PendingMove = nullptr; 
			}
			else
			{
				UE_LOGF(LogVRCharacterMovement, Verbose, "Not combining move [would collide at start location]");
			}
		}

	}

	Acceleration = NewMove->Acceleration.GetClampedToMaxSize(GetMaxAcceleration());
	AnalogInputModifier = ComputeAnalogInputModifier(); 

	CharacterOwner->ClientRootMotionParams.Clear();
	CharacterOwner->SavedRootMotion.Clear();
	PerformMovement(NewMove->DeltaTime);

	NewMove->PostUpdate(CharacterOwner, FSavedMove_Character::PostUpdate_Record);

	if (CharacterOwner->IsReplicatingMovement())
	{
		check(NewMove == NewMovePtr.Get());
		ClientData->SavedMoves.Push(NewMovePtr);

		static const auto CVarNetEnableMoveCombining = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetEnableMoveCombining"));
		const bool bCanDelayMove = (CVarNetEnableMoveCombining->GetInt() != 0) && CanDelaySendingMove(NewMovePtr);

		if (bCanDelayMove && ClientData->PendingMove.IsValid() == false)
		{

			const float NetMoveDeltaSeconds = FMath::Clamp(GetClientNetSendDeltaTime(PC, ClientData, NewMovePtr), 1.f / 120.f, 1.f / 5.f);
			const float SecondsSinceLastMoveSent = MyWorld->GetRealTimeSeconds() - ClientData->ClientUpdateRealTime;

			if (SecondsSinceLastMoveSent < NetMoveDeltaSeconds)
			{

				ClientData->PendingMove = NewMovePtr;
				return;
			}
		}

		ClientData->ClientUpdateRealTime = MyWorld->GetRealTimeSeconds();

		UE_CLOGF(CharacterOwner && UpdatedComponent, LogNetPlayerMovement, VeryVerbose, "ClientMove Time %f Acceleration %ls Velocity %ls Position %ls Rotation %ls DeltaTime %f Mode %ls MovementBase %ls.%ls (Dynamic:%d) DualMove? %d",
			NewMove->TimeStamp, *NewMove->Acceleration.ToString(), *Velocity.ToString(), *UpdatedComponent->GetComponentLocation().ToString(), *UpdatedComponent->GetComponentRotation().ToCompactString(), NewMove->DeltaTime, *GetMovementName(),
			*GetNameSafe(NewMove->EndMovementBaseInterfaceData.GetMovementBaseObject()), *NewMove->EndBoneName.ToString(),
			MovementBaseUtility::IsDynamicBase(&NewMove->EndMovementBaseInterfaceData) ? 1 : 0, ClientData->PendingMove.IsValid() ? 1 : 0);

		bool bSendServerMove = true;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

		const float TimeSinceLossStart = (MyWorld->RealTimeSeconds - ClientData->DebugForcedPacketLossTimerStart);

		static const auto CVarNetForceClientServerMoveLossDuration = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetForceClientServerMoveLossDuration"));
		static const auto CVarNetForceClientServerMoveLossPercent = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetForceClientServerMoveLossPercent"));
		if (ClientData->DebugForcedPacketLossTimerStart > 0.f && (TimeSinceLossStart < CVarNetForceClientServerMoveLossDuration->GetFloat()))
		{
			bSendServerMove = false;
			UE_LOGF(LogVRCharacterMovement, Log, "Drop ServerMove, %.2f time remains", CVarNetForceClientServerMoveLossDuration->GetFloat() - TimeSinceLossStart);
		}
		else if (CVarNetForceClientServerMoveLossPercent->GetFloat() != 0.f && (RandomStream.FRand() < CVarNetForceClientServerMoveLossPercent->GetFloat()))
		{
			bSendServerMove = false;
			ClientData->DebugForcedPacketLossTimerStart = (CVarNetForceClientServerMoveLossDuration->GetFloat() > 0) ? MyWorld->RealTimeSeconds : 0.0f;
			UE_LOGF(LogVRCharacterMovement, Log, "Drop ServerMove, %.2f time remains", CVarNetForceClientServerMoveLossDuration->GetFloat());
		}
		else
		{
			ClientData->DebugForcedPacketLossTimerStart = 0.f;
		}
#endif

		if (bSendServerMove)
		{
			SCOPE_CYCLE_COUNTER(STAT_CharacterMovementCallServerMove);
			if (ShouldUsePackedMovementRPCs())
			{
				CallServerMovePacked(NewMove, ClientData->PendingMove.Get(), OldMove.Get());
			}

		}
	}

	ClientData->PendingMove = NULL;
}

UVRCharacterMovementComponent::UVRCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PostPhysicsTickFunction.bCanEverTick = true;
	PostPhysicsTickFunction.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	VRRootCapsule = NULL;

	bUseClientControlRotation = false;
	bAllowMovementMerging = true;
	bRunClientCorrectionToHMD = false;
	bRequestedMoveUseAcceleration = false;
}

void UVRCharacterMovementComponent::OnRegister()
{
	Super::OnRegister();

	const UWorld* MyWorld = GetWorld();
	const bool bIsReplay = (MyWorld && MyWorld->IsPlayingReplay());

	if (bIsReplay)
	{

		NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
	}
}

void UVRCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (!HasValidData())
	{
		return;
	}

	if (CharacterOwner && CharacterOwner->IsLocallyControlled())
	{

		if (VRRootCapsule)
		{
			AdditionalVRInputVector = VRRootCapsule->DifferenceFromLastFrame;
		}
		else
		{
			AdditionalVRInputVector = FVector::ZeroVector;
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UVRCharacterMovementComponent::CanCrouch()
{
	return false;
}

void UVRCharacterMovementComponent::ApplyRepulsionForce(float DeltaSeconds)
{
	if (UpdatedPrimitive && RepulsionForce > 0.0f && CharacterOwner != nullptr)
	{
		const TArray<FOverlapInfo>& Overlaps = UpdatedPrimitive->GetOverlapInfos();
		if (Overlaps.Num() > 0)
		{
			FCollisionQueryParams QueryParams;
			QueryParams.bReturnFaceIndex = false;
			QueryParams.bReturnPhysicalMaterial = false;

			float CapsuleRadius = 0.f;
			float CapsuleHalfHeight = 0.f;
			CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
			const float RepulsionForceRadius = CapsuleRadius * 1.2f;
			const float StopBodyDistance = 2.5f;
			FVector MyLocation;

			if (VRRootCapsule)
				MyLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();
			else
				MyLocation = UpdatedPrimitive->GetComponentLocation();

			for (int32 i = 0; i < Overlaps.Num(); i++)
			{
				const FOverlapInfo& Overlap = Overlaps[i];

				UPrimitiveComponent* OverlapComp = Overlap.OverlapInfo.Component.Get();
				if (!OverlapComp || OverlapComp->Mobility < EComponentMobility::Movable)
				{
					continue;
				}

				FBodyInstance* OverlapBody = nullptr;
				const int32 OverlapBodyIndex = Overlap.GetBodyIndex();
				const USkeletalMeshComponent* SkelMeshForBody = (OverlapBodyIndex != INDEX_NONE) ? Cast<USkeletalMeshComponent>(OverlapComp) : nullptr;
				if (SkelMeshForBody != nullptr)
				{
					OverlapBody = SkelMeshForBody->Bodies.IsValidIndex(OverlapBodyIndex) ? SkelMeshForBody->Bodies[OverlapBodyIndex] : nullptr;
				}
				else
				{
					OverlapBody = OverlapComp->GetBodyInstance();
				}

				if (!OverlapBody)
				{
					UE_LOGF(LogVRCharacterMovement, Warning, "%ls could not find overlap body for body index %d", *GetName(), OverlapBodyIndex);
					continue;
				}

				if (!OverlapBody->IsInstanceSimulatingPhysics())
				{
					continue;
				}

				FTransform BodyTransform = OverlapBody->GetUnrealWorldTransform();

				FVector BodyVelocity = OverlapBody->GetUnrealWorldVelocity();
				FVector BodyLocation = BodyTransform.GetLocation();

				FHitResult Hit;
				bool bHasHit = UpdatedPrimitive->LineTraceComponent(Hit, BodyLocation,
					ProjectToGravityFloor(MyLocation) + GetGravitySpaceComponentZ(BodyLocation),
					QueryParams);

				FVector HitLoc = Hit.ImpactPoint;
				bool bIsPenetrating = Hit.bStartPenetrating || Hit.PenetrationDepth > StopBodyDistance;

				if (!bHasHit)
				{
					HitLoc = BodyLocation;
					bIsPenetrating = true;
				}

				const float DistanceNow = ProjectToGravityFloor(HitLoc - BodyLocation).SizeSquared();
				const float DistanceLater = ProjectToGravityFloor(HitLoc - (BodyLocation + BodyVelocity * DeltaSeconds)).SizeSquared();

				if (bHasHit && DistanceNow < StopBodyDistance && !bIsPenetrating)
				{
					OverlapBody->SetLinearVelocity(FVector::ZeroVector, false);
				}
				else if (DistanceLater <= DistanceNow || bIsPenetrating)
				{
					FVector ForceCenter = MyLocation;

					if (bHasHit)
					{
						SetGravitySpaceZ(ForceCenter, GetGravitySpaceZ(HitLoc));
					}
					else
					{
						const FVector::FReal MyLocationZ = GetGravitySpaceZ(MyLocation);
						SetGravitySpaceZ(ForceCenter, FMath::Clamp(GetGravitySpaceZ(BodyLocation), MyLocationZ - CapsuleHalfHeight, MyLocationZ + CapsuleHalfHeight));
					}

					OverlapBody->AddRadialForceToBody(ForceCenter, RepulsionForceRadius, RepulsionForce * Mass, ERadialImpulseFalloff::RIF_Constant);
				}
			}
		}
	}
}

void UVRCharacterMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);

	if (UpdatedComponent)
	{	

		VRRootCapsule = Cast<UVRRootComponent>(UpdatedComponent);

		UpdatedComponent->PrimaryComponentTick.RemovePrerequisite(this, PrimaryComponentTick);

		this->PrimaryComponentTick.AddPrerequisite(UpdatedComponent, UpdatedComponent->PrimaryComponentTick);
	}
}

FORCEINLINE_DEBUGGABLE bool UVRCharacterMovementComponent::SafeMoveUpdatedComponent(const FVector& Delta, const FRotator& NewRotation, bool bSweep, FHitResult& OutHit, ETeleportType Teleport)
{
	return SafeMoveUpdatedComponent(Delta, NewRotation.Quaternion(), bSweep, OutHit, Teleport);
}

bool UVRCharacterMovementComponent::SafeMoveUpdatedComponent(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult& OutHit, ETeleportType Teleport)
{
	if (UpdatedComponent == NULL)
	{
		OutHit.Reset(1.f);
		return false;
	}

	bool bMoveResult = MoveUpdatedComponent(Delta, NewRotation, bSweep, &OutHit, Teleport);

	if (OutHit.bStartPenetrating && UpdatedComponent)
	{
		const FVector RequestedAdjustment = GetPenetrationAdjustment(OutHit);
		if (ResolvePenetration(RequestedAdjustment, OutHit, NewRotation))
		{
			FHitResult TempHit;

			bMoveResult = MoveUpdatedComponent(Delta, NewRotation, bSweep, &TempHit, Teleport);

			if (TempHit.bStartPenetrating)
				OutHit = TempHit;
			else
				OutHit.bStartPenetrating = TempHit.bStartPenetrating;
		}
	}

	return bMoveResult;
}

void UVRCharacterMovementComponent::MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult)
{
	if (!CurrentFloor.IsWalkableFloor())
	{
		return;
	}

	const FVector Delta = ProjectToGravityFloor(InVelocity) * DeltaSeconds;
	FHitResult Hit(1.f);
	FVector RampVector = ComputeGroundMovementDelta(Delta, CurrentFloor.HitResult, CurrentFloor.bLineTrace);
	SafeMoveUpdatedComponent(RampVector, UpdatedComponent->GetComponentQuat(), true, Hit);
	float LastMoveTimeSlice = DeltaSeconds;

	if (Hit.bStartPenetrating)
	{

		HandleImpact(Hit);
		SlideAlongSurface(Delta, 1.f, Hit.Normal, Hit, true);

		if (Hit.bStartPenetrating)
		{
			OnCharacterStuckInGeometry(&Hit);
		}
	}
	else if (Hit.IsValidBlockingHit())
	{

		float PercentTimeApplied = Hit.Time;
		if ((Hit.Time > 0.f) && (GetGravitySpaceZ(Hit.Normal) > UE_KINDA_SMALL_NUMBER) && IsWalkable(Hit))
		{

			const float InitialPercentRemaining = 1.f - PercentTimeApplied;
			RampVector = ComputeGroundMovementDelta(Delta * InitialPercentRemaining, Hit, false);
			LastMoveTimeSlice = InitialPercentRemaining * LastMoveTimeSlice;
			SafeMoveUpdatedComponent(RampVector, UpdatedComponent->GetComponentQuat(), true, Hit);

			const float SecondHitPercent = Hit.Time * InitialPercentRemaining;
			PercentTimeApplied = FMath::Clamp(PercentTimeApplied + SecondHitPercent, 0.f, 1.f);
		}

		if (Hit.IsValidBlockingHit())
		{
			const FMovementBaseInterfaceData* MovementBaseData = GetMovementBaseInterfaceData();
			const UObject* MovementBaseObject = MovementBaseData ? GetMovementBaseInterfaceData()->PhysicsObjectOwner.Get() : nullptr;
			if (CanStepUp(Hit) ||
				(Hit.PhysicsObjectOwner.Get() == MovementBaseObject) ||
				(Hit.Component.Get() == MovementBaseObject))
			{

				const FVector PreStepUpLocation = UpdatedComponent->GetComponentLocation();
				const FVector GravDir = GetGravityDirection();

				if (!StepUp(GetGravityDirection(), Delta * (1.f - PercentTimeApplied), Hit, OutStepDownResult))
				{
					UE_LOGF(LogVRCharacterMovement, Verbose, "- StepUp (ImpactNormal %ls, Normal %ls", *Hit.ImpactNormal.ToString(), *Hit.Normal.ToString());
					HandleImpact(Hit, LastMoveTimeSlice, RampVector);
					SlideAlongSurface(Delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);

				}
				else
				{
					UE_LOGF(LogVRCharacterMovement, Verbose, "+ StepUp (ImpactNormal %ls, Normal %ls", *Hit.ImpactNormal.ToString(), *Hit.Normal.ToString());
					if (!bMaintainHorizontalGroundVelocity)
					{

						bJustTeleported = true;
						const float StepUpTimeSlice = (1.f - PercentTimeApplied) * DeltaSeconds;
						if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && StepUpTimeSlice >= UE_KINDA_SMALL_NUMBER)
						{
							Velocity = (UpdatedComponent->GetComponentLocation() - PreStepUpLocation) / StepUpTimeSlice;
							Velocity = ProjectToGravityFloor(Velocity);
						}
					}
				}
			}
			else if (Hit.Component.IsValid() && !Hit.Component.Get()->CanCharacterStepUp(CharacterOwner))
			{
				HandleImpact(Hit, LastMoveTimeSlice, RampVector);
				SlideAlongSurface(Delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);

			}
		}
	}
}

bool UVRCharacterMovementComponent::StepUp(const FVector& GravDir, const FVector& Delta, const FHitResult &InHit, FStepDownResult* OutStepDownResult)
{
	SCOPE_CYCLE_COUNTER(STAT_CharStepUp);

	if (!CanStepUp(InHit) || MaxStepHeight <= 0.f)
	{
		return false;
	}

	FVector OldLocation;

	if (VRRootCapsule)
		OldLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();
	else
		OldLocation = UpdatedComponent->GetComponentLocation();

	float PawnRadius, PawnHalfHeight;
	CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

	const float InitialImpactZ = InHit.ImpactPoint | -GravDir;
	const float OldLocationZ = OldLocation | -GravDir;
	if (InitialImpactZ > OldLocationZ + (PawnHalfHeight - PawnRadius))
	{
		return false;
	}

	if (GravDir.IsZero())
	{
		return false;
	}

	ensure(GravDir.IsNormalized());

	float StepTravelUpHeight = MaxStepHeight;
	float StepTravelDownHeight = StepTravelUpHeight;
	const float StepSideZ = -1.f * FVector::DotProduct(InHit.ImpactNormal, GravDir);
	float PawnInitialFloorBaseZ = OldLocationZ - PawnHalfHeight;
	float PawnFloorPointZ = PawnInitialFloorBaseZ;

	if (IsMovingOnGround() && CurrentFloor.IsWalkableFloor())
	{

		const float FloorDist = FMath::Max(0.f, CurrentFloor.GetDistanceToFloor());
		PawnInitialFloorBaseZ -= FloorDist;
		StepTravelUpHeight = FMath::Max(StepTravelUpHeight - FloorDist, 0.f);
		StepTravelDownHeight = (MaxStepHeight + MAX_FLOOR_DIST*2.f);

		const bool bHitVerticalFace = !IsWithinEdgeTolerance(InHit.Location, InHit.ImpactPoint, PawnRadius);
		if (!CurrentFloor.bLineTrace && !bHitVerticalFace)
		{
			PawnFloorPointZ = CurrentFloor.HitResult.ImpactPoint | -GravDir;
		}
		else
		{

			PawnFloorPointZ -= CurrentFloor.FloorDist;
		}
	}

	if (InitialImpactZ <= PawnInitialFloorBaseZ)
	{
		return false;
	}

	 FVRCharacterScopedMovementUpdate ScopedStepUpMovement(UpdatedComponent, EScopedUpdate::DeferredUpdates);

	FHitResult SweepUpHit(1.f);
	const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
	MoveUpdatedComponent(-GravDir * StepTravelUpHeight, PawnRotation, true, &SweepUpHit);

	if (SweepUpHit.bStartPenetrating)
	{

		ScopedStepUpMovement.RevertMove();
		return false;
	}

	FHitResult Hit(1.f);
	MoveUpdatedComponent(Delta, PawnRotation, true, &Hit);

	if (Hit.bBlockingHit)
	{
		if (Hit.bStartPenetrating)
		{

			ScopedStepUpMovement.RevertMove();
			return false;
		}

		if (SweepUpHit.bBlockingHit && Hit.bBlockingHit)
		{
			HandleImpact(SweepUpHit);
		}

		HandleImpact(Hit);
		if (IsFalling())
		{
			return true;
		}

		const float ForwardHitTime = Hit.Time;
		const float ForwardSlideAmount = SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);

		if (IsFalling())
		{
			ScopedStepUpMovement.RevertMove();
			return false;
		}

		if (ForwardHitTime == 0.f && ForwardSlideAmount == 0.f)
		{
			ScopedStepUpMovement.RevertMove();
			return false;
		}
	}

	MoveUpdatedComponent(GravDir * StepTravelDownHeight, UpdatedComponent->GetComponentQuat(), true, &Hit);

	if (Hit.bStartPenetrating)
	{
		ScopedStepUpMovement.RevertMove();
		return false;
	}

	FStepDownResult StepDownResult;
	if (Hit.IsValidBlockingHit())
	{

		const float DeltaZ = (Hit.ImpactPoint | -GravDir) - PawnFloorPointZ;
		if (DeltaZ > MaxStepHeight)
		{

			ScopedStepUpMovement.RevertMove();
			return false;
		}

		if (!IsWalkable(Hit))
		{

			const bool bNormalTowardsMe = (Delta | Hit.ImpactNormal) < 0.f;
			if (bNormalTowardsMe)
			{

				ScopedStepUpMovement.RevertMove();
				return false;
			}

			if ((Hit.Location | -GravDir) > OldLocationZ)
			{

				ScopedStepUpMovement.RevertMove();
				return false;
			}
		}

		if (!IsWithinEdgeTolerance(Hit.Location, Hit.ImpactPoint, PawnRadius))
		{

			ScopedStepUpMovement.RevertMove();
			return false;
		}

		if (DeltaZ > 0.f && !CanStepUp(Hit))
		{

			ScopedStepUpMovement.RevertMove();
			return false;
		}

		if (OutStepDownResult != NULL)
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), StepDownResult.FloorResult, false, &Hit);

			if ((Hit.Location | -GravDir) > OldLocationZ)
			{

				if (!StepDownResult.FloorResult.bBlockingHit && StepSideZ < CharacterMovementConstants::MAX_STEP_SIDE_ZVR)
				{
					ScopedStepUpMovement.RevertMove();
					return false;
				}
			}

			StepDownResult.bComputedFloor = true;

			if (bAutoOrientToFloorNormal && StepDownResult.FloorResult.IsWalkableFloor())
			{

				AutoTraceAndSetCharacterToNewGravity(StepDownResult.FloorResult.HitResult);
			}
		}
	}

	if (OutStepDownResult != NULL)
	{
		*OutStepDownResult = StepDownResult;
	}

	bJustTeleported |= !bMaintainHorizontalGroundVelocity;

	return true;
}

bool UVRCharacterMovementComponent::IsWithinEdgeTolerance(const FVector& CapsuleLocation, const FVector& TestImpactPoint, const float CapsuleRadius) const
{
	const FVector GravityRelativeToTestImpactPoint = RotateWorldToGravity(TestImpactPoint - CapsuleLocation);
	const float DistFromCenterSq = GravityRelativeToTestImpactPoint.SizeSquared2D();
	const float ReducedRadiusSq = FMath::Square(FMath::Max(VREdgeRejectDistance + UE_KINDA_SMALL_NUMBER, CapsuleRadius - VREdgeRejectDistance));
	return DistFromCenterSq < ReducedRadiusSq;
}

bool UVRCharacterMovementComponent::IsWithinClimbingEdgeTolerance(const FVector& CapsuleLocation, const FVector& TestImpactPoint, const float CapsuleRadius) const
{
	const float DistFromCenterSq = (TestImpactPoint - CapsuleLocation).SizeSquared2D();
	const float ReducedRadiusSq = FMath::Square(FMath::Max(VRClimbingEdgeRejectDistance + UE_KINDA_SMALL_NUMBER, CapsuleRadius - VRClimbingEdgeRejectDistance));
	return DistFromCenterSq < ReducedRadiusSq;
}

bool UVRCharacterMovementComponent::VRClimbStepUp(const FVector& GravDir, const FVector& Delta, const FHitResult &InHit, FStepDownResult* OutStepDownResult)
{
	SCOPE_CYCLE_COUNTER(STAT_CharStepUp);

	if (!CanStepUp(InHit) || MaxStepHeight <= 0.f)
	{
		return false;
	}

	FVector OldLocation;

	if (VRRootCapsule)
		OldLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();
	else
		OldLocation = UpdatedComponent->GetComponentLocation();

	float PawnRadius, PawnHalfHeight;
	CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

	const float InitialImpactZ = InHit.ImpactPoint | -GravDir;
	const float OldLocationZ = OldLocation | -GravDir;
	if (InitialImpactZ > OldLocationZ + (PawnHalfHeight - PawnRadius))
	{
		return false;
	}

	if (InitialImpactZ <= OldLocation.Z - PawnHalfHeight)
	{
		return false;
	}

	if (GravDir.IsZero())
	{
		return false;
	}

	ensure(GravDir.IsNormalized());

	float StepTravelUpHeight = MaxStepHeight;
	float StepTravelDownHeight = StepTravelUpHeight;
	const float StepSideZ = -1.f * (InHit.ImpactNormal | GravDir);
	float PawnInitialFloorBaseZ = OldLocationZ - PawnHalfHeight;
	float PawnFloorPointZ = PawnInitialFloorBaseZ;

	FVRCharacterScopedMovementUpdate ScopedStepUpMovement(UpdatedComponent, EScopedUpdate::DeferredUpdates);

	FHitResult SweepUpHit(1.f);
	const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
	MoveUpdatedComponent(-GravDir * StepTravelUpHeight, PawnRotation, true, &SweepUpHit);

	if (SweepUpHit.bStartPenetrating)
	{

		ScopedStepUpMovement.RevertMove();
		return false;
	}

	FHitResult Hit(1.f);

		MoveUpdatedComponent(Delta, PawnRotation, true, &Hit);

	if (Hit.bBlockingHit)
	{
		if (Hit.bStartPenetrating)
		{

			ScopedStepUpMovement.RevertMove();
			return false;
		}

		if (SweepUpHit.bBlockingHit && Hit.bBlockingHit)
		{

			HandleImpact(SweepUpHit);
		}

		HandleImpact(Hit);
		if (IsFalling())
		{
			return true;
		}

		ScopedStepUpMovement.RevertMove();
		return false;

	}

	MoveUpdatedComponent(GravDir * StepTravelDownHeight, UpdatedComponent->GetComponentQuat(), true, &Hit);

	if (Hit.bStartPenetrating)
	{
		ScopedStepUpMovement.RevertMove();
		return false;
	}

	FStepDownResult StepDownResult;
	if (Hit.IsValidBlockingHit())
	{

		const float DeltaZ = (Hit.ImpactPoint | -GravDir) - PawnFloorPointZ;
		if (DeltaZ > MaxStepHeight)
		{
			UE_LOGF(LogVRCharacterMovement, VeryVerbose, "- Reject StepUp (too high Height %.3f) up from floor base %f", DeltaZ, PawnInitialFloorBaseZ);
			ScopedStepUpMovement.RevertMove();
			return false;
		}

		if (!IsWalkable(Hit))
		{

			const bool bNormalTowardsMe = (Delta | Hit.ImpactNormal) < 0.f;
			if (bNormalTowardsMe)
			{

				ScopedStepUpMovement.RevertMove();
				return false;
			}

			if ((Hit.Location | -GravDir) > OldLocationZ)
			{
				UE_LOGF(LogVRCharacterMovement, VeryVerbose, "- Reject StepUp (unwalkable normal %ls above old position)", *Hit.ImpactNormal.ToString());
				ScopedStepUpMovement.RevertMove();
				return false;
			}
		}

		if (!IsWithinClimbingEdgeTolerance(Hit.Location, Hit.ImpactPoint, PawnRadius))
		{
			UE_LOGF(LogVRCharacterMovement, VeryVerbose, "- Reject StepUp (outside edge tolerance)");
			ScopedStepUpMovement.RevertMove();
			return false;
		}

		if (DeltaZ > 0.f && !CanStepUp(Hit))
		{
			UE_LOGF(LogVRCharacterMovement, VeryVerbose, "- Reject StepUp (up onto surface with !CanStepUp())");
			ScopedStepUpMovement.RevertMove();
			return false;		
		}

		if (OutStepDownResult != NULL)
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), StepDownResult.FloorResult, false, &Hit);

			if ((Hit.Location | -GravDir) > OldLocationZ)
			{

				if (!StepDownResult.FloorResult.bBlockingHit && StepSideZ < CharacterMovementConstants::MAX_STEP_SIDE_ZVR)
				{
					ScopedStepUpMovement.RevertMove();
					return false;
				}
			}

			StepDownResult.bComputedFloor = true;

			if (bAutoOrientToFloorNormal && StepDownResult.FloorResult.IsWalkableFloor())
			{

				AutoTraceAndSetCharacterToNewGravity(StepDownResult.FloorResult.HitResult);
			}
		}
	}

	if (OutStepDownResult != NULL)
	{
		*OutStepDownResult = StepDownResult;
	}

	bJustTeleported |= !bMaintainHorizontalGroundVelocity;
	return true;
}

FVector UVRCharacterMovementComponent::GetActorFeetLocation() const
{

	return GetActorFeetLocationVR();
}

void UVRCharacterMovementComponent::UpdateBasedMovement(float DeltaSeconds)
{
	if (!HasValidData())
	{
		return;
	}

	const FMovementBaseInterfaceData* MovementBaseInterfaceData = GetMovementBaseInterfaceData();
	if (!MovementBaseInterfaceData || !MovementBaseInterfaceData->IsValid() || !MovementBaseUtility::UseRelativeLocation(MovementBaseInterfaceData))
	{
		return;
	}

	if (!MovementBaseUtility::IsMovementBaseDataValid(MovementBaseInterfaceData))
	{
		FMovementBaseInterfaceData EmptyMovementBaseData;
		SetBase(&EmptyMovementBaseData);
		return;
	}

	TGuardValue<EMoveComponentFlags> ScopedFlagRestore(MoveComponentFlags, MoveComponentFlags | MOVECOMP_IgnoreBases);

	FQuat DeltaQuat = FQuat::Identity;
	FVector DeltaPosition = FVector::ZeroVector;

	FQuat NewBaseQuat;
	FVector NewBaseLocation;
	if (!MovementBaseUtility::GetMovementBaseTransform(MovementBaseInterfaceData, CharacterOwner->GetBasedMovement().BoneName, NewBaseLocation, NewBaseQuat))
	{
		return;
	}

	const bool bRotationChanged = !OldBaseQuat.Equals(NewBaseQuat, 1e-8f);
	if (bRotationChanged)
	{
		DeltaQuat = NewBaseQuat * OldBaseQuat.Inverse();
	}

	if (bRotationChanged || (OldBaseLocation != NewBaseLocation))
	{

		const FQuatRotationTranslationMatrix OldLocalToWorld(OldBaseQuat, OldBaseLocation);
		const FQuatRotationTranslationMatrix NewLocalToWorld(NewBaseQuat, NewBaseLocation);

		FQuat FinalQuat = UpdatedComponent->GetComponentQuat();

		if (bRotationChanged && !bIgnoreBaseRotation)
		{

			const FQuat PawnOldQuat = UpdatedComponent->GetComponentQuat();
			const FQuat TargetQuat = DeltaQuat * FinalQuat;
			FRotator TargetRotator(TargetQuat);
			CharacterOwner->FaceRotation(TargetRotator, 0.f);
			FinalQuat = UpdatedComponent->GetComponentQuat();

			if (PawnOldQuat.Equals(FinalQuat, 1e-6f))
			{

				if (bOrientRotationToMovement || (bUseControllerDesiredRotation && CharacterOwner->GetController()))
				{

					if (!HasCustomGravity())
					{
						TargetRotator.Pitch = 0.f;
						TargetRotator.Roll = 0.f;
					}
					MoveUpdatedComponent(FVector::ZeroVector, TargetRotator, false);
					FinalQuat = UpdatedComponent->GetComponentQuat();
				}
			}

			if (CharacterOwner->GetController())
			{
				const FQuat PawnDeltaRotation = FinalQuat * PawnOldQuat.Inverse();
				FRotator FinalRotation = FinalQuat.Rotator();
				UpdateBasedRotation(FinalRotation, PawnDeltaRotation.Rotator());
				FinalQuat = UpdatedComponent->GetComponentQuat();
			}
		}

		FVector NewWorldPos;
		if (HasCustomGravity())
		{
			const FVector RotationRadius = UpdatedComponent->GetComponentLocation() - NewBaseLocation;
			const FVector RotationDelta = DeltaQuat.RotateVector(RotationRadius) - RotationRadius;
			const FVector LinearDelta = NewBaseLocation - OldBaseLocation;
			NewWorldPos = ConstrainLocationToPlane(UpdatedComponent->GetComponentLocation() + RotationDelta + LinearDelta);
		}
		else
		{

			float HalfHeight, Radius;
			CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(Radius, HalfHeight);

			if (!BaseVRCharacterOwner || BaseVRCharacterOwner->bRetainRoomscale)
			{
				HalfHeight = 0;
			}

			FVector const BaseOffset(0.0f, 0.0f, HalfHeight);

			FVector const LocalBasePos = OldLocalToWorld.InverseTransformPosition(UpdatedComponent->GetComponentLocation() - BaseOffset);
			NewWorldPos = ConstrainLocationToPlane(NewLocalToWorld.TransformPosition(LocalBasePos) + BaseOffset);
		}

		DeltaPosition = ConstrainDirectionToPlane(NewWorldPos - UpdatedComponent->GetComponentLocation());

		if (bFastAttachedMove)
		{

			UpdatedComponent->SetWorldLocationAndRotation(NewWorldPos, FinalQuat, false);
		}
		else
		{

			const bool bIgnoreBaseActor = bDeferUpdateBasedMovement && bBasedMovementIgnorePhysicsBase;
			AActor* MovementBaseRootActor = nullptr;
			if (bIgnoreBaseActor)
			{
				if (MovementBaseInterfaceData && MovementBaseInterfaceData->IsValid())
				{
					MovementBaseRootActor = Cast<AActor>(MovementBaseInterfaceData->GetBodyInstanceOwner()->GetPhysicsOwnerAttachmentRoot());
				}
				UpdatedPrimitive->IgnoreActorWhenMoving(MovementBaseRootActor, true);
				MoveComponentFlags |= MOVECOMP_CheckBlockingRootActorInIgnoreList; 
			}

			FVector BaseMoveDelta = NewBaseLocation - OldBaseLocation;
			if (!bRotationChanged && (BaseMoveDelta.X == 0.f) && (BaseMoveDelta.Y == 0.f))
			{
				DeltaPosition.X = 0.f;
				DeltaPosition.Y = 0.f;
			}

			FHitResult MoveOnBaseHit(1.f);
			const FVector OldLocation = UpdatedComponent->GetComponentLocation();
			MoveUpdatedComponent(DeltaPosition, FinalQuat, true, &MoveOnBaseHit);

			if ((UpdatedComponent->GetComponentLocation() - (OldLocation + DeltaPosition)).IsNearlyZero() == false)
			{
				OnUnableToFollowBaseMove(DeltaPosition, OldLocation, MoveOnBaseHit);
			}

			if (bIgnoreBaseActor)
			{
				MoveComponentFlags &= ~MOVECOMP_CheckBlockingRootActorInIgnoreList;
				UpdatedPrimitive->IgnoreActorWhenMoving(MovementBaseRootActor, false);
			}
		}

		if (MovementBaseInterfaceData && MovementBaseInterfaceData->IsValid())
		{
			const FBodyInstance* MovementBaseBodyInstance = MovementBaseInterfaceData->GetBodyInstanceOwner()->GetBodyInstance();
			if (MovementBaseBodyInstance && MovementBaseBodyInstance->IsInstanceSimulatingPhysics() && CharacterOwner->GetMesh())
			{
				CharacterOwner->GetMesh()->ApplyDeltaToAllPhysicsTransforms(DeltaPosition, DeltaQuat);
			}
		}
	}

	if (IsFalling() && bStayBasedInAir)
	{
		FVector PawnLocation = UpdatedComponent->GetComponentLocation();
		if (VRRootCapsule)
			PawnLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();

		FFindFloorResult OutFloorResult;
		ComputeFloorDist(PawnLocation, StayBasedInAirHeight, StayBasedInAirHeight, OutFloorResult, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius(), NULL);

		FMovementBaseInterfaceData FoundMovementBaseData = MovementBaseUtility::GetMovementBaseDataFromHitResult(&OutFloorResult.HitResult);
		if (!FoundMovementBaseData.IsValid() || FoundMovementBaseData.GetBodyInstanceOwner()->GetPhysicsOwnerAttachmentRoot() != MovementBaseInterfaceData->GetBodyInstanceOwner()->GetPhysicsOwnerAttachmentRoot())
		{

			ApplyImpartedMovementBaseVelocity();
			FMovementBaseInterfaceData EmptyMovementBaseData;
			SetBase(&EmptyMovementBaseData);
			return;
		}
	}
}

FVector UVRCharacterMovementComponent::GetImpartedMovementBaseVelocity() const
{
	FVector Result = FVector::ZeroVector;

	if (CharacterOwner)
	{
		const FMovementBaseInterfaceData* MovementBaseInterfaceData = GetMovementBaseInterfaceData();
		if (MovementBaseUtility::IsDynamicBase(MovementBaseInterfaceData))
		{
			FVector BaseVelocity = MovementBaseUtility::GetMovementBaseVelocity(MovementBaseInterfaceData, CharacterOwner->GetBasedMovement().BoneName);

			if (bImpartBaseAngularVelocity)
			{

				float HalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				if (BaseVRCharacterOwner && BaseVRCharacterOwner->bRetainRoomscale)
				{
					HalfHeight = 0.0f;
				}

				const FVector CharacterBasePosition = (UpdatedComponent->GetComponentLocation() + HalfHeight * GetGravityDirection());
				const FVector BaseTangentialVel = MovementBaseUtility::GetMovementBaseTangentialVelocity(MovementBaseInterfaceData, CharacterOwner->GetBasedMovement().BoneName, CharacterBasePosition);
				BaseVelocity += BaseTangentialVel;
			}

			if (bImpartBaseVelocityX)
			{
				Result.X = BaseVelocity.X;
			}
			if (bImpartBaseVelocityY)
			{
				Result.Y = BaseVelocity.Y;
			}
			if (bImpartBaseVelocityZ)
			{
				Result.Z = BaseVelocity.Z;
			}
		}
	}

	return Result;
}

void UVRCharacterMovementComponent::FindFloor(const FVector& CapsuleLocation, FFindFloorResult& OutFloorResult, bool bCanUseCachedLocation, const FHitResult* DownwardSweepResult) const
{
	SCOPE_CYCLE_COUNTER(STAT_CharFindFloor);

	if (!HasValidData() || !UpdatedComponent->IsQueryCollisionEnabled())
	{
		OutFloorResult.Clear();
		return;
	}

	check(CharacterOwner->GetCapsuleComponent());

	FVector UseCapsuleLocation = CapsuleLocation;
	if (VRRootCapsule)
		UseCapsuleLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();

	const float HeightCheckAdjust = ((IsMovingOnGround() || IsClimbing()) ? MAX_FLOOR_DIST + UE_KINDA_SMALL_NUMBER : -MAX_FLOOR_DIST);

	float FloorSweepTraceDist = FMath::Max(MAX_FLOOR_DIST, MaxStepHeight + HeightCheckAdjust);
	float FloorLineTraceDist = FloorSweepTraceDist;
	bool bNeedToValidateFloor = true;

	FFindFloorResult LastFloor = CurrentFloor;

	if (FloorLineTraceDist > 0.f || FloorSweepTraceDist > 0.f)
	{
		UCharacterMovementComponent* MutableThis = const_cast<UCharacterMovementComponent*>((UCharacterMovementComponent*)this);

		if (bAlwaysCheckFloor || !bCanUseCachedLocation || bForceNextFloorCheck || bJustTeleported)
		{
			MutableThis->bForceNextFloorCheck = false;
			ComputeFloorDist(UseCapsuleLocation, FloorLineTraceDist, FloorSweepTraceDist, OutFloorResult, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius(), DownwardSweepResult);
		}
		else
		{

			const FMovementBaseInterfaceData* MovementBaseInterfaceData = GetMovementBaseInterfaceData();

			const bool bHasValidMovementBase = MovementBaseInterfaceData && MovementBaseInterfaceData->IsValid();
			const AActor* BaseActor = bHasValidMovementBase ? Cast<const AActor>(MovementBaseInterfaceData->GetMovementBaseObject()) : nullptr;
			const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

			if (bHasValidMovementBase)
			{
				MutableThis->bForceNextFloorCheck = !CollisionEnabledHasQuery(MovementBaseInterfaceData->GetBodyInstanceOwner()->GetCollisionEnabled())
					|| MovementBaseInterfaceData->GetBodyInstanceOwner()->GetCollisionResponseToChannel(CollisionChannel) != ECR_Block
					|| MovementBaseUtility::IsDynamicBase(MovementBaseInterfaceData);
			}

			const bool IsActorBasePendingKill = !IsValid(BaseActor);

			if (!bForceNextFloorCheck && !IsActorBasePendingKill && bHasValidMovementBase)
			{

				OutFloorResult = CurrentFloor;
				bNeedToValidateFloor = false;
			}
			else
			{
				MutableThis->bForceNextFloorCheck = false;
				ComputeFloorDist(UseCapsuleLocation, FloorLineTraceDist, FloorSweepTraceDist, OutFloorResult, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius(), DownwardSweepResult);
			}
		}
	}

	if (VRRootCapsule && VRRootCapsule->bUseWalkingCollisionOverride && OutFloorResult.bBlockingHit && OutFloorResult.FloorDist <= 0.0f)
	{ 

		if (OutFloorResult.FloorDist <= -FMath::Max(MAX_FLOOR_DIST, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius()))
		{

			ECollisionResponse FloorResponse;
			if (OutFloorResult.HitResult.Component.IsValid())
			{
				FloorResponse = OutFloorResult.HitResult.Component->GetCollisionResponseToChannel(VRRootCapsule->WalkingCollisionOverride);
				if (FloorResponse == ECR_Ignore || FloorResponse == ECR_Overlap)
					OutFloorResult = LastFloor;	
			}
		}
	}

	if (bNeedToValidateFloor && OutFloorResult.bBlockingHit && !OutFloorResult.bLineTrace)
	{
		const bool bCheckRadius = true;
		if (ShouldComputePerchResult(OutFloorResult.HitResult, bCheckRadius))
		{
			float MaxPerchFloorDist = FMath::Max(MAX_FLOOR_DIST, MaxStepHeight + HeightCheckAdjust);
			if (IsMovingOnGround() || IsClimbing())
			{
				MaxPerchFloorDist += FMath::Max(0.f, PerchAdditionalHeight);
			}

			FFindFloorResult PerchFloorResult;
			if (ComputePerchResult(GetValidPerchRadius(), OutFloorResult.HitResult, MaxPerchFloorDist, PerchFloorResult))
			{

				const float AvgFloorDist = (MIN_FLOOR_DIST + MAX_FLOOR_DIST) * 0.5f;
				const float MoveUpDist = (AvgFloorDist - OutFloorResult.FloorDist);
				if (MoveUpDist + PerchFloorResult.FloorDist >= MaxPerchFloorDist)
				{
					OutFloorResult.FloorDist = AvgFloorDist;
				}

				if (!OutFloorResult.bWalkableFloor)
				{
					OutFloorResult.SetFromLineTrace(PerchFloorResult.HitResult, OutFloorResult.FloorDist, FMath::Max(OutFloorResult.FloorDist, MIN_FLOOR_DIST), true);
				}
			}
			else
			{

				OutFloorResult.bWalkableFloor = false;
			}
		}
	}
}

float UVRCharacterMovementComponent::ImmersionDepth() const
{
	float depth = 0.f;

	if (CharacterOwner && GetPhysicsVolume()->bWaterVolume)
	{
		const float CollisionHalfHeight = CharacterOwner->GetSimpleCollisionHalfHeight();

		if ((CollisionHalfHeight == 0.f) || (Buoyancy == 0.f))
		{
			depth = 1.f;
		}
		else
		{
			UBrushComponent* VolumeBrushComp = GetPhysicsVolume()->GetBrushComponent();
			FHitResult Hit(1.f);
			if (VolumeBrushComp)
			{
				FVector TraceStart;
				FVector TraceEnd;

				if (VRRootCapsule)
				{
					TraceStart = VRRootCapsule->OffsetComponentToWorld.GetLocation() + CollisionHalfHeight * -GetGravityDirection();
					TraceEnd = VRRootCapsule->OffsetComponentToWorld.GetLocation() - CollisionHalfHeight * -GetGravityDirection();
				}
				else
				{
					TraceStart = UpdatedComponent->GetComponentLocation() + CollisionHalfHeight * -GetGravityDirection();
					TraceEnd = UpdatedComponent->GetComponentLocation() - CollisionHalfHeight * -GetGravityDirection();
				}

				FCollisionQueryParams NewTraceParams(CharacterMovementComponentStatics::ImmersionDepthName, true);
				VolumeBrushComp->LineTraceComponent(Hit, TraceStart, TraceEnd, NewTraceParams);
			}

			depth = (Hit.Time == 1.f) ? 1.f : (1.f - Hit.Time);
		}
	}
	return depth;
}

bool UVRCharacterMovementComponent::TryToLeaveNavWalking()
{
	SetNavWalkingPhysics(false);

	bool bCanTeleport = true;
	if (CharacterOwner)
	{
		FVector CollisionFreeLocation;
		if (VRRootCapsule)
			CollisionFreeLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();
		else
			CollisionFreeLocation =	UpdatedComponent->GetComponentLocation();

		bCanTeleport = GetWorld()->FindTeleportSpot(CharacterOwner, CollisionFreeLocation, UpdatedComponent->GetComponentRotation());
		if (bCanTeleport)
		{

			if (VRRootCapsule)
			{

				CharacterOwner->SetActorLocation(CollisionFreeLocation - (VRRootCapsule->OffsetComponentToWorld.GetLocation() - UpdatedComponent->GetComponentLocation()));
			}
			else
				CharacterOwner->SetActorLocation(CollisionFreeLocation);
		}
		else
		{
			SetNavWalkingPhysics(true);
		}
	}

	bWantsToLeaveNavWalking = !bCanTeleport;
	return bCanTeleport;
}

void UVRCharacterMovementComponent::PhysFlying(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		if (bCheatFlying && Acceleration.IsZero())
		{
			Velocity = FVector::ZeroVector;
		}
		const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction;
		CalcVelocity(deltaTime, Friction, true, GetMaxBrakingDeceleration());
	}

	ApplyRootMotionToVelocity(deltaTime);

	LastPreAdditiveVRVelocity = (AdditionalVRInputVector) / deltaTime;
	bool bExtremeInput = false;
	if (LastPreAdditiveVRVelocity.SizeSquared() > FMath::Square(TrackingLossThreshold))
	{

		AdditionalVRInputVector = FVector::ZeroVector;
		LastPreAdditiveVRVelocity = FVector::ZeroVector;
	}

	RewindVRRelativeMovement();

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted + AdditionalVRInputVector, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		const FVector VelDir = Velocity.GetSafeNormal();
		const float UpDown = VelDir | GetGravityDirection();

		bool bSteppedUp = false;
		if ((FMath::Abs(GetGravitySpaceZ(Hit.ImpactNormal)) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(Hit))
		{
			const FVector::FReal StepZ = GetGravitySpaceZ(UpdatedComponent->GetComponentLocation());
			bSteppedUp = StepUp(GetGravityDirection(), (Adjusted + AdditionalVRInputVector) * (1.f - Hit.Time), Hit);
			if (bSteppedUp)
			{
				const FVector::FReal LocationZ = GetGravitySpaceZ(UpdatedComponent->GetComponentLocation()) + (GetGravitySpaceZ(OldLocation) - StepZ);
				SetGravitySpaceZ(OldLocation, LocationZ);
			}
		}

		if (!bSteppedUp)
		{

			HandleImpact(Hit, deltaTime, Adjusted);
			SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
		}
	}

	if (!bJustTeleported)
	{
		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{

			Velocity = ((UpdatedComponent->GetComponentLocation() - OldLocation)) / deltaTime;
		}

		RestorePreAdditiveVRMotionVelocity();
	}
}

void UVRCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharPhysFalling);

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const FVector FallAcceleration = ProjectToGravityFloor(GetFallingLateralAcceleration(deltaTime));
	const bool bHasLimitedAirControl = ShouldLimitAirControl(deltaTime, FallAcceleration);

	RewindVRRelativeMovement();

	float remainingTime = deltaTime;
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
	{
		Iterations++;
		float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FVector OldCapsuleLocation = VRRootCapsule ? VRRootCapsule->OffsetComponentToWorld.GetLocation() : OldLocation;

		const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
		bJustTeleported = false;

		const FVector OldVelocityWithRootMotion = Velocity;

		RestorePreAdditiveRootMotionVelocity();

		const FVector OldVelocity = Velocity;

		const float MaxDecel = GetMaxBrakingDeceleration();
		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{

			{

				TGuardValue<FVector> RestoreAcceleration(Acceleration, FallAcceleration);
				if (HasCustomGravity())
				{
					Velocity = ProjectToGravityFloor(Velocity);
					const FVector GravityRelativeOffset = OldVelocity - Velocity;
					CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
					Velocity += GravityRelativeOffset;
				}
				else
				{
					Velocity.Z = 0.f;
					CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
					Velocity.Z = OldVelocity.Z;
				}
			}
		}

		const FVector Gravity = -GetGravityDirection() * GetGravityZ();

		float GravityTime = timeTick;

		bool bEndingJumpForce = false;
		if (CharacterOwner->JumpForceTimeRemaining > 0.0f)
		{

			const float JumpForceTime = FMath::Min(CharacterOwner->JumpForceTimeRemaining, timeTick);
			GravityTime = bApplyGravityWhileJumping ? timeTick : FMath::Max(0.0f, timeTick - JumpForceTime);

			CharacterOwner->JumpForceTimeRemaining -= JumpForceTime;
			if (CharacterOwner->JumpForceTimeRemaining <= 0.0f)
			{
				CharacterOwner->ResetJumpState();
				bEndingJumpForce = true;
			}
		}

		Velocity = NewFallVelocity(Velocity, Gravity, GravityTime);

		ApplyRootMotionToVelocity(timeTick);
		DecayFormerBaseVelocity(timeTick);

		const FVector::FReal GravityRelativeOldVelocityWithRootMotionZ = GetGravitySpaceZ(OldVelocityWithRootMotion);
		static const auto CVarForceJumpPeakSubstep = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ForceJumpPeakSubstep"));
		if (CVarForceJumpPeakSubstep->GetInt() && GravityRelativeOldVelocityWithRootMotionZ > 0.f && GetGravitySpaceZ(Velocity) <= 0.f && NumJumpApexAttempts < MaxJumpApexAttemptsPerSimulation)
		{
			const FVector DerivedAccel = (Velocity - OldVelocityWithRootMotion) / timeTick;
			const FVector::FReal GravityRelativeDerivedAccelZ = GetGravitySpaceZ(DerivedAccel);
			if (!FMath::IsNearlyZero(GravityRelativeDerivedAccelZ))
			{
				const float TimeToApex = -GravityRelativeOldVelocityWithRootMotionZ / GravityRelativeDerivedAccelZ;

				const float ApexTimeMinimum = 0.0001f;
				if (TimeToApex >= ApexTimeMinimum && TimeToApex < timeTick)
				{
					const FVector ApexVelocity = OldVelocityWithRootMotion + (DerivedAccel * TimeToApex);
					if (HasCustomGravity())
					{
						Velocity = ProjectToGravityFloor(ApexVelocity); 
					}
					else
					{
						Velocity = ApexVelocity;
						Velocity.Z = 0.f; 

					}

					const float TimeToRefund = (timeTick - TimeToApex);

					remainingTime += TimeToRefund;
					timeTick = TimeToApex;
					Iterations--;
					NumJumpApexAttempts++;

					for (TSharedPtr<FRootMotionSource> RootMotionSource : CurrentRootMotion.RootMotionSources)
					{
						const float RewoundRMSTime = FMath::Max(0.0f, RootMotionSource->GetTime() - TimeToRefund);
						RootMotionSource->SetTime(RewoundRMSTime);
					}
				}
			}
		}

		ApplyRootMotionToVelocity(timeTick);

		if (bNotifyApex && (GetGravitySpaceZ(Velocity) < 0.f))
		{

			bNotifyApex = false;
			NotifyJumpApex();
		}

		FVector Adjusted = (0.5f * (OldVelocityWithRootMotion + Velocity) * timeTick) + ((AdditionalVRInputVector / deltaTime) * timeTick);

		ApplyVRMotionToVelocity(deltaTime);

		if (bEndingJumpForce && !bApplyGravityWhileJumping)
		{

			const float NonGravityTime = FMath::Max(0.f, timeTick - GravityTime);
			Adjusted = ((OldVelocityWithRootMotion * NonGravityTime) + (0.5f * (OldVelocityWithRootMotion + Velocity) * GravityTime)) ;
		}

		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Adjusted, PawnRotation, true, Hit);

		if (!HasValidData())
		{
			RestorePreAdditiveVRMotionVelocity();
			return;
		}

		float LastMoveTimeSlice = timeTick;
		float subTimeTickRemaining = timeTick * (1.f - Hit.Time);

		if (IsSwimming()) 
		{
			RestorePreAdditiveVRMotionVelocity();
			remainingTime += subTimeTickRemaining;
			StartSwimmingVR(OldCapsuleLocation, OldVelocity, timeTick, remainingTime, Iterations);
			return;
		}
		else if (Hit.bBlockingHit)
		{
			if (IsValidLandingSpot(VRRootCapsule->OffsetComponentToWorld.GetLocation(), Hit))
			{
				RestorePreAdditiveVRMotionVelocity();
				remainingTime += subTimeTickRemaining;
				ProcessLanded(Hit, remainingTime, Iterations);
				return;
			}
			else
			{

				Adjusted = Velocity * timeTick;

				if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(timeTick, Adjusted, Hit))
				{
					FVector PawnLocation = UpdatedComponent->GetComponentLocation();
					if (VRRootCapsule)
						PawnLocation = VRRootCapsule->OffsetComponentToWorld.GetLocation();

					FFindFloorResult FloorResult;
					FindFloor(PawnLocation, FloorResult, false, NULL);

					if (!FloorResult.bLineTrace && FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult))
					{

						remainingTime += subTimeTickRemaining;
						ProcessLanded(FloorResult.HitResult, remainingTime, Iterations);

						if (bAutoOrientToFloorNormal && FloorResult.IsWalkableFloor())
						{

							AutoTraceAndSetCharacterToNewGravity(FloorResult.HitResult, timeTick);
						}
						return;
					}
				}

				HandleImpact(Hit, LastMoveTimeSlice, Adjusted);

				if (!HasValidData() || !IsFalling())
				{
					RestorePreAdditiveVRMotionVelocity();
					return;
				}

				FVector VelocityNoAirControl = OldVelocity;
				FVector AirControlAccel = Acceleration;
				if (bHasLimitedAirControl)
				{

					{

						TGuardValue<FVector> RestoreAcceleration(Acceleration, FVector::ZeroVector);
						TGuardValue<FVector> RestoreVelocity(Velocity, OldVelocity);
						if (HasCustomGravity())
						{
							Velocity = ProjectToGravityFloor(Velocity);
							const FVector GravityRelativeOffset = OldVelocity - Velocity;
							CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
							VelocityNoAirControl = Velocity + GravityRelativeOffset;
						}
						else
						{
							Velocity.Z = 0.f;
							CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
							VelocityNoAirControl = FVector(Velocity.X, Velocity.Y, OldVelocity.Z);
						}
						VelocityNoAirControl = NewFallVelocity(VelocityNoAirControl, Gravity, GravityTime);
					}

					const bool bCheckLandingSpot = false; 
					AirControlAccel = (Velocity - VelocityNoAirControl) / timeTick;
					const FVector AirControlDeltaV = LimitAirControl(LastMoveTimeSlice, AirControlAccel, Hit, bCheckLandingSpot) * LastMoveTimeSlice;
					Adjusted = (VelocityNoAirControl + AirControlDeltaV) * LastMoveTimeSlice;
				}

				const FVector OldHitNormal = Hit.Normal;
				const FVector OldHitImpactNormal = Hit.ImpactNormal;
				FVector Delta = ComputeSlideVector(Adjusted, 1.f - Hit.Time, OldHitNormal, Hit);

				static const auto CVarUseTargetVelocityOnImpact = IConsoleManager::Get().FindConsoleVariable(TEXT("p.UseTargetVelocityOnImpact"));
				FMovementBaseInterfaceData HitMovementBaseInterfaceData(MovementBaseUtility::GetMovementBaseDataFromHitResult(&Hit));
				if (CVarUseTargetVelocityOnImpact->GetBool() && !Velocity.IsNearlyZero() && MovementBaseUtility::IsSimulatedBase(&HitMovementBaseInterfaceData))
				{
					const FVector ContactVelocity = MovementBaseUtility::GetMovementBaseVelocity(&HitMovementBaseInterfaceData, NAME_None) + MovementBaseUtility::GetMovementBaseTangentialVelocity(&HitMovementBaseInterfaceData, NAME_None, Hit.ImpactPoint);
					const FVector NewVelocity = Velocity - Hit.ImpactNormal * FVector::DotProduct(Velocity - ContactVelocity, Hit.ImpactNormal);
					Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? ProjectToGravityFloor(Velocity) + GetGravitySpaceComponentZ(NewVelocity) : NewVelocity;
				}
				else if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && !bJustTeleported)
				{
					const FVector NewVelocity = (Delta / subTimeTickRemaining);
					Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? ProjectToGravityFloor(Velocity) + GetGravitySpaceComponentZ(NewVelocity) : NewVelocity;
				}

				if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && (Delta | Adjusted) > 0.f)
				{

					SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);

					if (Hit.bBlockingHit)
					{

						LastMoveTimeSlice = subTimeTickRemaining;
						subTimeTickRemaining = subTimeTickRemaining * (1.f - Hit.Time);

						if (IsValidLandingSpot(VRRootCapsule->OffsetComponentToWorld.GetLocation(), Hit))
						{
							RestorePreAdditiveVRMotionVelocity();
							remainingTime += subTimeTickRemaining;
							ProcessLanded(Hit, remainingTime, Iterations);
							return;
						}

						HandleImpact(Hit, LastMoveTimeSlice, Delta);

						if (!HasValidData() || !IsFalling())
						{
							RestorePreAdditiveVRMotionVelocity();
							return;
						}

						if (bHasLimitedAirControl && GetGravitySpaceZ(Hit.Normal) > CharacterMovementConstants::VERTICAL_SLOPE_NORMAL_ZVR)
						{
							const FVector LastMoveNoAirControl = VelocityNoAirControl * LastMoveTimeSlice;
							Delta = ComputeSlideVector(LastMoveNoAirControl, 1.f, OldHitNormal, Hit);
						}

						FVector PreTwoWallDelta = Delta;
						TwoWallAdjust(Delta, Hit, OldHitNormal);

						Delta = HandleSlopeBoosting(Delta, PreTwoWallDelta, 1.f - Hit.Time, Hit.Normal, Hit);

						if (bHasLimitedAirControl)
						{
							const bool bCheckLandingSpot = false; 
							const FVector AirControlDeltaV = LimitAirControl(subTimeTickRemaining, AirControlAccel, Hit, bCheckLandingSpot) * subTimeTickRemaining;

							if (FVector::DotProduct(AirControlDeltaV, OldHitNormal) > 0.f)
							{
								Delta += (AirControlDeltaV * subTimeTickRemaining);
							}
						}

						if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && !bJustTeleported)
						{
							const FVector NewVelocity = (Delta / subTimeTickRemaining);
							Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? ProjectToGravityFloor(Velocity) + GetGravitySpaceComponentZ(NewVelocity) : NewVelocity;
						}

						bool bDitch = ((GetGravitySpaceZ(OldHitImpactNormal) > 0.f) && (GetGravitySpaceZ(Hit.ImpactNormal) > 0.f) && (FMath::Abs(GetGravitySpaceZ(Delta)) <= UE_KINDA_SMALL_NUMBER) && ((Hit.ImpactNormal | OldHitImpactNormal) < 0.f));
						SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
						if (Hit.Time == 0.f)
						{

							FVector SideDelta = ProjectToGravityFloor(OldHitNormal + Hit.ImpactNormal).GetSafeNormal();
							if (SideDelta.IsNearlyZero())
							{
								if (HasCustomGravity())
								{
									const FVector GravityRelativeHitNormal = RotateWorldToGravity(OldHitNormal);
									SideDelta = RotateGravityToWorld(FVector(GravityRelativeHitNormal.Y, -GravityRelativeHitNormal.X, 0.f)).GetSafeNormal();
								}
								else
								{
									SideDelta = FVector(OldHitNormal.Y, -OldHitNormal.X, 0).GetSafeNormal();
								}
							}
							SafeMoveUpdatedComponent(SideDelta, PawnRotation, true, Hit);
						}

						if (bDitch || IsValidLandingSpot(VRRootCapsule->OffsetComponentToWorld.GetLocation(), Hit) || Hit.Time == 0.f)
						{
							RestorePreAdditiveVRMotionVelocity();
							remainingTime = 0.f;
							ProcessLanded(Hit, remainingTime, Iterations);
							return;
						}
						else if (GetPerchRadiusThreshold() > 0.f && Hit.Time == 1.f && GetGravitySpaceZ(OldHitImpactNormal) >= GetWalkableFloorZ())
						{

							const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
							const float ZMovedDist = FMath::Abs(GetGravitySpaceZ(PawnLocation - OldLocation));
							const float MovedDist2D = ProjectToGravityFloor(PawnLocation - OldLocation).Size();
							if (ZMovedDist <= 0.2f * timeTick && MovedDist2D <= 4.f * timeTick)
							{
								FVector GravityRelativeVelocity = RotateWorldToGravity(Velocity);
								GravityRelativeVelocity.X += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
								GravityRelativeVelocity.Y += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
								GravityRelativeVelocity.Z = FMath::Max<float>(JumpZVelocity * 0.25f, 1.f);
								Velocity = RotateGravityToWorld(GravityRelativeVelocity);
								Delta = Velocity * timeTick;
								SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
							}
						}
					}
				}
			}
		}
		else
		{

			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, false, NULL);

			if (CurrentFloor.IsWalkableFloor())
			{

				if (CurrentFloor.GetDistanceToFloor() < (MIN_FLOOR_DIST + MAX_FLOOR_DIST) / 2)
				{

					AdjustFloorHeight();
					FMovementBaseInterfaceData MovementBaseInterfaceData(CurrentFloor.HitResult.Component.Get());
					SetBase(&MovementBaseInterfaceData, CurrentFloor.HitResult.BoneName);

					if (IsValidLandingSpot(VRRootCapsule->OffsetComponentToWorld.GetLocation(), CurrentFloor.HitResult))
					{
						remainingTime += subTimeTickRemaining;
						ProcessLanded(CurrentFloor.HitResult, remainingTime, Iterations);
						return;
					}
				}
			}
			else if (CurrentFloor.HitResult.bStartPenetrating)
			{

				FHitResult Hit2(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit2.TraceStart + RotateGravityToWorld(FVector(0.f, 0.f, MAX_FLOOR_DIST));
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit2);
				ResolvePenetration(RequestedAdjustment, Hit2, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			if (bAutoOrientToFloorNormal && CurrentFloor.IsWalkableFloor())
			{

				AutoTraceAndSetCharacterToNewGravity(CurrentFloor.HitResult, timeTick);
			}
		}

		const FVector GravityProjectedVelocity = ProjectToGravityFloor(Velocity);
		if (GravityProjectedVelocity.SizeSquared() <= UE_KINDA_SMALL_NUMBER * 10.f)
		{
			Velocity = GetGravitySpaceComponentZ(Velocity);
		}

		RestorePreAdditiveVRMotionVelocity();
	}
}

void UVRCharacterMovementComponent::PhysNavWalking(float deltaTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharPhysNavWalking);

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if ((!CharacterOwner || !CharacterOwner->GetController()) && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	const EMovementMode StartingMovementMode = MovementMode;
	const uint8 StartingCustomMovementMode = CustomMovementMode;

	RewindVRRelativeMovement();

	RestorePreAdditiveRootMotionVelocity();

	MaintainHorizontalGroundVelocity();
	devCodeVR(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysNavWalking: Velocity contains NaN before CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	Acceleration = ProjectToGravityFloor(Acceleration);

	CalcVelocity(deltaTime, GroundFriction, false, BrakingDecelerationWalking);
	devCodeVR(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysNavWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	ApplyRootMotionToVelocity(deltaTime);
	ApplyVRMotionToVelocity(deltaTime);

	if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
	{

		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	Iterations++;

	const FVector DesiredMove = ProjectToGravityFloor(Velocity);

	const FVector OldLocation = GetActorFeetLocationVR();
	const FVector DeltaMove = DesiredMove * deltaTime;
	const bool bDeltaMoveNearlyZero = DeltaMove.IsNearlyZero();

	FVector AdjustedDest = OldLocation + DeltaMove;
	FNavLocation DestNavLocation;

	bool bSameNavLocation = false;
	if (CachedNavLocation.NodeRef != INVALID_NAVNODEREF)
	{
		if (bProjectNavMeshWalking)
		{
			const float DistSq2D = ProjectToGravityFloor(OldLocation - CachedNavLocation.Location).SizeSquared();
			const float DistZ = FMath::Abs(GetGravitySpaceZ(OldLocation - CachedNavLocation.Location));

			const float TotalCapsuleHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;
			const float ProjectionScale = (GetGravitySpaceZ(OldLocation) > GetGravitySpaceZ(CachedNavLocation.Location)) ? NavMeshProjectionHeightScaleUp : NavMeshProjectionHeightScaleDown;
			const float DistZThr = TotalCapsuleHeight * FMath::Max(0.f, ProjectionScale);

			bSameNavLocation = (DistSq2D <= UE_KINDA_SMALL_NUMBER) && (DistZ < DistZThr);
		}
		else
		{
			bSameNavLocation = CachedNavLocation.Location.Equals(OldLocation);
		}

		if (bDeltaMoveNearlyZero && bSameNavLocation)
		{
			if (const INavigationDataInterface* NavData = GetNavData())
			{
				if (!NavData->IsNodeRefValid(CachedNavLocation.NodeRef))
				{
					CachedNavLocation.NodeRef = INVALID_NAVNODEREF;
					bSameNavLocation = false;
				}
			}
		}
	}

	if (bDeltaMoveNearlyZero && bSameNavLocation)
	{
		DestNavLocation = CachedNavLocation;
		UE_LOGF(LogVRCharacterMovement, VeryVerbose, "%ls using cached navmesh location! (bProjectNavMeshWalking = %d)", *GetNameSafe(CharacterOwner), bProjectNavMeshWalking);
	}
	else
	{
		SCOPE_CYCLE_COUNTER(STAT_CharNavProjectPoint);

		if (bSameNavLocation && bProjectNavMeshWalking)
		{
			SetGravitySpaceZ(AdjustedDest, GetGravitySpaceZ(CachedNavLocation.Location));
		}

		bool bFoundPointOnNavMesh = false;
		if (bSlideAlongNavMeshEdge)
		{
			if (const INavigationDataInterface* NavDataInterface = GetNavData())
			{
				const IPathFollowingAgentInterface* PathFollowingAgent = GetPathFollowingAgent();
				const bool bIsOnNavLink = PathFollowingAgent && PathFollowingAgent->IsFollowingNavLink();

				if (!bIsOnNavLink)
				{
					FNavLocation StartingNavFloorLocation;
					bool bHasValidCachedNavLocation = NavDataInterface->IsNodeRefValid(CachedNavLocation.NodeRef);

					if (!bHasValidCachedNavLocation)
					{
						bHasValidCachedNavLocation = FindNavFloor(OldLocation, OUT StartingNavFloorLocation);
					}
					else
					{
						StartingNavFloorLocation = CachedNavLocation;
					}

					if (bHasValidCachedNavLocation)
					{
						bFoundPointOnNavMesh = NavDataInterface->FindMoveAlongSurface(StartingNavFloorLocation, AdjustedDest, OUT DestNavLocation);

						if (bFoundPointOnNavMesh)
						{
							AdjustedDest = ProjectToGravityFloor(DestNavLocation.Location) + GetGravitySpaceComponentZ(AdjustedDest);
						}
					}
				}
				else
				{
					bFoundPointOnNavMesh = FindNavFloor(AdjustedDest, DestNavLocation);
				}
			}
		}
		else
		{
			bFoundPointOnNavMesh = FindNavFloor(AdjustedDest, DestNavLocation);
		}

		if (!bFoundPointOnNavMesh)		
		{
			RestorePreAdditiveVRMotionVelocity();
			SetMovementMode(MOVE_Walking);
			return;
		}

		CachedNavLocation = DestNavLocation;
	}

	if (DestNavLocation.NodeRef != INVALID_NAVNODEREF)
	{
		FVector NewLocation = ProjectToGravityFloor(AdjustedDest) + GetGravitySpaceComponentZ(DestNavLocation.Location);
		if (bProjectNavMeshWalking)
		{
			SCOPE_CYCLE_COUNTER(STAT_CharNavProjectLocation);
			const float TotalCapsuleHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;
			const float UpOffset = TotalCapsuleHeight * FMath::Max(0.f, NavMeshProjectionHeightScaleUp);
			const float DownOffset = TotalCapsuleHeight * FMath::Max(0.f, NavMeshProjectionHeightScaleDown);
			NewLocation = ProjectLocationFromNavMesh(deltaTime, OldLocation, NewLocation, UpOffset, DownOffset);
		}

		FVector AdjustedDelta = NewLocation - OldLocation;

		if (!AdjustedDelta.IsNearlyZero())
		{

			FHitResult HitResult;
			SafeMoveUpdatedComponent(AdjustedDelta, UpdatedComponent->GetComponentQuat(), bSweepWhileNavWalking, HitResult);

		}

		if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasVelocity())
		{
			Velocity = (GetActorFeetLocationVR() - OldLocation) / deltaTime;
			MaintainHorizontalGroundVelocity();
		}

		bJustTeleported = false;
	}
	else
	{
		StartFalling(Iterations, deltaTime, deltaTime, DeltaMove, OldLocation);
	}

	RestorePreAdditiveVRMotionVelocity();
}

void UVRCharacterMovementComponent::PhysSwimming(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	RewindVRRelativeMovement();

	RestorePreAdditiveRootMotionVelocity();

	float NetFluidFriction = 0.f;
	float Depth = ImmersionDepth();
	float NetBuoyancy = Buoyancy * Depth;
	float OriginalAccelZ = GetGravitySpaceZ(Acceleration);
	bool bLimitedUpAccel = false;

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (GetGravitySpaceZ(Velocity) > 0.33f * MaxSwimSpeed) && (NetBuoyancy != 0.f))
	{

		SetGravitySpaceZ(Velocity, FMath::Max<FVector::FReal>(0.33f * MaxSwimSpeed, GetGravitySpaceZ(Velocity) * Depth * Depth));
	}
	else if (Depth < 0.65f)
	{
		bLimitedUpAccel = (OriginalAccelZ > 0.f);
		SetGravitySpaceZ(Acceleration, FMath::Min<FVector::FReal>(0.1f, OriginalAccelZ));
	}

	Iterations++;
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	bJustTeleported = false;
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction * Depth;
		CalcVelocity(deltaTime, Friction, true, GetMaxBrakingDeceleration());
		Velocity += (GetGravityZ() * deltaTime * (1.f - NetBuoyancy)) * -GetGravityDirection();
	}

	ApplyRootMotionToVelocity(deltaTime);
	ApplyVRMotionToVelocity(deltaTime);

	FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	const float remainingTime = deltaTime * Swim(Adjusted, Hit);

	if (!IsSwimming())
	{
		RestorePreAdditiveVRMotionVelocity();
		StartNewPhysics(remainingTime, Iterations);
		return;
	}

	if (Hit.Time < 1.f && CharacterOwner)
	{
		HandleSwimmingWallHit(Hit, deltaTime);
		if (bLimitedUpAccel && (GetGravitySpaceZ(Velocity) >= 0.f))
		{

			Velocity += OriginalAccelZ * deltaTime * -GetGravityDirection();
			Adjusted = Velocity * (1.f - Hit.Time)*deltaTime;
			SwimVR(Adjusted, Hit);
			if (!IsSwimming())
			{
				RestorePreAdditiveVRMotionVelocity();
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
		}

		const FVector VelDir = Velocity.GetSafeNormal();
		const float UpDown = VelDir | GetGravityDirection();

		bool bSteppedUp = false;
		if ((FMath::Abs(GetGravitySpaceZ(Hit.ImpactNormal)) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(Hit))
		{
			const float StepZ = GetGravitySpaceZ(UpdatedComponent->GetComponentLocation());
			const FVector RealVelocity = Velocity;
			SetGravitySpaceZ(Velocity, 1.f);	
			bSteppedUp = StepUp(GetGravityDirection(), Adjusted * (1.f - Hit.Time), Hit);
			if (bSteppedUp)
			{

				if (!IsSwimming())
				{
					RestorePreAdditiveVRMotionVelocity();
					StartNewPhysics(remainingTime, Iterations);
					return;
				}
				SetGravitySpaceZ(OldLocation, GetGravitySpaceZ(UpdatedComponent->GetComponentLocation()) + (GetGravitySpaceZ(OldLocation) - StepZ));
			}
			Velocity = RealVelocity;
		}

		if (!bSteppedUp)
		{

			HandleImpact(Hit, deltaTime, Adjusted);
			SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
		}
	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && !bJustTeleported && ((deltaTime - remainingTime) > UE_KINDA_SMALL_NUMBER) && CharacterOwner)
	{
		const bool bWaterJump = !GetPhysicsVolume()->bWaterVolume;
		const FVector::FReal VelZ = GetGravitySpaceZ(Velocity);
		Velocity = ((UpdatedComponent->GetComponentLocation() - OldLocation)) / (deltaTime - remainingTime);
		if (bWaterJump)
		{
			SetGravitySpaceZ(Velocity, VelZ);
		}
	}

	if (!GetPhysicsVolume()->bWaterVolume && IsSwimming())
	{
		SetMovementMode(MOVE_Falling); 
	}

	RestorePreAdditiveVRMotionVelocity();

	if (!IsSwimming())
	{
		StartNewPhysics(remainingTime, Iterations);
	}
}

void UVRCharacterMovementComponent::StartSwimmingVR(FVector OldLocation, FVector OldVelocity, float timeTick, float remainingTime, int32 Iterations)
{
	if (remainingTime < MIN_TICK_TIME || timeTick < MIN_TICK_TIME)
	{
		return;
	}

	FVector NewLocation = VRRootCapsule ? VRRootCapsule->OffsetComponentToWorld.GetLocation() : UpdatedComponent->GetComponentLocation();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && !bJustTeleported)
	{
		Velocity = (NewLocation - OldLocation) / timeTick; 
		Velocity = 2.f*Velocity - OldVelocity; 
		Velocity = Velocity.GetClampedToMaxSize(GetPhysicsVolume()->TerminalVelocity);
	}
	const FVector End = FindWaterLine(NewLocation, OldLocation);
	float waterTime = 0.f;
	if (End != NewLocation)
	{
		const float ActualDist = (NewLocation - OldLocation).Size();
		if (ActualDist > UE_KINDA_SMALL_NUMBER)
		{
			waterTime = timeTick * (End - NewLocation).Size() / ActualDist;
			remainingTime += waterTime;
		}
		MoveUpdatedComponent(End - NewLocation, UpdatedComponent->GetComponentQuat(), true);
	}
	const FVector::FReal GravityRelativeVelocityZ = GetGravitySpaceZ(Velocity);
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (GravityRelativeVelocityZ > 2.f * CharacterMovementConstants::SWIMBOBSPEEDVR) && (GravityRelativeVelocityZ < 0.f)) 
	{
		SetGravitySpaceZ(Velocity, CharacterMovementConstants::SWIMBOBSPEEDVR - ProjectToGravityFloor(Velocity).Size() * 0.7f); 
	}
	if ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
	{
		PhysSwimming(remainingTime, Iterations);
	}
}

float UVRCharacterMovementComponent::SwimVR(FVector Delta, FHitResult& Hit)
{
	FVector Start = VRRootCapsule ? VRRootCapsule->OffsetComponentToWorld.GetLocation() : UpdatedComponent->GetComponentLocation();

	float airTime = 0.f;
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (!GetPhysicsVolume()->bWaterVolume) 
	{
		FVector NewLoc = VRRootCapsule ? VRRootCapsule->OffsetComponentToWorld.GetLocation() : UpdatedComponent->GetComponentLocation();

		const FVector End = FindWaterLine(Start, NewLoc);
		const float DesiredDist = Delta.Size();
		if (End != NewLoc && DesiredDist > UE_KINDA_SMALL_NUMBER)
		{
			airTime = (End - NewLoc).Size() / DesiredDist;
			if (((NewLoc - Start) | (End - NewLoc)) > 0.f)
			{
				airTime = 0.f;
			}
			SafeMoveUpdatedComponent(End - NewLoc, UpdatedComponent->GetComponentQuat(), true, Hit);
		}
	}
	return airTime;
}

bool UVRCharacterMovementComponent::CheckWaterJump(FVector CheckPoint, FVector& WallNormal)
{
	if (!HasValidData())
	{
		return false;
	}
	FVector currentLoc = VRRootCapsule ? VRRootCapsule->OffsetComponentToWorld.GetLocation() : UpdatedComponent->GetComponentLocation();

	CheckPoint = ProjectToGravityFloor(CheckPoint);
	FVector CheckNorm = CheckPoint.GetSafeNormal();
	float PawnCapsuleRadius, PawnCapsuleHalfHeight;
	CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnCapsuleRadius, PawnCapsuleHalfHeight);
	CheckPoint = currentLoc + 1.2f * PawnCapsuleRadius * CheckNorm;
	FHitResult HitInfo(1.f);
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CheckWaterJump), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);
	FCollisionShape CapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_None);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bHit = GetWorld()->SweepSingleByChannel(HitInfo, UpdatedComponent->GetComponentLocation(), CheckPoint, GetWorldToGravityTransform(), CollisionChannel, CapsuleShape, CapsuleParams, ResponseParam);

	if (bHit && !HitInfo.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
	{

		WallNormal = -1.f * HitInfo.ImpactNormal;
		FVector Start = currentLoc;
		Start += MaxOutOfWaterStepHeight * -GetGravityDirection();
		CheckPoint = Start + 3.2f * PawnCapsuleRadius * WallNormal;
		FCollisionQueryParams LineParams(SCENE_QUERY_STAT(CheckWaterJump), true, CharacterOwner);
		FCollisionResponseParams LineResponseParam;
		InitCollisionParams(LineParams, LineResponseParam);
		bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, Start, CheckPoint, CollisionChannel, LineParams, LineResponseParam);

		return !bHit || IsWalkable(HitInfo);
	}
	return false;
}

FBasedPosition UVRCharacterMovementComponent::GetActorFeetLocationBased() const
{
	return FBasedPosition(NULL, GetActorFeetLocationVR());
}

void UVRCharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharProcessLanded);

	if (CharacterOwner && CharacterOwner->ShouldNotifyLanded(Hit))
	{
		CharacterOwner->Landed(Hit);
	}
	if (IsFalling())
	{

		if (GetGroundMovementMode() == MOVE_NavWalking)
		{

			const FVector TestLocation = GetActorFeetLocationVR();
			FNavLocation NavLocation;

			const bool bHasNavigationData = FindNavFloor(TestLocation, NavLocation);
			if (!bHasNavigationData || NavLocation.NodeRef == INVALID_NAVNODEREF)
			{
				SetGroundMovementMode(MOVE_Walking);

				UE_LOGF(LogVRCharacterMovement, Verbose, "ProcessLanded(): %ls tried to go to NavWalking but couldn't find NavMesh! Using Walking instead.", *GetNameSafe(CharacterOwner));
			}
		}

		SetPostLandedPhysics(Hit);
	}

	IPathFollowingAgentInterface* PFAgent = GetPathFollowingAgent();
	if (PFAgent)
	{
		PFAgent->OnLanded();
	}

	StartNewPhysics(remainingTime, Iterations);
}

void UVRCharacterMovementComponent::PostPhysicsTickComponent(float DeltaTime, FCharacterMovementComponentPostPhysicsTickFunction& ThisTickFunction)
{
	if (bDeferUpdateBasedMovement)
	{
		FVRCharacterScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);
		UpdateBasedMovement(DeltaTime);
		SaveBaseLocation();
		bDeferUpdateBasedMovement = false;
	}
}

void UVRCharacterMovementComponent::SimulateMovement(float DeltaSeconds)
{
	if (!HasValidData() || UpdatedComponent->Mobility != EComponentMobility::Movable || UpdatedComponent->IsSimulatingPhysics())
	{
		return;
	}

	const bool bIsSimulatedProxy = (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy);
	const FRepMovement& ConstRepMovement = CharacterOwner->GetReplicatedMovement();

	if (bIsSimulatedProxy &&
		ConstRepMovement.Location.IsZero() &&
		ConstRepMovement.Rotation.IsZero() &&
		ConstRepMovement.LinearVelocity.IsZero())
	{
		return;
	}

	const FBasedMovementInfo& RepBasedMovement = CharacterOwner->GetReplicatedBasedMovement();
	FMovementBaseInterfaceData RepMovementBaseInterfaceData = MovementBaseUtility::GetMovementBaseDataFromPhysicsOwner(RepBasedMovement.MovementBaseInterfaceData.PhysicsObjectOwner.Get());

	if (!RepMovementBaseInterfaceData.IsValid() && CharacterOwner->GetReplicatedBasedMovement().bServerHasBaseComponent)
	{
		UE_LOGF(LogVRCharacterMovement, Verbose, "Base for simulated character '%ls' is not resolved on client, skipping SimulateMovement", *CharacterOwner->GetName());
		return;
	}

	FVector OldVelocity;
	FVector OldLocation;

	{
		FVRCharacterScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);

		bool bHandledNetUpdate = false;
		if (bIsSimulatedProxy)
		{

			if (bNetworkUpdateReceived)
			{
				bNetworkUpdateReceived = false;
				bHandledNetUpdate = true;
				UE_LOGF(LogVRCharacterMovement, Verbose, "Proxy %ls received net update", *CharacterOwner->GetName());
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
				else if (bJustTeleported || bForceNextFloorCheck)
				{

					bJustTeleported = false;
					UpdateFloorFromAdjustment();
				}
			}
			else if (bForceNextFloorCheck)
			{
				UpdateFloorFromAdjustment();
			}
		}

		UpdateCharacterStateBeforeMovement(DeltaSeconds);

		if (MovementMode != MOVE_None)
		{

			HandlePendingLaunch();
		}
		ClearAccumulatedForces();

		if (MovementMode == MOVE_None)
		{
			return;
		}

		const bool bSimGravityDisabled = (bIsSimulatedProxy && CharacterOwner->bSimGravityDisabled);
		const bool bZeroReplicatedGroundVelocity = (bIsSimulatedProxy && IsMovingOnGround() && ConstRepMovement.LinearVelocity.IsZero());

		if (bSimGravityDisabled || bZeroReplicatedGroundVelocity)
		{
			Velocity = FVector::ZeroVector;
		}

		MaybeUpdateBasedMovement(DeltaSeconds);

		OldVelocity = Velocity;
		OldLocation = UpdatedComponent->GetComponentLocation();

		UpdateProxyAcceleration();

		static const auto CVarNetEnableSkipProxyPredictionOnNetUpdate = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetEnableSkipProxyPredictionOnNetUpdate"));

		if (!bHandledNetUpdate || !bNetworkSkipProxyPredictionOnNetUpdate || !CVarNetEnableSkipProxyPredictionOnNetUpdate->GetInt())
		{
			UE_LOGF(LogVRCharacterMovement, Verbose, "Proxy %ls simulating movement", *GetNameSafe(CharacterOwner));
			FStepDownResult StepDownResult;

			if(!bDisableSimulatedTickWhenSmoothingMovement)
			{ 
				MoveSmooth(Velocity, DeltaSeconds, &StepDownResult);
			}

			if (IsMovingOnGround() || MovementMode == MOVE_Falling)
			{
				const bool bShouldFindFloor = GetGravitySpaceZ(Velocity) <= UE_KINDA_SMALL_NUMBER;

				if (StepDownResult.bComputedFloor)
				{
					CurrentFloor = StepDownResult.FloorResult;
				}
				else if (bDisableSimulatedTickWhenSmoothingMovement || bShouldFindFloor)
				{
					FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, Velocity.IsZero(), NULL);
				}
				else
				{
					CurrentFloor.Clear();
				}

				if (CurrentFloor.HitResult.bStartPenetrating && MovementBaseUtility::IsDynamicBase(GetMovementBaseInterfaceData()))
				{

					FHitResult Hit(CurrentFloor.HitResult);
					Hit.TraceEnd = Hit.TraceStart - GetGravityDirection() * MAX_FLOOR_DIST;

					const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
					const bool bResolved = ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
					bForceNextFloorCheck |= bResolved;

					if (bAutoOrientToFloorNormal && CurrentFloor.IsWalkableFloor())
					{

						AutoTraceAndSetCharacterToNewGravity(CurrentFloor.HitResult, DeltaSeconds);
					}
				}
				else if (!CurrentFloor.IsWalkableFloor())
				{
					if (!bSimGravityDisabled)
					{

						if (GetGravitySpaceZ(Velocity) <= UE_KINDA_SMALL_NUMBER || bApplyGravityWhileJumping || !CharacterOwner->IsJumpProvidingForce())
						{
							Velocity = NewFallVelocity(Velocity, -GetGravityDirection() * GetGravityZ(), DeltaSeconds);
						}
					}

					SetMovementMode(MOVE_Falling);
				}
				else
				{

					if (IsMovingOnGround())
					{
						AdjustFloorHeight();
						SetBaseFromFloor(CurrentFloor);
					}
					else if (MovementMode == MOVE_Falling)
					{
						if (CurrentFloor.FloorDist <= MIN_FLOOR_DIST || (bSimGravityDisabled && CurrentFloor.FloorDist <= MAX_FLOOR_DIST))
						{

							SetPostLandedPhysics(CurrentFloor.HitResult);
						}
						else
						{
							if (!bSimGravityDisabled)
							{

								Velocity = NewFallVelocity(Velocity, -GetGravityDirection() * GetGravityZ(), DeltaSeconds);
							}
							CurrentFloor.Clear();
						}
					}

					if (bAutoOrientToFloorNormal && CurrentFloor.IsWalkableFloor())
					{

						AutoTraceAndSetCharacterToNewGravity(CurrentFloor.HitResult, DeltaSeconds);
					}
				}
			}
		}
		else
		{
			UE_LOGF(LogVRCharacterMovement, Verbose, "Proxy %ls SKIPPING simulate movement", *GetNameSafe(CharacterOwner));
		}

		UpdateCharacterStateAfterMovement(DeltaSeconds);

		LastUpdateRequestedVelocity = bHasRequestedVelocity ? RequestedVelocity : FVector::ZeroVector;
		bHasRequestedVelocity = false;

		OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	} 

	CallMovementUpdateDelegate(DeltaSeconds, OldLocation, OldVelocity);

	static const auto CVarBasedMovementMode = IConsoleManager::Get().FindConsoleVariable(TEXT("p.BasedMovementMode"));
	if (CVarBasedMovementMode->GetInt() == 0)
	{
		SaveBaseLocation(); 
	}
	else
	{
		MaybeSaveBaseLocation();
	}

	UpdateComponentVelocity();
	bJustTeleported = false;

	LastUpdateLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
	LastUpdateRotation = UpdatedComponent ? UpdatedComponent->GetComponentQuat() : FQuat::Identity;
	LastUpdateVelocity = Velocity;
}

void UVRCharacterMovementComponent::MoveSmooth(const FVector& InVelocity, const float DeltaSeconds, FStepDownResult* OutStepDownResult)
{
	if (!HasValidData())
	{
		return;
	}

	if (MovementMode == MOVE_Custom)
	{
		FVRCharacterScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);
		PhysCustom(DeltaSeconds, 0);
		return;
	}

	FVector Delta = InVelocity * DeltaSeconds;
	if (Delta.IsZero())
	{
		return;
	}

	FVRCharacterScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);

	if (IsMovingOnGround())
	{
		MoveAlongFloor(InVelocity, DeltaSeconds, OutStepDownResult);
	}
	else
	{
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			bool bSteppedUp = false;

			if (IsFlying())
			{
				if (CanStepUp(Hit))
				{
					OutStepDownResult = NULL; 
					bool bShouldAttemptStepUp = false;
					bShouldAttemptStepUp = FMath::Abs(GetGravitySpaceZ(Hit.ImpactNormal)) < 0.2;
					if (bShouldAttemptStepUp)
					{
						const FVector GravDir = GetGravityDirection();
						const FVector DesiredDir = Delta.GetSafeNormal();
						const float UpDown = GravDir | DesiredDir;
						if ((UpDown < 0.5f) && (UpDown > -0.2f))
						{
							bSteppedUp = StepUp(GravDir, Delta * (1.f - Hit.Time), Hit, OutStepDownResult);
						}
					}
				}
			}

			if (!bSteppedUp)
			{
				SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, false);
			}
		}
	}
}

void UVRCharacterMovementComponent::ClientHandleMoveResponse(const FCharacterMoveResponseDataContainer& MoveResponse)
{
	if (MoveResponse.IsGoodMove())
	{
		ClientAckGoodMove_Implementation(MoveResponse.ClientAdjustment.TimeStamp);
	}
	else
	{
		FMovementBaseInterfaceData MovementBaseInterfaceDataFromClientAdjustment = MovementBaseUtility::GetMovementBaseDataFromPhysicsOwner(MoveResponse.ClientAdjustment.NewMovementBasePhysicsObjectOwner);

		if (MoveResponse.bRootMotionSourceCorrection)
		{
			if (FRootMotionSourceGroup* RootMotionSourceGroup = MoveResponse.GetRootMotionSourceGroup(*this))
			{
				ClientAdjustRootMotionSourcePosition_Implementation(
					MoveResponse.ClientAdjustment.TimeStamp,
					*RootMotionSourceGroup,
					MoveResponse.bRootMotionMontageCorrection,
					MoveResponse.RootMotionTrackPosition,
					MoveResponse.ClientAdjustment.NewLoc,
					MoveResponse.RootMotionRotation,
					GetGravitySpaceZ(MoveResponse.ClientAdjustment.NewVel),
					&MovementBaseInterfaceDataFromClientAdjustment,
					MoveResponse.ClientAdjustment.NewBaseBoneName,
					MoveResponse.bHasBase,
					MoveResponse.ClientAdjustment.bBaseRelativePosition,
					MoveResponse.ClientAdjustment.MovementMode);
			}
		}
		else if (MoveResponse.bRootMotionMontageCorrection)
		{
			ClientAdjustRootMotionPosition_Implementation(
				MoveResponse.ClientAdjustment.TimeStamp,
				MoveResponse.RootMotionTrackPosition,
				MoveResponse.ClientAdjustment.NewLoc,
				MoveResponse.RootMotionRotation,
				GetGravitySpaceZ(MoveResponse.ClientAdjustment.NewVel),
				&MovementBaseInterfaceDataFromClientAdjustment,
				MoveResponse.ClientAdjustment.NewBaseBoneName,
				MoveResponse.bHasBase,
				MoveResponse.ClientAdjustment.bBaseRelativePosition,
				MoveResponse.ClientAdjustment.MovementMode);
		}
		else
		{
			ClientAdjustPositionVR_Implementation(
				MoveResponse.ClientAdjustment.TimeStamp,
				MoveResponse.ClientAdjustment.NewLoc,

				MoveResponse.ClientAdjustment.NewVel,
				&MovementBaseInterfaceDataFromClientAdjustment,
				MoveResponse.ClientAdjustment.NewBaseBoneName,
				MoveResponse.bHasBase,
				MoveResponse.ClientAdjustment.bBaseRelativePosition,
				MoveResponse.ClientAdjustment.MovementMode,
				MoveResponse.bHasRotation ? MoveResponse.ClientAdjustment.NewRot : TOptional<FRotator>(),
				MoveResponse.ClientAdjustment.GravityDirection
			);

		}
	}
}

void UVRCharacterMovementComponent::ClientAdjustPositionVR_Implementation
(
	float TimeStamp,
	FVector NewLocation,

	FVector NewVelocity,
	UPrimitiveComponent* NewBase,
	FName NewBaseBoneName,
	bool bHasBase,
	bool bBaseRelativePosition,
	uint8 ServerMovementMode,
	TOptional<FRotator> OptionalRotation,
	TOptional<FVector> OptionalGravityDirection
) 
{
	FMovementBaseInterfaceData NewMovementBaseInterfaceData(NewBase);
	ClientAdjustPositionVR_Implementation(TimeStamp, NewLocation, NewVelocity, &NewMovementBaseInterfaceData, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode, OptionalRotation);
}

void UVRCharacterMovementComponent::ClientAdjustPositionVR_Implementation
(
	float TimeStamp,
	FVector NewLocation,

	FVector NewVelocity,
	FMovementBaseInterfaceData* NewMovementBaseInterfaceData,
	FName NewBaseBoneName,
	bool bHasBase,
	bool bBaseRelativePosition,
	uint8 ServerMovementMode,
	TOptional<FRotator> OptionalRotation,
	TOptional<FVector> OptionalGravityDirection
)
{
	if (!HasValidData() || !IsActive())
	{
		return;
	}

	if (bClientIgnoreMovementCorrections)
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			const int32 MoveIndex = ClientData->GetSavedMoveIndex(TimeStamp);
			if (MoveIndex != INDEX_NONE)
			{
				ClientData->AckMove(MoveIndex, *this);
			}
		}

		UE_LOGF(LogNetPlayerMovement, Verbose, "ClientAdjustPosition_Implementation: Ignoring server correction. bClientIgnoreMovementCorrections is set. TimeStamp: %f", TimeStamp);

		return;
	}

	FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
	check(ClientData);

	const bool bUnresolvedBase = bHasBase && !MovementBaseUtility::IsMovementBaseDataValid(NewMovementBaseInterfaceData);
	if (bUnresolvedBase)
	{
		if (bBaseRelativePosition)
		{
			UE_LOGF(LogNetPlayerMovement, Warning, "ClientAdjustPosition_Implementation could not resolve the new relative movement base actor, ignoring server correction! Client currently at world location %ls on base %ls",
				*UpdatedComponent->GetComponentLocation().ToString(), *GetNameSafe(NewMovementBaseInterfaceData ? NewMovementBaseInterfaceData->GetMovementBaseObject() : nullptr));
			return;
		}
		else
		{
			UE_LOGF(LogNetPlayerMovement, Verbose, "ClientAdjustPosition_Implementation could not resolve the new absolute movement base actor, but WILL use the position!");
		}
	}

	int32 MoveIndex = ClientData->GetSavedMoveIndex(TimeStamp);
	if (MoveIndex == INDEX_NONE)
	{
		if (ClientData->LastAckedMove.IsValid())
		{
			UE_LOGF(LogNetPlayerMovement, Log, "ClientAdjustPosition_Implementation could not find Move for TimeStamp: %f, LastAckedTimeStamp: %f, CurrentTimeStamp: %f", TimeStamp, ClientData->LastAckedMove->TimeStamp, ClientData->CurrentTimeStamp);
		}
		return;
	}

	ClientData->AckMove(MoveIndex, *this);

	FVector WorldShiftedNewLocation;

	if (bBaseRelativePosition)
	{
		MovementBaseUtility::TransformLocationToWorld(NewMovementBaseInterfaceData, NewBaseBoneName, NewLocation, WorldShiftedNewLocation); 

	}
	else
	{
		WorldShiftedNewLocation = FRepMovement::RebaseOntoLocalOrigin(NewLocation, this);
	}

	const FCharacterMoveResponseDataContainer& ResponseDataContainer = GetMoveResponseDataContainer();
	if (ResponseDataContainer.ClientAdjustment.bBaseRelativeVelocity)
	{

		const FVector CurrentVelocity = NewVelocity;
		MovementBaseUtility::TransformDirectionToWorld(NewMovementBaseInterfaceData, NewBaseBoneName, CurrentVelocity, NewVelocity);
	}

	FVector GravityCorrection = OptionalGravityDirection.IsSet() ? OptionalGravityDirection.GetValue() : DefaultGravityDirection;
	OnClientCorrectionReceived(*ClientData, TimeStamp, WorldShiftedNewLocation, NewVelocity, NewMovementBaseInterfaceData, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode, GravityCorrection);

	if (UpdatedComponent)
	{

		SetCharacterToNewGravity(GravityCorrection, bAutoOrientToFloorNormal);

		if (bRunClientCorrectionToHMD && BaseVRCharacterOwner)
		{
			if (OptionalRotation.IsSet())
			{
				BaseVRCharacterOwner->SetActorLocationAndRotationVR(WorldShiftedNewLocation, OptionalRotation.GetValue(), false, false, true, true);

				if (ClientData->LastAckedMove.IsValid() && !ClientData->LastAckedMove->SavedControlRotation.Equals(OptionalRotation.GetValue()))
				{
					if (BaseVRCharacterOwner)
					{
						if (BaseVRCharacterOwner->bUseControllerRotationYaw)
						{
							AController* myController = BaseVRCharacterOwner->GetController();
							if (myController)
							{

								myController->SetControlRotation(OptionalRotation.GetValue());
							}
						}
					}
				}

			}
			else
			{
				BaseVRCharacterOwner->SetActorLocationVR(WorldShiftedNewLocation, true);
			}
		}
		else
		{
			if (OptionalRotation.IsSet())
			{
				UpdatedComponent->SetWorldLocationAndRotation(WorldShiftedNewLocation, OptionalRotation.GetValue(), false, nullptr, ETeleportType::TeleportPhysics);

				if (ClientData->LastAckedMove.IsValid() && !ClientData->LastAckedMove->SavedControlRotation.Equals(OptionalRotation.GetValue()))
				{
					if (BaseVRCharacterOwner)
					{
						if (BaseVRCharacterOwner->bUseControllerRotationYaw)
						{
							AController* myController = BaseVRCharacterOwner->GetController();
							if (myController)
							{

								myController->SetControlRotation(OptionalRotation.GetValue());
							}
						}
					}
				}
			}
			else
			{
				UpdatedComponent->SetWorldLocation(WorldShiftedNewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}

	Velocity = NewVelocity;

	ApplyNetworkMovementMode(ServerMovementMode);

	FMovementBaseInterfaceData FinalMovementBaseInterfaceData = NewMovementBaseInterfaceData;
	FName FinalBaseBoneName = NewBaseBoneName;
	if (bUnresolvedBase)
	{
		check(!FinalMovementBaseInterfaceData.IsValid());
		check(!bBaseRelativePosition);

		if (MovementBaseUtility::IsMovementBaseDataValid(GetMovementBaseInterfaceData()) && UpdatedComponent)
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, false);
			if (CurrentFloor.IsWalkableFloor())
			{
				FinalMovementBaseInterfaceData = MovementBaseUtility::GetMovementBaseDataFromHitResult(&CurrentFloor.HitResult);
				FinalBaseBoneName = CurrentFloor.HitResult.BoneName;
			}
			else
			{
				FinalMovementBaseInterfaceData.Clear();
				FinalBaseBoneName = NAME_None;
			}

		}
	}
	SetBase(&FinalMovementBaseInterfaceData, FinalBaseBoneName);

	UpdateFloorFromAdjustment();
	bJustTeleported = true;

	SaveBaseLocation();

	LastUpdateLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
	LastUpdateRotation = UpdatedComponent ? UpdatedComponent->GetComponentQuat() : FQuat::Identity;
	LastUpdateVelocity = Velocity;

	UpdateComponentVelocity();
	ClientData->bUpdatePosition = true;
}

bool UVRCharacterMovementComponent::ServerCheckClientErrorVR(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, FRotator ClientRot, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	FMovementBaseInterfaceData ClientMovementBaseInterfaceData(ClientMovementBase);
	return ServerCheckClientErrorVR(ClientTimeStamp, DeltaTime, Accel, ClientWorldLocation, ClientRot, RelativeClientLocation, &ClientMovementBaseInterfaceData, ClientBaseBoneName, ClientMovementMode);
}

bool UVRCharacterMovementComponent::ServerCheckClientErrorVR(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, FRotator ClientRot, const FVector& RelativeClientLocation, FMovementBaseInterfaceData * ClientMovementBaseInterfaceData, FName ClientBaseBoneName, uint8 ClientMovementMode)
{

	if (!bIgnoreClientMovementErrorChecksAndCorrection)
	{

		if (!bUseClientControlRotation)
		{

			if (GetGravityDirection().Equals(DefaultGravityDirection))
			{
				if (!FMath::IsNearlyEqual(FRotator::ClampAxis(ClientRot.Yaw), FRotator::ClampAxis(UpdatedComponent->GetComponentRotation().Yaw), CharacterMovementComponentStatics::fRotationCorrectionThreshold))
				{
					return true;
				}
			}
			else
			{
				float CorrectionValue = bIsBlendingOrientation ? CharacterMovementComponentStatics::fRotationChangingCorrectionThreshold : CharacterMovementComponentStatics::fRotationCorrectionThreshold;
				bIsBlendingOrientation = false;

				if (FQuat::ErrorAutoNormalize(UpdatedComponent->GetComponentQuat(), ClientRot.Quaternion()) > CorrectionValue)
				{
					return true;
				}
			}
		}

#if ROOT_MOTION_DEBUG
		if (RootMotionSourceDebug::CVarDebugRootMotionSources.GetValueOnAnyThread() == 1)
		{
			const FVector LocDiff = UpdatedComponent->GetComponentLocation() - ClientWorldLocation;
			FString AdjustedDebugString = FString::Printf(TEXT("ServerCheckClientError LocDiff(%.1f) ExceedsAllowablePositionError(%d) TimeStamp(%f)"),
				LocDiff.Size(), GetDefault<AGameNetworkManager>()->ExceedsAllowablePositionError(LocDiff), ClientTimeStamp);
			RootMotionSourceDebug::PrintOnScreen(*CharacterOwner, AdjustedDebugString);
		}
#endif

		if (ServerExceedsAllowablePositionError(ClientTimeStamp, DeltaTime, Accel, ClientWorldLocation, RelativeClientLocation, ClientMovementBaseInterfaceData, ClientBaseBoneName, ClientMovementMode))
		{
			return true;
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		static const auto CVarNetForceClientAdjustmentPercent = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetForceClientAdjustmentPercent"));
		if (CVarNetForceClientAdjustmentPercent->GetFloat() > UE_SMALL_NUMBER)
		{
			if (RandomStream.FRand() < CVarNetForceClientAdjustmentPercent->GetFloat())
			{
				UE_LOGF(LogVRCharacterMovement, VeryVerbose, "** ServerCheckClientError forced by p.NetForceClientAdjustmentPercent");
				return true;
			}
		}
#endif
	}
	else
	{
#if !UE_BUILD_SHIPPING
		static const auto CVarNetShowCorrections = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetShowCorrections"));
		if (CVarNetShowCorrections->GetInt() != 0)
		{
			UE_LOGF(LogVRCharacterMovement, Warning, "*** Server: %ls is set to ignore error checks and corrections.", *GetNameSafe(CharacterOwner));
		}
#endif 
	}

	return false;
}

void UVRCharacterMovementComponent::ServerMoveHandleClientErrorVR(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& RelativeClientLoc, FRotator ClientRot, FMovementBaseInterfaceData* ClientMovementBaseInterfaceData, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	if (!ShouldUsePackedMovementRPCs())
	{
		if (RelativeClientLoc == FVector(1.f, 2.f, 3.f)) 
		{
			return;
		}
	}

	FNetworkPredictionData_Server_VRCharacter* ServerData = ((FNetworkPredictionData_Server_VRCharacter*)GetPredictionData_Server_Character());
	check(ServerData);

	APlayerController* PC = Cast<APlayerController>(CharacterOwner->GetController());
	if ((ServerData->LastUpdateTime != GetWorld()->TimeSeconds))
	{
		const AGameNetworkManager* GameNetworkManager = (const AGameNetworkManager*)(AGameNetworkManager::StaticClass()->GetDefaultObject());
		if (GameNetworkManager->WithinUpdateDelayBounds(PC, ServerData->LastUpdateTime))
		{
			return;
		}
	}

	FVector ClientLoc = RelativeClientLoc;
	if (MovementBaseUtility::UseRelativeLocation(ClientMovementBaseInterfaceData))
	{
		MovementBaseUtility::TransformLocationToWorld(ClientMovementBaseInterfaceData, ClientBaseBoneName, RelativeClientLoc, ClientLoc);
	}
	else
	{
		ClientLoc = FRepMovement::RebaseOntoLocalOrigin(ClientLoc, this);
	}

	FVector ServerLoc = UpdatedComponent->GetComponentLocation();

	if (!MovementBaseUtility::IsMovementBaseDataValid(ClientMovementBaseInterfaceData))
	{
		TEnumAsByte<EMovementMode> NetMovementMode(MOVE_None);
		TEnumAsByte<EMovementMode> NetGroundMode(MOVE_None);
		uint8 NetCustomMode(0);
		UnpackNetworkMovementMode(ClientMovementMode, NetMovementMode, NetCustomMode, NetGroundMode);
		if (NetMovementMode == MOVE_Walking)
		{
			ClientMovementBaseInterfaceData = GetMovementBaseInterfaceData_Mutable();
			ClientBaseBoneName = CharacterOwner->GetBasedMovement().BoneName;
		}
	}

	FName MovementBaseBoneName = CharacterOwner->GetBasedMovement().BoneName;
	const FMovementBaseInterfaceData* MovementBaseInterfaceData = GetMovementBaseInterfaceData();
	const bool bServerIsFalling = IsFalling();
	const bool bClientIsFalling = ClientMovementMode == MOVE_Falling;
	const bool bServerJustLanded = bLastServerIsFalling && !bServerIsFalling;
	const bool bClientJustLanded = bLastClientIsFalling && !bClientIsFalling;

	FVector RelativeLocation = ServerLoc;
	FVector RelativeVelocity = Velocity;
	bool bUseLastBase = false;
	bool bFallingWithinAcceptableError = false;

	static const auto CVarClientAuthorityThresholdOnBaseChange = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ClientAuthorityThresholdOnBaseChange"));
	static const auto CVarMaxFallingCorrectionLeash = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxFallingCorrectionLeash"));

	const float ClientAuthorityThreshold = CVarClientAuthorityThresholdOnBaseChange->GetFloat();
	const float MaxFallingCorrectionLeash = CVarMaxFallingCorrectionLeash->GetFloat();
	const bool bDeferServerCorrectionsWhenFalling = ClientAuthorityThreshold > 0.f || MaxFallingCorrectionLeash > 0.f;
	if (bDeferServerCorrectionsWhenFalling)
	{

		if (bTeleportedSinceLastUpdate || (MovementMode != MOVE_Walking && MovementMode != MOVE_Falling))
		{
			MaxServerClientErrorWhileFalling = 0.f;
			bCanTrustClientOnLanding = false;
		}

		float MaxLandingCorrection = 0.f;
		if (ClientAuthorityThreshold > 0.f && MaxFallingCorrectionLeash > 0.f)
		{
			MaxLandingCorrection = FMath::Min(ClientAuthorityThreshold, MaxServerClientErrorWhileFalling);
		}
		else
		{
			MaxLandingCorrection = FMath::Max(ClientAuthorityThreshold, MaxServerClientErrorWhileFalling);
		}

		if (bCanTrustClientOnLanding && MaxLandingCorrection > 0.f && (bClientJustLanded || bServerJustLanded))
		{

			const FVector LocDiff = ServerLoc - ClientLoc;

			if (!LocDiff.IsNearlyZero(UE_KINDA_SMALL_NUMBER))
			{
				if (LocDiff.SizeSquared() < FMath::Square(MaxLandingCorrection))
				{
					ServerLoc = ClientLoc;
					UpdatedComponent->MoveComponent(ServerLoc - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
					bJustTeleported = true;
				}
				else
				{
					const FVector ClampedDiff = LocDiff.GetSafeNormal() * MaxLandingCorrection;
					ServerLoc -= ClampedDiff;
					UpdatedComponent->MoveComponent(ServerLoc - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
					bJustTeleported = true;
				}
			}

			MaxServerClientErrorWhileFalling = 0.f;
			bCanTrustClientOnLanding = false;
		}

		if (bServerIsFalling && bLastServerIsWalking && !bTeleportedSinceLastUpdate)
		{
			float ClientForwardFactor = 1.f;
			if (LastMovementBaseInterfaceDataVR.IsValid() && MovementBaseUtility::IsDynamicBase(&LastMovementBaseInterfaceDataVR) && MaxWalkSpeed > UE_KINDA_SMALL_NUMBER)
			{
				const FVector LastBaseVelocity = MovementBaseUtility::GetMovementBaseVelocity(&LastMovementBaseInterfaceDataVR, LastServerMovementBaseBoneName);
				RelativeVelocity = Velocity - LastBaseVelocity;
				const FVector BaseDirection = ProjectToGravityFloor(LastBaseVelocity).GetSafeNormal();
				const FVector RelativeDirection = RelativeVelocity * (1.f / MaxWalkSpeed);

				ClientForwardFactor = FMath::Clamp(FVector::DotProduct(BaseDirection, RelativeDirection), 0.f, 1.f);

				if (MovementBaseUtility::UseRelativeLocation(&LastMovementBaseInterfaceDataVR))
				{

					MovementBaseUtility::TransformLocationToLocal(&LastMovementBaseInterfaceDataVR, LastServerMovementBaseBoneName, UpdatedComponent->GetComponentLocation(), RelativeLocation);
					bUseLastBase = true;
				}
			}

			if (ClientAuthorityThreshold > 0.f && ClientForwardFactor < 1.f)
			{
				const float AdjustedClientAuthorityThreshold = ClientAuthorityThreshold * (1.f - ClientForwardFactor);
				const FVector LocDiff = ServerLoc - ClientLoc;

				if (!LocDiff.IsNearlyZero(UE_KINDA_SMALL_NUMBER))
				{
					if (LocDiff.SizeSquared() < FMath::Square(AdjustedClientAuthorityThreshold))
					{
						ServerLoc = ClientLoc;
						UpdatedComponent->MoveComponent(ServerLoc - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						bJustTeleported = true;
					}
					else
					{
						const FVector ClampedDiff = LocDiff.GetSafeNormal() * AdjustedClientAuthorityThreshold;
						ServerLoc -= ClampedDiff;
						UpdatedComponent->MoveComponent(ServerLoc - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						bJustTeleported = true;
					}
				}
			}

			if (ClientForwardFactor < 1.f)
			{
				MaxServerClientErrorWhileFalling = FMath::Min((ServerLoc - ClientLoc).Size() * (1.f - ClientForwardFactor), MaxFallingCorrectionLeash);
				bCanTrustClientOnLanding = true;
			}
			else
			{
				MaxServerClientErrorWhileFalling = 0.f;
				bCanTrustClientOnLanding = false;
			}
		}
		else if (!bServerIsFalling && bCanTrustClientOnLanding)
		{
			MaxServerClientErrorWhileFalling = 0.f;
			bCanTrustClientOnLanding = false;
		}

		if (MaxServerClientErrorWhileFalling > 0.f && (bServerIsFalling || bClientIsFalling))
		{
			const FVector LocDiff = ServerLoc - ClientLoc;
			if (LocDiff.SizeSquared() <= FMath::Square(MaxServerClientErrorWhileFalling))
			{
				ServerLoc = ClientLoc;

				bFallingWithinAcceptableError = true;
			}
			else
			{

				static const auto CVarMaxFallingCorrectionLeashBuffer = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxFallingCorrectionLeashBuffer"));
				ServerLoc = ServerLoc - LocDiff.GetSafeNormal() * FMath::Clamp(MaxServerClientErrorWhileFalling - CVarMaxFallingCorrectionLeashBuffer->GetFloat(), 0.f, MaxServerClientErrorWhileFalling);
			}
		}
	}

	bool bInClientAuthoritativeMovementMode = false;

	TEnumAsByte<EMovementMode> NetMovementMode(MOVE_None);
	TEnumAsByte<EMovementMode> NetGroundMode(MOVE_None);
	uint8 NetCustomMode(0);
	UnpackNetworkMovementMode(ClientMovementMode, NetMovementMode, NetCustomMode, NetGroundMode);
	if (NetMovementMode == EMovementMode::MOVE_Custom)
	{
		if (NetCustomMode == (uint8)EVRCustomMovementMode::VRMOVE_Climbing)
			bInClientAuthoritativeMovementMode = true;
	}

	ServerData = ((FNetworkPredictionData_Server_VRCharacter*)GetPredictionData_Server_Character());
	check(ServerData);

	bNetworkLargeClientCorrection = ServerData->bForceClientUpdate;
	if (ServerData->bForceClientUpdate || (!bFallingWithinAcceptableError && ServerCheckClientErrorVR(ClientTimeStamp, DeltaTime, Accel, ClientLoc, ClientRot, RelativeClientLoc, ClientMovementBaseInterfaceData, ClientBaseBoneName, ClientMovementMode)))
	{

		ServerData->PendingAdjustment.NewVel = Velocity;
		ServerData->PendingAdjustment.NewMovementBasePhysicsObjectOwner = MovementBaseInterfaceData ? MovementBaseInterfaceData->PhysicsObjectOwner.Get() : nullptr;
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
			ServerData->PendingAdjustment.NewBase = CharacterOwner->GetBasedMovement().MovementBase;
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
		ServerData->PendingAdjustment.NewBaseBoneName = MovementBaseBoneName;
		ServerData->PendingAdjustment.NewRot = UpdatedComponent->GetComponentRotation();
		ServerData->PendingAdjustment.GravityDirection = GetGravityDirection();

		if (bRunClientCorrectionToHMD && IsValid(BaseVRCharacterOwner))
		{
			FVector CapsuleLoc = BaseVRCharacterOwner->GetVRLocation();
			CapsuleLoc.Z = ServerLoc.Z;
			ServerData->PendingAdjustment.NewLoc = FRepMovement::RebaseOntoZeroOrigin(CapsuleLoc, this);

		}
		else
		{
			ServerData->PendingAdjustment.NewLoc = FRepMovement::RebaseOntoZeroOrigin(ServerLoc, this);

		}

		ServerData->PendingAdjustment.bBaseRelativePosition = (bDeferServerCorrectionsWhenFalling && bUseLastBase) || MovementBaseUtility::UseRelativeLocation(MovementBaseInterfaceData);
		ServerData->PendingAdjustment.bBaseRelativeVelocity = false;

		if (ServerData->PendingAdjustment.bBaseRelativePosition)
		{
			if (bDeferServerCorrectionsWhenFalling && bUseLastBase)
			{
				ServerData->PendingAdjustment.NewVel = RelativeVelocity;
				ServerData->PendingAdjustment.NewMovementBasePhysicsObjectOwner = LastMovementBaseInterfaceDataVR.PhysicsObjectOwner.Get();
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
					ServerData->PendingAdjustment.NewBase = LastServerMovementBaseVR.Get();
				PRAGMA_ENABLE_DEPRECATION_WARNINGS
				ServerData->PendingAdjustment.NewBaseBoneName = LastServerMovementBaseBoneName;

				if (bRunClientCorrectionToHMD && IsValid(BaseVRCharacterOwner))
				{	
					FBasedMovementInfo BaseInfo = CharacterOwner->GetBasedMovement();
					FVector BaseLocation;
					FQuat BaseQuat;

					const bool bResult = MovementBaseUtility::GetMovementBaseTransform(MovementBaseInterfaceData, BaseInfo.BoneName, BaseLocation, BaseQuat);
					if (bResult)
					{
						ServerData->PendingAdjustment.NewLoc = FTransform(BaseQuat, BaseLocation).InverseTransformPositionNoScale(BaseInfo.Location);
					}
					else
					{
						ServerData->PendingAdjustment.NewLoc = RelativeLocation;
					}
				}
				else
				{
					ServerData->PendingAdjustment.NewLoc = RelativeLocation;
				}
			}
			else
			{
				if (bRunClientCorrectionToHMD && IsValid(BaseVRCharacterOwner))
				{
					FBasedMovementInfo BaseInfo = CharacterOwner->GetBasedMovement();
					FVector BaseLocation;
					FQuat BaseQuat;
					const bool bResult = MovementBaseUtility::GetMovementBaseTransform(MovementBaseInterfaceData, BaseInfo.BoneName, BaseLocation, BaseQuat);

					if (bResult)
					{
						ServerData->PendingAdjustment.NewLoc = FTransform(BaseQuat, BaseLocation).InverseTransformPosition(ServerData->PendingAdjustment.NewLoc);
					}
					else
					{
						ServerData->PendingAdjustment.NewLoc = CharacterOwner->GetBasedMovement().Location;
					}
				}
				else
				{
					ServerData->PendingAdjustment.NewLoc = CharacterOwner->GetBasedMovement().Location;
				}

				static const auto CVarNetUseBaseRelativeVelocity = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetUseBaseRelativeVelocity"));
				if (CVarNetUseBaseRelativeVelocity->GetInt())
				{

					ServerData->PendingAdjustment.bBaseRelativeVelocity = true;
					const FVector CurrentVelocity = ServerData->PendingAdjustment.NewVel;
					MovementBaseUtility::TransformDirectionToLocal(MovementBaseInterfaceData, MovementBaseBoneName, CurrentVelocity, ServerData->PendingAdjustment.NewVel);
				}
			}

		}

#if !UE_BUILD_SHIPPING
		static const auto CVarNetShowCorrections = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetShowCorrections"));
		static const auto CVarNetCorrectionLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetCorrectionLifetime"));
		if (CVarNetShowCorrections->GetInt() != 0)
		{
			const FVector LocDiff = UpdatedComponent->GetComponentLocation() - ClientLoc;
			const UObject* MovementBaseObject = MovementBaseInterfaceData ? MovementBaseInterfaceData->GetMovementBaseObject() : nullptr;
			const FString BaseString = MovementBaseObject ? MovementBaseObject->GetPathName(MovementBaseObject->GetOutermost()) : TEXT("None");
			UE_LOGF(LogNetPlayerMovement, Warning, "*** Server: Error for %ls at Time=%.3f is %3.3f LocDiff(%ls) ClientLoc(%ls) ServerLoc(%ls) Base: %ls Bone: %ls Accel(%ls) Velocity(%ls)",
				*GetNameSafe(CharacterOwner), ClientTimeStamp, LocDiff.Size(), *LocDiff.ToString(), *ClientLoc.ToString(), *UpdatedComponent->GetComponentLocation().ToString(), *BaseString, *ServerData->PendingAdjustment.NewBaseBoneName.ToString(), *Accel.ToString(), *Velocity.ToString());
			const float DebugLifetime = CVarNetCorrectionLifetime->GetFloat();
			DrawDebugCapsule(GetWorld(), UpdatedComponent->GetComponentLocation(), CharacterOwner->GetSimpleCollisionHalfHeight(), CharacterOwner->GetSimpleCollisionRadius(), FQuat::Identity, FColor(100, 255, 100), false, DebugLifetime);
			DrawDebugCapsule(GetWorld(), ClientLoc, CharacterOwner->GetSimpleCollisionHalfHeight(), CharacterOwner->GetSimpleCollisionRadius(), FQuat::Identity, FColor(255, 100, 100), false, DebugLifetime);
		}
#endif

		ServerData->LastUpdateTime = GetWorld()->TimeSeconds;
		ServerData->PendingAdjustment.DeltaTime = DeltaTime;
		ServerData->PendingAdjustment.TimeStamp = ClientTimeStamp;
		ServerData->PendingAdjustment.bAckGoodMove = false;
		ServerData->PendingAdjustment.MovementMode = PackNetworkMovementMode();

	}
	else
	{
		if (bInClientAuthoritativeMovementMode || ServerShouldUseAuthoritativePosition(ClientTimeStamp, DeltaTime, Accel, ClientLoc, RelativeClientLoc, ClientMovementBaseInterfaceData, ClientBaseBoneName, ClientMovementMode))
		{
			const FVector LocDiff = UpdatedComponent->GetComponentLocation() - ClientLoc; 
			if (!LocDiff.IsZero() || ClientMovementMode != PackNetworkMovementMode() || GetMovementBaseInterfaceData() != ClientMovementBaseInterfaceData || (CharacterOwner && CharacterOwner->GetBasedMovement().BoneName != ClientBaseBoneName))
			{

				UpdatedComponent->SetWorldLocation(ClientLoc, false);

				ApplyNetworkMovementMode(ClientMovementMode);

				SetBase(ClientMovementBaseInterfaceData, ClientBaseBoneName);
				UpdateFloorFromAdjustment();

				SaveBaseLocation();

				LastUpdateLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
				LastUpdateRotation = UpdatedComponent ? UpdatedComponent->GetComponentQuat() : FQuat::Identity;
				LastUpdateVelocity = Velocity;
			}
		}

		ServerData->PendingAdjustment.TimeStamp = ClientTimeStamp;
		ServerData->PendingAdjustment.bAckGoodMove = true;
	}

	ServerData->bForceClientUpdate = false;

	LastMovementBaseInterfaceDataVR = MovementBaseInterfaceData;
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
		LastServerMovementBaseVR = CharacterOwner->GetBasedMovement().MovementBase;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	LastServerMovementBaseBoneName = MovementBaseBoneName;
	bLastClientIsFalling = bClientIsFalling;
	bLastServerIsFalling = bServerIsFalling;
	bLastServerIsWalking = MovementMode == MOVE_Walking;
}

bool UVRCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate()
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

	FVector Orig_CameraLoc = VRRootCapsule->curCameraLoc;
	FRotator Orig_curCameraRot = VRRootCapsule->curCameraRot;
	FVector Orig_DifferenceFromLastFrame = VRRootCapsule->DifferenceFromLastFrame;
	FVector Orig_AdditionalVRInputVector = AdditionalVRInputVector;
	FRotator Orig_CameraRotOffset = VRRootCapsule->StoredCameraRotOffset;

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

	VRRootCapsule->curCameraLoc = Orig_CameraLoc;
	VRRootCapsule->curCameraRot = Orig_curCameraRot;
	VRRootCapsule->DifferenceFromLastFrame = Orig_DifferenceFromLastFrame;
	AdditionalVRInputVector = Orig_AdditionalVRInputVector;
	VRRootCapsule->StoredCameraRotOffset = Orig_CameraRotOffset;
	VRRootCapsule->GenerateOffsetToWorld(false, false);

	return (ClientData->SavedMoves.Num() > 0);
}

FVector UVRCharacterMovementComponent::GetPenetrationAdjustment(const FHitResult& Hit) const
{

	if (MovementMode == EMovementMode::MOVE_Walking && VRRootCapsule && VRRootCapsule->bUseWalkingCollisionOverride && Hit.Component.IsValid())
	{
		ECollisionResponse WalkingResponse;
		WalkingResponse = Hit.Component->GetCollisionResponseToChannel(VRRootCapsule->WalkingCollisionOverride);

		if (WalkingResponse == ECR_Ignore || WalkingResponse == ECR_Overlap)
		{
			return FVector::ZeroVector;
		}
	}

	FVector Result = Super::GetPenetrationAdjustment(Hit);

	if (CharacterOwner)
	{
		const bool bIsProxy = (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy);
		float MaxDistance = bIsProxy ? MaxDepenetrationWithGeometryAsProxy : MaxDepenetrationWithGeometry;
		if (Hit.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
		{
			MaxDistance = bIsProxy ? MaxDepenetrationWithPawnAsProxy : MaxDepenetrationWithPawn;
		}

		Result = Result.GetClampedToMaxSize(MaxDistance);
	}

	return Result;
}