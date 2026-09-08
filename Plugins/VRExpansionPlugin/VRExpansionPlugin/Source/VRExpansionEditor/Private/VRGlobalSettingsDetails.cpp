

#include "VRGlobalSettingsDetails.h"
#include "VRGlobalSettings.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"

#include "UObject/SavePackage.h"
#include "Misc/MessageDialog.h"

#include "Editor.h"
#include "EditorStyleSet.h"
#include "Styling/CoreStyle.h"

#include "Animation/AnimData/AnimDataModel.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/ARFilter.h"

#include "FileHelpers.h"
#include "Engine/Engine.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "VRGlobalSettingsDetails"

TSharedRef< IDetailCustomization > FVRGlobalSettingsDetails::MakeInstance()
{
    return MakeShareable(new FVRGlobalSettingsDetails);
}

void FVRGlobalSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

	DetailBuilder.EditCategory("Utilities")
		.AddCustomRow(LOCTEXT("FixInvalidAnimationAssets", "Fix Invalid 5.2 Animation Assets"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("FixInvalidAnimationAssets", "Fix Invalid 5.2 Animation Assets"))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FVRGlobalSettingsDetails::OnCorrectInvalidAnimationAssets)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("FixInvalidAnimationAssetsButton", "Fix Animation Assets"))
		]
		];

}

FReply FVRGlobalSettingsDetails::OnCorrectInvalidAnimationAssets()
{

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked< FAssetRegistryModule >(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray< FString > ContentPaths;
	ContentPaths.Add(TEXT("/Game"));
	AssetRegistry.ScanPathsSynchronous(ContentPaths);

	FARFilter Filter;
	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());

	Filter.bRecursiveClasses = true;

	Filter.bRecursivePaths = true;

	TArray< FAssetData > AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	for (auto& Asset : AssetList)
	{

		if (UAnimSequence* AnimSeq = Cast<UAnimSequence>(Asset.GetAsset()))
		{
			IAnimationDataController& AnimController = AnimSeq->GetController();
			{
				IAnimationDataController::FScopedBracket ScopedBracket(AnimController, LOCTEXT("FixAnimationAsset_VRE", "Fixing invalid anim sequences"));
				const IAnimationDataModel* AnimModel = AnimController.GetModel();

				FFrameRate FrameRate = AnimModel->GetFrameRate();

				double FrameRateD = FrameRate.AsDecimal();

				if (FrameRateD < 1.0f)
				{

					AnimController.SetFrameRate(FFrameRate(1, 1));
					AnimSeq->MarkPackageDirty();

					UPackage* const Package = AnimSeq->GetOutermost();
					FString const PackageName = Package->GetName();
					FString const PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

					double StartTime = FPlatformTime::Seconds();

					FSavePackageArgs PackageArguments;
					PackageArguments.SaveFlags = RF_Standalone;
					PackageArguments.SaveFlags = SAVE_NoError;
					UPackage::SavePackage(Package, NULL, *PackageFileName, PackageArguments);

					double ElapsedTime = FPlatformTime::Seconds() - StartTime;
					UE_LOGF(LogAnimation, Log, "Animation re-saved %ls in %0.2f seconds", *PackageName, ElapsedTime);
				}

			}
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
