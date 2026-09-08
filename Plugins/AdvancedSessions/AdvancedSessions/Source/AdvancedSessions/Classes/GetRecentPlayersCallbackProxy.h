
#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "GetRecentPlayersCallbackProxy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdvancedGetRecentPlayersLog, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintGetRecentPlayersDelegate, const TArray<FBPOnlineRecentPlayer>&, Results);

UCLASS(MinimalAPI)
class UGetRecentPlayersCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FBlueprintGetRecentPlayersDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FBlueprintGetRecentPlayersDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedFriends")
	static UGetRecentPlayersCallbackProxy* GetAndStoreRecentPlayersList(UObject* WorldContextObject, const FBPUniqueNetId &UniqueNetId);

	virtual void Activate() override;

private:

	void OnQueryRecentPlayersCompleted(const FUniqueNetId &UserID, const FString &Namespace, bool bWasSuccessful, const FString& ErrorString);

	FDelegateHandle DelegateHandle;

	FBPUniqueNetId cUniqueNetId;

	FOnQueryRecentPlayersCompleteDelegate QueryRecentPlayersCompleteDelegate;

	TWeakObjectPtr<UObject> WorldContextObject;
};

