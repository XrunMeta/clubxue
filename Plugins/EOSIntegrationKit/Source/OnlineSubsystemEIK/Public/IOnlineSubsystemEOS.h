

#pragma once

#include "OnlineSubsystemImpl.h"

class FUniqueNetId;
class IVoiceChatUser;
using IEIKPlatformHandlePtr = TSharedPtr<class IEIKPlatformHandle, ESPMode::ThreadSafe>;

class ONLINESUBSYSTEMEIK_API IOnlineSubsystemEOS : 
	public FOnlineSubsystemImpl
{
public:
	IOnlineSubsystemEOS(FName InSubsystemName, FName InInstanceName) : FOnlineSubsystemImpl(InSubsystemName, InInstanceName) {}
	virtual ~IOnlineSubsystemEOS() = default;

	virtual IVoiceChatUser* GetVoiceChatUserInterface(const FUniqueNetId& LocalUserId) = 0;
	virtual IEIKPlatformHandlePtr GetEOSPlatformHandle() const = 0;
};