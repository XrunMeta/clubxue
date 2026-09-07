

using UnrealBuildTool;
using System.Collections.Generic;

public class ClubXClientTarget : TargetRules
{
	public ClubXClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V7;

		ExtraModuleNames.AddRange( new string[] { "ClubX" } );
	}
}
