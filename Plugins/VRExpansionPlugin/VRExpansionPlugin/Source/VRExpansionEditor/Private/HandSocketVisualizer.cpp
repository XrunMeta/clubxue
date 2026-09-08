

#include "HandSocketVisualizer.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "SceneManagement.h"

#include "VRBPDatatypes.h"
#include "ScopedTransaction.h"
#include "Modules/ModuleManager.h"
#include "EditorViewportClient.h"
#include "Components/PoseableMeshComponent.h"
#include "Misc/PackageName.h"

IMPLEMENT_HIT_PROXY(HHandSocketVisProxy, HComponentVisProxy);
#define LOCTEXT_NAMESPACE "HandSocketVisualizer"

bool FHandSocketVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		if (VisProxy->IsA(HHandSocketVisProxy::StaticGetType()))
		{

			if( const UHandSocketComponent * HandComp = UpdateSelectedHandComponent(VisProxy))
			{
				HHandSocketVisProxy* Proxy = (HHandSocketVisProxy*)VisProxy;
				if (Proxy)
				{
					CurrentlySelectedBone = Proxy->TargetBoneName;
					CurrentlySelectedBoneIdx = Proxy->BoneIdx;
					TargetViewport = InViewportClient->Viewport;
				}
			}
		}
	}

	return bEditing;
}

bool FHandSocketVisualizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const
{
	if (TargetViewport == nullptr || TargetViewport != ViewportClient->Viewport)
	{
		return false;
	}

	if (HandPropertyPath.IsValid() && CurrentlySelectedBone != NAME_None)
	{
		if (CurrentlySelectedBone == "HandSocket")
		{
			UHandSocketComponent* CurrentlyEditingComponent = GetCurrentlyEditingComponent();
			if (CurrentlyEditingComponent)
			{
				if (CurrentlyEditingComponent->bMirrorVisualizationMesh)
				{
					FTransform NewTrans = CurrentlyEditingComponent->GetRelativeTransform();
					NewTrans.Mirror(CurrentlyEditingComponent->GetAsEAxis(CurrentlyEditingComponent->MirrorAxis), CurrentlyEditingComponent->GetAsEAxis(CurrentlyEditingComponent->FlipAxis));

					if (USceneComponent* ParentComp = CurrentlyEditingComponent->GetAttachParent())
					{
						NewTrans = NewTrans * ParentComp->GetComponentTransform();
					}

					OutMatrix = FRotationMatrix::Make(NewTrans.GetRotation());
				}
			}

			return false;
		}
		else if (CurrentlySelectedBone == "Visualizer")
		{
			if (UHandSocketComponent* CurrentlyEditingComponent = GetCurrentlyEditingComponent())
			{

				FTransform newTrans = FTransform::Identity;
				if (CurrentlyEditingComponent->bDecoupleMeshPlacement)
				{
					if (USceneComponent* ParentComp = CurrentlyEditingComponent->GetAttachParent())
					{
						newTrans = CurrentlyEditingComponent->HandRelativePlacement * ParentComp->GetComponentTransform();
					}
				}
				else
				{
					newTrans = CurrentlyEditingComponent->GetHandRelativePlacement() * CurrentlyEditingComponent->GetComponentTransform();
				}

				OutMatrix = FRotationMatrix::Make(newTrans.GetRotation());
			}
		}
		else
		{
			if (UHandSocketComponent* CurrentlyEditingComponent = GetCurrentlyEditingComponent())
			{
				if (IsValid(CurrentlyEditingComponent->HandVisualizerComponent))
				{
					FTransform newTrans = CurrentlyEditingComponent->HandVisualizerComponent->GetBoneTransform(CurrentlySelectedBoneIdx);
					FQuat Rot = newTrans.GetRotation();
					if (!newTrans.GetRotation().ContainsNaN())
					{
						Rot.Normalize();
						OutMatrix = FRotationMatrix::Make(Rot);
						return true;
					}
				}

				return false;
			}
		}

		return true;
	}

	return false;
}

bool FHandSocketVisualizer::IsVisualizingArchetype() const
{
	return (HandPropertyPath.IsValid() && HandPropertyPath.GetParentOwningActor() && FActorEditorUtils::IsAPreviewOrInactiveActor(HandPropertyPath.GetParentOwningActor()));
}

void FHandSocketVisualizer::DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	if (TargetViewport == nullptr || TargetViewport != Viewport)
	{
		return;
	}

	if (const UHandSocketComponent* HandComp = Cast<const UHandSocketComponent>(Component))
	{
		if (CurrentlySelectedBone != NAME_None)
		{
			if (UHandSocketComponent* CurrentlyEditingComponent = GetCurrentlyEditingComponent())
			{
				if (!IsValid(CurrentlyEditingComponent->HandVisualizerComponent))
				{
					return;
				}

				int32 XL;
				int32 YL;
				const FIntRect CanvasRect = Canvas->GetViewRect();

				FPlane location = View->Project(CurrentlyEditingComponent->HandVisualizerComponent->GetBoneTransform(CurrentlySelectedBoneIdx).GetLocation());
				StringSize(GEngine->GetLargeFont(), XL, YL, *CurrentlySelectedBone.ToString());

				const float DrawPositionX = FMath::FloorToFloat(CanvasRect.Min.X + (CanvasRect.Width() - XL) * 0.5f);
				const float DrawPositionY = CanvasRect.Min.Y + 50.0f;
				Canvas->DrawShadowedString(DrawPositionX, DrawPositionY, *CurrentlySelectedBone.ToString(), GEngine->GetLargeFont(), FLinearColor::Yellow);

			}
		}
	}
}

void FHandSocketVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{

	if (const UHandSocketComponent* HandComponent = Cast<UHandSocketComponent>(Component))
	{
		if (!HandComponent->HandVisualizerComponent)
			return;

		const FLinearColor SelectedColor = FLinearColor::Yellow;
		const FLinearColor UnselectedColor = FLinearColor::White;
		const FVector Location = HandComponent->HandVisualizerComponent->GetComponentLocation();
		float BoneScale = 1.0f - ((View->ViewLocation - Location).SizeSquared() / FMath::Square(100.0f));
		BoneScale = FMath::Clamp(BoneScale, 0.2f, 1.0f);
		HHandSocketVisProxy* newHitProxy = new HHandSocketVisProxy(Component);
		newHitProxy->TargetBoneName = "Visualizer";
		PDI->SetHitProxy(newHitProxy);
		PDI->DrawPoint(Location, CurrentlySelectedBone == newHitProxy->TargetBoneName ? SelectedColor : FLinearColor::Red, 20.f * BoneScale, SDPG_Foreground);
		PDI->SetHitProxy(NULL);
		newHitProxy = nullptr;

		newHitProxy = new HHandSocketVisProxy(Component);
		newHitProxy->TargetBoneName = "HandSocket";
		BoneScale = 1.0f - ((View->ViewLocation - HandComponent->GetComponentLocation()).SizeSquared() / FMath::Square(100.0f));
		BoneScale = FMath::Clamp(BoneScale, 0.2f, 1.0f);
		PDI->SetHitProxy(newHitProxy);
		PDI->DrawPoint(HandComponent->GetComponentLocation(), FLinearColor::Green, 20.f * BoneScale, SDPG_Foreground);
		PDI->SetHitProxy(NULL);
		newHitProxy = nullptr;

		if (HandComponent->bUseCustomPoseDeltas)
		{
			TArray<FTransform> BoneTransforms = HandComponent->HandVisualizerComponent->GetBoneSpaceTransforms();
			FTransform ParentTrans = HandComponent->HandVisualizerComponent->GetComponentTransform();

			for (int i = 1; i < HandComponent->HandVisualizerComponent->GetNumBones(); i++)
			{

				FName BoneName = HandComponent->HandVisualizerComponent->GetBoneName(i);

				if (HandComponent->bFilterBonesByPostfix)
				{
					if (BoneName.ToString().Right(2) != HandComponent->FilterPostfix)
					{

						continue;
					}
				}

				if (HandComponent->BonesToSkip.Contains(BoneName))
				{

					continue;
				}

				FTransform BoneTransform = HandComponent->HandVisualizerComponent->GetBoneTransform(i);
				FVector BoneLoc = BoneTransform.GetLocation();
				BoneScale = 1.0f - ((View->ViewLocation - BoneLoc).SizeSquared() / FMath::Square(100.0f));
				BoneScale = FMath::Clamp(BoneScale, 0.1f, 0.9f);
				newHitProxy = new HHandSocketVisProxy(Component);
				newHitProxy->TargetBoneName = BoneName;
				newHitProxy->BoneIdx = i;
				PDI->SetHitProxy(newHitProxy);
				PDI->DrawPoint(BoneLoc, CurrentlySelectedBone == newHitProxy->TargetBoneName ? SelectedColor : UnselectedColor, 20.f * BoneScale, SDPG_Foreground);
				PDI->SetHitProxy(NULL);
				newHitProxy = nullptr;
			}
		}

		if (HandComponent->bShowRangeVisualization)
		{
			float RangeVisualization = HandComponent->OverrideDistance;

			if (RangeVisualization <= 0.0f)
			{
				if (USceneComponent* Parent = Cast<USceneComponent>(HandComponent->GetAttachParent()))
				{
					FStructProperty* ObjectProperty = CastField<FStructProperty>(Parent->GetClass()->FindPropertyByName("VRGripInterfaceSettings"));

					AActor* ParentsActor = nullptr;
					if (!ObjectProperty)
					{
						ParentsActor = Parent->GetOwner();
						if (ParentsActor)
						{
							ObjectProperty = CastField<FStructProperty>(Parent->GetOwner()->GetClass()->FindPropertyByName("VRGripInterfaceSettings"));
						}
					}

					if (ObjectProperty)
					{
						UObject* Target = ParentsActor;

						if (Target == nullptr)
						{
							Target = Parent;
						}

						if (const FBPInterfaceProperties* Curve = ObjectProperty->ContainerPtrToValuePtr<FBPInterfaceProperties>(Target))
						{
							if (HandComponent->SlotPrefix == "VRGripS")
							{
								RangeVisualization = Curve->SecondarySlotRange;
							}
							else
							{
								RangeVisualization = Curve->PrimarySlotRange;
							}
						}
					}
				}
			}

			FBox BoxToDraw = FBox::BuildAABB(FVector::ZeroVector, FVector(RangeVisualization) * HandComponent->GetAttachParent()->GetComponentScale());
			BoxToDraw.Min += HandComponent->GetComponentLocation();
			BoxToDraw.Max += HandComponent->GetComponentLocation();

			DrawWireBox(PDI, BoxToDraw, FColor::Green, 0.0f);
		}
	}
}

bool FHandSocketVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (TargetViewport == nullptr || TargetViewport != ViewportClient->Viewport)
	{
		return false;
	}

	if (HandPropertyPath.IsValid() && CurrentlySelectedBone != NAME_None && CurrentlySelectedBone != "HandSocket")
	{
		if (CurrentlySelectedBone == "HandSocket")
		{
			return false;
		}
		else if (CurrentlySelectedBone == "Visualizer")
		{
			if (UHandSocketComponent* CurrentlyEditingComponent = GetCurrentlyEditingComponent())
			{
				FTransform newTrans = FTransform::Identity;
				if (CurrentlyEditingComponent->bDecoupleMeshPlacement)
				{
					if (USceneComponent* ParentComp = CurrentlyEditingComponent->GetAttachParent())
					{
						newTrans = CurrentlyEditingComponent->HandRelativePlacement * ParentComp->GetComponentTransform();
					}
				}
				else
				{
					newTrans = CurrentlyEditingComponent->GetHandRelativePlacement() * CurrentlyEditingComponent->GetComponentTransform();
				}

				OutLocation = newTrans.GetLocation();
			}
		}
		else
		{
			if (UHandSocketComponent* CurrentlyEditingComponent = GetCurrentlyEditingComponent())
			{
				if (IsValid(CurrentlyEditingComponent->HandVisualizerComponent))
				{
					OutLocation = CurrentlyEditingComponent->HandVisualizerComponent->GetBoneTransform(CurrentlySelectedBoneIdx).GetLocation();
				}
				else
				{
					return false;
				}
			}
		}

		return true;
	}

	return false;
}

bool FHandSocketVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{

	if (TargetViewport == nullptr || TargetViewport != Viewport)
	{
		return false;
	}

	bool bHandled = false;

	if (HandPropertyPath.IsValid())
	{
		if (CurrentlySelectedBone == "HandSocket" || CurrentlySelectedBone == NAME_None)
		{
			bHandled = false;
		}
		else if (CurrentlySelectedBone == "Visualizer")
		{
			const FScopedTransaction Transaction(LOCTEXT("ChangingComp", "ChangingComp"));

			UHandSocketComponent* CurrentlyEditingComponent = GetCurrentlyEditingComponent();
			if (!CurrentlyEditingComponent)
			{
				return false;
			}

			CurrentlyEditingComponent->Modify();
			if (AActor* Owner = CurrentlyEditingComponent->GetOwner())
			{
				Owner->Modify();
			}
			bool bLevelEdit = ViewportClient->IsLevelEditorClient();

			FTransform CurrentTrans = FTransform::Identity;

			if (CurrentlyEditingComponent->bDecoupleMeshPlacement)
			{
				if (USceneComponent* ParentComp = CurrentlyEditingComponent->GetAttachParent())
				{
					CurrentTrans = CurrentlyEditingComponent->HandRelativePlacement * ParentComp->GetComponentTransform();
				}
			}
			else
			{
				CurrentTrans = CurrentlyEditingComponent->GetHandRelativePlacement() * CurrentlyEditingComponent->GetComponentTransform();
			}

			if (!DeltaTranslate.IsNearlyZero())
			{
				CurrentTrans.AddToTranslation(DeltaTranslate);
			}

			if (!DeltaRotate.IsNearlyZero())
			{
				CurrentTrans.SetRotation(DeltaRotate.Quaternion() * CurrentTrans.GetRotation());
			}

			if (!DeltaScale.IsNearlyZero())
			{
				CurrentTrans.MultiplyScale3D(DeltaScale);
			}

			if (CurrentlyEditingComponent->bDecoupleMeshPlacement)
			{
				if (USceneComponent* ParentComp = CurrentlyEditingComponent->GetAttachParent())
				{
					CurrentlyEditingComponent->HandRelativePlacement = CurrentTrans.GetRelativeTransform(ParentComp->GetComponentTransform());
				}
			}
			else
			{
				CurrentlyEditingComponent->HandRelativePlacement = CurrentTrans.GetRelativeTransform(CurrentlyEditingComponent->GetComponentTransform());
			}

			NotifyPropertyModified(CurrentlyEditingComponent, FindFProperty<FProperty>(UHandSocketComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UHandSocketComponent, HandRelativePlacement)));

			bHandled = true;

		}
		else
		{
			UHandSocketComponent* CurrentlyEditingComponent = GetCurrentlyEditingComponent();
			if (!CurrentlyEditingComponent || !CurrentlyEditingComponent->HandVisualizerComponent)
			{
				return false;
			}

			const FScopedTransaction Transaction(LOCTEXT("ChangingComp", "ChangingComp"));

			CurrentlyEditingComponent->Modify();
			if (AActor* Owner = CurrentlyEditingComponent->GetOwner())
			{
				Owner->Modify();
			}
			bool bLevelEdit = ViewportClient->IsLevelEditorClient();

			FTransform BoneTrans = CurrentlyEditingComponent->HandVisualizerComponent->GetBoneTransform(CurrentlySelectedBoneIdx);
			FTransform NewTrans = BoneTrans;
			NewTrans.SetRotation(DeltaRotate.Quaternion() * NewTrans.GetRotation());

			FQuat DeltaRotateMod = NewTrans.GetRelativeTransform(BoneTrans).GetRotation();
			bool bFoundBone = false;
			for (FBPVRHandPoseBonePair& BonePair : CurrentlyEditingComponent->CustomPoseDeltas)
			{
				if (BonePair.BoneName == CurrentlySelectedBone)
				{
					bFoundBone = true;
					BonePair.DeltaPose *= DeltaRotateMod;
					break;
				}
			}

			if (!bFoundBone)
			{
				FBPVRHandPoseBonePair newBonePair;
				newBonePair.BoneName = CurrentlySelectedBone;
				newBonePair.DeltaPose *= DeltaRotateMod;
				CurrentlyEditingComponent->CustomPoseDeltas.Add(newBonePair);
				bFoundBone = true;
			}

			if (bFoundBone)
			{
				NotifyPropertyModified(CurrentlyEditingComponent, FindFProperty<FProperty>(UHandSocketComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UHandSocketComponent, CustomPoseDeltas)));
			}

			bHandled = true;
		}
	}

	return bHandled;
}

void FHandSocketVisualizer::EndEditing()
{
	HandPropertyPath = FComponentPropertyPath();
	CurrentlySelectedBone = NAME_None;
	CurrentlySelectedBoneIdx = INDEX_NONE;
	TargetViewport = nullptr;
}

#undef LOCTEXT_NAMESPACE