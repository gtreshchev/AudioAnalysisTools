// Georgy Treshchev 2024.

#include "Analyzers/BeatDetection.h"
#include "AudioAnalysisToolsDefines.h"
#include "Math/UnrealMathUtility.h"
#include "Math/NumericLimits.h"

UBeatDetection::UBeatDetection()
	: HistoryPosition(0),
	  FFTSubbandSize(0),
	  EnergyHistorySize(0)
{
}

UBeatDetection* UBeatDetection::CreateBeatDetection(int64 InFFTSubbandsSize, int64 InEnergyHistorySize)
{
	UBeatDetection* BeatDetection = NewObject<UBeatDetection>();
	
	// We'll set this here, so we only resize the energy history arrays once, in the UpdateFFTSubbandsSize function
	BeatDetection->EnergyHistorySize = InEnergyHistorySize;
	BeatDetection->UpdateFFTSubbandSize(InFFTSubbandsSize);
	return BeatDetection;
}

void UBeatDetection::UpdateFFTSubbandSize(int64 InFFTSubbandSize)
{
	// We'll assume nothing, and make sure our user has made a reasonable request
	if (InFFTSubbandSize <= 0)
	{
		// Tell the user that they've tried to use an incorrect value, and where they tried it
		UE_LOG(LogAudioAnalysis, Log, TEXT("Beat Detection FFT subbands size '%lld' is invalid. The value '%lld' will remain unchanged."), InFFTSubbandSize, FFTSubbandSize);
		return;
	}

	UE_LOG(LogAudioAnalysis, Log, TEXT("Updating Beat Detection FFT subbands size from '%lld' to '%lld'"), FFTSubbandSize, InFFTSubbandSize);
	
	FFTSubbandSize = InFFTSubbandSize;

	FFTSubbands.SetNum(FFTSubbandSize);
	FFTAverageEnergy.SetNum(FFTSubbandSize);
	FFTVariance.SetNum(FFTSubbandSize);
	FFTBeatValues.SetNum(FFTSubbandSize);
	EnergyHistory.SetNum(FFTSubbandSize);

	// We resized the external array, so we have to resize the new array
	UpdateEnergyHistorySize(EnergyHistorySize);
}

void UBeatDetection::UpdateEnergyHistorySize(int64 InEnergyHistorySize)
{
	// We'll assume nothing, and make sure our user has made a reasonable request
	if (InEnergyHistorySize <= 0)
	{
		// Tell the user that they've tried to use an incorrect value, and where they tried it
		UE_LOG(LogAudioAnalysis, Log, TEXT("Beat Detection energy history size '%lld' is invalid, value '%lld' remains"), InEnergyHistorySize, EnergyHistorySize);
		return;
	}

	UE_LOG(LogAudioAnalysis, Log, TEXT("Updating Beat Detection energy history size from '%lld' to '%lld'"), EnergyHistorySize, InEnergyHistorySize);
	
	EnergyHistorySize = InEnergyHistorySize;

	for (int64 SubbandIndex = 0; SubbandIndex < FFTSubbandSize; ++SubbandIndex)
	{
		EnergyHistory[SubbandIndex].SetNum(EnergyHistorySize);
	}
}

void UBeatDetection::UpdateFFT(const TArray64<float>& MagnitudeSpectrum)
{
	const int64 MagnitudeSpectrumSize{MagnitudeSpectrum.Num()};

	// Sub-band calculation
	for (int64 SubbandIndex = 0; SubbandIndex < FFTSubbandSize; ++SubbandIndex)
	{
		FFTSubbands[SubbandIndex] = 0;

		for (int64 SubbandInternalIndex = 0; SubbandInternalIndex < MagnitudeSpectrumSize / FFTSubbandSize; ++SubbandInternalIndex)
		{
			FFTSubbands[SubbandIndex] += MagnitudeSpectrum[SubbandIndex * (MagnitudeSpectrumSize / FFTSubbandSize) + SubbandInternalIndex];
		}
		// After summing the subband values, divide the added number of times to get the average value
		FFTSubbands[SubbandIndex] *= static_cast<float>(FFTSubbandSize) / MagnitudeSpectrumSize;

		// Calculation of subband variance value
		for (int64 SubbandInternalIndex = 0; SubbandInternalIndex < MagnitudeSpectrumSize / FFTSubbandSize; ++SubbandInternalIndex)
		{
			FFTVariance[SubbandIndex] += FMath::Pow(MagnitudeSpectrum[SubbandIndex * (MagnitudeSpectrumSize / FFTSubbandSize) + SubbandInternalIndex] - FFTSubbands[SubbandIndex], 2);
		}
		FFTVariance[SubbandIndex] *= static_cast<float>(FFTSubbandSize) / MagnitudeSpectrumSize;

		// Reduce possible noise with linear digression using some magic numbers
		FFTBeatValues[SubbandIndex] = (-0.0025714 * FFTVariance[SubbandIndex]) + 1.15142857;
	}

	// Calculation of energy average
	for (int64 SubbandIndex = 0; SubbandIndex < FFTSubbandSize; ++SubbandIndex)
	{
		FFTAverageEnergy[SubbandIndex] = 0;
		for (int64 EnergyHistoryIndex = 0; EnergyHistoryIndex < EnergyHistorySize; ++EnergyHistoryIndex)
		{
			// Average of total energy += Energy history of each subband
			FFTAverageEnergy[SubbandIndex] += EnergyHistory[SubbandIndex][EnergyHistoryIndex];
		}

		// Divide the sum by the history energy to get a weighted average
		FFTAverageEnergy[SubbandIndex] /= EnergyHistorySize;
	}

	// Put new values into the energy history
	for (int64 SubbandIndex = 0; SubbandIndex < FFTSubbandSize; ++SubbandIndex)
	{
		// Add the calculated subband to the HistoryPosition in the energy history
		EnergyHistory[SubbandIndex][HistoryPosition] = FFTSubbands[SubbandIndex];
	}

	// A pseudo-cyclic list is represented by circular array indexes
	HistoryPosition = (HistoryPosition + 1) % EnergyHistorySize;
}

void UBeatDetection::ProcessMagnitude(const TArray<float>& MagnitudeSpectrum)
{
	ProcessMagnitude(TArray64<float>(MagnitudeSpectrum));
}

void UBeatDetection::ProcessMagnitude(const TArray64<float>& MagnitudeSpectrum)
{
	UpdateFFT(MagnitudeSpectrum);
}

bool UBeatDetection::IsBeat(int64 SubBand) const
{
	// Prevent out of array exception
	if (SubBand >= FFTSubbandSize)
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot detect a beat: FFT sub-band ('%lld') must not exceed the sub-band size ('%lld')"), SubBand, FFTSubbandSize);
		return false;
	}
	return FFTSubbands[SubBand] > FFTAverageEnergy[SubBand] * FFTBeatValues[SubBand];
}

bool UBeatDetection::IsKick() const
{
	return IsBeat(KICK_BAND);
}

bool UBeatDetection::IsSnare() const
{
	constexpr int64 Low = 1;
	const int64 High = FFTSubbandSize / 3;
	const int64 Threshold = (High - Low) / 3;

	return IsBeatRange(Low, High, Threshold);
}

bool UBeatDetection::IsHiHat() const
{
	const int64 Low = FFTSubbandSize / 2;
	const int64 High = FFTSubbandSize - 1;
	const int64 Threshold = (High - Low) / 3;

	return IsBeatRange(Low, High, Threshold);
}

bool UBeatDetection::IsBeatRange(int64 Low, int64 High, int64 Threshold) const
{
	if (!(Low >= 0 && Low < FFTSubbandSize))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot detect if the beat is in range: the low sub-band is '%lld', expected to be >= '0' and < '%lld'"), Low, FFTSubbandSize);
		return false;
	}

	if (!(High >= 0 && High < FFTSubbandSize))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot detect if the beat is in range: the high sub-band is '%lld', expected to be >= '0', < '%lld'"), High, FFTSubbandSize);
		return false;
	}

	if (!(High > Low))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot detect if the beat is in range: the high sub-band ('%lld') must be greater than the low sub-band ('%lld')"), High, Low);
		return false;
	}

	int64 NumOfBeats = 0;

	for (int64 Index = Low; Index < High + 1; ++Index)
	{
		if (IsBeat(Index))
		{
			NumOfBeats++;
		}
	}

	return NumOfBeats > Threshold;
}

float UBeatDetection::GetBand(int64 Subband) const
{
	if (!(Subband >= 0 && Subband < FFTSubbandSize))
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Cannot obtain FFT sub-band: the specified sub-band is '%lld', but it is expected to be >= '0' and < '%lld'"), Subband, FFTSubbandSize);
		return -1;
	}
	return FFTSubbands[Subband];
}

TArray<float> UBeatDetection::GetFFTSubbands_BP() const
{
	if (FFTSubbands.Num() > TNumericLimits<int32>::Max())
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to obtain FFT sub-bands: array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), TNumericLimits<int32>::Max(), FFTSubbands.Num());
		return TArray<float>();
	}
	return TArray<float>(FFTSubbands);
}

TArray<float> UBeatDetection::GetFFTAverageEnergy_BP() const
{
	if (FFTAverageEnergy.Num() > TNumericLimits<int32>::Max())
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to obtain FFT average energy: array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), TNumericLimits<int32>::Max(), FFTAverageEnergy.Num());
		return TArray<float>();
	}
	return TArray<float>(FFTAverageEnergy);
}

TArray<float> UBeatDetection::GetFFTVariance_BP() const
{
	if (FFTVariance.Num() > TNumericLimits<int32>::Max())
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to obtain FFT variance: array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), TNumericLimits<int32>::Max(), FFTVariance.Num());
		return TArray<float>();
	}
	return TArray<float>(FFTVariance);
}

TArray<float> UBeatDetection::GetFFTBeatValues_BP() const
{
	if (FFTBeatValues.Num() > TNumericLimits<int32>::Max())
	{
		UE_LOG(LogAudioAnalysis, Error, TEXT("Unable to obtain FFT beat values: array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), TNumericLimits<int32>::Max(), FFTBeatValues.Num());
		return TArray<float>();
	}
	return TArray<float>(FFTBeatValues);
}
