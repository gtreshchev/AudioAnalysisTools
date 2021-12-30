// Georgy Treshchev 2022.

#include "OnsetDetectionLibrary.h"
#include "Async/Async.h"

UOnsetDetectionLibrary::UOnsetDetectionLibrary()
{
	TimePreviousDetectedOnset = 0;
	OnsetThreshold = 0;
}

UOnsetDetectionLibrary* UOnsetDetectionLibrary::CreateOnsetDetection()
{
	UOnsetDetectionLibrary* OnsetDetection = NewObject<UOnsetDetectionLibrary>();
	OnsetDetection->AddToRoot();
	return OnsetDetection;
}

void UOnsetDetectionLibrary::DetectOnsetFromEnvelope(FOnsetDetectionStruct OnsetDetectionInfo)
{
	AsyncTask(ENamedThreads::AnyThread, [=]()
	{
		DetectOnsetFromEnvelope_Internal(OnsetDetectionInfo);
	});
}

bool UOnsetDetectionLibrary::DetectOnsetFromEnvelope_Internal(FOnsetDetectionStruct OnsetDetectionInfo)
{
	if (OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData.Num() <= 0)
	{
		OnFinished_Internal(DetectedOnsetArray, EOnsetDetectionStatus::EnvelopeTimeDataError);
		return false;
	}

	if (OnsetDetectionInfo.BufferDetectionLength < 2)
	{
		OnFinished_Internal(DetectedOnsetArray, EOnsetDetectionStatus::BufferDetectionLengthError);
		return false;
	}

	if (OnsetDetectionInfo.DetectionStepUnitFeature.bUseStepUnit && OnsetDetectionInfo.DetectionStepUnitFeature.
		WaitingTime <= 0.0f)
	{
		OnFinished_Internal(DetectedOnsetArray, EOnsetDetectionStatus::StepWaitingTimeError);
		return false;
	}

	if (OnsetDetectionInfo.DetectionTimeoutFeature.bUseTimeout && OnsetDetectionInfo.DetectionTimeoutFeature.WaitingTime
		<= 0.0f)
	{
		OnFinished_Internal(DetectedOnsetArray, EOnsetDetectionStatus::TimeoutWaitingTimeError);
		return false;
	}

	EnvelopeBufferHistory.Empty();
	EnvelopeBufferHistory.Init(OnsetDetectionInfo.AnalysedEnvelopeData.AverageValue,
	                           OnsetDetectionInfo.BufferDetectionLength);

	for (int32 Index = 0; Index < OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData.Num(); Index++)
	{
		ShiftElementsDown(OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData[Index].Amplitude);

		UpdateThreshold(OnsetDetectionInfo.OnsetThreshold.ThresholdExcessAverageDivider,
		                OnsetDetectionInfo.OnsetThreshold.ThresholdUpdatingDivider);

		if (OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData[Index].TimeSec - TimePreviousDetectedOnset >
			OnsetDetectionInfo.OnsetThreshold.ThresholdDecreasingDelay)
			DecreaseThreshold(OnsetDetectionInfo.OnsetThreshold.ThresholdDecreasingDivider);


		// If the Timeout Feature is used and Onset was not detected for more than specified seconds, Onset will be forcibly marked as detected
		if (OnsetDetectionInfo.DetectionTimeoutFeature.bUseTimeout && (OnsetDetectionInfo.AnalysedEnvelopeData.
			EnvelopeTimeData[Index].TimeSec
			- TimePreviousDetectedOnset) > OnsetDetectionInfo.DetectionTimeoutFeature.WaitingTime)
		{
			OnsetDetected(OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData[Index].TimeSec,
			              OnsetDetectionInfo.OnsetMultiplier);


			TimePreviousDetectedOnset = OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData[Index].TimeSec + (
				OnsetDetectionInfo.DetectionStepUnitFeature.bUseStepUnit
					? OnsetDetectionInfo.DetectionStepUnitFeature.WaitingTime
					: 0);
		}
		else
		{
			// When the Step Unit Feature is used, it is possible to ignore the Onset detection if the previous Onset was detected for a shorter period of time than a certain number of seconds ago
			if (OnsetDetectionInfo.DetectionStepUnitFeature.bUseStepUnit)
			{
				if (OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData[Index].TimeSec - TimePreviousDetectedOnset
					>= OnsetDetectionInfo.DetectionStepUnitFeature.WaitingTime)
					CheckOnsetDetection(false, OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData[Index].TimeSec,
					                    OnsetDetectionInfo.OnsetMultiplier);
				else
					CheckOnsetDetection(true, OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData[Index].TimeSec,
					                    OnsetDetectionInfo.OnsetMultiplier);
			}
				// Otherwise, regular detection
			else
				CheckOnsetDetection(false, OnsetDetectionInfo.AnalysedEnvelopeData.EnvelopeTimeData[Index].TimeSec,
				                    OnsetDetectionInfo.OnsetMultiplier);
		}
	}

	EnvelopeBufferHistory.Empty();

	if (DetectedOnsetArray.Num() <= 0)
	{
		OnFinished_Internal(DetectedOnsetArray, EOnsetDetectionStatus::WasNotDetected);
		return false;
	}

	OnFinished_Internal(DetectedOnsetArray, EOnsetDetectionStatus::SuccessfulDetection);
	DetectedOnsetArray.Empty();
	return true;
}

void UOnsetDetectionLibrary::CheckOnsetDetection(bool IgnoreAdd, float PlaybackTime,
                                          float OnsetMultiplier)
{
	if (DetectOnset(IgnoreAdd, PlaybackTime) && !IgnoreAdd)
	{
		OnsetDetected(PlaybackTime, OnsetMultiplier);
	}
}

void UOnsetDetectionLibrary::OnsetDetected(float DetectedTime, float OnsetMultiplier)
{
	DetectedOnsetArray.Add(DetectedTime * OnsetMultiplier);
}

bool UOnsetDetectionLibrary::DetectOnset(bool IgnoreAdd, float PlaybackTime)
{
	if (EnvelopeBufferHistory[0] > OnsetThreshold)
	{
		if (!IgnoreAdd) TimePreviousDetectedOnset = PlaybackTime;
		return true;
	}
	return false;
}

void UOnsetDetectionLibrary::ShiftElementsDown(float ValueToSet)
{
	for (int32 Index = EnvelopeBufferHistory.Num() - 1; Index > 0; Index--) EnvelopeBufferHistory[Index] =
		EnvelopeBufferHistory[Index - 1];
	EnvelopeBufferHistory[0] = ValueToSet;
}

void UOnsetDetectionLibrary::UpdateThreshold(float ThresholdExcessAverageDivider, float ThresholdUpdatingDivider)
{
	float TempSum = 0;
	for (int32 Index = 0; Index < EnvelopeBufferHistory.Num(); Index++) TempSum = TempSum + EnvelopeBufferHistory[
		Index];

	const float Average = (TempSum / static_cast<float>(EnvelopeBufferHistory.Num() - 1));

	const float TempOnsetThreshold = Average + (Average / ThresholdExcessAverageDivider);

	if (TimePreviousDetectedOnset <= 0 || TempOnsetThreshold <= 0)
		OnsetThreshold = (1.0f / 8.0f) /
			ThresholdExcessAverageDivider;
	else OnsetThreshold = OnsetThreshold + ((TempOnsetThreshold - OnsetThreshold) / ThresholdUpdatingDivider);
}

void UOnsetDetectionLibrary::DecreaseThreshold(float ThresholdDecreasingDivider)
{
	OnsetThreshold = OnsetThreshold - (OnsetThreshold / ThresholdDecreasingDivider);
}

void UOnsetDetectionLibrary::OnFinished_Internal(const TArray<float>& InDetectedOnsetArray,
                                          const EOnsetDetectionStatus& Status)
{
	AsyncTask(ENamedThreads::GameThread, [=]()
	{
		if (OnDetectionFinished.IsBound())
		{
			OnDetectionFinished.Broadcast(InDetectedOnsetArray, Status);
		}
		RemoveFromRoot();
	});
}
