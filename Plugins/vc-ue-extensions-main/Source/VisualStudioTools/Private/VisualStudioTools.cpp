

#include "VisualStudioTools.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogVisualStudioTools);

class FVisualStudioToolsModule : public IModuleInterface
{
public:

	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FVisualStudioToolsModule, VisualStudioTools)
