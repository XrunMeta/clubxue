
#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "GetUserPrivilegeCallbackProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlueprintGetUserPrivilegeDelegate, EBPUserPrivileges, QueriedPrivilege, bool, HadPrivilege);

UCLASS(MinimalAPI)
class UGetUserPrivilegeCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FBlueprintGetUserPrivilegeDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|AdvancedIdentity")
	static UGetUserPrivilegeCallbackProxy* GetUserPrivilege(UObject* WorldContextObject, const EBPUserPrivileges & PrivilegeToCheck, const FBPUniqueNetId & PlayerUniqueNetID);

	virtual void Activate() override;

private:

	void OnCompleted(const FUniqueNetId& PlayerID, EUserPrivileges::Type Privilege, uint32 Result);

private:

	FBPUniqueNetId PlayerUniqueNetID;

	EBPUserPrivileges UserPrivilege;

	TWeakObjectPtr<UObject> WorldContextObject;
};
