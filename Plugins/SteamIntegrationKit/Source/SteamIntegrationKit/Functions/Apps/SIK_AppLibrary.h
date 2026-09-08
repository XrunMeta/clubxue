

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SIK_AppLibrary.generated.h"

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_AppLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, DisplayName="Get DLC Data By Index", Category="Steam Integration Kit || SDK Functions || Apps")
	static bool GetDLCDataByIndex(int32 Index, FSIK_AppId& AppID, bool& Available, FString& Name);

	UFUNCTION(BlueprintCallable, DisplayName="Is App Installed", meta=(Keywords="BIsAppInstalled"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsAppInstalled(FSIK_AppId AppID);

	UFUNCTION(BlueprintCallable, DisplayName="Is Cybercafe", meta=(Keywords="BIsCybercafe"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsCybercafe();

	UFUNCTION(BlueprintCallable, DisplayName="Is DLC Installed", meta=(Keywords="BIsDLCInstalled"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsDLCInstalled(int32 AppID);

	UFUNCTION(BlueprintCallable, DisplayName="Is Low Violence", meta=(Keywords="BIsLowViolence"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsLowViolence();

	UFUNCTION(BlueprintCallable, DisplayName="Is Subscribed", meta=(Keywords="BIsSubscribed"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsSubscribed();

	UFUNCTION(BlueprintCallable, DisplayName="Is Subscribed App", meta=(Keywords="BIsSubscribedApp"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsSubscribedApp(int32 AppID);

	UFUNCTION(BlueprintCallable, DisplayName="Is Subscribed From Family Sharing", meta=(Keywords="BIsSubscribedFromFamilySharing"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsSubscribedFromFamilySharing();

	UFUNCTION(BlueprintCallable, DisplayName="Is Subscribed From Free Weekend", meta=(Keywords="BIsSubscribedFromFreeWeekend"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsSubscribedFromFreeWeekend();

	UFUNCTION(BlueprintCallable, DisplayName="Is Timed Trial", meta=(Keywords="BIsTimedTrial"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsTimedTrial(int32& SecondsAllowed, int32& SecondsPlayed);

	UFUNCTION(BlueprintCallable, DisplayName="Is VAC Banned", meta=(Keywords="BIsVACBanned"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool IsVACBanned();

	UFUNCTION(BlueprintCallable, DisplayName="Get App Build ID", meta=(Keywords="GetAppBuildID"), Category="Steam Integration Kit || SDK Functions || Apps")
	static int32 GetAppBuildID();

	UFUNCTION(BlueprintCallable, DisplayName="Get App Install Dir", meta=(Keywords="GetAppInstallDir"), Category="Steam Integration Kit || SDK Functions || Apps")
	static FString GetAppInstallDir(int32 AppID);

	UFUNCTION(BlueprintCallable, DisplayName="Get App Owner", meta=(Keywords="GetAppOwner"), Category="Steam Integration Kit || SDK Functions || Apps")
	static FSIK_SteamId GetAppOwner();

	UFUNCTION(BlueprintCallable, DisplayName="Get Available Game Languages", meta=(Keywords="GetAvailableGameLanguages"), Category="Steam Integration Kit || SDK Functions || Apps")
	static TArray<FString> GetAvailableGameLanguages();

	UFUNCTION(BlueprintCallable, DisplayName="Get Current Beta Name", meta=(Keywords="GetCurrentBetaName"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool GetCurrentBetaName(FString& Name);

	UFUNCTION(BlueprintCallable, DisplayName="Get Current Game Language", meta=(Keywords="GetCurrentGameLanguage"), Category="Steam Integration Kit || SDK Functions || Apps")
	static FString GetCurrentGameLanguage();

	UFUNCTION(BlueprintCallable, DisplayName="Get DLC Count", meta=(Keywords="GetDLCCount"), Category="Steam Integration Kit || SDK Functions || Apps")
	static int32 GetDLCCount();

	UFUNCTION(BlueprintCallable, DisplayName="Get DLC Download Progress", meta=(Keywords="GetDLCDownloadProgress"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool GetDLCDownloadProgress(FSIK_AppId AppID, int64& BytesDownloaded, int64& BytesTotal);

	UFUNCTION(BlueprintCallable, DisplayName="Get Earliest Purchase Unix Time", meta=(Keywords="GetEarliestPurchaseUnixTime"), Category="Steam Integration Kit || SDK Functions || Apps")
	static int32 GetEarliestPurchaseUnixTime(FSIK_AppId AppID);

	UFUNCTION(BlueprintCallable, DisplayName="Get Installed Depots", meta=(Keywords="GetInstalledDepots"), Category="Steam Integration Kit || SDK Functions || Apps")
	static TArray<int32> GetInstalledDepots(FSIK_AppId AppID);

	UFUNCTION(BlueprintCallable, DisplayName="Get Launch Command Line", meta=(Keywords="GetLaunchCommandLine"), Category="Steam Integration Kit || SDK Functions || Apps")
	static int32 GetLaunchCommandLine(FString& CommandLine);

	UFUNCTION(BlueprintCallable, DisplayName="Get Launch Query Param", meta=(Keywords="GetLaunchQueryParam"), Category="Steam Integration Kit || SDK Functions || Apps")
	static FString GetLaunchQueryParam(FString Key);

	UFUNCTION(BlueprintCallable, DisplayName="Install DLC", meta=(Keywords="InstallDLC"), Category="Steam Integration Kit || SDK Functions || Apps")
	static void InstallDLC(FSIK_AppId AppID);

	UFUNCTION(BlueprintCallable, DisplayName="Mark Content Corrupt", meta=(Keywords="MarkContentCorrupt"), Category="Steam Integration Kit || SDK Functions || Apps")
	static bool MarkContentCorrupt(bool MissingFilesOnly);

	UFUNCTION(BlueprintCallable, DisplayName="Uninstall DLC", meta=(Keywords="UninstallDLC"), Category="Steam Integration Kit || SDK Functions || Apps")
	static void UninstallDLC(FSIK_AppId AppID);
};
