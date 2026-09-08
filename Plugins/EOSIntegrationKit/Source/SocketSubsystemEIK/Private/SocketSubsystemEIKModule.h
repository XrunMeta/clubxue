

#pragma once

#include "Modules/ModuleInterface.h"

class FSocketSubsystemEIKModule: public IModuleInterface
{
public:
	FSocketSubsystemEIKModule() = default;
	~FSocketSubsystemEIKModule() = default;

private:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};