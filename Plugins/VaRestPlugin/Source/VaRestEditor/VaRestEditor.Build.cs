

using UnrealBuildTool;

public class VaRestEditor : ModuleRules
{
	public VaRestEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        DefaultBuildSettings = BuildSettingsVersion.V5;

		PrivateIncludePaths.AddRange(
			new string[] {
				"VaRestEditor/Private",

			});

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "VaRest"

			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "InputCore",
                "AssetTools",
                "UnrealEd",     
                "KismetWidgets",
                "KismetCompiler",
                "BlueprintGraph",
                "GraphEditor",
                "Kismet",       
                "PropertyEditor",
                "EditorStyle",
                "Sequencer",
                "DetailCustomizations",
                "Settings",
                "RenderCore"
			});

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			});
	}
}
