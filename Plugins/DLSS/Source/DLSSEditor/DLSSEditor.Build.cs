

using UnrealBuildTool;
using System.IO;

public class DLSSEditor : ModuleRules
{
	public DLSSEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Settings",

				"NGX",
				"DLSS",
				"NGXRHI",
			}
			);
	}
}
