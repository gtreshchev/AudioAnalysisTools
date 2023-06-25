
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AudioAnalysisToolsLibrary.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "MyBlueprintFunctionLibrary.generated.h"


//USTRUCT(BlueprintType)
//struct FFrequancyCurve
//{
//	GENERATED_BODY()
//
//	//Allows you to multiply this color channel by a specified amount.
//	UPROPERTY(BlueprintReadWrite)
//	float BrightnessMultiplier = 1.0;
//
//	//Allows you to set the max value of the First Curve.
//	UPROPERTY(BlueprintReadWrite)
//	float FirstCurveHeight = 0.0;
//
//	//Allows you to define how the color values will react across volume levels. 0 will disable this curve. 0.5 will quickly curve in, then taper off, and 2.0 will curve in flat, then quickly shoot up as volume approaches 1.
//	UPROPERTY(BlueprintReadWrite)
//	float FirstCurveExponent = 1.0;
//
//	//Allows you to set the max value of the First Curve.
//	UPROPERTY(BlueprintReadWrite)
//	float SecondCurveHeight = 0.0;
//
//	//Allows you to define how the color values will react across volume levels. 0 will disable this curve. 0.5 will quickly curve in, then taper off, and 2.0 will curve in flat, then quickly shoot up as volume approaches 1.
//	UPROPERTY(BlueprintReadWrite)
//	float SecondCurveExponent = 1.0;
//
//	//Allows you to set the max value of the First Curve.
//	UPROPERTY(BlueprintReadWrite)
//	float ThirdCurveHeight = 1.0;
//
//	//Allows you to define how the color values will react across volume levels. 0 will disable this curve. 0.5 will quickly curve in, then taper off, and 2.0 will curve in flat, then quickly shoot up as volume approaches 1.
//	UPROPERTY(BlueprintReadWrite)
//	float ThirdCurveExponent = 1.0;
//};
//
//
////Allows you to set and toggle (for performance) how you want the spectrogram to be colored.
//USTRUCT(BlueprintType)
//struct FSpectrogramColorSettings
//{
//	GENERATED_BODY()
//
//	//Removes the Red color channel and increases performance.
//	UPROPERTY(BlueprintReadWrite)
//	bool UseRed = true;
//
//	//Allows you to set how the Red color channel reacts to input spectrogram height values.
//	UPROPERTY(BlueprintReadWrite)
//	FFrequancyCurve Red;
//
//	//Removes the Green color channel and increases performance.
//	UPROPERTY(BlueprintReadWrite)
//	bool UseGreen = true;
//
//	//Allows you to set how the Green color channel reacts to input spectrogram height values.
//	UPROPERTY(BlueprintReadWrite)
//	FFrequancyCurve Green;
//
//	//Removes the Blue color channel and increases performance.
//	UPROPERTY(BlueprintReadWrite)
//	bool UseBlue = true;
//
//	//Allows you to set how the Blue color channel reacts to input spectrogram height values.
//	UPROPERTY(BlueprintReadWrite)
//	FFrequancyCurve Blue;
//
//	//Removes the Alpha (transparency) color channel and increases performance.
//	UPROPERTY(BlueprintReadWrite)
//	bool UseAlpha = true;
//
//	//Allows you to set how the Alpha color channel reacts to input spectrogram height values.
//	UPROPERTY(BlueprintReadWrite)
//	FFrequancyCurve Alpha;
//};


//Input nodes for the fft. Allows you to simplify your setup when it comes to making a thread pool.
USTRUCT(BlueprintType)
struct FSpectrogramInput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UImportedSoundWave> ImportedSoundWave;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UAudioAnalysisToolsLibrary> AudioAnalysisObject;

	//Allows you to tell the function how many threads are going to be used total. Set to 1 for a single thread.
	UPROPERTY(BlueprintReadWrite)
	int32 ThreadCount;
	
	//Determines how many times you wish for the spectrogram to calculate across a second.
	UPROPERTY(BlueprintReadWrite)
	int32 SpectrogramSamples;
	
	//Determines how many bands you wish for the spectrogram to calculate. Value MUST be a multiple of two.
	UPROPERTY(BlueprintReadWrite)
	int32 SpectrogramBands;
	
	//defines how many bands you wish to chop from the low frequencies of the texture, as a float from 0 to 1. set to 0 to include everything from 0 up.
	UPROPERTY(BlueprintReadWrite)
	float BandsMin;

	//defines how many bands you wish to chop from the high frequencies of the texture, as a float from 0 to 1. set to 1 to include everything from 0 up.
	UPROPERTY(BlueprintReadWrite)
	float BandsMax;
};

//Input nodes for the waveform. Allows you to simplify your setup when it comes to making a thread pool.
USTRUCT(BlueprintType)
struct FWaveformInput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UImportedSoundWave> ImportedSoundWave;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UAudioAnalysisToolsLibrary> AudioAnalysisObject;

	//Allows you to tell the function how many threads are going to be used total. Set to 1 for a single thread.
	UPROPERTY(BlueprintReadWrite)
	int32 ThreadCount;

	//Determines how often you wish to sample the original sound wave. Values higher than 2^14 will not generate.
	UPROPERTY(BlueprintReadWrite)
	int32 WaveformSampleRate;

	//Determines the audio granularity of the waveform. also determines the texture width. Recommended to be an even number. Values of 256 are recommended.
	UPROPERTY(BlueprintReadWrite)
	int32 WaveformAudioGranularity;

};

UENUM(BlueprintType)
enum FGenerationStatus
{

	Loop			UMETA(DisplayName = "Continue Looping"),

	InvalidObject	UMETA(DisplayName = "Objects Failed to Verify"),

	DontLoop		UMETA(DisplayName = "End Reached"),

};

UENUM(BlueprintType)
enum FGenerationType
{

	Waveform		UMETA(DisplayName = "Waveform"),

	Spectrogram		UMETA(DisplayName = "Spectrogram"),

};


//Output from the delegate. break apart to get the goodies inside.
USTRUCT(BlueprintType)
struct FSpectrogramOutput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, meta = (ToolTip = "generation status"))
	TEnumAsByte<FGenerationStatus> Status;

	UPROPERTY(BlueprintReadWrite, meta = (ToolTip = "Stick back into the function"))
	int32 ChunkIndex;

	UPROPERTY(BlueprintReadWrite, meta = (ToolTip = "Where in the song this particular chunk is representing in seconds", DisplayName = "Time (Seconds)"))
	int32 Time;

	UPROPERTY(BlueprintReadWrite, meta = (ToolTip = "The texture for this chunk"))
	TWeakObjectPtr<UTexture2D> Texture;

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGeneratedTextures, FSpectrogramOutput, Output);

UCLASS(BlueprintType, Category = "Dans FFT Calculation Stuff")
class CUSTOMSONGEDITOR_API UMyBlueprintFunctionLibrary : public UObject
{

	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Dans FFT Calculation Stuff")
	FGeneratedTextures DoneCalculating;

	UFUNCTION(BlueprintCallable, Category = "Dans FFT Calculation Stuff")
	static UMyBlueprintFunctionLibrary* CreateMyBlueprintLib();

	/** Calculates the frequency spectrum for a window of time for the SoundWave
	 * @param Samples - The Samples to use to generate the spectrogram
	 * @param Channel - The channel of the sound to calculate.  Specify 0 to combine channels together
	 * @param StartTime - The beginning of the window to calculate the spectrum of
	 * @param TimeLength - The duration of the window to calculate the spectrum of
	 * @return OutSpectrum - The resulting spectrum
	 */
	UFUNCTION(BlueprintCallable, Category = "Dans FFT Calculation Stuff")
	static void CalculateFFT(const TArray<float> samples, const int32 NumChannels, const int32 SampleRate, TArray<float>& OutFrequencies, FString& Warnings);

	
	/** Generates a 3D Spectrogram. Each chunk is 1 second in size
	* @param ImportedSoundWave - The audio to use to generate the spectrogram.
	* @param AudioAnalysisObject - Ref
	* @param ChunkIndex - What index (and therefor second) you wish to calculate and generate.
	* @param ThreadIndex - For multithreading.
	* @param SpectrogramSamples - Deturmines how many times you wish for the spectrogram to calculate per second.
	* @param SpectrogramBands - Deturmines how many bands you wish for the spectrogram to calculate. High values decrease accuracy in time, but increase in frequancies. Value Will be floored to a multiple of 2.
	* @param BandsClamp - Allows you to Clamp the maximum bands value you wish for the spectrogram to generate.
	* @param LatancyAdjustSamples - This creates a secondary, low latancy, high time accuracy spectrogram to reference for time.
	* @param SpectrogramLogarithm - Allows you to set the logarithum of the generated mesh. inverted, so 0.5 will create a standard 2.0 mesh.
	* @param SpectrogramSharpness - Above 1.0 will make dark areas darker, and bright areas brighter. the inverse is true if set between 0 and 1.
	* @param SpectrogramBrightness - A global brightness modifier for the spectrogram.
	* @param UseVertexPainting - Allows you to completely toggle off the use of vertex colors. this way you can use a mesh instead if you wish.
	* @param ColorSettings - Allows you to tweak how the spectrogram is displayed to the user.
	*/
	//UFUNCTION(BlueprintCallable, Category = "Dans FFT Calculation Stuff")
	//static void MakeSpectrogramMesh(UImportedSoundWave* ImportedSoundWave, UAudioAnalysisToolsLibrary* AudioAnalysisObject, const int32 ChunkIndex, const int32 ThreadId, const int32 ThreadCount, const int32 SpectrogramSamples, const int32 SpectrogramBands, const int32 BandsClamp, const int32 SpectrogramTimeSamples, const float SpectrogramLogarithm, const float SpectrogramSharpness, const float SpectrogramBrightness, const bool UseVertexPainting, FSpectrogramColorSettings ColorSettings, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FLinearColor>& VertexColors, bool& endloop);

	/** Generates a 3D Spectrogram. Each chunk is 1 second in size
	* @param SpectrogramValues - Input nodes for the fft. Allows you to simplify your setup when it comes to making a thread pool. Best to leave these as constant values once the spectrogram is running.
	* @param ChunkIndex - What index you wish to calculate and generate. This variable should be thread specific.
	* @param ThreadId - Allows you to give the thread calculation offset so that chunks can be generated correctly. Set 0 for a single thread.
	* @
	*/
	UFUNCTION(BlueprintCallable, Category = "Dans FFT Calculation Stuff")
	static void MakeSpectrogramColorArray(FSpectrogramInput SpectrogramValues, const int32 ChunkIndex, const int32 ThreadId, TEnumAsByte<FGenerationStatus>& ContinueLooping, TArray<FColor>& color);

	/** Generates a 3D Spectrogram. Each chunk is 1 second in size
	* @param SpectrogramValues - Input nodes for the fft. Allows you to simplify your setup when it comes to making a thread pool. Best to leave these as constant values once the spectrogram is running.
	* @param ChunkIndex - What index you wish to calculate and generate. This variable should be thread specific.
	* @param ThreadId - Allows you to give the thread calculation offset so that chunks can be generated correctly. Set 0 for a single thread.
	* @
	*/
	UFUNCTION(BlueprintCallable, Category = "Dans FFT Calculation Stuff")
	static void MakeWaveformColorArray(FWaveformInput WaveformValues, const int32 ChunkIndex, const int32 ThreadId, TEnumAsByte<FGenerationStatus>& ContinueLooping, TArray<FColor>& color);

	/** this is intended to be created many times for a Spectrogram thread pool. FFT calculation is very slow, especially with high sample counts.
	* make sure that you leave at minimum 1 thread (for the game thread), in order for things to work smoothly in the background. 2 is much more ideal, as it leaves one for Unreal Engine async stuff.
	* @param type - What texture you wish to generate.
	* @param WaveformInput - Input nodes for the waveform. Allows you to simplify your setup when it comes to making a thread pool. Best to leave these as constant values once the spectrogram is running.
	* @param SpectrogramInput - Input nodes for the fft. Allows you to simplify your setup when it comes to making a thread pool. Best to leave these as constant values once the spectrogram is running.
	* @param ChunkIndex - Plug 'Output Chunk Index' that you get from the delegate back in here if you intend to loop across the entire song
	* @param ThreadId - Allows you to give the thread calculation offset so that chunks can be generated correctly. Set 0 for a single thread.
	* @
	*/
	UFUNCTION(BlueprintCallable, Category = "Dans FFT Calculation Stuff")
	static void CalculateSpectrogramAsync(UMyBlueprintFunctionLibrary* MyLibRef, FGenerationType type, FWaveformInput WaveformInput, FSpectrogramInput SpectrogramInput, int32 ChunkIndex, int32 ThreadID);

protected:

	//spectrogram done all its calculations
	void DoneCalculating_Internal(FSpectrogramOutput output, UMyBlueprintFunctionLibrary* Ref);
};



