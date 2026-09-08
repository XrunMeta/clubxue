

#pragma once

#include "Modules/ModuleManager.h"

class FNISViewExtension;
class FNVImageUpscaler;

class FNISCoreModule final: public IModuleInterface
{
public:

	virtual void StartupModule();
	virtual void ShutdownModule();

private:
	TSharedPtr< FNISViewExtension , ESPMode::ThreadSafe> NISViewExtension;
	TUniquePtr<FNVImageUpscaler> NISUpscaler;
};
