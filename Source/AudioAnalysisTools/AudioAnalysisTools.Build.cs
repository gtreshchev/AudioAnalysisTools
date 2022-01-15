// Georgy Treshchev 2022.

using UnrealBuildTool;

public class AudioAnalysisTools : ModuleRules
{
	public AudioAnalysisTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RuntimeAudioImporter"
			}
		);
	}
}