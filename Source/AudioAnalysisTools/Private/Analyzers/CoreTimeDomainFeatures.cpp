// Georgy Treshchev 2024.

#include "Analyzers/CoreTimeDomainFeatures.h"
#include "AudioAnalysisToolsDefines.h"

float UCoreTimeDomainFeatures::GetRootMeanSquare(const TArray<float>& AudioFrame)
{
	return GetRootMeanSquare(TArray64<float>(AudioFrame));
}

float UCoreTimeDomainFeatures::GetRootMeanSquare(const TArray64<float>& AudioFrame)
{
	float Sum = 0;

	// Sum the squared samples
	for (const float Frame : AudioFrame)
	{
		Sum += FMath::Pow(Frame, 2);
	}

	// Return the square root of the mean of squared samples
	return FMath::Sqrt(Sum / static_cast<float>(AudioFrame.Num()));
}

float UCoreTimeDomainFeatures::GetPeakEnergy(const TArray<float>& AudioFrame)
{
	return GetPeakEnergy(TArray64<float>(AudioFrame));
}

float UCoreTimeDomainFeatures::GetPeakEnergy(const TArray64<float>& AudioFrame)
{
	// Create variable with very small value to hold the peak value
	float Peak = -10000.f;

	for (const float Frame : AudioFrame)
	{
		// Store the absolute value of the sample
		const float AbsSample = FMath::Abs(Frame);

		// If the absolute value is larger than the peak
		if (AbsSample > Peak)
		{
			// The peak takes on the sample value
			Peak = AbsSample;
		}
	}

	return Peak;
}

float UCoreTimeDomainFeatures::GetZeroCrossingRate(const TArray<float>& AudioFrame)
{
	return GetZeroCrossingRate(TArray64<float>(AudioFrame));
}

float UCoreTimeDomainFeatures::GetZeroCrossingRate(const TArray64<float>& AudioFrame)
{
	// Create a variable to hold the zero crossing rate
	float ZeroCrossingRateValue{0.f};

	// For each audio sample, starting from the second one
	for (TArray64<float>::SizeType FrameIndex = 1; FrameIndex < AudioFrame.Num(); ++FrameIndex)
	{
		// Initialise two booleans indicating whether or not
		// The current and previous sample are positive

		const bool bIsCurrentPositive{AudioFrame[FrameIndex] > 0};
		const bool bIsPreviousPositive{AudioFrame[FrameIndex - 1] > 0};

		// If the sign is different
		if (bIsCurrentPositive != bIsPreviousPositive)
		{
			// Add one to the zero crossing rate
			ZeroCrossingRateValue += 1.0;
		}
	}

	return ZeroCrossingRateValue;
}
