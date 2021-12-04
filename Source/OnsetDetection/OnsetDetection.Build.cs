// Georgy Treshchev 2021.

using UnrealBuildTool;

public class OnsetDetection : ModuleRules
{
	public OnsetDetection(ReadOnlyTargetRules Target) : base(Target)
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
				"EnvelopeAnalysis"
			}
		);
	}
}