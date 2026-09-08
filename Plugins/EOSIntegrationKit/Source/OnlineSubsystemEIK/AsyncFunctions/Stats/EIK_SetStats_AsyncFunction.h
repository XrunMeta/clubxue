

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineError.h"
#include "Runtime/Launch/Resources/Version.h"
#include "EIK_SetStats_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSetStatsResult);
UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_SetStats_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	FString StatName;
	int32 StatValue;
	UPROPERTY(BlueprintAssignable)
	FSetStatsResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FSetStatsResult OnFail;

	bool bDelegateCalled = false;

	UFUNCTION(BlueprintCallable, DisplayName="Set EIK Stats", meta = (BlueprintInternalUseOnly = "true"), Category="EOS Integration Kit || Statistics")
	static UEIK_SetStats_AsyncFunction* SetEIKStats(const FString& StatName, const int32 StatValue);

	void OnEUpdateStatsCompleted(const FOnlineError& Result);

	void Activate() override;
	void SetEIKStatsLocal();
};
