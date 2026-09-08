

#include "DiscordGame.h"
#include "Misc/Paths.h"
#include "Windows/WindowsPlatformProcess.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"  

DEFINE_LOG_CATEGORY(LogDiscord);

const FName FDiscordGameModule::ModuleName ("EOSIntegrationKit");

void FDiscordGameModule::StartupModule()
{
#if EIKDISCORDACTIVE

	const FString LibraryPath = GetPathToDLL();

	if ensureAlwaysMsgf(!LibraryPath.IsEmpty(), TEXT("Expect LibraryPath to not be empty"))
	{

		DiscordGameSDKHandle = FPlatformProcess::GetDllHandle(*LibraryPath);

		if ensureAlwaysMsgf(DiscordGameSDKHandle, TEXT("Expect to load Discord SDK at path [%s]"), *LibraryPath)
		{
			UE_LOG(LogDiscord, Log, TEXT("Loaded Discord GameSDK DLL [%s]"), *LibraryPath);
			if(DiscordGameSDKHandle)
			{
				bDiscordSDKLoaded = true;
			}
			else
			{
				UE_LOG(LogDiscord, Error, TEXT("Failed to load Discord GameSDK DLL [%s]"), *LibraryPath);
			}
		}
		else
		{
			UE_LOG(LogDiscord, Error, TEXT("Failed to load Discord GameSDK DLL [%s]"), *LibraryPath);
		}
	}
	else
	{
		UE_LOG(LogDiscord, Error, TEXT("Failed to determine path to Discord GameSDK DLL"));
	}
#else
	UE_LOG(LogDiscord, Log, TEXT("EIK: DiscordGame is disabled, skipping DiscordGame SDK load"));
#endif
}

void FDiscordGameModule::ShutdownModule()
{
	if (DiscordGameSDKHandle)
	{

		FPlatformProcess::FreeDllHandle(DiscordGameSDKHandle);
		DiscordGameSDKHandle = nullptr;
	}
}

FString FDiscordGameModule::GetPathToDLL() const
{

	FString LibraryPath;

#if WITH_EDITOR && (PLATFORM_WINDOWS || PLATFORM_LINUX)

	FString BaseDir = IPluginManager::Get().FindPlugin(ModuleName.ToString())->GetBaseDir();
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/DiscordGameSDK"));

#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*LibraryPath, TEXT("x86_64/discord_game_sdk.dll"));
#elif PLATFORM_LINUX
    LibraryPath = FPaths::Combine(*LibraryPath, TEXT("x86_64/discord_game_sdk.so"));
#else
#error Unsupported platform
#endif

#else

	FString BaseDir = FPaths::ProjectDir();
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries"));

#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*LibraryPath, TEXT("Win64/discord_game_sdk.dll"));
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*LibraryPath, TEXT("Mac/discord_game_sdk.dylib"));
#elif PLATFORM_LINUX
	LibraryPath = FPaths::Combine(*LibraryPath, TEXT("Linux/discord_game_sdk.so"));
#else
#error Unsupported platform
#endif

#endif

	const FString FullPath (FPaths::ConvertRelativePathToFull(*LibraryPath));
	return FullPath;
}

IMPLEMENT_MODULE(FDiscordGameModule, DiscordGame)
