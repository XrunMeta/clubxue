

#include "NetDriverEIKBase.h"
#include "OnlineBeaconHost.h"
#include "OnlineBeaconClient.h"
#include "EngineUtils.h"
#include "NetConnectionEIK.h"
#include "SocketEIK.h"
#include "SocketSubsystemEIK.h"
#include "Misc/EngineVersionComparison.h"
#include "EOSSharedTypes.h"
#include "Engine/Engine.h"
#include "Misc/ConfigCacheIni.h"

#if ENGINE_MAJOR_VERSION >= 5
#include UE_INLINE_GENERATED_CPP_BY_NAME(NetDriverEIKBase)
#endif

UNetDriverEIKBase::UNetDriverEIKBase(const FObjectInitializer& ObjectInitializer)
	: UIpNetDriver(ObjectInitializer)
{
	bIsPassthrough = false;

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6

	bool bUnused;
	if (GConfig->GetBool(TEXT("/Script/SocketSubsystemEIK.NetDriverEIKBase"), TEXT("bIsUsingP2PSockets"), bUnused, GEngineIni))
	{
		UE_LOG(LogTemp, Warning, TEXT("EIK NetDriver: bIsUsingP2PSockets is deprecated, please remove any related config values"));
	}
#else

	if (!GConfig->GetBool(TEXT("/Script/SocketSubsystemEIK.NetDriverEIKBase"), TEXT("bIsUsingP2PSockets"), bIsUsingP2PSockets, GEngineIni))
	{
		bIsUsingP2PSockets = true; 
	}
#endif
}

bool UNetDriverEIKBase::IsAvailable() const
{

	if (IsRunningDedicatedServer())
	{
		return false;
	}

	if (ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(EOS_SOCKETSUBSYSTEM))
	{
		return true;
	}

	return false;
}

bool UNetDriverEIKBase::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (bIsPassthrough)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Running as pass-through"));
		return Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error);
	}
	if (!UNetDriver::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to init driver base"));
		return false;
	}

	FSocketSubsystemEIK* const SocketSubsystem = static_cast<FSocketSubsystemEIK*>(GetSocketSubsystem());
	if (!SocketSubsystem)
	{
		if(!GetSocketSubsystem())
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not get socket subsystem that is the base of EOS"));
		}
		UE_LOG(LogTemp, Warning, TEXT("Could not get socket subsystem"));
		return false;
	}

	const UWorld* const MyWorld = FindWorld();

	TSharedRef<FInternetAddr> LocalAddress = SocketSubsystem->GetLocalBindAddr(MyWorld, *GLog);
	if (!LocalAddress->IsValid())
	{

		Error = TEXT("Could not bind local address");
		UE_LOG(LogTemp, Warning, TEXT("Could not bind local address"));
		return false;
	}

	FUniqueSocket NewSocket = SocketSubsystem->CreateUniqueSocket(NAME_DGram, TEXT("UE4"), NAME_None);
	TSharedPtr<FSocket> SharedSocket(NewSocket.Release(), FSocketDeleter(NewSocket.GetDeleter()));

	SetSocketAndLocalAddress(SharedSocket);

	if (GetSocket() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not create socket"));
		return false;
	}

	TSharedRef<FInternetAddrEOS> EOSLocalAddress = StaticCastSharedRef<FInternetAddrEOS>(LocalAddress);
	if(IsBeaconDriver())
	{

		EOSLocalAddress->SetSocketName(TEXT("BeaconSession"));

		EOSLocalAddress->SetChannel(71);
	}
	else
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6

		FString NetDriverDefinitionStr = GetNetDriverDefinition().ToString();
		EOSLocalAddress->SetChannel(GetTypeHash(NetDriverDefinitionStr));
		EOSLocalAddress->SetSocketName(NetDriverDefinitionStr);
#else

		EOSLocalAddress->SetChannel(GetTypeHash(NetDriverName.ToString()));
		EOSLocalAddress->SetSocketName(NetDriverName.ToString());
#endif
	}

	static_cast<FSocketEOS*>(GetSocket())->SetLocalAddress(*EOSLocalAddress);

	LocalAddr = LocalAddress;

	return true;
}

bool UNetDriverEIKBase::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{

	bool bIsEOSURL = ConnectURL.Host.StartsWith(EOS_CONNECTION_URL_PREFIX, ESearchCase::IgnoreCase);
	bool bIsAvailableResult = IsAvailable();

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6

	if (!bIsAvailableResult || !bIsEOSURL)
#else

	if (!bIsUsingP2PSockets || !bIsAvailableResult || !bIsEOSURL)
#endif
	{

		bIsPassthrough = true;
		return Super::InitConnect(InNotify, ConnectURL, Error);
	}

	bool bIsValid = false;
	TSharedRef<FInternetAddrEOS> RemoteHost = MakeShared<FInternetAddrEOS>();
	RemoteHost->SetIp(*ConnectURL.Host, bIsValid);
	if (!bIsValid || ConnectURL.Port < 0)
	{
		Error = TEXT("Invalid remote address");
		UE_LOG(LogTemp, Warning, TEXT("Invalid Remote Address. ConnectUrl = (%s)"), *ConnectURL.ToString());
		return false;
	}

	if (!InitBase(true, InNotify, ConnectURL, false, Error))
	{
		return false;
	}

	LocalAddr = RemoteHost;

	FSocket* CurSocket = GetSocket();

	FSocketSubsystemEIK* const SocketSubsystem = static_cast<FSocketSubsystemEIK*>(GetSocketSubsystem());
	check(SocketSubsystem);
	if (!SocketSubsystem->BindNextPort(CurSocket, *LocalAddr, MaxPortCountToTry + 1, 1))
	{

		Error = TEXT("Could not bind local port");
		UE_LOG(LogTemp, Warning, TEXT("Could not bind local port in %d attempts"), MaxPortCountToTry);
		return false;
	}

	UNetConnectionEIK* Connection = NewObject<UNetConnectionEIK>(NetConnectionClass);
	check(Connection);

	ServerConnection = Connection;
	Connection->InitLocalConnection(this, CurSocket, ConnectURL, USOCK_Pending);

	CreateInitialClientChannels();

	return true;
}

bool UNetDriverEIKBase::InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error)
{

	bool bIsAvailableResult = IsAvailable();
	bool bHasLanMatch = LocalURL.HasOption(TEXT("bIsLanMatch"));
	bool bUseIPSockets = LocalURL.HasOption(TEXT("bUseIPSockets"));

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6

	if (!bIsAvailableResult || bHasLanMatch || bUseIPSockets)
#else

	if (!bIsUsingP2PSockets || !bIsAvailableResult || bHasLanMatch || bUseIPSockets)
#endif
	{

		bIsPassthrough = true;
		return Super::InitListen(InNotify, LocalURL, bReuseAddressAndPort, Error);
	}

	if (!InitBase(false, InNotify, LocalURL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	FSocket* CurSocket = GetSocket();
	if (!CurSocket->Listen(0))
	{
		Error = TEXT("Could not listen");
		UE_LOG(LogTemp, Warning, TEXT("Could not listen on socket"));
		return false;
	}

	InitConnectionlessHandler();

	UE_LOG(LogTemp, Verbose, TEXT("Initialized as an EOSP2P listen server"));

	UWorld* TWorld = FindWorld();
	if (!TWorld)
	{
		Error = TEXT("Invalid world context");
		return false;
	}

	bool bBeaconHostExists = false;
    for (TActorIterator<AOnlineBeaconHost> It(TWorld); It; ++It)
	{
		if (*It)
		{
			bBeaconHostExists = true;
			break;
		}
	}

	return true;
}

ISocketSubsystem* UNetDriverEIKBase::GetSocketSubsystem()
{
	if (bIsPassthrough)
	{
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	}
	UWorld* CurrentWorld = FindWorld();
	FSocketSubsystemEIK* DefaultSocketSubsystem = static_cast<FSocketSubsystemEIK*>(ISocketSubsystem::Get(EOS_SOCKETSUBSYSTEM));
	return DefaultSocketSubsystem->GetSocketSubsystemForWorld(CurrentWorld);
}

void UNetDriverEIKBase::Shutdown()
{
	Super::Shutdown();

	if (!bIsPassthrough)
	{
		if(ServerConnection)
		{
			if (UNetConnectionEIK* const EOSServerConnection = Cast<UNetConnectionEIK>(ServerConnection))
			{
				EOSServerConnection->DestroyEOSConnection();
			}
		}
		for (UNetConnection* Client : ClientConnections)
		{
			if(Client)
			{
				if (UNetConnectionEIK* const EOSClient = Cast<UNetConnectionEIK>(Client))
				{
					EOSClient->DestroyEOSConnection();
				}
			}
		}
	}
}

int UNetDriverEIKBase::GetClientPort()
{
	if (bIsPassthrough)
	{
		return Super::GetClientPort();
	}

	return 49152;
}

bool UNetDriverEIKBase::IsBeaconDriver() const
{
	if (!GEngine) return false;

	for (const auto &WorldContext : GEngine->GetWorldContexts())
	{
		if (UWorld *ItWorld = WorldContext.World())
		{
			for (AOnlineBeacon* Beacon : TActorRange<AOnlineBeacon>(ItWorld))
			{
				if (Beacon->GetNetDriver() == this)
				{
					return true;
				}
			}
		}
	}
	return false;
}

UWorld* UNetDriverEIKBase::FindWorld() const
{
	UWorld* MyWorld = GetWorld();

	if (!MyWorld && GEngine)
	{
		if (FWorldContext* WorldContext = GEngine->GetWorldContextFromPendingNetGameNetDriver(this))
		{
			MyWorld = WorldContext->World();
		}
	}

	if(!MyWorld)
	{
		if (GEngine != nullptr)
		{
			for (const auto &WorldContext : GEngine->GetWorldContexts())
			{
				UWorld *ItWorld = WorldContext.World();
				if (ItWorld != nullptr)
				{
					for (TActorIterator<AOnlineBeacon> It(ItWorld); It; ++It)
					{
						if (It->GetNetDriver() == this)
						{
							return ItWorld;
						}
					}
				}
			}
		}
	}

	return MyWorld;
}

