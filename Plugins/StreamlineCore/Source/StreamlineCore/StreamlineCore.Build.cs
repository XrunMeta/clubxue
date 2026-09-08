

using System.IO;
using UnrealBuildTool;

public class StreamlineCore : ModuleRules
{
	public StreamlineCore(ReadOnlyTargetRules Target) : base(Target)
	{

#if !UE_5_0_OR_LATER
		if (CppStandard < CppStandardVersion.Cpp17)
		{
			CppStandard = CppStandardVersion.Cpp17;
		}
#endif

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
				EngineDirectory + "/Source/Runtime/Renderer/Private",
#if UE_5_6_OR_LATER
				EngineDirectory + "/Source/Runtime/Renderer/Internal",
#endif
				Path.Combine(ModuleDirectory, "ThirdParty"),
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
					"CoreUObject",
					"EngineSettings",
					"Engine",
					"RenderCore",
					"Renderer",
					"RHI",
					"Projects",
					"SlateCore",
					"Slate",

					"Streamline",
					"StreamlineRHI",
					"StreamlineDXGIRHI",
					"StreamlineShaders",

					"ApplicationCore",

			}
			);

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("Settings");
		}

	}
}
