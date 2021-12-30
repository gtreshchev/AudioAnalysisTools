// Georgy Treshchev 2022.

#pragma once

#include "EnvelopeAnalysisLibrary.h"
#include "OnsetDetectionLibrary.generated.h"

/** Possible onset detection results */
UENUM(BlueprintType, Category = "Onset Detection")
enum class EOnsetDetectionStatus : uint8
{
	/** Successfully detected */
	SuccessfulDetection UMETA(DisplayName = "Success"),

	/** Onset was not found. Apparently the audio data is completely silent for analysis */
	WasNotDetected,

	/** Envelope data is empty */
	EnvelopeTimeDataError,

	/** Buffer detection length is lower than 2 */
	BufferDetectionLengthError,

	/** Step waiting time is less than or equal to zero (relevant when using the Step Unit feature) */
	StepWaitingTimeError,

	/** Timeout waiting time is less than or equal to zero (relevant when using the Timeout feature) */
	TimeoutWaitingTimeError
};

/** Delegate broadcast when onset was successfully detected */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDetectionFinished, const TArray<float>&, DetectedOnsetArray,
                                             const EOnsetDetectionStatus&, Status);


/**
 * Information about the feature of automatic detection by timeout
 */
USTRUCT(BlueprintType, Category = "Onset Detection")
struct FDetectionTimeoutFeature
{
	GENERATED_BODY()

	/** Whether to use this feature of automatic Onset detection or not */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection")
	bool bUseTimeout;

	/** Time after which the Onset should be determined automatically, in seconds */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection",
		meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float WaitingTime;

	/** Constructor disabling timeout feature */
	FDetectionTimeoutFeature()
		: bUseTimeout(false), WaitingTime(0)
	{
	}

	/** Constructor enabling timeout feature */
	FDetectionTimeoutFeature(float InWaitingTime)
		: bUseTimeout(true), WaitingTime(InWaitingTime)
	{
	}
};

/**
 * Information about the detection feature with a step unit
 */
USTRUCT(BlueprintType, Category = "Onset Detection")
struct FDetectionStepUnitFeature
{
	GENERATED_BODY()

	/** Whether to use this feature of the Onset detection with a step unit or not */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection")
	bool bUseStepUnit;

	/** Delay between each step, in seconds. If used, then the recommended value is about "0.25" */
	UPROPERTY(BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "25.0", UIMin = "0.0", UIMax = "25.0"), Category = "Onset Detection")
	float WaitingTime;

	/** Constructor disabling step unit feature */
	FDetectionStepUnitFeature()
		: bUseStepUnit(false), WaitingTime(0)
	{
	}

	/** Constructor enabling step unit feature */
	FDetectionStepUnitFeature(float InWaitingTime)
		: bUseStepUnit(true), WaitingTime(InWaitingTime)
	{
	}
};

/**
 * Information about different Onset Threshold properties
 */
USTRUCT(BlueprintType, Category = "Onset Detection")
struct FOnsetThreshold
{
	GENERATED_BODY()

	/**
	 * How much the Onset value must be exceeded compared to the average value of the Onset Buffer for the Onset Threshold to be successfully detected
	 * The greater this value, the lower the threshold value
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection",
		meta = (ClampMin = "2", ClampMax = "24", UIMin = "2", UIMax = "24"))
	float ThresholdExcessAverageDivider;

	/**
	 * How smoothly change the Onset Threshold value each iteration
	 * The greater this value, the larger and sharper the change
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection",
		meta = (ClampMin = "1", ClampMax = "100", UIMin = "1", UIMax = "100"))
	float ThresholdUpdatingDivider;

	/**
	 * How many seconds to wait if Onset has not been detected for more than the specified number of seconds to adjust the Onset Threshold by decreasing it
	 * The greater this value, the faster and smoother the decrease
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection",
		meta = (ClampMin = "0.2", ClampMax = "15", UIMin = "0.2", UIMax = "15"))
	float ThresholdDecreasingDelay;

	/**
	 * How to smoothly decrease the Onset Threshold when it needs to be adjusted due to the Onset not being detected for more than a specified number of seconds
	 * The greater this value, the larger and sharper the decrease
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection",
		meta = (ClampMin = "2", ClampMax = "24", UIMin = "2", UIMax = "24"))
	float ThresholdDecreasingDivider;

	/** Default constructor */
	FOnsetThreshold()
		: ThresholdExcessAverageDivider(3), ThresholdUpdatingDivider(1.5f), ThresholdDecreasingDelay(3.0f),
		  ThresholdDecreasingDivider(1.25f)
	{
	}

	/** Custom constructor */
	FOnsetThreshold(float InThresholdExcessAverageDivider, float InThresholdUpdatingDivider,
	                float InThresholdDecreasingDelay, float InThresholdDecreasingDivider)
		: ThresholdExcessAverageDivider(InThresholdExcessAverageDivider),
		  ThresholdUpdatingDivider(InThresholdUpdatingDivider), ThresholdDecreasingDelay(InThresholdDecreasingDelay),
		  ThresholdDecreasingDivider(InThresholdDecreasingDivider)
	{
	}
};


/**
 * Onset detection information
 */
USTRUCT(BlueprintType, Category = "Onset Detection")
struct FOnsetDetectionStruct
{
	GENERATED_BODY()

	/** Array to scan Onset, considered to contain Envelope data */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection")
	FAnalysedEnvelopeData AnalysedEnvelopeData;

	/** Information about different Onset Threshold properties */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection")
	FOnsetThreshold OnsetThreshold;

	/** The number of stored elements in the buffer for the detection analysis (recommended range: 32-48) */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection",
		meta = (ClampMin = "2", ClampMax = "2048", UIMin = "2", UIMax = "2048"))
	int32 BufferDetectionLength;

	/** Information about the Timeout Feature */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection")
	FDetectionTimeoutFeature DetectionTimeoutFeature;

	/** Information about the Step Unit feature */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection")
	FDetectionStepUnitFeature DetectionStepUnitFeature;

	/** By how much to multiply the value of each element of the variable */
	UPROPERTY(BlueprintReadWrite, Category = "Onset Detection",
		meta = (ClampMin = "1.0", ClampMax = "100000.0", UIMin = "1.0", UIMax = "100000.0"))
	float OnsetMultiplier;

	/** Default constructor */
	FOnsetDetectionStruct()
		: AnalysedEnvelopeData(FAnalysedEnvelopeData()),
		  OnsetThreshold(FOnsetThreshold()),
		  BufferDetectionLength(48), DetectionTimeoutFeature(FDetectionTimeoutFeature()),
		  DetectionStepUnitFeature(FDetectionStepUnitFeature()), OnsetMultiplier(1)
	{
	}

	/** Custom full constructor */
	FOnsetDetectionStruct(FAnalysedEnvelopeData InAnalysedEnvelopeData, FOnsetThreshold InOnsetThreshold,
	                      int32 InBufferDetectionLength, FDetectionTimeoutFeature InDetectionTimeoutFeature,
	                      FDetectionStepUnitFeature InDetectionStepUnitFeature,
	                      float InOnsetMultiplier)
		: AnalysedEnvelopeData(InAnalysedEnvelopeData),
		  OnsetThreshold(InOnsetThreshold),
		  BufferDetectionLength(InBufferDetectionLength), DetectionTimeoutFeature(InDetectionTimeoutFeature),
		  DetectionStepUnitFeature(InDetectionStepUnitFeature),
		  OnsetMultiplier(InOnsetMultiplier)
	{
	}

	/** Custom constructor without AnalysedEnvelopeData */
	FOnsetDetectionStruct(FOnsetThreshold InOnsetThreshold,
	                      int32 InBufferDetectionLength, FDetectionTimeoutFeature InDetectionTimeoutFeature,
	                      FDetectionStepUnitFeature InDetectionStepUnitFeature,
	                      float InOnsetMultiplier)
		: AnalysedEnvelopeData(FAnalysedEnvelopeData()),
		  OnsetThreshold(InOnsetThreshold),
		  BufferDetectionLength(InBufferDetectionLength), DetectionTimeoutFeature(InDetectionTimeoutFeature),
		  DetectionStepUnitFeature(InDetectionStepUnitFeature),
		  OnsetMultiplier(InOnsetMultiplier)
	{
	}
};

UCLASS(BlueprintType, Category = "Onset Detection")
class ONSETDETECTION_API UOnsetDetectionLibrary : public UObject
{
	GENERATED_BODY()

	UOnsetDetectionLibrary();

public:
	/**
	 * Bind to know when the detection has finished.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Onset Detection")
	FOnDetectionFinished OnDetectionFinished;

private:
	/** Playback time when the last onset was detected. Used to compare the time between current and previous onset detection */
	float TimePreviousDetectedOnset;

	/** Onset threshold value, updated each scanning iteration */
	float OnsetThreshold;

	/** An array of the previous envelope data. A size of an array is defined by "NumberOfStoredEnvelope" variable */
	TArray<float> EnvelopeBufferHistory;

	/**
	 * An array of the already detected onset. It is recommended to get this array from "OnDetectionFinished" delegate.
	 */
	TArray<float> DetectedOnsetArray;

public:
	/**
	 * Instantiates an Onset Detection object
	 *
	 * @return The OnsetDetection object
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	static UOnsetDetectionLibrary* CreateOnsetDetection();


	/**
	 * Detect the onset from Envelope array data. A statistical method is used for the detection
	 *
	 * @param OnsetDetectionInfo Information about the details of the detection
	 */
	UFUNCTION(BlueprintCallable, Category = "Onset Detection")
	void DetectOnsetFromEnvelope(FOnsetDetectionStruct OnsetDetectionInfo);


private:
	/**
	* Internal function to detect the onset from Envelope array data
	*
	* @param OnsetDetectionInfo Information about the details of the detection
	*/
	/**
	 * Internal function to detect the onset from Envelope array data
	 *
	 * @param OnsetDetectionInfo Information about the details of the detection
	 * @return Whether the detection was successful or not
	 */
	bool DetectOnsetFromEnvelope_Internal(FOnsetDetectionStruct OnsetDetectionInfo);


	/**
	 * Function called to check for onset detection
	 *
	 * @param IgnoreAdd Whether to ignore adding the detected onset to the buffer or not
	 * @param PlaybackTime The current sound playback time
	 * @param OnsetMultiplier How much to multiply onset value which is stored in the onset buffer
	 */
	void CheckOnsetDetection(bool IgnoreAdd, float PlaybackTime, float OnsetMultiplier);

	/**
	 * Method called when onset was detected
	 * 
	 * @param DetectedTime audio playback time when onset was detected
	 * @param OnsetMultiplier
	 */
	void OnsetDetected(float DetectedTime, float OnsetMultiplier);

	/**
	 * Shifts all elements down and sets the specified value to the zero element (similar to a circular buffer)
	 *
	 * @param ValueToSet The new value of the zero array element
	 */
	void ShiftElementsDown(float ValueToSet);

	/**
	 * Update threshold value which is considered to be called each iteration
	 *
	 * @param ThresholdExcessAverageDivider How much the Onset value must be exceeded compared to the average value of the Onset Buffer for the Onset Threshold to be successfully detected
	 * @param ThresholdUpdatingDivider How smoothly change the Onset Threshold value each iteration
	 */
	void UpdateThreshold(float ThresholdExcessAverageDivider, float ThresholdUpdatingDivider);

	/**
	 * Decrease threshold value which is considered to be called each iteration if needed
	 *
	 * @param ThresholdDecreasingDivider How to smoothly decrease the Onset Threshold when it needs to be adjusted due to the Onset not being detected for more than a specified number of seconds
	 */
	void DecreaseThreshold(float ThresholdDecreasingDivider);

	/**
	 * Determine if an onset has occurred or not. For the detection, a statistical method is used, comparing the current (zero) array element with the arithmetic mean of all array elements. If the value of the zero element exceeds "OnsetThreshold", onset is considered to be detected
	 *
	 * @param IgnoreAdd Whether to ignore adding the detected onset to the buffer or not
	 * @param PlaybackTime The current sound playback time (supposed to be next)
	 */
	bool DetectOnset(bool IgnoreAdd, float PlaybackTime);

	/**
	 * Onset detection finish callback
	 *
	 * @param InDetectedOnsetArray Detected onset data
	 * @param Status Possible onset detection results
	 */
	void OnFinished_Internal(const TArray<float>& InDetectedOnsetArray,
	                         const EOnsetDetectionStatus& Status);
};
