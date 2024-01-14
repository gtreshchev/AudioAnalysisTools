// Georgy Treshchev 2024.

#pragma once

#include "WindowsLibrary.generated.h"

/**
 * A type indicator for different windows.
 * Used in spectral analysis, in particular for the analysis of Magnitude Spectrum and Real and Imaginary parts of FFT
 * More information: https://en.wikipedia.org/wiki/Window_function
 */
UENUM()
enum class EAnalysisWindowType : uint8
{
	RectangularWindow,
	HanningWindow,
	HammingWindow,
	BlackmanWindow,
	TukeyWindow
};

/**
 * Library for creating windows functions of different types
 */
UCLASS(BlueprintType, Category = "Window Library")
class AUDIOANALYSISTOOLS_API UWindowsLibrary : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Create a window with a specified type. It is used in spectral analysis
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @param WindowType A type of the window
	 * @return A window with the specified type
	 */
	UFUNCTION(BlueprintCallable, Category = "Window Library")
	static TArray<float> CreateWindowByType(int32 FrameSize, EAnalysisWindowType WindowType);

	/**
	 * Create a window with a specified type. It is used in spectral analysis
	 * Suitable for use with 64-bit data size
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @param WindowType A type of the window
	 * @return A window with the specified type
	 */
	static TArray64<float> CreateWindowByType(int64 FrameSize, EAnalysisWindowType WindowType);

	/**
	 * Create a window with Hanning type. It is used in spectral analysis
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @return A window with a Hanning type
	 */
	UFUNCTION(BlueprintCallable, Category = "Window Library")
	static TArray<float> CreateHanningWindow(int32 FrameSize);

	/**
	 * Create a window with Hanning type. It is used in spectral analysis
	 * Suitable for use with 64-bit data size
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @return A window with a Hanning type
	 */
	static TArray64<float> CreateHanningWindow(int64 FrameSize);

	/**
	 * Create a window with Hamming type. It is used in spectral analysis
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @return A window with a Hamming type
	 */
	UFUNCTION(BlueprintCallable, Category = "Window Library")
	static TArray<float> CreateHammingWindow(int32 FrameSize);

	/**
	 * Create a window with Hamming type. It is used in spectral analysis
	 * Suitable for use with 64-bit data size
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @return A window with a Hamming type
	 */
	static TArray64<float> CreateHammingWindow(int64 FrameSize);

	/**
	 * Create a window with Blackman type. It is used in spectral analysis
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @return A window with a Blackman type
	 */
	UFUNCTION(BlueprintCallable, Category = "Window Library")
	static TArray<float> CreateBlackmanWindow(int32 FrameSize);

	/**
	 * Create a window with Blackman type. It is used in spectral analysis
	 * Suitable for use with 64-bit data size
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @return A window with a Blackman type
	 */
	static TArray64<float> CreateBlackmanWindow(int64 FrameSize);

	/**
	 * Create a window with Tukey type. It is used in spectral analysis
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @param CosineFraction Cosine fraction (or Alpha) used for a Tukey window type
	 * @return A window with a Tukey type
	 */
	UFUNCTION(BlueprintCallable, Category = "Window Library")
	static TArray<float> CreateTukeyWindow(int32 FrameSize, float CosineFraction = 0.5);

	/**
	 * Create a window with Tukey type. It is used in spectral analysis
	 * Suitable for use with 64-bit data size
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @param CosineFraction Cosine fraction (or Alpha) used for a Tukey window type
	 * @return A window with a Tukey type
	 */
	static TArray64<float> CreateTukeyWindow(int64 FrameSize, float CosineFraction = 0.5);

	/**
	 * Create a window with Rectangular type. It is used in spectral analysis
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @return A window with a Rectangular type
	 */
	UFUNCTION(BlueprintCallable, Category = "Window Library")
	static TArray<float> CreateRectangularWindow(int32 FrameSize);

	/**
	 * Create a window with Rectangular type. It is used in spectral analysis
	 * Suitable for use with 64-bit data size
	 *
	 * @param FrameSize The frame size of internal buffers
	 * @return A window with a Rectangular type
	 */
	static TArray64<float> CreateRectangularWindow(int64 FrameSize);
};