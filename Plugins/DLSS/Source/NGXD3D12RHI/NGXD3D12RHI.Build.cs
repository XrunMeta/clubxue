

using UnrealBuildTool;
using System.IO;
public class NGXD3D12RHI : ModuleRules
{
	public NGXD3D12RHI(ReadOnlyTargetRules Target) : base(Target)
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
					"D3D12RHI",

					"NGX",
					"NGXRHI",
			}
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "DX12");
	}
}
