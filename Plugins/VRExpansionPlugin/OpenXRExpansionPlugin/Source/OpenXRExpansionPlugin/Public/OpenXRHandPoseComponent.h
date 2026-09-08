

#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Texture.h"
#include "Engine/EngineTypes.h"
#include "Components/ActorComponent.h"
#include "HeadMountedDisplayTypes.h"

#include "Animation/AnimInstanceProxy.h"
#include "OpenXRExpansionTypes.h"
#include "Engine/DataAsset.h"

#include "OpenXRHandPoseComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(OpenXRExpansionHandTrackingLog, Log, All);

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|OpenXR|HandSkeleton")
struct OPENXREXPANSIONPLUGIN_API FBPXRSkeletalRepContainer
{
	GENERATED_BODY()
public:

	UPROPERTY(Transient, NotReplicated)
		EVRSkeletalHandIndex TargetHand;

	UPROPERTY(Transient, NotReplicated)
		bool bAllowDeformingMesh;

	UPROPERTY(Transient, NotReplicated)
		bool bEnableUE4HandRepSavings;

	UPROPERTY(Transient, NotReplicated)
		TArray<FTransform> SkeletalTransforms;

	UPROPERTY(Transient, NotReplicated)
		uint8 BoneCount;

	FBPXRSkeletalRepContainer()
	{
		TargetHand = EVRSkeletalHandIndex::EActionHandIndex_Left;
		bAllowDeformingMesh = false;
		bEnableUE4HandRepSavings = false;
		BoneCount = 0;
	}

	bool bHasValidData()
	{
		return SkeletalTransforms.Num() > 0;
	}

	void CopyForReplication(FBPOpenXRActionSkeletalData& Other);
	static void CopyReplicatedTo(const FBPXRSkeletalRepContainer& Container, FBPOpenXRActionSkeletalData& Other);

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits< FBPXRSkeletalRepContainer > : public TStructOpsTypeTraitsBase2<FBPXRSkeletalRepContainer>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType, Category = "VRGestures")
struct OPENXREXPANSIONPLUGIN_API FOpenXRGestureFingerPosition
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VRGesture")
		EXRHandJointType IndexType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
		FVector Value;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
		float Threshold;

	FOpenXRGestureFingerPosition(FVector TipLoc, EXRHandJointType Type)
	{
		IndexType = Type;
		Value = TipLoc;
		Threshold = 5.0f;
	}
	FOpenXRGestureFingerPosition()
	{
		IndexType = EXRHandJointType::OXR_HAND_JOINT_INDEX_TIP_EXT;
		Value = FVector(0.f);
		Threshold = 5.0f;
	}
};

USTRUCT(BlueprintType, Category = "VRGestures")
struct OPENXREXPANSIONPLUGIN_API FOpenXRGesture
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
		FName Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
		TArray<FOpenXRGestureFingerPosition> FingerValues;

	FOpenXRGesture()
	{
		InitPoseValues();
		Name = NAME_None;
	}

	void InitPoseValues()
	{
		FingerValues.Add(FOpenXRGestureFingerPosition(FVector::ZeroVector, EXRHandJointType::OXR_HAND_JOINT_THUMB_TIP_EXT));
		FingerValues.Add(FOpenXRGestureFingerPosition(FVector::ZeroVector, EXRHandJointType::OXR_HAND_JOINT_INDEX_TIP_EXT));
		FingerValues.Add(FOpenXRGestureFingerPosition(FVector::ZeroVector, EXRHandJointType::OXR_HAND_JOINT_MIDDLE_TIP_EXT));
		FingerValues.Add(FOpenXRGestureFingerPosition(FVector::ZeroVector, EXRHandJointType::OXR_HAND_JOINT_RING_TIP_EXT));
		FingerValues.Add(FOpenXRGestureFingerPosition(FVector::ZeroVector, EXRHandJointType::OXR_HAND_JOINT_LITTLE_TIP_EXT));
	}
};

UCLASS(BlueprintType, Category = "VRGestures")
class OPENXREXPANSIONPLUGIN_API UOpenXRGestureDatabase : public UDataAsset
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		TArray <FOpenXRGesture> Gestures;

	UOpenXRGestureDatabase()
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOpenXRGestureDetected, const FName &, GestureDetected, int32, GestureIndex, EVRSkeletalHandIndex, ActionHandType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOpenXRGestureEnded, const FName &, GestureEnded, int32, GestureIndex, EVRSkeletalHandIndex, ActionHandType);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class OPENXREXPANSIONPLUGIN_API UOpenXRHandPoseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOpenXRHandPoseComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		bool bDetectGestures;

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void SetDetectGestures(bool bNewDetectGestures)
	{
		bDetectGestures = bNewDetectGestures;
	}

	UPROPERTY(BlueprintAssignable, Category = "VRGestures")
		FOpenXRGestureDetected OnNewGestureDetected;

	UPROPERTY(BlueprintAssignable, Category = "VRGestures")
		FOpenXRGestureEnded OnGestureEnded;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		UOpenXRGestureDatabase *GesturesDB;

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		bool SaveCurrentPose(FName RecordingName, EVRSkeletalHandIndex HandToSave = EVRSkeletalHandIndex::EActionHandIndex_Right);

	UFUNCTION(BlueprintCallable, Category = "VRGestures", meta = (DisplayName = "DetectCurrentPose"))
		bool K2_DetectCurrentPose(UPARAM(ref) FBPOpenXRActionSkeletalData& SkeletalAction, FOpenXRGesture & GestureOut);

	bool DetectCurrentPose(FBPOpenXRActionSkeletalData& SkeletalAction);

	inline bool IsLocallyControlled() const
	{

		const AActor* MyOwner = GetOwner();
		return MyOwner->HasLocalNetOwner();

	}

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkeletalData|Actions")
		bool bGetMockUpPoseForDebugging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkeletalData|Actions")
		TArray<FBPOpenXRActionSkeletalData> HandSkeletalActions;

	UPROPERTY(Replicated, Transient, ReplicatedUsing = OnRep_SkeletalTransformLeft)
		FBPXRSkeletalRepContainer LeftHandRep;

	UPROPERTY(Replicated, Transient, ReplicatedUsing = OnRep_SkeletalTransformRight)
		FBPXRSkeletalRepContainer RightHandRep;

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_SendSkeletalTransforms(const FBPXRSkeletalRepContainer& SkeletalInfo);

	bool bLerpingPositionLeft;
	bool bLerpingPositionRight;

	struct FTransformLerpManager
	{
		bool bReplicatedOnce;
		bool bLerping;
		float UpdateCount;
		float UpdateRate;
		TArray<FTransform> OldTransforms;
		TArray<FTransform> NewTransforms;

		FTransformLerpManager();
		void PreCopyNewData(FBPOpenXRActionSkeletalData& ActionInfo, int NetUpdateRate, bool bExponentialSmoothing);
		void NotifyNewData(FBPOpenXRActionSkeletalData& ActionInfo, int NetUpdateRate, bool bExponentialSmoothing);

		FORCEINLINE void BlendBone(uint8 BoneToBlend, FBPOpenXRActionSkeletalData& ActionInfo, float& LerpVal, bool bExponentialSmoothing)
		{
			ActionInfo.SkeletalTransforms[BoneToBlend].Blend(OldTransforms[BoneToBlend], NewTransforms[BoneToBlend], LerpVal);

			if (bExponentialSmoothing)
			{

				OldTransforms[BoneToBlend] = ActionInfo.SkeletalTransforms[BoneToBlend];
			}
		}

		void UpdateManager(float DeltaTime, FBPOpenXRActionSkeletalData& ActionInfo, UOpenXRHandPoseComponent * ParentComp);

	}; 

	FTransformLerpManager LeftHandRepManager;
	FTransformLerpManager RightHandRepManager;

	UFUNCTION()
	virtual void OnRep_SkeletalTransformLeft()
	{
		for (int i = 0; i < HandSkeletalActions.Num(); i++)
		{
			if (HandSkeletalActions[i].TargetHand == LeftHandRep.TargetHand)
			{
				if (bSmoothReplicatedSkeletalData)
				{
					LeftHandRepManager.PreCopyNewData(HandSkeletalActions[i], ReplicationRateForSkeletalAnimations, bUseExponentialSmoothing);
				}

				FBPXRSkeletalRepContainer::CopyReplicatedTo(LeftHandRep, HandSkeletalActions[i]);

				if (bSmoothReplicatedSkeletalData)
				{
					LeftHandRepManager.NotifyNewData(HandSkeletalActions[i], ReplicationRateForSkeletalAnimations, bUseExponentialSmoothing);
				}

				break;
			}
		}
	}

	UFUNCTION()
	virtual void OnRep_SkeletalTransformRight()
	{
		for (int i = 0; i < HandSkeletalActions.Num(); i++)
		{
			if (HandSkeletalActions[i].TargetHand == RightHandRep.TargetHand)
			{
				if (bSmoothReplicatedSkeletalData)
				{
					RightHandRepManager.PreCopyNewData(HandSkeletalActions[i], ReplicationRateForSkeletalAnimations, bUseExponentialSmoothing);
				}

				FBPXRSkeletalRepContainer::CopyReplicatedTo(RightHandRep, HandSkeletalActions[i]);

				if (bSmoothReplicatedSkeletalData)
				{
					RightHandRepManager.NotifyNewData(HandSkeletalActions[i], ReplicationRateForSkeletalAnimations, bUseExponentialSmoothing);
				}
				break;
			}
		}
	}

	UPROPERTY(EditAnywhere, Category = SkeletalData)
		bool bReplicateSkeletalData;

	UPROPERTY(EditAnywhere, Category = SkeletalData)
		bool bSmoothReplicatedSkeletalData;

	UPROPERTY(EditAnywhere, Category = "SkeletalData", meta = (editcondition = "bSmoothReplicatedSkeletalData"))
		bool bUseExponentialSmoothing = false;

	UPROPERTY(EditAnywhere, Category = "SkeletalData", meta = (editcondition = "bUseExponentialSmoothing"))
		float InterpolationSpeed = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SkeletalData)
		float ReplicationRateForSkeletalAnimations;

	float SkeletalNetUpdateCount;

	float SkeletalUpdateCount;
};	

USTRUCT()
struct OPENXREXPANSIONPLUGIN_API FOpenXRAnimInstanceProxy : public FAnimInstanceProxy
{
public:
	GENERATED_BODY()

		FOpenXRAnimInstanceProxy() {}
		FOpenXRAnimInstanceProxy(UAnimInstance* InAnimInstance);

		virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;

public:

	EVRSkeletalHandIndex TargetHand;
	TArray<FBPOpenXRActionSkeletalData> HandSkeletalActionData;

};

UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class OPENXREXPANSIONPLUGIN_API UOpenXRAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(transient)
		UOpenXRHandPoseComponent * OwningPoseComp;

	FOpenXRAnimInstanceProxy AnimInstanceProxy;

	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override
	{
		return new FOpenXRAnimInstanceProxy(this);

	}

	virtual void NativeBeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "BoneMappings")
	void InitializeCustomBoneMapping(UPARAM(ref) FBPOpenXRSkeletalMappingData & SkeletalMappingData);

};
