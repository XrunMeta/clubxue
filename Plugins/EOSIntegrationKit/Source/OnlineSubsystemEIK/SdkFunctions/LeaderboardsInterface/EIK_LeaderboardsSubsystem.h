

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_leaderboards.h"
#include "eos_leaderboards_types.h"
THIRD_PARTY_INCLUDES_END
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_LeaderboardsSubsystem.generated.h"

UCLASS(DisplayName="Leaderboards Interface", meta=(DisplayName="Leaderboards Interface"))
class ONLINESUBSYSTEMEIK_API UEIK_LeaderboardsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardDefinitionByIndex")
	TEnumAsByte<EEIK_Result> EIK_Leaderboards_CopyLeaderboardDefinitionByIndex(int32 LeaderboardIndex, FEIK_Leaderboards_Definition& OutLeaderboardDefinition);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId")
	TEnumAsByte<EEIK_Result> EIK_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(FString LeaderboardId, FEIK_Leaderboards_Definition& OutLeaderboardDefinition);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardRecordByIndex")
	TEnumAsByte<EEIK_Result> EIK_Leaderboards_CopyLeaderboardRecordByIndex(int32 LeaderboardRecordIndex, FEIK_Leaderboards_LeaderboardRecord& OutLeaderboardRecord);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardRecordByUserId")
	TEnumAsByte<EEIK_Result> EIK_Leaderboards_CopyLeaderboardRecordByUserId(FEIK_ProductUserId UserId, FEIK_Leaderboards_LeaderboardRecord& OutLeaderboardRecord);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardUserScoreByIndex")
	TEnumAsByte<EEIK_Result> EIK_Leaderboards_CopyLeaderboardUserScoreByIndex(int32 LeaderboardUserScoreIndex, FString StatName, FEIK_Leaderboards_LeaderboardUserScore& OutLeaderboardUserScore);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardUserScoreByUserId")
	TEnumAsByte<EEIK_Result> EIK_Leaderboards_CopyLeaderboardUserScoreByUserId(FEIK_ProductUserId UserId, FString StatName, FEIK_Leaderboards_LeaderboardUserScore& OutLeaderboardUserScore);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_Definition_Release")
	void EIK_Leaderboards_LeaderboardDefinition_Release(FEIK_Leaderboards_Definition& LeaderboardDefinition);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_GetLeaderboardDefinitionCount")
	int32 EIK_Leaderboards_GetLeaderboardDefinitionCount();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_GetLeaderboardRecordCount")
	int32 EIK_Leaderboards_GetLeaderboardRecordCount();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_GetLeaderboardUserScoreCount")
	int32 EIK_Leaderboards_GetLeaderboardUserScoreCount();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_LeaderboardRecord_Release")
	void EIK_Leaderboards_LeaderboardRecord_Release(FEIK_Leaderboards_LeaderboardRecord& LeaderboardRecord);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_LeaderboardUserScore_Release")
	void EIK_Leaderboards_LeaderboardUserScore_Release(FEIK_Leaderboards_LeaderboardUserScore& LeaderboardUserScore);
};
