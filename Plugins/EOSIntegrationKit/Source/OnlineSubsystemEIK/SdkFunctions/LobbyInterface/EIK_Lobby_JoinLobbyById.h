

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Lobby_JoinLobbyById.generated.h"

USTRUCT(BlueprintType)
struct FEIK_Lobby_JoinLobbyByIdOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_LobbyId LobbyId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_ProductUserId LocalUserId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bPresenceEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_Lobby_LocalRTCOptions LobbyRTCOptions;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bCrossplayOptOut;

	FEIK_Lobby_JoinLobbyByIdOptions()
	{
		bPresenceEnabled = false;
		bCrossplayOptOut = false;
	}
	EOS_Lobby_JoinLobbyByIdOptions ToEOSOptions(EOS_Lobby_LocalRTCOptions* LocalRTCOptions = nullptr)
	{
		EOS_Lobby_JoinLobbyByIdOptions Options;
		Options.ApiVersion = EOS_LOBBY_JOINLOBBYBYID_API_LATEST;
		Options.LobbyId = LobbyId.Ref;
		Options.LocalUserId = LocalUserId.GetValueAsEosType();
		Options.bPresenceEnabled = bPresenceEnabled ? EOS_TRUE : EOS_FALSE;
		Options.LocalRTCOptions = LocalRTCOptions;
		Options.bCrossplayOptOut = bCrossplayOptOut ? EOS_TRUE : EOS_FALSE;
		return Options;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEIK_Lobby_JoinLobbyByIdDelegate, const TEnumAsByte<EEIK_Result>&, Result, const FEIK_LobbyId&, LobbyId);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Lobby_JoinLobbyById : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_JoinLobbyById")
	static UEIK_Lobby_JoinLobbyById* EIK_Lobby_JoinLobbyById(FEIK_Lobby_JoinLobbyByIdOptions Options);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_Lobby_JoinLobbyByIdDelegate OnCallback;
private:
	static void EOS_CALL OnJoinLobbyByIdComplete(const EOS_Lobby_JoinLobbyByIdCallbackInfo* Data);
	virtual void Activate() override;
	FEIK_Lobby_JoinLobbyByIdOptions Var_Options;
};
