

using System;
using System.IO;
using UnrealBuildTool;

public class SteamIntegrationKit : ModuleRules
{
	public SteamIntegrationKit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		bool bUseEngineSteam = true;

		PublicIncludePaths.AddRange(
			new string[] {

			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {

			System.IO.Path.Combine(
                EngineDirectory,
				"Source/ThirdParty/Steamworks/Steamv164/sdk/public/steam"),
            }
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreOnline",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"Networking"

			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"DeveloperSettings",
				"Projects",
				"HTTP",
				"Json",
				"JsonUtilities",
				"OnlineSubsystemSteam",
				"AudioExtensions",
				"AudioMixer"

			}
			);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd",
					"ToolMenus",
					"EditorFramework"
				}
			);
		}

		if (bUseEngineSteam)
		{
			PublicDefinitions.Add("WITH_ENGINE_STEAM=1");
			PublicDefinitions.Add("WITH_STEAMKIT=1");
			if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Mac)
			{
				Console.WriteLine("SteamIntegrationKit: Game build, enabling ONLINESUBSYSTEMSTEAM_PACKAGE");
				PublicDefinitions.Add("ONLINESUBSYSTEMSTEAM_PACKAGE=1");
				AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
			}
			else
			{
				Console.WriteLine("SteamIntegrationKit: Editor build, disabling ONLINESUBSYSTEMSTEAM_PACKAGE");
				PublicDefinitions.Add("ONLINESUBSYSTEMSTEAM_PACKAGE=0");
			}
		}
		else
		{
			PublicDefinitions.Add("ONLINESUBSYSTEMSTEAM_PACKAGE=1");
			PublicDefinitions.Add("WITH_ENGINE_STEAM=0");
			PublicDependencyModuleNames.Add("SteamSdk");
		}
	}
}
