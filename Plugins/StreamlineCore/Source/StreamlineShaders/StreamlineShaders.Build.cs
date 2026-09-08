

using UnrealBuildTool;
using System.IO;

public class StreamlineShaders : ModuleRules
{
	public StreamlineShaders(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(EngineDirectory,"Source/Runtime/Renderer/Private"),
#if UE_5_6_OR_LATER
				Path.Combine(EngineDirectory,"Source/Runtime/Renderer/Internal"),
#endif
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"RenderCore",
				"Renderer",

			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
					"Engine",
					"RHI",
					"Projects"
			}
			);

		bool bEngineUsesFVector2D = (Target.Version.MajorVersion == 4) || (Target.Version.BranchName == "++UE5+Release-5.0-EarlyAccess");
		PrivateDefinitions.Add(string.Format("DLSS_ENGINE_USES_FVECTOR2D={0}", bEngineUsesFVector2D ? "1" : "0"));
	}
}
