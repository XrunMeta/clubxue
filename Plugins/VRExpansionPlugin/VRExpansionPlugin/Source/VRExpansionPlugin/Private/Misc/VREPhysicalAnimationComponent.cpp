#include "Misc/VREPhysicalAnimationComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VREPhysicalAnimationComponent)

#include "SceneManagement.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/ShapeElem.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "ReferenceSkeleton.h"
#include "Engine/SkinnedAsset.h"
#include "DrawDebugHelpers.h"

#if UE_ENABLE_DEBUG_DRAWING
#include "Chaos/ImplicitObject.h"
#include "Chaos/TriangleMeshImplicitObject.h"
#include "Chaos/ShapeInstance.h"
#include "Chaos/DebugDrawQueue.h"
#endif

#include "Physics/PhysicsInterfaceCore.h"
#include "Physics/PhysicsInterfaceTypes.h"

UVREPhysicalAnimationComponent::UVREPhysicalAnimationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoSetPhysicsSleepSensitivity = true;
	SleepThresholdMultiplier = 0.0f;
}

void UVREPhysicalAnimationComponent::SetWeldedBoneDriverPaused(bool bPaused)
{
	bIsPaused = bPaused;
}

bool UVREPhysicalAnimationComponent::IsWeldedBoneDriverPaused()
{
	return bIsPaused;
}

void UVREPhysicalAnimationComponent::RefreshWeldedBoneDriver()
{
	SetupWeldedBoneDriver_Implementation(true);
}

void UVREPhysicalAnimationComponent::SetupWeldedBoneDriver(TArray<FName> BaseBoneNames)
{
	if (BaseBoneNames.Num())
		BaseWeldedBoneDriverNames = BaseBoneNames;

	SetupWeldedBoneDriver_Implementation(false);
}

FTransform UVREPhysicalAnimationComponent::GetWorldSpaceRefBoneTransform(FReferenceSkeleton& RefSkel, int32 BoneIndex, int32 ParentBoneIndex)
{
	FTransform BoneTransform;

	if (BoneIndex > 0 && BoneIndex != ParentBoneIndex)
	{
		BoneTransform = RefSkel.GetRefBonePose()[BoneIndex];

		FMeshBoneInfo BoneInfo = RefSkel.GetRefBoneInfo()[BoneIndex];
		if (BoneInfo.ParentIndex != 0 && BoneInfo.ParentIndex != ParentBoneIndex)
		{
			BoneTransform *= GetWorldSpaceRefBoneTransform(RefSkel, BoneInfo.ParentIndex, ParentBoneIndex);
		}
	}

	return BoneTransform;
}

FTransform UVREPhysicalAnimationComponent::GetRefPoseBoneRelativeTransform(USkeletalMeshComponent* SkeleMesh, FName BoneName, FName ParentBoneName)
{
	FTransform BoneTransform;

	if (SkeleMesh && !BoneName.IsNone() && !ParentBoneName.IsNone())
	{

		FReferenceSkeleton RefSkel;
		RefSkel = SkeleMesh->GetSkinnedAsset()->GetRefSkeleton();

		BoneTransform = GetWorldSpaceRefBoneTransform(RefSkel, RefSkel.FindBoneIndex(BoneName), RefSkel.FindBoneIndex(ParentBoneName));
	}

	return BoneTransform;
}

void UVREPhysicalAnimationComponent::SetupWeldedBoneDriver_Implementation(bool bReInit)
{
	TArray<FWeldedBoneDriverData> OriginalData;
	if (bReInit)
	{
		OriginalData = BoneDriverMap;
	}

	BoneDriverMap.Empty();

	USkeletalMeshComponent* SkeleMesh = GetSkeletalMesh();

	if (!SkeleMesh || !SkeleMesh->Bodies.Num())
		return;

	UPhysicsAsset* PhysAsset = SkeleMesh ? SkeleMesh->GetPhysicsAsset() : nullptr;
	if (PhysAsset && SkeleMesh->GetSkinnedAsset())
	{

		for (FName BaseWeldedBoneDriverName : BaseWeldedBoneDriverNames)
		{
			int32 ParentBodyIdx = PhysAsset->FindBodyIndex(BaseWeldedBoneDriverName);

			if (FBodyInstance* ParentBody = (ParentBodyIdx == INDEX_NONE ? nullptr : SkeleMesh->Bodies[ParentBodyIdx]))
			{

				FPhysicsActorHandle& ActorHandle = ParentBody->WeldParent ? ParentBody->WeldParent->GetPhysicsActorHandle() : ParentBody->GetPhysicsActorHandle();

				if (FPhysicsInterface::IsValid(ActorHandle) )
				{
					FPhysicsCommand::ExecuteWrite(ActorHandle, [&](FPhysicsActorHandle& Actor)
					{

						PhysicsInterfaceTypes::FInlineShapeArray Shapes;
						FPhysicsInterface::GetAllShapes_AssumedLocked(Actor, Shapes);

						for (FPhysicsShapeHandle& Shape : Shapes)
						{
							if (ParentBody->WeldParent)
							{
								const FBodyInstance* OriginalBI = ParentBody->WeldParent->GetOriginalBodyInstance(Shape);

								if (OriginalBI != ParentBody)
								{

									continue;
								}
							}

							FKShapeElem* ShapeElem = FChaosUserData::Get<FKShapeElem>(FPhysicsInterface::GetUserData(Shape));
							if (ShapeElem)
							{
								FName TargetBoneName = ShapeElem->GetName();
								int32 BoneIdx = SkeleMesh->GetBoneIndex(TargetBoneName);

								if (BoneIdx != INDEX_NONE)
								{
									FWeldedBoneDriverData DriverData;
									DriverData.BoneName = TargetBoneName;

									if (bReInit && OriginalData.Num() - 1 >= BoneDriverMap.Num())
									{
										DriverData.RelativeTransform = OriginalData[BoneDriverMap.Num()].RelativeTransform;
									}
									else
									{
										FTransform BoneTransform = FTransform::Identity;
										if (SkeleMesh->GetBoneIndex(TargetBoneName) != INDEX_NONE)
											BoneTransform = GetRefPoseBoneRelativeTransform(SkeleMesh, TargetBoneName, BaseWeldedBoneDriverName).Inverse();

										DriverData.RelativeTransform = FPhysicsInterface::GetLocalTransform(Shape) * BoneTransform;
									}

									BoneDriverMap.Add(DriverData);
								}
							}
						}

						if (bAutoSetPhysicsSleepSensitivity && !ParentBody->WeldParent && BoneDriverMap.Num() > 0)
						{
							ParentBody->SleepFamily = ESleepFamily::Custom;
							ParentBody->CustomSleepThresholdMultiplier = SleepThresholdMultiplier;
							float SleepEnergyThresh = FPhysicsInterface::GetSleepEnergyThreshold_AssumesLocked(Actor);
							SleepEnergyThresh *= ParentBody->GetSleepThresholdMultiplier();
							FPhysicsInterface::SetSleepEnergyThreshold_AssumesLocked(Actor, SleepEnergyThresh);
						}
					});
				}
			}
		}
	}
}

void UVREPhysicalAnimationComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateWeldedBoneDriver(DeltaTime);
}

void UVREPhysicalAnimationComponent::UpdateWeldedBoneDriver(float DeltaTime)
{
	if (!BoneDriverMap.Num())
		return;

	USkeletalMeshComponent* SkeleMesh = GetSkeletalMesh();

	if (!SkeleMesh || !SkeleMesh->Bodies.Num())
		return;

	UPhysicsAsset* PhysAsset = SkeleMesh ? SkeleMesh->GetPhysicsAsset() : nullptr;
	if(PhysAsset && SkeleMesh->GetSkinnedAsset())
	{
		for (FName BaseWeldedBoneDriverName : BaseWeldedBoneDriverNames)
		{
			int32 ParentBodyIdx = PhysAsset->FindBodyIndex(BaseWeldedBoneDriverName);

			if (FBodyInstance* ParentBody = (ParentBodyIdx == INDEX_NONE ? nullptr : SkeleMesh->Bodies[ParentBodyIdx]))
			{

				FPhysicsActorHandle& ActorHandle = ParentBody->WeldParent ? ParentBody->WeldParent->GetPhysicsActorHandle() : ParentBody->GetPhysicsActorHandle();

				if (FPhysicsInterface::IsValid(ActorHandle) )
				{

#if UE_ENABLE_DEBUG_DRAWING
					if (false)
					{
						Chaos::FDebugDrawQueue::GetInstance().SetConsumerActive(this, true); 
						Chaos::FDebugDrawQueue::GetInstance().SetMaxCost(20000);

						Chaos::FDebugDrawQueue::GetInstance().SetEnabled(true);
					}
#endif

					bool bModifiedBody = false;
					FPhysicsCommand::ExecuteWrite(ActorHandle, [&](FPhysicsActorHandle& Actor)
					{
						PhysicsInterfaceTypes::FInlineShapeArray Shapes;
						FPhysicsInterface::GetAllShapes_AssumedLocked(Actor, Shapes);

						FTransform GlobalPose = FPhysicsInterface::GetGlobalPose_AssumesLocked(ActorHandle);
						FTransform GlobalPoseInv = GlobalPose.Inverse();

#if UE_ENABLE_DEBUG_DRAWING
						if (false)
						{
							Chaos::FDebugDrawQueue::GetInstance().SetRegionOfInterest(GlobalPose.GetLocation(), 100.0f);
						}

#endif

						for (FPhysicsShapeHandle& Shape : Shapes)
						{
							if (ParentBody->WeldParent)
							{
								const FBodyInstance* OriginalBI = ParentBody->WeldParent->GetOriginalBodyInstance(Shape);

								if (OriginalBI != ParentBody)
								{

									continue;
								}
							}

							FName TargetBoneName = NAME_None;
							if (FKShapeElem* ShapeElem = FChaosUserData::Get<FKShapeElem>(FPhysicsInterface::GetUserData(Shape)))
							{
								TargetBoneName = ShapeElem->GetName();
							}
							else
							{

								continue;
							}

							if (FWeldedBoneDriverData* WeldedData = BoneDriverMap.FindByKey(TargetBoneName))
							{
								bModifiedBody = true;

								FTransform Trans = SkeleMesh->GetSocketTransform(WeldedData->BoneName, ERelativeTransformSpace::RTS_World);

								FTransform GlobalTransform = WeldedData->RelativeTransform * Trans;
								FTransform RelativeTM = GlobalTransform * GlobalPoseInv;

								RelativeTM.RemoveScaling();

								if (!WeldedData->LastLocal.Equals(RelativeTM))
								{
									FPhysicsInterface::SetLocalTransform(Shape, RelativeTM);
									WeldedData->LastLocal = RelativeTM;
								}
							}

#if UE_ENABLE_DEBUG_DRAWING
							if (false)
							{

							}
#endif
						}
					});

#if UE_ENABLE_DEBUG_DRAWING
					if (false)
					{

					}
#endif
				}

			}
		}
	}
}

#if UE_ENABLE_DEBUG_DRAWING

#endif