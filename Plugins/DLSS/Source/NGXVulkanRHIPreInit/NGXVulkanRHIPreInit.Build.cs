

using UnrealBuildTool;

public class NGXVulkanRHIPreInit : ModuleRules
{
	public NGXVulkanRHIPreInit(ReadOnlyTargetRules Target) : base(Target)
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

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
					"Core",
					"Engine",
					"RenderCore",
					"RHI",

					"VulkanRHI",
					"NGX",
			}
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");
	}
}
