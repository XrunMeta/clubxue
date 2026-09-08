#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"
#include "VRBPDatatypes.h"
#include "Curves/CurveFloat.h"
#include "GripScripts/GS_Melee.h"
#include "GripScripts/GS_GunTools.h"
#include "VRGlobalSettings.generated.h"

class UGrippableSkeletalMeshComponent;

USTRUCT(BlueprintType, Category = "ControllerProfiles")
struct VREXPANSIONPLUGIN_API FBPVRControllerProfile
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControllerProfiles")
		FName ControllerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControllerProfiles")
		FTransform_NetQuantize SocketOffsetTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControllerProfiles")
		bool bUseSeperateHandOffsetTransforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControllerProfiles", meta = (editcondition = "bUseSeperateHandOffsetTransforms"))
		FTransform_NetQuantize SocketOffsetTransformRightHand;

	FBPVRControllerProfile() :
		ControllerName(NAME_None),
		SocketOffsetTransform(FTransform::Identity),
		bUseSeperateHandOffsetTransforms(false),
		SocketOffsetTransformRightHand(FTransform::Identity)
	{}

	FBPVRControllerProfile(FName ControllerName) :
		ControllerName(ControllerName),
		SocketOffsetTransform(FTransform::Identity),		
		bUseSeperateHandOffsetTransforms(false),
		SocketOffsetTransformRightHand(FTransform::Identity)
	{}

	FBPVRControllerProfile(FName ControllerNameIn, const FTransform & Offset) :
		ControllerName(ControllerNameIn),
		SocketOffsetTransform(Offset),
		bUseSeperateHandOffsetTransforms(false),
		SocketOffsetTransformRightHand(FTransform::Identity)
	{}

	FBPVRControllerProfile(FName ControllerNameIn, const FTransform & Offset, const FTransform & OffsetRight) :
		ControllerName(ControllerNameIn),
		SocketOffsetTransform(Offset),
		bUseSeperateHandOffsetTransforms(true),
		SocketOffsetTransformRightHand(OffsetRight)
	{}

	FORCEINLINE bool operator==(const FBPVRControllerProfile &Other) const
	{
		return this->ControllerName == Other.ControllerName;
	}
};

UCLASS(config = Engine, defaultconfig)
class VREXPANSIONPLUGIN_API UVRGlobalSettings : public UObject
{
	GENERATED_BODY()

public:
	UVRGlobalSettings(const FObjectInitializer& ObjectInitializer);

	void SetScalers();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "Misc")
		TSubclassOf<class UGrippableSkeletalMeshComponent> DefaultGrippableCharacterMeshComponentClass;

	static TSubclassOf<class UGrippableSkeletalMeshComponent> GetDefaultGrippableCharacterMeshComponentClass();

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics|CollisionIgnore")
		bool bUseCollisionModificationForCollisionIgnore;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics|CollisionIgnore")
		float CollisionIgnoreSubsystemUpdateRate;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics")
		bool bUseChaosTranslationScalers; 

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics")
		bool bSetEngineChaosScalers;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics")
		float LinearDriveStiffnessScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics")
		float LinearDriveDampingScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics")
		float AngularDriveStiffnessScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics")
		float AngularDriveDampingScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics|Constraints")
		float JointStiffness;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics|Constraints")
		float SoftLinearStiffnessScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics|Constraints")
		float SoftLinearDampingScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics|Constraints")
		float SoftAngularStiffnessScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics|Constraints")
		float SoftAngularDampingScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics|Constraints")
		float JointLinearBreakScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "ChaosPhysics|Constraints")
		float JointAngularBreakScale;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "HybridWithSweepLerp")
		bool bLerpHybridWithSweepGrips;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "HybridWithSweepLerp")
		bool bOnlyLerpHybridRotation;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "HybridWithSweepLerp")
		bool bHybridWithSweepUseDistanceBasedLerp;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "HybridWithSweepLerp")
		float HybridWithSweepLerpDuration;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "GlobalLerpToHand")
		bool bUseGlobalLerpToHand;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "GlobalLerpToHand")
		bool bSkipLerpToHandIfHeld;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "GlobalLerpToHand")
		float MinDistanceForLerp;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "GlobalLerpToHand")
		float LerpDuration;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "GlobalLerpToHand")
		float MinSpeedForLerp;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "GlobalLerpToHand")
		float MaxSpeedForLerp;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "GlobalLerpToHand")
		EVRLerpInterpolationMode LerpInterpolationMode;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "GlobalLerpToHand|Curve")
		bool bUseCurve;

	UPROPERTY(config, Category = "GlobalLerpToHand|Curve", EditAnywhere, meta = (editcondition = "bUseCurve"))
		FRuntimeFloatCurve OptionalCurveToFollow;

	UFUNCTION(BlueprintPure, Category = "GlobalLerpToHand")
		static bool IsGlobalLerpEnabled();

	UPROPERTY(config, EditAnywhere, Category = "MeleeSettings")
		TArray<FBPHitSurfaceProperties> MeleeSurfaceSettings;

	UPROPERTY(config, EditAnywhere, Category = "GunSettings")
		FBPVirtualStockSettings VirtualStockSettings;

	UPROPERTY(config, EditAnywhere, Category = "GunSettings|Secondary Grip 1Euro Settings")
		float OneEuroMinCutoff;

	UPROPERTY(config, EditAnywhere, Category = "GunSettings|Secondary Grip 1Euro Settings")
		float OneEuroCutoffSlope;

	UPROPERTY(config, EditAnywhere, Category = "GunSettings|Secondary Grip 1Euro Settings")
		float OneEuroDeltaCutoff;

	UFUNCTION(BlueprintCallable, Category = "MeleeSettings")
		static void GetMeleeSurfaceGlobalSettings(TArray<FBPHitSurfaceProperties>& OutMeleeSurfaceSettings);

	UFUNCTION(BlueprintCallable, Category = "GunSettings|VirtualStock")
		static void GetVirtualStockGlobalSettings(FBPVirtualStockSettings& OutVirtualStockSettings);

	UFUNCTION(BlueprintCallable, Category = "GunSettings|VirtualStock")
		static void SaveVirtualStockGlobalSettings(FBPVirtualStockSettings NewVirtualStockSettings);

	DECLARE_MULTICAST_DELEGATE(FVRControllerProfileChangedEvent);

	FVRControllerProfileChangedEvent OnControllerProfileChangedEvent;

	UPROPERTY(config, EditAnywhere, Category = "ControllerProfiles")
	TArray<FBPVRControllerProfile> ControllerProfiles;

	FName CurrentControllerProfileInUse;
	FTransform CurrentControllerProfileTransform;
	bool bUseSeperateHandTransforms;
	FTransform CurrentControllerProfileTransformRight;

	UFUNCTION(BlueprintPure, Category = "VRControllerProfiles")
		static FTransform AdjustTransformByControllerProfile(FName OptionalControllerProfileName, const FTransform& SocketTransform, bool bIsRightHand = false);

	UFUNCTION(BlueprintPure, Category = "VRControllerProfiles")
		static FTransform AdjustTransformByGivenControllerProfile(UPARAM(ref) FBPVRControllerProfile& ControllerProfile, const FTransform& SocketTransform, bool bIsRightHand = false);

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles")
		static TArray<FBPVRControllerProfile> GetControllerProfiles();

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles|Operations")
		static void OverwriteControllerProfile(UPARAM(ref)FBPVRControllerProfile& OverwritingProfile, bool bSaveOutToConfig = true);

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles|Operations")
		static void AddControllerProfile(UPARAM(ref)FBPVRControllerProfile& NewProfile, bool bSaveOutToConfig = true);

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles|Operations")
		static void DeleteControllerProfile(FName ControllerProfileName, bool bSaveOutToConfig = true);

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles|Operations")
		static void SaveControllerProfiles();

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles")
		static FName GetCurrentProfileName(bool& bHadLoadedProfile);

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles")
		static FBPVRControllerProfile GetCurrentProfile(bool& bHadLoadedProfile);

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles")
		static bool GetControllerProfile(FName ControllerProfileName, FBPVRControllerProfile& OutProfile);

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles")
		static bool LoadControllerProfileByName(FName ControllerProfileName, bool bSetAsCurrentProfile = true);

	UFUNCTION(BlueprintCallable, Category = "VRControllerProfiles")
		static bool LoadControllerProfile(const FBPVRControllerProfile& ControllerProfile, bool bSetAsCurrentProfile = true);

	virtual void PostInitProperties() override;
};
