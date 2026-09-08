

#include "DiscordGameSubsystem.h"
#include "DiscordGame.h"

UDiscordGameSubsystem::UDiscordGameSubsystem(): ClientId(0)
{
#if EIKDISCORDACTIVE
	MinimumLogLevel = discord::LogLevel::Debug;
#endif
	CreateRetryTime = 5.0f;
}

bool UDiscordGameSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), OUT ChildClasses, false);

	UE_LOG(LogDiscord, Verbose, TEXT("Found %i derived classes when attemping to create DiscordGameSubsystem (%s)"), ChildClasses.Num(), *GetClass()->GetName());

	return ChildClasses.Num() == 0;
}

void UDiscordGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DiscordGameModule = FDiscordGameModule::Get();

	if (IsDiscordSDKLoaded())
	{
		UE_LOG(LogDiscord, Log, TEXT("SDK loaded, enabling subsystem ticking"));

		SetTickEnabled(true);
	}
	else
	{
		UE_LOG(LogDiscord, Log, TEXT("SDK load failed, disabling Discord subsystem"));
	}
}

void UDiscordGameSubsystem::Deinitialize()
{

	SetTickEnabled(false);
	ResetDiscordCore();
#if EIKDISCORDACTIVE
	DiscordGameModule = nullptr;
#endif
	Super::Deinitialize();
}

void UDiscordGameSubsystem::NativeOnDiscordCoreCreated()
{
#if EIKDISCORDACTIVE
	check(DiscordCorePtr);

	DiscordCorePtr->SetLogHook(MinimumLogLevel, [this](discord::LogLevel Level, const char* RawMessage)
	{
		const FString Message (UTF8_TO_TCHAR(RawMessage));
		NativeOnDiscordLogMessage(Level, Message);
	});
	DiscordCorePtr->UserManager().OnCurrentUserUpdate.Connect([this]()
	{
		UE_LOG(LogDiscord, Log, TEXT("Current User Updated"));
	});

	bLogConnectionErrors = true;
#endif
}

void UDiscordGameSubsystem::NativeOnDiscordCoreReset()
{
}

#if EIKDISCORDACTIVE
void UDiscordGameSubsystem::NativeOnDiscordConnectError(discord::Result Result)
{
	switch (Result)
	{

	case discord::Result::InternalError:
		if (bLogConnectionErrors)
		{
			UE_LOG(LogDiscord, Warning, TEXT("Error(%i) Connecting to Discord; Discord App not running?"), Result);

			bLogConnectionErrors = false;
		}
		else
		{

			UE_LOG(LogDiscord, VeryVerbose, TEXT("Error(%i) Connecting to Discord; Discord App not running?"), Result);
		}
		break;

	default:

		UE_LOG(LogDiscord, Error, TEXT("Error(%i) Connecting to Discord; Unknown error"), Result);
		break;
	}
}

void UDiscordGameSubsystem::NativeOnDiscordLogMessage(discord::LogLevel Level, const FString& Message) const
{
	switch (Level)
	{
	case discord::LogLevel::Debug:
		UE_LOG(LogDiscord, Verbose, TEXT("Discord Internal Debug: %s"), *Message);
		break;
	case discord::LogLevel::Info:
		UE_LOG(LogDiscord, Log, TEXT("Discord Internal Message: %s"), *Message);
		break;
	case discord::LogLevel::Warn:
		UE_LOG(LogDiscord, Warning, TEXT("Discord Internal Warning: %s"), *Message);
		break;
	case discord::LogLevel::Error:
	default:
		UE_LOG(LogDiscord, Error, TEXT("Discord Internal Error: %s"), *Message);
		break;
	}
}

void UDiscordGameSubsystem::LogDiscordResult(discord::Result Result, const FString& RequestDescription) const
{
	switch (Result)
	{
	case discord::Result::Ok:
		UE_LOG(LogDiscord, Log, TEXT("Success %s"), *RequestDescription);
		break;
	default:
		UE_LOG(LogDiscord, Error, TEXT("Error(%i) %s"), Result, *RequestDescription);
		break;
	}
}
#endif
void UDiscordGameSubsystem::SetTickEnabled(bool bWantTicking)
{
#if EIKDISCORDACTIVE
	if (bWantTicking && !TickDelegateHandle.IsValid())
	{

		TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick));
	}
	else if (!bWantTicking && TickDelegateHandle.IsValid())
	{

		FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
		TickDelegateHandle.Reset();
	}
#endif
}

bool UDiscordGameSubsystem::Tick(float DeltaTime)
{
#if EIKDISCORDACTIVE
	if (IsDiscordRunning())
	{
		const discord::Result Result = DiscordCorePtr->RunCallbacks();
		switch (Result)
		{
		case discord::Result::Ok:

			break;

		case discord::Result::NotRunning:

			UE_LOG(LogDiscord, Warning, TEXT("Error(%i) Running Callbacks; Discord app is no longer running"), Result);
			ResetDiscordCore();
			break;

		default:

			UE_LOG(LogDiscord, Error, TEXT("Error(%i) Running Callbacks"), Result);
			break;
		}
	}
	else if (IsDiscordSDKLoaded())
	{

		TryCreateDiscordCore(DeltaTime);
	}

	return true;
#else
	return false;
#endif
}

void UDiscordGameSubsystem::TryCreateDiscordCore(float DeltaTime)
{
	UE_LOG(LogDiscord, Log, TEXT("Attempting to create Discord Core"));
#if EIKDISCORDACTIVE
	RetryWaitRemaining -= DeltaTime;

	if (RetryWaitRemaining <= 0.f)
	{
		switch (const discord::Result Result = discord::Core::Create(ClientId, DiscordCreateFlags_NoRequireDiscord, &DiscordCorePtr))
		{
		case discord::Result::Ok:
			UE_LOG(LogDiscord, Log, TEXT("Created Discord Core"));
			NativeOnDiscordCoreCreated();
			break;

		default:
			NativeOnDiscordConnectError(Result);
			break;
		}

		RetryWaitRemaining = CreateRetryTime;
	}
#endif
}

void UDiscordGameSubsystem::ResetDiscordCore()
{
#if EIKDISCORDACTIVE
	if (DiscordCorePtr)
	{

		DiscordCorePtr = nullptr;

		NativeOnDiscordCoreReset();
	}

	RetryWaitRemaining = -1;
#endif
}
