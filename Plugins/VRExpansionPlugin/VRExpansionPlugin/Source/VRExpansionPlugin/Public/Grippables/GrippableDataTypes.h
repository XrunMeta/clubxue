

#pragma once
#include "CoreMinimal.h"

#include "Engine/ReplicatedState.h"
#include "GrippableDataTypes.generated.h"

USTRUCT()
struct VREXPANSIONPLUGIN_API FRepAttachmentWithWeld : public FRepAttachment
{
public:
	GENERATED_BODY()

	UPROPERTY()
	bool bIsWelded;

	FRepAttachmentWithWeld();
};

