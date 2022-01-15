// Georgy Treshchev 2022.

#include "Analyzers/OnsetDetection.h"
#include "Math/UnrealMathUtility.h"

#include "AudioAnalysisToolsDefines.h"

UOnsetDetection* UOnsetDetection::CreateOnsetDetection(int32 FrameSize)
{
	UOnsetDetection* OnsetDetection{NewObject<UOnsetDetection>()};
	OnsetDetection->UpdateFrameSize(FrameSize);
	return OnsetDetection;
}

void UOnsetDetection::UpdateFrameSize(int32 FrameSize)
{
	PrevMagnitudeSpectrum_spectralDifference.Init(0.f, FrameSize);
	PrevMagnitudeSpectrum_spectralDifferenceHWR.Init(0.f, FrameSize);
	PrevPhaseSpectrum_complexSpectralDifference.Init(0.f, FrameSize);
	PrevPhaseSpectrum2_complexSpectralDifference.Init(0.f, FrameSize);
	PrevMagnitudeSpectrum_complexSpectralDifference.Init(0.f, FrameSize);

	PrevEnergySum = 0;
}

float UOnsetDetection::GetEnergyDifference(const TArray<float>& AudioFrame)
{
	float Energy{0};

	/** Sum the squares of the samples */
	for (const auto& BufferValue : AudioFrame)
	{
		Energy += FMath::Pow(BufferValue, 2);
	}

	/** Sample is first order difference in energy */
	const float Difference{Energy - PrevEnergySum};

	/** Store energy value for next calculation */
	PrevEnergySum = Energy;

	if (Difference > 0)
	{
		return Difference;
	}

	return 0.0;
}

float UOnsetDetection::GetSpectralDifference(const TArray<float>& magnitudeSpectrum)
{
	float SpectralDifferenceValue{0};

	for (TArray<float>::SizeType Index = 0; Index < magnitudeSpectrum.Num(); ++Index)
	{
		/** Calculate difference */
		const float Difference{magnitudeSpectrum[Index] - PrevMagnitudeSpectrum_spectralDifference[Index]};

		/** Ensure all difference values are positive */
		FMath::Abs(Difference);

		/** Add difference to sum */
		SpectralDifferenceValue += Difference;

		/** Store the sample for next time */
		PrevMagnitudeSpectrum_spectralDifference[Index] = magnitudeSpectrum[Index];
	}

	return SpectralDifferenceValue;
}

float UOnsetDetection::GetSpectralDifferenceHWR(const TArray<float>& MagnitudeSpectrum)
{
	float SpectralDifferenceHWRValue{0};

	for (TArray<float>::SizeType Index = 0; Index < MagnitudeSpectrum.Num(); ++Index)
	{
		/** Calculate difference */
		const float Difference = MagnitudeSpectrum[Index] - PrevMagnitudeSpectrum_spectralDifferenceHWR[Index];

		/** Only for positive changes */
		if (Difference > 0)
		{
			/** Add difference to sum */
			SpectralDifferenceHWRValue += Difference;
		}

		/** Store the sample for next time */
		PrevMagnitudeSpectrum_spectralDifferenceHWR[Index] = MagnitudeSpectrum[Index];
	}

	return SpectralDifferenceHWRValue;
}

float UOnsetDetection::GetComplexSpectralDifference(const TArray<float>& FFTReal, const TArray<float>& FFTImaginary)
{
	float ComplexSpectralDifferenceValue{0};

	/** Compute phase values from fft output and sum deviations */
	for (TArray<float>::SizeType Index = 0; Index < FFTReal.Num(); ++Index)
	{
		/** Calculate phase value */
		const float PhaseValue{FMath::Atan2(FFTImaginary[Index], FFTReal[Index])};

		/** Calculate magnitude value */
		const float MagnitudeValue{FMath::Sqrt(FMath::Pow(FFTReal[Index], 2) + FMath::Pow(FFTImaginary[Index], 2))};

		/** Phase deviation */
		const float PhaseDeviation{PhaseValue - (2 * PrevPhaseSpectrum_complexSpectralDifference[Index]) + PrevPhaseSpectrum2_complexSpectralDifference[Index]};

		/** Wrap into [-pi,pi] range */
		const float PhasePiRange{Princarg(PhaseDeviation)};

		/** Calculate magnitude difference (real part of Euclidean distance between complex frames) */
		const float MagnitudeDifference{MagnitudeValue - PrevMagnitudeSpectrum_complexSpectralDifference[Index]};

		/** Calculate phase difference (imaginary part of Euclidean distance between complex frames) */
		const float PhaseDifference{-MagnitudeValue * FMath::Sin(PhasePiRange)};

		/** Square real and imaginary parts, sum and take square root */
		const float Value{FMath::Sqrt(FMath::Pow(MagnitudeDifference, 2) + FMath::Pow(PhaseDifference, 2))};

		ComplexSpectralDifferenceValue += Value;

		/** Store values for the next calculation */
		PrevPhaseSpectrum2_complexSpectralDifference[Index] = PrevPhaseSpectrum_complexSpectralDifference[Index];
		PrevPhaseSpectrum_complexSpectralDifference[Index] = PhaseValue;
		PrevMagnitudeSpectrum_complexSpectralDifference[Index] = MagnitudeValue;
	}

	return ComplexSpectralDifferenceValue;
}

float UOnsetDetection::GetHighFrequencyContent(const TArray<float>& MagnitudeSpectrum)
{
	float HighFrequencyContentValue{0};

	for (TArray<float>::SizeType Index = 0; Index < MagnitudeSpectrum.Num(); ++Index)
	{
		HighFrequencyContentValue += MagnitudeSpectrum[Index] * static_cast<float>(Index + 1);
	}

	return HighFrequencyContentValue;
}
