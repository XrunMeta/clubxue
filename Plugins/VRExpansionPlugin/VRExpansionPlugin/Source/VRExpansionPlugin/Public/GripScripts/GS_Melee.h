#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "VRGripScriptBase.h"
#include "GripScripts/GS_Default.h"
#include "GS_Melee.generated.h"

UENUM(BlueprintType)
enum class EVRMeleeZoneType : uint8
{

	VRPMELLE_ZONETYPE_Stab UMETA(DisplayName = "Stab"),

	VRPMELLE_ZONETYPE_Hit UMETA(DisplayName = "Hit"),

	VRPMELLE_ZONETYPE_StabAndHit UMETA(DisplayName = "StabAndHit")

};

UENUM(BlueprintType)
enum class EVRMeleeComType : uint8
{

	VRPMELEECOM_Normal UMETA(DisplayName = "Normal"),

	VRPMELEECOM_BetweenHands UMETA(DisplayName = "BetweenHands"),

	VRPMELEECOM_PrimaryHand  UMETA(DisplayName = "PrimaryHand")
};

UENUM(BlueprintType)
enum class EVRMeleePrimaryHandType : uint8
{

	VRPHAND_Rear UMETA(DisplayName = "Rear"),

	VRPHAND_Front  UMETA(DisplayName = "Front"),

	VRPHAND_Slotted UMETA(DisplayName = "Slotted")
};

USTRUCT(BlueprintType, Category = "Lodging")
struct VREXPANSIONPLUGIN_API FBPHitSurfaceProperties
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Property")
		bool bSurfaceAllowsPenetration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Property")
		float BluntDamageScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Property")
		float SharpDamageScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Property")
		float StabVelocityScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Property")
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	FBPHitSurfaceProperties()
	{

		bSurfaceAllowsPenetration = true;
		BluntDamageScaler = 1.f;
		SharpDamageScaler = 1.f;
		StabVelocityScaler = 1.f;
		SurfaceType = EPhysicalSurface::SurfaceType_Default;
	}
};

USTRUCT(BlueprintType, Category = "Lodging")
struct VREXPANSIONPLUGIN_API FBPLodgeComponentInfo
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LodgeComponentInfo")
		FName ComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LodgeComponentInfo")
		EVRMeleeZoneType ZoneType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LodgeComponentInfo")
		bool bIgnoreForwardVectorForHitImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LodgeComponentInfo")
		float DamageScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LodgeComponentInfo")
		float PenetrationDepth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LodgeComponentInfo")
		bool bAllowPenetrationInReverseAsWell;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		float PenetrationVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		float MinimumHitVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		float AcceptableForwardProductRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		float AcceptableForwardProductRangeForHits;

	FBPLodgeComponentInfo()
	{
		ComponentName = NAME_None;
		ZoneType = EVRMeleeZoneType::VRPMELLE_ZONETYPE_StabAndHit;
		bIgnoreForwardVectorForHitImpulse = false;
		DamageScaler = 0.f;
		PenetrationDepth = 100.f;
		bAllowPenetrationInReverseAsWell = false;
		PenetrationVelocity = 8000.f;
		MinimumHitVelocity = 1000.f;
		AcceptableForwardProductRange = 0.1f;
		AcceptableForwardProductRangeForHits = 0.1f;
		TargetComponent = nullptr;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LodgeComponentInfo")
	TObjectPtr<UPrimitiveComponent> TargetComponent;

	FORCEINLINE bool operator==(const FName& Other) const
	{
		return (ComponentName == Other);
	}

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(FVROnMeleeShouldLodgeSignature, FBPLodgeComponentInfo, LogComponent, AActor *, OtherActor, UPrimitiveComponent *, OtherComp, ECollisionChannel, OtherCompCollisionChannel, FBPHitSurfaceProperties, HitSurfaceProperties, FVector, NormalImpulse, const FHitResult&, Hit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(FVROnMeleeOnHit, FBPLodgeComponentInfo, LogComponent, AActor*, OtherActor, UPrimitiveComponent*, OtherComp, ECollisionChannel, OtherCompCollisionChannel, FBPHitSurfaceProperties, HitSurfaceProperties, FVector, NormalImpulse, const FHitResult&, Hit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FVROnMeleeInvalidHitSignature, AActor*, OtherActor, UPrimitiveComponent*, OtherComp, FVector, NormalImpulse, const FHitResult&, Hit);

UCLASS(NotBlueprintable, ClassGroup = (VRExpansionPlugin), hideCategories = TickSettings)
class VREXPANSIONPLUGIN_API UGS_Melee : public UGS_Default
{
	GENERATED_BODY()
public:

	UGS_Melee(const FObjectInitializer& ObjectInitializer);

	UFUNCTION()
	virtual void OnLodgeHitCallback(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Weapon Settings")
	void SetIsLodged(bool IsLodged, UPrimitiveComponent* LodgeComponent);

	bool bIsLodged;
	TWeakObjectPtr<UPrimitiveComponent> LodgedComponent;

	UPROPERTY(BlueprintAssignable, Category = "Melee|Lodging")
		FVROnMeleeShouldLodgeSignature OnShouldLodgeInObject;

	UPROPERTY(BlueprintAssignable, Category = "Melee|Hit")
		FVROnMeleeOnHit OnMeleeHit;

	UPROPERTY(BlueprintAssignable, Category = "Melee|Hit")
		FVROnMeleeInvalidHitSignature OnMeleeInvalidHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee|Lodging")
		bool bAlwaysTickPenetration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee|Lodging")
		bool bOnlyPenetrateWithTwoHands;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee|Lodging")
		TArray<FBPHitSurfaceProperties> OverrideMeleeSurfaceSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		FName WeaponRootOrientationComponent;
	FTransform OrientationComponentRelativeFacing;

	UFUNCTION(BlueprintCallable, Category = "Weapon Settings")
		void UpdateHandPosition(FBPGripPair HandPair, FVector HandWorldPosition, FVector & LocDifference);

	UFUNCTION(BlueprintCallable, Category = "Weapon Settings")
	void UpdateHandPositionAndRotation(FBPGripPair HandPair, FTransform HandWorldTransform, FVector& LocDifference, float& RotDifference, bool bUpdateLocation = true, bool bUpdateRotation = true);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		TArray<FBPLodgeComponentInfo> PenetrationNotifierComponents;

	bool bCheckLodge;
	bool bIsHeld;

	FVector LastRelativePos;
	FVector RelativeBetweenGripsCenterPos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		bool bAutoSetPrimaryAndSecondaryHands;

	bool bHasValidPrimaryHand;

	UFUNCTION(BlueprintCallable, Category = "Weapon Settings")
		void SetPrimaryAndSecondaryHands(FBPGripPair & PrimaryGrip, FBPGripPair & SecondaryGrip);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		EVRMeleePrimaryHandType PrimaryHandSelectionType;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Settings")
	FBPGripPair PrimaryHand;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Settings")
	FBPGripPair SecondaryHand;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Settings")
		bool bUsePrimaryHandSettingsWithOneHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		EVRMeleeComType COMType;

	FTransform ObjectRelativeGripCenter;

	void SetComBetweenHands(UGripMotionControllerComponent* GrippingController, FBPActorPhysicsHandleInformation * HandleInfo);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		FBPAdvancedPhysicsHandleSettings PrimaryHandPhysicsSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		FBPAdvancedPhysicsHandleSettings SecondaryHandPhysicsSettings;

	void UpdateDualHandInfo();

	virtual void HandlePostPhysicsHandle(UGripMotionControllerComponent* GrippingController, FBPActorPhysicsHandleInformation* HandleInfo) override;
	virtual void HandlePrePhysicsHandle(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation &GripInfo, FBPActorPhysicsHandleInformation* HandleInfo, FTransform& KinPose) override;
	virtual void OnBeginPlay_Implementation(UObject* CallingOwner) override;
	virtual void OnEndPlay_Implementation(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnSecondaryGrip_Implementation(UGripMotionControllerComponent* Controller, USceneComponent* SecondaryGripComponent, const FBPActorGripInformation& GripInformation) override;

	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;

	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;

	virtual bool Wants_DenyTeleport_Implementation(UGripMotionControllerComponent* Controller) override;

	virtual bool GetWorldTransform_Implementation(UGripMotionControllerComponent * GrippingController, float DeltaTime, FTransform & WorldTransform, const FTransform &ParentTransform, FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport) override;

};
