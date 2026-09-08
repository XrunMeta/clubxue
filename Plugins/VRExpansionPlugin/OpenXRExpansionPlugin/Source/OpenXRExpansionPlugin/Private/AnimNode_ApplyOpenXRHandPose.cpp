
#include "AnimNode_ApplyOpenXRHandPose.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNode_ApplyOpenXRHandPose)

#include "OpenXRExpansionFunctionLibrary.h"
#include "AnimNode_ApplyOpenXRHandPose.h"
#include "AnimationRuntime.h"
#include "DrawDebugHelpers.h"
#include "OpenXRHandPoseComponent.h"
#include "Runtime/Engine/Public/Animation/AnimInstanceProxy.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"

FAnimNode_ApplyOpenXRHandPose::FAnimNode_ApplyOpenXRHandPose()
	: FAnimNode_SkeletalControlBase()
{
	WorldIsGame = false;
	Alpha = 1.f;
	SkeletonType = EVROpenXRSkeletonType::OXR_SkeletonType_UE4Default_Right;
	bIsOpenInputAnimationInstance = false;
	bSkipRootBone = false;
	bOnlyApplyWristTransform = false;

}

void FAnimNode_ApplyOpenXRHandPose::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	Super::OnInitializeAnimInstance(InProxy, InAnimInstance);

	if (const UOpenXRAnimInstance * OpenXRAnimInstance = Cast<UOpenXRAnimInstance>(InAnimInstance))
	{
		bIsOpenInputAnimationInstance = true;

		if (OpenXRAnimInstance->AnimInstanceProxy.HandSkeletalActionData.Num())
		{
			for (int i = 0; i < OpenXRAnimInstance->AnimInstanceProxy.HandSkeletalActionData.Num(); ++i)
			{
				EVRSkeletalHandIndex TargetHand = OpenXRAnimInstance->AnimInstanceProxy.HandSkeletalActionData[i].TargetHand;

				if (OpenXRAnimInstance->AnimInstanceProxy.HandSkeletalActionData[i].bMirrorLeftRight)
				{
					TargetHand = (TargetHand == EVRSkeletalHandIndex::EActionHandIndex_Left) ? EVRSkeletalHandIndex::EActionHandIndex_Right : EVRSkeletalHandIndex::EActionHandIndex_Left;
				}

				if (TargetHand == MappedBonePairs.TargetHand)
				{
					bIsMirroringHand = OpenXRAnimInstance->AnimInstanceProxy.HandSkeletalActionData[i].bMirrorLeftRight;
					break;
				}
			}
		}
	}
}

void FAnimNode_ApplyOpenXRHandPose::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	Super::Initialize_AnyThread(Context);
}

void FAnimNode_ApplyOpenXRHandPose::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	Super::CacheBones_AnyThread(Context);
}

void FAnimNode_ApplyOpenXRHandPose::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	UObject* OwningAsset = RequiredBones.GetAsset();
	if (!OwningAsset)
		return;

	USkeleton* AssetSkeleton = RequiredBones.GetSkeletonAsset();

	if (!AssetSkeleton)
		return;

	if (!MappedBonePairs.bInitialized || OwningAsset->GetFName() != MappedBonePairs.LastInitializedName || SkeletonType != MappedBonePairs.LastInitializedSkeleton)
	{

		if (MappedBonePairs.bInitialized && (OwningAsset->GetFName() != MappedBonePairs.LastInitializedName || SkeletonType != MappedBonePairs.LastInitializedSkeleton))
		{
			MappedBonePairs.ClearMapping();
		}

		MappedBonePairs.LastInitializedName = OwningAsset->GetFName();
		MappedBonePairs.LastInitializedSkeleton = SkeletonType;
		MappedBonePairs.bInitialized = false;

		if (AssetSkeleton)
		{

			if (!MappedBonePairs.BonePairs.Num())
			{

				MappedBonePairs.ConstructDefaultMappings(SkeletonType, bSkipRootBone);
			}

			MappedBonePairs.ConstructReverseMapping();

			TArray<FTransform> RefBones = AssetSkeleton->GetReferenceSkeleton().GetRefBonePose();
			TArray<FMeshBoneInfo> RefBonesInfo = AssetSkeleton->GetReferenceSkeleton().GetRefBoneInfo();

			for (FBPOpenXRSkeletalPair& BonePair : MappedBonePairs.BonePairs)
			{

				BonePair.ReferenceToConstruct.BoneName = BonePair.BoneToTarget;

				BonePair.ReferenceToConstruct.Initialize(AssetSkeleton);

				BonePair.ReferenceToConstruct.CachedCompactPoseIndex = BonePair.ReferenceToConstruct.GetCompactPoseIndex(RequiredBones);

				if ((BonePair.ReferenceToConstruct.CachedCompactPoseIndex != INDEX_NONE))
				{

					BonePair.ParentReference = RequiredBones.GetParentBoneIndex(BonePair.ReferenceToConstruct.CachedCompactPoseIndex);
				}
			}

			MappedBonePairs.bInitialized = true;

			if (SkeletonType == EVROpenXRSkeletonType::OXR_SkeletonType_OpenVRDefault_Left || SkeletonType == EVROpenXRSkeletonType::OXR_SkeletonType_OpenVRDefault_Right)
			{

				MappedBonePairs.AdjustmentQuat = FRotator(0.f, 90.f, 180.f).Quaternion(); 

			}
			else
			{
				CalculateSkeletalAdjustment(AssetSkeleton);
			}

		}
	}
}

void FAnimNode_ApplyOpenXRHandPose::CalculateSkeletalAdjustment(USkeleton* AssetSkeleton)
{

	TArray<FTransform> RefBones = AssetSkeleton->GetReferenceSkeleton().GetRefBonePose();
	TArray<FMeshBoneInfo> RefBonesInfo = AssetSkeleton->GetReferenceSkeleton().GetRefBoneInfo();

	if (!MappedBonePairs.bInitialized || MappedBonePairs.BonePairs.Num() < 4 || !RefBones.Num())
	{
		UE_LOGF(LogTemp, Error, "Empty or incorrect mapping or skeleton data when calculating skeletal adjustment!");
		return;
	}

	FBPOpenXRSkeletalPair KnuckleIndexPair = MappedBonePairs.BonePairs[MappedBonePairs.ReverseBonePairMap[(int8)EXRHandJointType::OXR_HAND_JOINT_INDEX_PROXIMAL_EXT]];
	FBPOpenXRSkeletalPair KnuckleMiddlePair = MappedBonePairs.BonePairs[MappedBonePairs.ReverseBonePairMap[(int8)EXRHandJointType::OXR_HAND_JOINT_MIDDLE_PROXIMAL_EXT]];
	FBPOpenXRSkeletalPair KnuckleRingPair = MappedBonePairs.BonePairs[MappedBonePairs.ReverseBonePairMap[(int8)EXRHandJointType::OXR_HAND_JOINT_RING_PROXIMAL_EXT]];
	FBPOpenXRSkeletalPair KnucklePinkyPair = MappedBonePairs.BonePairs[MappedBonePairs.ReverseBonePairMap[(int8)EXRHandJointType::OXR_HAND_JOINT_LITTLE_PROXIMAL_EXT]];

	FBPOpenXRSkeletalPair WristPair = MappedBonePairs.BonePairs[MappedBonePairs.ReverseBonePairMap[(int8)EXRHandJointType::OXR_HAND_JOINT_WRIST_EXT]];

	FVector KnuckleAverage = GetRefBoneInCS(RefBones, RefBonesInfo, KnuckleIndexPair.ReferenceToConstruct.BoneIndex).GetTranslation();
	KnuckleAverage += GetRefBoneInCS(RefBones, RefBonesInfo, KnuckleMiddlePair.ReferenceToConstruct.BoneIndex).GetTranslation();
	KnuckleAverage += GetRefBoneInCS(RefBones, RefBonesInfo, KnuckleRingPair.ReferenceToConstruct.BoneIndex).GetTranslation();
	KnuckleAverage += GetRefBoneInCS(RefBones, RefBonesInfo, KnucklePinkyPair.ReferenceToConstruct.BoneIndex).GetTranslation();

	KnuckleAverage /= 4.f;

	FTransform WristTransform_UE = GetRefBoneInCS(RefBones, RefBonesInfo, WristPair.ReferenceToConstruct.BoneIndex);
	FVector ToKnuckleAverage_UE = KnuckleAverage - WristTransform_UE.GetTranslation();
	ToKnuckleAverage_UE.Normalize();

	WristForwardLS_UE = WristTransform_UE.GetRotation().UnrotateVector(ToKnuckleAverage_UE);
	SetVectorToMaxElement(WristForwardLS_UE);
	WristSideDirectionLS = FVector::CrossProduct(WristForwardLS_UE, FVector::RightVector);
	SetVectorToMaxElement(WristSideDirectionLS);

	CalculateOpenXRAdjustment();
}

void FAnimNode_ApplyOpenXRHandPose::CalculateOpenXRAdjustment()
{

	static FVector OpenXRForwardDirection = FVector(1.0f, 0.f, 0.f);

	bool bUseLeftHandOffsets = false;
	if ((!bIsMirroringHand && MappedBonePairs.TargetHand == EVRSkeletalHandIndex::EActionHandIndex_Left) ||
		(bIsMirroringHand && MappedBonePairs.TargetHand == EVRSkeletalHandIndex::EActionHandIndex_Right))
	{
		bUseLeftHandOffsets = true;
	}

	FVector OpenXRSideDirection = bUseLeftHandOffsets ? FVector(0.f, -1.f, 0.f) : FVector(0.f, 1.f, 0.f);

	FQuat AlignmentRot = FQuat::FindBetweenNormals(WristForwardLS_UE, OpenXRForwardDirection);

	FVector WristSideDirectionMS_UE = AlignmentRot * WristSideDirectionLS;

	FQuat TwistRotation = CalcRotationAboutAxis(WristSideDirectionMS_UE, OpenXRSideDirection, OpenXRForwardDirection);

	FRotator Difference = (TwistRotation * AlignmentRot).Rotator();

	MappedBonePairs.AdjustmentQuat = (TwistRotation * AlignmentRot).GetNormalized();

}

void FAnimNode_ApplyOpenXRHandPose::ConvertHandTransformsSpace(TArray<FTransform>& OutTransforms, const TArray<FTransform>& WorldTransforms, FTransform AddTrans, bool bMirrorLeftRight, bool bMergeMissingUE4Bones)
{

	if (WorldTransforms.Num() < EHandKeypointCount)
		return;

	if (OutTransforms.Num() < WorldTransforms.Num())
	{
		OutTransforms.Empty(WorldTransforms.Num());
		OutTransforms.AddUninitialized(WorldTransforms.Num());
	}

	TArray<FTransform> TempWorldTransforms = WorldTransforms;

	AddTrans.NormalizeRotation();

	int32 BoneParents[26] =
	{

		1,	
		-1,	
		1,	
		2,	
		3,	
		4,	

		1,	
		6,	
		7,	
		8,	
		9,	

		1,	
		11,	
		12,	
		13,	
		14,	

		1,	
		16,	
		17,	
		18,	
		19,	

		1,	
		21,	
		22,	
		23,	
		24,	
	};

	bool bUseAutoCalculatedRetarget = AddTrans.Equals(FTransform::Identity);

	for (int32 Index = 0; Index < EHandKeypointCount; ++Index)
	{
		if (TempWorldTransforms[Index].ContainsNaN() || TempWorldTransforms[Index].Equals(FTransform::Identity))
		{
			OutTransforms[Index] = FTransform::Identity;

		}

		TempWorldTransforms[Index].NormalizeRotation();

		if (bMirrorLeftRight)
		{
			TempWorldTransforms[Index].Mirror(EAxis::Y, EAxis::Y);
		}

		if (bUseAutoCalculatedRetarget)
		{
			TempWorldTransforms[Index].ConcatenateRotation(MappedBonePairs.AdjustmentQuat);

		}
		else
		{
			TempWorldTransforms[Index].ConcatenateRotation(AddTrans.GetRotation());
		}
	}

	for (int32 Index = 0; Index < EHandKeypointCount; ++Index)
	{
		FTransform& BoneTransform = TempWorldTransforms[Index];

		int32 ParentIndex = BoneParents[Index];
		int32 ParentParent = -1;

		if (bMergeMissingUE4Bones)
		{
			if (Index != (int32)EXRHandJointType::OXR_HAND_JOINT_THUMB_PROXIMAL_EXT && ParentIndex > 0)
			{
				ParentParent = BoneParents[ParentIndex];
			}
		}

		if (ParentIndex < 0)
		{

			OutTransforms[Index] = BoneTransform;
		}
		else
		{
			FTransform ParentTransform = FTransform::Identity;

			if (bMergeMissingUE4Bones && ParentParent == 1) 
			{
				ParentTransform = TempWorldTransforms[ParentParent];
			}
			else
			{
				ParentTransform = TempWorldTransforms[ParentIndex];
			}

			OutTransforms[Index] = BoneTransform.GetRelativeTransform(ParentTransform);
		}
	}
}

void FAnimNode_ApplyOpenXRHandPose::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	if (!MappedBonePairs.bInitialized)
		return;

	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	UObject* OwningAsset = BoneContainer.GetAsset();
	if (!OwningAsset)
		return;

	if ((OwningAsset->GetFName() != MappedBonePairs.LastInitializedName || SkeletonType != MappedBonePairs.LastInitializedSkeleton))
	{
		InitializeBoneReferences(BoneContainer);
	}

	FBPOpenXRActionSkeletalData *StoredActionInfoPtr = nullptr;
	if (bIsOpenInputAnimationInstance)
	{
		 FOpenXRAnimInstanceProxy* OpenXRAnimInstance = (FOpenXRAnimInstanceProxy*)Output.AnimInstanceProxy;
		if (OpenXRAnimInstance->HandSkeletalActionData.Num())
		{
			for (int i = 0; i <OpenXRAnimInstance->HandSkeletalActionData.Num(); ++i)
			{
				EVRSkeletalHandIndex TargetHand = OpenXRAnimInstance->HandSkeletalActionData[i].TargetHand;

				if (OpenXRAnimInstance->HandSkeletalActionData[i].bMirrorLeftRight)
				{
					TargetHand = (TargetHand == EVRSkeletalHandIndex::EActionHandIndex_Left) ? EVRSkeletalHandIndex::EActionHandIndex_Right : EVRSkeletalHandIndex::EActionHandIndex_Left;
				}

				if (TargetHand == MappedBonePairs.TargetHand)
				{
					StoredActionInfoPtr = &OpenXRAnimInstance->HandSkeletalActionData[i];
					break;
				}
			}
		}
	}

	if (StoredActionInfoPtr == nullptr || !StoredActionInfoPtr->SkeletalTransforms.Num())
	{
		StoredActionInfoPtr = &OptionalStoredActionInfo;
	}

	if (!StoredActionInfoPtr->bHasValidData)
	{
		return;
	}

	const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
	uint8 BoneTransIndex = 0;
	uint8 NumBones = StoredActionInfoPtr ? StoredActionInfoPtr->SkeletalTransforms.Num() : 0;

	if (NumBones < 1)
	{

		return;
	}

	FTransform trans = FTransform::Identity;
	OutBoneTransforms.Reserve(MappedBonePairs.BonePairs.Num());
	TArray<FBoneTransform> TransBones;
	FTransform AdditionTransform = StoredActionInfoPtr->AdditionTransform;

	FTransform TempTrans = FTransform::Identity;
	FTransform ParentTrans = FTransform::Identity;
	FTransform * ParentTransPtr = nullptr;

	TArray<FTransform> HandTransforms;
	ConvertHandTransformsSpace(HandTransforms, StoredActionInfoPtr->SkeletalTransforms, AdditionTransform, StoredActionInfoPtr->bMirrorLeftRight, MappedBonePairs.bMergeMissingBonesUE4);

	for (const FBPOpenXRSkeletalPair& BonePair : MappedBonePairs.BonePairs)
	{
		BoneTransIndex = (int8)BonePair.OpenXRBone;
		ParentTrans = FTransform::Identity;

		if (bSkipRootBone && BonePair.OpenXRBone == EXRHandJointType::OXR_HAND_JOINT_WRIST_EXT)
			continue;

		if (BoneTransIndex >= NumBones || BonePair.ReferenceToConstruct.CachedCompactPoseIndex == INDEX_NONE)
			continue;		

		if (!BonePair.ReferenceToConstruct.IsValidToEvaluate(BoneContainer))
		{
			continue;
		}

		trans = Output.Pose.GetComponentSpaceTransform(BonePair.ReferenceToConstruct.CachedCompactPoseIndex);

		if (BonePair.ParentReference != INDEX_NONE)
		{
			ParentTrans = Output.Pose.GetComponentSpaceTransform(BonePair.ParentReference);
			ParentTrans.SetScale3D(FVector(1.f));
		}

		EXRHandJointType CurrentBone = (EXRHandJointType)BoneTransIndex;
		TempTrans = (HandTransforms[BoneTransIndex]);

		TempTrans = TempTrans * ParentTrans;

		if (StoredActionInfoPtr->bAllowDeformingMesh || bOnlyApplyWristTransform)
			trans.SetTranslation(TempTrans.GetTranslation());

		trans.SetRotation(TempTrans.GetRotation());

		TransBones.Add(FBoneTransform(BonePair.ReferenceToConstruct.CachedCompactPoseIndex, trans));

		if (TransBones.Num())
		{
			Output.Pose.LocalBlendCSBoneTransforms(TransBones, BlendWeight);
			TransBones.Reset();
		}

		if (bOnlyApplyWristTransform && CurrentBone == EXRHandJointType::OXR_HAND_JOINT_WRIST_EXT)
		{
			break; 
		}
	}
}

bool FAnimNode_ApplyOpenXRHandPose::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return(MappedBonePairs.BonePairs.Num() > 0);
}
