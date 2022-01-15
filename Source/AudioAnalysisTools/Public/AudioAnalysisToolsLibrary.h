// Georgy Treshchev 2022.

#pragma once

#include "RuntimeAudioImporterLibrary.h"
#include "WindowsLibrary.h"

#define KISS_FFT_MALLOC FMemory::Malloc
#define KISS_FFT_FREE FMemory::Free
#include "ThirdParty/kiss_fft.h"

#include "AudioAnalysisToolsLibrary.generated.h"

class UEnvelopeAnalysis;
class UOnsetDetection;
class UPitchDetection;

/**
 * Audio Analysis Tools object. Main class simplifying the analysis of audio data.
 * Works in conjunction with the Runtime Audio Importer plugin.
 */
UCLASS(BlueprintType, Category = "Audio Analysis Tools")
class AUDIOANALYSISTOOLS_API UAudioAnalysisToolsLibrary : public UObject
{
	GENERATED_BODY()

	UAudioAnalysisToolsLibrary();

	/**
	 * Begin Destroy override method
	 */
	virtual void BeginDestroy() override;

public:
	/**
	 * Instantiates an Audio Analysis object
	 * 
	 * @param FrameSize The frame size of internal buffers. The smaller the buffer size, the greater the performance, but less accuracy
	 * @param SampleRate The input audio sample rate. Leave the field blank if you will use "ProcessAudioFrameFromSoundWave"
	 * @param WindowType The type of window function to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	static UAudioAnalysisToolsLibrary* CreateAudioAnalysisTools(int32 FrameSize = 512, int32 SampleRate = 0, EAnalysisWindowType WindowType = EAnalysisWindowType::HanningWindow);

	/**
	 * Process an audio frame from the sound wave
	 * 
	 * @param ImportedSoundWave Sound wave from RuntimeAudioImporter where to get audio data from
	 * @param TimeLength The length of the sound for which to take the audio data, sec
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	bool ProcessAudioFrameFromSoundWave(UImportedSoundWave* ImportedSoundWave, float TimeLength = 0.5f);

	/**
	 * Process an audio frame from the sound wave
	 * 
	 * @param ImportedSoundWave Sound wave from RuntimeAudioImporter where to get audio data from
	 * @param StartTime Start time for analysis
	 * @param EndTime End time for analysis
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	bool ProcessAudioFrameFromSoundWave_Custom(UImportedSoundWave* ImportedSoundWave, float StartTime, float EndTime);


	/**
	 * Update the Sample Rate used for the analysis
	 *
	 * @param SampleRate The sampling frequency
	 * @note You do not need to call it manually if you use "ProcessAudioFrameFromSoundWave"
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	void UpdateSampleRate(int32 SampleRate);

	/**
	 * Update the frame size. The smaller the buffer size, the greater the performance, but less accuracy.
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @note You do not need to call it manually if you use "ProcessAudioFrameFromSoundWave"
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	void UpdateFrameSize(int32 FrameSize);

private:
	/**
	 * Initialize Audio Analysis
	 * 
	 * @param FrameSize The frame size of internal buffers. The smaller the buffer size, the greater the performance, but less accuracy
	 * @param SampleRate The input audio sample rate. Leave the field blank if you will use "ProcessAudioFrameFromSoundWave"
	 * @param WindowType The type of window function to use
	 */
	void Initialize(int32 FrameSize, int32 SampleRate, EAnalysisWindowType WindowType = EAnalysisWindowType::HanningWindow);

public:
	/**
	 * Get the audio frame from the sound wave. AudioFrame shrinks to fit FrameSize
	 *
	 * @param ImportedSoundWave Sound wave from RuntimeAudioImporter where to get audio data from
	 * @param StartTime Start time for analysis
	 * @param EndTime End time for analysis
	 * @param AudioFrame Analyzed audio frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	bool GetAudioFrameFromSoundWave(UImportedSoundWave* ImportedSoundWave, float StartTime, float EndTime, TArray<float>& AudioFrame);

	/**
	 * Process an audio frame
	 * 
	 * @param AudioFrame An array containing audio frame in 32-bit float PCM format
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	void ProcessAudioFrame(TArray<float> AudioFrame);

public:
	/**
	 * Gist automatically calculates the magnitude spectrum when processAudioFrame() is called, this function returns it.
	 *
	 * @return The current magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	const TArray<float>& GetMagnitudeSpectrum();

public:
	/**
	 * Calculate envelope data
	 *
	 * @param AnalyzedData Analysed Envelope Data
	 * @return Whether the analysis was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	bool GetEnvelopeValues(TArray<float>& AnalyzedData);

public:
	/**
	 * Calculate the Root Mean Square (RMS) of an audio buffer in vector format
	 *
	 * @return The root mean square (RMS) of the currently stored audio frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Time Domain Features")
	float GetRootMeanSquare();

	/**
	 * Calculate the peak energy (max absolute value) in a time domain audio signal buffer in vector format
	 *
	 * @return the peak energy of the currently stored audio frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Time Domain Features")
	float GetPeakEnergy();

	/**
	 * Calculate the zero crossing rate of a time domain audio signal buffer
	 *
	 * @return The zero crossing rate of the currently stored audio frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Time Domain Features")
	float GetZeroCrossingRate();

public:
	/**
	 * Calculate the spectral centroid given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral centroid from the magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	float GetSpectralCentroid();

	/**
	 * Calculate the spectral flatness given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral flatness of the magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	float GetSpectralFlatness();

	/**
	 * Calculate the spectral crest given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral crest
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	float GetSpectralCrest();

	/**
	 * Calculate the spectral rolloff given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral rolloff of the magnitude spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	float GetSpectralRolloff();

	/**
	 * Calculate the spectral kurtosis given the first half of the magnitude spectrum of an audio signal
	 *
	 * @return The spectral kurtosis of the magnitude spectrum
	 * @note https://en.wikipedia.org/wiki/Kurtosis#Sample_kurtosis
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Frequency Domain Features")
	float GetSpectralKurtosis();

public:
	/**
	 * Calculate the energy difference between the current and previous energy sum
	 *
	 * @return The energy difference onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	float GetEnergyDifference();

	/**
	 * Calculate the spectral difference between the current and the previous magnitude spectrum
	 *
	 * @return The spectral difference onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	float GetSpectralDifference();

	/**
	 * Calculate the half wave rectified spectral difference between the current and the previous magnitude spectrum
	 *
	 * @return The half wave rectified complex spectral difference onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	float GetSpectralDifferenceHWR();

	/**
	 * Calculate the complex spectral difference from the real and imaginary parts of the FFT
	 *
	 * @return the complex spectral difference onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	float GetComplexSpectralDifference();

	/**
	 * Calculate the high frequency content onset detection function
	 *
	 * @return The high frequency content onset detection function sample for the magnitude spectrum frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
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
	/** The sampling frequency used for analysis */
	int32 SampleRate;

	/** The window type used in FFT analysis */
	EAnalysisWindowType WindowType;

	/** Current audio frame */
	TArray<float> AudioFrame;

	/** The window function used in FFT processing */
	TArray<float> WindowFunction;

	/** The magnitude spectrum of the current audio frame */
	TArray<float> MagnitudeSpectrum;

public:
	/** Reference to the Envelope Analysis */
	UPROPERTY(BlueprintReadOnly, Category = "Audio Analysis Tools")
	UEnvelopeAnalysis* EnvelopeAnalysisRef;

	/** Reference to the Onset Detection */
	UPROPERTY(BlueprintReadOnly, Category = "Audio Analysis Tools")
	UOnsetDetection* OnsetDetectionRef;
};
