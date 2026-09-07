

using UnrealBuildTool;
using System.Collections.Generic;

public class ClubXServerTarget : TargetRules
{
	public ClubXServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V7;

		ExtraModuleNames.AddRange( new string[] { "ClubX" } );
	}
}
