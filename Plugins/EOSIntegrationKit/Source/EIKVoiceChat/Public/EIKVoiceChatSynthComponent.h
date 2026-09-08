

#pragma once

#include "CoreMinimal.h"
#include "Components/SynthComponent.h"
#include "DSP/Osc.h"
#include "DSP/Dsp.h"
#include "DSP/FloatArrayMath.h"
#include "Containers/CircularBuffer.h"
#include "Runtime/Launch/Resources/Version.h"
#include "EIKVoiceChatSynthComponent.generated.h"

UCLASS(ClassGroup = Synth, meta = (BlueprintSpawnableComponent))
class EIKVOICECHAT_API UEIKVoiceChatSynthComponent : public USynthComponent
{
	GENERATED_BODY()

	constexpr static int32 MinLatencySamples = 2048;

	Audio::TCircularAudioBuffer<float> AudioBuffer;

	bool bIsOutArrayInitialized = false;
	TArray<float> OutArray;
	TArrayView<float> OutArrayView;

	virtual bool Init(int32& SampleRate) override;

	virtual int32 OnGenerateAudio(float* OutAudio, int32 NumSamples) override;

protected:

	friend class FEOSVoiceChatUser;

	void WriteSamples(TArrayView<int16> Samples)
	{

		if (OutArray.Num() < Samples.Num())
		{
			OutArray.SetNum(Samples.Num());  
			OutArrayView = TArrayView<float>(OutArray.GetData(), Samples.Num());  
		}
#if ENGINE_MAJOR_VERSION >= 5

		Audio::ArrayPcm16ToFloat(Samples, OutArrayView);
#else

		for (int32 Index = 0; Index < Samples.Num(); ++Index)
		{
			OutArrayView[Index] = static_cast<float>(Samples[Index]) / 32768.0f;
		}
#endif

		AudioBuffer.Push(OutArrayView.GetData(), Samples.Num());
	}

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EOS Integration Kit|Voice Settings")
	TArray<FString> SupportedRooms;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EOS Integration Kit|Voice Settings")
	bool bUseGlobalRoom = false;

};