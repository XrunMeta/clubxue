

#pragma once

#include "Modules/ModuleInterface.h"

class FVaRestEditorModule : public IModuleInterface
{

public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
