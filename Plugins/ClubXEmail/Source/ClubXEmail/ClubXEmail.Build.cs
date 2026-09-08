using UnrealBuildTool;
using System.IO;

public class ClubXEmail : ModuleRules
{
    public ClubXEmail(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Projects"
        });

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("Launch");

            string APLPath = Path.Combine(
                ModuleDirectory,
                "Private",
                "ClubXEmail_APL.xml"
            );

            AdditionalPropertiesForReceipt.Add(
                "AndroidPlugin",
                APLPath
            );
        }
    }
}