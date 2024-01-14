// Georgy Treshchev 2024.

#pragma once

#include "UObject/Object.h"
#include "OnsetDetection.generated.h"

/**
 * Provides various functions to detect onset
 */
UCLASS(BlueprintType, Category = "Onset Detection")
class AUDIOANALYSISTOOLS_API UOnsetDetection : public UObject
{
	GENERATED_BODY()

	/**
	 * OnsetDetection constructor
	 */
	UOnsetDetection();
public:
	/**
	 * Instantiates an Onset Detection object
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @return The OnsetDetection object
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	static UOnsetDetection* CreateOnsetDetection(int64 FrameSize);

	/**
	 * Update the frame size
	 *
	 * @param FrameSize The frame size of internal buffers
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	void UpdateFrameSize(int32 FrameSize);

	/**
	 * Update the frame size. Suitable for use with 64-bit data size
	 *
	 * @param FrameSize The frame size of internal buffers
	 */
	void UpdateFrameSize(int64 FrameSize);

	/**
	 * Calculate the energy envelope
	 *
	 * @param AudioFrames An array containing audio frame in 32-bit float PCM format
	 * @return The energy difference onset detection function sample for the frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	float GetEnergyEnvelope(const TArray<float>& AudioFrames);

	/**
	 * Calculate the energy envelope. Suitable for use with 64-bit data size
	 *
	 * @param AudioFrames An array containing audio frame in 32-bit float PCM format
	 * @return The energy difference onset detection function sample for the frame
	 */
	float GetEnergyEnvelope(const TArray64<float>& AudioFrames);

	/**
	 * Calculate the energy difference between the current and previous energy sum
	 *
	 * @param AudioFrames An array containing audio frame in 32-bit float PCM format
	 * @return The energy difference onset detection function sample for the frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	float GetEnergyDifference(const TArray<float>& AudioFrames);

	/**
	 * Calculate the energy difference between the current and previous energy sum
	 * Suitable for use with 64-bit data size
	 *
	 * @param AudioFrames An array containing audio frame in 32-bit float PCM format
	 * @return The energy difference onset detection function sample for the frame
	 */
	float GetEnergyDifference(const TArray64<float>& AudioFrames);

	/**
	 * Calculate the spectral difference between the current and the previous magnitude spectrum
	 *
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 * @return The spectral difference onset detection function sample
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	float GetSpectralDifference(const TArray<float>& MagnitudeSpectrum);

	/**
	 * Calculate the spectral difference between the current and the previous magnitude spectrum
	 * Suitable for use with 64-bit data size
	 *
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 * @return The spectral difference onset detection function sample
	 */
	float GetSpectralDifference(const TArray64<float>& MagnitudeSpectrum);

	/**
	 * Calculate the half wave rectified spectral difference between the current and the previous magnitude spectrum
	 *
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 * @return The HWR spectral difference onset detection function sample
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	float GetSpectralDifferenceHWR(const TArray<float>& MagnitudeSpectrum);

	/**
	 * Calculate the half wave rectified spectral difference between the current and the previous magnitude spectrum
	 * Suitable for use with 64-bit data size
	 *
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 * @return The HWR spectral difference onset detection function sample
	 */
	float GetSpectralDifferenceHWR(const TArray64<float>& MagnitudeSpectrum);

	/**
	 * Calculate the complex spectral difference from the real and imaginary parts of the FFT
	 *
	 * @param FFTReal An array containing the real part of the FFT
	 * @param FFTImaginary An array containing the imaginary part of the FFT
	 * @return The complex spectral difference onset detection function sample
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	float GetComplexSpectralDifference(const TArray<float>& FFTReal, const TArray<float>& FFTImaginary);

	/**
	 * Calculate the complex spectral difference from the real and imaginary parts of the FFT
	 * Suitable for use with 64-bit data size
	 *
	 * @param FFTReal An array containing the real part of the FFT
	 * @param FFTImaginary An array containing the imaginary part of the FFT
	 * @return The complex spectral difference onset detection function sample
	 */
	float GetComplexSpectralDifference(const TArray64<float>& FFTReal, const TArray64<float>& FFTImaginary);

	/**
	 * Calculate the high frequency content onset detection function from the magnitude spectrum
	 *
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 * @return The high frequency content onset detection function sample
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	static float GetHighFrequencyContent(const TArray<float>& MagnitudeSpectrum);

	/**
	 * Calculate the high frequency content onset detection function from the magnitude spectrum
	 * Suitable for use with 64-bit data size
	 *
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 * @return The high frequency content onset detection function sample
	 */
	static float GetHighFrequencyContent(const TArray64<float>& MagnitudeSpectrum);

private:
	/**
	 * Set phase values between [-pi:pi] range
	 *
	 * @param PhaseValue the phase value to process
	 * @return The wrapped phase value
	 */
	static FORCEINLINE float Princarg(float PhaseValue)
	{
		// If phase value is less than or equal to -PI then add 2 * PI
		while (PhaseValue <= -PI)
		{
			PhaseValue += 2 * PI;
		}

		// If phase value is larger than PI, then subtract 2 * PI
		while (PhaseValue > PI)
		{
			PhaseValue -= 2 * PI;
		}

		return PhaseValue;
	}

	/** Holds the previous energy sum for the energy difference onset detection function */
	float PreviousEnergySum;

	/** An array containing the previous magnitude spectrum passed to the last spectral difference call */
	TArray64<float> PrevMagnitudeSpectrum_SpectralDifference;

	/** An array containing the previous magnitude spectrum passed to the last spectral difference (half wave rectified) call */
	TArray64<float> PrevMagnitudeSpectrum_SpectralDifferenceHWR;

	/** An array containing the previous phase spectrum passed to the last complex spectral difference call */
	TArray64<float> PrevPhaseSpectrum_ComplexSpectralDifference;

	/** An array containing the second previous phase spectrum passed to the last complex spectral difference call */
	TArray64<float> PrevPhaseSpectrum2_ComplexSpectralDifference;

	/** An array containing the previous magnitude spectrum passed to the last complex spectral difference call */
	TArray64<float> PrevMagnitudeSpectrum_ComplexSpectralDifference;

	int64 FrameSize;
};
