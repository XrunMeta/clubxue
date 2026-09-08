

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

#include <steam/steam_api.h>

#endif

#include "SteamNotificationsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamOverlayActivated, bool, bOverlayState);

UCLASS()
class ADVANCEDSTEAMSESSIONS_API USteamNotificationsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "SteamEvents")
		FOnSteamOverlayActivated OnSteamOverlayActivated_Bind;

	USteamNotificationsSubsystem() : Super()
	{

	}

	class cSteamEventsStore
	{
	public:
		USteamNotificationsSubsystem* ParentSubsystem = nullptr;
		void Initialize(USteamNotificationsSubsystem* MyParent)
		{
			ParentSubsystem = MyParent;

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
			OnExternalUITriggeredCallback.Register(this, &USteamNotificationsSubsystem::cSteamEventsStore::OnExternalUITriggered);
#endif
		}

		void UnInitialize(USteamNotificationsSubsystem* MyParent)
		{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
			OnExternalUITriggeredCallback.Unregister();
#endif
		}

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
		cSteamEventsStore()
		{}

#else

#endif

	private:
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

		STEAM_CALLBACK_MANUAL(cSteamEventsStore, OnExternalUITriggered, GameOverlayActivated_t, OnExternalUITriggeredCallback);
#endif
	};

	cSteamEventsStore MyEvents;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		MyEvents.Initialize(this);
	}

	virtual void Deinitialize() override
	{
		MyEvents.UnInitialize(this);
	}
};

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
void USteamNotificationsSubsystem::cSteamEventsStore::OnExternalUITriggered(GameOverlayActivated_t* CallbackData)
{
	if (ParentSubsystem)
	{
		ParentSubsystem->OnSteamOverlayActivated_Bind.Broadcast((bool)CallbackData->m_bActive);
	}
}
#endif