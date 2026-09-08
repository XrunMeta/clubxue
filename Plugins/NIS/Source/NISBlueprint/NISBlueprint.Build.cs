

using UnrealBuildTool;
using System.IO;

public class NISBlueprint : ModuleRules
{

	public NISBlueprint(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RenderCore",
				"Renderer",
				"Projects",
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{

				"NISShaders",
				"RHI",
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				EngineDirectory + "/Source/Runtime/Renderer/Private",
#if UE_5_6_OR_LATER
				EngineDirectory + "/Source/Runtime/Renderer/Internal",
#endif

			}
			);

	}

}

