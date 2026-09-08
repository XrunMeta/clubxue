

#pragma once
#include "CoreMinimal.h"
#include "VRBPDatatypes.h"
#include "VRBaseCharacterMovementComponent.h"
#include "ReplicatedVRCameraComponent.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"

#include "Iris/Serialization/NetSerializer.h"

#include "VRBaseCharacter.generated.h"

class AVRPlayerController;
class UGripMotionControllerComponent;
class UParentRelativeAttachmentComponent;
class AController;
class UNavigationQueryFilter;

DECLARE_LOG_CATEGORY_EXTERN(LogBaseVRCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVRSeatThresholdChangedSignature, bool, bIsWithinThreshold, float, ToThresholdScaler);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVRSeatZeroedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRPlayerStateReplicatedSignature, const APlayerState *, NewPlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVRPlayerTeleportedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVRPlayerNetworkCorrectedSignature);

USTRUCT()
struct VREXPANSIONPLUGIN_API FRepMovementVRCharacter : public FRepMovement
{
	GENERATED_BODY()

public:

	FRepMovementVRCharacter();

	UPROPERTY(Transient)
		bool bJustTeleported;

	UPROPERTY(Transient)
		bool bJustTeleportedGrips;

	UPROPERTY(Transient)
		bool bPausedTracking;

	UPROPERTY(Transient)
		FVector_NetQuantize100 PausedTrackingLoc;

	UPROPERTY(Transient)
		float PausedTrackingRot;

	UPROPERTY(Transient)
		TObjectPtr<AActor> Owner;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FRepMovementVRCharacter> : public TStructOpsTypeTraitsBase2<FRepMovementVRCharacter>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

USTRUCT(Blueprintable)
struct VREXPANSIONPLUGIN_API FVRSeatedCharacterInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "CharacterSeatInfo")
		bool bSitting;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterSeatInfo")
		bool bZeroToHead;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterSeatInfo")
		FTransform_NetQuantize StoredTargetTransform;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterSeatInfo")
		FTransform_NetQuantize InitialRelCameraTransform;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterSeatInfo")
		TObjectPtr<USceneComponent> SeatParent;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterSeatInfo")
		EVRConjoinedMovementModes PostSeatedMovementMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, NotReplicated, Category = "CharacterSeatInfo", meta = (ClampMin = "1.000", UIMin = "1.000", ClampMax = "256.000", UIMax = "256.000"))
		float AllowedRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NotReplicated, Category = "CharacterSeatInfo", meta = (ClampMin = "1.000", UIMin = "1.000", ClampMax = "256.000", UIMax = "256.000"))
		float AllowedRadiusThreshold;
	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = "CharacterSeatInfo")
		float CurrentThresholdScaler;
	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = "CharacterSeatInfo")
		bool bIsOverThreshold;

	bool bWasSeated;
	bool bOriginalControlRotation;
	bool bWasOverLimit;

	FVRSeatedCharacterInfo()
	{
		Clear();
	}

	void Clear()
	{
		bSitting = false;
		bIsOverThreshold = false;
		bWasOverLimit = false;
		bZeroToHead = true;
		StoredTargetTransform = FTransform::Identity;
		InitialRelCameraTransform = FTransform::Identity;
		bWasSeated = false;
		bOriginalControlRotation = false;
		AllowedRadius = 40.0f;
		AllowedRadiusThreshold = 20.0f;
		CurrentThresholdScaler = 0.0f;
		SeatParent = nullptr;
		PostSeatedMovementMode = EVRConjoinedMovementModes::C_MOVE_Walking;
	}

	void ClearTempVals()
	{
		bWasOverLimit = false;
		bWasSeated = false;
		bOriginalControlRotation = false;
		CurrentThresholdScaler = 0.0f;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		Ar.SerializeBits(&bSitting, 1);
		Ar.SerializeBits(&bZeroToHead, 1);

		if (bSitting)
		{
			InitialRelCameraTransform.NetSerialize(Ar, Map, bOutSuccess);

			if (Ar.IsSaving())
			{
				bOutSuccess &= WriteFixedCompressedFloat<256, 16>(AllowedRadius, Ar);
				bOutSuccess &= WriteFixedCompressedFloat<256, 16>(AllowedRadiusThreshold, Ar);
			}
			else
			{
				bOutSuccess &= ReadFixedCompressedFloat<256, 16>(AllowedRadius, Ar);
				bOutSuccess &= ReadFixedCompressedFloat<256, 16>(AllowedRadiusThreshold, Ar);
			}
		}

		StoredTargetTransform.NetSerialize(Ar, Map, bOutSuccess);
		Ar << SeatParent;
		Ar << PostSeatedMovementMode;	
		return bOutSuccess;
	}
};
template<>
struct TStructOpsTypeTraits< FVRSeatedCharacterInfo > : public TStructOpsTypeTraitsBase2<FVRSeatedCharacterInfo>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT()
struct VREXPANSIONPLUGIN_API FVRReplicatedCapsuleHeight
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
		float CapsuleHeight;

	FVRReplicatedCapsuleHeight() :
		CapsuleHeight(0.0f)
	{}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		if (Ar.IsSaving())
		{
			bOutSuccess &= WriteFixedCompressedFloat<1024, 18>(CapsuleHeight, Ar);
		}
		else
		{
			bOutSuccess &= ReadFixedCompressedFloat<1024, 18>(CapsuleHeight, Ar);
		}

		return bOutSuccess;
	}
};
template<>
struct TStructOpsTypeTraits< FVRReplicatedCapsuleHeight > : public TStructOpsTypeTraitsBase2<FVRReplicatedCapsuleHeight>
{
	enum
	{
		WithNetSerializer = true
	};
};

UCLASS()
class VREXPANSIONPLUGIN_API AVRBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVRBaseCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(Transient, DuplicateTransient)
		TObjectPtr<AVRPlayerController> OwningVRPlayerController;

	UPROPERTY(Category = VRBaseCharacter, EditAnywhere, BlueprintReadOnly)
		bool bRetainRoomscale = false;

	virtual void PostInitializeComponents() override;

	virtual void PossessedBy(AController* NewController);
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedMovement)
		struct FRepMovementVRCharacter ReplicatedMovementVR;

	bool bFlagTeleported;
	bool bFlagTeleportedGrips;
	bool bTrackingPaused;
	FVector PausedTrackingLoc;
	float PausedTrackingRot;

	virtual void OnRep_ReplicatedMovement() override;
	virtual void GatherCurrentMovement() override;

	UPROPERTY(BlueprintAssignable, Category = "VRMovement")
		FVRPlayerTeleportedSignature OnCharacterTeleported_Bind;

	UPROPERTY(BlueprintAssignable, Category = "VRMovement")
		FVRPlayerNetworkCorrectedSignature OnCharacterNetworkCorrected_Bind;

	UPROPERTY(BlueprintAssignable, Category = "VRMovement")
		FVRPlayerStateReplicatedSignature OnPlayerStateReplicated_Bind;

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_SendTransformCamera(FBPVRComponentPosRep NewTransform);

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_SendTransformLeftController(FBPVRComponentPosRep NewTransform);

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_SendTransformRightController(FBPVRComponentPosRep NewTransform);

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRBaseCharacter")
		bool VRReplicateCapsuleHeight;
public:
	bool GetVRReplicateCapsuleHeight() { return VRReplicateCapsuleHeight; }
	void SetVRReplicateCapsuleHeight(bool bNewVRReplicateCapsuleHeight);

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_CapsuleHeight)
		FVRReplicatedCapsuleHeight ReplicatedCapsuleHeight;

	UFUNCTION()
	void OnRep_CapsuleHeight();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRMovement")
		void OnClimbingSteppedUp();
	virtual void OnClimbingSteppedUp_Implementation();

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "UpdateLowGravMovement", ScriptName = "UpdateLowGravMovement"))
		void UpdateLowGravMovement(float DeltaTime);
	virtual void UpdateLowGravMovement_Implementation(float DeltaTime) {} 

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "UpdateClimbingMovement", ScriptName = "UpdateClimbingMovement"))
		void UpdateClimbingMovement(float DeltaTime);
	virtual void UpdateClimbingMovement_Implementation(float DeltaTime){} 

	UPROPERTY(BlueprintReadOnly, Transient, Category = "VRExpansionLibrary")
	FTransform OffsetComponentToWorld;

	UFUNCTION(BlueprintPure, Category = "BaseVRCharacter|VRLocations")
	FVector GetVRForwardVector() const
	{
		return OffsetComponentToWorld.GetRotation().GetForwardVector();
	}

	UFUNCTION(BlueprintPure, Category = "BaseVRCharacter|VRLocations")
		FVector GetVRRightVector() const
	{
		return OffsetComponentToWorld.GetRotation().GetRightVector();
	}

	UFUNCTION(BlueprintPure, Category = "BaseVRCharacter|VRLocations")
		FVector GetVRUpVector() const
	{
		return OffsetComponentToWorld.GetRotation().GetUpVector();
	}

	UFUNCTION(BlueprintPure, Category = "BaseVRCharacter|VRLocations")
	FVector GetVRLocation() const
	{
		return OffsetComponentToWorld.GetLocation();
	}

	inline FVector GetVRLocation_Inline() const
	{
		return OffsetComponentToWorld.GetLocation();
	}

	virtual FVector GetProjectedVRLocation() const;

	UFUNCTION(BlueprintPure, Category = "BaseVRCharacter|VRLocations")
		FRotator GetVRRotation() const
	{
		return OffsetComponentToWorld.GetRotation().Rotator();
	}

	UFUNCTION(BlueprintPure, Category = "BaseVRCharacter|VRLocations", meta = (DisplayName = "GetVRHeadLocation", ScriptName = "GetVRHeadLocation", Keywords = "position"))
		FVector K2_GetVRHeadLocation() const
	{
		return GetVRHeadLocation();
	}

	inline FVector GetVRHeadLocation() const
	{
		return VRReplicatedCamera != nullptr ? VRReplicatedCamera->GetComponentLocation() : OffsetComponentToWorld.GetLocation();
	}

	virtual FVector GetTargetLocation(AActor* RequestedBy) const override
	{
		return GetVRLocation_Inline();
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRBaseCharacter")
		bool bUseExperimentalUnseatModeFix;

	UPROPERTY(BlueprintReadOnly, Replicated, EditAnywhere, Category = "Seating", ReplicatedUsing = OnRep_SeatedCharInfo)
	FVRSeatedCharacterInfo SeatInformation;

	UFUNCTION(BlueprintNativeEvent, Category = "Seating")
		void OnSeatedModeChanged(bool bNewSeatedMode, bool bWasAlreadySeated);
	virtual void OnSeatedModeChanged_Implementation(bool bNewSeatedMode, bool bWasAlreadySeated) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Seating")
		void OnSeatingRepositioned();
	virtual void OnSeatingRepositioned_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Seating")
		void OnSeatThreshholdChanged(bool bIsWithinThreshold, float ToThresholdScaler);
	virtual void OnSeatThreshholdChanged_Implementation(bool bIsWithinThreshold, float ToThresholdScaler) {}

	UPROPERTY(BlueprintAssignable, Category = "Seating")
		FVRSeatThresholdChangedSignature OnSeatThreshholdChanged_Bind;

	virtual FVector GetTargetHeightOffset()
	{
		return FVector::ZeroVector;
	}

	virtual void ZeroToSeatInformation()
	{

		SetSeatRelativeLocationAndRotationVR(FVector::ZeroVector);
		if (!FVector2D(NetSmoother->GetRelativeLocation()).Equals(FVector2D::ZeroVector))
		{
			NetSmoother->SetRelativeLocation(FVector(0.f, 0.f, NetSmoother->GetRelativeLocation().Z));
		}

		NotifyOfTeleport();

	}

	void TickSeatInformation(float DeltaTime);

	UFUNCTION()
		virtual void OnRep_SeatedCharInfo();

	void InitSeatedModeTransition();

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "BaseVRCharacter", meta = (DisplayName = "ReZeroSeating"))
		void Server_ReZeroSeating(FTransform_NetQuantize NewTargetTransform, FTransform_NetQuantize NewInitialRelCameraTransform, bool bZeroToHead = true);

	UFUNCTION(Reliable, Server, WithValidation)
		void Server_SeatedSnapTurn(float Yaw);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "BaseVRCharacter", meta = (DisplayName = "SetSeatedMode"))
		void Server_SetSeatedMode(USceneComponent * SeatParent, bool bSetSeatedMode, FTransform_NetQuantize TargetTransform, FTransform_NetQuantize InitialRelCameraTransform, float AllowedRadius = 40.0f, float AllowedRadiusThreshold = 20.0f, bool bZeroToHead = true, EVRConjoinedMovementModes PostSeatedMovementMode = EVRConjoinedMovementModes::C_MOVE_Walking);

	bool SetSeatedMode(USceneComponent * SeatParent, bool bSetSeatedMode, FTransform TargetTransform, FTransform InitialRelCameraTransform, float AllowedRadius = 40.0f, float AllowedRadiusThreshold = 20.0f, bool bZeroToHead = true, EVRConjoinedMovementModes PostSeatedMovementMode = EVRConjoinedMovementModes::C_MOVE_Walking);

	void SetSeatRelativeLocationAndRotationVR(FVector LocDelta);

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacter|VRLocations")
		FVector AddActorWorldRotationVR(FRotator DeltaRot, bool bUseYawOnly = true, bool bRotateAroundCapsule = true);

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacter|VRLocations")
		FVector SetActorRotationVR(FRotator NewRot, bool bUseYawOnly = true, bool bAccountForHMDRotation = true, bool bRotateAroundCapsule = true);

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacter|VRLocations")
		FVector SetActorLocationAndRotationVR(FVector NewLoc, FRotator NewRot, bool bUseYawOnly = true, bool bAccountForHMDRotation = true, bool bTeleport = false, bool bRotateAroundCapsule = true);

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacter|VRLocations")
		FVector SetActorLocationVR(FVector NewLoc, bool bTeleport, bool bSetCapsuleLocation = true);

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacter|VRLocations")
	virtual void RegenerateOffsetComponentToWorld(bool bUpdateBounds, bool bCalculatePureYaw)
	{}

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacter")
		virtual void SetCharacterSizeVR(float NewRadius, float NewHalfHeight, bool bUpdateOverlaps = true);

	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacter")
		virtual void SetCharacterHalfHeightVR(float HalfHeight, bool bUpdateOverlaps = true);

	UPROPERTY(Category = VRBaseCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USceneComponent> NetSmoother;

	UPROPERTY(Category = VRBaseCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USceneComponent> VRProxyComponent;

	UPROPERTY(Category = VRBaseCharacter, VisibleAnywhere, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UVRBaseCharacterMovementComponent> VRMovementReference;

	UPROPERTY(Category = VRBaseCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UReplicatedVRCameraComponent> VRReplicatedCamera;

	UPROPERTY(Category = VRBaseCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UParentRelativeAttachmentComponent> ParentRelativeAttachment;

	UPROPERTY(Category = VRBaseCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UGripMotionControllerComponent> LeftMotionController;

	UPROPERTY(Category = VRBaseCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UGripMotionControllerComponent> RightMotionController;

	static FName LeftMotionControllerComponentName;

	static FName RightMotionControllerComponentName;

	static FName ReplicatedCameraComponentName;

	static FName ParentRelativeAttachmentComponentName;

	static FName SmoothingSceneParentComponentName;

	static FName VRProxyComponentName;

	UFUNCTION(BlueprintPure, Category = "VRGrip")
		virtual FVector GetTeleportLocation(FVector OriginalLocation);

	UFUNCTION(BlueprintCallable, Category = "VRGrip")
		virtual void NotifyOfTeleport(bool bRegisterAsTeleport = true);

	UFUNCTION(BlueprintNativeEvent, Category = "VRMovement")
		void OnCustomMoveActionPerformed(EVRMoveAction MoveActionType, FVector MoveActionVector, FRotator MoveActionRotator, uint8 MoveActionFlags);
	virtual void OnCustomMoveActionPerformed_Implementation(EVRMoveAction MoveActionType, FVector MoveActionVector, FRotator MoveActionRotator, uint8 MoveActionFlags);

	UFUNCTION(BlueprintNativeEvent, Category = "VRMovement")
		void OnBeginWallPushback(FHitResult HitResultOfImpact, bool bHadLocomotionInput, FVector HmdInput);
	virtual void OnBeginWallPushback_Implementation(FHitResult HitResultOfImpact, bool bHadLocomotionInput, FVector HmdInput);

	UFUNCTION(BlueprintNativeEvent, Category = "VRMovement")
		void OnEndWallPushback();
	virtual void OnEndWallPushback_Implementation();

	UFUNCTION(BlueprintImplementableEvent, Category = "VRBaseCharacter|Navigation")
		void ReceiveNavigationMoveCompleted(EPathFollowingResult::Type PathingResult);

	virtual void NavigationMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

	UFUNCTION(BlueprintCallable, Category = "VRBaseCharacter|Navigation")
	EPathFollowingStatus::Type GetMoveStatus() const;

	UFUNCTION(BlueprintCallable, Category = "VRBaseCharacter|Navigation")
	bool HasPartialPath() const;

	UFUNCTION(BlueprintCallable, Category = "VRBaseCharacter|Navigation")
	void StopNavigationMovement();

	UPROPERTY(BlueprintReadWrite, Category = AI)
		TSubclassOf<UNavigationQueryFilter> DefaultNavigationFilterClass;

	UFUNCTION(BlueprintCallable, Category = "VRBaseCharacter|Navigation", Meta = (AdvancedDisplay = "bStopOnOverlap,bCanStrafe,bAllowPartialPath"))
		virtual void ExtendedSimpleMoveToLocation(const FVector& GoalLocation, float AcceptanceRadius = -1, bool bStopOnOverlap = false,
			bool bUsePathfinding = true, bool bProjectDestinationToNavigation = true, bool bCanStrafe = false,
			TSubclassOf<UNavigationQueryFilter> FilterClass = NULL, bool bAllowPartialPath = true);

	UFUNCTION(BlueprintCallable, Category = "VRBaseCharacter|Navigation")
		bool GetCurrentNavigationPathPoints(TArray<FVector>& NavigationPointList);

};

USTRUCT()
struct FVRReplicatedCapsuleHeightNetSerializerConfig : public FNetSerializerConfig
{
	GENERATED_BODY()
};

namespace UE::Net
{
	UE_NET_DECLARE_SERIALIZER(FVRReplicatedCapsuleHeightNetSerializer, VREXPANSIONPLUGIN_API);
}

USTRUCT()
struct FVRSeatedCharacterInfoNetSerializerConfig : public FNetSerializerConfig
{
	GENERATED_BODY()
};

namespace UE::Net
{
	UE_NET_DECLARE_SERIALIZER(FVRSeatedCharacterInfoNetSerializer, VREXPANSIONPLUGIN_API);
}