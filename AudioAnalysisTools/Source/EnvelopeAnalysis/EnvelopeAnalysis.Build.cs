// Georgy Treshchev 2021.

using UnrealBuildTool;

public class EnvelopeAnalysis : ModuleRules
{
	public EnvelopeAnalysis(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"RuntimeAudioImporter"
			}
		);
	}
}