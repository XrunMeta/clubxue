

namespace UnrealBuildTool.Rules
{
	public class MinimapPlugin : ModuleRules
	{

        public MinimapPlugin(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            IWYUSupport = IWYUSupport.Full;

            PublicIncludePaths.AddRange(
	            new string[]
	            {
		            ModuleDirectory
	            }
            );

            PublicDependencyModuleNames.AddRange(
				new string[]
				{
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "InputCore",
                    "SlateCore",
                    "Slate",
                    "UMG",
                    "NavigationSystem"
                }
			);
		}
	}
}
