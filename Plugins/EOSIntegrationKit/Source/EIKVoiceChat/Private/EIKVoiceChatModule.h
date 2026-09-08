

#pragma once

#include "Modules/ModuleInterface.h"
#include "Templates/SharedPointer.h"

using IVoiceChatPtr = TSharedPtr<class IVoiceChat, ESPMode::ThreadSafe>;
typedef TSharedPtr<class FEOSVoiceChatFactory, ESPMode::ThreadSafe> FEOSVoiceChatFactoryPtr;

class FEIKVoiceChatModule : public IModuleInterface
{
public:
	FEIKVoiceChatModule() = default;
	~FEIKVoiceChatModule() = default;

private:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FEOSVoiceChatFactoryPtr EOSFactory;
	IVoiceChatPtr EOSObj;
};