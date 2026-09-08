

using UnrealBuildTool;

public class NGXVulkanRHI : ModuleRules
{
	public NGXVulkanRHI(ReadOnlyTargetRules Target) : base(Target)
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
					"NGXRHI",
			}
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");

	}
}
