

#pragma once
#include "Modules/ModuleManager.h"

class FNGXVulkanRHIPreInitModule final : public IModuleInterface
{
public:

	virtual void StartupModule();
	virtual void ShutdownModule();
};