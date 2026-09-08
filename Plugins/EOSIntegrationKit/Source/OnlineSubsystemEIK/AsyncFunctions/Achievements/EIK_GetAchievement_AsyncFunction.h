

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EIK_GetAchievement_AsyncFunction.generated.h"

USTRUCT(BlueprintType)
struct FEIK_Achievement
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category="EIK Struct")
	FString Id = "";

	UPROPERTY(BlueprintReadOnly, Category="EIK Struct")
	float Progress = 0.0;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAchievement_Delegate, const TArray<FEIK_Achievement>&, Achievements);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_GetAchievement_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FAchievement_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FAchievement_Delegate OnFail;

	UFUNCTION(BlueprintCallable, DisplayName="Get EIK Achievements",meta = (BlueprintInternalUseOnly = "true"), Category="EOS Integration Kit || Achievements")
	static UEIK_GetAchievement_AsyncFunction* GetEIKAchievements();

	void Activate() override;

	void GetAchievements();

	bool bDelegateCalled = false;

	void OnAchievementsCompleted(const FUniqueNetId& UniqueNetId, bool bWasSuccess);

};
