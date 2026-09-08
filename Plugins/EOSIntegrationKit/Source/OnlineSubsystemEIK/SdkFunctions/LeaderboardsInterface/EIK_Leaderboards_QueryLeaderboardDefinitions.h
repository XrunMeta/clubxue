

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_leaderboards.h"
#include "eos_leaderboards_types.h"
THIRD_PARTY_INCLUDES_END
#include "EIK_Leaderboards_QueryLeaderboardDefinitions.generated.h"

USTRUCT(BlueprintType)
struct FEIK_Leaderboards_QueryLeaderboardDefinitionsOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	int64 StartTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	int64 EndTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	FEIK_ProductUserId LocalUserId;

	FEIK_Leaderboards_QueryLeaderboardDefinitionsOptions()
		: StartTime(EOS_LEADERBOARDS_TIME_UNDEFINED)
		, EndTime(EOS_LEADERBOARDS_TIME_UNDEFINED)
		, LocalUserId()
	{
	}
	EOS_Leaderboards_QueryLeaderboardDefinitionsOptions ToEOSLeaderboardsQueryLeaderboardDefinitionsOptions()
	{
		EOS_Leaderboards_QueryLeaderboardDefinitionsOptions Result;
		Result.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST;
		Result.StartTime = StartTime;
		Result.EndTime = EndTime;
		Result.LocalUserId = LocalUserId.GetValueAsEosType();
		return Result;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEIK_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback, const TEnumAsByte<EEIK_Result>&, Result);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Leaderboards_QueryLeaderboardDefinitions : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName = "EOS_Leaderboards_QueryLeaderboardDefinitions")
	static UEIK_Leaderboards_QueryLeaderboardDefinitions* EIK_Leaderboards_QueryLeaderboardDefinitions(const FEIK_Leaderboards_QueryLeaderboardDefinitionsOptions& Options);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface")
	FEIK_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback OnCallback;

private:
	FEIK_Leaderboards_QueryLeaderboardDefinitionsOptions Var_Options;
	static void EOS_CALL OnQueryLeaderboardDefinitionsCompleteCallback(const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo* Data);
	virtual void Activate() override;
};
