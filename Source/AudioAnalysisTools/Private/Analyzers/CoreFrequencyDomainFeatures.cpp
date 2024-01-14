// Georgy Treshchev 2024.

#include "Analyzers/CoreFrequencyDomainFeatures.h"
#include "AudioAnalysisToolsDefines.h"
#include "Algo/Accumulate.h"

float UCoreFrequencyDomainFeatures::GetSpectralCentroid(const TArray<float>& MagnitudeSpectrum)
{
	return GetSpectralCentroid(TArray64<float>(MagnitudeSpectrum));
}

float UCoreFrequencyDomainFeatures::GetSpectralCentroid(const TArray64<float>& MagnitudeSpectrum)
{
	float SumAmplitudes = 0;

	float SumWeightedAmplitudes = 0;

	// For each bin in the first half of the magnitude spectrum
	for (TArray64<float>::SizeType MagnitudeIndex = 0; MagnitudeIndex < MagnitudeSpectrum.Num(); MagnitudeIndex++)
	{
		// Sum amplitudes
		SumAmplitudes += MagnitudeSpectrum[MagnitudeIndex];

		// Sum amplitudes weighted by the bin number
		SumWeightedAmplitudes += MagnitudeSpectrum[MagnitudeIndex] * MagnitudeIndex;
	}

	const float SpectralCentroidValue{SumAmplitudes > 0 ? SumWeightedAmplitudes / SumAmplitudes : 0.f};

	return SpectralCentroidValue;
}

float UCoreFrequencyDomainFeatures::GetSpectralFlatness(const TArray<float>& MagnitudeSpectrum)
{
	return GetSpectralFlatness(TArray64<float>(MagnitudeSpectrum));
}

float UCoreFrequencyDomainFeatures::GetSpectralFlatness(const TArray64<float>& MagnitudeSpectrum)
{
	float SumValue = 0;
	float LogSumValue = 0;

	for (const float MagnitudeValue : MagnitudeSpectrum)
	{
		// Add one to stop zero values making it always zero
		const float Value{1 + MagnitudeValue};

		SumValue += Value;
		LogSumValue += FGenericPlatformMath::Loge(Value);
	}

	SumValue = SumValue / static_cast<float>(MagnitudeSpectrum.Num());
	LogSumValue = LogSumValue / static_cast<float>(MagnitudeSpectrum.Num());

	const float SpectralFlatnessValue = SumValue > 0 ? FGenericPlatformMath::Exp(LogSumValue) / SumValue : 0.f;

	return SpectralFlatnessValue;
}

float UCoreFrequencyDomainFeatures::GetSpectralCrest(const TArray<float>& MagnitudeSpectrum)
{
	return GetSpectralCrest(TArray64<float>(MagnitudeSpectrum));
}

float UCoreFrequencyDomainFeatures::GetSpectralCrest(const TArray64<float>& MagnitudeSpectrum)
{
	float SumValue = 0;
	float MaxValue = 0;

	for (const float MagnitudeValue : MagnitudeSpectrum)
	{
		const float Value{FMath::Pow(MagnitudeValue, 2)};

		SumValue += Value;

		if (Value > MaxValue)
		{
			MaxValue = Value;
		}
	}

	float SpectralCrestValue;

	if (SumValue > 0)
	{
		const float MeanValue{SumValue / static_cast<float>(MagnitudeSpectrum.Num())};
		SpectralCrestValue = MaxValue / MeanValue;
	}
	else
	{
		// This is a ratio so we return 1.0 if the buffer is just zeros
		SpectralCrestValue = 1.0;
	}

	return SpectralCrestValue;
}

float UCoreFrequencyDomainFeatures::GetSpectralRolloff(const TArray<float>& MagnitudeSpectrum, const float Percentile)
{
	return GetSpectralRolloff(TArray64<float>(MagnitudeSpectrum), Percentile);
}

float UCoreFrequencyDomainFeatures::GetSpectralRolloff(const TArray64<float>& MagnitudeSpectrum, const float Percentile)
{
	TArray64<float>::SizeType Index{0};

	{
		const float SumOfMagnitudeSpectrum{Algo::Accumulate<float>(MagnitudeSpectrum, 0.f)};
		const float Threshold{SumOfMagnitudeSpectrum * Percentile};

		float CumulativeSum{0};

		for (TArray64<float>::SizeType i = 0; i < MagnitudeSpectrum.Num(); ++i)
		{
			CumulativeSum += MagnitudeSpectrum[i];

			if (CumulativeSum > Threshold)
			{
				Index = i;
				break;
			}
		}
	}

	const float SpectralRolloff{static_cast<float>(Index) / static_cast<float>(MagnitudeSpectrum.Num())};

	return SpectralRolloff;
}

float UCoreFrequencyDomainFeatures::GetSpectralKurtosis(const TArray<float>& MagnitudeSpectrum)
{
	return GetSpectralKurtosis(TArray64<float>(MagnitudeSpectrum));
}

float UCoreFrequencyDomainFeatures::GetSpectralKurtosis(const TArray64<float>& MagnitudeSpectrum)
{
	float Moment2{0.f};
	float Moment4{0.f};

	{
		const float SumOfMagnitudeSpectrum{Algo::Accumulate<float>(MagnitudeSpectrum, 0.f)};
		const float Mean{SumOfMagnitudeSpectrum / static_cast<float>(MagnitudeSpectrum.Num())};

		for (const float MagnitudeValue : MagnitudeSpectrum)
		{
			const float Difference{MagnitudeValue - Mean};
			const float SquaredDifference{FMath::Pow(Difference, 2)};

			Moment2 += SquaredDifference;
			Moment4 += SquaredDifference * SquaredDifference;
		}
	}

	Moment2 = Moment2 / static_cast<float>(MagnitudeSpectrum.Num());
	Moment4 = Moment4 / static_cast<float>(MagnitudeSpectrum.Num());

	if (Moment2 == 0)
	{
		return -3.f;
	}

	return (Moment4 / FMath::Pow(Moment4, 2)) - 3.f;
}
