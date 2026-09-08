

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SIK_FindSessions_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSIK_FindSession_Delegate, const TArray<FSSessionFindStruct>&, SessionResults);

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_FindSessions_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FSIK_FindSession_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FSIK_FindSession_Delegate OnFail;

	UFUNCTION(BlueprintCallable, DisplayName="Find SIK Sessions",meta = (BlueprintInternalUseOnly = "true",AutoCreateRefTerm = SessionSettings), Category="Steam Integration Kit || Game Functions || Sessions")
	static USIK_FindSessions_AsyncFunction* FindSIKSessions(TMap<FName, FSIKAttribute> SessionSettings, ESMatchType MatchType = ESMatchType::MT_Lobby, int32 MaxResults = 15, bool bLanSearch = false, bool bEmptyServerOnly = false, bool bSecureServerOnly = false);

	virtual void Activate() override;

	void FindSession();

	void OnFindSessionCompleted(bool bWasSuccess);

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	TMap<FName, FSIKAttribute> SessionSettings;
	ESMatchType E_MatchType;
	int32 I_MaxResults;
	bool B_bLanSearch;
	bool B_bEmptyServerOnly;
	bool B_bSecureServerOnly;

	bool bDelegateCalled = false;

};
