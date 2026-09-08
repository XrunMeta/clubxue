

#pragma once
#include "CoreMinimal.h"
#include "VRBPDatatypes.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/ScopedMovementUpdate.h"
#include "Interfaces/MovementBaseInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterMovementCompTypes.generated.h"

class AVRBaseCharacter;
class UVRBaseCharacterMovementComponent;

UENUM(Blueprintable)
enum class EVRMoveAction : uint8
{
	VRMOVEACTION_None = 0x00,
	VRMOVEACTION_SnapTurn = 0x01,
	VRMOVEACTION_Teleport = 0x02,
	VRMOVEACTION_StopAllMovement = 0x03,
	VRMOVEACTION_SetRotation = 0x04,
	VRMOVEACTION_PauseTracking = 0x14,
	VRMOVEACTION_SetGravityDirection = 0x15, 
	VRMOVEACTION_CUSTOM1 = 0x05,
	VRMOVEACTION_CUSTOM2 = 0x06,
	VRMOVEACTION_CUSTOM3 = 0x07,
	VRMOVEACTION_CUSTOM4 = 0x08,
	VRMOVEACTION_CUSTOM5 = 0x09,
	VRMOVEACTION_CUSTOM6 = 0x0A,
	VRMOVEACTION_CUSTOM7 = 0x0B,
	VRMOVEACTION_CUSTOM8 = 0x0C,
	VRMOVEACTION_CUSTOM9 = 0x0D,
	VRMOVEACTION_CUSTOM10 = 0x0E,
	VRMOVEACTION_CUSTOM11 = 0x0F,
	VRMOVEACTION_CUSTOM12 = 0x10,
	VRMOVEACTION_CUSTOM13 = 0x11,
	VRMOVEACTION_CUSTOM14 = 0x12,
	VRMOVEACTION_CUSTOM15 = 0x13,

};

UENUM(Blueprintable)
enum class EVRMoveActionVelocityRetention : uint8
{

	VRMOVEACTION_Velocity_None = 0x00,

	VRMOVEACTION_Velocity_Clear = 0x01,

	VRMOVEACTION_Velocity_Turn = 0x02
};

UENUM(Blueprintable)
enum class EVRMoveActionDataReq : uint8
{
	VRMOVEACTIONDATA_None = 0x00,
	VRMOVEACTIONDATA_LOC = 0x01,
	VRMOVEACTIONDATA_ROT = 0x02,
	VRMOVEACTIONDATA_LOC_AND_ROT = 0x03
};

USTRUCT()
struct VREXPANSIONPLUGIN_API FVRMoveActionContainer
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
		EVRMoveAction MoveAction;
	UPROPERTY()
		EVRMoveActionDataReq MoveActionDataReq;
	UPROPERTY()
		FVector MoveActionLoc;
	UPROPERTY()
		FVector MoveActionVel;
	UPROPERTY()
		FRotator MoveActionRot;
	UPROPERTY()
		float MoveActionDeltaYaw;
	UPROPERTY()
		uint8 MoveActionFlags;
	UPROPERTY()
		TArray<TObjectPtr<UObject>> MoveActionObjectReferences;
	UPROPERTY()
		EVRMoveActionVelocityRetention VelRetentionSetting;

	FVRMoveActionContainer()
	{
		Clear();
	}

	void Clear()
	{
		MoveAction = EVRMoveAction::VRMOVEACTION_None;
		MoveActionDataReq = EVRMoveActionDataReq::VRMOVEACTIONDATA_None;
		MoveActionLoc = FVector::ZeroVector;
		MoveActionVel = FVector::ZeroVector;
		MoveActionRot = FRotator::ZeroRotator;
		MoveActionDeltaYaw = 0.0f;
		MoveActionFlags = 0;
		VelRetentionSetting = EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_None;
		MoveActionObjectReferences.Empty();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		Ar.SerializeBits(&MoveAction, 6); 

		switch (MoveAction)
		{
		case EVRMoveAction::VRMOVEACTION_None: break;
		case EVRMoveAction::VRMOVEACTION_SetRotation:
		case EVRMoveAction::VRMOVEACTION_SnapTurn:
		{
			uint16 Yaw = 0;
			uint16 Pitch = 0;

			if (Ar.IsSaving())
			{
				bool bUseLocOnly = MoveActionFlags & 0x04;
				Ar.SerializeBits(&bUseLocOnly, 1);

				if (!bUseLocOnly)
				{

					Ar << MoveActionRot;
				}
				else
				{
					Ar << MoveActionLoc;
				}

				bool bTeleportGrips = MoveActionFlags & 0x01;
				Ar.SerializeBits(&bTeleportGrips, 1);

				if (!bTeleportGrips)
				{
					bool bTeleportCharacter = MoveActionFlags & 0x02;
					Ar.SerializeBits(&bTeleportCharacter, 1);
				}

				Ar.SerializeBits(&VelRetentionSetting, 2);

				if (VelRetentionSetting == EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn)
				{
					bOutSuccess &= SerializePackedVector<100, 30>(MoveActionVel, Ar);
				}

				Pitch = FRotator::CompressAxisToShort(MoveActionDeltaYaw);
				Ar << Pitch;

				bool bRotateAroundCapsule = MoveActionFlags & 0x08;
				Ar.SerializeBits(&bRotateAroundCapsule, 1);
			}
			else
			{

				bool bUseLocOnly = false;
				Ar.SerializeBits(&bUseLocOnly, 1);
				MoveActionFlags |= (bUseLocOnly << 2);

				if (!bUseLocOnly)
				{

					Ar << MoveActionRot;
				}
				else
				{
					Ar << MoveActionLoc;
				}

				bool bTeleportGrips = false;
				Ar.SerializeBits(&bTeleportGrips, 1);
				MoveActionFlags |= (uint8)bTeleportGrips; 

				if (!bTeleportGrips)
				{
					bool bTeleportCharacter = false;
					Ar.SerializeBits(&bTeleportCharacter, 1);
					MoveActionFlags |= ((uint8)bTeleportCharacter << 1);

				}

				Ar.SerializeBits(&VelRetentionSetting, 2);

				if (VelRetentionSetting == EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn)
				{
					bOutSuccess &= SerializePackedVector<100, 30>(MoveActionVel, Ar);
				}

				Ar << Pitch;
				MoveActionDeltaYaw = FRotator::DecompressAxisFromShort(Pitch);

				bool bRotateAroundCapsule = false;
				Ar.SerializeBits(&bRotateAroundCapsule, 1);
				MoveActionFlags |= (uint8)(bRotateAroundCapsule << 3);
			}

		}break;
		case EVRMoveAction::VRMOVEACTION_Teleport: 
		{
			uint16 Yaw = 0;
			uint16 Pitch = 0;

			if (Ar.IsSaving())
			{
				Yaw = FRotator::CompressAxisToShort(MoveActionRot.Yaw);
				Ar << Yaw;

				bool bSkipEncroachment = MoveActionFlags & 0x01;
				Ar.SerializeBits(&bSkipEncroachment, 1);
				Ar.SerializeBits(&VelRetentionSetting, 2);

				if (VelRetentionSetting == EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn)
				{
					bOutSuccess &= SerializePackedVector<100, 30>(MoveActionVel, Ar);

				}
			}
			else
			{
				Ar << Yaw;
				MoveActionRot.Yaw = FRotator::DecompressAxisFromShort(Yaw);

				bool bSkipEncroachment = false;
				Ar.SerializeBits(&bSkipEncroachment, 1);
				MoveActionFlags |= (uint8)bSkipEncroachment;

				Ar.SerializeBits(&VelRetentionSetting, 2);

				if (VelRetentionSetting == EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn)
				{
					bOutSuccess &= SerializePackedVector<100, 30>(MoveActionVel, Ar);

				}
			}

			bOutSuccess &= SerializePackedVector<100, 30>(MoveActionLoc, Ar);
		}break;
		case EVRMoveAction::VRMOVEACTION_StopAllMovement:
		{}break;
		case EVRMoveAction::VRMOVEACTION_SetGravityDirection:
		{
			bOutSuccess = SerializeFixedVector<1, 16>(MoveActionVel, Ar);
			if (Ar.IsSaving())
			{
				bool bOrientToGravity = MoveActionFlags > 0;
				Ar.SerializeBits(&bOrientToGravity, 1);
			}
			else
			{
				bool bOrientToGravity = false;
				Ar.SerializeBits(&bOrientToGravity, 1);
				MoveActionFlags |= (uint8)bOrientToGravity;
			}

		}break;
		case EVRMoveAction::VRMOVEACTION_PauseTracking:
		{

			Ar.SerializeBits(&MoveActionFlags, 1);
			bOutSuccess &= SerializePackedVector<100, 30>(MoveActionLoc, Ar);

			uint16 Yaw = 0;

			if (Ar.IsSaving())
			{
				Yaw = FRotator::CompressAxisToShort(MoveActionRot.Yaw);
				Ar << Yaw;
			}
			else
			{
				Ar << Yaw;
				MoveActionRot.Yaw = FRotator::DecompressAxisFromShort(Yaw);
			}

		}break;
		default: 
		{

			Ar.SerializeBits(&MoveActionDataReq, 2);

			if (((uint8)MoveActionDataReq & (uint8)EVRMoveActionDataReq::VRMOVEACTIONDATA_LOC) != 0)
				bOutSuccess &= SerializePackedVector<100, 30>(MoveActionLoc, Ar);

			if (((uint8)MoveActionDataReq & (uint8)EVRMoveActionDataReq::VRMOVEACTIONDATA_ROT) != 0)
				MoveActionRot.SerializeCompressedShort(Ar);

			bool bSerializeObjects = MoveActionObjectReferences.Num() > 0;
			Ar.SerializeBits(&bSerializeObjects, 1);
			if (bSerializeObjects)
			{
				Ar << MoveActionObjectReferences;
			}

			bool bSerializeFlags = MoveActionFlags != 0x00;
			Ar.SerializeBits(&bSerializeFlags, 1);
			if (bSerializeFlags)
			{
				Ar << MoveActionFlags;
			}

		}break;
		}

		return bOutSuccess;
	}
};
template<>
struct TStructOpsTypeTraits< FVRMoveActionContainer > : public TStructOpsTypeTraitsBase2<FVRMoveActionContainer>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT()
struct VREXPANSIONPLUGIN_API FVRMoveActionArray
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
		TArray<FVRMoveActionContainer> MoveActions;

	bool CanCombine() const 
	{
		return !MoveActions.Num();

	}

	void Clear()
	{
		MoveActions.Empty();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;
		uint8 MoveActionCount = (uint8)MoveActions.Num();
		bool bHasAMoveAction = MoveActionCount > 0;
		Ar.SerializeBits(&bHasAMoveAction, 1);

		if (bHasAMoveAction)
		{
			bool bHasMoreThanOneMoveAction = MoveActionCount > 1;
			Ar.SerializeBits(&bHasMoreThanOneMoveAction, 1);

			if (Ar.IsSaving())
			{
				if (bHasMoreThanOneMoveAction)
				{
					Ar << MoveActionCount;

					for (int i = 0; i < MoveActionCount; i++)
					{
						bOutSuccess &= MoveActions[i].NetSerialize(Ar, Map, bOutSuccess);
					}
				}
				else
				{
					bOutSuccess &= MoveActions[0].NetSerialize(Ar, Map, bOutSuccess);
				}
			}
			else
			{
				if (bHasMoreThanOneMoveAction)
				{
					Ar << MoveActionCount;
				}
				else
					MoveActionCount = 1;

				for (int i = 0; i < MoveActionCount; i++)
				{
					FVRMoveActionContainer MoveAction;
					bOutSuccess &= MoveAction.NetSerialize(Ar, Map, bOutSuccess);
					MoveActions.Add(MoveAction);
				}
			}
		}

		return bOutSuccess;
	}
};
template<>
struct TStructOpsTypeTraits< FVRMoveActionArray > : public TStructOpsTypeTraitsBase2<FVRMoveActionArray>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT()
struct VREXPANSIONPLUGIN_API FVRConditionalMoveRep
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY(Transient)
		FVector CustomVRInputVector;
	UPROPERTY(Transient)
		FVector RequestedVelocity;
	UPROPERTY(Transient)
		FVRMoveActionArray MoveActionArray;

	FVRConditionalMoveRep()
	{
		CustomVRInputVector = FVector::ZeroVector;
		RequestedVelocity = FVector::ZeroVector;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		bool bIsLoading = Ar.IsLoading();

		bool bHasVRinput = !CustomVRInputVector.IsZero();
		bool bHasRequestedVelocity = !RequestedVelocity.IsZero();
		bool bHasMoveAction = MoveActionArray.MoveActions.Num() > 0;

		bool bHasAnyProperties = bHasVRinput || bHasRequestedVelocity || bHasMoveAction;
		Ar.SerializeBits(&bHasAnyProperties, 1);

		if (bHasAnyProperties)
		{
			Ar.SerializeBits(&bHasVRinput, 1);
			Ar.SerializeBits(&bHasRequestedVelocity, 1);

			if (bHasVRinput)
			{
				bOutSuccess &= SerializePackedVector<100, 22>(CustomVRInputVector, Ar);
			}
			else if (bIsLoading)
			{
				CustomVRInputVector = FVector::ZeroVector;
			}

			if (bHasRequestedVelocity)
			{
				bOutSuccess &= SerializePackedVector<100, 22>(RequestedVelocity, Ar);
			}
			else if (bIsLoading)
			{
				RequestedVelocity = FVector::ZeroVector;
			}

			MoveActionArray.NetSerialize(Ar, Map, bOutSuccess);
		}
		else if (bIsLoading)
		{
			CustomVRInputVector = FVector::ZeroVector;
			RequestedVelocity = FVector::ZeroVector;
			MoveActionArray.Clear();
		}

		return bOutSuccess;
	}

};

template<>
struct TStructOpsTypeTraits< FVRConditionalMoveRep > : public TStructOpsTypeTraitsBase2<FVRConditionalMoveRep>
{
	enum
	{
		WithNetSerializer = true
	};
};

struct FScopedMeshBoneUpdateOverrideVR
{
	FScopedMeshBoneUpdateOverrideVR(USkeletalMeshComponent* Mesh, EKinematicBonesUpdateToPhysics::Type OverrideSetting);

	~FScopedMeshBoneUpdateOverrideVR();

private:
	USkeletalMeshComponent* MeshRef;
	EKinematicBonesUpdateToPhysics::Type SavedUpdateSetting;
};

class VREXPANSIONPLUGIN_API FSavedMove_VRBaseCharacter : public FSavedMove_Character
{

public:

	EVRConjoinedMovementModes VRReplicatedMovementMode;

	FVector VRCapsuleLocation;
	FVector LFDiff;
	FRotator VRCapsuleRotation;
	float CapsuleHeight;
	FVRConditionalMoveRep ConditionalValues;

	void Clear();
	virtual void SetInitialPosition(ACharacter* C);
	virtual void PrepMoveFor(ACharacter* Character) override;
	virtual void CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation) override;

	virtual void PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode) override;

	FSavedMove_VRBaseCharacter();

	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;
};

class VREXPANSIONPLUGIN_API FVRCharacterScopedMovementUpdate : public FScopedMovementUpdate
{
public:

	FVRCharacterScopedMovementUpdate(USceneComponent* Component, EScopedUpdate::Type ScopeBehavior = EScopedUpdate::DeferredUpdates, bool bRequireOverlapsEventFlagToQueueOverlaps = true);

	FTransform InitialVRTransform;

	void RevertMove();
};

struct VREXPANSIONPLUGIN_API FVRCharacterNetworkMoveData : public FCharacterNetworkMoveData
{
public:

	FVector_NetQuantize100 VRCapsuleLocation;
	FVector LFDiff;
	float CapsuleHeight;
	uint16 VRCapsuleRotation;
	EVRConjoinedMovementModes ReplicatedMovementMode;
	FVRConditionalMoveRep ConditionalMoveReps;

	FVRCharacterNetworkMoveData();

	virtual ~FVRCharacterNetworkMoveData();
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;
};

struct VREXPANSIONPLUGIN_API FVRCharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
public:

	FVRCharacterNetworkMoveDataContainer() : FCharacterNetworkMoveDataContainer()
	{
		NewMoveData = &VRBaseDefaultMoveData[0];
		PendingMoveData = &VRBaseDefaultMoveData[1];
		OldMoveData = &VRBaseDefaultMoveData[2];
	}

	virtual ~FVRCharacterNetworkMoveDataContainer()
	{
	}

protected:

	FVRCharacterNetworkMoveData VRBaseDefaultMoveData[3];

};

struct VREXPANSIONPLUGIN_API FVRCharacterMoveResponseDataContainer : public FCharacterMoveResponseDataContainer
{
public:

	FVRCharacterMoveResponseDataContainer() : FCharacterMoveResponseDataContainer()
	{
	}

	virtual ~FVRCharacterMoveResponseDataContainer()
	{
	}

	virtual void ServerFillResponseData(const UCharacterMovementComponent& CharacterMovement, const FClientAdjustment& PendingAdjustment) override;

};