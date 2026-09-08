

#pragma once

#include "Modules/ModuleManager.h"

class FOpenXRExpansionPluginModule : public IModuleInterface
{
public:

	FOpenXRExpansionPluginModule()
	{
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};