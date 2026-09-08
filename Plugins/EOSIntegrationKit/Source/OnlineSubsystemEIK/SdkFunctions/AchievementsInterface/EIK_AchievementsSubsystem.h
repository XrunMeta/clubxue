

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_AchievementsSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnAchievementsUnlockedV2Callback, FEIK_ProductUserId, UserId, FString, AchievementId, int64, UnlockTime);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_AchievementsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FOnAchievementsUnlockedV2Callback OnAchievementsUnlockedV2;

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_AddNotifyAchievementsUnlockedV2")
	FEIK_NotificationId EIK_Achievements_AddNotifyAchievementsUnlockedV2(FOnAchievementsUnlockedV2Callback Callback);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId")
	TEnumAsByte<EEIK_Result> EIK_Achievements_CopyAchievementDefinitionV2ByAchievementId(FString AchievementId, FEIK_Achievements_DefinitionV2& OutAchievementDefinition);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_CopyAchievementDefinitionV2ByIndex")
	TEnumAsByte<EEIK_Result> EIK_Achievements_CopyAchievementDefinitionV2ByIndex(int32 Index, FEIK_Achievements_DefinitionV2& OutAchievementDefinition);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_CopyPlayerAchievementByAchievementId")
	TEnumAsByte<EEIK_Result> EIK_Achievements_CopyPlayerAchievementByAchievementId(FEIK_ProductUserId TargetUserId, FString AchievementId, FEIK_ProductUserId LocalUserId, FEIK_Achievements_PlayerAchievement& OutPlayerAchievement);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_CopyPlayerAchievementByIndex")
	TEnumAsByte<EEIK_Result> EIK_Achievements_CopyPlayerAchievementByIndex(FEIK_ProductUserId TargetUserId, int32 Index, FEIK_ProductUserId LocalUserId, FEIK_Achievements_PlayerAchievement& OutPlayerAchievement);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_DefinitionV2_Release")
	void EIK_Achievements_DefinitionV2_Release(FEIK_Achievements_DefinitionV2& AchievementDefinition);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_GetAchievementDefinitionCount")
	int32 EIK_Achievements_GetAchievementDefinitionCount();

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_GetPlayerAchievementCount")
	int32 EIK_Achievements_GetPlayerAchievementCount(FEIK_ProductUserId UserId);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_PlayerAchievement_Release")
	void EIK_Achievements_PlayerAchievement_Release(FEIK_Achievements_PlayerAchievement& PlayerAchievement);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_QueryDefinitions")
	TEnumAsByte<EEIK_Result> EIK_Achievements_QueryDefinitions(FEIK_ProductUserId UserId);

	UFUNCTION(BlueprintCallable, Category="EOS Integration Kit | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_RemoveNotifyAchievementsUnlocked")
	void EIK_Achievements_RemoveNotifyAchievementsUnlocked(FEIK_NotificationId Id);

};
