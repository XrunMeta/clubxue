

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "eos_p2p.h"
#include "Runtime/Launch/Resources/Version.h"
#include "eos_p2p_types.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_P2PSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FEIK_P2P_OnIncomingPacketQueueFullInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | P2P Interface")
	int64 PacketQueueMaxSizeBytes;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | P2P Interface")
	int64 PacketQueueCurrentSizeBytes;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | P2P Interface")
	FEIK_ProductUserId OverflowPacketLocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | P2P Interface")
	int32 OverflowPacketChannel;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | P2P Interface")
	int32 OverflowPacketSizeBytes;

	FEIK_P2P_OnIncomingPacketQueueFullInfo(): PacketQueueMaxSizeBytes(0), PacketQueueCurrentSizeBytes(0),
	                                          OverflowPacketChannel(0),
	                                          OverflowPacketSizeBytes(0)
	{
	}

	FEIK_P2P_OnIncomingPacketQueueFullInfo(const EOS_P2P_OnIncomingPacketQueueFullInfo& Data)
	{
		PacketQueueMaxSizeBytes = Data.PacketQueueMaxSizeBytes;
		PacketQueueCurrentSizeBytes = Data.PacketQueueCurrentSizeBytes;
		OverflowPacketLocalUserId = Data.OverflowPacketLocalUserId;
		OverflowPacketChannel = Data.OverflowPacketChannel;
		OverflowPacketSizeBytes = Data.OverflowPacketSizeBytes;
	}
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FEIK_OnIncomingPacketQueueFull, const FEIK_P2P_OnIncomingPacketQueueFullInfo&, Data);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_OnPeerConnectionClosed, const FEIK_ProductUserId&, LocalUserId, const FEIK_ProductUserId&, RemoteUserId, const FEIK_P2P_SocketId&, SocketId, const TEnumAsByte<EEIK_EConnectionClosedReason>&, Reason);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEIK_OnPeerConnectionEstablished, const FEIK_ProductUserId&, LocalUserId, const FEIK_ProductUserId&, RemoteUserId, const FEIK_P2P_SocketId&, SocketId, const TEnumAsByte<EEIK_EConnectionEstablishedType>&, ConnectionType, const TEnumAsByte<EEIK_ENetworkConnectionType>&, NetworkType);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_OnPeerConnectionInterrupted, const FEIK_ProductUserId&, LocalUserId, const FEIK_ProductUserId&, RemoteUserId, const FEIK_P2P_SocketId&, SocketId);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_OnIncomingConnectionRequest, const FEIK_ProductUserId&, LocalUserId, const FEIK_ProductUserId&, RemoteUserId, const FEIK_P2P_SocketId&, SocketId);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_P2PSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_AcceptConnection")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_AcceptConnection(FEIK_ProductUserId LocalUserId, FEIK_ProductUserId RemoteUserId, FEIK_P2P_SocketId SocketId);

	FEIK_OnIncomingPacketQueueFull OnIncomingPacketQueueFull;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_AddNotifyIncomingPacketQueueFull")
	const FEIK_NotificationId EIK_P2P_AddNotifyIncomingPacketQueueFull(FEIK_ProductUserId LocalUserId, const FEIK_P2P_SocketId SocketId, const FEIK_OnIncomingPacketQueueFull& Callback);

	FEIK_OnPeerConnectionClosed OnPeerConnectionClosed;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_AddNotifyPeerConnectionClosed")
	const FEIK_NotificationId EIK_P2P_AddNotifyPeerConnectionClosed(FEIK_ProductUserId LocalUserId, const FEIK_P2P_SocketId SocketId, const FEIK_OnPeerConnectionClosed& Callback);

	FEIK_OnPeerConnectionEstablished OnPeerConnectionEstablished;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_AddNotifyPeerConnectionEstablished")
	const FEIK_NotificationId EIK_P2P_AddNotifyPeerConnectionEstablished(FEIK_ProductUserId LocalUserId, const FEIK_P2P_SocketId SocketId, const FEIK_OnPeerConnectionEstablished& Callback);

	FEIK_OnPeerConnectionInterrupted OnPeerConnectionInterrupted;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_AddNotifyPeerConnectionInterrupted")
	const FEIK_NotificationId EIK_P2P_AddNotifyPeerConnectionInterrupted(FEIK_ProductUserId LocalUserId, const FEIK_P2P_SocketId SocketId, const FEIK_OnPeerConnectionInterrupted& Callback);

	FEIK_OnIncomingConnectionRequest OnIncomingConnectionRequest;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_AddNotifyPeerConnectionRequest")
	const FEIK_NotificationId EIK_P2P_AddNotifyPeerConnectionRequest(FEIK_ProductUserId LocalUserId, const FEIK_P2P_SocketId SocketId, const FEIK_OnIncomingConnectionRequest& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_ClearPacketQueue")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_ClearPacketQueue(FEIK_ProductUserId LocalUserId, FEIK_ProductUserId RemoteUserId, FEIK_P2P_SocketId SocketId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_CloseConnection")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_CloseConnection(FEIK_ProductUserId LocalUserId, FEIK_ProductUserId RemoteUserId, FEIK_P2P_SocketId SocketId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_CloseConnections")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_CloseConnections(FEIK_ProductUserId LocalUserId, FEIK_P2P_SocketId SocketId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_GetNATType")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_GetNATType(TEnumAsByte<EEIK_ENATType>& OutNATType);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_GetNextReceivedPacketSize")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_GetNextReceivedPacketSize(FEIK_ProductUserId LocalUserId, int32 Channel, int32& OutPacketSizeBytes);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_GetPacketQueueInfo")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_GetPacketQueueInfo(FEIK_P2P_PacketQueueInfo& OutPacketQueueInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_GetPortRange")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_GetPortRange(int32& OutPort, int32& OutNumAdditionalPortsToTry);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_GetRelayControl")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_GetRelayControl(TEnumAsByte<EEIK_ERelayControl>& OutControl);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_ReceivePacket")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_ReceivePacket(FEIK_ProductUserId LocalUserId, int32 MaxDataSizeBytes, int32 RequestedChannel, FEIK_P2P_SocketId& OutSocketId, FEIK_ProductUserId& OutPeerId, int32& OutChannel, TArray<uint8>& OutData, int32& OutBytesRead);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_RemoveNotifyIncomingPacketQueueFull")
	static void EIK_P2P_RemoveNotifyIncomingPacketQueueFull(const FEIK_NotificationId& NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_RemoveNotifyPeerConnectionClosed")
	static void EIK_P2P_RemoveNotifyPeerConnectionClosed(const FEIK_NotificationId& NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_RemoveNotifyPeerConnectionEstablished")
	static void EIK_P2P_RemoveNotifyPeerConnectionEstablished(const FEIK_NotificationId& NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_RemoveNotifyPeerConnectionInterrupted")
	static void EIK_P2P_RemoveNotifyPeerConnectionInterrupted(const FEIK_NotificationId& NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_RemoveNotifyPeerConnectionRequest")
	static void EIK_P2P_RemoveNotifyPeerConnectionRequest(const FEIK_NotificationId& NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_SendPacket")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_SendPacket(FEIK_ProductUserId LocalUserId, FEIK_ProductUserId RemoteUserId, FEIK_P2P_SocketId SocketId, int32 Channel, const TArray<uint8>& Data, bool bAllowDelayedDelivery, bool bDisableAutoAcceptConnection, const TEnumAsByte<EEIK_EPacketReliability>& Reliability);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_SetPacketQueueSize")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_SetPacketQueueSize(int64 IncomingPacketQueueMaxSizeBytes, int64 OutgoingPacketQueueMaxSizeBytes);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_SetPortRange")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_SetPortRange(int32 Port, int32 MaxAdditionalPortsToTry);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | P2P Interface", DisplayName="EOS_P2P_SetRelayControl")
	static const TEnumAsByte<EEIK_Result> EIK_P2P_SetRelayControl(TEnumAsByte<EEIK_ERelayControl> Control);
};
