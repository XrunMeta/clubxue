

#include "CharacterMovementCompTypes.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterMovementCompTypes)

#include "VRBaseCharacterMovementComponent.h"
#include "VRBPDatatypes.h"
#include "VRBaseCharacter.h"
#include "VRRootComponent.h"
#include "VRPlayerController.h"

FSavedMove_VRBaseCharacter::FSavedMove_VRBaseCharacter() : FSavedMove_Character()
{
	VRCapsuleLocation = FVector::ZeroVector;
	LFDiff = FVector::ZeroVector;
	VRCapsuleRotation = FRotator::ZeroRotator;
	VRReplicatedMovementMode = EVRConjoinedMovementModes::C_MOVE_MAX;
}

uint8 FSavedMove_VRBaseCharacter::GetCompressedFlags() const
{

	uint8 Result = FSavedMove_Character::GetCompressedFlags();

	return Result;
}

bool FSavedMove_VRBaseCharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	FSavedMove_VRBaseCharacter* nMove = (FSavedMove_VRBaseCharacter*)NewMove.Get();

	if (!nMove || (VRReplicatedMovementMode != nMove->VRReplicatedMovementMode))
		return false;

	if (!ConditionalValues.MoveActionArray.CanCombine() || !nMove->ConditionalValues.MoveActionArray.CanCombine())
		return false;

	if (!ConditionalValues.CustomVRInputVector.IsZero() || !nMove->ConditionalValues.CustomVRInputVector.IsZero())
		return false;

	if (!ConditionalValues.RequestedVelocity.IsZero() || !nMove->ConditionalValues.RequestedVelocity.IsZero())
		return false;

	if (!FMath::IsNearlyEqual(CapsuleHeight, nMove->CapsuleHeight))
		return false;

	if (!LFDiff.IsZero() && !nMove->LFDiff.IsZero() && !FVector::Coincident(LFDiff.GetSafeNormal(), nMove->LFDiff.GetSafeNormal(), AccelDotThresholdCombine))
		return false;

	return FSavedMove_Character::CanCombineWith(NewMove, Character, MaxDelta);
}

bool FSavedMove_VRBaseCharacter::IsImportantMove(const FSavedMovePtr& LastAckedMove) const
{

	if (VRReplicatedMovementMode != EVRConjoinedMovementModes::C_MOVE_MAX)
		return true;

	if (!ConditionalValues.CustomVRInputVector.IsZero())
		return true;

	if (!ConditionalValues.RequestedVelocity.IsZero())
		return true;

	if (ConditionalValues.MoveActionArray.MoveActions.Num() > 0)
		return true;

	return FSavedMove_Character::IsImportantMove(LastAckedMove);
}

void FSavedMove_VRBaseCharacter::SetInitialPosition(ACharacter* C)
{

	if (UVRBaseCharacterMovementComponent* moveComp = Cast<UVRBaseCharacterMovementComponent>(C->GetMovementComponent()))
	{

		VRReplicatedMovementMode = moveComp->VRReplicatedMovementMode;

		if (moveComp->HasRequestedVelocity())
			ConditionalValues.RequestedVelocity = moveComp->RequestedVelocity;
		else
			ConditionalValues.RequestedVelocity = FVector::ZeroVector;

		if (AVRBaseCharacter* BaseChar = Cast<AVRBaseCharacter>(C))
		{
			if (BaseChar->GetVRReplicateCapsuleHeight())
				CapsuleHeight = BaseChar->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
			else
				CapsuleHeight = 0.0f;
		}
		else
			CapsuleHeight = 0.0f;
	}
	else
	{
		VRReplicatedMovementMode = EVRConjoinedMovementModes::C_MOVE_MAX;
		ConditionalValues.CustomVRInputVector = FVector::ZeroVector;
		ConditionalValues.RequestedVelocity = FVector::ZeroVector;
	}

	FSavedMove_Character::SetInitialPosition(C);
}

void FSavedMove_VRBaseCharacter::CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation)
{
	UCharacterMovementComponent* CharMovement = InCharacter->GetCharacterMovement();

	CharMovement->UpdatedComponent->SetWorldLocationAndRotation(OldStartLocation, OldMove->StartRotation, false, nullptr, CharMovement->GetTeleportType());
	CharMovement->Velocity = OldMove->StartVelocity;

	FMovementBaseInterfaceData OldMovementBaseInterfaceData = OldMove->StartMovementBaseInterfaceData;
	CharMovement->SetBase(&OldMovementBaseInterfaceData, OldMove->StartBoneName);
	CharMovement->CurrentFloor = OldMove->StartFloor;

	DeltaTime += OldMove->DeltaTime;

	FSavedMove_VRBaseCharacter* BaseSavedMovePending = (FSavedMove_VRBaseCharacter*)OldMove;

	if (BaseSavedMovePending)
	{
		LFDiff.X += BaseSavedMovePending->LFDiff.X;
		LFDiff.Y += BaseSavedMovePending->LFDiff.Y;
	}

	InCharacter->JumpForceTimeRemaining = OldMove->JumpForceTimeRemaining;
	InCharacter->JumpKeyHoldTime = OldMove->JumpKeyHoldTime;

	ConditionalValues.MoveActionArray.MoveActions.Append(BaseSavedMovePending->ConditionalValues.MoveActionArray.MoveActions);
}

void FSavedMove_VRBaseCharacter::PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode)
{
	FSavedMove_Character::PostUpdate(C, PostUpdateMode);

	if (UVRBaseCharacterMovementComponent* moveComp = Cast<UVRBaseCharacterMovementComponent>(C->GetMovementComponent()))
	{
		ConditionalValues.CustomVRInputVector = moveComp->CustomVRInputVector;
		ConditionalValues.MoveActionArray = moveComp->MoveActionArray;
		moveComp->MoveActionArray.Clear();

		if (!moveComp->bUseClientControlRotation)
		{
			if (const USceneComponent* UpdatedComponent = moveComp->UpdatedComponent)
			{
				SavedControlRotation = UpdatedComponent->GetComponentRotation().Clamp();
			}
		}
	}

}

void FSavedMove_VRBaseCharacter::Clear()
{
	VRReplicatedMovementMode = EVRConjoinedMovementModes::C_MOVE_MAX;

	VRCapsuleLocation = FVector::ZeroVector;
	VRCapsuleRotation = FRotator::ZeroRotator;
	LFDiff = FVector::ZeroVector;
	CapsuleHeight = 0.0f;

	ConditionalValues.CustomVRInputVector = FVector::ZeroVector;
	ConditionalValues.RequestedVelocity = FVector::ZeroVector;
	ConditionalValues.MoveActionArray.Clear();

	FSavedMove_Character::Clear();
}

void FSavedMove_VRBaseCharacter::PrepMoveFor(ACharacter* Character)
{
	UVRBaseCharacterMovementComponent* BaseCharMove = Cast<UVRBaseCharacterMovementComponent>(Character->GetCharacterMovement());

	if (BaseCharMove)
	{
		BaseCharMove->MoveActionArray = ConditionalValues.MoveActionArray;

		BaseCharMove->CustomVRInputVector = ConditionalValues.CustomVRInputVector;
		BaseCharMove->VRReplicatedMovementMode = this->VRReplicatedMovementMode;
	}

	if (!ConditionalValues.RequestedVelocity.IsZero())
	{
		BaseCharMove->RequestedVelocity = ConditionalValues.RequestedVelocity;
		BaseCharMove->SetHasRequestedVelocity(true);
	}
	else
	{
		BaseCharMove->SetHasRequestedVelocity(false);
	}

	FSavedMove_Character::PrepMoveFor(Character);
}

FVRCharacterScopedMovementUpdate::FVRCharacterScopedMovementUpdate(USceneComponent* Component, EScopedUpdate::Type ScopeBehavior, bool bRequireOverlapsEventFlagToQueueOverlaps)
	: FScopedMovementUpdate(Component, ScopeBehavior, bRequireOverlapsEventFlagToQueueOverlaps)
{
	UVRRootComponent* RootComponent = Cast<UVRRootComponent>(Owner);
	if (RootComponent)
	{
		InitialVRTransform = RootComponent->OffsetComponentToWorld;
	}
}

void FVRCharacterScopedMovementUpdate::RevertMove()
{
	bool bTransformIsDirty = IsTransformDirty();

	FScopedMovementUpdate::RevertMove();

	UVRRootComponent* RootComponent = Cast<UVRRootComponent>(Owner);
	if (RootComponent)
	{

		if (!bTransformIsDirty && !IsDeferringUpdates() && !InitialVRTransform.Equals(RootComponent->OffsetComponentToWorld))
		{
			RootComponent->UpdateOverlaps();
		}

		RootComponent->GenerateOffsetToWorld();
	}
}

FVRCharacterNetworkMoveData::FVRCharacterNetworkMoveData() : FCharacterNetworkMoveData()
{
	VRCapsuleLocation = FVector::ZeroVector;
	LFDiff = FVector::ZeroVector;
	CapsuleHeight = 0.f;
	VRCapsuleRotation = 0.f;
	ReplicatedMovementMode = EVRConjoinedMovementModes::C_MOVE_MAX;
}

FVRCharacterNetworkMoveData::~FVRCharacterNetworkMoveData()
{
}

void FVRCharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{

	FCharacterNetworkMoveData::ClientFillNetworkMoveData(ClientMove, MoveType);

	if (const FSavedMove_VRBaseCharacter* SavedMove = (const FSavedMove_VRBaseCharacter*)(&ClientMove))
	{
		ReplicatedMovementMode = SavedMove->VRReplicatedMovementMode;
		ConditionalMoveReps = SavedMove->ConditionalValues;

		VRCapsuleLocation = SavedMove->VRCapsuleLocation;
		LFDiff = SavedMove->LFDiff;
		CapsuleHeight = SavedMove->CapsuleHeight;
		VRCapsuleRotation = FRotator::CompressAxisToShort(SavedMove->VRCapsuleRotation.Yaw);
	}
}

bool FVRCharacterNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	NetworkMoveType = MoveType;

	bool bLocalSuccess = true;
	const bool bIsSaving = Ar.IsSaving();

	Ar << TimeStamp;

	bool bRepAccel = bIsSaving ? !Acceleration.IsNearlyZero() : false;
	Ar.SerializeBits(&bRepAccel, 1);

	if (bRepAccel)
	{
		Acceleration.NetSerialize(Ar, PackageMap, bLocalSuccess);
	}
	else
	{
		if (!bIsSaving)
		{
			Acceleration = FVector::ZeroVector;
		}
	}

	uint16 Yaw = bIsSaving ? FRotator::CompressAxisToShort(ControlRotation.Yaw) : 0;
	uint16 Pitch = bIsSaving ? FRotator::CompressAxisToShort(ControlRotation.Pitch) : 0;
	uint16 Roll = bIsSaving ? FRotator::CompressAxisToShort(ControlRotation.Roll) : 0;
	bool bRepYaw = Yaw != 0;

	ACharacter* CharacterOwner = CharacterMovement.GetCharacterOwner();

	bool bRepRollAndPitch = false;

	if (AVRBaseCharacter* BaseChar = Cast<AVRBaseCharacter>(CharacterOwner))
	{
		if (BaseChar->VRMovementReference && !BaseChar->VRMovementReference->bUseClientControlRotation)
		{
			bRepRollAndPitch = (Roll != 0 || Pitch != 0);
		}
		else
		{
			bool bCanRepRollAndPitch = (CharacterOwner && (CharacterOwner->bUseControllerRotationRoll || CharacterOwner->bUseControllerRotationPitch));
			bRepRollAndPitch = bCanRepRollAndPitch && (Roll != 0 || Pitch != 0);
		}
	}
	else
	{
		bool bCanRepRollAndPitch = (CharacterOwner && (CharacterOwner->bUseControllerRotationRoll || CharacterOwner->bUseControllerRotationPitch));
		bRepRollAndPitch = bCanRepRollAndPitch && (Roll != 0 || Pitch != 0);
	}

	Ar.SerializeBits(&bRepRollAndPitch, 1);

	if (bRepRollAndPitch)
	{

		uint32 Rotation32 = 0;
		uint32 Yaw32 = bIsSaving ? Yaw : 0;

		if (bIsSaving)
		{
			Rotation32 = (((uint32)Roll) << 16) | ((uint32)Pitch);
			Ar.SerializeIntPacked(Rotation32);
		}
		else
		{
			Ar.SerializeIntPacked(Rotation32);

			Pitch = (Rotation32 & 65535);
			Roll = (Rotation32 >> 16);
		}
	}

	uint32 Yaw32 = bIsSaving ? Yaw : 0;

	Ar.SerializeBits(&bRepYaw, 1);
	if (bRepYaw)
	{
		Ar.SerializeIntPacked(Yaw32);
		Yaw = (uint16)Yaw32;
	}

	if (!bIsSaving)
	{
		ControlRotation.Yaw = bRepYaw ? FRotator::DecompressAxisFromShort(Yaw) : 0;
		ControlRotation.Pitch = bRepRollAndPitch ? FRotator::DecompressAxisFromShort(Pitch) : 0;
		ControlRotation.Roll = bRepRollAndPitch ? FRotator::DecompressAxisFromShort(Roll) : 0;
	}

	SerializeOptionalValue<uint8>(bIsSaving, Ar, CompressedMoveFlags, 0);
	SerializeOptionalValue<uint8>(bIsSaving, Ar, MovementMode, MOVE_Walking);
	VRCapsuleLocation.NetSerialize(Ar, PackageMap, bLocalSuccess);
	Ar << VRCapsuleRotation;

		Location.NetSerialize(Ar, PackageMap, bLocalSuccess);

	SerializeOptionalValue<TObjectPtr<UObject>>(bIsSaving, Ar, MovementBasePhysicsObjectOwner, nullptr);
	SerializeOptionalValue<FName>(bIsSaving, Ar, MovementBaseBoneName, NAME_None);

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
		MovementBase = Cast<UPrimitiveComponent>(MovementBasePhysicsObjectOwner.Get());
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	bool bHasReplicatedMovementMode = ReplicatedMovementMode != EVRConjoinedMovementModes::C_MOVE_MAX;
	Ar.SerializeBits(&bHasReplicatedMovementMode, 1);

	if (bHasReplicatedMovementMode)
	{

		Ar.SerializeBits(&ReplicatedMovementMode, 6);
	}
	else if(!bIsSaving)
	{
		ReplicatedMovementMode = EVRConjoinedMovementModes::C_MOVE_MAX;
	}

	ConditionalMoveReps.NetSerialize(Ar, PackageMap, bLocalSuccess);

	if (AVRBaseCharacter* VRChar = Cast<AVRBaseCharacter>(CharacterOwner))
	{
		if (!VRChar->bRetainRoomscale)
		{
			SerializePackedVector<10000, 32>(LFDiff, Ar);
		}
		else
		{
			SerializePackedVector<100, 30>(LFDiff, Ar);
		}
	}
	else
	{
		SerializePackedVector<100, 30>(LFDiff, Ar);
	}

	bool bHasCapsuleHeight = CapsuleHeight > 0.f;
	Ar.SerializeBits(&bHasCapsuleHeight, 1);

	if (bHasCapsuleHeight)
	{

		if (Ar.IsSaving())
		{
			WriteFixedCompressedFloat<1024, 18>(CapsuleHeight, Ar);
		}
		else
		{
			ReadFixedCompressedFloat<1024, 18>(CapsuleHeight, Ar);
		}
	}

	return !Ar.IsError();
}

void FVRCharacterMoveResponseDataContainer::ServerFillResponseData(const UCharacterMovementComponent& CharacterMovement, const FClientAdjustment& PendingAdjustment)
{
	FCharacterMoveResponseDataContainer::ServerFillResponseData(CharacterMovement, PendingAdjustment);

	if (const UVRBaseCharacterMovementComponent* BaseMovecomp = Cast<const UVRBaseCharacterMovementComponent>(&CharacterMovement))
	{

		bHasRotation = !BaseMovecomp->bUseClientControlRotation;
	}
}

FScopedMeshBoneUpdateOverrideVR::FScopedMeshBoneUpdateOverrideVR(USkeletalMeshComponent* Mesh, EKinematicBonesUpdateToPhysics::Type OverrideSetting)
	: MeshRef(Mesh)
{
	if (MeshRef)
	{

		SavedUpdateSetting = MeshRef->KinematicBonesUpdateType;

		MeshRef->KinematicBonesUpdateType = OverrideSetting;
	}
}

FScopedMeshBoneUpdateOverrideVR::~FScopedMeshBoneUpdateOverrideVR()
{
	if (MeshRef)
	{

		MeshRef->KinematicBonesUpdateType = SavedUpdateSetting;
	}
}