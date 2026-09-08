

using UnrealBuildTool;
using System.IO;

public class StreamlineD3D12RHI : ModuleRules
{
	public StreamlineD3D12RHI(ReadOnlyTargetRules Target) : base(Target)
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
				"StreamlineRHI",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"D3D12RHI",
				"Engine",
				"RenderCore",
				"RHI",
				"RHICore",
				"Streamline",
				"StreamlineDXGIRHI",
				"StreamlineRHI",
			}
		);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "DX12");
	}
}
