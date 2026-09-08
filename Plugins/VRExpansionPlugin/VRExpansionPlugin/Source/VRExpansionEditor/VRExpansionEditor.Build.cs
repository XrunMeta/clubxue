

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class VRExpansionEditor : ModuleRules
	{

		public VRExpansionEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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

					"Engine",
					"Core",
					"CoreUObject",
					"VRExpansionPlugin",
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
                    "BlueprintGraph",
                    "AnimGraph",
                    "AnimGraphRuntime",
                    "SlateCore",
                    "Slate",
                    "InputCore",
                    "Engine",
                    "UnrealEd",
                    "EditorStyle",
					"AssetRegistry"
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{

				}
				);
		}
	}
}