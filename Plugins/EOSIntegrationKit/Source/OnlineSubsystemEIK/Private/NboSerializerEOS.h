

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOSTypes.h"
#if ENGINE_MAJOR_VERSION == 5
#include "NboSerializerOSS.h"
#else
#include "NboSerializer.h"
#endif

#if WITH_EOS_SDK

#if ENGINE_MAJOR_VERSION == 5
class FNboSerializeToBufferEOS : public FNboSerializeToBufferOSS
#else
class FNboSerializeToBufferEOS : public FNboSerializeToBuffer
#endif
{
public:

	FNboSerializeToBufferEOS() :
#if ENGINE_MAJOR_VERSION == 5
	FNboSerializeToBufferOSS(512)
	{
	}

	FNboSerializeToBufferEOS(uint32 Size) :
		FNboSerializeToBufferOSS(Size)
	{
	}
#else
		FNboSerializeToBuffer(512)
	{
	}

	FNboSerializeToBufferEOS(uint32 Size) :
		FNboSerializeToBuffer(Size)
	{
	}
#endif

 	friend inline FNboSerializeToBufferEOS& operator<<(FNboSerializeToBufferEOS& Ar, const FOnlineSessionInfoEOS& SessionInfo)
 	{
		check(SessionInfo.HostAddr.IsValid());

		Ar << *SessionInfo.SessionId;
		((FNboSerializeToBuffer&)Ar) << *SessionInfo.HostAddr;
		return Ar;
 	}

	friend inline FNboSerializeToBufferEOS& operator<<(FNboSerializeToBufferEOS& Ar, const FUniqueNetIdString& UniqueId)
	{
		((FNboSerializeToBuffer&)Ar) << UniqueId.UniqueNetIdStr;
		return Ar;
	}
};

#if ENGINE_MAJOR_VERSION == 5
class FNboSerializeFromBufferEOS : public FNboSerializeFromBufferOSS
#else
class FNboSerializeFromBufferEOS : public FNboSerializeFromBuffer
#endif
{
public:

	FNboSerializeFromBufferEOS(uint8* Packet,int32 Length) :
#if ENGINE_MAJOR_VERSION == 5
	FNboSerializeFromBufferOSS(Packet,Length)
#else
		FNboSerializeFromBuffer(Packet,Length)
#endif
	{
	}

 	friend inline FNboSerializeFromBufferEOS& operator>>(FNboSerializeFromBufferEOS& Ar, FOnlineSessionInfoEOS& SessionInfo)
 	{
		check(SessionInfo.HostAddr.IsValid());

		FString SessionId;
		Ar >> SessionId;

		Ar >> *SessionInfo.HostAddr;

		SessionInfo.SessionId = FUniqueNetIdString::Create(MoveTemp(SessionId), FName("EOS"));

		return Ar;
 	}

	friend inline FNboSerializeFromBufferEOS& operator>>(FNboSerializeFromBufferEOS& Ar, FUniqueNetIdString& UniqueId)
	{
		Ar >> UniqueId.UniqueNetIdStr;
		return Ar;
	}
};

#endif
