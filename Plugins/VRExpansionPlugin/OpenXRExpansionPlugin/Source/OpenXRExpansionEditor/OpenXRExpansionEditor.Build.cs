

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class OpenXRExpansionEditor : ModuleRules
	{

		public OpenXRExpansionEditor(ReadOnlyTargetRules Target) : base(Target)
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
					"CoreUObject"
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
                    "EditorStyle",
					"AssetRegistry",
					"OpenXRExpansionPlugin"
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