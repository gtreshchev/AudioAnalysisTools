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

	CurrentMainAction = EMainAction::NoMainAction;
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
	CurrentMainAction = EMainAction::ImportAudioAndAnalyseEnvelopeAction;

	AudioImportingInfo = InAudioImportingInfo;
	EnvelopeAnalysisInfo = InEnvelopeAnalysisInfo;

	InitializeAndImportAudio();
}

void UAudioAnalysisToolsLibrary::ImportAudioAndDetectOnset(FAudioImportingStruct InAudioImportingInfo,
                                                           FEnvelopeAnalysisStruct InEnvelopeAnalysisInfo,
                                                           FOnsetDetectionStruct InOnsetDetectionInfo)
{
	CurrentMainAction = EMainAction::ImportAudioAndDetectOnsetActon;

	AudioImportingInfo = InAudioImportingInfo;
	EnvelopeAnalysisInfo = InEnvelopeAnalysisInfo;
	OnsetDetectionInfo = InOnsetDetectionInfo;

	InitializeAndImportAudio();
}

void UAudioAnalysisToolsLibrary::AudioImportingProgress(int32 Percentage)
{
	if (OnActionProgress.IsBound())
	{
		if (CurrentMainAction == EMainAction::ImportAudioAndAnalyseEnvelopeAction)
		{
			Percentage -= static_cast<int32>((static_cast<float>(Percentage) * 20) / 100.0f);
		}
		else if (CurrentMainAction == EMainAction::ImportAudioAndDetectOnsetActon)
		{
			Percentage -= static_cast<int32>((static_cast<float>(Percentage) * 35) / 100.0f);
		}
		OnActionProgress.Broadcast(Percentage);
	}
}

void UAudioAnalysisToolsLibrary::AudioImportingFinished(URuntimeAudioImporterLibrary* RuntimeAudioImporterObjectRef,
                                                        UImportedSoundWave* SoundWaveRef,
                                                        const ETranscodingStatus& Status)
{
	if (Status == ETranscodingStatus::SuccessfulImport)
	{
		ImportedSoundWave = SoundWaveRef;

		if (OnAudioImportingFinished.IsBound()) OnAudioImportingFinished.Broadcast(SoundWaveRef);

		if (CurrentMainAction == EMainAction::ImportAudioAndAnalyseEnvelopeAction || CurrentMainAction ==
			EMainAction::ImportAudioAndDetectOnsetActon)
			InitializeAndAnalyzeEnvelope();
		else UnitializeAudioImporter();
	}
	else
	{
		if (OnActionError.IsBound())
			OnActionError.Broadcast(FProcessErrorInfo(CurrentMainAction, EDetailedAction::ImportAudio), Status,
			                        EEnvelopeAnalysisStatus::SuccessfulAnalysis,
			                        EOnsetDetectionStatus::SuccessfulDetection);
		if (CurrentMainAction != EMainAction::ImportAudioAndAnalyseEnvelopeAction && CurrentMainAction !=
			EMainAction::ImportAudioAndDetectOnsetActon)
			UnitializeAudioImporter();
	}
}

void UAudioAnalysisToolsLibrary::EnvelopeAnalysisFinished(const FAnalysedEnvelopeData& AnalysedEnvelopeData,
                                                          const EEnvelopeAnalysisStatus& Status)
{
	if (Status == EEnvelopeAnalysisStatus::SuccessfulAnalysis)
	{
		if (OnActionProgress.IsBound())
		{
			float Percentage = 0;

			if (CurrentMainAction == EMainAction::ImportAudioAndAnalyseEnvelopeAction) Percentage = 100;
			else if (CurrentMainAction == EMainAction::ImportAudioAndDetectOnsetActon) Percentage = 85;

			OnActionProgress.Broadcast(Percentage);
		}

		if (CurrentMainAction == EMainAction::ImportAudioAndAnalyseEnvelopeAction || CurrentMainAction ==
			EMainAction::ImportAudioAndDetectOnsetActon)
			UnitializeAudioImporter();


		if (OnEnvelopeAnalysisFinished.IsBound()) OnEnvelopeAnalysisFinished.Broadcast(AnalysedEnvelopeData);


		if (CurrentMainAction == EMainAction::ImportAudioAndDetectOnsetActon) InitializeAndDetectOnset(AnalysedEnvelopeData);
		else UnitializeEnvelopeAnalysis();
	}
	else
	{
		if (OnActionError.IsBound())
			OnActionError.Broadcast(FProcessErrorInfo(CurrentMainAction, EDetailedAction::AnalyseEnvelope), ETranscodingStatus::SuccessfulImport, Status,
			                        EOnsetDetectionStatus::SuccessfulDetection);

		if (CurrentMainAction == EMainAction::ImportAudioAndDetectOnsetActon || CurrentMainAction ==
			EMainAction::ImportAudioAndAnalyseEnvelopeAction)
			ImportedSoundWave->ReleaseMemory();

		if (CurrentMainAction != EMainAction::ImportAudioAndDetectOnsetActon) UnitializeAudioImporter();
	}
}

void UAudioAnalysisToolsLibrary::OnsetDetectionFinished(const TArray<float>& DetectedOnsetArray,
                                                        const EOnsetDetectionStatus& Status)
{
	if (Status == EOnsetDetectionStatus::SuccessfulDetection)
	{
		if (CurrentMainAction == EMainAction::ImportAudioAndDetectOnsetActon)
		{
			if (OnActionProgress.IsBound()) OnActionProgress.Broadcast(100);

			UnitializeEnvelopeAnalysis();
		}


		if (OnOnsetDetectionFinished.IsBound()) OnOnsetDetectionFinished.Broadcast(DetectedOnsetArray);

		UnitializeOnsetDetection();
	}
	else
	{
		if (CurrentMainAction == EMainAction::ImportAudioAndDetectOnsetActon)
		{
			if (ImportedSoundWave) ImportedSoundWave->ReleaseMemory();
		}
		if (OnActionError.IsBound())
			OnActionError.Broadcast(FProcessErrorInfo(CurrentMainAction, EDetailedAction::DetectOnset), ETranscodingStatus::SuccessfulImport,
			                        EEnvelopeAnalysisStatus::SuccessfulAnalysis, Status);

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
	UEnvelopeAnalysisLibrary* EnvelopeAnalysis = UEnvelopeAnalysisLibrary::CreateEnvelopeAnalysis();
	EnvelopeAnalysisObject = EnvelopeAnalysis;

	EnvelopeAnalysis->OnAnalysisFinished.AddDynamic(this, &UAudioAnalysisToolsLibrary::EnvelopeAnalysisFinished);

	EnvelopeAnalysisInfo.NumberOfChannels = ImportedSoundWave->NumChannels;
	EnvelopeAnalysisInfo.SampleRate = ImportedSoundWave->SamplingRate;

	EnvelopeAnalysis->AnalyzeEnvelopeFromImportedSoundWave(ImportedSoundWave, EnvelopeAnalysisInfo);
}

void UAudioAnalysisToolsLibrary::InitializeAndDetectOnset(const FAnalysedEnvelopeData& AnalysedEnvelopeData)
{
	UOnsetDetectionLibrary* OnsetDetection = UOnsetDetectionLibrary::CreateOnsetDetection();
	OnsetDetectionObject = OnsetDetection;
	OnsetDetection->OnDetectionFinished.AddDynamic(
		this, &UAudioAnalysisToolsLibrary::OnsetDetectionFinished);

	OnsetDetectionInfo.AnalysedEnvelopeData = AnalysedEnvelopeData;

	OnsetDetection->DetectOnsetFromEnvelope(OnsetDetectionInfo);
}
