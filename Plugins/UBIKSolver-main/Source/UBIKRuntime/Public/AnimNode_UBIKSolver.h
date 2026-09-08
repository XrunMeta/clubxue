

#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "UBIK.h"
#include "AnimNode_UBIKSolver.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct UBIKRUNTIME_API FAnimNode_UBIKSolver : public FAnimNode_SkeletalControlBase
{
    GENERATED_BODY()
public:
    FAnimNode_UBIKSolver();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UBIK, meta = (PinShownByDefault))
    FTransform InHeadTransformWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UBIK, meta = (PinShownByDefault))
    FTransform InLeftHandTransformWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UBIK, meta = (PinShownByDefault))
    FTransform InRightHandTransformWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UBIK, meta = (PinShownByDefault))
    bool bApplyHeadTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UBIK, meta = (PinShownByDefault))
    bool bApplyRightHandTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UBIK, meta = (PinShownByDefault))
    bool bApplyLeftHandTransform;

    UPROPERTY(EditAnywhere, Category = UBIK)
    bool bIgnorePelvisLocation;

    UPROPERTY(EditAnywhere, Category = UBIK, meta = (InlineEditConditionToggle))
    bool bApplyBoneAxis;

    UPROPERTY(EditAnywhere, Category = UBIK, meta = (EditCondition="bApplyBoneAxis"))
    TEnumAsByte<EBoneAxis> BoneAxis;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UBIK, meta = (PinShownByDefault))
    FUBIKSettings Settings;

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference HeadBoneToModify = FBoneReference("head");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference LeftClavicleBoneToModify = FBoneReference("clavicle_l");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference RightClavicleBoneToModify = FBoneReference("clavicle_r");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference LeftUpperArmBoneToModify = FBoneReference("upperarm_l");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference RightUpperArmBoneToModify = FBoneReference("upperarm_r");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference LeftLowerArmBoneToModify = FBoneReference("lowerarm_l");;

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference RightLowerArmBoneToModify = FBoneReference("lowerarm_r");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference LeftHandBoneToModify = FBoneReference("hand_l");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference RightHandBoneToModify = FBoneReference("hand_r");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference Spine01_BoneToModify = FBoneReference("spine_01");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference Spine02_BoneToModify = FBoneReference("spine_02");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference Spine03_BoneToModify = FBoneReference("spine_03");

    UPROPERTY(EditAnywhere, Category = Bones)
    FBoneReference PelvisBoneToModify = FBoneReference("pelvis");

    UPROPERTY(EditAnywhere, Category = Debug)
    bool bDrawDebug;

    virtual void GatherDebugData(FNodeDebugData& DebugData) override;

    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
    virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
    virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
    virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
    virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;

private:

    bool bAdjustOculusOffset = false;

    TArray<FBoneReference> AllBones;

    UPROPERTY(Transient)
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY(Transient)
    UWorld* World;

    float CachedDeltaTime;
private:
    FTransform LeftHandTransformWorld;
    FTransform RightHandTransformWorld;

    FTransform ComponentSpaceWorld;
    FTransform ShoulderTransformWorld;
    FTransform LeftUpperArmTransformWorld;
    FTransform RightUpperArmTransformWorld;
    FTransform LeftLowerArmTransformWorld;
    FTransform RightLowerArmTransformWorld;

    FTransform HeadTransformComponentSpace;
    FTransform LeftHandTransformComponentSpace;
    FTransform RightHandTransformComponentSpace;
    FTransform ShoulderTransformComponentSpace;
    FTransform LeftClavicleComponentSpace; 
    FTransform RightClavicleComponentSpace; 
    FTransform BaseCharTransformComponentSpace;
    FTransform LeftUpperArmTransformComponentSpace;
    FTransform LeftLowerArmTransformComponentSpace;
    FTransform RightUpperArmTransformComponentSpace;
    FTransform RightLowerArmTransformComponentSpace;

    FTransform ShoulderTransform;
    FTransform ComponentSpace;

    FTransform HeadTransformS;
    FTransform LeftHandTransformS;
    FTransform RightHandTransformS;
    FTransform LeftUpperArmTransformS;
    FTransform RightUpperArmTransformS;
    FTransform LeftLowerArmTransformS;
    FTransform RightLowerArmTransformS;

    float LeftHeadHandAngle;
    float RightHeadHandAngle;

    float LeftElbowHandAngle;
    float RightElbowHandAngle;

    FRotator HeadRotation;
    FRotator Spine03_Rotation;
    FRotator Spine02_Rotation;
    FRotator Spine01_Rotation;
    FTransform PelvisRotation;
    FRotator ClavicleLRotation;
    FRotator UpperArmLRotation;
    FRotator LowerArmLRotation;
    FRotator HandLRotation;
    FRotator ClavicleRRotation;
    FRotator UpperArmRRotation;
    FRotator LowerArmRRotation;
    FRotator HandRRotation;

private:
    void ConvertTransforms();
    void SetShoulder();
    FVector GetShoulderLocation();
    FRotator GetShoulderRotationFromHead();
    FRotator GetShoulderRotationFromHands();
    float GetHeadHandAngle(float LastAngle, const FVector& Hand, const FVector& HandHeadDelta);

    void SetLeftUpperArm();
    void SetRightUpperArm();
    FTransform RotateUpperArm(bool IsLeftArm, const FVector& HandTranslation);

    void ResetUpperArmsLocation();
    void SolveArms();
    void SetElbowBasePosition(const FVector& UpperArm, const FVector& Hand, bool bIsLeftArm, FTransform& UpperArmTransform,
                              FTransform& LowerArmTransform);
    float RotateElbowByHandPosition(const FVector& Hand, bool bIsLeftArm);
    float RotateElbowByHandRotation(const FTransform& LowerArm, FRotator Hand);
    void RotateElbow(float Angle, const FTransform& UpperArm, const FTransform& LowerArm, const FVector& HandLoc, bool bIsLeftArm,
                     FTransform& NewUpperArm, FTransform& NewLowerArm);

    FTransform GetBaseCharTransform();

    void DrawDebug(FAnimInstanceProxy* AnimInstanceProxy);
    void DebugDrawAxes(FAnimInstanceProxy* AnimInstanceProxy, const FTransform& Transform, bool DrawAxis = true,
                       FColor SphereColor = FColor::Silver);

    void SetBoneTransform(TArray<FBoneTransform>& OutBoneTransforms, const FBoneReference& BoneToModify, const FTransform& InTransform,
                          FComponentSpacePoseContext& Output, const FBoneContainer& BoneContainer, bool bApplyRotation,
                          bool bApplyTranslation = false);

    FVector GetAxisVector(const EBoneAxis Axis) const;
};
