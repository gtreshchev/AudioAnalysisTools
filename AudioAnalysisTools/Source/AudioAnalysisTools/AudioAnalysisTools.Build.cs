// Georgy Treshchev 2021.

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
				"RuntimeAudioImporter"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine"
			}
		);
	}
}