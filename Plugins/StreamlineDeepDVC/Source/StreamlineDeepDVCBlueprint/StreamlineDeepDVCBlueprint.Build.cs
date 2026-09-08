

using UnrealBuildTool;
using System.IO;

public class StreamlineDeepDVCBlueprint : ModuleRules
{

	protected virtual bool IsSupportedPlatform(ReadOnlyTargetRules Target)
	{
		return Target.Platform.IsInGroup(UnrealPlatformGroup.Windows);
	}

	public StreamlineDeepDVCBlueprint(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RenderCore",
				"Renderer",
				"Projects",
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);

		bool bPlatformSupportsStreamline = IsSupportedPlatform(Target);

		PublicDefinitions.Add("WITH_STREAMLINE=" + (bPlatformSupportsStreamline ? '1' : '0'));

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"StreamlineBlueprint"
			}
		);

		if (bPlatformSupportsStreamline)
		{ 
			PublicIncludePaths.AddRange(
				new string[]
				{
				}
			);

			PrivateIncludePaths.AddRange(
				new string[]
				{
				}
			);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"StreamlineCore",
					"StreamlineRHI",
					"Streamline",

				}
			);
		}
	}

}
