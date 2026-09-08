

using UnrealBuildTool;
using System.IO;

public class NISShaders: ModuleRules
{
	public NISShaders(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(EngineDirectory,"Source/Runtime/Renderer/Private"),
#if UE_5_6_OR_LATER
				Path.Combine(EngineDirectory,"Source/Runtime/Renderer/Internal"),
#endif
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"RenderCore",
				"Renderer",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
					"Engine",
					"RHI",
					"Projects",
					"Renderer",
			}
			);
	}
}
