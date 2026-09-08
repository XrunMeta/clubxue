#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class AdvancedSteamSessions : public IModuleInterface
{
public:

	void StartupModule();
	void ShutdownModule();
};