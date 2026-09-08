

#pragma once

#include "Commandlets/Commandlet.h"

#include <Runtime/Core/Public/Misc/AutomationTest.h>
#include <Runtime/CoreUObject/Public/UObject/ObjectMacros.h>
#include <Runtime/Engine/Classes/Commandlets/Commandlet.h>

#include "VSTestAdapterCommandlet.generated.h"

UCLASS()
class UVSTestAdapterCommandlet
	: public UCommandlet
{
	GENERATED_BODY()

public:
	UVSTestAdapterCommandlet();

public:
	virtual int32 Main(const FString &Params) override;

private:
	void PrintHelp() const;
};
