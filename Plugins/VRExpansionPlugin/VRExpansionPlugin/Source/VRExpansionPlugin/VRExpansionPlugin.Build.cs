
using System.IO;
using UnrealBuildTool;

public class VRExpansionPlugin : ModuleRules
{
    private string PluginsPath
    {
        get { return Path.GetFullPath(Target.RelativeEnginePath) + "Plugins/Runtime/"; }
    }

    public VRExpansionPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDefinitions.Add("WITH_VR_EXPANSION=1");
        SetupIrisSupport(Target);

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[] {
                    "UnrealEd"
                }
            );
        }

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                        "Settings"
            }
        );

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
                    "Core",
                    "NetCore",
                    "CoreUObject",
                    "Engine",
                    "PhysicsCore",
                    "HeadMountedDisplay",
                    "UMG",
                    "NavigationSystem",
                    "AIModule",
                    "AnimGraphRuntime",
                    "XRBase",
                    "GameplayTags",
                    "Mover"
        });

            PublicDependencyModuleNames.Add("Chaos");
            PublicDependencyModuleNames.Add("ChaosVehicles");

        PrivateDependencyModuleNames.AddRange(
            new string[] 
            {

                "InputCore",
                "ImageCore", 

                "RHI",
				"ApplicationCore",
                "RenderCore",

                "NetworkReplayStreaming"

            });

        PrivateDependencyModuleNames.AddRange(
			new string[]
			{

				"Slate",
				"SlateCore"

			}
			);

        if (Target.bBuildDeveloperTools || (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Configuration != UnrealTargetConfiguration.Test))
        {
            PrivateDependencyModuleNames.Add("GameplayDebugger");
            PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=1"); 
        }
        else
        {
            PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=0");
        }
    }
}
