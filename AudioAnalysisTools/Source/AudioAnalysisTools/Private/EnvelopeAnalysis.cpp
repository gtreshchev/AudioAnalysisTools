// Georgy Treshchev 2021.

#include "EnvelopeAnalysis.h"
#include "Async/Async.h"
#include "RuntimeAudioImporterLibrary.h"

UEnvelopeAnalysis::UEnvelopeAnalysis()
{
	Reset();
}

UEnvelopeAnalysis* UEnvelopeAnalysis::CreateEnvelopeAnalysis()
{
	UEnvelopeAnalysis* EnvelopeAnalysis = NewObject<UEnvelopeAnalysis>();
	EnvelopeAnalysis->AddToRoot();
	return EnvelopeAnalysis;
}

void UEnvelopeAnalysis::Init(const FEnvelopeAnalysisStruct EnvelopeAnalysisInfo)
{
	NumberOfChannels = EnvelopeAnalysisInfo.NumberOfChannels;
	SampleRate = EnvelopeAnalysisInfo.SampleRate;
	EnvelopeFrameSize = EnvelopeAnalysisInfo.EnvelopeAnalysisFrameSize;
	EnvelopeMode = EnvelopeAnalysisInfo.EnvelopeMode;
	bIsAnalog = EnvelopeAnalysisInfo.bIsAnalog;

	SetAttackTime(EnvelopeAnalysisInfo.AttackTimeMSec);
	SetReleaseTime(EnvelopeAnalysisInfo.ReleaseTimeMSec);
}

void UEnvelopeAnalysis::Reset()
{
	CurrentEnvelopeValue = 0.0f;

	NumberOfChannels = 0;
	SampleRate = 44100.0f;
	EnvelopeFrameSize = 1024;
	EnvelopeMode = Squared;
	bIsAnalog = true;

	SetAttackTime(10.0f);
	SetReleaseTime(100.0f);
}

void UEnvelopeAnalysis::AnalyzeEnvelopeFromImportedSoundWave(class UImportedSoundWave* ImportedSoundWave,
                                                             FEnvelopeAnalysisStruct EnvelopeAnalysisInfo)
{
	// Retrieving PCM data and its size
	float* PCMData = reinterpret_cast<float*>(ImportedSoundWave->PCMBufferInfo.PCMData);
	const int32 PCMDataSize = ImportedSoundWave->PCMBufferInfo.PCMDataSize;

	// Checking if the PCM data is valid
	if (!PCMData || PCMDataSize == 0) OnFinished_Internal(FAnalysedEnvelopeData(), ZeroAudioData);
	else
	{
		EnvelopeAnalysisInfo.NumberOfChannels = ImportedSoundWave->NumChannels;
		EnvelopeAnalysisInfo.SampleRate = ImportedSoundWave->SamplingRate;

		AnalyzeEnvelopeFromMemory(PCMData, PCMDataSize, EnvelopeAnalysisInfo);
	}
}

void UEnvelopeAnalysis::AnalyzeEnvelopeFromMemory(float* PCMData, const int32 PCMDataSize,
                                                  const FEnvelopeAnalysisStruct EnvelopeAnalysisInfo)
{
	AsyncTask(ENamedThreads::AnyThread, [=]()
	{
		AnalyzeEnvelopeFromMemory_Internal(PCMData, PCMDataSize, EnvelopeAnalysisInfo);
	});
}

void UEnvelopeAnalysis::AnalyzeEnvelopeFromMemory_Internal(float* PCMData, const int32 PCMDataSize,
                                                           const FEnvelopeAnalysisStruct EnvelopeAnalysisInfo)
{
	FAnalysedEnvelopeData AnalysedEnvelopeData;

	// Initialize envelope analysis
	Init(EnvelopeAnalysisInfo);

	if (!PCMData || PCMDataSize <= 0) OnFinished_Internal(AnalysedEnvelopeData, ZeroAudioData);
	else if (NumberOfChannels < 1 || NumberOfChannels > 2) OnFinished_Internal(AnalysedEnvelopeData, InvalidChannels);
	else if (SampleRate <= 0) OnFinished_Internal(AnalysedEnvelopeData, InvalidChannels);
	else
	{
		// After the loop, it will contain all analyzed time data
		TArray<FEnvelopeTimeData> EnvelopeTimeData;

		// Get the number of frames
		const uint32 NumFrames = (PCMDataSize / sizeof(float)) / NumberOfChannels;

		// After the loop, it will contain the sum of all amplitudes (for calculating the arithmetic mean)
		float SumNumber = 0;

		// Go through all frames
		for (uint32 FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
		{
			float SampleValue = 0.0f;

			// Get the average sample value of all channels
			{
				for (int32 ChannelIndex = 0; ChannelIndex < NumberOfChannels; ++ChannelIndex)
				{
					SampleValue += PCMData[FrameIndex * NumberOfChannels];
				}

				SampleValue /= NumberOfChannels;
			}

			// Calculating the envelope value
			CalculateEnvelope(SampleValue);

			// Until we reached the frame size
			if (FrameIndex % EnvelopeFrameSize == 0)
			{
				// New envelope time data
				FEnvelopeTimeData NewData;

				// Setting the time of the calculated amplitude
				NewData.TimeSec = static_cast<float>(FrameIndex) / SampleRate;

				// Setting the amplitude
				NewData.Amplitude = CurrentEnvelopeValue;

				// Add calculated data to array
				EnvelopeTimeData.Add(NewData);

				// Increase SumNumber by the current amplitude value
				SumNumber += NewData.Amplitude;
			}
		}

		if (EnvelopeTimeData.Num() > 0)
		{
			// Calculating the arithmetic mean
			AnalysedEnvelopeData.AverageValue = SumNumber / EnvelopeTimeData.Num();

			// Filling the Envelope time data
			AnalysedEnvelopeData.EnvelopeTimeData = EnvelopeTimeData;

			// OnFinished Callback
			OnFinished_Internal(AnalysedEnvelopeData, SuccessfulAnalysis);
			
			AnalysedEnvelopeData.EnvelopeTimeData.Empty();
		}
		else OnFinished_Internal(AnalysedEnvelopeData, WasNotFound);
	}
}

void UEnvelopeAnalysis::SetAttackTime(const float InAttackTimeMSec)
{
	AttackTimeMSec = InAttackTimeMSec;
	const float TimeConstant = bIsAnalog ? AnalogTimeConstant : DigitalTimeConstant;
	AttackTimeSamples = FMath::Exp(-1000.0f * TimeConstant / (AttackTimeMSec * SampleRate));
}

void UEnvelopeAnalysis::SetReleaseTime(const float InReleaseTimeMSec)
{
	ReleaseTimeMSec = InReleaseTimeMSec;
	const float TimeConstant = bIsAnalog ? AnalogTimeConstant : DigitalTimeConstant;
	ReleaseTimeSamples = FMath::Exp(-1000.0f * TimeConstant / (ReleaseTimeMSec * SampleRate));
}

void UEnvelopeAnalysis::CalculateEnvelope(const float InAudioSample)
{
	const float Sample = (EnvelopeMode != Peak) ? InAudioSample * InAudioSample : FMath::Abs(InAudioSample);
	const float TimeSamples = (Sample > CurrentEnvelopeValue) ? AttackTimeSamples : ReleaseTimeSamples;
	float NewEnvelopeValue = TimeSamples * (CurrentEnvelopeValue - Sample) + Sample;;
	NewEnvelopeValue = Audio::UnderflowClamp(NewEnvelopeValue);
	NewEnvelopeValue = FMath::Clamp(NewEnvelopeValue, 0.0f, 1.0f);

	CurrentEnvelopeValue = NewEnvelopeValue;
}

void UEnvelopeAnalysis::OnFinished_Internal(const FAnalysedEnvelopeData& ReadyEnvelopeData,
                                            const TEnumAsByte<EEnvelopeAnalysisStatus> Status)
{
	AsyncTask(ENamedThreads::GameThread, [=]()
	{
		if (OnAnalysisFinished.IsBound()) OnAnalysisFinished.Broadcast(ReadyEnvelopeData, Status);
		RemoveFromRoot();
	});
}
