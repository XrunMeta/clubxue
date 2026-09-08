#pragma once

#include "AnimGraphDefinitions.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Editor/AnimGraph/Public/AnimGraphNode_SkeletalControlBase.h"

#include "AnimNode_ApplyOpenXRHandPose.h"

#include "AnimGraphNode_ApplyOpenXRHandPose.generated.h"

UCLASS(MinimalAPI)
class UAnimGraphNode_ApplyOpenXRHandPose : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_ApplyOpenXRHandPose Node;

public:

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FString GetNodeCategory() const override;

protected:

	virtual FText GetControllerDescription() const;
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }

};