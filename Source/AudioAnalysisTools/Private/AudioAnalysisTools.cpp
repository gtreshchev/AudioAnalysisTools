// Georgy Treshchev 2024.

#include "AudioAnalysisTools.h"

#include "AudioAnalysisToolsDefines.h"

#define LOCTEXT_NAMESPACE "FAudioAnalysisToolsModule"

void FAudioAnalysisToolsModule::StartupModule()
{
}

void FAudioAnalysisToolsModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAudioAnalysisToolsModule, AudioAnalysisTools)

DEFINE_LOG_CATEGORY(LogAudioAnalysis);
