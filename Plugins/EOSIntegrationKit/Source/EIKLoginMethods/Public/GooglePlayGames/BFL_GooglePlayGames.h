

#pragma once

#include "CoreMinimal.h"
#include "GooglePlayGamesStructures.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFL_GooglePlayGames.generated.h"

UCLASS()
class EIKLOGINMETHODS_API UBFL_GooglePlayGames : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UBFL_GooglePlayGames();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|SignIn")
	static void ManualSignIn();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|SignIn")
	static bool IsSignedIn();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|SignIn")
	static FGPGS_Player GetPlayer();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|Achievements")
	static void UnlockAchievement(const FString& AchievementID);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|Achievements")
	static void IncrementAchievement(const FString& AchievementID, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|Achievements")
	static void DisplayAchievementsUI();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|Leaderboards")
	static void SubmitLeaderboardScore(const FString& LeaderboardID, int64 Value);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|Leaderboards")
	static void ShowLeaderboard(const FString& LeaderboardID);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|Friends")
	static void ComparePlayerProfile(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit|Google Play Games|Events")
	static void SubmitEvent(const FString& EventID, int32 NumberOfOccurrences);

};
