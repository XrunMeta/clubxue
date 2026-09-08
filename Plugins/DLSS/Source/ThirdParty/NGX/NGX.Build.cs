

using EpicGames.Core;
using UnrealBuildTool;
using System.IO;

public class NGX : ModuleRules
{

	protected virtual bool IsSupportedWindowsPlatform(ReadOnlyTargetRules Target)
	{
		return Target.Platform.IsInGroup(UnrealPlatformGroup.Windows);
	}

	protected virtual string GetPlatformDir(ReadOnlyTargetRules Target)
	{
		return Target.Platform.ToString();
	}

	protected virtual string GetLibDir(ReadOnlyTargetRules Target)
	{
		return "x64";
	}

	public NGX(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (IsSupportedWindowsPlatform(Target))
		{

			string PlatformDir = GetPlatformDir(Target); 
			string LibDir = GetLibDir(Target);

			string NGXPath = ModuleDirectory + "/";

			PublicSystemIncludePaths.Add(NGXPath + "Include/");

			System.Console.WriteLine("NGX Using Lib Dir {0} Platform Dir {1}", LibDir,PlatformDir);

			if ((Target.Configuration == UnrealTargetConfiguration.Debug) && Target.bDebugBuildsActuallyUseDebugCRT)
			{
				if (Target.bUseStaticCRT)
				{
					PublicAdditionalLibraries.Add(NGXPath + "Lib/" + LibDir + "/nvsdk_ngx_s_dbg.lib");
				}
				else
				{
					PublicAdditionalLibraries.Add(NGXPath + "Lib/" + LibDir + "/nvsdk_ngx_d_dbg.lib");
				}
			}
			else
			{
				if (Target.bUseStaticCRT)
				{
					PublicAdditionalLibraries.Add(NGXPath + "Lib/" + LibDir + "/nvsdk_ngx_s.lib");
				}
				else
				{
					PublicAdditionalLibraries.Add(NGXPath + "Lib/" + LibDir + "/nvsdk_ngx_d.lib");
				}
			}

			string[] NGXSnippetDLLs =
			{
				"nvngx_dlss.dll",
				"nvngx_dlssd.dll"
			};

			PublicDefinitions.Add("NGX_DLSS_SR_BINARY_NAME=TEXT(\"" + NGXSnippetDLLs[0] + "\")");
			PublicDefinitions.Add("NGX_DLSS_RR_BINARY_NAME=TEXT(\"" + NGXSnippetDLLs[1] + "\")");
			PublicDefinitions.Add("NGX_PLATFORM_DIR=TEXT(\"" + PlatformDir + "\")");

			foreach (string NGXSnippetDLL in NGXSnippetDLLs)
			{
				bool bHasProjectBinary = false;
				if (Target.ProjectFile != null)
				{
					string ProjectDLLPath = DirectoryReference.Combine(Target.ProjectFile.Directory, "Binaries/ThirdParty/NVIDIA/NGX/", PlatformDir, NGXSnippetDLL).FullName;
					if (File.Exists(ProjectDLLPath))
					{
						bHasProjectBinary = true;

						RuntimeDependencies.Add(ProjectDLLPath, StagedFileType.NonUFS);
					}
				}

				string SnippetBasePath = Path.Combine(PluginDirectory, "Binaries/ThirdParty/", PlatformDir);

				if (!bHasProjectBinary || Target.Configuration != UnrealTargetConfiguration.Shipping)
				{
					bool bProdSnippetExists = File.Exists(Path.Combine(SnippetBasePath, NGXSnippetDLL));
					if (bProdSnippetExists)
					{
						RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/" + PlatformDir + "/" + NGXSnippetDLL, StagedFileType.NonUFS);
					}
				}

				if (Target.Configuration != UnrealTargetConfiguration.Shipping)
				{
					bool bDevSnippetExists = File.Exists(Path.Combine(SnippetBasePath, "Development", NGXSnippetDLL));
					if (bDevSnippetExists)
					{
						RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/" + PlatformDir + "/Development/" + NGXSnippetDLL, StagedFileType.DebugNonUFS);
					}
				}
			}
		}
	}
}

