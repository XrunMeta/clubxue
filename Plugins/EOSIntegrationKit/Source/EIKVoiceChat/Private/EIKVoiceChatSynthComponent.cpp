

#include "EIKVoiceChatSynthComponent.h"

bool UEIKVoiceChatSynthComponent::Init(int32& SampleRate)
{
	NumChannels = 1;
#if ENGINE_MAJOR_VERSION >= 5
	AudioBuffer = Audio::TCircularAudioBuffer<float>(SampleRate * NumChannels);
#else

	AudioBuffer.Reset(SampleRate * NumChannels);

	OutArray.Reserve(SampleRate / 10);
	OutArrayView = TArrayView<float>(OutArray.GetData(), SampleRate / 10);
#endif
	OutArray.Reserve(SampleRate / 10);
	OutArrayView = TArrayView<float>(OutArray.GetData(), SampleRate / 10);

	return true;
}

int32 UEIKVoiceChatSynthComponent::OnGenerateAudio(float* OutAudio, int32 NumSamples)
{

	if (AudioBuffer.Num() >= uint32(NumSamples))
	{
		AudioBuffer.Pop(OutAudio, NumSamples);
		return NumSamples;

	}
	else
	{
		return 0;
	}
}

