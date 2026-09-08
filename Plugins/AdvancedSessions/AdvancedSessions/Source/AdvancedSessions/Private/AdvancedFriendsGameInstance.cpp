
#include "AdvancedFriendsGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY(AdvancedFriendsInterfaceLog);

UAdvancedFriendsGameInstance::UAdvancedFriendsGameInstance(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
	, bCallFriendInterfaceEventsOnPlayerControllers(true)
	, bCallIdentityInterfaceEventsOnPlayerControllers(true)
	, bCallVoiceInterfaceEventsOnPlayerControllers(true)
	, bEnableTalkingStatusDelegate(true)
	, SessionInviteReceivedDelegate(FOnSessionInviteReceivedDelegate::CreateUObject(this, &ThisClass::OnSessionInviteReceivedMaster))
	, SessionInviteAcceptedDelegate(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::OnSessionInviteAcceptedMaster))
	, PlayerTalkingStateChangedDelegate(FOnPlayerTalkingStateChangedDelegate::CreateUObject(this, &ThisClass::OnPlayerTalkingStateChangedMaster))
	, PlayerLoginChangedDelegate(FOnLoginChangedDelegate::CreateUObject(this, &ThisClass::OnPlayerLoginChangedMaster))
	, PlayerLoginStatusChangedDelegate(FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::OnPlayerLoginStatusChangedMaster))
{
}

void UAdvancedFriendsGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bAutoJoinSessionOnAcceptedUserInviteReceived)
		return;

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid())
	{

		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
		OnJoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UAdvancedFriendsGameInstance::OnJoinSessionComplete));

		if (!InviteResult.Session.SessionSettings.bIsDedicated)
		{
			FOnlineSessionSearchResult ModResult = InviteResult;
			ModResult.Session.SessionSettings.bUsesPresence = true;
			ModResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;
			SessionInterface->JoinSession(0, NAME_GameSession, ModResult);
		}
		else
		{
			SessionInterface->JoinSession(0, NAME_GameSession, InviteResult);
		}
	}
	UE_LOGF(AdvancedFriendsInterfaceLog, Log, "Called Join Session for Steam Friends List UI InviteResults: %ls, UserId: %ls", *InviteResult.GetSessionIdStr(), *UserId->ToString());
}

void UAdvancedFriendsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{

	if (!bAutoTravelOnAcceptedUserInviteReceived)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid())
	{
		FString ConnectInfo;
		if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
		{
			APlayerController* PlayerController = GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UAdvancedFriendsGameInstance::Shutdown()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (!SessionInterface.IsValid())
	{
		UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsGameInstance Failed to get session system!");

	}
	else
	{

		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegateHandle);
		SessionInterface->ClearOnSessionInviteReceivedDelegate_Handle(SessionInviteReceivedDelegateHandle);
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}

	if (bEnableTalkingStatusDelegate)
	{
		IOnlineVoicePtr VoiceInterface = Online::GetVoiceInterface(GetWorld());

		if (VoiceInterface.IsValid())
		{
			VoiceInterface->ClearOnPlayerTalkingStateChangedDelegate_Handle(PlayerTalkingStateChangedDelegateHandle);
		}
		else
		{

			UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Failed to get voice interface!");
		}
	}

	IOnlineIdentityPtr IdentityInterface = Online::GetIdentityInterface(GetWorld());

	if (IdentityInterface.IsValid())
	{
		IdentityInterface->ClearOnLoginChangedDelegate_Handle(PlayerLoginChangedDelegateHandle);

		IdentityInterface->ClearOnLoginStatusChangedDelegate_Handle(0, PlayerLoginStatusChangedDelegateHandle);
	}

	Super::Shutdown();
}

void UAdvancedFriendsGameInstance::Init()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (SessionInterface.IsValid())
	{

		SessionInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegate);

		SessionInviteReceivedDelegateHandle = SessionInterface->AddOnSessionInviteReceivedDelegate_Handle(SessionInviteReceivedDelegate);

		SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UAdvancedFriendsGameInstance::OnSessionUserInviteAccepted);
	}
	else
	{
		UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Failed to get session interface!");

	}

	if (bEnableTalkingStatusDelegate)
	{
		IOnlineVoicePtr VoiceInterface = Online::GetVoiceInterface(GetWorld());

		if (VoiceInterface.IsValid())
		{
			PlayerTalkingStateChangedDelegateHandle = VoiceInterface->AddOnPlayerTalkingStateChangedDelegate_Handle(PlayerTalkingStateChangedDelegate);
		}
		else
		{

			UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Failed to get voice interface!");
		}
	}

	IOnlineIdentityPtr IdentityInterface = Online::GetIdentityInterface(GetWorld());

	if (IdentityInterface.IsValid())
	{
		PlayerLoginChangedDelegateHandle = IdentityInterface->AddOnLoginChangedDelegate_Handle(PlayerLoginChangedDelegate);

		PlayerLoginStatusChangedDelegateHandle = IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(0, PlayerLoginStatusChangedDelegate);
	}
	else
	{
		UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Failed to get identity interface!");
	}

	Super::Init();
}

void UAdvancedFriendsGameInstance::OnPlayerLoginStatusChangedMaster(int32 PlayerNum, ELoginStatus::Type PreviousStatus, ELoginStatus::Type NewStatus, const FUniqueNetId & NewPlayerUniqueNetID)
{
	EBPLoginStatus OrigStatus = (EBPLoginStatus)PreviousStatus;
	EBPLoginStatus CurrentStatus = (EBPLoginStatus)NewStatus;
	FBPUniqueNetId PlayerID;
	PlayerID.SetUniqueNetId(&NewPlayerUniqueNetID);

	OnPlayerLoginStatusChanged(PlayerNum, OrigStatus,CurrentStatus,PlayerID);

	if (bCallIdentityInterfaceEventsOnPlayerControllers)
	{
		APlayerController* Player = UGameplayStatics::GetPlayerController(GetWorld(), PlayerNum);

		if (Player != NULL)
		{

			if (Player->GetClass()->ImplementsInterface(UAdvancedFriendsInterface::StaticClass()))
			{
				IAdvancedFriendsInterface::Execute_OnPlayerLoginStatusChanged(Player, OrigStatus, CurrentStatus, PlayerID);
			}
		}
		else
		{
			UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Failed to get a controller with the specified index in OnPlayerLoginStatusChangedMaster!");
		}
	}
}

void UAdvancedFriendsGameInstance::OnPlayerLoginChangedMaster(int32 PlayerNum)
{
	OnPlayerLoginChanged(PlayerNum);

	if (bCallIdentityInterfaceEventsOnPlayerControllers)
	{
		APlayerController* Player = UGameplayStatics::GetPlayerController(GetWorld(), PlayerNum);

		if (Player != NULL)
		{

			if (Player->GetClass()->ImplementsInterface(UAdvancedFriendsInterface::StaticClass()))
			{
				IAdvancedFriendsInterface::Execute_OnPlayerLoginChanged(Player, PlayerNum);
			}
		}
		else
		{
			UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Failed to get a controller with the specified index in OnPlayerLoginChanged!");
		}
	}
}

void UAdvancedFriendsGameInstance::OnPlayerTalkingStateChangedMaster(TSharedRef<const FUniqueNetId> PlayerId, bool bIsTalking)
{
	FBPUniqueNetId PlayerTalking;
	PlayerTalking.SetUniqueNetId(PlayerId);
	OnPlayerTalkingStateChanged(PlayerTalking, bIsTalking);

	if (bCallVoiceInterfaceEventsOnPlayerControllers)
	{
		APlayerController* Player = NULL;

		for (const ULocalPlayer* LPlayer : LocalPlayers)
		{
			Player = UGameplayStatics::GetPlayerController(GetWorld(), LPlayer->GetControllerId());

			if (Player != NULL)
			{

				if (Player->GetClass()->ImplementsInterface(UAdvancedFriendsInterface::StaticClass()))
				{
					IAdvancedFriendsInterface::Execute_OnPlayerVoiceStateChanged(Player, PlayerTalking, bIsTalking);
				}
			}
			else
			{
				UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Failed to get a controller with the specified index in OnVoiceStateChanged!");
			}
		}
	}
}

void UAdvancedFriendsGameInstance::OnSessionInviteReceivedMaster(const FUniqueNetId & PersonInvited, const FUniqueNetId & PersonInviting, const FString& AppId, const FOnlineSessionSearchResult& SessionToJoin)
{
	if (SessionToJoin.IsValid())
	{
		FBlueprintSessionResult BluePrintResult;
		BluePrintResult.OnlineResult = SessionToJoin;

		FBPUniqueNetId PInvited;
		PInvited.SetUniqueNetId(&PersonInvited);

		FBPUniqueNetId PInviting;
		PInviting.SetUniqueNetId(&PersonInviting);

		TArray<APlayerController*> PlayerList;
		GEngine->GetAllLocalPlayerControllers(PlayerList);

		APlayerController* Player = NULL;

		int32 LocalPlayer = 0;
		for (int i = 0; i < PlayerList.Num(); i++)
		{
			if (*PlayerList[i]->PlayerState->GetUniqueId().GetUniqueNetId() == PersonInvited)
			{
				LocalPlayer = i;
				Player = PlayerList[i];
				break;
			}
		}

		if (!BluePrintResult.OnlineResult.Session.SessionSettings.bIsDedicated)
		{
			BluePrintResult.OnlineResult.Session.SessionSettings.bUsesPresence = true;
			BluePrintResult.OnlineResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;
		}

		OnSessionInviteReceived(LocalPlayer, PInviting, AppId, BluePrintResult);

		if (Player != NULL)
		{

			if (Player->GetClass()->ImplementsInterface(UAdvancedFriendsInterface::StaticClass()))
			{
				IAdvancedFriendsInterface::Execute_OnSessionInviteReceived(Player, PInviting, BluePrintResult);
			}
		}
		else
		{
			UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Failed to get a controller with the specified index in OnSessionInviteReceived!");
		}
	}
	else
	{
		UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Return a bad search result in OnSessionInviteReceived!");
	}
}

void UAdvancedFriendsGameInstance::OnSessionInviteAcceptedMaster(const bool bWasSuccessful, int32 LocalPlayer, TSharedPtr<const FUniqueNetId> PersonInvited, const FOnlineSessionSearchResult& SessionToJoin)
{
	if (bWasSuccessful)
	{
		if (SessionToJoin.IsValid())
		{

			FBlueprintSessionResult BluePrintResult;
			BluePrintResult.OnlineResult = SessionToJoin;

			FBPUniqueNetId PInvited;
			PInvited.SetUniqueNetId(PersonInvited);

			if (!BluePrintResult.OnlineResult.Session.SessionSettings.bIsDedicated)
			{
				BluePrintResult.OnlineResult.Session.SessionSettings.bUsesPresence = true;
				BluePrintResult.OnlineResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;
			}

			OnSessionInviteAccepted(LocalPlayer,PInvited, BluePrintResult);

			APlayerController* Player = UGameplayStatics::GetPlayerController(GetWorld(), LocalPlayer);

			if (Player != NULL)
			{

				if (Player->GetClass()->ImplementsInterface(UAdvancedFriendsInterface::StaticClass()))
				{
					IAdvancedFriendsInterface::Execute_OnSessionInviteAccepted(Player,PInvited, BluePrintResult);
				}
			}
			else
			{ 
				UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Failed to get a controller with the specified index in OnSessionInviteAccepted!");
			}
		}
		else
		{
			UE_LOGF(AdvancedFriendsInterfaceLog, Warning, "UAdvancedFriendsInstance Return a bad search result in OnSessionInviteAccepted!");
		}
	}
}