

#pragma once

#include "Modules/ModuleManager.h"

class FVRExpansionPluginModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RegisterSettings();

	void UnregisterSettings();
};