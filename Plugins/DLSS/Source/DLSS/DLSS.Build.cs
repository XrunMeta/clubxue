

using UnrealBuildTool;
using System.IO;

public class DLSS : ModuleRules
{
	public virtual string [] SupportedDynamicallyLoadedNGXRHIModules(ReadOnlyTargetRules Target)
	{
		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			return new string[]
			{
				"NGXD3D11RHI",
				"NGXD3D12RHI",
				"NGXVulkanRHI"
			};
		}
		return new string[] { "" };
	}

	public DLSS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
#if !UE_5_3_OR_LATER

		bUseRTTI = true;
#endif

		PublicIncludePaths.AddRange(
			new string[] {

			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(GetModuleDirectory("Renderer"), "Private"),
#if UE_5_6_OR_LATER
				Path.Combine(GetModuleDirectory("Renderer"), "Internal"),
#endif

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
					"CoreUObject",
					"EngineSettings",
					"Engine",
					"RenderCore",
					"Renderer",
					"RHI",
					"NGX",
					"Projects",
                    "DeveloperSettings",
					"DLSSUtility",
					"NGXRHI",

			}
			);

#if UE_5_6_OR_LATER
		PrivateDefinitions.Add("ENGINE_SUPPORTS_UPSCALER_MODULAR_FEATURE=1");
		PublicDependencyModuleNames.Add("VirtualProduction");
#else
		PrivateDefinitions.Add("ENGINE_SUPPORTS_UPSCALER_MODULAR_FEATURE=0");
#endif

		DynamicallyLoadedModuleNames.AddRange(SupportedDynamicallyLoadedNGXRHIModules(Target));
	}
}
