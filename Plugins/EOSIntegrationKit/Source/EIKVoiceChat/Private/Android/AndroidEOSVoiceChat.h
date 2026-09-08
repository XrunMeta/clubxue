

#pragma once

#include "EOSVoiceChat.h"

#if WITH_EOS_RTC

class FAndroidEOSVoiceChat : public FEOSVoiceChat
{
public:
	FAndroidEOSVoiceChat(IEOSSDKManager& InSDKManager, const IEIKPlatformHandlePtr& InPlatformHandle);
	virtual ~FAndroidEOSVoiceChat() = default;

	virtual IVoiceChatUser* CreateUser() override;

};

using FPlatformEOSVoiceChat = FAndroidEOSVoiceChat;

#endif 