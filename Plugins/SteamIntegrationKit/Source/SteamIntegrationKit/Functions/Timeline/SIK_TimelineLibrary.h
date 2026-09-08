

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SIK_TimelineLibrary.generated.h"

UENUM(BlueprintType)
enum ESIK_TimelineGameMode
{
	ESIK_TimelineGameMode_Invalid = 0 UMETA(DisplayName = "Invalid"),

	ESIK_TimelineGameMode_Playing = 1 UMETA(DisplayName = "Playing"),

	ESIK_TimelineGameMode_Staging = 2 UMETA(DisplayName = "Staging"),

	ESIK_TimelineGameMode_Menus = 3 UMETA(DisplayName = "Menus"),

	ESIK_TimelineGameMode_LoadingScreen = 4 UMETA(DisplayName = "Loading Screen"),
};

UENUM(BlueprintType)
enum ESIK_TimelineEventClipPriority
{
	ESIK_TimelineEventClipPriority_Invalid = 0 UMETA(DisplayName = "Invalid"),

	ESIK_TimelineEventClipPriority_None = 1 UMETA(DisplayName = "None"),

	ESIK_TimelineEventClipPriority_Standard = 2 UMETA(DisplayName = "Standard"),

	ESIK_TimelineEventClipPriority_Featured = 3 UMETA(DisplayName = "Featured"),
};

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_TimelineLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Timeline")
	static void SetTimelineStateDescription(FString pchDescription, float flTimeDelta);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Timeline")
	static void ClearTimelineStateDescription(float flTimeDelta);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Timeline")
	static void AddTimelineEvent(FString pchIcon, FString pchTitle, FString pchDescription, int32 unPriority, float flStartOffsetSeconds, float flDurationSecondsm, TEnumAsByte<ESIK_TimelineEventClipPriority> ePossibleClip);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Timeline")
	static void SetTimelineGameMode(TEnumAsByte<ESIK_TimelineGameMode> eMode);
};
