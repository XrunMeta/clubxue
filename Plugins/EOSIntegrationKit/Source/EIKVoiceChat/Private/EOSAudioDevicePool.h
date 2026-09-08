

#pragma once

#include "EOSShared.h"

#if WITH_EOS_RTC

#include "VoiceChat.h"

#include "eos_rtc_types.h"

class FEOSAudioDevicePool final : public TSharedFromThis<FEOSAudioDevicePool>
{
public:
	explicit FEOSAudioDevicePool(EOS_HRTC& InEosRtcInterface);

	DECLARE_DELEGATE_OneParam(FOnAudioDevicePoolRefreshAudioDevicesCompleteDelegate, const FVoiceChatResult& );
	void RefreshAudioDevices(const FOnAudioDevicePoolRefreshAudioDevicesCompleteDelegate& Delegate);

	DECLARE_DELEGATE_OneParam(FOnAudioDevicePoolRefreshAudioInputDevicesCompleteDelegate, const FVoiceChatResult& );
	void RefreshAudioInputDevices(const FOnAudioDevicePoolRefreshAudioInputDevicesCompleteDelegate& Delegate);

	DECLARE_DELEGATE_OneParam(FOnAudioDevicePoolRefreshAudioOutputDevicesCompleteDelegate, const FVoiceChatResult& );
	void RefreshAudioOutputDevices(const FOnAudioDevicePoolRefreshAudioOutputDevicesCompleteDelegate& Delegate);

	const TArray<FVoiceChatDeviceInfo>& GetCachedInputDeviceInfos() const;
	const TArray<FVoiceChatDeviceInfo>& GetCachedOutputDeviceInfos() const;

	int32 GetDefaultInputDeviceInfoIdx() const;
	int32 GetDefaultOutputDeviceInfoIdx() const;
private:
	TArray<FVoiceChatDeviceInfo> GetRtcInputDeviceInfos(int32& OutDefaultDeviceIdx) const;
	TArray<FVoiceChatDeviceInfo> GetRtcOutputDeviceInfos(int32& OutDefaultDeviceIdx) const;

	EOS_HRTC& EosRtcInterface;

	int32 DefaultInputDeviceInfoIdx = -1;
	int32 DefaultOutputDeviceInfoIdx = -1;
	TArray<FVoiceChatDeviceInfo> CachedInputDeviceInfos;
	TArray<FVoiceChatDeviceInfo> CachedOutputDeviceInfos;
};

#endif 
