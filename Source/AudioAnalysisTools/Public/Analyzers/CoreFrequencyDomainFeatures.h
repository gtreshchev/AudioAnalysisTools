// Georgy Treshchev 2024.

#pragma once

#include "UObject/Object.h"
#include "CoreFrequencyDomainFeatures.generated.h"

/**
 * Implementations of common frequency domain audio features
 */
UCLASS(BlueprintType, Category = "Core Frequency Domain Features")
class AUDIOANALYSISTOOLS_API UCoreFrequencyDomainFeatures : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Calculate the spectral centroid given the first half of the magnitude spectrum of an audio signal.
	 * Do not pass the whole (i.e. mirrored) magnitude spectrum into this function or you will always get the middle index as the spectral centroid
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @returns The spectral centroid as an index value
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	static float GetSpectralCentroid(const TArray<float>& MagnitudeSpectrum);

	/**
	 * Calculate the spectral centroid given the first half of the magnitude spectrum of an audio signal.
	 * Do not pass the whole (i.e. mirrored) magnitude spectrum into this function or you will always get the middle index as the spectral centroid
	 * Suitable for use with 64-bit data size
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @returns The spectral centroid as an index value
	 */
	static float GetSpectralCentroid(const TArray64<float>& MagnitudeSpectrum);

	/**
	 * Calculate the spectral flatness given the first half of the magnitude spectrum of an audio signal
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @returns The spectral flatness
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	static float GetSpectralFlatness(const TArray<float>& MagnitudeSpectrum);

	/**
	 * Calculate the spectral flatness given the first half of the magnitude spectrum of an audio signal
	 * Suitable for use with 64-bit data size
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @returns The spectral flatness
	 */
	static float GetSpectralFlatness(const TArray64<float>& MagnitudeSpectrum);

	/**
	 * Calculate the spectral crest given the first half of the magnitude spectrum of an audio signal
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @return The spectral crest
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	static float GetSpectralCrest(const TArray<float>& MagnitudeSpectrum);

	/**
	 * Calculate the spectral crest given the first half of the magnitude spectrum of an audio signal
	 * Suitable for use with 64-bit data size
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @return The spectral crest
	 */
	static float GetSpectralCrest(const TArray64<float>& MagnitudeSpectrum);

	/**
	 * Calculate the spectral rolloff given the first half of the magnitude spectrum of an audio signal
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @param Percentile The rolloff threshold
	 * @return The spectral rolloff
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	static float GetSpectralRolloff(const TArray<float>& MagnitudeSpectrum, const float Percentile = 0.85);

	/**
	 * Calculate the spectral rolloff given the first half of the magnitude spectrum of an audio signal
	 * Suitable for use with 64-bit data size
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @param Percentile The rolloff threshold
	 * @return The spectral rolloff
	 */
	static float GetSpectralRolloff(const TArray64<float>& MagnitudeSpectrum, const float Percentile = 0.85);

	/**
	 * Calculate the spectral kurtosis given the first half of the magnitude spectrum of an audio signal
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @return The spectral kurtosis
	 * @note https://en.wikipedia.org/wiki/Kurtosis#Sample_kurtosis
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	static float GetSpectralKurtosis(const TArray<float>& MagnitudeSpectrum);

	/**
	 * Calculate the spectral kurtosis given the first half of the magnitude spectrum of an audio signal
	 * Suitable for use with 64-bit data size
	 *
	 * @param MagnitudeSpectrum The first half of the magnitude spectrum (i.e. not mirrored)
	 * @return The spectral kurtosis
	 * @note https://en.wikipedia.org/wiki/Kurtosis#Sample_kurtosis
	 */
	static float GetSpectralKurtosis(const TArray64<float>& MagnitudeSpectrum);
};
