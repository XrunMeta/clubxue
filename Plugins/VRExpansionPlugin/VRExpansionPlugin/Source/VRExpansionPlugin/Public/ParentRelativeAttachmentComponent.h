

#pragma once

#include "CoreMinimal.h"

#include "VRExpansionFunctionLibrary.h"
#include "Components/ShapeComponent.h"
#include "VRTrackedParentInterface.h"
#include "ParentRelativeAttachmentComponent.generated.h"

class AVRBaseCharacter;
class AVRCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVRPRCBeginYawRotationEventSignature);

UENUM(BlueprintType)
enum class EVR_PRC_RotationMethod : uint8
{

	PRC_ROT_HMD UMETA(DisplayName = "HMD rotation"),

	PRC_ROT_HMDControllerBlend UMETA(DisplayName = "ROT HMD Controller Blend"),

	PRC_ROT_ControllerHMDClamped UMETA(DisplayName = "Controller Clamped to HMD")
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = VRExpansionLibrary)
class VREXPANSIONPLUGIN_API UParentRelativeAttachmentComponent : public USceneComponent, public IVRTrackedParentInterface
{
	GENERATED_BODY()

public:
	UParentRelativeAttachmentComponent(const FObjectInitializer& ObjectInitializer);
	virtual void InitializeComponent() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary", meta = (ClampMin = "0", UIMin = "0"))
		float YawTolerance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary", meta = (ClampMin = "0", UIMin = "0"))
		float LerpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		bool bLerpTransition;

	float LastRot;
	float LastLerpVal;
	float LerpTarget;
	bool bWasSetOnce;
	FTransform LeftControllerTrans;
	FTransform RightControllerTrans;

	UPROPERTY(BlueprintAssignable, Category = "PRC Events")
		FVRPRCBeginYawRotationEventSignature OnYawToleranceExceeded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
	bool bUseFeetLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary", meta = (EditCondition = "bUseFeetLocation"))
		bool bUseCenterAsFeetLocation = false; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		FVector CustomOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
	bool bIgnoreRotationFromParent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		bool bUpdateInCharacterMovement;

private:
	UPROPERTY()
		bool bIsPaused;
public:

	UFUNCTION(BlueprintCallable, Category = "VRExpansionLibrary")
		void SetPaused(bool bNewPaused, bool bZeroOutRotation, bool bZeroOutLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRTrackedParentInterface")
	FBPVRWaistTracking_Info OptionalWaistTrackingParent;

	virtual void SetTrackedParent(UPrimitiveComponent * NewParentComponent, float WaistRadius, EBPVRWaistTrackingMode WaistTrackingMode) override
	{
		IVRTrackedParentInterface::Default_SetTrackedParent_Impl(NewParentComponent, WaistRadius, WaistTrackingMode, OptionalWaistTrackingParent, this);
	}

	UPROPERTY()
		TObjectPtr<AVRCharacter> AttachChar;
	UPROPERTY()
		TObjectPtr<AVRBaseCharacter> AttachBaseChar;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void OnAttachmentChanged() override;
	void UpdateTracking(float DeltaTime);

	bool IsLocallyControlled() const
	{

		const AActor* MyOwner = GetOwner();
		return MyOwner->HasLocalNetOwner();

	}

	inline void SetRelativeRotAndLoc(FVector NewRelativeLocation, FRotator NewRelativeRotation, float DeltaTime);

	FQuat GetCalculatedRotation(FRotator InverseRot, float DeltaTime)
	{
		FRotator FinalRot = FRotator::ZeroRotator;
		if (FPlatformMath::Abs(FMath::FindDeltaAngleDegrees(InverseRot.Yaw, LastRot)) < YawTolerance)	
		{
			if (!bWasSetOnce)
			{
				LastRot = FRotator::ClampAxis(InverseRot.Yaw);
				LastLerpVal = LastRot;
				LerpTarget = LastRot;
				bWasSetOnce = true;
			}

			if (bLerpTransition && !FMath::IsNearlyEqual(LastLerpVal, LerpTarget))
			{
				LastLerpVal = FMath::FixedTurn(LastLerpVal, LerpTarget, LerpSpeed * DeltaTime);
				FinalRot = FRotator(0, LastLerpVal, 0);
			}
			else
			{
				FinalRot = FRotator(0, LastRot, 0);
				LastLerpVal = LastRot;
			}
		}
		else
		{

			if (!FMath::IsNearlyZero(YawTolerance))
			{
				LerpTarget = FRotator::ClampAxis(InverseRot.Yaw);
				LastLerpVal = FMath::FixedTurn(LastLerpVal, LerpTarget, LerpSpeed * DeltaTime);
				FinalRot = FRotator(0, LastLerpVal, 0);
				OnYawToleranceExceeded.Broadcast();
			}
			else 
			{
				FinalRot = FRotator(0, FRotator::ClampAxis(InverseRot.Yaw), 0);
			}

			LastRot = FRotator::ClampAxis(InverseRot.Yaw);
		}

		return FinalRot.Quaternion();
	}

	void RunSampling(FRotator &HMDRotation, FVector &HMDLocation)
	{

	}

	void GetEstShoulderRotation(FRotator &InputHMDRotation, FVector &InputHMDLocation)
	{

	}

	void DetectHandsBehindHead(float& TargetRot, FRotator HMDRotation)
	{

	}

	void ClampHeadRotationDelta(float& TargetRotation, FRotator HMDRotation)
	{

	}
};

