// Georgy Treshchev 2024.

#include "Analyzers/FFTAudioAnalyzer.h"

#include "AudioAnalysisToolsDefines.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/EngineVersionComparison.h"

#include "Async/ParallelFor.h"

void MultiplySamples(FFTComplexSamples& SamplesOut, const FFTComplexSamples& SamplesA, const FFTComplexSamples& SamplesB)
{
	SamplesOut.Real = SamplesA.Real * SamplesB.Real - SamplesA.Imaginary * SamplesB.Imaginary;
	SamplesOut.Imaginary = SamplesA.Real * SamplesB.Imaginary + SamplesA.Imaginary * SamplesB.Real;
}

void MultiplySamplesBy(FFTComplexSamples& SamplesOut, float Value)
{
	SamplesOut.Real *= Value;
	SamplesOut.Imaginary *= Value;
}

void AddSamples(FFTComplexSamples& SamplesOut, const FFTComplexSamples& SamplesA, const FFTComplexSamples& SamplesB)
{
	SamplesOut.Real = SamplesA.Real + SamplesB.Real;
	SamplesOut.Imaginary = SamplesA.Imaginary + SamplesB.Imaginary;
}

void RemoveSamples(FFTComplexSamples& SamplesOut, const FFTComplexSamples& SamplesA, const FFTComplexSamples& SamplesB)
{
	SamplesOut.Real = SamplesA.Real - SamplesB.Real;
	SamplesOut.Imaginary = SamplesA.Imaginary - SamplesB.Imaginary;
}

void AddSamplesTo(FFTComplexSamples& SamplesOut, const FFTComplexSamples& SamplesIn)
{
	SamplesOut.Real += SamplesIn.Real;
	SamplesOut.Imaginary += SamplesIn.Imaginary;
}

void ApplyExponent(FFTComplexSamples* Samples, double Phase)
{
	Samples->Real = FMath::Cos(Phase);
	Samples->Imaginary = FMath::Sin(Phase);
}

void CalculateButterfly2(FFTComplexSamples* SamplesOut, int64 Stride, const FFTStateStruct* FFTState, int64 StageFFTLength)
{
	const FFTComplexSamples* SamplesTwiddles = FFTState->Twiddles;

	FFTComplexSamples Samples;
	FFTComplexSamples* SamplesOut2 = SamplesOut + StageFFTLength;

	do
	{
		MultiplySamples(Samples, *SamplesOut2, *SamplesTwiddles);

		SamplesTwiddles += Stride;

		RemoveSamples(*SamplesOut2, *SamplesOut, Samples);
		AddSamplesTo(*SamplesOut, Samples);

		++SamplesOut2;
		++SamplesOut;
	}
	while (--StageFFTLength);
}

void CalculateButterfly4(FFTComplexSamples* SamplesOut, int64 Stride, const FFTStateStruct* FFTState, int64 StageFFTLength)
{
	int64 StageFFTLengthTemp = StageFFTLength;

	const int64 StageFFTLength2 = 2 * StageFFTLength;
	const int64 StageFFTLength3 = 3 * StageFFTLength;

	const FFTComplexSamples *Twiddles1, *Twiddles2, *Twiddles3;
	Twiddles3 = Twiddles2 = Twiddles1 = FFTState->Twiddles;

	FFTComplexSamples Scratch[6];

	do
	{
		MultiplySamples(Scratch[0], SamplesOut[StageFFTLength], *Twiddles1);
		MultiplySamples(Scratch[1], SamplesOut[StageFFTLength2], *Twiddles2);
		MultiplySamples(Scratch[2], SamplesOut[StageFFTLength3], *Twiddles3);

		RemoveSamples(Scratch[5], *SamplesOut, Scratch[1]);
		AddSamplesTo(*SamplesOut, Scratch[1]);
		AddSamples(Scratch[3], Scratch[0], Scratch[2]);
		RemoveSamples(Scratch[4], Scratch[0], Scratch[2]);
		RemoveSamples(SamplesOut[StageFFTLength2], *SamplesOut, Scratch[3]);
		Twiddles1 += Stride;
		Twiddles2 += Stride * 2;
		Twiddles3 += Stride * 3;
		AddSamplesTo(*SamplesOut, Scratch[3]);

		if (FFTState->Inverse)
		{
			SamplesOut[StageFFTLength].Real = Scratch[5].Real - Scratch[4].Imaginary;
			SamplesOut[StageFFTLength].Imaginary = Scratch[5].Imaginary + Scratch[4].Real;
			SamplesOut[StageFFTLength3].Real = Scratch[5].Real + Scratch[4].Imaginary;
			SamplesOut[StageFFTLength3].Imaginary = Scratch[5].Imaginary - Scratch[4].Real;
		}
		else
		{
			SamplesOut[StageFFTLength].Real = Scratch[5].Real + Scratch[4].Imaginary;
			SamplesOut[StageFFTLength].Imaginary = Scratch[5].Imaginary - Scratch[4].Real;
			SamplesOut[StageFFTLength3].Real = Scratch[5].Real - Scratch[4].Imaginary;
			SamplesOut[StageFFTLength3].Imaginary = Scratch[5].Imaginary + Scratch[4].Real;
		}
		++SamplesOut;
	}
	while (--StageFFTLengthTemp);
}

void CalculateButterfly3(FFTComplexSamples* SamplesOut, int64 Stride, const FFTStateStruct* FFTState, int64 StageFFTLength)
{
	int64 StageFFTLengthTemp = StageFFTLength;
	const int64 DoubleStageFFTLength = 2 * StageFFTLength;

	const FFTComplexSamples EPI3 = FFTState->Twiddles[Stride * StageFFTLength];

	const FFTComplexSamples* Twiddles2;
	const FFTComplexSamples* Twiddles1 = Twiddles2 = FFTState->Twiddles;

	FFTComplexSamples Scratch[5];

	do
	{
		MultiplySamples(Scratch[1], SamplesOut[StageFFTLength], *Twiddles1);
		MultiplySamples(Scratch[2], SamplesOut[DoubleStageFFTLength], *Twiddles2);

		AddSamples(Scratch[3], Scratch[1], Scratch[2]);
		RemoveSamples(Scratch[0], Scratch[1], Scratch[2]);
		Twiddles1 += Stride;
		Twiddles2 += Stride * 2;

		SamplesOut[StageFFTLength].Real = SamplesOut->Real - (Scratch[3].Real / 2);
		SamplesOut[StageFFTLength].Imaginary = SamplesOut->Imaginary - (Scratch[3].Imaginary / 2);

		MultiplySamplesBy(Scratch[0], EPI3.Imaginary);

		AddSamplesTo(*SamplesOut, Scratch[3]);

		SamplesOut[DoubleStageFFTLength].Real = SamplesOut[StageFFTLength].Real + Scratch[0].Imaginary;
		SamplesOut[DoubleStageFFTLength].Imaginary = SamplesOut[StageFFTLength].Imaginary - Scratch[0].Real;

		SamplesOut[StageFFTLength].Real -= Scratch[0].Imaginary;
		SamplesOut[StageFFTLength].Imaginary += Scratch[0].Real;

		++SamplesOut;
	}
	while (--StageFFTLengthTemp);
}

void CalculateButterfly5(FFTComplexSamples* SamplesOut, int64 Stride, const FFTStateStruct* FFTState, int64 StageFFTLength)
{
	const FFTComplexSamples* Twiddles = FFTState->Twiddles;
	const FFTComplexSamples YaSamples = Twiddles[Stride * StageFFTLength];
	const FFTComplexSamples YbSamples = Twiddles[Stride * 2 * StageFFTLength];

	FFTComplexSamples* SamplesOut0 = SamplesOut;
	FFTComplexSamples* SamplesOut1 = SamplesOut0 + StageFFTLength;
	FFTComplexSamples* SamplesOut2 = SamplesOut0 + 2 * StageFFTLength;
	FFTComplexSamples* SamplesOut3 = SamplesOut0 + 3 * StageFFTLength;
	FFTComplexSamples* SamplesOut4 = SamplesOut0 + 4 * StageFFTLength;

	FFTComplexSamples Scratch[13];

	for (int64 StageFFTIndex = 0; StageFFTIndex < StageFFTLength; ++StageFFTIndex)
	{
		Scratch[0] = *SamplesOut0;

		MultiplySamples(Scratch[1], *SamplesOut1, Twiddles[StageFFTIndex * Stride]);
		MultiplySamples(Scratch[2], *SamplesOut2, Twiddles[2 * StageFFTIndex * Stride]);
		MultiplySamples(Scratch[3], *SamplesOut3, Twiddles[3 * StageFFTIndex * Stride]);
		MultiplySamples(Scratch[4], *SamplesOut4, Twiddles[4 * StageFFTIndex * Stride]);

		AddSamples(Scratch[7], Scratch[1], Scratch[4]);
		RemoveSamples(Scratch[10], Scratch[1], Scratch[4]);
		AddSamples(Scratch[8], Scratch[2], Scratch[3]);
		RemoveSamples(Scratch[9], Scratch[2], Scratch[3]);

		SamplesOut0->Real += Scratch[7].Real + Scratch[8].Real;
		SamplesOut0->Imaginary += Scratch[7].Imaginary + Scratch[8].Imaginary;

		Scratch[5].Real = Scratch[0].Real + (Scratch[7].Real * YaSamples.Real) + (Scratch[8].Real * YbSamples.Real);
		Scratch[5].Imaginary = Scratch[0].Imaginary + (Scratch[7].Imaginary * YaSamples.Real) + (Scratch[8].Imaginary * YbSamples.Real);

		Scratch[6].Real = (Scratch[10].Imaginary * YaSamples.Imaginary) + (Scratch[9].Imaginary * YbSamples.Imaginary);
		Scratch[6].Imaginary = -(Scratch[10].Real * YaSamples.Imaginary) - (Scratch[9].Real * YbSamples.Imaginary);

		RemoveSamples(*SamplesOut1, Scratch[5], Scratch[6]);
		AddSamples(*SamplesOut4, Scratch[5], Scratch[6]);

		Scratch[11].Real = Scratch[0].Real + (Scratch[7].Real * YbSamples.Real) + (Scratch[8].Real * YaSamples.Real);
		Scratch[11].Imaginary = Scratch[0].Imaginary + (Scratch[7].Imaginary * YbSamples.Real) + (Scratch[8].Imaginary * YaSamples.Real);
		Scratch[12].Real = -(Scratch[10].Imaginary * YbSamples.Imaginary) + (Scratch[9].Imaginary * YaSamples.Imaginary);
		Scratch[12].Imaginary = (Scratch[10].Real * YbSamples.Imaginary) - (Scratch[9].Real * YaSamples.Imaginary);

		AddSamples(*SamplesOut2, Scratch[11], Scratch[12]);
		RemoveSamples(*SamplesOut3, Scratch[11], Scratch[12]);

		++SamplesOut0;
		++SamplesOut1;
		++SamplesOut2;
		++SamplesOut3;
		++SamplesOut4;
	}
}

void CalculateButterfly_Generic(FFTComplexSamples* Samples, int64 Stride, const FFTStateStruct* FFTState, int64 StageFFTLength, int64 Radix)
{
	const FFTComplexSamples* Twiddles = FFTState->Twiddles;

	const int64 Norig = FFTState->NFFT;

	FFTComplexSamples* Scratch = static_cast<FFTComplexSamples*>(FMemory::Malloc(sizeof(FFTComplexSamples) * Radix));

	for (int64 StageFFTIndex = 0; StageFFTIndex < StageFFTLength; ++StageFFTIndex)
	{
		int64 RadixIndex;

		int64 TempStageFFTIndex = StageFFTIndex;
		for (RadixIndex = 0; RadixIndex < Radix; ++RadixIndex)
		{
			Scratch[RadixIndex] = Samples[TempStageFFTIndex];
			TempStageFFTIndex += StageFFTLength;
		}

		TempStageFFTIndex = StageFFTIndex;
		for (RadixIndex = 0; RadixIndex < Radix; ++RadixIndex)
		{
			int64 Twidx = 0;
			Samples[TempStageFFTIndex] = Scratch[0];
			
			for (int64 RadixIndex1 = 1; RadixIndex1 < Radix; ++RadixIndex1)
			{
				FFTComplexSamples OutSamples;
				
				Twidx += Stride * TempStageFFTIndex;
				
				if (Twidx >= Norig)
				{
					Twidx -= Norig;
				}
				
				MultiplySamples(OutSamples, Scratch[RadixIndex1], Twiddles[Twidx]);
				AddSamplesTo(Samples[TempStageFFTIndex], OutSamples);
			}
			TempStageFFTIndex += StageFFTLength;
		}
	}

	FMemory::Free(Scratch);
}

void DoWork(FFTComplexSamples* SamplesOut, const FFTComplexSamples* SamplesIn, int64 Stride, int64 InStride, int64* Factors, const FFTStateStruct* FFTState)
{
	FFTComplexSamples* SamplesOut_Beg = SamplesOut;

	const int64 Radix = *Factors++;
	const int64 StageFFTLength = *Factors++;

	const FFTComplexSamples* SamplesOut_End = SamplesOut + Radix * StageFFTLength;

	if (Stride == 1 && Radix <= 5 && StageFFTLength != 1)
	{
		ParallelFor(
			Radix, [&](int64 RadixIndex)
			{
				DoWork(SamplesOut + RadixIndex * StageFFTLength, SamplesIn + Stride * InStride * RadixIndex, Stride * Radix, InStride, Factors, FFTState);
			},
			false);
	}
	else if (StageFFTLength == 1)
	{
		do
		{
			*SamplesOut = *SamplesIn;
			SamplesIn += Stride * InStride;
		}
		while (++SamplesOut != SamplesOut_End);

		SamplesOut = SamplesOut_Beg;
	}
	else
	{
		do
		{
			DoWork(SamplesOut, SamplesIn, Stride * Radix, InStride, Factors, FFTState);
			SamplesIn += Stride * InStride;
		}
		while ((SamplesOut += StageFFTLength) != SamplesOut_End);

		SamplesOut = SamplesOut_Beg;
	}

	switch (Radix)
	{
	case 2:
		CalculateButterfly2(SamplesOut, Stride, FFTState, StageFFTLength);
		break;
	case 3:
		CalculateButterfly3(SamplesOut, Stride, FFTState, StageFFTLength);
		break;
	case 4:
		CalculateButterfly4(SamplesOut, Stride, FFTState, StageFFTLength);
		break;
	case 5:
		CalculateButterfly5(SamplesOut, Stride, FFTState, StageFFTLength);
		break;
	default:
		CalculateButterfly_Generic(SamplesOut, Stride, FFTState, StageFFTLength, Radix);
		break;
	}
}

void UFFTAudioAnalyzer::PerformFFTStride(FFTStateStruct* FFTState, const FFTComplexSamples* SamplesIn, FFTComplexSamples* SamplesOut, int64 Stride)
{
	if (SamplesIn == SamplesOut)
	{
		FFTComplexSamples* TempBuffer = static_cast<FFTComplexSamples*>(FMemory::Malloc(sizeof(FFTComplexSamples) * FFTState->NFFT));

		DoWork(TempBuffer, SamplesIn, 1, Stride, FFTState->Factors, FFTState);

		FMemory::Memcpy(SamplesOut, TempBuffer, sizeof(FFTComplexSamples) * FFTState->NFFT);
		FMemory::Free(TempBuffer);
	}
	else
	{
		DoWork(SamplesOut, SamplesIn, 1, Stride, FFTState->Factors, FFTState);
	}
}

void UFFTAudioAnalyzer::PerformFFT(FFTStateStruct* FFTState, const FFTComplexSamples* SamplesIn, FFTComplexSamples* SamplesOut)
{
	PerformFFTStride(FFTState, SamplesIn, SamplesOut, 1);
}

void CalculateFactors(int64 Number, int64* Factors)
{
	int64 Primes = 4;

#if UE_VERSION_OLDER_THAN(4, 26, 0)
auto ProcessFloor = [](double Value){ return floor(Value); };
#else
auto ProcessFloor = [](double Value){ return FMath::Floor(Value); };
#endif

	const double NumberFlooredSqrt = ProcessFloor(FMath::Sqrt(static_cast<double>(Number)));

	// Factor out powers of 4, powers of 2, then any remaining primes
	do
	{
		while (Number % Primes)
		{
			switch (Primes)
			{
			case 4:
				Primes = 2;
				break;
			case 2:
				Primes = 3;
				break;
			default:
				Primes += 2;
				break;
			}
			if (Primes > NumberFlooredSqrt)
			{
				// No more Factors, skip to end
				Primes = Number;
			}
		}
		Number /= Primes;
		*Factors++ = Primes;
		*Factors++ = Number;
	}
	while (Number > 1);
}

FFTStateStruct* UFFTAudioAnalyzer::PerformFFTAlloc(int64 NFFT, int64 Inverse_FFT, void* MemoryPtr, int64* MemoryLength)
{
	FFTStateStruct* FFTState = nullptr;

	const int64 MemoryRequired = sizeof(FFTStateStruct) + sizeof(FFTComplexSamples) * (NFFT - 1);

	if (MemoryLength == nullptr)
	{
		FFTState = static_cast<FFTStateStruct*>(FMemory::Malloc(MemoryRequired));
	}
	else
	{
		if (MemoryPtr && *MemoryLength >= MemoryRequired)
		{
			FFTState = static_cast<FFTStateStruct*>(MemoryPtr);
		}

		*MemoryLength = MemoryRequired;
	}

	if (FFTState)
	{
		FFTState->NFFT = NFFT;
		FFTState->Inverse = Inverse_FFT;

		for (int64 NFFT_Index = 0; NFFT_Index < NFFT; ++NFFT_Index)
		{
			double Phase = -2 * PI * NFFT_Index / NFFT;

			if (FFTState->Inverse)
			{
				Phase *= -1;
			}

			ApplyExponent(FFTState->Twiddles + NFFT_Index, Phase);
		}

		CalculateFactors(NFFT, FFTState->Factors);
	}

	return FFTState;
}
