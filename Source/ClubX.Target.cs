

using UnrealBuildTool;
using System.Collections.Generic;

public class ClubXTarget : TargetRules
{
	public ClubXTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;

		ExtraModuleNames.AddRange( new string[] { "ClubX" } );
	}
}
