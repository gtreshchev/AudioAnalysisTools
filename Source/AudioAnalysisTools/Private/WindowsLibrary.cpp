// Georgy Treshchev 2024.

#include "WindowsLibrary.h"

TArray<float> UWindowsLibrary::CreateWindowByType(int32 FrameSize, EAnalysisWindowType WindowType)
{
	return TArray<float>(CreateWindowByType(static_cast<int64>(FrameSize), WindowType));
}

TArray64<float> UWindowsLibrary::CreateWindowByType(int64 FrameSize, EAnalysisWindowType WindowType)
{
	switch (WindowType)
	{
	case EAnalysisWindowType::RectangularWindow: return CreateRectangularWindow(FrameSize);
	case EAnalysisWindowType::HanningWindow: return CreateHanningWindow(FrameSize);
	case EAnalysisWindowType::HammingWindow: return CreateHammingWindow(FrameSize);
	case EAnalysisWindowType::BlackmanWindow: return CreateBlackmanWindow(FrameSize);
	case EAnalysisWindowType::TukeyWindow: return CreateTukeyWindow(FrameSize);
	default: return CreateRectangularWindow(FrameSize);
	}
}

TArray<float> UWindowsLibrary::CreateHanningWindow(int32 FrameSize)
{
	return TArray<float>(CreateHanningWindow(static_cast<int64>(FrameSize)));
}

TArray64<float> UWindowsLibrary::CreateHanningWindow(int64 FrameSize)
{
	TArray64<float> Window;
	Window.Init(0, FrameSize);

	for (TArray64<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		Window[Index] = 0.5 * (1 - FMath::Cos(2. * PI * (Index / static_cast<float>(FrameSize - 1))));
	}

	return Window;
}

TArray<float> UWindowsLibrary::CreateHammingWindow(int32 FrameSize)
{
	return TArray<float>(CreateHammingWindow(static_cast<int64>(FrameSize)));
}

TArray64<float> UWindowsLibrary::CreateHammingWindow(int64 FrameSize)
{
	TArray64<float> Window;
	Window.Init(0, FrameSize);

	for (TArray64<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		Window[Index] = 0.54 - (0.46 * FMath::Cos(2. * PI * (static_cast<float>(Index) / static_cast<float>(FrameSize - 1))));
	}

	return Window;
}

TArray<float> UWindowsLibrary::CreateBlackmanWindow(int32 FrameSize)
{
	return TArray<float>(CreateBlackmanWindow(static_cast<int64>(FrameSize)));
}

TArray64<float> UWindowsLibrary::CreateBlackmanWindow(int64 FrameSize)
{
	TArray64<float> Window;
	Window.Init(0, FrameSize);

	const int64 FrameSizeMinusOne = FrameSize - 1;

	for (TArray64<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		Window[Index] = 0.42 - (0.5 * FMath::Cos(2. * PI * (static_cast<float>(Index) / FrameSizeMinusOne))) + (0.08 * FMath::Cos(4. * PI * (static_cast<float>(Index) / FrameSizeMinusOne)));
	}

	return Window;
}

TArray<float> UWindowsLibrary::CreateTukeyWindow(int32 FrameSize, float CosineFraction)
{
	return TArray<float>(CreateTukeyWindow(static_cast<int64>(FrameSize), CosineFraction));
}

TArray64<float> UWindowsLibrary::CreateTukeyWindow(int64 FrameSize, float CosineFraction)
{
	TArray64<float> Window;
	Window.Init(0, FrameSize);

	const int64 FrameSizeMinusOne = FrameSize - 1;

	float Value = static_cast<float>(-1 * (FrameSize / 2)) + 1;

	for (TArray64<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		if (Value >= 0 && Value <= CosineFraction * (static_cast<float>(FrameSizeMinusOne) / 2))
		{
			Window[Index] = 1.0;
		}
		else if (Value <= 0 && Value >= -1 * CosineFraction * (static_cast<float>(FrameSizeMinusOne) / 2))
		{
			Window[Index] = 1.0;
		}
		else
		{
			Window[Index] = 0.5 * (1 + FMath::Cos(PI * (((2 * Value) / (CosineFraction * FrameSizeMinusOne)) - 1)));
		}

		Value += 1;
	}

	return Window;
}

TArray<float> UWindowsLibrary::CreateRectangularWindow(int32 FrameSize)
{
	return TArray<float>(CreateRectangularWindow(static_cast<int64>(FrameSize)));
}

TArray64<float> UWindowsLibrary::CreateRectangularWindow(int64 FrameSize)
{
	TArray64<float> Window;
	Window.Init(0, FrameSize);

	for (TArray64<float>::SizeType Index = 0; Index < Window.Num(); ++Index)
	{
		Window[Index] = 1.f;
	}

	return Window;
}
