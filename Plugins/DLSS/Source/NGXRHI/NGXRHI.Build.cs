

using UnrealBuildTool;
using System.IO;
public class NGXRHI : ModuleRules
{
	public NGXRHI(ReadOnlyTargetRules Target) : base(Target)
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
				"StreamlineNGXCommon"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{

					"Core",
					"Engine",
					"Projects",
					"RenderCore",
					"RHI",

					"NGX",
			}
			);
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
