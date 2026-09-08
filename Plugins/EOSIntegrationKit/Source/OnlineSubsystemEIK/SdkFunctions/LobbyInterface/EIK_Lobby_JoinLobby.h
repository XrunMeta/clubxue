

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Lobby_JoinLobby.generated.h"

USTRUCT(BlueprintType)
struct FEIK_Lobby_JoinLobbyOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_HLobbyDetails LobbyDetailsHandle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_ProductUserId LocalUserId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bPresenceEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_Lobby_LocalRTCOptions LobbyRTCOptions;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bCrossplayOptOut;

	FEIK_Lobby_JoinLobbyOptions()
	{
		bPresenceEnabled = false;
		bCrossplayOptOut = false;
	}
	EOS_Lobby_JoinLobbyOptions ToEOSOptions(EOS_Lobby_LocalRTCOptions* LocalRTCOptions = nullptr)
	{
		EOS_Lobby_JoinLobbyOptions Options;
		Options.ApiVersion = EOS_LOBBY_JOINLOBBY_API_LATEST;
		Options.LobbyDetailsHandle = LobbyDetailsHandle.Ref;
		Options.LocalUserId = LocalUserId.GetValueAsEosType();
		Options.bPresenceEnabled = bPresenceEnabled ? EOS_TRUE : EOS_FALSE;
		Options.LocalRTCOptions = LocalRTCOptions;
		Options.bCrossplayOptOut = bCrossplayOptOut ? EOS_TRUE : EOS_FALSE;
		return Options;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEIK_Lobby_JoinLobbyDelegate, const TEnumAsByte<EEIK_Result>&, Result, const FEIK_LobbyId&, LobbyId);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Lobby_JoinLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_JoinLobby")
	static UEIK_Lobby_JoinLobby* EIK_Lobby_JoinLobby(FEIK_Lobby_JoinLobbyOptions Options);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_Lobby_JoinLobbyDelegate OnCallback;

private:
	static void EOS_CALL OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data);
	virtual void Activate() override;
	FEIK_Lobby_JoinLobbyOptions Var_Options;
};
