// Georgy Treshchev 2021.

#include "AudioAnalysisToolsLibrary.h"

UAudioAnalysisToolsLibrary::UAudioAnalysisToolsLibrary()
{
	AudioImportingInfo = FAudioImportingStruct();
	EnvelopeAnalysisInfo = FEnvelopeAnalysisStruct();
	OnsetDetectionInfo = FOnsetDetectionStruct();

	AudioImporterObject = nullptr;
	OnsetDetectionObject = nullptr;
	EnvelopeAnalysisObject = nullptr;
	ImportedSoundWave = nullptr;

	CurrentMainAction = NoMainAction;
}

UAudioAnalysisToolsLibrary* UAudioAnalysisToolsLibrary::CreateAudioAnalysisTools()
{
	UAudioAnalysisToolsLibrary* AudioAnalysisTools = NewObject<UAudioAnalysisToolsLibrary>();
	AudioAnalysisTools->AddToRoot();
	return AudioAnalysisTools;
}

void UAudioAnalysisToolsLibrary::ImportAudioAndAnalyseEnvelope(FAudioImportingStruct InAudioImportingInfo,
                                                               FEnvelopeAnalysisStruct InEnvelopeAnalysisInfo)
{
	CurrentMainAction = ImportAudioAndAnalyseEnvelopeAction;

	AudioImportingInfo = InAudioImportingInfo;
	EnvelopeAnalysisInfo = InEnvelopeAnalysisInfo;

	InitializeAndImportAudio();
}

void UAudioAnalysisToolsLibrary::ImportAudioAndDetectOnset(FAudioImportingStruct InAudioImportingInfo,
                                                           FEnvelopeAnalysisStruct InEnvelopeAnalysisInfo,
                                                           FOnsetDetectionStruct InOnsetDetectionInfo)
{
	CurrentMainAction = ImportAudioAndDetectOnsetActon;

	AudioImportingInfo = InAudioImportingInfo;
	EnvelopeAnalysisInfo = InEnvelopeAnalysisInfo;
	OnsetDetectionInfo = InOnsetDetectionInfo;

	InitializeAndImportAudio();
}

void UAudioAnalysisToolsLibrary::AudioImportingProgress(int32 Percentage)
{
	if (OnActionProgress.IsBound())
	{
		if (CurrentMainAction == ImportAudioAndAnalyseEnvelopeAction)
		{
			Percentage -= static_cast<int32>((static_cast<float>(Percentage) * 20) / 100.0f);
		}
		else if (CurrentMainAction == ImportAudioAndDetectOnsetActon)
		{
			Percentage -= static_cast<int32>((static_cast<float>(Percentage) * 35) / 100.0f);
		}
		OnActionProgress.Broadcast(Percentage);
	}
}

void UAudioAnalysisToolsLibrary::AudioImportingFinished(URuntimeAudioImporterLibrary* RuntimeAudioImporterObjectRef,
                                                        UImportedSoundWave* SoundWaveRef,
                                                        const TEnumAsByte<ETranscodingStatus>& Status)
{
	if (Status == SuccessfulImport)
	{
		ImportedSoundWave = SoundWaveRef;

		if (OnAudioImportingFinished.IsBound()) OnAudioImportingFinished.Broadcast(SoundWaveRef);

		if (CurrentMainAction == ImportAudioAndAnalyseEnvelopeAction || CurrentMainAction ==
			ImportAudioAndDetectOnsetActon)
			InitializeAndAnalyzeEnvelope();
		else UnitializeAudioImporter();
	}
	else
	{
		if (OnActionError.IsBound())
			OnActionError.Broadcast(FProcessErrorInfo(CurrentMainAction, ImportAudio), Status,
			                        SuccessfulAnalysis,
			                        SuccessfulDetection);
		if (CurrentMainAction != ImportAudioAndAnalyseEnvelopeAction && CurrentMainAction !=
			ImportAudioAndDetectOnsetActon)
			UnitializeAudioImporter();
	}
}

void UAudioAnalysisToolsLibrary::EnvelopeAnalysisFinished(const FAnalysedEnvelopeData& AnalysedEnvelopeData,
                                                          const TEnumAsByte<EEnvelopeAnalysisStatus>& Status)
{
	if (Status == SuccessfulAnalysis)
	{
		if (OnActionProgress.IsBound())
		{
			float Percentage = 0;

			if (CurrentMainAction == ImportAudioAndAnalyseEnvelopeAction) Percentage = 100;
			else if (CurrentMainAction == ImportAudioAndDetectOnsetActon) Percentage = 85;

			OnActionProgress.Broadcast(Percentage);
		}

		if (CurrentMainAction == ImportAudioAndAnalyseEnvelopeAction || CurrentMainAction ==
			ImportAudioAndDetectOnsetActon)
			UnitializeAudioImporter();


		if (OnEnvelopeAnalysisFinished.IsBound()) OnEnvelopeAnalysisFinished.Broadcast(AnalysedEnvelopeData);


		if (CurrentMainAction == ImportAudioAndDetectOnsetActon) InitializeAndDetectOnset(AnalysedEnvelopeData);
		else UnitializeEnvelopeAnalysis();
	}
	else
	{
		if (OnActionError.IsBound())
			OnActionError.Broadcast(FProcessErrorInfo(CurrentMainAction, AnalyseEnvelope), SuccessfulImport, Status,
			                        SuccessfulDetection);

		if (CurrentMainAction == ImportAudioAndDetectOnsetActon || CurrentMainAction ==
			ImportAudioAndAnalyseEnvelopeAction)
			ImportedSoundWave->ReleaseMemory();

		if (CurrentMainAction != ImportAudioAndDetectOnsetActon) UnitializeAudioImporter();
	}
}

void UAudioAnalysisToolsLibrary::OnsetDetectionFinished(const TArray<float>& DetectedOnsetArray,
                                                        const TEnumAsByte<EOnsetDetectionStatus>& Status)
{
	if (Status == SuccessfulDetection)
	{
		if (CurrentMainAction == ImportAudioAndDetectOnsetActon)
		{
			if (OnActionProgress.IsBound()) OnActionProgress.Broadcast(100);

			UnitializeEnvelopeAnalysis();
		}


		if (OnOnsetDetectionFinished.IsBound()) OnOnsetDetectionFinished.Broadcast(DetectedOnsetArray);

		UnitializeOnsetDetection();
	}
	else
	{
		if (CurrentMainAction == ImportAudioAndDetectOnsetActon)
		{
			if (ImportedSoundWave) ImportedSoundWave->ReleaseMemory();
		}
		if (OnActionError.IsBound())
			OnActionError.Broadcast(FProcessErrorInfo(CurrentMainAction, DetectOnset), SuccessfulImport,
			                        SuccessfulAnalysis, Status);

		UnitializeOnsetDetection();
	}
}

void UAudioAnalysisToolsLibrary::UnitializeAudioImporter()
{
	AudioImporterObject->OnProgress.RemoveAll(this);
	AudioImporterObject->OnResult.RemoveAll(this);
	AudioImporterObject = nullptr;
}

void UAudioAnalysisToolsLibrary::UnitializeEnvelopeAnalysis()
{
	EnvelopeAnalysisObject->OnAnalysisFinished.RemoveAll(this);
	EnvelopeAnalysisObject->Reset();
	EnvelopeAnalysisObject = nullptr;
}

void UAudioAnalysisToolsLibrary::UnitializeOnsetDetection()
{
	OnsetDetectionObject->OnDetectionFinished.RemoveAll(this);
	OnsetDetectionObject = nullptr;
}

void UAudioAnalysisToolsLibrary::InitializeAndImportAudio()
{
	URuntimeAudioImporterLibrary* AudioImporter = URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter();

	AudioImporterObject = AudioImporter;

	AudioImporterObject->OnProgress.AddDynamic(this, &UAudioAnalysisToolsLibrary::AudioImportingProgress);
	AudioImporterObject->OnResult.AddDynamic(this, &UAudioAnalysisToolsLibrary::AudioImportingFinished);

	if (AudioImportingInfo.ImportAudioFromPreImportedAsset)
		AudioImporter->ImportAudioFromPreImportedSound(
			AudioImportingInfo.PreImportedSoundAssetRef);
	else AudioImporter->ImportAudioFromFile(AudioImportingInfo.FilePath, AudioImportingInfo.AudioFormat);
}

void UAudioAnalysisToolsLibrary::InitializeAndAnalyzeEnvelope()
{
	UEnvelopeAnalysis* EnvelopeAnalysis = UEnvelopeAnalysis::CreateEnvelopeAnalysis();
	EnvelopeAnalysisObject = EnvelopeAnalysis;

	EnvelopeAnalysis->OnAnalysisFinished.AddDynamic(this, &UAudioAnalysisToolsLibrary::EnvelopeAnalysisFinished);

	EnvelopeAnalysisInfo.NumberOfChannels = ImportedSoundWave->NumChannels;
	EnvelopeAnalysisInfo.SampleRate = ImportedSoundWave->SamplingRate;

	EnvelopeAnalysis->AnalyzeEnvelopeFromImportedSoundWave(ImportedSoundWave, EnvelopeAnalysisInfo);
}

void UAudioAnalysisToolsLibrary::InitializeAndDetectOnset(const FAnalysedEnvelopeData& AnalysedEnvelopeData)
{
	UOnsetDetection* OnsetDetection = UOnsetDetection::CreateOnsetDetection();
	OnsetDetectionObject = OnsetDetection;
	OnsetDetection->OnDetectionFinished.AddDynamic(
		this, &UAudioAnalysisToolsLibrary::OnsetDetectionFinished);

	OnsetDetectionInfo.AnalysedEnvelopeData = AnalysedEnvelopeData;

	OnsetDetection->DetectOnsetFromEnvelope(OnsetDetectionInfo);
}
