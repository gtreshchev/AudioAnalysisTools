// Georgy Treshchev 2021.

#pragma once

#include "EnvelopeAnalysis.generated.h"

/**
 * Possible envelope analysis results
 */
UENUM(BlueprintType, Category = "EnvelopeAnalysis")
enum EEnvelopeAnalysisStatus
{
	SuccessfulAnalysis UMETA(DisplayName = "Success"),
	WasNotFound,
	ZeroAudioData,
	InvalidChannels
};

/**
 * Envelope data including amplitude and time
 */
USTRUCT(BlueprintType, Category = "Envelope Analysis")
struct FEnvelopeTimeData
{
	GENERATED_BODY()

	/** Amplitude at a specific time */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	float Amplitude;

	/** The time when the amplitude was calculated */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	float TimeSec;
};

/**
 * Envelope data including EnvelopeTimeData and arithmetical mean
 */
USTRUCT(BlueprintType, Category = "Envelope Analysis")
struct FAnalysedEnvelopeData
{
	GENERATED_BODY()

	/** Analyzed envelope data including amplitude and time */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	TArray<FEnvelopeTimeData> EnvelopeTimeData;

	/** Arithmetic mean of Envelope */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	float AverageValue;

	FAnalysedEnvelopeData()
		: EnvelopeTimeData(TArray<FEnvelopeTimeData>()), AverageValue(0)
	{
	}

	FAnalysedEnvelopeData(TArray<FEnvelopeTimeData>& InEnvelopeTimeData, float InAverageValue)
		: EnvelopeTimeData(InEnvelopeTimeData), AverageValue(InAverageValue)
	{
	}


};

/**
 * Envelope analysis mode
 */
UENUM(BlueprintType, Category = "Envelope Analysis")
enum EEnvelopeMode
{
	/** Peak method for calculating the envelope */
	Peak UMETA(DisplayName = "Peak"),

	/** Squared method for calculating the envelope */
	Squared UMETA(DisplayName = "Squared")
};

/**
 * Envelope Analysis information
 */
USTRUCT(BlueprintType, Category = "Envelope Analysis")
struct FEnvelopeAnalysisStruct
{
	GENERATED_BODY()

	/** The number of channels for a sound wave. The value is usually between 1 (mono) and 2 (stereo) */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	int32 NumberOfChannels;

	/** How many samples per second is needed for a sound wave */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	float SampleRate;

	/** Frame (window) size for containing samples */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	int32 EnvelopeAnalysisFrameSize;

	/** Attack is the time taken for initial run-up of level from nil to peak, beginning when the key is pressed */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	float AttackTimeMSec;

	/** Release is the time taken for the level to decay to zero after the key is released */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	float ReleaseTimeMSec;

	/** Envelope analysis mode */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	TEnumAsByte<EEnvelopeMode> EnvelopeMode;

	/**
	 * Whether to use an analog or a digital signal in the envelope calculation
	 * Defines how quickly the envelope analysis responds to changes in input
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Envelope Analysis")
	bool bIsAnalog;

	/** Default constructor */
	FEnvelopeAnalysisStruct()
		: NumberOfChannels(0), SampleRate(44100.0f), EnvelopeAnalysisFrameSize(1024), AttackTimeMSec(10.0f),
		  ReleaseTimeMSec(100.0f), EnvelopeMode(Squared), bIsAnalog(true)
	{
	}

	/** Custom constructor */
	FEnvelopeAnalysisStruct(int32 InNumberOfChannels, float InSampleRate, int32 InEnvelopeAnalysisFrameSize,
	                        float InAttackTimeMSec, float InReleaseTimeMSec, TEnumAsByte<EEnvelopeMode> InEnvelopeMode,
	                        bool InbIsAnalog)
		: NumberOfChannels(InNumberOfChannels), SampleRate(InSampleRate),
		  EnvelopeAnalysisFrameSize(InEnvelopeAnalysisFrameSize), AttackTimeMSec(InAttackTimeMSec),
		  ReleaseTimeMSec(InReleaseTimeMSec), EnvelopeMode(InEnvelopeMode), bIsAnalog(InbIsAnalog)
	{
	}
};

/** Delegate broadcast when analysis is complete */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnalysisFinished, const FAnalysedEnvelopeData&, ReadyEnvelopeData,
                                             const TEnumAsByte<EEnvelopeAnalysisStatus>&, Status);

UCLASS(BlueprintType, Category = "Envelope Analysis")
class AUDIOANALYSISTOOLS_API UEnvelopeAnalysis : public UObject
{
	GENERATED_BODY()

	UEnvelopeAnalysis();

public:
	/**
	 * Bind to know when the analysis is complete
	 */
	UPROPERTY(BlueprintAssignable, Category = "Envelope Analysis")
	FOnAnalysisFinished OnAnalysisFinished;

	/**
	 * Instantiates an Envelope Analysis object
	 *
	 * @return The EnvelopeAnalysis object
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	static UEnvelopeAnalysis* CreateEnvelopeAnalysis();


	/**
	 * Reset the state of the envelope analysis
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	void Reset();

	/**
	 * Initiate envelope analysis
	 *
	 * @param EnvelopeAnalysisInfo Envelope analysis information
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	void Init(const FEnvelopeAnalysisStruct EnvelopeAnalysisInfo);

	/**
	 * Analyze and retrieve envelope information using the imported sound wave. Use a delegate to access the result
	 *
	 * @param ImportedSoundWave Imported sound wave object to retrieve wave data
	 * @param EnvelopeAnalysisInfo Envelope analysis information
	 */
	UFUNCTION(BlueprintCallable, Category = "Envelope Analysis")
	void AnalyzeEnvelopeFromImportedSoundWave(class UImportedSoundWave* ImportedSoundWave,
	                                          FEnvelopeAnalysisStruct EnvelopeAnalysisInfo);


private:
	/**
	 * Analyze and retrieve envelope information using sound wave data from memory. Use a delegate to access the result
	 *
	 * @param PCMData Reference to memory location of 16-bit wave byte data.
	 * @param PCMDataSize Memory size allocated for 16-bit wave byte data.
	 * @param EnvelopeAnalysisInfo Envelope analysis information
	 */
	void AnalyzeEnvelopeFromMemory(float* PCMData, const int32 PCMDataSize,
	                               const FEnvelopeAnalysisStruct EnvelopeAnalysisInfo);

	/**
	 * Internal function to analyze and retrieve envelope information
	 *
	 * @param PCMData Pointer to memory location of 32-bit PCM data
	 * @param PCMDataSize Memory size allocated for 32-bit PCM data
	 * @param EnvelopeAnalysisInfo Envelope analysis information
	 */
	void AnalyzeEnvelopeFromMemory_Internal(float* PCMData, const int32 PCMDataSize,
	                                        const FEnvelopeAnalysisStruct EnvelopeAnalysisInfo);

	/**
	 * Calculate current envelope value
	 *
	 * @param InAudioSample Audio sample for analysis
	 */
	void CalculateEnvelope(const float InAudioSample);

	/**
	 * Set attack time. Attack is the time taken for initial run-up of level from nil to peak, beginning when the key is pressed
	 *
	 * @param InAttackTimeMSec Establish what level of attack the calculation should take
	 */
	void SetAttackTime(const float InAttackTimeMSec);

	/**
	 * Set release time. Release is the time taken for the level to decay to zero after the key is released
	 *
	 * @param InReleaseTimeMSec Establish what level of release the calculation should take
	 */
	void SetReleaseTime(const float InReleaseTimeMSec);

	/**
	 * Envelope analysis finish callback
	 * 
	 * @param ReadyEnvelopeData Analyzed envelope data
	 * @param Status Analysis results
	 */
	void OnFinished_Internal(const FAnalysedEnvelopeData& ReadyEnvelopeData,
	                         const TEnumAsByte<EEnvelopeAnalysisStatus> Status);

protected:
	/** Number of channels */
	int32 NumberOfChannels;

	/** Sample rate (samples per second) */
	float SampleRate;

	/** Frame (window) size for containing samples */
	int32 EnvelopeFrameSize;

	/** Attack is the time taken for initial run-up of level from nil to peak, beginning when the key is pressed */
	float AttackTimeMSec;

	/** Samples used for attack time */
	float AttackTimeSamples;

	/** Release is the time taken for the level to decay to zero after the key is released */
	float ReleaseTimeMSec;

	/** Samples used for release time */
	float ReleaseTimeSamples;

	/** Envelope analysis mode */
	TEnumAsByte<EEnvelopeMode> EnvelopeMode;

	/** Whether to use an analog or a digital signal in the envelope calculation */
	bool bIsAnalog;

	/** Last analyzed envelope value */
	float CurrentEnvelopeValue;

	/** Time constants indicate how quickly the envelope analysis responds to changes in input */
	const float AnalogTimeConstant = 1.00239343f;
	const float DigitalTimeConstant = 4.60517019f;
};
