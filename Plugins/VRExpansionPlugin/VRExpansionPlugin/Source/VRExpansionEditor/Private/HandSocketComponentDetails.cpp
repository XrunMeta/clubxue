

#include "HandSocketComponentDetails.h"
#include "HandSocketVisualizer.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"

#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Editor/ContentBrowser/Public/IContentBrowserSingleton.h"
#include "Editor/ContentBrowser/Public/ContentBrowserModule.h"
#include "AnimationUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Styling/CoreStyle.h"

#include "Animation/AnimData/AnimDataModel.h"

#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "HandSocketComponentDetails"

FText SCreateHandAnimationDlg::LastUsedAssetPath;

static bool PromptUserForAssetPath(FString& AssetPath, FString& AssetName)
{
	TSharedRef<SCreateHandAnimationDlg> NewAnimDlg = SNew(SCreateHandAnimationDlg);
	if (NewAnimDlg->ShowModal() != EAppReturnType::Cancel)
	{
		AssetPath = NewAnimDlg->GetFullAssetPath();
		AssetName = NewAnimDlg->GetAssetName();
		return true;
	}

	return false;
}

TWeakObjectPtr<UAnimSequence> FHandSocketComponentDetails::SaveAnimationAsset(const FString& InAssetPath, const FString& InAssetName)
{

	TWeakObjectPtr<UAnimSequence> FinalAnimation;

	if (!HandSocketComponent.IsValid())
		return FinalAnimation;

	if (!HandSocketComponent->HandTargetAnimation && (!HandSocketComponent->VisualizationMesh || !HandSocketComponent->VisualizationMesh->GetSkeleton()))
	{
		return FinalAnimation;
	}

	FText InvalidPathReason;
	bool const bValidPackageName = FPackageName::IsValidLongPackageName(InAssetPath, false, &InvalidPathReason);
	if (bValidPackageName == false)
	{
		UE_LOGF(LogAnimation, Log, "%ls is an invalid asset path, prompting user for new asset path. Reason: %ls", *InAssetPath, *InvalidPathReason.ToString());
	}

	FString ValidatedAssetPath = InAssetPath;
	FString ValidatedAssetName = InAssetName;

	UObject* Parent = bValidPackageName ? CreatePackage(*ValidatedAssetPath) : nullptr;
	if (Parent == nullptr)
	{

		if (PromptUserForAssetPath(ValidatedAssetPath, ValidatedAssetName) == false)
		{
			return FinalAnimation;
		}

		Parent = CreatePackage(*ValidatedAssetPath);
	}

	UObject* const Object = LoadObject<UObject>(Parent, *ValidatedAssetName, nullptr, LOAD_Quiet, nullptr);

	if (Object)
	{
		EAppReturnType::Type ReturnValue = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("Error_AssetExist", "Asset with same name exists. Do you wish to overwrite it?"));
		if (ReturnValue == EAppReturnType::No)
		{
			return FinalAnimation; 
		}
	}

	UAnimSequence* BaseAnimation = HandSocketComponent->HandTargetAnimation;
	TArray<FTransform> LocalPoses;

	if (!BaseAnimation)
	{
		LocalPoses = HandSocketComponent->VisualizationMesh->GetRefSkeleton().GetRefBonePose();
	}

	UAnimSequence* const NewSeq = NewObject<UAnimSequence>(Parent, *ValidatedAssetName, RF_Public | RF_Standalone);
	if (NewSeq)
	{

		if (BaseAnimation)
		{
			NewSeq->SetSkeleton(BaseAnimation->GetSkeleton());
		}
		else
		{
			NewSeq->SetSkeleton(HandSocketComponent->VisualizationMesh->GetSkeleton());
		}

		FAssetRegistryModule::AssetCreated(NewSeq);

		UAnimSequence* AnimationObject = NewSeq;

		IAnimationDataController& AnimController = AnimationObject->GetController();
		{
			IAnimationDataController::FScopedBracket ScopedBracket(AnimController, LOCTEXT("SaveAnimationAsset_VRE", "Creating Animation Sequence based on hand pose"));
			AnimationObject->ResetAnimation();
			if (BaseAnimation)
			{
				AnimationObject->BoneCompressionSettings = BaseAnimation->BoneCompressionSettings;
			}
			else
			{
				AnimationObject->BoneCompressionSettings = FAnimationUtils::GetDefaultAnimationBoneCompressionSettings();
			}

			AnimController.InitializeModel();
			AnimController.RemoveAllBoneTracks(false);

			AnimController.SetNumberOfFrames(FFrameNumber(1), false);

			AnimController.SetFrameRate(FFrameRate(1, 1));

			TArray<FName> TrackNames;
			const IAnimationDataModel* BaseDataModel = BaseAnimation ? BaseAnimation->GetController().GetModel() : nullptr;

			if (BaseAnimation)
			{
				if (BaseDataModel)
				{
					BaseDataModel->GetBoneTrackNames(TrackNames);
					for (FName TrackName : TrackNames)
					{
						AnimController.AddBoneCurve(TrackName);
					}
				}
				else
				{
					return FinalAnimation;
				}
			}
			else
			{
				int numBones = HandSocketComponent->VisualizationMesh->GetRefSkeleton().GetNum();
				for (int i = 0; i < LocalPoses.Num() && i < numBones; ++i)
				{
					AnimController.AddBoneCurve(HandSocketComponent->VisualizationMesh->GetRefSkeleton().GetBoneName(i));

				}
			}

			if (BaseAnimation)
			{
				AnimationObject->RetargetSource = BaseAnimation->RetargetSource;
			}
			else
			{
				AnimationObject->RetargetSource = HandSocketComponent->VisualizationMesh ? HandSocketComponent->VisualizationMesh->GetSkeleton()->GetRetargetSourceForMesh(HandSocketComponent->VisualizationMesh) : NAME_None;
			}

			const IAnimationDataModel* DataModel = AnimController.GetModel();

			if (BaseAnimation && DataModel && BaseDataModel)
			{
				for (int32 TrackIndex = 0; TrackIndex < BaseDataModel->GetNumBoneTracks(); ++TrackIndex)
				{
					FName TrackName = TrackIndex < TrackNames.Num() ? TrackNames[TrackIndex] : NAME_None;
					if (!BaseDataModel->IsValidBoneTrackName(TrackName))
					{
						continue;
					}

					FTransform FinalTrans = BaseDataModel->GetBoneTrackTransform(TrackName, 0);

					FQuat DeltaQuat = FQuat::Identity;
					for (FBPVRHandPoseBonePair& HandPair : HandSocketComponent->CustomPoseDeltas)
					{
						if (HandPair.BoneName == TrackName)
						{
							DeltaQuat = HandPair.DeltaPose;
							break;
						}
					}

					FinalTrans.ConcatenateRotation(DeltaQuat);
					FinalTrans.NormalizeRotation();

					AnimController.SetBoneTrackKeys(TrackName, { FinalTrans.GetTranslation() }, { FinalTrans.GetRotation() }, { FinalTrans.GetScale3D() });
				}
			}
			else if(DataModel)
			{
				USkeletalMesh* SkeletalMesh = HandSocketComponent->VisualizationMesh;
				FReferenceSkeleton RefSkeleton = SkeletalMesh->GetRefSkeleton();	
				USkeleton* AnimSkeleton = SkeletalMesh->GetSkeleton();

				for (int32 TrackIndex = 0; TrackIndex < RefSkeleton.GetNum(); ++TrackIndex)
				{

					FName TrackName = RefSkeleton.GetBoneName(TrackIndex);
					if (!DataModel->IsValidBoneTrackName(TrackName))
					{
						continue;
					}

					int32 BoneTreeIndex = RefSkeleton.FindBoneIndex(TrackName);

					if (BoneTreeIndex != INDEX_NONE)
					{

						int32 BoneIndex = BoneTreeIndex;

						FTransform LocalTransform = LocalPoses[BoneIndex];

						FName BoneName = AnimSkeleton->GetReferenceSkeleton().GetBoneName(BoneIndex);

						FQuat DeltaQuat = FQuat::Identity;
						for (FBPVRHandPoseBonePair& HandPair : HandSocketComponent->CustomPoseDeltas)
						{
							if (HandPair.BoneName == BoneName)
							{
								DeltaQuat = HandPair.DeltaPose;
							}
						}

						LocalTransform.ConcatenateRotation(DeltaQuat);
						LocalTransform.NormalizeRotation();

						AnimController.SetBoneTrackKeys(BoneName, { LocalTransform.GetTranslation() }, { LocalTransform.GetRotation() }, { LocalTransform.GetScale3D() });
					}
				}
			}

			AnimController.NotifyPopulated();
		}

		AnimationObject->InitializeNotifyTrack();

		AnimationObject->MarkPackageDirty();

		{
			UPackage* const Package = AnimationObject->GetOutermost();
			FString const PackageName = Package->GetName();
			FString const PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

			double StartTime = FPlatformTime::Seconds();

			FSavePackageArgs PackageArguments;
			PackageArguments.SaveFlags = RF_Standalone;
			PackageArguments.SaveFlags = SAVE_NoError;
			UPackage::SavePackage(Package, NULL, *PackageFileName, PackageArguments);

			double ElapsedTime = FPlatformTime::Seconds() - StartTime;
			UE_LOGF(LogAnimation, Log, "Animation Recorder saved %ls in %0.2f seconds", *PackageName, ElapsedTime);
		}

		FinalAnimation = AnimationObject;
		return FinalAnimation;
	}

	return FinalAnimation;
}
TSharedRef< IDetailCustomization > FHandSocketComponentDetails::MakeInstance()
{
    return MakeShareable(new FHandSocketComponentDetails);
}

void FHandSocketComponentDetails::OnHandRelativeUpdated(IDetailLayoutBuilder* LayoutBuilder)
{

	if (!HandSocketComponent.IsValid())
	{
		return;
	}

	HandSocketComponent->Modify();
	if (AActor* Owner = HandSocketComponent->GetOwner())
	{
		Owner->Modify();
	}

	TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(HandSocketComponent->GetClass());
	FHandSocketVisualizer* HandVisualizer = (FHandSocketVisualizer*)Visualizer.Get();

	if (HandVisualizer)
	{
		if (UHandSocketComponent* RefHand = HandVisualizer->GetCurrentlyEditingComponent())
		{
			RefHand->HandRelativePlacement = HandSocketComponent->HandRelativePlacement;
		}
	}

	FComponentVisualizer::NotifyPropertyModified(HandSocketComponent.Get(), FindFProperty<FProperty>(UHandSocketComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UHandSocketComponent, HandRelativePlacement)));
}

void FHandSocketComponentDetails::OnLeftDominantUpdated(IDetailLayoutBuilder* LayoutBuilder)
{

	if (!HandSocketComponent.IsValid())
	{
		return;
	}

	{
		FTransform relTrans = HandSocketComponent->GetRelativeTransform();
		FTransform HandPlacement = HandSocketComponent->GetHandRelativePlacement();

		if (HandSocketComponent->bDecoupleMeshPlacement)
		{
			relTrans = FTransform::Identity;
		}

		FTransform ReturnTrans = (HandPlacement * relTrans);

		HandSocketComponent->MirrorHandTransform(ReturnTrans, relTrans);

		HandSocketComponent->Modify();
		if (AActor* Owner = HandSocketComponent->GetOwner())
		{
			Owner->Modify();
		}
		ReturnTrans = ReturnTrans.GetRelativeTransform(relTrans);
		HandSocketComponent->HandRelativePlacement = ReturnTrans;

		TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(HandSocketComponent->GetClass());
		FHandSocketVisualizer* HandVisualizer = (FHandSocketVisualizer*)Visualizer.Get();

		if (HandVisualizer)
		{
			if (UHandSocketComponent* RefHand = HandVisualizer->GetCurrentlyEditingComponent())
			{
				RefHand->HandRelativePlacement = HandSocketComponent->HandRelativePlacement;

			}
		}

		FComponentVisualizer::NotifyPropertyModified(HandSocketComponent.Get(), FindFProperty<FProperty>(UHandSocketComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UHandSocketComponent, HandRelativePlacement)));
	}
}

void FHandSocketComponentDetails::OnLockedStateUpdated(IDetailLayoutBuilder* LayoutBuilder)
{

	if (!HandSocketComponent.IsValid())
	{
		return;
	}

	if (HandSocketComponent->bDecoupleMeshPlacement)
	{

		{

			HandSocketComponent->Modify();
			if (AActor* Owner = HandSocketComponent->GetOwner())
			{
				Owner->Modify();
			}

			HandSocketComponent->HandRelativePlacement = HandSocketComponent->HandRelativePlacement * HandSocketComponent->GetRelativeTransform();
			HandSocketComponent->bDecoupled = true;

			TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(HandSocketComponent->GetClass());
			FHandSocketVisualizer* HandVisualizer = (FHandSocketVisualizer*)Visualizer.Get();

			if (HandVisualizer)
			{
				if (UHandSocketComponent* RefHand = HandVisualizer->GetCurrentlyEditingComponent())
				{
					RefHand->HandRelativePlacement = HandSocketComponent->HandRelativePlacement;
					RefHand->bDecoupled = true;

				}
			}
		}
	}
	else
	{

		{
			HandSocketComponent->Modify();
			if (AActor* Owner = HandSocketComponent->GetOwner())
			{
				Owner->Modify();
			}
			HandSocketComponent->HandRelativePlacement = HandSocketComponent->HandRelativePlacement.GetRelativeTransform(HandSocketComponent->GetRelativeTransform());
			HandSocketComponent->bDecoupled = false;

			TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(HandSocketComponent->GetClass());
			FHandSocketVisualizer* HandVisualizer = (FHandSocketVisualizer*)Visualizer.Get();

			if (HandVisualizer)
			{
				if (UHandSocketComponent* RefHand = HandVisualizer->GetCurrentlyEditingComponent())
				{
					RefHand->HandRelativePlacement = HandSocketComponent->HandRelativePlacement;
					RefHand->bDecoupled = false;

				}
			}
		}
	}

	TArray<FProperty*> PropertiesToModify;
	PropertiesToModify.Add(FindFProperty<FProperty>(UHandSocketComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UHandSocketComponent, HandRelativePlacement)));
	PropertiesToModify.Add(FindFProperty<FProperty>(UHandSocketComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UHandSocketComponent, bDecoupled)));
	FComponentVisualizer::NotifyPropertiesModified(HandSocketComponent.Get(), PropertiesToModify);
}

void FHandSocketComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() == 1)
	{
		UHandSocketComponent* CurrentHandSocket = Cast<UHandSocketComponent>(ObjectsBeingCustomized[0]);
		if (CurrentHandSocket != NULL)
		{
			if (HandSocketComponent != CurrentHandSocket)
			{
				TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(CurrentHandSocket->GetClass());
				FHandSocketVisualizer* HandVisualizer = (FHandSocketVisualizer*)Visualizer.Get();

				if (HandVisualizer)
				{
					HandVisualizer->CurrentlySelectedBoneIdx = INDEX_NONE;
					HandVisualizer->CurrentlySelectedBone = NAME_None;
					HandVisualizer->HandPropertyPath = FComponentPropertyPath();

				}

				HandSocketComponent = CurrentHandSocket;
			}
		}
	}

	DetailBuilder.HideCategory(FName("ComponentTick"));
	DetailBuilder.HideCategory(FName("GameplayTags"));
	DetailBuilder.HideCategory(FName("VRGripInterface"));
	DetailBuilder.HideCategory(FName("VRGripInterface|Replication"));
	DetailBuilder.HideCategory(FName("Tags"));
	DetailBuilder.HideCategory(FName("AssetUserData"));
	DetailBuilder.HideCategory(FName("Events"));
	DetailBuilder.HideCategory(FName("Activation"));
	DetailBuilder.HideCategory(FName("Cooking"));
	DetailBuilder.HideCategory(FName("ComponentReplication"));

	TSharedPtr<IPropertyHandle> LockedLocationProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UHandSocketComponent, bDecoupleMeshPlacement));
	TSharedPtr<IPropertyHandle> HandRelativePlacementProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UHandSocketComponent, HandRelativePlacement));
	TSharedPtr<IPropertyHandle> LeftHandDominateProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UHandSocketComponent, bLeftHandDominant));

	FSimpleDelegate OnHandRelativeChangedDelegate = FSimpleDelegate::CreateSP(this, &FHandSocketComponentDetails::OnHandRelativeUpdated, &DetailBuilder);
	HandRelativePlacementProperty->SetOnPropertyValueChanged(OnHandRelativeChangedDelegate);

	FSimpleDelegate OnLockedStateChangedDelegate = FSimpleDelegate::CreateSP(this, &FHandSocketComponentDetails::OnLockedStateUpdated, &DetailBuilder);
	LockedLocationProperty->SetOnPropertyValueChanged(OnLockedStateChangedDelegate);

	FSimpleDelegate OnLeftDominateChangedDelegate = FSimpleDelegate::CreateSP(this, &FHandSocketComponentDetails::OnLeftDominantUpdated, &DetailBuilder);
	LeftHandDominateProperty->SetOnPropertyValueChanged(OnLeftDominateChangedDelegate);

	TSharedPtr<IPropertyHandle> ShowVisualizationProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UHandSocketComponent, bShowVisualizationMesh));

	FSimpleDelegate OnShowVisChangedDelegate = FSimpleDelegate::CreateSP(this, &FHandSocketComponentDetails::OnUpdateShowMesh, &DetailBuilder);
	ShowVisualizationProperty->SetOnPropertyValueChanged(OnShowVisChangedDelegate);

	DetailBuilder.EditCategory("Hand Animation")
		.AddCustomRow(LOCTEXT("UpdateHandSocketRow", "Save Current Pose"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("UpdateHandSocketText", "Save Current Pose"))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FHandSocketComponentDetails::OnUpdateSavePose)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("UpdateHandSocketButton", "Save"))
		]
		];
}

void FHandSocketComponentDetails::OnUpdateShowMesh(IDetailLayoutBuilder* LayoutBuilder)
{
	if (!HandSocketComponent.IsValid())
		return;

	TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(HandSocketComponent->GetClass());
	FHandSocketVisualizer* HandVisualizer = (FHandSocketVisualizer*)Visualizer.Get();

	if (HandVisualizer)
	{
		HandVisualizer->CurrentlySelectedBoneIdx = INDEX_NONE;
		HandVisualizer->CurrentlySelectedBone = NAME_None;
		HandVisualizer->HandPropertyPath = FComponentPropertyPath();
	}
}

FReply FHandSocketComponentDetails::OnUpdateSavePose()
{
	if (HandSocketComponent.IsValid() && HandSocketComponent->CustomPoseDeltas.Num() > 0)
	{
		if (HandSocketComponent->HandTargetAnimation || HandSocketComponent->VisualizationMesh)
		{

			FString AssetPath;
			FString AssetName;
			PromptUserForAssetPath(AssetPath, AssetName);
			TWeakObjectPtr<UAnimSequence> NewAnim = SaveAnimationAsset(AssetPath, AssetName);

			if (NewAnim.IsValid())
			{
				HandSocketComponent->Modify();
				if (AActor* Owner = HandSocketComponent->GetOwner())
				{
					Owner->Modify();
				}

				HandSocketComponent->HandTargetAnimation = NewAnim.Get();
				HandSocketComponent->CustomPoseDeltas.Empty();
				HandSocketComponent->bUseCustomPoseDeltas = false;

				TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(HandSocketComponent->GetClass());
				FHandSocketVisualizer* HandVisualizer = (FHandSocketVisualizer*)Visualizer.Get();

				if (HandVisualizer)
				{
					if (UHandSocketComponent* RefHand = HandVisualizer->GetCurrentlyEditingComponent())
					{
						RefHand->HandTargetAnimation = NewAnim.Get();
						RefHand->CustomPoseDeltas.Empty();
						RefHand->bUseCustomPoseDeltas = false;

					}
				}

				TArray<FProperty*> PropertiesToModify;
				PropertiesToModify.Add(FindFProperty<FProperty>(UHandSocketComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UHandSocketComponent, HandTargetAnimation)));
				PropertiesToModify.Add(FindFProperty<FProperty>(UHandSocketComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UHandSocketComponent, bUseCustomPoseDeltas)));
				PropertiesToModify.Add(FindFProperty<FProperty>(UHandSocketComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UHandSocketComponent, CustomPoseDeltas)));
				FComponentVisualizer::NotifyPropertiesModified(HandSocketComponent.Get(), PropertiesToModify);
			}
		}
	}

	return FReply::Handled();
}

void SCreateHandAnimationDlg::Construct(const FArguments& InArgs)
{
	AssetPath = FText::FromString(FPackageName::GetLongPackagePath(InArgs._DefaultAssetPath.ToString()));
	AssetName = FText::FromString(FPackageName::GetLongPackageAssetName(InArgs._DefaultAssetPath.ToString()));

	if (AssetPath.IsEmpty())
	{
		AssetPath = LastUsedAssetPath;

		if (AssetPath.IsEmpty())
		{
			AssetPath = FText::FromString(TEXT("/Game"));
		}
	}
	else
	{
		LastUsedAssetPath = AssetPath;
	}

	if (AssetName.IsEmpty())
	{

		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FString OutPackageName, OutAssetName;
		FString PackageName = AssetPath.ToString() + TEXT("/NewAnimation");

		AssetToolsModule.Get().CreateUniqueAssetName(PackageName, TEXT(""), OutPackageName, OutAssetName);
		AssetName = FText::FromString(OutAssetName);
	}

	FPathPickerConfig PathPickerConfig;
	PathPickerConfig.DefaultPath = AssetPath.ToString();
	PathPickerConfig.OnPathSelected = FOnPathSelected::CreateSP(this, &SCreateHandAnimationDlg::OnPathChange);
	PathPickerConfig.bAddDefaultPath = true;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	SWindow::Construct(SWindow::FArguments()
		.Title(LOCTEXT("SCreateHandAnimationDlg_Title", "Create New Animation Object"))
		.SupportsMinimize(false)
		.SupportsMaximize(false)

		.ClientSize(FVector2D(450, 450))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot() 
		.Padding(2)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SelectPath", "Select Path to create animation"))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
		]

	+ SVerticalBox::Slot()
		.FillHeight(1)
		.Padding(3)
		[
			ContentBrowserModule.Get().CreatePathPicker(PathPickerConfig)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(3)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 10, 0)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AnimationName", "Animation Name"))
		]

	+ SHorizontalBox::Slot()
		[
			SNew(SEditableTextBox)
			.Text(AssetName)
		.OnTextCommitted(this, &SCreateHandAnimationDlg::OnNameChange)
		.MinDesiredWidth(250)
		]
		]
		]
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(5)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))
		.MinDesiredSlotWidth(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
		.MinDesiredSlotHeight(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
		+ SUniformGridPanel::Slot(0, 0)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
		.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
		.Text(LOCTEXT("OK", "OK"))
		.OnClicked(this, &SCreateHandAnimationDlg::OnButtonClick, EAppReturnType::Ok)
		]
	+ SUniformGridPanel::Slot(1, 0)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
		.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
		.Text(LOCTEXT("Cancel", "Cancel"))
		.OnClicked(this, &SCreateHandAnimationDlg::OnButtonClick, EAppReturnType::Cancel)
		]
		]
		]);
}

void SCreateHandAnimationDlg::OnNameChange(const FText& NewName, ETextCommit::Type CommitInfo)
{
	AssetName = NewName;
}

void SCreateHandAnimationDlg::OnPathChange(const FString& NewPath)
{
	AssetPath = FText::FromString(NewPath);
	LastUsedAssetPath = AssetPath;
}

FReply SCreateHandAnimationDlg::OnButtonClick(EAppReturnType::Type ButtonID)
{
	UserResponse = ButtonID;

	if (ButtonID != EAppReturnType::Cancel)
	{
		if (!ValidatePackage())
		{

			return FReply::Handled();
		}
	}

	RequestDestroyWindow();

	return FReply::Handled();
}

bool SCreateHandAnimationDlg::ValidatePackage()
{
	FText Reason;
	FString FullPath = GetFullAssetPath();

	if (!FPackageName::IsValidLongPackageName(FullPath, false, &Reason)
		|| !FName(*AssetName.ToString()).IsValidObjectName(Reason))
	{
		FMessageDialog::Open(EAppMsgType::Ok, Reason);
		return false;
	}

	return true;
}

EAppReturnType::Type SCreateHandAnimationDlg::ShowModal()
{
	GEditor->EditorAddModalWindow(SharedThis(this));
	return UserResponse;
}

FString SCreateHandAnimationDlg::GetAssetPath()
{
	return AssetPath.ToString();
}

FString SCreateHandAnimationDlg::GetAssetName()
{
	return AssetName.ToString();
}

FString SCreateHandAnimationDlg::GetFullAssetPath()
{
	return AssetPath.ToString() + "/" + AssetName.ToString();
}

#undef LOCTEXT_NAMESPACE
