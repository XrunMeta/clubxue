

#pragma once

#include "CoreMinimal.h"

#include "VRGripScriptBase.h"
#include "GripScripts/GS_Default.h"
#include "GS_GunTools.generated.h"

class UGripMotionControllerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRVirtualStockModeChangedSignature, bool, IsVirtualStockEngaged);

USTRUCT(BlueprintType, Category = "GunSettings")
struct VREXPANSIONPLUGIN_API FBPVirtualStockSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock")
		bool bUseDistanceBasedStockSnapping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock")
		float StockSnapDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock", meta = (ClampMin = "0.00", UIMin = "0.00"))
		float StockSnapLerpThreshold;

	UPROPERTY(BlueprintReadOnly, Category = "VirtualStock")
		float StockLerpValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock")
		FVector_NetQuantize100 StockSnapOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock")
		bool bAdjustZOfStockToPrimaryHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock|Smoothing")
		bool bSmoothStockHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock|Smoothing", meta = (editcondition = "bSmoothStockHand", ClampMin = "0.00", UIMin = "0.00", ClampMax = "1.00", UIMax = "1.00"))
		float SmoothingValueForStock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GunSettings|VirtualStock|Smoothing")
		FBPEuroLowPassFilterTrans StockHandSmoothing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GunSettings|VirtualStock|Debug")
	bool bDebugDrawVirtualStock;

	void CopyFrom(FBPVirtualStockSettings & B)
	{
		bUseDistanceBasedStockSnapping = B.bUseDistanceBasedStockSnapping;
		StockSnapDistance = B.StockSnapDistance;
		StockSnapLerpThreshold = B.StockSnapLerpThreshold;
		StockSnapOffset = B.StockSnapOffset;
		bAdjustZOfStockToPrimaryHand = B.bAdjustZOfStockToPrimaryHand;
		bSmoothStockHand = B.bSmoothStockHand;
		SmoothingValueForStock = B.SmoothingValueForStock;
		StockHandSmoothing = B.StockHandSmoothing;
	}

	FBPVirtualStockSettings()
	{
		StockSnapOffset = FVector(0.f, 0.f, 0.f);
		bAdjustZOfStockToPrimaryHand = true;
		StockSnapDistance = 35.f;
		StockSnapLerpThreshold = 20.0f;
		StockLerpValue = 0.0f;
		bUseDistanceBasedStockSnapping = true;
		SmoothingValueForStock = 0.0f;
		bSmoothStockHand = false;

		StockHandSmoothing.DeltaCutoff = 20.0f;
		StockHandSmoothing.MinCutoff = 5.0f;

		bDebugDrawVirtualStock = false;
	}
};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FGunTools_AdvSecondarySettings
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvSecondarySettings")
		bool bUseAdvancedSecondarySettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvSecondarySettings|Smoothing", meta = (editcondition = "bUseAdvancedSecondarySettings", ClampMin = "0.00", UIMin = "0.00", ClampMax = "1.00", UIMax = "1.00"))
		float SecondaryGripScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvSecondarySettings|Smoothing", meta = (editcondition = "bUseAdvancedSecondarySettings"))
		bool bUseConstantGripScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvSecondarySettings|Smoothing", meta = (editcondition = "bUseAdvancedSecondarySettings"))
		bool bUseGlobalSmoothingSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvSecondarySettings|Smoothing")
		FBPEuroLowPassFilter SecondarySmoothing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvSecondarySettings|DistanceInfluence", meta = (editcondition = "bUseAdvancedSecondarySettings"))
		bool bUseSecondaryGripDistanceInfluence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvSecondarySettings|DistanceInfluence", meta = (editcondition = "bUseSecondaryGripDistanceInfluence", ClampMin = "0.00", UIMin = "0.00", ClampMax = "256.00", UIMax = "256.00"))
		float GripInfluenceDeadZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvSecondarySettings|DistanceInfluence", meta = (editcondition = "bUseSecondaryGripDistanceInfluence", ClampMin = "1.00", UIMin = "1.00", ClampMax = "256.00", UIMax = "256.00"))
		float GripInfluenceDistanceToZero;

	FGunTools_AdvSecondarySettings()
	{
		bUseAdvancedSecondarySettings = false;
		SecondaryGripScaler = 0.0f;
		bUseGlobalSmoothingSettings = true;
		bUseSecondaryGripDistanceInfluence = false;

		GripInfluenceDeadZone = 50.0f;
		GripInfluenceDistanceToZero = 100.0f;
		bUseConstantGripScaler = false;
	}
};

UCLASS(NotBlueprintable, ClassGroup = (VRExpansionPlugin), hideCategories = TickSettings)
class VREXPANSIONPLUGIN_API UGS_GunTools : public UGS_Default
{
	GENERATED_BODY()
public:

	UGS_GunTools(const FObjectInitializer& ObjectInitializer);

	virtual void OnGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation) override;
	virtual void OnSecondaryGrip_Implementation(UGripMotionControllerComponent * Controller, USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation) override;
	virtual void OnBeginPlay_Implementation(UObject* CallingOwner) override;
	virtual void HandlePrePhysicsHandle(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation &GripInfo, FBPActorPhysicsHandleInformation* HandleInfo, FTransform& KinPose) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		FName WeaponRootOrientationComponent;
	FTransform OrientationComponentRelativeFacing;
	FQuat StoredRootOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GunSettings")
		bool bUseHighQualityRemoteSimulation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GunSettings")
	FGunTools_AdvSecondarySettings AdvSecondarySettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pivot")
		FVector_NetQuantize100 PivotOffset;

	UFUNCTION(BlueprintCallable, Category = "VirtualStock")
		void SetVirtualStockComponent(USceneComponent * NewStockComponent)
	{
		VirtualStockComponent = NewStockComponent;
	}

	UFUNCTION(BlueprintCallable, Category = "VirtualStock")
		void SetVirtualStockEnabled(bool bAllowVirtualStock)
	{
		if (!bUseVirtualStock && bAllowVirtualStock)
			ResetStockVariables();

		bUseVirtualStock = bAllowVirtualStock;
	}

	void ResetStockVariables()
	{
		VirtualStockSettings.StockHandSmoothing.ResetSmoothingFilter();
	}

	void GetVirtualStockTarget(UGripMotionControllerComponent * GrippingController);

	UPROPERTY(BlueprintAssignable, Category = "VirtualStock")
		FVRVirtualStockModeChangedSignature OnVirtualStockModeChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock")
		bool bUseVirtualStock;

	FTransform MountWorldTransform;
	bool bIsMounted;
	FTransform RelativeTransOnSecondaryRelease;
	TObjectPtr<USceneComponent> CameraComponent;

	UPROPERTY(BlueprintReadWrite, Category = "VirtualStock")
		TObjectPtr<USceneComponent> VirtualStockComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock")
		bool bUseGlobalVirtualStockSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VirtualStock", meta = (editcondition = "!bUseGlobalVirtualStockSettings"))
		FBPVirtualStockSettings VirtualStockSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
		bool bHasRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
		bool bApplyRecoilAsPhysicalForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (editcondition = "bHasRecoil"))
		FVector_NetQuantize100 MaxRecoilTranslation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (editcondition = "bHasRecoil"))
		FVector_NetQuantize100 MaxRecoilRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (editcondition = "bHasRecoil"))
		FVector_NetQuantize100 MaxRecoilScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (editcondition = "bHasRecoil"))
		float DecayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (editcondition = "bHasRecoil"))
		float LerpRate;

	FTransform BackEndRecoilStorage;

	FTransform BackEndRecoilTarget;

	bool bHasActiveRecoil;

	UFUNCTION(BlueprintCallable, Category = "Recoil")
		void AddRecoilInstance(const FTransform & RecoilAddition, FVector Optional_Location = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "Recoil")
		void ResetRecoil();

	virtual bool GetWorldTransform_Implementation(UGripMotionControllerComponent * GrippingController, float DeltaTime, FTransform & WorldTransform, const FTransform &ParentTransform, FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport) override;

	static void ApplyTwoHandModifier(FTransform & OriginalTransform)
	{

	}

	inline FVector GunTools_ApplySmoothingAndLerp(FBPActorGripInformation & Grip, FVector &frontLoc, FVector & frontLocOrig, float DeltaTime, bool bSkipHighQualitySimulations)
	{
		FVector SmoothedValue = frontLoc;

		if (Grip.SecondaryGripInfo.GripLerpState == EGripLerpState::StartLerp) 
		{
			if (!bSkipHighQualitySimulations && AdvSecondarySettings.SecondaryGripScaler < 1.0f)
			{
				SmoothedValue = AdvSecondarySettings.SecondarySmoothing.RunFilterSmoothing(frontLoc, DeltaTime);
				frontLoc = FMath::Lerp(frontLoc, SmoothedValue, AdvSecondarySettings.SecondaryGripScaler);

			}

		}
		else if (!bSkipHighQualitySimulations && AdvSecondarySettings.bUseAdvancedSecondarySettings && AdvSecondarySettings.bUseConstantGripScaler) 
		{
			SmoothedValue = AdvSecondarySettings.SecondarySmoothing.RunFilterSmoothing(frontLoc, DeltaTime);
			frontLoc = FMath::Lerp(frontLoc, SmoothedValue, AdvSecondarySettings.SecondaryGripScaler);
		}

		return SmoothedValue;
	}
};

