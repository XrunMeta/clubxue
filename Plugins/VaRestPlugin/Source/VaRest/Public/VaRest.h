

#pragma once

#include "Modules/ModuleManager.h"

class UVaRestSettings;

class FVaRestModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static inline FVaRestModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FVaRestModule>("VaRest");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("VaRest");
	}

	UVaRestSettings* GetSettings() const;

protected:

	UVaRestSettings* ModuleSettings;
};
