

using UnrealBuildTool;
using System.IO;

public class StreamlineDXGIRHI : ModuleRules
{
	public StreamlineDXGIRHI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"RHI",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"RenderCore",
				"Streamline",
				"StreamlineRHI",
			}
		);
	}
}
