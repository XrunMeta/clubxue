

using UnrealBuildTool;
using System.IO;
public class ArmASR : ModuleRules
{
	public ArmASR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {

			}
		);

        PrivateIncludePaths.AddRange(
        new string[] {

                Path.Combine(EngineDirectory,"Source/Runtime/Renderer/Private"),
                Path.Combine(EngineDirectory,"Source/Runtime/Renderer/Internal"),
                Path.Combine(PluginDirectory,"Shaders/Private/fsr2"),
			}
		);

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"DeveloperSettings"

			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Renderer",
				"RenderCore",
				"Projects",
				"RHI",
				"VulkanRHI"

			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			}
			);

		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows)
			|| Target.IsInPlatformGroup(UnrealPlatformGroup.Android))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");
		}
	}
}
