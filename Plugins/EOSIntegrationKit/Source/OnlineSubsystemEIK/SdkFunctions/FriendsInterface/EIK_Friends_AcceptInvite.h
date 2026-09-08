

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
THIRD_PARTY_INCLUDES_START
#include "eos_friends.h"
#include "eos_friends_types.h"
THIRD_PARTY_INCLUDES_END
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Friends_AcceptInvite.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEIK_Friends_AcceptInviteCallback, const TEnumAsByte<EEIK_Result>&, Result, const FEIK_EpicAccountId&, LocalUserId, const FEIK_EpicAccountId&, TargetUserId);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Friends_AcceptInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | Friends Interface", DisplayName="EOS_Friends_AcceptInvite")
	static UEIK_Friends_AcceptInvite* EIK_Friends_AcceptInvite(FEIK_EpicAccountId LocalUserId, FEIK_EpicAccountId TargetUserId);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit | Friends Interface")
	FEIK_Friends_AcceptInviteCallback OnCallback;

private:
	static void EOS_CALL OnAcceptInviteCallback(const EOS_Friends_AcceptInviteCallbackInfo* Data);
	virtual void Activate() override;
	FEIK_EpicAccountId Var_LocalUserId;
	FEIK_EpicAccountId Var_TargetUserId;

};
