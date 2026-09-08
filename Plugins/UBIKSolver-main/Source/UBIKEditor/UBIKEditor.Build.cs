

using UnrealBuildTool;

public class UBIKEditor : ModuleRules
{
	public UBIKEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {

			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {

			}
			);

        PublicDependencyModuleNames.AddRange(new string[] { "UBIKRuntime", "Core", "CoreUObject", "Engine", "InputCore"});

        PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "EditorStyle", "AnimGraph", "AnimGraphRuntime", "BlueprintGraph", "PropertyEditor", "Slate", "SlateCore" });

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			}
			);
	}
}
