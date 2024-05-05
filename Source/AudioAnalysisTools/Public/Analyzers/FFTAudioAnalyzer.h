// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FFTAudioAnalyzer.generated.h"

struct FFTComplexSamples
{
	float Real;
	float Imaginary;
};

constexpr int32 MaxFactors = 32;

struct FFTStateStruct
{
	int64 NFFT;
	int64 Inverse;
	int64 Factors[2 * MaxFactors];
	FFTComplexSamples Twiddles[1];
};

/**
 * FFT Analyzer. Based on https://github.com/mborgerding/kissfft
 */
UCLASS()
class AUDIOANALYSISTOOLS_API UFFTAudioAnalyzer : public UObject
{
	GENERATED_BODY()

public:
	static void PerformFFT(FFTStateStruct* FFTState, const FFTComplexSamples* SamplesIn, FFTComplexSamples* SamplesOut);

	static FFTStateStruct* PerformFFTAlloc(int64 NFFT, int64 Inverse_FFT, void* MemoryPtr, int64* MemoryLength);
	
	static void PerformFFTStride(FFTStateStruct* FFTState, const FFTComplexSamples* SamplesIn, FFTComplexSamples* SamplesOut, int64 Stride);
};
