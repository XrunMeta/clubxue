

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Components/SceneComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BoneReference.h"
#include "Misc/Guid.h"

#include "HandSocketComponent.generated.h"

class USkeletalMeshComponent;
class UPoseableMeshComponent;
class USkeletalMesh;
class UGripMotionControllerComponent;
class UAnimSequence;
struct FPoseSnapshot;

DECLARE_LOG_CATEGORY_EXTERN(LogVRHandSocketComponent, Log, All);

struct VREXPANSIONPLUGIN_API FVRHandSocketCustomVersion
{
	enum Type
	{

		BeforeCustomVersionWasAdded = 0,

		HandSocketStoringSetState = 1,

		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	const static FGuid GUID;

private:
	FVRHandSocketCustomVersion() {}
};

UENUM(BlueprintType)
namespace EVRAxis
{
	enum Type
	{
		X,
		Y,
		Z
	};
}

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPVRHandPoseBonePair
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
		FName BoneName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
		FQuat DeltaPose;

	FBoneReference ReferenceToConstruct;

	FBPVRHandPoseBonePair()
	{
		BoneName = NAME_None;
		DeltaPose = FQuat::Identity;
	}

	FORCEINLINE bool operator==(const FName& Other) const
	{
		return (BoneName == Other);
	}
};

UCLASS(Blueprintable, ClassGroup = (VRExpansionPlugin), hideCategories = ("Component Tick", Events, Physics, Lod, "Asset User Data", Collision))
class VREXPANSIONPLUGIN_API UHandSocketComponent : public USceneComponent, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	UHandSocketComponent(const FObjectInitializer& ObjectInitializer);
	~UHandSocketComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data|Mirroring|Advanced")
		TEnumAsByte<EVRAxis::Type> MirrorAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data|Mirroring|Advanced")
		TEnumAsByte<EVRAxis::Type> FlipAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Hand Socket Data")
		FTransform HandRelativePlacement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data")
		FName SlotPrefix;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hand Socket Data")
		bool bDecoupleMeshPlacement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data")
		bool bOnlySnapMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data")
		bool bOnlyUseHandPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data")
		bool bIgnoreAttachBone;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hand Socket Data")
		bool bLeftHandDominant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data|Mirroring", meta = (DisplayName = "Flip For Off Hand"))
		bool bFlipForLeftHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data|Mirroring", meta = (editcondition = "bFlipForLeftHand"))
		bool bOnlyFlipRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data|Searching")
		bool bAlwaysInRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data|Searching")
		bool bMatchRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data|Control")
		bool bDisabled;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hand Socket Data|Control")
		bool bLockInPlace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data|Searching")
		float OverrideDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Animation")
		bool bUseCustomPoseDeltas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Animation")
		TArray<FBPVRHandPoseBonePair> CustomPoseDeltas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Animation")
		TObjectPtr<UAnimSequence> HandTargetAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Socket Data|Mirroring")
		FVector MirroredScale;

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Animation|Misc")
		bool bFilterBonesByPostfix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Animation|Misc")
		FString FilterPostfix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Animation|Misc")
		TArray<FName> BonesToSkip;

	FTransform GetBoneTransformAtTime(UAnimSequence* MyAnimSequence,  int BoneIdx, FName BoneName, bool bUseRawDataOnly);
#endif

	UFUNCTION(BlueprintCallable, Category = "Hand Socket Data")
		UAnimSequence* GetTargetAnimation();

	UFUNCTION(BlueprintCallable, Category = "Hand Socket Data")
		bool GetBlendedPoseSnapShot(FPoseSnapshot& PoseSnapShot, USkeletalMeshComponent* TargetMesh = nullptr, bool bSkipRootBone = false, bool bFlipHand = false);

	UFUNCTION(BlueprintCallable, Category = "Hand Socket Data", meta = (bIgnoreSelf = "true"))
		static bool GetAnimationSequenceAsPoseSnapShot(UAnimSequence * InAnimationSequence, FPoseSnapshot& OutPoseSnapShot, USkeletalMeshComponent* TargetMesh = nullptr, bool bSkipRootBone = false, bool bFlipHand = false);

	UFUNCTION(BlueprintCallable, Category = "Hand Socket Data")
		static void GetAllHandSocketComponents(TArray<UHandSocketComponent*>& OutHandSockets);

	UFUNCTION(BlueprintCallable, Category = "Hand Socket Data")
		static bool GetAllHandSocketComponentsInRange(FVector SearchFromWorldLocation, float SearchRange, TArray<UHandSocketComponent*>& OutHandSockets);

	UFUNCTION(BlueprintCallable, Category = "Hand Socket Data")
		static UHandSocketComponent* GetClosestHandSocketComponentInRange(FVector SearchFromWorldLocation, float SearchRange);

	FTransform GetHandRelativePlacement();

	inline void MirrorHandTransform(FTransform& ReturnTrans, FTransform& relTrans)
	{
		if (bOnlyFlipRotation)
		{
			ReturnTrans.SetTranslation(ReturnTrans.GetTranslation() - relTrans.GetTranslation());
			ReturnTrans.Mirror(GetAsEAxis(MirrorAxis), GetCrossAxis());
			ReturnTrans.SetTranslation(ReturnTrans.GetTranslation() + relTrans.GetTranslation());
		}
		else
		{
			ReturnTrans.Mirror(GetAsEAxis(MirrorAxis), GetCrossAxis());
		}
	}

	inline TEnumAsByte<EAxis::Type> GetAsEAxis(TEnumAsByte<EVRAxis::Type> InAxis)
	{
		switch (InAxis)
		{
		case EVRAxis::X:
		{
			return EAxis::X;
		}break;
		case EVRAxis::Y:
		{
			return EAxis::Y;
		}break;
		case EVRAxis::Z:
		{
			return EAxis::Z;
		}break;
		}

		return EAxis::X;
	}

	inline FVector GetMirrorVector()
	{
		switch (MirrorAxis)
		{
		case EVRAxis::Y:
		{
			return FVector::RightVector;
		}break;
		case EVRAxis::Z:
		{
			return FVector::UpVector;
		}break;
		case EVRAxis::X:
		default:
		{
			return FVector::ForwardVector;
		}break;
		}
	}

	inline FVector GetFlipVector()
	{
		switch (FlipAxis)
		{
		case EVRAxis::Y:
		{
			return FVector::RightVector;
		}break;
		case EVRAxis::Z:
		{
			return FVector::UpVector;
		}break;
		case EVRAxis::X:
		default:
		{
			return FVector::ForwardVector;
		}break;
		}
	}

	inline TEnumAsByte<EAxis::Type> GetCrossAxis()
	{

		FVector SignVec = MirroredScale.GetSignVector();

		if (SignVec.X < 0)
		{
			return EAxis::X;
		}
		else if (SignVec.Z < 0)
		{
			return EAxis::Z;
		}
		else if (SignVec.Y < 0)
		{
			return EAxis::Y;
		}

		return GetAsEAxis(FlipAxis);

	}

	UFUNCTION(BlueprintCallable, Category = "Hand Socket Data")
	FTransform GetMeshRelativeTransform(bool bIsRightHand, bool bUseParentScale = false, bool bUseMirrorScale = false);

	UFUNCTION(BlueprintCallable, Category = "Hand Socket Data")
	static UHandSocketComponent* GetHandSocketComponentFromObject(UObject* ObjectToCheck, FName SocketName);

	virtual FTransform GetHandSocketTransform(UGripMotionControllerComponent* QueryController, bool bIgnoreOnlySnapMesh = false);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
#if WITH_EDITORONLY_DATA
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	void PoseVisualizationToAnimation(bool bForceRefresh = false);
	bool bTickedPose;

	UPROPERTY()
	bool bDecoupled;

#endif
	virtual void Serialize(FArchive& Ar) override;
	virtual void OnRegister() override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer = GameplayTags;
	}

	protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "GameplayTags")
		FGameplayTagContainer GameplayTags;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface|Replication")
		bool bRepGameplayTags;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface|Replication")
		bool bReplicateMovement;

	public:
		FGameplayTagContainer& GetGameplayTags();

		void SetRepGameplayTags(bool NewRepGameplayTags);
		inline bool GetRepGameplayTags() { return bRepGameplayTags; };
		void SetReplicateMovement(bool NewReplicateMovement);
		inline bool GetReplicateMovement() { return bReplicateMovement; };

#if WITH_EDITORONLY_DATA

	UPROPERTY()
		TObjectPtr<UPoseableMeshComponent> HandVisualizerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Hand Visualization")
		TObjectPtr<USkeletalMesh> VisualizationMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Visualization")
		bool bShowVisualizationMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Visualization")
		bool bMirrorVisualizationMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Visualization")
		bool bShowRangeVisualization;

	void PositionVisualizationMesh();
	void HideVisualizationMesh();

#endif

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Visualization")
		TObjectPtr<UMaterialInterface> HandPreviewMaterial;

#endif
};

UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class VREXPANSIONPLUGIN_API UHandSocketAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, transient, Category = "Socket Data")
		TObjectPtr<UHandSocketComponent> OwningSocket;

	virtual void NativeInitializeAnimation() override;
};