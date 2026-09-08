
#include "UpdateSessionCallbackProxyAdvanced.h"

UUpdateSessionCallbackProxyAdvanced::UUpdateSessionCallbackProxyAdvanced(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, OnUpdateSessionCompleteDelegate(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateCompleted))
	, NumPublicConnections(1)
{
}	

UUpdateSessionCallbackProxyAdvanced* UUpdateSessionCallbackProxyAdvanced::UpdateSession(UObject* WorldContextObject, const TArray<FSessionPropertyKeyPair> &ExtraSettings, int32 PublicConnections, int32 PrivateConnections, bool bUseLAN, bool bAllowInvites, bool bAllowJoinInProgress, bool bRefreshOnlineData, bool bIsDedicatedServer, bool bShouldAdvertise, bool bAllowJoinViaPresence, bool bAllowJoinViaPresenceFriendsOnly)
{
	UUpdateSessionCallbackProxyAdvanced* Proxy = NewObject<UUpdateSessionCallbackProxyAdvanced>();
	Proxy->NumPublicConnections = PublicConnections;
	Proxy->NumPrivateConnections = PrivateConnections;
	Proxy->bUseLAN = bUseLAN;
	Proxy->WorldContextObject = WorldContextObject;
	Proxy->bAllowInvites = bAllowInvites;
	Proxy->ExtraSettings = ExtraSettings;
	Proxy->bRefreshOnlineData = bRefreshOnlineData;
	Proxy->bAllowJoinInProgress = bAllowJoinInProgress;
	Proxy->bDedicatedServer = bIsDedicatedServer;
	Proxy->bShouldAdvertise = bShouldAdvertise;
	Proxy->bAllowJoinViaPresence = bAllowJoinViaPresence;
	Proxy->bAllowJoinViaPresenceFriendsOnly = bAllowJoinViaPresenceFriendsOnly;
	return Proxy;	
}

void UUpdateSessionCallbackProxyAdvanced::Activate()
{
	const FOnlineSubsystemBPCallHelperAdvanced Helper(TEXT("UpdateSession"), GEngine->GetWorldFromContextObject(WorldContextObject.Get(), EGetWorldErrorMode::LogAndReturnNull));

	if (Helper.OnlineSub != nullptr)
	{
		const auto Sessions = Helper.OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			if (Sessions->GetNumSessions() < 1)
			{
				OnFailure.Broadcast();
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("NO REGISTERED SESSIONS!"));
				return;
			}

			FOnlineSessionSettings* Settings = Sessions->GetSessionSettings(NAME_GameSession);

			if (!Settings)
			{

				OnFailure.Broadcast();
				return;
			}

			OnUpdateSessionCompleteDelegateHandle = Sessions->AddOnUpdateSessionCompleteDelegate_Handle(OnUpdateSessionCompleteDelegate);

			Settings->NumPublicConnections = NumPublicConnections;
			Settings->NumPrivateConnections = NumPrivateConnections;
			Settings->bShouldAdvertise = bShouldAdvertise;
			Settings->bAllowJoinInProgress = bAllowJoinInProgress;
			Settings->bIsLANMatch = bUseLAN;

			Settings->bAllowInvites = bAllowInvites;
			Settings->bAllowJoinInProgress = bAllowJoinInProgress;
			Settings->bIsDedicated = bDedicatedServer;

			Settings->bAllowJoinViaPresence = bAllowJoinViaPresence;
			Settings->bAllowJoinViaPresenceFriendsOnly = bAllowJoinViaPresenceFriendsOnly;

			FOnlineSessionSetting * fSetting = NULL;
			FOnlineSessionSetting ExtraSetting;
			for (int i = 0; i < ExtraSettings.Num(); i++)
			{
				fSetting = Settings->Settings.Find(ExtraSettings[i].Key);

				if (fSetting)
				{
					fSetting->Data = ExtraSettings[i].Data;
				}
				else
				{
					ExtraSetting.Data = ExtraSettings[i].Data;
					ExtraSetting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
					Settings->Settings.Add(ExtraSettings[i].Key, ExtraSetting);
				}
			}

			Sessions->UpdateSession(NAME_GameSession, *Settings, bRefreshOnlineData);

			return;
		}
		else
		{
			FFrame::KismetExecutionMessage(TEXT("Sessions not supported by Online Subsystem"), ELogVerbosity::Warning);
		}
	}

	OnFailure.Broadcast();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Sessions not supported"));
}

void UUpdateSessionCallbackProxyAdvanced::OnUpdateCompleted(FName SessionName, bool bWasSuccessful)
{
	const FOnlineSubsystemBPCallHelperAdvanced Helper(TEXT("UpdateSessionCallback"), GEngine->GetWorldFromContextObject(WorldContextObject.Get(), EGetWorldErrorMode::LogAndReturnNull));

	if (Helper.OnlineSub != nullptr)
	{
		const auto Sessions = Helper.OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnUpdateSessionCompleteDelegate_Handle(OnUpdateSessionCompleteDelegateHandle);

			if (bWasSuccessful)
			{
				OnSuccess.Broadcast();
				return;
			}
		}
	}

	if (!bWasSuccessful)
	{
		OnFailure.Broadcast();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("WAS NOT SUCCESSFUL"));
	}
}