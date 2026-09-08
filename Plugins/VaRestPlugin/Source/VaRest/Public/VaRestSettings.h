

#pragma once

#include "VaRestSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class VAREST_API UVaRestSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(Config, EditAnywhere, Category = "VaRest")
	bool bExtendedLog;

	UPROPERTY(Config, EditAnywhere, Category = "VaRest")
	bool bUseChunkedParser;
};
