

#pragma once
#include "CoreMinimal.h"
#include "VRBaseCharacter.h"
#include "VRCharacter.generated.h"

class UVRRootComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogVRCharacter, Log, All);

UCLASS()
class VREXPANSIONPLUGIN_API AVRCharacter : public AVRBaseCharacter
{
	GENERATED_BODY()

public:
	AVRCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool TeleportTo(const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest = false, bool bNoCheck = false) override;

	UPROPERTY(Category = VRCharacter, VisibleAnywhere, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))	
		TObjectPtr<UVRRootComponent> VRRootReference;

	virtual FVector GetTargetHeightOffset() override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void RegenerateOffsetComponentToWorld(bool bUpdateBounds, bool bCalculatePureYaw) override;
	virtual void SetCharacterSizeVR(float NewRadius, float NewHalfHeight, bool bUpdateOverlaps = true) override;
	virtual void SetCharacterHalfHeightVR(float HalfHeight, bool bUpdateOverlaps = true) override;

	virtual FVector GetTeleportLocation(FVector OriginalLocation) override;

	virtual FVector GetProjectedVRLocation() const override;

	virtual void ZeroToSeatInformation() override;

	FVector GetNavAgentLocation() const override;

		virtual void ExtendedSimpleMoveToLocation(const FVector& GoalLocation, float AcceptanceRadius = -1, bool bStopOnOverlap = false,
			bool bUsePathfinding = true, bool bProjectDestinationToNavigation = true, bool bCanStrafe = false,
			TSubclassOf<UNavigationQueryFilter> FilterClass = NULL, bool bAllowPartialPath = true) override;

};