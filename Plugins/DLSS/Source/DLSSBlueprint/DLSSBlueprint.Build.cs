

using UnrealBuildTool;
using System.IO;

public class DLSSBlueprint : ModuleRules
{
	protected virtual bool IsSupportedPlatform(ReadOnlyTargetRules Target)
	{
		return Target.Platform.IsInGroup(UnrealPlatformGroup.Windows);
	}

	public DLSSBlueprint(ReadOnlyTargetRules Target) : base(Target)
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

		bool bPlatformSupportsDLSS = IsSupportedPlatform(Target);

		PublicDefinitions.Add("WITH_DLSS=" + (bPlatformSupportsDLSS ? '1' : '0'));
#if UE_5_6_OR_LATER
		PrivateDefinitions.Add("ENGINE_SUPPORTS_UPSCALER_MODULAR_FEATURE=1");
		PrivateDependencyModuleNames.Add("VirtualProduction");
#else
		PrivateDefinitions.Add("ENGINE_SUPPORTS_UPSCALER_MODULAR_FEATURE=0");
#endif

		if (bPlatformSupportsDLSS)
		{ 
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"NGX",
					"NGXRHI",
					"DLSS",
				}
			);
		}
	}

}
