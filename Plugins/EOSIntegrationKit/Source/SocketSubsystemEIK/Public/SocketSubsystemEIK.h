

#pragma once

#include "CoreMinimal.h"
#include "SocketSubsystem.h"
#include "IEOSSDKManager.h"
#include "SocketSubsystemEIKUtils.h"

#if WITH_EOS_SDK
	#if defined(EOS_PLATFORM_BASE_FILE_NAME)
	#include EOS_PLATFORM_BASE_FILE_NAME
	#endif

	#include "eos_p2p_types.h"
#endif

#ifndef EOS_SOCKETSUBSYSTEM
#define EOS_SOCKETSUBSYSTEM FName(TEXT("EOS"))
#endif

class UNetConnectionEIK;
class FSocket;
class FResolveInfoCached;
class FResolveInfo;
class FInternetAddr;
class FInternetAddrEOS;
class FSocketEOS;

typedef TSet<uint8> FChannelSet;

class SOCKETSUBSYSTEMEIK_API FSocketSubsystemEIK
	: public ISocketSubsystem
{
public:
	FSocketSubsystemEIK(IEIKPlatformHandlePtr InPlatformHandle, ISocketSubsystemEOSUtilsPtr InUtils);
	virtual ~FSocketSubsystemEIK();

	virtual bool Init(FString& Error) override;
	virtual void Shutdown() override;
	virtual FSocket* CreateSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolType) override;
	virtual FResolveInfoCached* CreateResolveInfoCached(TSharedPtr<FInternetAddr> Addr) const override;
	virtual void DestroySocket(FSocket* Socket) override;
	virtual FAddressInfoResult GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName = nullptr, EAddressInfoFlags QueryFlags = EAddressInfoFlags::Default, const FName ProtocolTypeName = NAME_None, ESocketType SocketType = ESocketType::SOCKTYPE_Unknown) override;
	virtual bool RequiresChatDataBeSeparate() override;
	virtual bool RequiresEncryptedPackets() override;
	virtual bool GetHostName(FString& HostName) override;
	virtual TSharedRef<FInternetAddr> CreateInternetAddr() override;
	virtual TSharedPtr<FInternetAddr> GetAddressFromString(const FString& InString) override;
	virtual bool HasNetworkDevice() override;
	virtual const TCHAR* GetSocketAPIName() const override;
	virtual ESocketErrors GetLastErrorCode() override;
	virtual ESocketErrors TranslateErrorCode(int32 Code) override;
	virtual bool GetLocalAdapterAddresses(TArray<TSharedPtr<FInternetAddr>>& OutAdresses) override;
	virtual TSharedRef<FInternetAddr> GetLocalBindAddr(FOutputDevice& Out) override;
	virtual TArray<TSharedRef<FInternetAddr>> GetLocalBindAddresses() override;
	TSharedRef<FInternetAddr> GetLocalBindAddr(const UWorld* const OwningWorld, FOutputDevice& Out);
	virtual bool IsSocketWaitSupported() const override;

	void SetLastSocketError(const ESocketErrors NewSocketError);

#if WITH_EOS_SDK
	EOS_HP2P GetP2PHandle();
	EOS_ProductUserId GetLocalUserId();
#endif

	bool BindChannel(const FInternetAddrEOS& Address);

	bool UnbindChannel(const FInternetAddrEOS& Address);

	FSocketSubsystemEIK* GetSocketSubsystemForWorld(UWorld* InWorld);

private:

	void RemoveFromStaticContainers();

private:
#if WITH_EOS_SDK
	EOS_HP2P P2PHandle;
#endif
	ISocketSubsystemEOSUtilsPtr Utils;

	TArray<TUniquePtr<FSocketEOS>> TrackedSockets;

	TMap<FString, FChannelSet> BoundAddresses;

	ESocketErrors LastSocketError;

	static TArray<FSocketSubsystemEIK*> SocketSubsystemEOSInstances;

	static TMap<UWorld*, FSocketSubsystemEIK*> SocketSubsystemEOSPerWorldMap;
};
