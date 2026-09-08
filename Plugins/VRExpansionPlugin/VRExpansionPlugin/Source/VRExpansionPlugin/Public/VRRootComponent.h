

#pragma once
#include "CoreMinimal.h"

#include "VRTrackedParentInterface.h"
#include "VRBaseCharacter.h"
#include "VRExpansionFunctionLibrary.h"
#include "GameFramework/PhysicsVolume.h"
#include "Components/CapsuleComponent.h"
#include "VRRootComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVRRootComponent, Log, All);

DECLARE_STATS_GROUP(TEXT("VRRootComponent"), STATGROUP_VRRootComponent, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("VR Root Set Half Height"), STAT_VRRootSetHalfHeight, STATGROUP_VRRootComponent);
DECLARE_CYCLE_STAT(TEXT("VR Root Set Capsule Size"), STAT_VRRootSetCapsuleSize, STATGROUP_VRRootComponent);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = VRExpansionLibrary)
class VREXPANSIONPLUGIN_API UVRRootComponent : public UCapsuleComponent, public IVRTrackedParentInterface
{
	GENERATED_BODY()

public:
	UVRRootComponent(const FObjectInitializer& ObjectInitializer);

	friend class FDrawCylinderSceneProxy;

	bool bCalledUpdateTransform;

	virtual void GetNavigationData(FNavigationRelevantData& Data) const override;

	FORCEINLINE void GenerateOffsetToWorld(bool bUpdateBounds = true, bool bGetPureYaw = true);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRTrackedParentInterface")
		FBPVRWaistTracking_Info OptionalWaistTrackingParent;

	virtual void SetTrackedParent(UPrimitiveComponent * NewParentComponent, float WaistRadius, EBPVRWaistTrackingMode WaistTrackingMode) override
	{
		IVRTrackedParentInterface::Default_SetTrackedParent_Impl(NewParentComponent, WaistRadius, WaistTrackingMode, OptionalWaistTrackingParent, this);
	}

	inline FVector GetTargetHeightOffset()
	{

		if (bCenterCapsuleOnHMD)
		{
			return FVector(0.f, 0.f, (-VRCapsuleOffset.Z) - curCameraLoc.Z);
		}
		else
		{
			return FVector(0.f, 0.f, (-this->GetUnscaledCapsuleHalfHeight()) - VRCapsuleOffset.Z);
		}
	}

	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		virtual void SetCapsuleSizeVR(float NewRadius, float NewHalfHeight, bool bUpdateOverlaps = true);

	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		void SetCapsuleHalfHeightVR(float HalfHeight, bool bUpdateOverlaps = true);

	inline void OnUpdateTransform_Public(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None)
	{
		OnUpdateTransform(UpdateTransformFlags, Teleport);
		if (bNavigationRelevant && bRegistered)
		{
			UpdateNavigationData();
			PostUpdateNavigationData();
		}
	}

	virtual void SetSimulatePhysics(bool bSimulate) override;

protected:
	virtual bool MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit = NULL, EMoveComponentFlags MoveFlags = MOVECOMP_NoFlags, ETeleportType Teleport = ETeleportType::None) override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;

	void SendPhysicsTransform(ETeleportType Teleport);
	virtual bool UpdateOverlapsImpl(const TOverlapArrayView* NewPendingOverlaps = nullptr, bool bDoNotifies = true, const TOverlapArrayView* OverlapsAtEndLocation = nullptr) override;

	template<typename AllocatorType>
	bool ConvertRotationOverlapsToCurrentOverlapsVR(TArray<FOverlapInfo, AllocatorType>& OutOverlapsAtEndLocation, const TOverlapArrayView& CurrentOverlaps);

	template<typename AllocatorType>
	bool GetOverlapsWithActor_TemplateVR(const AActor* Actor, TArray<FOverlapInfo, AllocatorType>& OutOverlaps) const;

	template<typename AllocatorType>
	bool ConvertSweptOverlapsToCurrentOverlapsVR(TArray<FOverlapInfo, AllocatorType>& OutOverlapsAtEndLocation, const TOverlapArrayView& SweptOverlaps, int32 SweptOverlapsIndex, const FVector& EndLocation, const FQuat& EndRotationQuat);

public:
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;

	bool IsLocallyControlled() const;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "VRExpansionLibrary")
		TObjectPtr<USceneComponent> TargetPrimitiveComponent;

	TObjectPtr<AVRBaseCharacter> owningVRChar;

	FVector DifferenceFromLastFrame;

	FTransform OffsetComponentToWorld;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
	FVector VRCapsuleOffset;

	float LastCapsuleHalfHeight = 0.0f;

	void UpdateCharacterCapsuleOffset();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		bool bPauseTracking;

	UFUNCTION(BlueprintCallable, Category = "VRExpansionLibrary")
		void SetTrackingPaused(bool bPaused);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
	bool bCenterCapsuleOnHMD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		bool bAllowSimulatingCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
	bool bUseWalkingCollisionOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
	TEnumAsByte<ECollisionChannel> WalkingCollisionOverride;

	bool bIsOverridingCollision = false;
	TEnumAsByte<ECollisionChannel> OriginalCollision = ECollisionChannel::ECC_Pawn;

	void SetCollisionOverride(bool bOverrideCollision)
	{
		if (bOverrideCollision && !bIsOverridingCollision)
		{
			OriginalCollision = this->GetCollisionObjectType();
			SetCollisionObjectType(WalkingCollisionOverride);
			bIsOverridingCollision = true;
		}
		else if (!bOverrideCollision && bIsOverridingCollision)
		{
			SetCollisionObjectType(OriginalCollision);
			bIsOverridingCollision = false;
		}
	}

	FVector curCameraLoc;
	FRotator curCameraRot;
	FRotator StoredCameraRotOffset;

	FVector lastCameraLoc = FVector::ZeroVector;
	FRotator lastCameraRot = FRotator::ZeroRotator;
	bool bTickedOnce = false;

	UPROPERTY(BlueprintReadOnly, Category = "VRExpansionLibrary")
	bool bHadRelativeMovement;

	FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void UpdatePhysicsVolume(bool bTriggerNotifiers) override;

	inline bool AreWeOverlappingVolume(APhysicsVolume* V)
	{
		bool bInsideVolume = true;
		if (!V->bPhysicsOnContact)
		{
			FVector ClosestPoint(0.f);

			UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(V->GetRootComponent());
			if (RootPrimitive)
			{
				float DistToCollisionSqr = -1.f;
				if (RootPrimitive->GetSquaredDistanceToCollision(OffsetComponentToWorld.GetTranslation(), DistToCollisionSqr, ClosestPoint))
				{
					bInsideVolume = (DistToCollisionSqr == 0.f);
				}
				else
				{
					bInsideVolume = false;
				}
			}
		}

		return bInsideVolume;
	}

	inline float GetLineThickness() const {return LineThickness;}

public:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void PreEditChange(FProperty* PropertyThatWillChange);
#endif 

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	private:
		friend class FVRCharacterScopedMovementUpdate;
};

void inline UVRRootComponent::GenerateOffsetToWorld(bool bUpdateBounds, bool bGetPureYaw)
{
	FRotator CamRotOffset;

	if (bGetPureYaw)
		CamRotOffset = StoredCameraRotOffset;
	else
		CamRotOffset = curCameraRot;

	if(owningVRChar && !owningVRChar->bRetainRoomscale)
	{
		OffsetComponentToWorld = FTransform(CamRotOffset.Quaternion(), FVector(0.0f, 0.0f, 0.0f), FVector(1.0f)) * GetComponentTransform();
	}
	else
	{
		OffsetComponentToWorld = FTransform(CamRotOffset.Quaternion(), FVector(curCameraLoc.X, curCameraLoc.Y, bCenterCapsuleOnHMD ? curCameraLoc.Z : CapsuleHalfHeight) + CamRotOffset.RotateVector(VRCapsuleOffset), FVector(1.0f)) * GetComponentTransform();
	}

	if (owningVRChar)
	{
		owningVRChar->OffsetComponentToWorld = OffsetComponentToWorld;

		UpdateCharacterCapsuleOffset();
	}

	if (bUpdateBounds)
		UpdateBounds();
}

FORCEINLINE void UVRRootComponent::SetCapsuleHalfHeightVR(float HalfHeight, bool bUpdateOverlaps)
{
	SCOPE_CYCLE_COUNTER(STAT_VRRootSetHalfHeight);

	if (FMath::IsNearlyEqual(HalfHeight, CapsuleHalfHeight))
	{
		return;
	}

	SetCapsuleSizeVR(GetUnscaledCapsuleRadius(), HalfHeight, bUpdateOverlaps);
}