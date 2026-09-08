

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDiscord, Log, All);

class FDiscordGameModule : public IModuleInterface
{
public:

	static const FName ModuleName;

	static FDiscordGameModule* Get()
	{
		return static_cast<FDiscordGameModule*>(FModuleManager::Get().GetModule(ModuleName));
	}

	FORCEINLINE bool IsDiscordSDKLoaded() const { return bDiscordSDKLoaded; }

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:

	FString GetPathToDLL() const;

private:

	void* DiscordGameSDKHandle {nullptr};
	bool bDiscordSDKLoaded = false;

};
