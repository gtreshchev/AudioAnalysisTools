// Georgy Treshchev 2023.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FFTAnalyzer.generated.h"

struct FFTComplexSamples
{
	float Real;
	float Imaginary;
};

#define MAXFACTORS 32

struct FFTStateStruct
{
	int32 NFFT;
	int32 Inverse;
	int32 Factors[2 * MAXFACTORS];
	FFTComplexSamples Twiddles[1];
};

/**
 * FFT Analyzer. Based on https://github.com/mborgerding/kissfft
 */
UCLASS()
class AUDIOANALYSISTOOLS_API UFFTAnalyzer : public UObject
{
	GENERATED_BODY()

public:
	static void PerformFFT(FFTStateStruct* FFTState, const FFTComplexSamples* SamplesIn, FFTComplexSamples* SamplesOut);

	static FFTStateStruct* PerformFFTAlloc(int32 NFFT, int32 Inverse_FFT, void* MemoryPtr, int32* MemoryLength);
	
	static void PerformFFTStride(FFTStateStruct* FFTState, const FFTComplexSamples* SamplesIn, FFTComplexSamples* SamplesOut, int32 Stride);
};
