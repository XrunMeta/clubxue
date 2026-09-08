

using UnrealBuildTool;

public class VisualStudioTools : ModuleRules
{
    public VisualStudioTools(ReadOnlyTargetRules Target) : base(Target)
    {
        bool bIsCustomDevBuild = System.Environment.GetEnvironmentVariable("VSTUE_IsCustomDevBuild") == "1";
        if (bIsCustomDevBuild)
        {

            PCHUsage = ModuleRules.PCHUsageMode.NoPCHs;
            bUseUnity = false;

            OptimizeCode = CodeOptimization.Never;

            if (Target.Version.MajorVersion >= 5)
            {
                UnsafeTypeCastWarningLevel = WarningLevel.Error;
            }
        }
        else
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        }

        if ((Target.Version.MajorVersion == 5 && Target.Version.MinorVersion >= 1) || Target.Version.MajorVersion > 5)
        {
            PrivateDefinitions.Add("FILTER_ASSETS_BY_CLASS_PATH=1");
        }
        else
        {
            PrivateDefinitions.Add("FILTER_ASSETS_BY_CLASS_PATH=0");
        }

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "AssetRegistry",
                "CoreUObject",
                "Engine",
                "Json",
                "JsonUtilities",
                "Kismet",
                "UnrealEd",
            }
        );
    }
}
