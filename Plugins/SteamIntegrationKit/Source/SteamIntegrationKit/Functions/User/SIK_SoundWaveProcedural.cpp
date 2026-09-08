

#include "SIK_SoundWaveProcedural.h"

void USIK_SoundWaveProcedural::SIK_QueueAudio(const TArray<uint8>& AudioData)
{
	ResetAudio();
	if (AudioData.Num() > 0 && AudioData.GetData())
	{
		QueueAudio(AudioData.GetData(), AudioData.Num());
	}
}