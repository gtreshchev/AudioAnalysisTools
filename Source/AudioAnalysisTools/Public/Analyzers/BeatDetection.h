// Georgy Treshchev 2024.

#pragma once

#include "UObject/Object.h"
#include "BeatDetection.generated.h"

#define KICK_BAND 0
#define SNARE_BAND 1
#define HIHAT_BAND 2

/**
 * Beat detection
 */
UCLASS(BlueprintType, Category = "Beat Detection")
class AUDIOANALYSISTOOLS_API UBeatDetection : public UObject
{
	GENERATED_BODY()

	UBeatDetection();

public:
	/**
	 * Instantiates a Beat Detection object
	 *
	 * @return The BeatDetection object
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	static UBeatDetection* CreateBeatDetection(UPARAM(DisplayName = "FFT Subband Size") int64 FFTSubbandSize = 32, int64 EnergyHistorySize = 41);

public:
	/**
	 * Update FFT sub-band size
	 *
	 * @param FFTSubbandSize FFT sub-band size
	 * @note Commonly used 32
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update FFT Subband Size"), Category = "Beat Detection|Update")
	void UpdateFFTSubbandSize(UPARAM(DisplayName = "FFT Subband Size") int64 FFTSubbandSize = 32);

	/**
	 * Update the size of the energy history storage
	 *
	 * @param EnergyHistorySize Energy history storage size
	 * @note The larger the specified value, the more the detector will remember the magnitudes to calculate the average energy value. However, a value that is too high may affect performance
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Update")
	void UpdateEnergyHistorySize(int64 EnergyHistorySize = 41);

	/**
	 * Process magnitude spectrum
	 * 
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	void ProcessMagnitude(const TArray<float>& MagnitudeSpectrum);

	/**
	 * Process magnitude spectrum. Suitable for use with 64-bit data size
	 * 
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 */
	void ProcessMagnitude(const TArray64<float>& MagnitudeSpectrum);

	/**
	 * Calculate if there was beat in the processed magnitude spectrum
	 *
	 * @param Subband FFT sub-band index
	 * @return Whether there was a beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	bool IsBeat(int64 Subband) const;

	/**
	 * Calculate if there was a kick beat in the processed magnitude spectrum
	 *
	 * @return Whether there was a kick beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	bool IsKick() const;

	/**
	 * Calculate if there was a snare drum beat in the processed magnitude spectrum
	 *
	 * @return Whether there was a snare drum beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	bool IsSnare() const;

	/**
	 * Calculate if there was a hit-hat beat in the processed magnitude spectrum
	 *
	 * @return Whether there was a hit-hat beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	bool IsHiHat() const;

	/**
	 * Calculate if there is a beat within the given sub-bands span
	 *
	 * @param Low Start FFT sub-band index
	 * @param High End FFT sub-band index
	 * @param Threshold Beat detection threshold
	 * @return Whether there was a beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	bool IsBeatRange(int64 Low, int64 High, int64 Threshold) const;

	/**
	 * Get the value of the specified sub-band
	 * 
	 * @param Subband FFT sub-band index
	 * @return The value of the specified sub-band
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	float GetBand(int64 Subband) const;

	/**
	 * Get FFT Sub-bands values
	 * @return FFT Sub-bands values
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get FFT Subbands"), Category = "Beat Detection|Main")
	TArray<float> GetFFTSubbands_BP() const;

	/**
	 * Get FFT Sub-bands values. Suitable for use with 64-bit data size
	 * @return FFT Sub-bands values
	 */
	const TArray64<float>& GetFFTSubbands() const { return FFTSubbands; }

	/**
	 * Get FFT Average Energy values
	 * @return FFT Average Energy values
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get FFT Average Energy"), Category = "Beat Detection|Main")
	TArray<float> GetFFTAverageEnergy_BP() const;

	/**
	 * Get FFT Average Energy values. Suitable for use with 64-bit data size
	 * @return FFT Average Energy values
	 */
	const TArray64<float>& GetFFTAverageEnergy() const { return FFTAverageEnergy; }

	/**
	 * Get FFT Variance values
	 * @return FFT Variance values
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get FFT Variance"), Category = "Beat Detection|Main")
	TArray<float> GetFFTVariance_BP() const;

	/**
	 * Get FFT Variance values. Suitable for use with 64-bit data size
	 * @return FFT Variance values
	 */
	const TArray64<float>& GetFFTVariance() const { return FFTVariance; }

	/**
	 * Get FFT Beat values
	 * @return FFT Beat values
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get FFT Beat Values"), Category = "Beat Detection|Main")
	TArray<float> GetFFTBeatValues_BP() const;

	/**
	 * Get FFT Beat values. Suitable for use with 64-bit data size
	 * @return FFT Beat values
	 */
	const TArray64<float>& GetFFTBeatValues() const { return FFTBeatValues; }

protected:
	/**
	 * Update FFT data (sub-bands, average energy, etc.)
	 * 
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 */
	void UpdateFFT(const TArray64<float>& MagnitudeSpectrum);

	/** Raw value for each sub-band */
	TArray64<float> FFTSubbands;

	/** Average sub-band energy value based on energy history */
	TArray64<float> FFTAverageEnergy;

	/** Sub-band variance value */
	TArray64<float> FFTVariance;

	/** Normalized beat values for each sub-band */
	TArray64<float> FFTBeatValues;

	/** History of energy needed to "memorize" previous magnitudes */
	TArray64<TArray64<float>> EnergyHistory;

	/** Current position to track energy history */
	int64 HistoryPosition;

	/** FFT sub-band size (usually 32) */
	int64 FFTSubbandSize;

	/** Energy history storage size */
	int64 EnergyHistorySize;
};
