// Georgy Treshchev 2024.

#include "AudioAnalysisToolsLibrary.h"
#include "AudioAnalysisToolsDefines.h"

#include "Analyzers/CoreFrequencyDomainFeatures.h"
#include "Analyzers/CoreTimeDomainFeatures.h"
#include "Analyzers/BeatDetection.h"
#include "Analyzers/OnsetDetection.h"

#include "Analyzers/FFTAudioAnalyzer.h"

#include "Async/Async.h"
#include "Misc/ScopeLock.h"

UAudioAnalysisToolsLibrary::UAudioAnalysisToolsLibrary()
	: FFTConfigured(false)
{
}

void UAudioAnalysisToolsLibrary::BeginDestroy()
{
	if (FFTConfigured)
	{
		FreeFFT();
	}

	Super::BeginDestroy();
}

UAudioAnalysisToolsLibrary* UAudioAnalysisToolsLibrary::CreateAudioAnalysisTools(int64 FrameSize, EAnalysisWindowType WindowType)
{
	UAudioAnalysisToolsLibrary* AudioAnalysisTools = NewObject<UAudioAnalysisToolsLibrary>();
	AudioAnalysisTools->Initialize(FrameSize, WindowType);
	return AudioAnalysisTools;
}

bool UAudioAnalysisToolsLibrary::GetAudioByCurrentTime(UImportedSoundWave* ImportedSoundWave, TArray<float>& AudioFrames)
{
	return GetAudioByFrameSize(ImportedSoundWave, CurrentAudioFrames.Num(), AudioFrames);
}

bool UAudioAnalysisToolsLibrary::GetAudioByFrameSize(UImportedSoundWave* ImportedSoundWave, int64 FrameSize, TArray<float>& AudioFrames)
{
	const int64 StartFrame = ImportedSoundWave ? ImportedSoundWave->GetNumOfPlayedFrames() : 0;
	const int64 EndFrame = StartFrame + FrameSize;

	return GetAudioByFrameRange(ImportedSoundWave, StartFrame, EndFrame, AudioFrames);
}

bool UAudioAnalysisToolsLibrary::GetAudioByFrameRange(UImportedSoundWave* ImportedSoundWave, int64 StartFrame, int64 EndFrame, TArray<float>& AudioFrames)
{
	if (!(StartFrame >= 0 && StartFrame < EndFrame))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to get the frame data: start frame is '%lld', expected >= '0.0' and < '%lld'"), StartFrame, EndFrame);
		return false;
	}

	if (!(EndFrame > 0 && EndFrame > StartFrame))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to get the frame data: end frame is '%lld', expected > '0.0' and < '%lld'"), EndFrame, StartFrame);
		return false;
	}

	FScopeLock Lock(&*ImportedSoundWave->DataGuard);

	if (EndFrame > ImportedSoundWave->GetPCMBuffer().PCMNumOfFrames)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to get the PCM Data: end frame ('%lld') must be less than total number of frames ('%d')"), EndFrame, ImportedSoundWave->GetPCMBuffer().PCMNumOfFrames);
		return false;
	}

	const int32 NumChannels = ImportedSoundWave->NumChannels;

	const float* RetrievedPCMData = ImportedSoundWave->GetPCMBuffer().PCMData.GetView().GetData() + StartFrame * NumChannels;
	const int64 RetrievedPCMDataSize = (EndFrame - StartFrame + 1) * NumChannels;

	if (!RetrievedPCMData)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to get the PCM Data: retrieved PCM Data is nullptr"));
		return false;
	}

	if (RetrievedPCMDataSize <= 0)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to get the PCM Data: retrieved PCM Data size is '%lld', must be > 0 (%lld - %lld)"), RetrievedPCMDataSize, EndFrame, StartFrame);
		return false;
	}

	if (RetrievedPCMDataSize > static_cast<int64>(ImportedSoundWave->GetPCMBuffer().PCMData.GetView().Num()))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to get the PCM Data: retrieved PCM Data size (%lld) must be less than the total size (%lld)"), RetrievedPCMDataSize, static_cast<int64>(ImportedSoundWave->GetPCMBuffer().PCMData.GetView().Num()));
		return false;
	}

	if (RetrievedPCMDataSize > TNumericLimits<int32>::Max())
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Failed to get audio by frame range: Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)"), TNumericLimits<int32>::Max(), RetrievedPCMDataSize);
		return false;
	}

	AudioFrames = TArray<float>(RetrievedPCMData, RetrievedPCMDataSize);
	return true;
}

bool UAudioAnalysisToolsLibrary::GetAudioByTimeLength(UImportedSoundWave* ImportedSoundWave, float TimeLength, TArray<float>& AudioFrames)
{
	const float StartTime = ImportedSoundWave ? ImportedSoundWave->GetPlaybackTime() : 0.f;
	const float EndTime = StartTime + TimeLength;

	return GetAudioByTimeRange(ImportedSoundWave, StartTime, EndTime, AudioFrames);
}

bool UAudioAnalysisToolsLibrary::GetAudioByTimeRange(UImportedSoundWave* ImportedSoundWave, float StartTime, float EndTime, TArray<float>& AudioFrames)
{
	if (!ImportedSoundWave)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Failed to get audio frames: the specified sound wave is invalid"));
		return false;
	}

	int64 StartFrame, EndFrame;
	{
		FScopeLock Lock(&*ImportedSoundWave->DataGuard);

		if (!ImportedSoundWave->GetPCMBuffer().IsValid())
		{
			UE_LOG(LogAudioAnalysis, Error, TEXT("Failed to get audio frames: PCM buffer is invalid"));
			return false;
		}

		if (ImportedSoundWave->GetPCMBuffer().PCMData.GetView().Num() <= 0)
		{
			UE_LOG(LogAudioAnalysis, Error, TEXT("Failed to get audio frames data: PCM Data Size is '%lld', expected > '0'"), static_cast<int64>(ImportedSoundWave->GetPCMBuffer().PCMData.GetView().Num()));
			return false;
		}

		if (ImportedSoundWave->GetPCMBuffer().PCMNumOfFrames <= 0)
		{
			UE_LOG(LogAudioAnalysis, Error, TEXT("Failed to get audio frames data: PCM Num Of Frames is '%d', expected > '0'"), ImportedSoundWave->GetPCMBuffer().PCMNumOfFrames);
			return false;
		}

		if (!(StartTime >= 0 && StartTime < EndTime))
		{
			UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to get the frame data: start time is '%f', expected >= '0.0' and < '%f'"), StartTime, EndTime);
			return false;
		}

		if (!(EndTime > 0 && EndTime > StartTime))
		{
			UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to get the frame data: end time is '%f', expected > '0.0' and < '%f'"), EndTime, StartTime);
			return false;
		}

		const float Duration = ImportedSoundWave->GetDurationConst_Internal();
		if (EndTime > Duration)
		{
			UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to get the PCM Data: end time (%f) must be less than the sound wave duration (%f)"), EndTime, Duration);
			return false;
		}

		const int32 SamplingRate = ImportedSoundWave->GetSampleRate();

		StartFrame = StartTime * SamplingRate;
		EndFrame = EndTime * SamplingRate;
	}

	return GetAudioByFrameRange(ImportedSoundWave, StartFrame, EndFrame, AudioFrames);
}

void UAudioAnalysisToolsLibrary::Initialize(int64 FrameSize, EAnalysisWindowType InWindowType)
{
	BeatDetection = UBeatDetection::CreateBeatDetection();
	check(BeatDetection);

	OnsetDetection = UOnsetDetection::CreateOnsetDetection(FrameSize);
	check(OnsetDetection);

	WindowType = InWindowType;

	UpdateFrameSize(FrameSize);
}

TArray<float> UAudioAnalysisToolsLibrary::GetMagnitudeSpectrum() const
{
	if (MagnitudeSpectrum.Num() > TNumericLimits<int32>::Max())
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Failed to get Magnitude Spectrum Real: Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)"), TNumericLimits<int32>::Max(), MagnitudeSpectrum.Num());
		return TArray<float>();
	}

	return TArray<float>(MagnitudeSpectrum);
}

const TArray64<float>& UAudioAnalysisToolsLibrary::GetMagnitudeSpectrum64() const
{
	return MagnitudeSpectrum;
}

TArray<float> UAudioAnalysisToolsLibrary::GetFFTReal() const
{
	if (FFTReal.Num() > TNumericLimits<int32>::Max())
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Failed to get FFT Real: Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)"), TNumericLimits<int32>::Max(), FFTReal.Num());
		return TArray<float>();
	}

	return TArray<float>(FFTReal);
}

const TArray64<float>& UAudioAnalysisToolsLibrary::GetFFTReal64() const
{
	return FFTReal;
}

TArray<float> UAudioAnalysisToolsLibrary::GetFFTImaginary() const
{
	if (FFTImaginary.Num() > TNumericLimits<int32>::Max())
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Failed to get FFT Imaginary: Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)"), TNumericLimits<int32>::Max(), FFTReal.Num());
		return TArray<float>();
	}

	return TArray<float>(FFTImaginary);
}

const TArray64<float>& UAudioAnalysisToolsLibrary::GetFFTImaginary64() const
{
	return FFTImaginary;
}

void UAudioAnalysisToolsLibrary::ProcessAudioFrames(TArray<float> AudioFrames, bool bProcessToBeatDetection)
{
	if (IsInGameThread())
	{
		AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [WeakThis = MakeWeakObjectPtr(this), AudioFrames = MoveTemp(AudioFrames), bProcessToBeatDetection]() mutable 
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ProcessAudioFrames(MoveTemp(AudioFrames), bProcessToBeatDetection);
			}
			else
			{
				UE_LOG(LogAudioAnalysis, Error, TEXT("Failed to process audio frames because the AudioAnalysisToolsLibrary has been destroyed"));
			}
		});
		return;
	}

	FScopeLock Lock(&DataGuard);

	if (AudioFrames.Num() != CurrentAudioFrames.Num())
	{
		UpdateFrameSize(AudioFrames.Num());
	}
	CurrentAudioFrames = MoveTemp(AudioFrames);

	PerformFFT();

	if (bProcessToBeatDetection)
	{
		BeatDetection->ProcessMagnitude(MagnitudeSpectrum);
	}
}

void UAudioAnalysisToolsLibrary::UpdateFrameSize(int64 FrameSize)
{
	const int64 MagnitudeSpectrumSize = FrameSize / 2;

	CurrentAudioFrames.SetNum(FrameSize);

	WindowFunction = UWindowsLibrary::CreateWindowByType(FrameSize, WindowType);

	FFTReal.SetNum(FrameSize);
	FFTImaginary.SetNum(FrameSize);
	MagnitudeSpectrum.SetNum(MagnitudeSpectrumSize);

	ConfigureFFT();
}

bool UAudioAnalysisToolsLibrary::IsBeat(int64 Subband) const
{
	check(BeatDetection);
	return BeatDetection->IsBeat(Subband);
}

bool UAudioAnalysisToolsLibrary::IsKick() const
{
	check(BeatDetection);
	return BeatDetection->IsKick();
}

bool UAudioAnalysisToolsLibrary::IsSnare() const
{
	check(BeatDetection);
	return BeatDetection->IsSnare();
}

bool UAudioAnalysisToolsLibrary::IsHiHat() const
{
	check(BeatDetection);
	return BeatDetection->IsHiHat();
}

bool UAudioAnalysisToolsLibrary::IsBeatRange(int64 Low, int64 High, int64 Threshold) const
{
	check(BeatDetection);
	return BeatDetection->IsBeatRange(Low, High, Threshold);
}

float UAudioAnalysisToolsLibrary::GetBand(int64 Subband) const
{
	check(BeatDetection);
	return BeatDetection->GetBand(Subband);
}

float UAudioAnalysisToolsLibrary::GetRootMeanSquare()
{
	return UCoreTimeDomainFeatures::GetRootMeanSquare(CurrentAudioFrames);
}

float UAudioAnalysisToolsLibrary::GetPeakEnergy()
{
	return UCoreTimeDomainFeatures::GetPeakEnergy(CurrentAudioFrames);
}

float UAudioAnalysisToolsLibrary::GetZeroCrossingRate()
{
	return UCoreTimeDomainFeatures::GetZeroCrossingRate(CurrentAudioFrames);
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
	check(OnsetDetection);
	return OnsetDetection->GetEnergyDifference(CurrentAudioFrames);
}

float UAudioAnalysisToolsLibrary::GetSpectralDifference()
{
	check(OnsetDetection);
	return OnsetDetection->GetSpectralDifference(MagnitudeSpectrum);
}

float UAudioAnalysisToolsLibrary::GetSpectralDifferenceHWR()
{
	check(OnsetDetection);
	return OnsetDetection->GetSpectralDifferenceHWR(MagnitudeSpectrum);
}

float UAudioAnalysisToolsLibrary::GetComplexSpectralDifference()
{
	check(OnsetDetection);
	return OnsetDetection->GetComplexSpectralDifference(FFTReal, FFTImaginary);
}

float UAudioAnalysisToolsLibrary::GetHighFrequencyContent()
{
	check(OnsetDetection);
	return OnsetDetection->GetHighFrequencyContent(MagnitudeSpectrum);
}

void UAudioAnalysisToolsLibrary::ConfigureFFT()
{
	if (FFTConfigured)
	{
		FreeFFT();
	}

	const int64 FrameSize = CurrentAudioFrames.Num();

	FFT_InSamples = new FFTComplexSamples[FrameSize];
	FFT_OutSamples = new FFTComplexSamples[FrameSize];
	FFT_Configuration = UFFTAudioAnalyzer::PerformFFTAlloc(FrameSize, 0, nullptr, nullptr);

	FFTConfigured = true;
}

void UAudioAnalysisToolsLibrary::FreeFFT()
{
	// Free the Kiss FFT configuration
	FMemory::Free(FFT_Configuration);

	delete[] FFT_InSamples;
	delete[] FFT_OutSamples;

	FFT_Configuration = nullptr;
	FFT_InSamples = nullptr;
	FFT_OutSamples = nullptr;
}

void UAudioAnalysisToolsLibrary::PerformFFT()
{
	if (!FFT_InSamples || !FFT_OutSamples || !FFT_Configuration)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to perform FFT analysis because the buffers are invalid"));
		return;
	}

	const int64 FrameSize = CurrentAudioFrames.Num();

	for (int64 Index = 0; Index < FrameSize; ++Index)
	{
		FFT_InSamples[Index].Real = CurrentAudioFrames[Index] * WindowFunction[Index];
		FFT_InSamples[Index].Imaginary = 0.0;
	}

	// Execute kiss fft
	UFFTAudioAnalyzer::PerformFFT(FFT_Configuration, FFT_InSamples, FFT_OutSamples);

	// Store real and imaginary parts of FFT
	for (int64 Index = 0; Index < FrameSize; ++Index)
	{
		FFTReal[Index] = FFT_OutSamples[Index].Real;
		FFTImaginary[Index] = FFT_OutSamples[Index].Imaginary;
	}

	// Calculate the magnitude spectrum
	for (int64 Index = 0; Index < FrameSize / 2; ++Index)
	{
		MagnitudeSpectrum[Index] = FMath::Sqrt(FMath::Pow(FFTReal[Index], 2) + FMath::Pow(FFTImaginary[Index], 2));
	}
}
