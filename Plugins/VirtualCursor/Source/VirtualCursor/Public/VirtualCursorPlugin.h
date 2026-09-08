#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVirtualCursor, Log, All);

class FVirtualCursorPlugin : public IModuleInterface
{
public:

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	static inline FVirtualCursorPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked<FVirtualCursorPlugin>("VirtualCursor");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("VirtualCursor");
	}
};
