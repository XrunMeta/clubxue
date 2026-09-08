

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_sanctions.h"
#include "eos_sanctions_types.h"
THIRD_PARTY_INCLUDES_END
#include "EIK_SanctionsSubsystem.generated.h"

UCLASS(DisplayName="Sanctions Interface", meta=(DisplayName="Sanctions Interface"))
class ONLINESUBSYSTEMEIK_API UEIK_SanctionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sanctions Interface", DisplayName="EOS_Sanctions_CopyPlayerSanctionByIndex")
	static TEnumAsByte<EEIK_Result> EIK_Sanctions_CopyPlayerSanctionByIndex(FEIK_ProductUserId LocalUserId, int32 Index, FEIK_Sanctions_PlayerSanction& OutSanction);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Sanctions Interface", DisplayName="EOS_Sanctions_GetPlayerSanctionCount")
	static int32 EIK_Sanctions_GetPlayerSanctionCount(FEIK_ProductUserId LocalUserId);

};
