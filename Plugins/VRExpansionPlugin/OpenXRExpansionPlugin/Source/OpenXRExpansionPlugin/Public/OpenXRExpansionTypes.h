

#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeadMountedDisplayTypes.h"
#include "Animation/BoneReference.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"

#include "OpenXRExpansionTypes.generated.h"

UENUM()
enum class EBPXRResultSwitch : uint8
{

	OnSucceeded,

	OnFailed
};

UENUM(BlueprintType)
enum class EVRSkeletalHandIndex : uint8
{
	EActionHandIndex_Left = 0,
	EActionHandIndex_Right
};

UENUM(BlueprintType)
enum class EXRHandJointType : uint8
{
	OXR_HAND_JOINT_PALM_EXT = 0,
	OXR_HAND_JOINT_WRIST_EXT = 1,
	OXR_HAND_JOINT_THUMB_METACARPAL_EXT = 2,
	OXR_HAND_JOINT_THUMB_PROXIMAL_EXT = 3,
	OXR_HAND_JOINT_THUMB_DISTAL_EXT = 4,
	OXR_HAND_JOINT_THUMB_TIP_EXT = 5,
	OXR_HAND_JOINT_INDEX_METACARPAL_EXT = 6,
	OXR_HAND_JOINT_INDEX_PROXIMAL_EXT = 7,
	OXR_HAND_JOINT_INDEX_INTERMEDIATE_EXT = 8,
	OXR_HAND_JOINT_INDEX_DISTAL_EXT = 9,
	OXR_HAND_JOINT_INDEX_TIP_EXT = 10,
	OXR_HAND_JOINT_MIDDLE_METACARPAL_EXT = 11,
	OXR_HAND_JOINT_MIDDLE_PROXIMAL_EXT = 12,
	OXR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT = 13,
	OXR_HAND_JOINT_MIDDLE_DISTAL_EXT = 14,
	OXR_HAND_JOINT_MIDDLE_TIP_EXT = 15,
	OXR_HAND_JOINT_RING_METACARPAL_EXT = 16,
	OXR_HAND_JOINT_RING_PROXIMAL_EXT = 17,
	OXR_HAND_JOINT_RING_INTERMEDIATE_EXT = 18,
	OXR_HAND_JOINT_RING_DISTAL_EXT = 19,
	OXR_HAND_JOINT_RING_TIP_EXT = 20,
	OXR_HAND_JOINT_LITTLE_METACARPAL_EXT = 21,
	OXR_HAND_JOINT_LITTLE_PROXIMAL_EXT = 22,
	OXR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT = 23,
	OXR_HAND_JOINT_LITTLE_DISTAL_EXT = 24,
	OXR_HAND_JOINT_LITTLE_TIP_EXT = 25,
	OXR_HAND_JOINT_MAX_ENUM_EXT = 0xFF
};

UENUM(BlueprintType)
enum class EVROpenXRSkeletonType : uint8
{

	OXR_SkeletonType_UE4Default_Right,

	OXR_SkeletonType_UE4Default_Left,

	OXR_SkeletonType_OpenVRDefault_Right,

	OXR_SkeletonType_OpenVRDefault_Left,

	OXR_SkeletonType_UE5Default_Right,

	OXR_SkeletonType_UE5Default_Left,

	OXR_SkeletonType_OpenXRDefault_Right,

	OXR_SkeletonType_OpenXRDefault_Left,

	OXR_SkeletonType_Custom
};

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|OpenXR|HandSkeleton")
struct OPENXREXPANSIONPLUGIN_API FBPOpenXRActionSkeletalData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		EVRSkeletalHandIndex TargetHand;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
	EXRControllerPoseType PoseType;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		float WorldScaleOverride;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bAllowDeformingMesh;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bMirrorLeftRight;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		TArray<float> FingerCurls;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Transient, Category = Default)
		TArray<FTransform> SkeletalTransforms;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		bool bEnableUE4HandRepSavings;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadWrite, Category = Default)
		FTransform AdditionTransform;

	UPROPERTY(NotReplicated, BlueprintReadOnly, Category = Default)
		bool bHasValidData;

	FName LastHandGesture;
	int32 LastHandGestureIndex;

	FBPOpenXRActionSkeletalData()
	{

		AdditionTransform = FTransform::Identity;
		WorldScaleOverride = 0.0f;
		bAllowDeformingMesh = true;
		bMirrorLeftRight = false;
		bEnableUE4HandRepSavings = false;
		TargetHand = EVRSkeletalHandIndex::EActionHandIndex_Right;
		PoseType = EXRControllerPoseType::Grip;
		bHasValidData = false;
		LastHandGestureIndex = INDEX_NONE;
		LastHandGesture = NAME_None;
	}
};

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENXREXPANSIONPLUGIN_API FBPOpenXRSkeletalPair
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		EXRHandJointType OpenXRBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		FName BoneToTarget;

	FBoneReference ReferenceToConstruct;
	FCompactPoseBoneIndex ParentReference;
	FQuat RetargetRot;

	FBPOpenXRSkeletalPair() :
		ParentReference(INDEX_NONE)
	{
		OpenXRBone = EXRHandJointType::OXR_HAND_JOINT_WRIST_EXT;
		BoneToTarget = NAME_None;
		RetargetRot = FQuat::Identity;
	}

	FBPOpenXRSkeletalPair(EXRHandJointType Bone, FString TargetBone) :
		ParentReference(INDEX_NONE)
	{
		OpenXRBone = Bone;
		BoneToTarget = FName(*TargetBone);
		ReferenceToConstruct.BoneName = BoneToTarget;
		RetargetRot = FQuat::Identity;
	}

	FORCEINLINE bool operator==(const int32& Other) const
	{
		return ReferenceToConstruct.CachedCompactPoseIndex.GetInt() == Other;

	}
};

USTRUCT(BlueprintType, Category = "VRExpansionFunctions|SteamVR|HandSkeleton")
struct OPENXREXPANSIONPLUGIN_API FBPOpenXRSkeletalMappingData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		TArray<FBPOpenXRSkeletalPair> BonePairs;

	TArray<int32> ReverseBonePairMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		bool bMergeMissingBonesUE4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		EVRSkeletalHandIndex TargetHand;

	FQuat AdjustmentQuat;
	bool bInitialized;

	FName LastInitializedName;
	EVROpenXRSkeletonType LastInitializedSkeleton;

	void ClearMapping()
	{
		bInitialized = false;
		LastInitializedName = NAME_None;
		AdjustmentQuat = FQuat::Identity;
		LastInitializedSkeleton = EVROpenXRSkeletonType::OXR_SkeletonType_Custom;

		BonePairs.Empty();
		ReverseBonePairMap.Empty();
	}

	void ConstructReverseMapping()
	{
		int32 MaxElements = ((uint8)EXRHandJointType::OXR_HAND_JOINT_LITTLE_TIP_EXT) + 1;
		ReverseBonePairMap.Empty(MaxElements);
		ReverseBonePairMap.AddUninitialized(MaxElements);
		FMemory::Memset(ReverseBonePairMap.GetData(), 0, MaxElements * sizeof(int32));

		for (int i = 0; i < BonePairs.Num(); ++i)
		{

			if (i < MaxElements)
			{
				ReverseBonePairMap[(uint8)BonePairs[i].OpenXRBone] = i;
			}
		}
	}

	void ConstructDefaultMappings(EVROpenXRSkeletonType SkeletonType, bool bSkipRootBone)
	{
		switch (SkeletonType)
		{

		case EVROpenXRSkeletonType::OXR_SkeletonType_OpenVRDefault_Left:
		case EVROpenXRSkeletonType::OXR_SkeletonType_OpenVRDefault_Right:
		{
			bMergeMissingBonesUE4 = false;
			SetDefaultOpenVRInputs(SkeletonType, bSkipRootBone);
		}break;
		case EVROpenXRSkeletonType::OXR_SkeletonType_UE4Default_Left:
		case EVROpenXRSkeletonType::OXR_SkeletonType_UE4Default_Right:
		{
			bMergeMissingBonesUE4 = true;
			SetDefaultUE4Inputs(SkeletonType, bSkipRootBone);
		}break;
		case EVROpenXRSkeletonType::OXR_SkeletonType_UE5Default_Left:
		case EVROpenXRSkeletonType::OXR_SkeletonType_UE5Default_Right:
		{
			bMergeMissingBonesUE4 = false;
			SetDefaultUE5Inputs(SkeletonType, bSkipRootBone);
		}break;
		case EVROpenXRSkeletonType::OXR_SkeletonType_OpenXRDefault_Left:
		case EVROpenXRSkeletonType::OXR_SkeletonType_OpenXRDefault_Right:
		{
			bMergeMissingBonesUE4 = false;
			SetDefaultOpenXRInputs(SkeletonType, bSkipRootBone);
		}break;
		}
	}

	void SetDefaultOpenVRInputs(EVROpenXRSkeletonType cSkeletonType, bool bSkipRootBone)
	{

		if (BonePairs.Num())
			return;

		bool bIsRightHand = cSkeletonType != EVROpenXRSkeletonType::OXR_SkeletonType_OpenVRDefault_Left;
		FString HandDelimiterS = !bIsRightHand ? "l" : "r";
		const TCHAR* HandDelimiter = *HandDelimiterS;

		TargetHand = bIsRightHand ? EVRSkeletalHandIndex::EActionHandIndex_Right : EVRSkeletalHandIndex::EActionHandIndex_Left;

		{
			BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_WRIST_EXT, FString::Printf(TEXT("wrist_%s"), HandDelimiter)));
		}

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_METACARPAL_EXT, FString::Printf(TEXT("finger_thumb_0_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_PROXIMAL_EXT, FString::Printf(TEXT("finger_thumb_1_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_DISTAL_EXT, FString::Printf(TEXT("finger_thumb_2_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_TIP_EXT, FString::Printf(TEXT("finger_thumb_%s_end"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_METACARPAL_EXT, FString::Printf(TEXT("finger_index_meta_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_PROXIMAL_EXT, FString::Printf(TEXT("finger_index_0_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_INTERMEDIATE_EXT, FString::Printf(TEXT("finger_index_1_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_DISTAL_EXT, FString::Printf(TEXT("finger_index_2_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_TIP_EXT, FString::Printf(TEXT("finger_index_%s_end"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_METACARPAL_EXT, FString::Printf(TEXT("finger_middle_meta_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_PROXIMAL_EXT, FString::Printf(TEXT("finger_middle_0_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT, FString::Printf(TEXT("finger_middle_1_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_DISTAL_EXT, FString::Printf(TEXT("finger_middle_2_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_TIP_EXT, FString::Printf(TEXT("finger_middle_%s_end"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_METACARPAL_EXT, FString::Printf(TEXT("finger_ring_meta_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_PROXIMAL_EXT, FString::Printf(TEXT("finger_ring_0_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_INTERMEDIATE_EXT, FString::Printf(TEXT("finger_ring_1_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_DISTAL_EXT, FString::Printf(TEXT("finger_ring_2_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_TIP_EXT, FString::Printf(TEXT("finger_ring_%s_end"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_METACARPAL_EXT, FString::Printf(TEXT("finger_pinky_meta_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_PROXIMAL_EXT, FString::Printf(TEXT("finger_pinky_0_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT, FString::Printf(TEXT("finger_pinky_1_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_DISTAL_EXT, FString::Printf(TEXT("finger_pinky_2_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_TIP_EXT, FString::Printf(TEXT("finger_pinky_%s_end"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_DISTAL_EXT, FString::Printf(TEXT("finger_thumb_%s_aux"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_DISTAL_EXT, FString::Printf(TEXT("finger_index_%s_aux"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_DISTAL_EXT, FString::Printf(TEXT("finger_middle_%s_aux"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_DISTAL_EXT, FString::Printf(TEXT("finger_ring_%s_aux"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_DISTAL_EXT, FString::Printf(TEXT("finger_pinky_%s_aux"), HandDelimiter)));
	}

	void SetDefaultUE4Inputs(EVROpenXRSkeletonType cSkeletonType, bool bSkipRootBone)
	{

		if (BonePairs.Num())
			return;

		bool bIsRightHand = cSkeletonType != EVROpenXRSkeletonType::OXR_SkeletonType_UE4Default_Left;
		FString HandDelimiterS = !bIsRightHand ? "l" : "r";
		const TCHAR* HandDelimiter = *HandDelimiterS;

		TargetHand = bIsRightHand ? EVRSkeletalHandIndex::EActionHandIndex_Right : EVRSkeletalHandIndex::EActionHandIndex_Left;

		{
			BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_WRIST_EXT, FString::Printf(TEXT("hand_%s"), HandDelimiter)));
		}

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_PROXIMAL_EXT, FString::Printf(TEXT("index_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_INTERMEDIATE_EXT, FString::Printf(TEXT("index_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_DISTAL_EXT, FString::Printf(TEXT("index_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_PROXIMAL_EXT, FString::Printf(TEXT("middle_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT, FString::Printf(TEXT("middle_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_DISTAL_EXT, FString::Printf(TEXT("middle_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_PROXIMAL_EXT, FString::Printf(TEXT("pinky_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT, FString::Printf(TEXT("pinky_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_DISTAL_EXT, FString::Printf(TEXT("pinky_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_PROXIMAL_EXT, FString::Printf(TEXT("ring_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_INTERMEDIATE_EXT, FString::Printf(TEXT("ring_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_DISTAL_EXT, FString::Printf(TEXT("ring_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_METACARPAL_EXT, FString::Printf(TEXT("thumb_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_PROXIMAL_EXT, FString::Printf(TEXT("thumb_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_DISTAL_EXT, FString::Printf(TEXT("thumb_03_%s"), HandDelimiter)));

	}

	void SetDefaultUE5Inputs(EVROpenXRSkeletonType cSkeletonType, bool bSkipRootBone)
	{

		if (BonePairs.Num())
			return;

		bool bIsRightHand = cSkeletonType != EVROpenXRSkeletonType::OXR_SkeletonType_UE5Default_Left;
		FString HandDelimiterS = !bIsRightHand ? "l" : "r";
		const TCHAR* HandDelimiter = *HandDelimiterS;

		TargetHand = bIsRightHand ? EVRSkeletalHandIndex::EActionHandIndex_Right : EVRSkeletalHandIndex::EActionHandIndex_Left;

		{
			BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_WRIST_EXT, FString::Printf(TEXT("hand_%s"), HandDelimiter)));
		}

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_PROXIMAL_EXT, FString::Printf(TEXT("index_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_INTERMEDIATE_EXT, FString::Printf(TEXT("index_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_DISTAL_EXT, FString::Printf(TEXT("index_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_PROXIMAL_EXT, FString::Printf(TEXT("middle_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT, FString::Printf(TEXT("middle_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_DISTAL_EXT, FString::Printf(TEXT("middle_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_PROXIMAL_EXT, FString::Printf(TEXT("pinky_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT, FString::Printf(TEXT("pinky_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_DISTAL_EXT, FString::Printf(TEXT("pinky_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_PROXIMAL_EXT, FString::Printf(TEXT("ring_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_INTERMEDIATE_EXT, FString::Printf(TEXT("ring_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_DISTAL_EXT, FString::Printf(TEXT("ring_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_METACARPAL_EXT, FString::Printf(TEXT("thumb_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_PROXIMAL_EXT, FString::Printf(TEXT("thumb_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_DISTAL_EXT, FString::Printf(TEXT("thumb_03_%s"), HandDelimiter)));

	}

	void SetDefaultOpenXRInputs(EVROpenXRSkeletonType cSkeletonType, bool bSkipRootBone)
	{

		if (BonePairs.Num())
			return;

		bool bIsRightHand = cSkeletonType != EVROpenXRSkeletonType::OXR_SkeletonType_OpenXRDefault_Left;
		FString HandDelimiterS = !bIsRightHand ? "l" : "r";
		const TCHAR* HandDelimiter = *HandDelimiterS;

		TargetHand = bIsRightHand ? EVRSkeletalHandIndex::EActionHandIndex_Right : EVRSkeletalHandIndex::EActionHandIndex_Left;

		{
			BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_WRIST_EXT, FString::Printf(TEXT("hand_%s"), HandDelimiter)));
		}

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_PROXIMAL_EXT, FString::Printf(TEXT("index_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_INTERMEDIATE_EXT, FString::Printf(TEXT("index_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_INDEX_DISTAL_EXT, FString::Printf(TEXT("index_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_PROXIMAL_EXT, FString::Printf(TEXT("middle_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT, FString::Printf(TEXT("middle_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_MIDDLE_DISTAL_EXT, FString::Printf(TEXT("middle_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_PROXIMAL_EXT, FString::Printf(TEXT("pinky_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT, FString::Printf(TEXT("pinky_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_LITTLE_DISTAL_EXT, FString::Printf(TEXT("pinky_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_PROXIMAL_EXT, FString::Printf(TEXT("ring_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_INTERMEDIATE_EXT, FString::Printf(TEXT("ring_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_RING_DISTAL_EXT, FString::Printf(TEXT("ring_03_%s"), HandDelimiter)));

		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_METACARPAL_EXT, FString::Printf(TEXT("thumb_01_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_PROXIMAL_EXT, FString::Printf(TEXT("thumb_02_%s"), HandDelimiter)));
		BonePairs.Add(FBPOpenXRSkeletalPair(EXRHandJointType::OXR_HAND_JOINT_THUMB_DISTAL_EXT, FString::Printf(TEXT("thumb_03_%s"), HandDelimiter)));

	}

	FBPOpenXRSkeletalMappingData()
	{
		AdjustmentQuat = FQuat::Identity;
		bInitialized = false;
		bMergeMissingBonesUE4 = false;
		TargetHand = EVRSkeletalHandIndex::EActionHandIndex_Right;
		LastInitializedName = NAME_None;
		LastInitializedSkeleton = EVROpenXRSkeletonType::OXR_SkeletonType_Custom;
	}
};
