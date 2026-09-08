

#pragma once

#include "EOSVoiceChatUser.h"

#if WITH_EOS_RTC

#include "IOSEOSVoiceChat.h"

class FIOSEOSVoiceChatUser : public FEOSVoiceChatUser
{
public:
	FIOSEOSVoiceChatUser(FEOSVoiceChat& InEOSVoiceChat);
	virtual ~FIOSEOSVoiceChatUser() = default;

	virtual void SetSetting(const FString& Name, const FString& Value) override;
	virtual void JoinChannel(const FString& ChannelName, const FString& ChannelCredentials, EVoiceChatChannelType ChannelType, const FOnVoiceChatChannelJoinCompleteDelegate& Delegate, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties) override;

	FIOSEOSVoiceChat& GetIOSVoiceChat();

private:
    friend class FIOSEOSVoiceChat;
	void HandleVoiceChatChannelExited(const FString& ChannelName, const FVoiceChatResult& Reason);
};

#endif 