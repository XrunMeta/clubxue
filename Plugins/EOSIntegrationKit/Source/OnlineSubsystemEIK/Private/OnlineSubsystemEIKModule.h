

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FOnlineSubsystemEIKModule : public IModuleInterface
{
private:

	class FOnlineFactoryEOS* EOSFactory;

public:

	FOnlineSubsystemEIKModule() :
		EOSFactory(NULL)
	{}

	virtual ~FOnlineSubsystemEIKModule() {}

#if WITH_EDITOR
	void OnPostEngineInit();
	void OnPreExit();
#endif

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}

	void PluginButtonClicked();

private:

	void RegisterMenus();

	void ConfigureOnlineSubsystemEIK();
};