

#pragma once

#include "Modules/ModuleInterface.h"

#include "EOSSDKManager.h"

class FEIKSharedModule: public IModuleInterface
{
public:
	FEIKSharedModule() = default;
	~FEIKSharedModule() = default;

private:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#if WITH_EOS_SDK
	TUniquePtr<FEIKSDKManager> SDKManager;
#endif
};