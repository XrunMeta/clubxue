

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEIK/SdkFunctions/ConnectInterface/EIK_ConnectSubsystem.h"
THIRD_PARTY_INCLUDES_START
#include "eos_leaderboards.h"
#include "eos_leaderboards_types.h"
THIRD_PARTY_INCLUDES_END
#include "EIK_Leaderboards_QueryLeaderboardUserScores.generated.h"

USTRUCT(BlueprintType)
struct FEIK_Leaderboards_QueryLeaderboardUserScoresOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	TArray<FEIK_ProductUserId> UserIds;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	TArray<FEIK_Leaderboards_UserScoresQueryStatInfo> StatInfo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	int64 StartTime = EOS_LEADERBOARDS_TIME_UNDEFINED;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	int64 EndTime = EOS_LEADERBOARDS_TIME_UNDEFINED;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	FEIK_ProductUserId LocalUserId;
	FEIK_Leaderboards_QueryLeaderboardUserScoresOptions()
		: UserIds()
		, StatInfo()
		, StartTime(-1)
		, EndTime(-1)
		, LocalUserId()
	{
	}
	EOS_Leaderboards_QueryLeaderboardUserScoresOptions ToEOSLeaderboardsQueryLeaderboardUserScoresOptions()
	{
		EOS_Leaderboards_QueryLeaderboardUserScoresOptions Result;
		Result.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDUSERSCORES_API_LATEST;
		Result.UserIdsCount = UserIds.Num();
		EOS_ProductUserId* TempUserIds = new EOS_ProductUserId[Result.UserIdsCount];
		for (int i = 0; i < UserIds.Num(); i++)
		{
			TempUserIds[i] = UserIds[i].GetValueAsEosType();
		}
		Result.StatInfoCount = StatInfo.Num();
		EOS_Leaderboards_UserScoresQueryStatInfo* TempStatInfo = new EOS_Leaderboards_UserScoresQueryStatInfo[Result.StatInfoCount];
		for (int i = 0; i < StatInfo.Num(); i++)
		{
			TempStatInfo[i] = StatInfo[i].GetValueAsEosType();
		}
		Result.UserIds = TempUserIds;
		Result.StatInfo = TempStatInfo;
		Result.StartTime = StartTime;
		Result.EndTime = EndTime;
		Result.LocalUserId = LocalUserId.GetValueAsEosType();
		return Result;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEIK_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback, const TEnumAsByte<EEIK_Result>&, Result);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Leaderboards_QueryLeaderboardUserScores : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName = "EOS_Leaderboards_QueryLeaderboardUserScores")
	static UEIK_Leaderboards_QueryLeaderboardUserScores* EIK_Leaderboards_QueryLeaderboardUserScores(const FEIK_Leaderboards_QueryLeaderboardUserScoresOptions& Options);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	FEIK_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback OnCallback;
private:
	FEIK_Leaderboards_QueryLeaderboardUserScoresOptions Var_Options;
	virtual void Activate() override;
	static void EOS_CALL Internal_OnQueryLeaderboardUserScoresCompleteCallback(const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo* Data);
};
