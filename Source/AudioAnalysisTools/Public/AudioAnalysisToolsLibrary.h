// Georgy Treshchev 2022.

#pragma once

#include "UObject/Object.h"
#include "ImportedSoundWave.h"
#include "WindowsLibrary.h"

#define KISS_FFT_MALLOC FMemory::Malloc
#define KISS_FFT_FREE FMemory::Free
#include "ThirdParty/kissfft/kiss_fft.h"

#include "AudioAnalysisToolsLibrary.generated.h"

class UBeatDetection;
class UEnvelopeAnalysis;
class UOnsetDetection;

/**
 * Audio Analysis Tools object. Main class simplifying the analysis of audio data.
 * Works in conjunction with the Runtime Audio Importer plugin.
 */
UCLASS(BlueprintType, Category = "Audio Analysis Tools")
class AUDIOANALYSISTOOLS_API UAudioAnalysisToolsLibrary : public UObject
{
	GENERATED_BODY()

	/**
	 * AudioAnalysisTools constructor
	 */
	UAudioAnalysisToolsLibrary();

	/**
	 * AudioAnalysisTools destructor
	 */
	virtual void BeginDestroy() override;

public:
	/**
	 * Instantiates an Audio Analysis object
	 * 
	 * @param FrameSize The frame size of internal buffers. The smaller the buffer size, the greater the performance, but less accuracy
	 * @param WindowType The type of window function to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Main")
	static UAudioAnalysisToolsLibrary* CreateAudioAnalysisTools(int32 FrameSize = 4096, EAnalysisWindowType WindowType = EAnalysisWindowType::HanningWindow);

	/**
	 * Process audio frame
	 * 
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 * @param bProcessToBeatDetection Whether to process audio frame to beat detection or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Main")
	void ProcessAudioFrame(const TArray<float>& AudioFrame, bool bProcessToBeatDetection = true);

	/**
	 * Get audio frame from sound wave. Intended to be called as the sound wave plays
	 *
	 * @param ImportedSoundWave Sound wave to extract audio data
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Main")
	bool GetAudioFrameFromSoundWave(UImportedSoundWave* ImportedSoundWave, TArray<float>& AudioFrame);

	/**
	 * Get audio frame from sound wave. Intended to be called as the sound wave plays
	 *
	 * @param ImportedSoundWave Sound wave to extract audio data
	 * @param FrameSize The frame size of internal buffers for extracting audio data. The smaller the buffer size, the greater the performance, but less accuracy
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Advanced")
	bool GetAudioFrameFromSoundWaveByFrames(UImportedSoundWave* ImportedSoundWave, int32 FrameSize, TArray<float>& AudioFrame);

	/**
	 * Get audio frame from sound wave. Intended to be called as the sound wave plays
	 *
	 * @param ImportedSoundWave Sound wave to extract audio data
	 * @param StartFrame Start frame size for extracting audio data
	 * @param EndFrame End frame size for extracting audio data
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Advanced")
	bool GetAudioFrameFromSoundWaveByFramesCustom(UImportedSoundWave* ImportedSoundWave, int32 StartFrame, int32 EndFrame, TArray<float>& AudioFrame);

	/**
	 * Get audio frame from sound wave. Intended to be called as the sound wave plays
	 *
	 * @param ImportedSoundWave Sound wave to extract audio data
	 * @param TimeLength Audio length to extract audio data
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Advanced")
	bool GetAudioFrameFromSoundWaveByTime(UImportedSoundWave* ImportedSoundWave, float TimeLength, TArray<float>& AudioFrame);

	/**
	 * Get audio frame from sound wave. Intended to be called as the sound wave plays
	 *
	 * @param ImportedSoundWave Sound wave to extract audio data
	 * @param StartTime Start time for extracting audio data
	 * @param EndTime End time for extracting audio data
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Advanced")
	bool GetAudioFrameFromSoundWaveByTimeCustom(UImportedSoundWave* ImportedSoundWave, float StartTime, float EndTime, TArray<float>& AudioFrame);

	/**
	 * Update the frame size. The smaller the buffer size, the greater the performance, but less accuracy.
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @note You do not need to call it manually if you use "ProcessAudioFrameFromSoundWave"
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Advanced")
	void UpdateFrameSize(int32 FrameSize);

private:
	/**
	 * Initialize Audio Analysis
	 * 
	 * @param FrameSize The frame size of internal buffers. The smaller the buffer size, the greater the performance, but less accuracy
	 * @param WindowType The type of window function to use
	 */
	void Initialize(int32 FrameSize, EAnalysisWindowType WindowType = EAnalysisWindowType::HanningWindow);

public:
	/**
	 * Get magnitude spectrum
	 *
	 * @return The current magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Advanced")
	const TArray<float>& GetMagnitudeSpectrum() const;

	/**
	 * Get FFT Real
	 *
	 * @return The current FFT Real
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get FFT Real"), Category = "Audio Analysis Tools|Analyzers|Advanced")
	const TArray<float>& GetFFTReal() const;

	/**
	 * Get FFT Imaginary
	 *
	 * @return The current FFT Imaginary
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get FFT Imaginary"), Category = "Audio Analysis Tools|Analyzers|Advanced")
	const TArray<float>& GetFFTImaginary() const;

public:
	/**
	 * Calculate if there was beat in the processed magnitude spectrum
	 *
	 * @param Subband FFT sub-band index
	 * @return Whether there was a beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Beat Detection")
	bool IsBeat(int32 Subband) const;

	/**
	 * Calculate if there was a kick beat in the processed magnitude spectrum
	 *
	 * @return Whether there was a kick beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Beat Detection")
	bool IsKick() const;

	/**
	 * Calculate if there was a snare drum beat in the processed magnitude spectrum
	 *
	 * @return Whether there was a snare drum beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Beat Detection")
	bool IsSnare() const;

	/**
	 * Calculate if there was a hi-hat beat in the processed magnitude spectrum
	 *
	 * @return Whether there was a hi-hat beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Beat Detection")
	bool IsHiHat() const;

	/**
	 * Calculate if there is a beat within the given sub-bands span
	 *
	 * @param Low Start FFT sub-band index
	 * @param High End FFT sub-band index
	 * @param Threshold Beat detection threshold
	 * @return Whether there was a beat or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Beat Detection")
	bool IsBeatRange(int32 Low, int32 High, int32 Threshold) const;

	/**
	 * Get the value of the specified sub-band
	 * 
	 * @param Subband FFT sub-band index
	 * @return The value of the specified sub-band
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Beat Detection")
	float GetBand(int32 Subband) const;

	/**
	 * Calculate the Root Mean Square (RMS) of an audio buffer in vector format
	 *
	 * @return The root mean square (RMS) of the currently stored audio frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Core Time Domain Features")
	float GetRootMeanSquare();

	/**
	 * Calculate the peak energy (max absolute value) in a time domain audio signal buffer in vector format
	 *
	 * @return The peak energy of the currently stored audio frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Core Time Domain Features")
	float GetPeakEnergy();

	/**
	 * Calculate the zero crossing rate of a time domain audio signal buffer
	 *
	 * @return The zero crossing rate of the currently stored audio frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Core Time Domain Features")
	float GetZeroCrossingRate();

	/**
	 * Calculate the spectral centroid given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral centroid from the magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Core Frequency Domain Features")
	float GetSpectralCentroid();

	/**
	 * Calculate the spectral flatness given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral flatness of the magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Core Frequency Domain Features")
	float GetSpectralFlatness();

	/**
	 * Calculate the spectral crest given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral crest
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Core Frequency Domain Features")
	float GetSpectralCrest();

	/**
	 * Calculate the spectral rolloff given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral rolloff of the magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Core Frequency Domain Features")
	float GetSpectralRolloff();

	/**
	 * Calculate the spectral kurtosis given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral kurtosis of the magnitude spectrum
	 * @note https://en.wikipedia.org/wiki/Kurtosis#Sample_kurtosis
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Core Frequency Domain Features")
	float GetSpectralKurtosis();

	/**
	 * Calculate the energy difference between the current and previous energy sum
	 *
	 * @return The energy difference onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Onset Detection")
	float GetEnergyDifference();

	/**
	 * Calculate the spectral difference between the current and the previous magnitude spectrum
	 *
	 * @return The spectral difference onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Onset Detection")
	float GetSpectralDifference();

	/**
	 * Calculate the half wave rectified spectral difference between the current and the previous magnitude spectrum
	 *
	 * @return The half wave rectified complex spectral difference onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Onset Detection")
	float GetSpectralDifferenceHWR();

	/**
	 * Calculate the complex spectral difference from the real and imaginary parts of the FFT
	 *
	 * @return the complex spectral difference onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Onset Detection")
	float GetComplexSpectralDifference();

	/**
	 * Calculate the high frequency content onset detection function
	 *
	 * @return The high frequency content onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools|Analyzers|Onset Detection")
	float GetHighFrequencyContent();

private:
	/** Configure the FFT implementation given the audio frame size) */
	void ConfigureFFT();

	/** Whether the FFT is configured or not */
	bool FFTConfigured;

	/** Free all FFT-related data */
	void FreeFFT();

	/** Perform the FFT on the current audio frame */
	void PerformFFT();

	/** Kiss FFT configuration */
	kiss_fft_cfg FFT_Configuration;

	/** FFT input samples, in complex for */
	kiss_fft_cpx* FFT_InSamples;

	/** FFT output samples, in complex form */
	kiss_fft_cpx* FFT_OutSamples;

	/** The real part of the FFT for the current audio frame */
	TArray<float> FFTReal;

	/** The imaginary part of the FFT for the current audio frame */
	TArray<float> FFTImaginary;

private:
	/** The window type used in FFT analysis */
	EAnalysisWindowType WindowType;

	/** Current audio frame */
	TArray<float> CurrentAudioFrame;

	/** The window function used in FFT processing */
	TArray<float> WindowFunction;

	/** The magnitude spectrum of the current audio frame */
	TArray<float> MagnitudeSpectrum;

public:
	/** Reference to the Beat Detection */
	UPROPERTY(BlueprintReadOnly, Category = "Audio Analysis Tools|References")
	UBeatDetection* BeatDetectionRef;

	/** Reference to the Onset Detection */
	UPROPERTY(BlueprintReadOnly, Category = "Audio Analysis Tools|References")
	UOnsetDetection* OnsetDetectionRef;
};
