using UnrealBuildTool;
using System.IO;

public class AdvancedSteamSessions : ModuleRules
{
    public AdvancedSteamSessions(ReadOnlyTargetRules Target) : base(Target)
    {
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDefinitions.Add("WITH_ADVANCED_STEAM_SESSIONS=1");

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "CoreUObject", "OnlineSubsystemUtils", "Networking", "Sockets", "AdvancedSessions" });
        PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem", "Sockets", "Networking", "OnlineSubsystemUtils" });

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux) || (Target.Platform == UnrealTargetPlatform.Mac))
        {
            PublicDependencyModuleNames.AddRange(new string[] { "SteamShared", "Steamworks", "OnlineSubsystemSteam" });
            AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");

        }
    }
}