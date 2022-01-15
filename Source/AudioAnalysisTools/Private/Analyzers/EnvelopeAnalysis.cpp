// Georgy Treshchev 2022.

#include "Analyzers/EnvelopeAnalysis.h"

#include "AudioAnalysisToolsDefines.h"

UEnvelopeAnalysis* UEnvelopeAnalysis::CreateEnvelopeAnalysis(int32 InNumberOfChannels, int32 InSampleRate, int32 InFrameSize, float InAttackTimeMSec, float InReleaseTimeMSec, EEnvelopeMode InEnvelopeMode, bool InbIsAnalog)
{
	UEnvelopeAnalysis* EnvelopeAnalysis = NewObject<UEnvelopeAnalysis>();
	EnvelopeAnalysis->Initialize(InNumberOfChannels, InSampleRate, InFrameSize, InAttackTimeMSec, InReleaseTimeMSec, InEnvelopeMode, InbIsAnalog);
	return EnvelopeAnalysis;
}

void UEnvelopeAnalysis::UpdateSampleRate(int32 InSampleRate)
{
	SampleRate = InSampleRate;
}

void UEnvelopeAnalysis::UpdateFrameSize(int32 FrameSize)
{
	EnvelopeFrameSize = FrameSize;
}

void UEnvelopeAnalysis::Initialize(int32 InNumberOfChannels, int32 InSampleRate, int32 InFrameSize, float InAttackTimeMSec, float InReleaseTimeMSec, EEnvelopeMode InEnvelopeMode, bool InbIsAnalog)
{
	NumberOfChannels = InNumberOfChannels;
	UpdateSampleRate(InSampleRate);
	UpdateFrameSize(InFrameSize);
	EnvelopeMode = InEnvelopeMode;
	bIsAnalog = InbIsAnalog;

	SetAttackTime(InAttackTimeMSec);
	SetReleaseTime(InReleaseTimeMSec);
}

bool UEnvelopeAnalysis::GetEnvelopeValues(const TArray<float>& PCMData, TArray<float>& AnalyzedData)
{
	if (NumberOfChannels <= 0)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Invalid number of channels. Received number of channels: %d, expected > 0"), NumberOfChannels)
		return false;
	}

	if (SampleRate <= 0)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Invalid sample rate. Received sampling frequency: %d, expected > 0"), SampleRate)
		return false;
	}

	/** After the loop, it will contain all analyzed time data */
	TArray<float> EnvelopeData;

	/** Getting the number of frames */
	const uint32 NumFrames = (static_cast<float>(PCMData.Num()) / sizeof(float)) / NumberOfChannels;

	/** Go through all frames */
	for (uint32 FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
	{
		float SampleValue = 0.0f;

		/** Get the average sample value of all channels */
		{
			for (int32 ChannelIndex = 0; ChannelIndex < NumberOfChannels; ++ChannelIndex)
			{
				SampleValue += PCMData[FrameIndex * NumberOfChannels];
			}

			SampleValue /= NumberOfChannels;
		}

		/** Calculating the envelope value */
		CalculateEnvelope(SampleValue);

		/** Until we reached the frame size */
		if (FrameIndex % EnvelopeFrameSize == 0)
		{
			/** Setting the amplitude */
			EnvelopeData.Add(CurrentEnvelopeValue);
		}
	}

	if (EnvelopeData.Num() <= 0)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Envelope was not found"))
		return false;
	}

	/** Filling the Envelope time data */
	AnalyzedData = MoveTemp(EnvelopeData);

	return true;
}

void UEnvelopeAnalysis::SetAttackTime(const float AttackTimeMSec)
{
	const float TimeConstant{bIsAnalog ? AnalogTimeConstant : DigitalTimeConstant};
	AttackTimeSamples = FMath::Exp(-1000.0f * TimeConstant / (AttackTimeMSec * SampleRate));
}

void UEnvelopeAnalysis::SetReleaseTime(const float ReleaseTimeMSec)
{
	const float TimeConstant{bIsAnalog ? AnalogTimeConstant : DigitalTimeConstant};
	ReleaseTimeSamples = FMath::Exp(-1000.0f * TimeConstant / (ReleaseTimeMSec * SampleRate));
}

void UEnvelopeAnalysis::CalculateEnvelope(const float InAudioSample)
{
	const float Sample{EnvelopeMode != EEnvelopeMode::Peak ? InAudioSample * InAudioSample : FMath::Abs(InAudioSample)};

	const float TimeSamples{Sample > CurrentEnvelopeValue ? AttackTimeSamples : ReleaseTimeSamples};
	const float NewEnvelopeValue{FMath::Clamp(Audio::UnderflowClamp(TimeSamples * (CurrentEnvelopeValue - Sample) + Sample), 0.f, 1.f)};

	CurrentEnvelopeValue = NewEnvelopeValue;
}
