

#pragma once

#include "CoreMinimal.h"
#include "Grippables/HandSocketComponent.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"

class IDetailLayoutBuilder;

class FHandSocketComponentDetails : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	TWeakObjectPtr<UHandSocketComponent> HandSocketComponent;
	FReply OnUpdateSavePose();
	TWeakObjectPtr<UAnimSequence> SaveAnimationAsset(const FString& InAssetPath, const FString& InAssetName);

	void OnLockedStateUpdated(IDetailLayoutBuilder* LayoutBuilder);
	void OnLeftDominantUpdated(IDetailLayoutBuilder* LayoutBuilder);
	void OnHandRelativeUpdated(IDetailLayoutBuilder* LayoutBuilder);
	void OnUpdateShowMesh(IDetailLayoutBuilder* LayoutBuilder);

	FHandSocketComponentDetails()
	{
	}
};

class SCreateHandAnimationDlg : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SCreateHandAnimationDlg)
	{
	}

	SLATE_ARGUMENT(FText, DefaultAssetPath)
		SLATE_END_ARGS()

		SCreateHandAnimationDlg()
		: UserResponse(EAppReturnType::Cancel)
	{
	}

	void Construct(const FArguments& InArgs);

public:

	EAppReturnType::Type ShowModal();

	FString GetAssetPath();

	FString GetAssetName();

	FString GetFullAssetPath();

protected:
	void OnPathChange(const FString& NewPath);
	void OnNameChange(const FText& NewName, ETextCommit::Type CommitInfo);
	FReply OnButtonClick(EAppReturnType::Type ButtonID);

	bool ValidatePackage();

	EAppReturnType::Type UserResponse;
	FText AssetPath;
	FText AssetName;

	static FText LastUsedAssetPath;

};

