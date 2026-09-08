

using UnrealBuildTool;

public class NGXD3D11RHI : ModuleRules
{
	public NGXD3D11RHI(ReadOnlyTargetRules Target) : base(Target)
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

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
					"Core",
					"Engine",
					"RenderCore",
					"RHI",
					"D3D11RHI",

					"NGX",
					"NGXRHI",
			}
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "DX11");
	}
}
