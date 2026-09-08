

using UnrealBuildTool;

public class UBIKRuntime : ModuleRules
{
	public UBIKRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {

			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {

			}
			);

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AnimGraphRuntime", "AnimationCore" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "Projects", "XRBase", "HeadMountedDisplay" });

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			}
			);
	}
}
