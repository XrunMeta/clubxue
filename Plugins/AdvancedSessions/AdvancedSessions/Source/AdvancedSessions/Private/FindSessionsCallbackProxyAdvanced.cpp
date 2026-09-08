
#include "FindSessionsCallbackProxyAdvanced.h"

#include "Online/OnlineSessionNames.h"

UFindSessionsCallbackProxyAdvanced::UFindSessionsCallbackProxyAdvanced(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Delegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnCompleted))
	, bUseLAN(false)
{
	bRunSecondSearch = false;
	bIsOnSecondSearch = false;
}

UFindSessionsCallbackProxyAdvanced* UFindSessionsCallbackProxyAdvanced::FindSessionsAdvanced(UObject* WorldContextObject, class APlayerController* PlayerController, int MaxResults, bool bUseLAN, EBPServerPresenceSearchType ServerTypeToSearch, const TArray<FSessionsSearchSetting> &Filters, bool bEmptyServersOnly, bool bNonEmptyServersOnly, bool bSecureServersOnly,  int MinSlotsAvailable)
{
	UFindSessionsCallbackProxyAdvanced* Proxy = NewObject<UFindSessionsCallbackProxyAdvanced>();	
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->bUseLAN = bUseLAN;
	Proxy->MaxResults = MaxResults;
	Proxy->WorldContextObject = WorldContextObject;
	Proxy->SearchSettings = Filters;
	Proxy->ServerSearchType = ServerTypeToSearch;
	Proxy->bEmptyServersOnly = bEmptyServersOnly,
	Proxy->bNonEmptyServersOnly = bNonEmptyServersOnly;
	Proxy->bSecureServersOnly = bSecureServersOnly;

	Proxy->MinSlotsAvailable = MinSlotsAvailable;
	return Proxy;
}

void UFindSessionsCallbackProxyAdvanced::Activate()
{
	FOnlineSubsystemBPCallHelperAdvanced Helper(TEXT("FindSessions"), GEngine->GetWorldFromContextObject(WorldContextObject.Get(), EGetWorldErrorMode::LogAndReturnNull));
	Helper.QueryIDFromPlayerController(PlayerControllerWeakPtr.Get());

	if (Helper.IsValid())
	{
		auto Sessions = Helper.OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{

			bRunSecondSearch = false;
			bIsOnSecondSearch = false;

			DelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(Delegate);

			SearchObject = MakeShareable(new FOnlineSessionSearch);
			SearchObject->MaxSearchResults = MaxResults;
			SearchObject->bIsLanQuery = bUseLAN;

			FOnlineSearchSettingsEx tem;

			if (bEmptyServersOnly)
				tem.Set(SEARCH_EMPTY_SERVERS_ONLY, true, EOnlineComparisonOp::Equals);

			if (bNonEmptyServersOnly)
				tem.Set(SEARCH_NONEMPTY_SERVERS_ONLY, true, EOnlineComparisonOp::Equals);

			if (bSecureServersOnly)
				tem.Set(SEARCH_SECURE_SERVERS_ONLY, true, EOnlineComparisonOp::Equals);

			if (MinSlotsAvailable != 0)
				tem.Set(SEARCH_MINSLOTSAVAILABLE, MinSlotsAvailable, EOnlineComparisonOp::GreaterThanEquals);

			if (SearchSettings.Num() > 0)
			{
				for (int i = 0; i < SearchSettings.Num(); i++)
				{

					tem.HardSet(SearchSettings[i].PropertyKeyPair.Key, SearchSettings[i].PropertyKeyPair.Data, SearchSettings[i].ComparisonOp);
				}
			}

			switch (ServerSearchType)
			{

			case EBPServerPresenceSearchType::ClientServersOnly:
			{

					tem.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
			}
			break;

			case EBPServerPresenceSearchType::DedicatedServersOnly:
			{

			}
			break;

			case EBPServerPresenceSearchType::AllServers:
			default:
			{

				bRunSecondSearch = true;

				SearchObjectDedicated = MakeShareable(new FOnlineSessionSearch);
				SearchObjectDedicated->MaxSearchResults = MaxResults;
				SearchObjectDedicated->bIsLanQuery = bUseLAN;

				FOnlineSearchSettingsEx DedicatedOnly = tem;

					tem.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

				SearchObjectDedicated->QuerySettings = DedicatedOnly;

			}
			break;
			}

			SearchObject->QuerySettings = tem;

			Sessions->FindSessions(*Helper.UserID, SearchObject.ToSharedRef());

			return;
		}
		else
		{
			FFrame::KismetExecutionMessage(TEXT("Sessions not supported by Online Subsystem"), ELogVerbosity::Warning);
		}
	}

	OnFailure.Broadcast(SessionSearchResults);
}

void UFindSessionsCallbackProxyAdvanced::OnCompleted(bool bSuccess)
{
	FOnlineSubsystemBPCallHelperAdvanced Helper(TEXT("FindSessionsCallback"), GEngine->GetWorldFromContextObject(WorldContextObject.Get(), EGetWorldErrorMode::LogAndReturnNull));
	Helper.QueryIDFromPlayerController(PlayerControllerWeakPtr.Get());

	if (!Helper.IsValid())
	{

		OnFailure.Broadcast(SessionSearchResults);
		return;
	}

	if (!bRunSecondSearch && Helper.IsValid())
	{
		auto Sessions = Helper.OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(DelegateHandle);
		}
	}

	if (bSuccess)
	{
		if (bIsOnSecondSearch)
		{
			if (SearchObjectDedicated.IsValid())
			{

				for (auto& Result : SearchObjectDedicated->SearchResults)
				{
					FString ResultText = FString::Printf(TEXT("Found a session. Ping is %d"), Result.PingInMs);

					FFrame::KismetExecutionMessage(*ResultText, ELogVerbosity::Log);

					FBlueprintSessionResult BPResult;
					BPResult.OnlineResult = Result;
					SessionSearchResults.AddUnique(BPResult);
				}
				OnSuccess.Broadcast(SessionSearchResults);
				return;
			}
		}
		else
		{
			if (SearchObject.IsValid())
			{

				for (auto& Result : SearchObject->SearchResults)
				{
					FString ResultText = FString::Printf(TEXT("Found a session. Ping is %d"), Result.PingInMs);

					FFrame::KismetExecutionMessage(*ResultText, ELogVerbosity::Log);

					FBlueprintSessionResult BPResult;
					BPResult.OnlineResult = Result;

					if (ServerSearchType != EBPServerPresenceSearchType::DedicatedServersOnly )
					{
						BPResult.OnlineResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;
						BPResult.OnlineResult.Session.SessionSettings.bUsesPresence = true;
					}

					SessionSearchResults.AddUnique(BPResult);
				}
				if (!bRunSecondSearch)
				{
					OnSuccess.Broadcast(SessionSearchResults);
					return;
				}
			}
		}
	}
	else
	{
		if (!bRunSecondSearch)
		{

			if (SessionSearchResults.Num() > 0)
				OnSuccess.Broadcast(SessionSearchResults);
			else
				OnFailure.Broadcast(SessionSearchResults);
			return;
		}
	}

	if (Helper.IsValid() && bRunSecondSearch && ServerSearchType == EBPServerPresenceSearchType::AllServers)
	{
		bRunSecondSearch = false;
		bIsOnSecondSearch = true;
		auto Sessions = Helper.OnlineSub->GetSessionInterface();
		Sessions->FindSessions(*Helper.UserID, SearchObjectDedicated.ToSharedRef());
	}
	else 
	{
		if (bSuccess && SessionSearchResults.Num() > 0)
			OnSuccess.Broadcast(SessionSearchResults);
		else
			OnFailure.Broadcast(SessionSearchResults);
	}
}

void UFindSessionsCallbackProxyAdvanced::FilterSessionResults(const TArray<FBlueprintSessionResult> &SessionResults, const TArray<FSessionsSearchSetting> &Filters, TArray<FBlueprintSessionResult> &FilteredResults)
{
	for (int j = 0; j < SessionResults.Num(); j++)
	{
		bool bAddResult = true;

		if (Filters.Num() > 0)
		{
			const FOnlineSessionSetting * setting;
			for (int i = 0; i < Filters.Num(); i++)
			{
				setting = SessionResults[j].OnlineResult.Session.SessionSettings.Settings.Find(Filters[i].PropertyKeyPair.Key);

				if (!setting)
					continue;

				if (!CompareVariants(setting->Data, Filters[i].PropertyKeyPair.Data, Filters[i].ComparisonOp))
				{
					bAddResult = false;
					break;
				}
			}
		}

		if (bAddResult)
			FilteredResults.Add(SessionResults[j]);
	}

	return;
}

bool UFindSessionsCallbackProxyAdvanced::CompareVariants(const FVariantData &A, const FVariantData &B, EOnlineComparisonOpRedux Comparator)
{
	if (A.GetType() != B.GetType())
		return false;

	switch (A.GetType())
	{
	case EOnlineKeyValuePairDataType::Bool:
	{
		bool bA, bB;
		A.GetValue(bA);
		B.GetValue(bB);
		switch (Comparator)
		{
		case EOnlineComparisonOpRedux::Equals:
			return bA == bB; break;
		case EOnlineComparisonOpRedux::NotEquals:
			return bA != bB; break;
		default:
			return false;break;
		}
	}
	case EOnlineKeyValuePairDataType::Double:
	{
		double bA, bB;
		A.GetValue(bA);
		B.GetValue(bB);
		switch (Comparator)
		{
		case EOnlineComparisonOpRedux::Equals:
			return bA == bB; break;
		case EOnlineComparisonOpRedux::NotEquals:
			return bA != bB; break;
		case EOnlineComparisonOpRedux::GreaterThanEquals:
			return (bA == bB || bA > bB); break;
		case EOnlineComparisonOpRedux::LessThanEquals:
			return (bA == bB || bA < bB); break;
		case EOnlineComparisonOpRedux::GreaterThan:
			return bA > bB; break;
		case EOnlineComparisonOpRedux::LessThan:
			return bA < bB; break;
		default:
			return false; break;
		}
	}
	case EOnlineKeyValuePairDataType::Float:
	{
		float tbA, tbB;
		double bA, bB;
		A.GetValue(tbA);
		B.GetValue(tbB);
		bA = (double)tbA;
		bB = (double)tbB;
		switch (Comparator)
		{
		case EOnlineComparisonOpRedux::Equals:
			return bA == bB; break;
		case EOnlineComparisonOpRedux::NotEquals:
			return bA != bB; break;
		case EOnlineComparisonOpRedux::GreaterThanEquals:
			return (bA == bB || bA > bB); break;
		case EOnlineComparisonOpRedux::LessThanEquals:
			return (bA == bB || bA < bB); break;
		case EOnlineComparisonOpRedux::GreaterThan:
			return bA > bB; break;
		case EOnlineComparisonOpRedux::LessThan:
			return bA < bB; break;
		default:
			return false; break;
		}
	}
	case EOnlineKeyValuePairDataType::Int32:
	{
		int32 bA, bB;
		A.GetValue(bA);
		B.GetValue(bB);
		switch (Comparator)
		{
		case EOnlineComparisonOpRedux::Equals:
			return bA == bB; break;
		case EOnlineComparisonOpRedux::NotEquals:
			return bA != bB; break;
		case EOnlineComparisonOpRedux::GreaterThanEquals:
			return (bA == bB || bA > bB); break;
		case EOnlineComparisonOpRedux::LessThanEquals:
			return (bA == bB || bA < bB); break;
		case EOnlineComparisonOpRedux::GreaterThan:
			return bA > bB; break;
		case EOnlineComparisonOpRedux::LessThan:
			return bA < bB; break;
		default:
			return false; break;
		}
	}
	case EOnlineKeyValuePairDataType::Int64:
	{
		uint64 bA, bB;
		A.GetValue(bA);
		B.GetValue(bB);
		switch (Comparator)
		{
		case EOnlineComparisonOpRedux::Equals:
			return bA == bB; break;
		case EOnlineComparisonOpRedux::NotEquals:
			return bA != bB; break;
		case EOnlineComparisonOpRedux::GreaterThanEquals:
			return (bA == bB || bA > bB); break;
		case EOnlineComparisonOpRedux::LessThanEquals:
			return (bA == bB || bA < bB); break;
		case EOnlineComparisonOpRedux::GreaterThan:
			return bA > bB; break;
		case EOnlineComparisonOpRedux::LessThan:
			return bA < bB; break;
		default:
			return false; break;
		}
	}

	case EOnlineKeyValuePairDataType::String:
	{
		FString bA, bB;
		A.GetValue(bA);
		B.GetValue(bB);
		switch (Comparator)
		{
		case EOnlineComparisonOpRedux::Equals:
			return bA == bB; break;
		case EOnlineComparisonOpRedux::NotEquals:
			return bA != bB; break;
		default:
			return false; break;
		}
	}

	case EOnlineKeyValuePairDataType::Empty:
	case EOnlineKeyValuePairDataType::Blob:
	default:
		return false; break;
	}

}