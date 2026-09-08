

using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class OpenXRExpansionPlugin: ModuleRules
    {
        public OpenXRExpansionPlugin(ReadOnlyTargetRules Target) 
				: base(Target)
        {
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            DefaultBuildSettings = BuildSettingsVersion.Latest;
            IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

            SetupIrisSupport(Target);

            PublicDependencyModuleNames.AddRange(
			   new string[]
			   {

			   }
		   );

            var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
            PrivateIncludePaths.AddRange(
                new string[] {
                    EngineDir + "Plugins/Runtime/OpenXR/Source/OpenXRHMD/Private",
                    EngineDir + "/Source/ThirdParty/OpenXR/include",

				}
                );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "NetCore",
                    "CoreUObject",

                    "Engine",

                    "InputCore",
					"Slate",
					"HeadMountedDisplay",

                    "AnimGraphRuntime",
                    "SlateCore",
                    "XRBase"

                }
				);

            if (Target.Platform != UnrealTargetPlatform.Mac && Target.Platform != UnrealTargetPlatform.IOS)
            {
                PrivateDependencyModuleNames.AddRange(
                    new string[]
                    {
                        "OpenXRHMD"
                    }
                );
                PrivateDefinitions.AddRange(new string[] { "OPENXR_SUPPORTED" });
                AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenXR");
            }

        }
    }
}
