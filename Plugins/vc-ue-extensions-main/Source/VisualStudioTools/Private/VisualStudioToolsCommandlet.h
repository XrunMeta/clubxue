

#pragma once

#include "CoreMinimal.h"
#include "VisualStudioToolsCommandletBase.h"

#include "VisualStudioToolsCommandlet.generated.h"

UCLASS()
class UVisualStudioToolsCommandlet
	: public UVisualStudioToolsCommandletBase
{
	GENERATED_BODY()

public:
	UVisualStudioToolsCommandlet();
	int32 Run(
		TArray<FString>& Tokens,
		TArray<FString>& Switches,
		TMap<FString, FString>& ParamVals,
		FArchive& OutArchive) override;
};
