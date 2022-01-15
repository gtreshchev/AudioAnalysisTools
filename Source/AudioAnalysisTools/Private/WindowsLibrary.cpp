// Georgy Treshchev 2022.

#include "WindowsLibrary.h"

TArray<float> UWindowsLibrary::CreateWindow(int32 NumOfSamples, EAnalysisWindowType WindowType)
{
	switch (WindowType)
	{
	case EAnalysisWindowType::RectangularWindow: return CreateRectangularWindow(NumOfSamples);
	case EAnalysisWindowType::HanningWindow: return CreateHanningWindow(NumOfSamples);
	case EAnalysisWindowType::HammingWindow: return CreateHammingWindow(NumOfSamples);
	case EAnalysisWindowType::BlackmanWindow: return CreateBlackmanWindow(NumOfSamples);
	case EAnalysisWindowType::TukeyWindow: return CreateTukeyWindow(NumOfSamples);
	default: return CreateRectangularWindow(NumOfSamples);
	}
}

TArray<float> UWindowsLibrary::CreateHanningWindow(int32 NumOfSamples)
{
	TArray<float> Window;
	Window.Init(0,NumOfSamples);

	for (TArray<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		Window[Index] = 0.5 * (1 - FMath::Cos(2. * PI * (Index / static_cast<float>(NumOfSamples - 1))));
	}

	return Window;
}

TArray<float> UWindowsLibrary::CreateHammingWindow(int32 NumOfSamples)
{
	TArray<float> Window;
	Window.Init(0,NumOfSamples);

	for (TArray<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		Window[Index] = 0.54 - (0.46 * FMath::Cos(2. * PI * (static_cast<float>(Index) / static_cast<float>(NumOfSamples - 1))));
	}

	return Window;
}

TArray<float> UWindowsLibrary::CreateBlackmanWindow(int32 NumOfSamples)
{
	TArray<float> Window;
	Window.Init(0,NumOfSamples);

	const float NumOfSamplesMinusOne{static_cast<float>(NumOfSamples - 1)};

	for (TArray<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		Window[Index] = 0.42 - (0.5 * FMath::Cos(2. * PI * (static_cast<float>(Index) / NumOfSamplesMinusOne))) + (0.08 * FMath::Cos(4. * PI * (static_cast<float>(Index) / NumOfSamplesMinusOne)));
	}

	return Window;
}

TArray<float> UWindowsLibrary::CreateTukeyWindow(int32 NumOfSamples, float CosineFraction)
{
	TArray<float> Window;
	Window.Init(0,NumOfSamples);

	const float NumOfSamplesMinusOne{static_cast<float>(NumOfSamples - 1)};

	float Value{static_cast<float>(-1 * (NumOfSamples / 2)) + 1};

	for (TArray<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		if (Value >= 0 && Value <= CosineFraction * (NumOfSamplesMinusOne / 2))
		{
			Window[Index] = 1.0;
		}
		else if (Value <= 0 && Value >= -1 * CosineFraction * (NumOfSamplesMinusOne / 2))
		{
			Window[Index] = 1.0;
		}
		else
		{
			Window[Index] = 0.5 * (1 + FMath::Cos(PI * (((2 * Value) / (CosineFraction * NumOfSamplesMinusOne)) - 1)));
		}

		Value += 1;
	}

	return Window;
}

TArray<float> UWindowsLibrary::CreateRectangularWindow(int32 NumOfSamples)
{
	TArray<float> Window;
	Window.Init(0,NumOfSamples);

	for (TArray<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		Window[Index] = 1.f;
	}

	return Window;
}
