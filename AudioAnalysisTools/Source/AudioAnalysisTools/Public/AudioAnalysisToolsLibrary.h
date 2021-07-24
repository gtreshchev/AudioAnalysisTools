// Georgy Treshchev 2021.

#pragma once

#include "EnvelopeAnalysis.h"
#include "OnsetDetection.h"
#include "RuntimeAudioImporterLibrary.h"

#include "AudioAnalysisToolsLibrary.generated.h"

/** Possible main actions */
UENUM(BlueprintType, Category = "Audio Analysis Tools")
enum EMainAction
{
	NoMainAction UMETA(Hidden),
	ImportAudioAndAnalyseEnvelopeAction,
	ImportAudioAndDetectOnsetActon
};

/** Possible detailed actions */
UENUM(BlueprintType, Category = "Audio Analysis Tools")
enum EDetailedAction
{
	NoDetailedAction UMETA(Hidden),
	ImportAudio,
	AnalyseEnvelope,
	DetectOnset
};

/** Structure containing error information */
USTRUCT(BlueprintType, Category = "Audio Analysis Tools")
struct FProcessErrorInfo
{
	GENERATED_BODY()

	/** Main action error information */
	UPROPERTY(BlueprintReadWrite, Category = "Audio Analysis Tools")
	TEnumAsByte<EMainAction> MainActionError;

	/** Detailed action error information */
	UPROPERTY(BlueprintReadWrite, Category = "Audio Analysis Tools")
	TEnumAsByte<EDetailedAction> DetailedActionError;

	/** Base constructor */
	FProcessErrorInfo()
		: MainActionError(NoMainAction), DetailedActionError(NoDetailedAction)
	{
	}

	/** Main constructor */
	FProcessErrorInfo(TEnumAsByte<EMainAction> InMainActionError, TEnumAsByte<EDetailedAction> InDetailedActionError)
		: MainActionError(InMainActionError), DetailedActionError(InDetailedActionError)
	{
	}
};

/** Audio importing information */
USTRUCT(BlueprintType, Category = "Audio Analysis Tools")
struct FAudioImportingStruct
{
	GENERATED_BODY()

	/** Whether to import audio via a pre-imported asset or file path */
	UPROPERTY(BlueprintReadWrite, Category = "Audio Analysis Tools")
	bool ImportAudioFromPreImportedAsset;

	/**
	 * Pre-imported sound asset reference
	 * Fill in only if "ImportAudioFromPreImportedAsset" = true
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Audio Analysis Tools")
	UPreImportedSoundAsset* PreImportedSoundAssetRef;

	/**
	 * The path to the audio file
	 * Fill in only if "ImportAudioFromPreImportedAsset" = false
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Audio Analysis Tools")
	FString FilePath;

	/**
	 * Audio format
	 * Fill in only if "ImportAudioFromPreImportedAsset" = false
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Audio Analysis Tools")
	TEnumAsByte<EAudioFormat> AudioFormat;

	/** Base constructor */
	FAudioImportingStruct()
		: ImportAudioFromPreImportedAsset(false), PreImportedSoundAssetRef(nullptr), FilePath(TEXT("")),
		  AudioFormat(Auto)
	{
	}

	/** Constructor using FilePath */
	FAudioImportingStruct(FString InFilePath, const TEnumAsByte<EAudioFormat>& InAudioFormat)
		: ImportAudioFromPreImportedAsset(false), PreImportedSoundAssetRef(nullptr), FilePath(InFilePath),
		  AudioFormat(InAudioFormat)
	{
	}

	/** Constructor using Pre-imported sound asset */
	FAudioImportingStruct(UPreImportedSoundAsset* InPreImportedSoundAssetRef)
		: ImportAudioFromPreImportedAsset(true), PreImportedSoundAssetRef(InPreImportedSoundAssetRef),
		  FilePath(TEXT("")), AudioFormat(Auto)
	{
	}
};

/** Delegate broadcast to get the audio analysis progress */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionProgress, const int32, Percentage);

/** Delegate broadcast when audio importing completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioImportingFinished, UImportedSoundWave*, ReadySoundWave);

/** Delegate broadcast when envelope analysis completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnvelopeAnalysisFinished, const FAnalysedEnvelopeData&,
                                            AnalysedEnvelopeData);

/** Delegate broadcast when onset detection completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOnsetDetectionFinished, const TArray<float>&, DetectedOnsetArray);

/** Delegate broadcast when an error occurs */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnActionError, const FProcessErrorInfo, ErrorInfo,
                                              const TEnumAsByte<ETranscodingStatus>, AudioImportingStatus,
                                              const TEnumAsByte<EEnvelopeAnalysisStatus>, EnvelopeAnalysisStatus,
                                              const TEnumAsByte<EOnsetDetectionStatus>, OnsetDetectionStatus);

/**
 * Audio Analysis Tools object
 * Works in conjunction with the Runtime Audio Importer plugin.
 */
UCLASS(BlueprintType, Category = "Audio Analysis Tools")
class AUDIOANALYSISTOOLS_API UAudioAnalysisToolsLibrary : public UObject
{
	GENERATED_BODY()

	UAudioAnalysisToolsLibrary();

	// Delegates

public:
	/** Bind to know when analysis is on progress */
	UPROPERTY(BlueprintAssignable, Category = "Audio Analysis Tools")
	FOnActionProgress OnActionProgress;

	/** Bind to know when an error occurred */
	UPROPERTY(BlueprintAssignable, Category = "Audio Analysis Tools")
	FOnActionError OnActionError;

	/** Bind to know when the audio import has been completed (Indicates that the Runtime Audio Importer has completed its work) */
	UPROPERTY(BlueprintAssignable, Category = "Audio Analysis Tools")
	FOnAudioImportingFinished OnAudioImportingFinished;

	/** Bind to know when the envelope analysis has been completed (Indicates that the Envelope Analysis has completed its work) */
	UPROPERTY(BlueprintAssignable, Category = "Audio Analysis Tools")
	FOnEnvelopeAnalysisFinished OnEnvelopeAnalysisFinished;

	/** Bind to know when the onset (beat) detection has been completed (Indicates that the Onset Detection has completed its work) */
	UPROPERTY(BlueprintAssignable, Category = "Audio Analysis Tools")
	FOnOnsetDetectionFinished OnOnsetDetectionFinished;

	// Delegates


	// Temporary parameters

private:
	/** Temporary information about importing audio */
	FAudioImportingStruct AudioImportingInfo;

	/** Temporary information about envelope analysis */
	FEnvelopeAnalysisStruct EnvelopeAnalysisInfo;

	/** Temporary information about onset detection */
	FOnsetDetectionStruct OnsetDetectionInfo;

	// Temporary parameters


	// References

private:
	/** Runtime Audio Importer object reference */
	UPROPERTY()
	URuntimeAudioImporterLibrary* AudioImporterObject;

	/** Onset Detection object reference */
	UPROPERTY()
	UOnsetDetection* OnsetDetectionObject;

	/** Envelope Analysis object reference */
	UPROPERTY()
	UEnvelopeAnalysis* EnvelopeAnalysisObject;

	/** Imported sound wave object reference */
	UPROPERTY()
	UImportedSoundWave* ImportedSoundWave;

	// References


	// Main

public:
	/**
	 * Instantiates an AudioAnalysisTools object
	 *
	 * @return The AudioAnalysisTools object. Bind to it's delegates depending on what operation you will be performing
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	static UAudioAnalysisToolsLibrary* CreateAudioAnalysisTools();

	/**
	 * Import audio and then analyze the Envelope data
	 * @param InAudioImportingInfo Audio import information
	 * @param InEnvelopeAnalysisInfo Envelope analysis information
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	void ImportAudioAndAnalyseEnvelope(FAudioImportingStruct InAudioImportingInfo,
	                                   FEnvelopeAnalysisStruct InEnvelopeAnalysisInfo);

	/**
	 * Import audio, analyze the Envelope data, then detect Onset
	 *
	 * @param InAudioImportingInfo Audio import information
	 * @param InEnvelopeAnalysisInfo Envelope analysis information
	 * @param InOnsetDetectionInfo Onset detection information
	 * 
	 * @note You should not fill InOnsetDetectionInfo.EnvelopeData Array, as it will be parsed through Envelope Analysis automatically
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio Analysis Tools")
	void ImportAudioAndDetectOnset(FAudioImportingStruct InAudioImportingInfo,
	                               FEnvelopeAnalysisStruct InEnvelopeAnalysisInfo,
	                               FOnsetDetectionStruct InOnsetDetectionInfo);

	// Main


	// Callbacks

private:
	/**
	 * Audio Importing Progress callback
	 *
	 * @param Percentage Percentage of importing completed (0-100%)
	 */
	UFUNCTION()
	void AudioImportingProgress(int32 Percentage);

	/**
	 * Audio Importing Finished callback
	 *
	 * @param RuntimeAudioImporterObjectRef Runtime Audio Importer object reference
	 * @param SoundWaveRef Imported sound wave object reference
	 * @param Status TranscodingStatus Enum in case an error occurs
	 */
	UFUNCTION()
	void AudioImportingFinished(URuntimeAudioImporterLibrary* RuntimeAudioImporterObjectRef,
	                            UImportedSoundWave* SoundWaveRef,
	                            const TEnumAsByte<ETranscodingStatus>& Status);

	/**
	 * Envelope Analysis Finished callback
	 *
	 * @param AnalysedEnvelopeData
	 * @param Status EnvelopeAnalysis Enum in case an error occurs
	 */
	UFUNCTION()
	void EnvelopeAnalysisFinished(const FAnalysedEnvelopeData& AnalysedEnvelopeData,
	                              const TEnumAsByte<EEnvelopeAnalysisStatus>& Status);

	/**
	 * Onset Detection Finished callback
	 *
	 * @param DetectedOnsetArray Detected onset data
	 * @param Status OnsetDetection Enum in case an error occurs
	 */
	UFUNCTION()
	void OnsetDetectionFinished(const TArray<float>& DetectedOnsetArray,
	                            const TEnumAsByte<EOnsetDetectionStatus>& Status);

	// Callbacks


	// Internal initializations

protected:
	/** Initialize and import audio */
	void InitializeAndImportAudio();

	/** Initialize and analyze envelope */
	void InitializeAndAnalyzeEnvelope();

	/** Initialize and detect onset */
	void InitializeAndDetectOnset(const FAnalysedEnvelopeData& AnalysedEnvelopeData);

	// Internal initializations


	// Internal uninitializations

protected:
	/** Remove audio importer */
	void UnitializeAudioImporter();

	/** Remove envelope analysis */
	void UnitializeEnvelopeAnalysis();

	/** Remove onset detection */
	void UnitializeOnsetDetection();

	// Internal uninitializations


	// Miscellaneous

protected:
	TEnumAsByte<EMainAction> CurrentMainAction;

	// Miscellaneous
};
