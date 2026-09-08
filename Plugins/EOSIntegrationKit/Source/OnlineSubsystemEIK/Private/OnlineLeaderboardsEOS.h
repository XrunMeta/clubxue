

#pragma once

#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OnlineSubsystemEOSTypes.h"

class FOnlineSubsystemEOS;

#if WITH_EOS_SDK
#include "eos_leaderboards_types.h"

#define EOS_MAX_NUM_RANKINGS 1000

class FOnlineLeaderboardsEOS
	: public IOnlineLeaderboards
	, public TSharedFromThis<FOnlineLeaderboardsEOS, ESPMode::ThreadSafe>
{
public:
	FOnlineLeaderboardsEOS() = delete;
	virtual ~FOnlineLeaderboardsEOS() = default;

	virtual bool ReadLeaderboards(const TArray< FUniqueNetIdRef >& Players, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual bool ReadLeaderboardsForFriends(int32 LocalUserNum, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual bool ReadLeaderboardsAroundRank(int32 Rank, uint32 Range, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual bool ReadLeaderboardsAroundUser(FUniqueNetIdRef Player, uint32 Range, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual void FreeStats(FOnlineLeaderboardRead& ReadObject) override;
	virtual bool WriteLeaderboards(const FName& SessionName, const FUniqueNetId& Player, FOnlineLeaderboardWrite& WriteObject) override;
	virtual bool FlushLeaderboards(const FName& SessionName) override;
	virtual bool WriteOnlinePlayerRatings(const FName& SessionName, int32 LeaderboardId, const TArray<FOnlinePlayerScore>& PlayerScores) override;

	FOnlineLeaderboardsEOS(FOnlineSubsystemEOS* InSubsystem)
		: EOSSubsystem(InSubsystem)
	{
	}

private:

	FOnlineSubsystemEOS* EOSSubsystem;
};

typedef TSharedPtr<FOnlineLeaderboardsEOS, ESPMode::ThreadSafe> FOnlineLeaderboardsEOSPtr;
typedef TWeakPtr<FOnlineLeaderboardsEOS, ESPMode::ThreadSafe> FOnlineLeaderboardsEOSWeakPtr;

#endif
