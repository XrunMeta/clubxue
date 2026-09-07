

using UnrealBuildTool;

public class ClubX : ModuleRules
{
	public ClubX(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "VRExpansionPlugin" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

	}
}
