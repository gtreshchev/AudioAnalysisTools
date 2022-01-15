// Georgy Treshchev 2022.

#pragma once

#include "AudioAnalysisToolsDefines.h"

#include "CoreTimeDomainFeatures.generated.h"

/**
 * Implementations of common time domain audio features
 */
UCLASS(BlueprintType, Category = "Core Time Domain Features")
class AUDIOANALYSISTOOLS_API UCoreTimeDomainFeatures : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Calculate the Root Mean Square (RMS) of an audio buffer in vector format
	 * 
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 * @return The RMS value
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Time Domain Features")
	static float GetRootMeanSquare(const TArray<float>& AudioFrame);
	
	/**
	 * Calculate the peak energy (max absolute value) in a time domain audio signal buffer in vector format
	 * 
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 * @return The peak energy value
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Time Domain Features")
	static float GetPeakEnergy(const TArray<float>& AudioFrame);


	/**
	 * Calculate the zero crossing rate of a time domain audio signal buffer
	 *
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 * @return The zero crossing rate
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Time Domain Features")
	static float GetZeroCrossingRate(const TArray<float>& AudioFrame);
};
