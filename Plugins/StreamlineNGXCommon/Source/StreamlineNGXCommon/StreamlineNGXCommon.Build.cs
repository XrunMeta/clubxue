

using UnrealBuildTool;
using System.IO;

public class StreamlineNGXCommon : ModuleRules
{
	public StreamlineNGXCommon(ReadOnlyTargetRules Target): base(Target)
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

#if UE_5_5_OR_LATER
		PublicDefinitions.Add("ENGINE_ID3D12DYNAMICRHI_NEEDS_CMDLIST=1");
#else
		PublicDefinitions.Add("ENGINE_ID3D12DYNAMICRHI_NEEDS_CMDLIST=0");
#endif

	}
}
