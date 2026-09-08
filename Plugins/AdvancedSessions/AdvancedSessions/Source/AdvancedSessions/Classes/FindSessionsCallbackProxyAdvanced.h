
#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "BlueprintDataDefinitions.h"
#include "FindSessionsCallbackProxyAdvanced.generated.h"

FORCEINLINE bool operator==(const FBlueprintSessionResult& A, const FBlueprintSessionResult& B)
{
	return (A.OnlineResult.IsValid() == B.OnlineResult.IsValid() && (A.OnlineResult.GetSessionIdStr() == B.OnlineResult.GetSessionIdStr()));
}

UCLASS(MinimalAPI)
class UFindSessionsCallbackProxyAdvanced : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FBlueprintFindSessionsResultDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FBlueprintFindSessionsResultDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm="Filters"), Category = "Online|AdvancedSessions")
	static UFindSessionsCallbackProxyAdvanced* FindSessionsAdvanced(UObject* WorldContextObject, class APlayerController* PlayerController, int32 MaxResults, bool bUseLAN, EBPServerPresenceSearchType ServerTypeToSearch, const TArray<FSessionsSearchSetting> &Filters, bool bEmptyServersOnly = false, bool bNonEmptyServersOnly = false, bool bSecureServersOnly = false,  int MinSlotsAvailable = 0);

	static bool CompareVariants(const FVariantData &A, const FVariantData &B, EOnlineComparisonOpRedux Comparator);

	UFUNCTION(BluePrintCallable, meta = (Category = "Online|AdvancedSessions"))
	static void FilterSessionResults(const TArray<FBlueprintSessionResult> &SessionResults, const TArray<FSessionsSearchSetting> &Filters, TArray<FBlueprintSessionResult> &FilteredResults);

	virtual void Activate() override;

private:

	void OnCompleted(bool bSuccess);

	bool bRunSecondSearch;
	bool bIsOnSecondSearch;

	TArray<FBlueprintSessionResult> SessionSearchResults;

private:

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FOnFindSessionsCompleteDelegate Delegate;

	FDelegateHandle DelegateHandle;

	TSharedPtr<FOnlineSessionSearch> SearchObject;
	TSharedPtr<FOnlineSessionSearch> SearchObjectDedicated;

	bool bUseLAN;

	EBPServerPresenceSearchType ServerSearchType;

	int MaxResults;

	TArray<FSessionsSearchSetting> SearchSettings;

	bool bEmptyServersOnly;

	bool bNonEmptyServersOnly;

	bool bSecureServersOnly;

	int MinSlotsAvailable;

	TWeakObjectPtr<UObject> WorldContextObject;
};
