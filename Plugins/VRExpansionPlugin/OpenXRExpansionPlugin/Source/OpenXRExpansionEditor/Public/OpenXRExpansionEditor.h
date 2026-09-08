

#pragma once

#include "Runtime/Core/Public/Modules/ModuleInterface.h"

class FOpenXRExpansionEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};