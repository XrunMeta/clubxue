

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEIK/SdkFunctions/ConnectInterface/EIK_ConnectSubsystem.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_LobbySearch_Find.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEIKLobbySearchFind, const TEnumAsByte<EEIK_Result>&, ResultCode);
UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_LobbySearch_Find : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_Find")
	static UEIK_LobbySearch_Find* EIK_LobbySearch_Find(FEIK_HLobbySearch Handle, FEIK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable)
	FEIKLobbySearchFind OnCallback;

private:
	virtual void Activate() override;
	static void EOS_CALL OnFindComplete(const EOS_LobbySearch_FindCallbackInfo* Data);
	FEIK_ProductUserId Var_LocalUserId;
	FEIK_HLobbySearch Var_Handle;
};
