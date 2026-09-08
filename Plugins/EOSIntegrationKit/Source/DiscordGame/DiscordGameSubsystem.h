

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#if EIKDISCORDACTIVE
#include "discord-cpp/discord.h"
#endif
#include "DiscordGame.h"
#include "Subsystems/EngineSubsystem.h"
#include "Engine/Engine.h"
#include "Containers/Ticker.h"
#include "DiscordGameSubsystem.generated.h"

UCLASS(Config=Game)
class DISCORDGAME_API UDiscordGameSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:

	static UDiscordGameSubsystem* Get() { return GEngine ? GEngine->GetEngineSubsystem<UDiscordGameSubsystem>() : nullptr; }

	UDiscordGameSubsystem();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool IsDiscordSDKLoaded() const
	{
#if EIKDISCORDACTIVE
		return DiscordGameModule && DiscordGameModule->IsDiscordSDKLoaded();
#else
		UE_LOG(LogTemp, Warning, TEXT("Discord SDK Not Loaded"));
		return false;
#endif
	}

	UFUNCTION(BlueprintCallable, Category = "Discord")
	bool IsDiscordRunning() const
	{
#if EIKDISCORDACTIVE
		return DiscordCorePtr != nullptr;
#else
		return false;
#endif
	}

	UFUNCTION(BlueprintCallable, Category = "Discord")
	FString GetDiscordDisplayName()
	{
#if EIKDISCORDACTIVE
		if(IsDiscordRunning() && IsDiscordSDKLoaded())
		{
			discord::User CurrentUser;
			auto Rsult = DiscordCore().UserManager().GetCurrentUser(&CurrentUser);
			if(Rsult == discord::Result::Ok)
			{
				return UTF8_TO_TCHAR(CurrentUser.GetUsername());
			}
			return TEXT("Failed to get Discord User");
		}
		return TEXT("Discord is not running");
#else
		return TEXT("Discord setup files are not included in the project");
#endif
	}

	UFUNCTION(BlueprintCallable, Category = "Discord")
	int64 GetDiscordUserId()
	{
#if EIKDISCORDACTIVE
		if(IsDiscordRunning() && IsDiscordSDKLoaded())
		{
			discord::User CurrentUser;
			auto Rsult = DiscordCore().UserManager().GetCurrentUser(&CurrentUser);
			if(Rsult == discord::Result::Ok)
			{
				return CurrentUser.GetId();
			}
			UE_LOG(LogDiscord, Error, TEXT("Failed to get Discord Auth Token due to error: %d"), static_cast<int32>(Rsult));
			return -1;
		}
		return -1;
#else
		return -1;
#endif
	}

	UFUNCTION(BlueprintCallable, Category = "Discord")
	void GetDiscordAvatarUrl(FString& Hash, FString& URL)
	{
#if EIKDISCORDACTIVE
		if(IsDiscordRunning() && IsDiscordSDKLoaded())
		{
			discord::User CurrentUser;
			auto Rsult = DiscordCore().UserManager().GetCurrentUser(&CurrentUser);
			if(Rsult == discord::Result::Ok)
			{
				Hash =  UTF8_TO_TCHAR(CurrentUser.GetAvatar());
				URL = FString::Printf(TEXT("https://cdn.discordapp.com/avatars/%lld/%s.png"), CurrentUser.GetId(), *Hash);
				return;
			}
			UE_LOG(LogDiscord, Error, TEXT("Failed to get Discord Auth Token due to error: %d"), static_cast<int32>(Rsult));
			return;
		}
		return;
#else
		UE_LOG(LogDiscord, Error, TEXT("Discord setup files are not included in the project"));
		return;
#endif
	}
#if EIKDISCORDACTIVE

	FORCEINLINE_DEBUGGABLE discord::Core& DiscordCore() const
	{
		checkf(DiscordCorePtr, TEXT("Discord is not running"));
		return *DiscordCorePtr;
	}
#endif

	UFUNCTION(BlueprintCallable, Category = "Discord")
	bool IsClientIdValid() const { return ClientId != 0; }

protected:

	virtual void NativeOnDiscordCoreCreated();

	virtual void NativeOnDiscordCoreReset();

#if EIKDISCORDACTIVE

	virtual void NativeOnDiscordConnectError(discord::Result Result);

	virtual void NativeOnDiscordLogMessage(discord::LogLevel Level, const FString& Message) const;

	void LogDiscordResult(discord::Result Result, const FString& RequestDescription) const;
#endif

	void SetTickEnabled(bool bWantTicking);

	UPROPERTY(Config, EditDefaultsOnly, Category = "Discord")
	uint64 ClientId {0};

#if EIKDISCORDACTIVE

	discord::LogLevel MinimumLogLevel;
#endif

	UPROPERTY(Config, EditDefaultsOnly, Category = "Discord")
	float CreateRetryTime;

private:

	bool Tick(float DeltaTime);

	void TryCreateDiscordCore(float DeltaTime);

	void ResetDiscordCore();

	FDiscordGameModule* DiscordGameModule {nullptr};

#if EIKDISCORDACTIVE

	discord::Core* DiscordCorePtr {nullptr};
#endif

#if ENGINE_MAJOR_VERSION == 5
	FTSTicker::FDelegateHandle TickDelegateHandle;
#else
	FDelegateHandle TickDelegateHandle;
#endif

	float RetryWaitRemaining {-1.f};

	bool bLogConnectionErrors {true};

};
