// Georgy Treshchev 2022.

#pragma once

#include "AudioAnalysisToolsDefines.h"

#include "EnvelopeAnalysis.generated.h"

/**
 * Envelope analysis mode
 */
UENUM(BlueprintType, Category = "Envelope Analysis")
enum class EEnvelopeMode : uint8
{
	/** Peak method for calculating the envelope */
	Peak UMETA(DisplayName = "Peak"),

	/** Squared method for calculating the envelope */
	Squared UMETA(DisplayName = "Squared")
};

/**
 * Analyzes the Envelope. Mainly used to reduce PCM Data size to increase performance
 */
UCLASS(BlueprintType, Category = "Envelope Analysis")
class AUDIOANALYSISTOOLS_API UEnvelopeAnalysis : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Instantiates an Envelope Analysis object
	 *
	 * @param NumberOfChannels The number of channels
	 * @param SampleRate Sampling frequency
	 * @param FrameSize Frame (window) size for containing samples
	 * @param AttackTimeMSec The time taken for initial run-up of level from nil to peak, beginning when the key is pressed
	 * @param ReleaseTimeMSec The time taken for the level to decay to zero after the key is released
	 * @param EnvelopeMode Envelope analysis mode
	 * @param bIsAnalog Whether to use an analog or a digital signal in the envelope calculation. Defines how quickly the envelope analysis responds to changes in input
	 * @return The EnvelopeAnalysis object
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	static UEnvelopeAnalysis* CreateEnvelopeAnalysis(int32 NumberOfChannels = 2, int32 SampleRate = 44100, int32 FrameSize = 1024, float AttackTimeMSec = 10.0f, float ReleaseTimeMSec = 100.0f, EEnvelopeMode EnvelopeMode = EEnvelopeMode::Squared, bool bIsAnalog = true);

	/**
	 * Update Sample Rate (sampling frequency) used for the analysis
	 *
	 * @param SampleRate Sampling frequency
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	void UpdateSampleRate(int32 SampleRate);

	/**
	 * Update Frame Size for containing samples
	 *
	 * @param FrameSize Frame (window) size for containing samples
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	void UpdateFrameSize(int32 FrameSize);

private:
	/**
	 * Initialize the envelope analysis
	 *
	 * @param NumberOfChannels The number of channels
	 * @param SampleRate Sampling frequency
	 * @param FrameSize Frame (window) size for containing samples
	 * @param AttackTimeMSec The time taken for initial run-up of level from nil to peak, beginning when the key is pressed
	 * @param ReleaseTimeMSec The time taken for the level to decay to zero after the key is released
	 * @param EnvelopeMode Envelope analysis mode
	 * @param bIsAnalog Whether to use an analog or a digital signal in the envelope calculation. Defines how quickly the envelope analysis responds to changes in input
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	void Initialize(const int32 NumberOfChannels = 2, int32 SampleRate = 44100, int32 FrameSize = 1024, float AttackTimeMSec = 10.0f, float ReleaseTimeMSec = 100.0f, EEnvelopeMode EnvelopeMode = EEnvelopeMode::Squared, bool bIsAnalog = true);

public:
	/**
	 * Calculate envelope data from the PCM Data
	 *
	 * @param PCMData 32-bit float PCM data
	 * @param AnalyzedData Analysed Envelope Data
	 * @return Whether the analysis was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	bool GetEnvelopeValues(const TArray<float>& PCMData, TArray<float>& AnalyzedData);

private:
	/**
	 * Calculate current envelope value
	 *
	 * @param InAudioSample Audio sample for analysis
	 */
	void CalculateEnvelope(float InAudioSample);

	/**
	 * Set attack time. Attack is the time taken for initial run-up of level from nil to peak, beginning when the key is pressed
	 *
	 * @param AttackTimeMSec Establish what level of attack the calculation should take
	 */
	void SetAttackTime(float AttackTimeMSec);

	/**
	 * Set release time. Release is the time taken for the level to decay to zero after the key is released
	 *
	 * @param ReleaseTimeMSec Establish what level of release the calculation should take
	 */
	void SetReleaseTime(float ReleaseTimeMSec);

protected:
	/** Number of channels */
	int32 NumberOfChannels;

	/** Sample rate (samples per second) */
	float SampleRate;

	/** Frame (window) size for containing samples */
	int32 EnvelopeFrameSize;

	/** Samples used for attack time */
	float AttackTimeSamples;

	/** Samples used for release time */
	float ReleaseTimeSamples;

	/** Envelope analysis mode */
	EEnvelopeMode EnvelopeMode;

	/** Whether to use an analog or a digital signal in the envelope calculation */
	bool bIsAnalog;

	/** Last analyzed envelope value */
	float CurrentEnvelopeValue;

	/** Time constants indicate how quickly the envelope analysis responds to changes in input */
	const float AnalogTimeConstant{1.00239343f};
	const float DigitalTimeConstant{4.60517019f};
};
