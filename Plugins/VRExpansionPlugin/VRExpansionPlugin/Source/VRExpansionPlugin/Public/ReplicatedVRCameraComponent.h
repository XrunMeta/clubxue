

#pragma once
#include "CoreMinimal.h"
#include "VRBPDatatypes.h"

#include "Camera/CameraComponent.h"
#include "ReplicatedVRCameraComponent.generated.h"

class AVRBaseCharacter;
class AVRCharacter;

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = VRExpansionLibrary)
class VREXPANSIONPLUGIN_API UReplicatedVRCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UReplicatedVRCameraComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		bool bUpdateInCharacterMovement;

	UPROPERTY()
		TObjectPtr<AVRCharacter> AttachChar;
	void UpdateTracking(float DeltaTime);

	virtual void OnAttachmentChanged() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	bool bHasAuthority;

	bool bIsServer;

	FTransform LastRelativePosition;
	bool bHadValidFirstVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|ComponentVelocity")
		bool bSampleVelocityInWorldSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera")
		bool bFPSDebugMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera")
	bool bSetPositionDuringTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Advanced|Tracking")
		bool bScaleTracking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Advanced|Tracking", meta = (ClampMin = "0.1", UIMin = "0.1", EditCondition = "bScaleTracking"))
		FVector TrackingScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Advanced|Tracking")
		bool bLimitMinHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Advanced|Tracking", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bLimitMinHeight"))
		float MinimumHeightAllowed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Advanced|Tracking")
		bool bLimitMaxHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Advanced|Tracking", meta = (ClampMin = "0.1", UIMin = "0.1", EditCondition = "bLimitMaxHeight"))
		float MaxHeightAllowed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Advanced|Tracking")
		bool bLimitBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Advanced|Tracking", meta = (ClampMin = "0.1", UIMin = "0.1", EditCondition = "bLimitBounds"))
		float MaximumTrackedBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Advanced|Tracking")
		uint32 bAutoSetLockToHmd : 1;

	void ApplyTrackingParameters(FVector & OriginalPosition, bool bSkipLocZero = false);
	bool HasTrackingParameters();

	virtual void HandleXRCamera(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedCameraTransform, Category = "ReplicatedCamera|Networking")
	FBPVRComponentPosRep ReplicatedCameraTransform;

	UFUNCTION(BlueprintPure, Category = "ReplicatedCamera|Tracking")
		FTransform GetHMDTrackingTransform();

	FVector LastUpdatesRelativePosition = FVector::ZeroVector;
	FRotator LastUpdatesRelativeRotation = FRotator::ZeroRotator;

	bool bLerpingPosition;
	bool bReppedOnce;

	void RunNetworkedSmoothing(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera|Networking")
		bool bSmoothReplicatedMotion = true;

	UPROPERTY(EditAnywhere, Category = "ReplicatedCamera|Networking|Smoothing", meta = (editcondition = "bSmoothReplicatedMotion"))
		bool bUseExponentialSmoothing = true;

	UPROPERTY(EditAnywhere, Category = "ReplicatedCamera|Networking|Smoothing", meta = (editcondition = "bUseExponentialSmoothing"))
		float InterpolationSpeed = 25.0f;

	UPROPERTY(EditAnywhere, Category = "ReplicatedCamera|Networking|Smoothing", meta = (editcondition = "bUseExponentialSmoothing"))
		float NetworkMaxSmoothUpdateDistance = 50.f;

	UPROPERTY(EditAnywhere, Category = "ReplicatedCamera|Networking|Smoothing", meta = (editcondition = "bUseExponentialSmoothing"))
		float NetworkNoSmoothUpdateDistance = 100.f;

	UFUNCTION()
    virtual void OnRep_ReplicatedCameraTransform();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "ReplicatedCamera|Networking")
	float NetUpdateRate;
public:

	float NetUpdateCount;

	float GetNetUpdateRate() { return NetUpdateRate; }
	void SetNetUpdateRate(float NewNetUpdateRate);

	UFUNCTION(Unreliable, Server, WithValidation)
	void Server_SendCameraTransform(FBPVRComponentPosRep NewTransform);

	typedef void (AVRBaseCharacter::*VRBaseCharTransformRPC_Pointer)(FBPVRComponentPosRep NewTransform);
	VRBaseCharTransformRPC_Pointer OverrideSendTransform;

	bool IsLocallyControlled() const;

};