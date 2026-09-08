

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class VaRest : ModuleRules
	{
		public VaRest(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			PrecompileForTargets = PrecompileTargetsType.Any;
			DefaultBuildSettings = BuildSettingsVersion.V5;

			PrivateIncludePaths.AddRange(
				new string[] {
					"VaRest/Private",

				});

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"HTTP",
					"Json",
					"Projects" 

				});
		}
	}
}
