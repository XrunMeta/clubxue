

#pragma once

#include "CoreMinimal.h"
#include "eos_userinfo.h"
#include "eos_userinfo_types.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemEIK/Subsystem/EIK_Subsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EIK_FindUserByDisplayName_Async.generated.h"

USTRUCT(BlueprintType, Category = "EOS Integration Kit|UserInfo")
struct FEIKUserInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit|UserInfo")
    FString EpicAccountID;

    UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit|UserInfo")
    FString Country;

    UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit|UserInfo")
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit|UserInfo")
    FString PreferredLanguage;

    UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit|UserInfo")
    FString Nickname;

    UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit|UserInfo")
    FString DisplayNameSanitized;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindUserByDisplayNameDelegate, const FEIKUserInfo, EIKUserInfo);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_FindUserByDisplayName_Async : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Find EIK Player By Display Name", meta = (BlueprintInternalUseOnly = "true"), Category = "EOS Integration Kit || UserInfo")
	static UEIK_FindUserByDisplayName_Async* FindEIkUserByDisplayName(FString TargetDisplayName, FString LocalEpicID);

	FString TargetDisplayName;

	FString LocalEpicID;

	void FindUserByDisplayName();

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit || UserInfo")
    FFindUserByDisplayNameDelegate Success;

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit || UserInfo")
    FFindUserByDisplayNameDelegate Failure;

	static void EOS_CALL FindUserByDisplayNameCallback(const EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo* Data);

	void ResultFaliure();

	void ResultSuccess(const FEIKUserInfo UserInfoStruct);

	void Activate() override;
};
