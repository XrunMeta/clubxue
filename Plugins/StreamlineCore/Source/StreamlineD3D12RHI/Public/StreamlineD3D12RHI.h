

#pragma once

#include "Modules/ModuleManager.h"

#include "CoreMinimal.h"
#include "StreamlineRHI.h"

class FStreamlineD3D12RHIModule final : public IStreamlineRHIModule
{
public:
	virtual TUniquePtr<FStreamlineRHI> CreateStreamlineRHI(const FStreamlineRHICreateArguments& Arguments) override;

	virtual void StartupModule();
	virtual void ShutdownModule();
};
