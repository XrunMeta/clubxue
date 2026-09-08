

using UnrealBuildTool;
using System.IO;

public class StreamlineRHI : ModuleRules
{
	public StreamlineRHI(ReadOnlyTargetRules Target) : base(Target)
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
				"RenderCore",	

				"Streamline",	
				"StreamlineNGXCommon",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Projects",
				"RHI",

				"StreamlineExtension",
			}
			);
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				"StreamlineD3D11RHI",
				"StreamlineD3D12RHI",
			}
			);

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

	}
}
