

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDLSSEditor, Log, All);

class FDLSSEditorModule final : public IModuleInterface
{
public:

	virtual void StartupModule() final;
	virtual void ShutdownModule() final;
};

