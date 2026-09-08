

#pragma once 

#include "SocketSubsystemEIK.h"

class FOnlineSubsystemEOS;

class FSocketSubsystemEOSUtils_OnlineSubsystemEOS : public ISocketSubsystemEOSUtils
{
public:
	FSocketSubsystemEOSUtils_OnlineSubsystemEOS(FOnlineSubsystemEOS& InSubsystemEOS);
	virtual ~FSocketSubsystemEOSUtils_OnlineSubsystemEOS() override;

	virtual EOS_ProductUserId GetLocalUserId() override;
	virtual FString GetSessionId() override;
	virtual FName GetSubsystemInstanceName() override;

private:
	FOnlineSubsystemEOS& SubsystemEOS;
};