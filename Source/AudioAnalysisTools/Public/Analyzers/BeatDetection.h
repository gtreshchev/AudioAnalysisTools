// Georgy Treshchev 2022.

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

	/**
	 * BeatDetection constructor
	 */
	UBeatDetection();
	
public:
	/**
	 * Instantiates a Beat Detection object
	 *
	 * @return The BeatDetection object
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	static UBeatDetection* CreateBeatDetection(int32 FFTSubbandsSize = 32, int32 EnergyHistorySize = 41);

public:

	/**
	 * Update FFT sub-bands size
	 *
	 * @param FFTSubbandsSize FFT sub-bands size
	 * @note Commonly used 32
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Update")
	void UpdateFFTSubbandsSize(int32 FFTSubbandsSize = 32);

	/**
	 * Update the size of the energy history storage
	 *
	 * @param EnergyHistorySize Energy history storage size
	 * @note The larger the specified value, the more the detector will remember the magnitudes to calculate the average energy value. However, a value that is too high may affect performance
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Update")
	void UpdateEnergyHistorySize(int32 EnergyHistorySize = 41);

	/**
	 * Process magnitude spectrum
	 * 
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	void ProcessMagnitude(const TArray<float>& MagnitudeSpectrum);

	/**
	 * Calculate if there was beat in the processed magnitude spectrum
	 *
	 * @param Subband FFT sub-band index
	 * @return Whether there was a beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	bool IsBeat(int32 Subband) const;

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
	 * Calculate if there was a hi-hat beat in the processed magnitude spectrum
	 *
	 * @return Whether there was a hi-hat beat or not
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
	bool IsBeatRange(int32 Low, int32 High, int32 Threshold) const;
	
	/**
	 * Get the value of the specified sub-band
	 * 
	 * @param Subband FFT sub-band index
	 * @return The value of the specified sub-band
	 */
	UFUNCTION(BlueprintCallable, Category = "Beat Detection|Main")
	float GetBand(int32 Subband) const;

private:
	/**
	 * Update FFT data (sub-bands, average energy, etc.)
	 * 
	 * @param MagnitudeSpectrum An array containing the magnitude spectrum
	 */
	void UpdateFFT(const TArray<float>& MagnitudeSpectrum);

	/** Raw value for each sub-band */
	TArray<float> FFTSubbands;
	
	/** Average sub-band energy value based on energy history */
	TArray<float> FFTAverageEnergy;

	/** Sub-band variance value */
	TArray<float> FFTVariance;

	/** Normalized beat values for each sub-band */
	TArray<float> FFTBeatValues;

	/** History of energy needed to "memorize" previous magnitudes */
	TArray<TArray<float>> EnergyHistory;

	/** Current position to track energy history */
	int32 HistoryPosition;

	/** FFT sub-bands size (usually 32) */
	int32 FFTSubbandsSize;

	/** Energy history storage size */
	int32 EnergyHistorySize;
};
