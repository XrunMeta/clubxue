

#pragma once

#include "EOSShared.h"

#if WITH_EOS_RTC

#include "Features/IModularFeatures.h"
#include "Misc/CoreMisc.h"
#include "Templates/SharedPointer.h"
#include "IEOSSDKManager.h"

using IVoiceChatPtr = TSharedPtr<class IVoiceChat, ESPMode::ThreadSafe>;
using IVoiceChatWeakPtr = TWeakPtr<class IVoiceChat, ESPMode::ThreadSafe>;

class EIKVOICECHAT_API FEOSVoiceChatFactory : public IModularFeature, public FSelfRegisteringExec
{
public:
	static FEOSVoiceChatFactory* Get()
	{
		if (IModularFeatures::Get().IsModularFeatureAvailable(GetModularFeatureName()))
		{
			return &IModularFeatures::Get().GetModularFeature<FEOSVoiceChatFactory>(GetModularFeatureName());
		}
		return nullptr;
	}

	static FName GetModularFeatureName()
	{
		static const FName FeatureName = TEXT("EOSVoiceChatFactory");
		return FeatureName;
	}

	virtual ~FEOSVoiceChatFactory() = default;

	IVoiceChatPtr CreateInstance();

	IVoiceChatPtr CreateInstanceWithPlatform(const IEIKPlatformHandlePtr& PlatformHandle);

	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

private:
	TArray<IVoiceChatWeakPtr> Instances;
};

#endif 