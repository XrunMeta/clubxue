

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SIK_SharedFile.h"
#include "SIK_CreateLeaderboard_AsyncFunction.generated.h"

UENUM(BlueprintType)
enum ESIK_LeaderboardSortMethod
{
	LeaderboardSortMethodNone = 0 UMETA(DisplayName = "None"),	
	LeaderboardSortMethodAscending = 1 UMETA(DisplayName = "Ascending"),		
	LeaderboardSortMethodDescending = 2 UMETA(DisplayName = "Descending"),	
};

UENUM(BlueprintType)
enum ESIK_LeaderboardDisplayType
{
	LeaderboardDisplayTypeNone = 0 UMETA(DisplayName = "None"),	
	LeaderboardDisplayTypeNumeric = 1 UMETA(DisplayName = "Numeric"),		
	LeaderboardDisplayTypeTimeSeconds = 2 UMETA(DisplayName = "TimeSeconds"),	
	LeaderboardDisplayTypeTimeMilliSeconds = 3 UMETA(DisplayName = "TimeMilliSeconds"),	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateorFindLeaderboard_Delegate, int32, LeaderboardID);

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_CreateLeaderboard_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, DisplayName="Create or Find Steam Leaderboard",meta = (BlueprintInternalUseOnly = "true"), Category="Steam Integration Kit || SDK Functions || User Stats")
	static USIK_CreateLeaderboard_AsyncFunction* CreateLeaderboard(const FString& LeaderboardName,TEnumAsByte<ESIK_LeaderboardSortMethod> SortMethod, TEnumAsByte<ESIK_LeaderboardDisplayType> DisplayType);

	UPROPERTY(BlueprintAssignable)
	FCreateorFindLeaderboard_Delegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FCreateorFindLeaderboard_Delegate OnFailure;

private:
	FString Var_LeaderboardName;
	TEnumAsByte<ESIK_LeaderboardSortMethod> Var_SortMethod;
	TEnumAsByte<ESIK_LeaderboardDisplayType> Var_DisplayType;
#if (WITH_ENGINE_STEAM && ONLINESUBSYSTEMSTEAM_PACKAGE) || (WITH_STEAMKIT && !WITH_ENGINE_STEAM)	
	SteamAPICall_t CallbackHandle;
	void OnFindLeaderboard( LeaderboardFindResult_t *pResult, bool bIOFailure);
	CCallResult<USIK_CreateLeaderboard_AsyncFunction, LeaderboardFindResult_t> OnFindLeaderboardCallResult;
#endif
	virtual void Activate() override;
};
