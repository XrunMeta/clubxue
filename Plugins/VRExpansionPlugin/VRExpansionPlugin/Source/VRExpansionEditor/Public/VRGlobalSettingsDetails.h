

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"

class IDetailLayoutBuilder;

class FVRGlobalSettingsDetails : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	FReply OnCorrectInvalidAnimationAssets();

	FReply OnFixShadowShader();

	void OnLockedStateUpdated(IDetailLayoutBuilder* LayoutBuilder);

	FVRGlobalSettingsDetails()
	{
	}
};

