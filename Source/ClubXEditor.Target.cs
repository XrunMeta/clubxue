

using UnrealBuildTool;
using System.Collections.Generic;

public class ClubXEditorTarget : TargetRules
{
    public ClubXEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;

        DisablePlugins.Add("OculusXR");
        DisablePlugins.Add("MRUtilityKit");
        DisablePlugins.Add("OculusXRAnchors");

        DefaultBuildSettings = BuildSettingsVersion.V7;

        ExtraModuleNames.AddRange(new string[] { "ClubX" });
    }
}