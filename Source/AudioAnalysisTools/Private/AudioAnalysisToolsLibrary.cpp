// Georgy Treshchev 2022.

#include "AudioAnalysisToolsLibrary.h"

#include "Analyzers/CoreFrequencyDomainFeatures.h"
#include "Analyzers/CoreTimeDomainFeatures.h"
#include "Analyzers/EnvelopeAnalysis.h"
#include "Analyzers/OnsetDetection.h"

#include "ThirdParty/kiss_fft.c"

UAudioAnalysisToolsLibrary::UAudioAnalysisToolsLibrary()
	: FFTConfigured(false)
{
}

void UAudioAnalysisToolsLibrary::BeginDestroy()
{
	UObject::BeginDestroy();

	if (FFTConfigured)
	{
		FreeFFT();
	}
}

UAudioAnalysisToolsLibrary* UAudioAnalysisToolsLibrary::CreateAudioAnalysisTools(int32 FrameSize, int32 SampleRate, EAnalysisWindowType WindowType)
{
	UAudioAnalysisToolsLibrary* AudioAnalysisTools = NewObject<UAudioAnalysisToolsLibrary>();

	AudioAnalysisTools->Initialize(FrameSize, SampleRate, WindowType);

	return AudioAnalysisTools;
}

bool UAudioAnalysisToolsLibrary::ProcessAudioFrameFromSoundWave(UImportedSoundWave* ImportedSoundWave, float TimeLength)
{
	TArray<float> AudioFrameBuffer;

	const float SoundWavePlaybackTime{ImportedSoundWave->GetPlaybackTime()};

	if (!GetAudioFrameFromSoundWave(ImportedSoundWave, SoundWavePlaybackTime, SoundWavePlaybackTime + TimeLength, AudioFrameBuffer))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("An audio frame from the sound wave '%s' was not processed!"), ImportedSoundWave != nullptr ? *ImportedSoundWave->GetName() : *FName().ToString());
		return false;
	}

	if (SampleRate != ImportedSoundWave->SamplingRate)
	{
		UpdateSampleRate(ImportedSoundWave->SamplingRate);
	}

	ProcessAudioFrame(AudioFrameBuffer);

	return true;
}

bool UAudioAnalysisToolsLibrary::ProcessAudioFrameFromSoundWave_Custom(UImportedSoundWave* ImportedSoundWave, float StartTime, float EndTime)
{
	TArray<float> AudioFrameBuffer;

	if (!GetAudioFrameFromSoundWave(ImportedSoundWave, StartTime, EndTime, AudioFrameBuffer))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("An audio frame from the sound wave '%s' was not processed!"), ImportedSoundWave != nullptr ? *ImportedSoundWave->GetName() : *FName().ToString());
		return false;
	}

	if (SampleRate != ImportedSoundWave->SamplingRate)
	{
		UpdateSampleRate(ImportedSoundWave->SamplingRate);
	}

	ProcessAudioFrame(AudioFrameBuffer);

	return true;
}

void UAudioAnalysisToolsLibrary::UpdateSampleRate(int32 InSampleRate)
{
	EnvelopeAnalysisRef->UpdateSampleRate(InSampleRate);
}

void UAudioAnalysisToolsLibrary::Initialize(int32 FrameSize, int32 InSampleRate, EAnalysisWindowType InWindowType)
{
	EnvelopeAnalysisRef = UEnvelopeAnalysis::CreateEnvelopeAnalysis(1, InSampleRate, FrameSize);
	OnsetDetectionRef = UOnsetDetection::CreateOnsetDetection(FrameSize);

	WindowType = InWindowType;

	UpdateFrameSize(FrameSize);
}

bool UAudioAnalysisToolsLibrary::GetAudioFrameFromSoundWave(UImportedSoundWave* ImportedSoundWave, float StartTime, float EndTime, TArray<float>& InAudioFrame)
{
	if (ImportedSoundWave == nullptr)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the Sample Data: the specified sound wave is nullptr"));
		return false;
	}

	if (ImportedSoundWave->PCMBufferInfo.PCMData == nullptr)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the Sample Data: it is nullptr"));
		return false;
	}

	if (ImportedSoundWave->PCMBufferInfo.PCMDataSize <= 0)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the Sample Data: PCM Data Size is '%d', expected > '0'"), ImportedSoundWave->PCMBufferInfo.PCMDataSize);
		return false;
	}

	if (ImportedSoundWave->PCMBufferInfo.PCMNumOfFrames <= 0)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the Sample Data: PCM Num Of Frames is '%d', expected more than '0'"), ImportedSoundWave->PCMBufferInfo.PCMNumOfFrames);
		return false;
	}

	if (!(StartTime > 0 || StartTime < EndTime))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the Sample Data: start time is '%f', expected > '0.0' and < '%s'"), StartTime, EndTime);
		return false;
	}

	if (!(EndTime > 0 || EndTime > StartTime))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the Sample Data: end time is '%f', expected > '0.0' and < '%f'"), EndTime, StartTime);
		return false;
	}

	const float Duration{ImportedSoundWave->GetDurationConst()};

	if (EndTime > Duration)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the PCM Data: end time (%f) must be less than the sound wave duration (%f)"), EndTime, Duration);
		return false;
	}

	const int32 SamplingRate{ImportedSoundWave->SamplingRate};

	const int32 NumOfChannels{ImportedSoundWave->NumChannels};

	const int32 StartSample{static_cast<int32>(StartTime * SamplingRate)};
	const int32 EndSample{static_cast<int32>(EndTime * SamplingRate)};

	if (EndSample > ImportedSoundWave->PCMBufferInfo.PCMNumOfFrames)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the PCM Data: end sample (%d) must be less than total number of frames (%d)"), EndSample, ImportedSoundWave->PCMBufferInfo.PCMNumOfFrames);
		return false;
	}

	uint8* RetrievedPCMData = ImportedSoundWave->PCMBufferInfo.PCMData + (StartSample * ImportedSoundWave->NumChannels * sizeof(float));
	const int32 RetrievedPCMDataSize = ((EndSample * sizeof(float)) - (StartSample * sizeof(float))) * NumOfChannels;

	if (RetrievedPCMData == nullptr)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the PCM Data: retrieved PCM Data is nullptr"));
		return false;
	}

	if (RetrievedPCMDataSize <= 0)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the PCM Data: retrieved PCM Data size is '%d', must be more > 0"), RetrievedPCMDataSize);
		return false;
	}

	if (RetrievedPCMDataSize > static_cast<int32>(ImportedSoundWave->PCMBufferInfo.PCMDataSize))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot get the PCM Data: retrieved PCM Data size (%d) must be less than the total size (%d)"), RetrievedPCMDataSize, static_cast<int32>(ImportedSoundWave->PCMBufferInfo.PCMDataSize));
		return false;
	}

	if (!EnvelopeAnalysisRef->GetEnvelopeValues(TArray<float>(reinterpret_cast<float*>(RetrievedPCMData), RetrievedPCMDataSize), InAudioFrame))
	{
		return false;
	}

	return true;
}

void UAudioAnalysisToolsLibrary::ProcessAudioFrame(TArray<float> InAudioFrame)
{
	if (InAudioFrame.Num() != AudioFrame.Num())
	{
		UpdateFrameSize(InAudioFrame.Num());
	}

	AudioFrame = MoveTemp(InAudioFrame);

	AsyncTask(ENamedThreads::AnyThread, [this]()
	{
		PerformFFT();
	});
}

void UAudioAnalysisToolsLibrary::UpdateFrameSize(int32 FrameSize)
{
	AudioFrame.SetNum(FrameSize);

	WindowFunction = UWindowsLibrary::CreateWindow(FrameSize, WindowType);

	FFTReal.SetNum(FrameSize);
	FFTImaginary.SetNum(FrameSize);
	MagnitudeSpectrum.SetNum(FrameSize / 2);
	
	ConfigureFFT();

	OnsetDetectionRef->UpdateFrameSize(FrameSize);
	EnvelopeAnalysisRef->UpdateFrameSize(FrameSize);
}

const TArray<float>& UAudioAnalysisToolsLibrary::GetMagnitudeSpectrum()
{
	return MagnitudeSpectrum;
}

bool UAudioAnalysisToolsLibrary::GetEnvelopeValues(TArray<float>& AnalyzedData)
{
	return EnvelopeAnalysisRef->GetEnvelopeValues(AudioFrame, AnalyzedData);
}

float UAudioAnalysisToolsLibrary::GetRootMeanSquare()
{
	return UCoreTimeDomainFeatures::GetRootMeanSquare(AudioFrame);
}

float UAudioAnalysisToolsLibrary::GetPeakEnergy()
{
	return UCoreTimeDomainFeatures::GetPeakEnergy(AudioFrame);
}

float UAudioAnalysisToolsLibrary::GetZeroCrossingRate()
{
	return UCoreTimeDomainFeatures::GetZeroCrossingRate(AudioFrame);
}

float UAudioAnalysisToolsLibrary::GetSpectralCentroid()
{
	return UCoreFrequencyDomainFeatures::GetSpectralCentroid(MagnitudeSpectrum);
}

float UAudioAnalysisToolsLibrary::GetSpectralFlatness()
{
	return UCoreFrequencyDomainFeatures::GetSpectralFlatness(MagnitudeSpectrum);
}

float UAudioAnalysisToolsLibrary::GetSpectralCrest()
{
	return UCoreFrequencyDomainFeatures::GetSpectralCrest(MagnitudeSpectrum);
}

float UAudioAnalysisToolsLibrary::GetSpectralRolloff()
{
	return UCoreFrequencyDomainFeatures::GetSpectralRolloff(MagnitudeSpectrum);
}

float UAudioAnalysisToolsLibrary::GetSpectralKurtosis()
{
	return UCoreFrequencyDomainFeatures::GetSpectralKurtosis(MagnitudeSpectrum);
}

float UAudioAnalysisToolsLibrary::GetEnergyDifference()
{
	return OnsetDetectionRef->GetEnergyDifference(AudioFrame);
}

float UAudioAnalysisToolsLibrary::GetSpectralDifference()
{
	return OnsetDetectionRef->GetSpectralDifference(MagnitudeSpectrum);
}

float UAudioAnalysisToolsLibrary::GetSpectralDifferenceHWR()
{
	return OnsetDetectionRef->GetSpectralDifferenceHWR(MagnitudeSpectrum);
}

float UAudioAnalysisToolsLibrary::GetComplexSpectralDifference()
{
	return OnsetDetectionRef->GetComplexSpectralDifference(FFTReal, FFTImaginary);
}

float UAudioAnalysisToolsLibrary::GetHighFrequencyContent()
{
	return OnsetDetectionRef->GetHighFrequencyContent(MagnitudeSpectrum);
}

void UAudioAnalysisToolsLibrary::ConfigureFFT()
{
	if (FFTConfigured)
	{
		FreeFFT();
	}

	const int32 FrameSize{AudioFrame.Num()};

	FFT_InSamples = new kiss_fft_cpx[FrameSize];
	FFT_OutSamples = new kiss_fft_cpx[FrameSize];
	FFT_Configuration = kiss_fft_alloc(FrameSize, 0, nullptr, nullptr);

	FFTConfigured = true;
}

void UAudioAnalysisToolsLibrary::FreeFFT()
{
	/** Free the Kiss FFT configuration */
	FMemory::Free(FFT_Configuration);

	delete[] FFT_InSamples;
	delete[] FFT_OutSamples;
}

void UAudioAnalysisToolsLibrary::PerformFFT()
{
	const int32 FrameSize{AudioFrame.Num()};

	for (int32 Index = 0; Index < FrameSize; ++Index)
	{
		FFT_InSamples[Index].r = AudioFrame[Index] * WindowFunction[Index];
		FFT_InSamples[Index].i = 0.0;
	}

	/** Execute kiss fft */
	kiss_fft(FFT_Configuration, FFT_InSamples, FFT_OutSamples);

	/** Store real and imaginary parts of FFT */
	for (int32 Index = 0; Index < FrameSize; ++Index)
	{
		FFTReal[Index] = FFT_OutSamples[Index].r;
		FFTImaginary[Index] = FFT_OutSamples[Index].i;
	}

	/** Calculate the magnitude spectrum */
	for (int32 Index = 0; Index < FrameSize / 2; ++Index)
	{
		MagnitudeSpectrum[Index] = FMath::Sqrt(FMath::Pow(FFTReal[Index], 2) + FMath::Pow(FFTImaginary[Index], 2));
	}
}
