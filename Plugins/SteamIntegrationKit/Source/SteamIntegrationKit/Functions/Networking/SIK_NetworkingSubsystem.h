

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SIK_NetworkingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnP2PSessionConnectFail, FSIK_SteamId, SteamID, TEnumAsByte<ESIK_P2PSessionError>, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnP2PSessionRequest, FSIK_SteamId, SteamID);

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_NetworkingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	USIK_NetworkingSubsystem();
	~USIK_NetworkingSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit || SDK Functions || Networking")
	FOnP2PSessionConnectFail OnP2PSessionConnectFail;

	UPROPERTY(BlueprintAssignable, Category = "Steam Integration Kit || SDK Functions || Networking")
	FOnP2PSessionRequest OnP2PSessionRequest;

private:
#if (WITH_ENGINE_STEAM && ONLINESUBSYSTEMSTEAM_PACKAGE) || (WITH_STEAMKIT && !WITH_ENGINE_STEAM)
	STEAM_CALLBACK_MANUAL(USIK_NetworkingSubsystem, OnP2PSessionConnectFailCallbck, P2PSessionConnectFail_t, m_CallbackP2PSessionConnectFail);
	STEAM_CALLBACK_MANUAL(USIK_NetworkingSubsystem, OnP2PSessionRequestCallback, P2PSessionRequest_t, m_CallbackP2PSessionRequest);
#endif
};
