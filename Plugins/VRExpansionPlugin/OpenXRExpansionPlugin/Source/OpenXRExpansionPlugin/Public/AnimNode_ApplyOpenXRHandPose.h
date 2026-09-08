

#pragma once
#include "CoreMinimal.h"
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_SkeletalControlBase.h"
#include "OpenXRExpansionTypes.h"

#include "AnimNode_ApplyOpenXRHandPose.generated.h"

USTRUCT()
struct OPENXREXPANSIONPLUGIN_API FAnimNode_ApplyOpenXRHandPose : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

public:

	FVector WristForwardLS_UE;
	FVector WristSideDirectionLS;

	UPROPERTY(EditAnywhere, Category = Skeletal, meta = (PinShownByDefault))
		EVROpenXRSkeletonType SkeletonType;

	UPROPERTY(EditAnywhere, Category = Skeletal, meta = (PinShownByDefault))
		bool bSkipRootBone;

	UPROPERTY(EditAnywhere, Category = Skeletal, meta = (PinShownByDefault))
		bool bOnlyApplyWristTransform;

	UPROPERTY(EditAnywhere, Category = Skeletal, meta = (PinShownByDefault))
		FBPOpenXRActionSkeletalData OptionalStoredActionInfo;

	UPROPERTY(EditAnywhere, Category = Skeletal, meta = (PinHiddenByDefault))
		FBPOpenXRSkeletalMappingData MappedBonePairs;

	bool bIsOpenInputAnimationInstance = false;
	bool bIsMirroringHand = false;

	void ConvertHandTransformsSpace(TArray<FTransform>& OutTransforms, const TArray<FTransform>& WorldTransforms, FTransform AddTrans, bool bMirrorLeftRight, bool bMergeMissingUE4Bones);

	void CalculateSkeletalAdjustment(USkeleton* AssetSkeleton);
	void CalculateOpenXRAdjustment();

	FQuat CalcRotationAboutAxis(const FVector& FromDirection, const FVector& ToDirection, const FVector& Axis)
	{
		FVector FromDirectionCp = FVector::CrossProduct(Axis, FromDirection);
		FVector ToDirectionCp = FVector::CrossProduct(Axis, ToDirection);

		return FQuat::FindBetweenVectors(FromDirectionCp, ToDirectionCp);
	}

	FTransform GetRefBoneInCS(TArray<FTransform>& RefBones, TArray<FMeshBoneInfo>& RefBonesInfo, int32 BoneIndex)
	{
		FTransform BoneTransform;

		if (BoneIndex >= 0)
		{
			BoneTransform = RefBones[BoneIndex];
			if (RefBonesInfo[BoneIndex].ParentIndex >= 0)
			{
				BoneTransform *= GetRefBoneInCS(RefBones, RefBonesInfo, RefBonesInfo[BoneIndex].ParentIndex);
			}
		}

		return BoneTransform;
	}

	void SetVectorToMaxElement(FVector& vec)
	{
		FVector absVal = vec.GetAbs();
		if (absVal.X > absVal.Y && absVal.X > absVal.Z)
		{
			vec = vec.GetSignVector() * FVector(1.0f, 0.f, 0.f);
			vec.Normalize();
		}
		else if (absVal.Y > absVal.X && absVal.Y > absVal.Z)
		{
			vec = vec.GetSignVector() * FVector(0.0f, 1.f, 0.f);
			vec.Normalize();
		}
		else if (absVal.Z > absVal.X && absVal.Z > absVal.Y)
		{
			vec = vec.GetSignVector() * FVector(0.0f, 0.f, 1.f);
			vec.Normalize();
		}
		else
		{
			vec.Normalize();
		}
	}

	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;

	virtual void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;
	virtual bool NeedsOnInitializeAnimInstance() const override { return true; }
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;

	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)  override;

	FAnimNode_ApplyOpenXRHandPose();

protected:
	bool WorldIsGame;
	AActor* OwningActor;

private:
};