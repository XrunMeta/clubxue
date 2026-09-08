
#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "BlueprintDataDefinitions.h"
#include "CreateSessionCallbackProxyAdvanced.generated.h"

UCLASS(MinimalAPI)
class UCreateSessionCallbackProxyAdvanced : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject",AutoCreateRefTerm="ExtraSettings"), Category = "Online|AdvancedSessions")
		static UCreateSessionCallbackProxyAdvanced* CreateAdvancedSession(UObject* WorldContextObject, const TArray<FSessionPropertyKeyPair>& ExtraSettings, class APlayerController* PlayerController = NULL, int32 PublicConnections = 100, int32 PrivateConnections = 0, bool bUseLAN = false, bool bAllowInvites = true, bool bIsDedicatedServer = false,  bool bUseLobbiesIfAvailable = true, bool bAllowJoinViaPresence = true, bool bAllowJoinViaPresenceFriendsOnly = false, bool bAntiCheatProtected = false, bool bUsesStats = false, bool bShouldAdvertise = true, bool bUseLobbiesVoiceChatIfAvailable = false, bool bStartAfterCreate = true);

	virtual void Activate() override;

private:

	void OnCreateCompleted(FName SessionName, bool bWasSuccessful);

	void OnStartCompleted(FName SessionName, bool bWasSuccessful);

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FOnCreateSessionCompleteDelegate CreateCompleteDelegate;

	FOnStartSessionCompleteDelegate StartCompleteDelegate;

	FDelegateHandle CreateCompleteDelegateHandle;
	FDelegateHandle StartCompleteDelegateHandle;

	int NumPublicConnections;

	int NumPrivateConnections;

	bool bUseLAN;

	bool bAllowInvites;

	bool bDedicatedServer;

	bool bUsePresence;

	bool bUseLobbiesIfAvailable;

	bool bAllowJoinViaPresence;

	bool bAllowJoinViaPresenceFriendsOnly;

	bool bAntiCheatProtected;

	bool bUsesStats;

	bool bShouldAdvertise;

	bool bUseLobbiesVoiceChatIfAvailable;

	bool bStartAfterCreate;

	TArray<FSessionPropertyKeyPair> ExtraSettings;

	TWeakObjectPtr<UObject> WorldContextObject;
};

