

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemEIK/Subsystem/EIK_Subsystem.h"
#include "EIK_FindSessions_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindSession_Delegate, const TArray<FSessionFindStruct>&, SessionResults);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_FindSessions_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FFindSession_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FFindSession_Delegate OnFail;

	UFUNCTION(BlueprintCallable, DisplayName="Find EIK Sessions",meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm=SessionSettings), Category="EOS Integration Kit || Sessions")
	static UEIK_FindSessions_AsyncFunction* FindEIKSessions(TMap<FString, FEIKAttribute> SessionSettings, EMatchType MatchType = EMatchType::MT_Lobby, int32 MaxResults = 15, ERegionInfo RegionToSearch = ERegionInfo::RE_NoSelection, bool bLanSearch = false, bool bIncludePartySessions = false);

	virtual void Activate() override;

	void FindSession();

	void OnFindSessionCompleted(bool bWasSuccess);

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	TMap<FString, FEIKAttribute> SessionSettings;
	ERegionInfo E_RegionToSearch;
	EMatchType E_MatchType;
	int32 I_MaxResults;
	bool B_bLanSearch;
	bool bIncludePartySessions = false;

	bool bDelegateCalled = false;

};
