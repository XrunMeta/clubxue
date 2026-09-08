#include "VirtualCursor/VirtualCursorManager.h"
#include "VirtualCursor/ExtendedAnalogCursor.h"
#include "VirtualCursor/CursorSettings.h"
#include "Framework/Application/SlateApplication.h"

#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogVirtualCursorManager);

void UVirtualCursorManager::Initialize(FSubsystemCollectionBase& Collection)
{
}

void UVirtualCursorManager::Deinitialize()
{

	DisableAnalogCursor();
	Cursor.Reset();
}

void UVirtualCursorManager::EnableAnalogCursor(const bool bUseLeftStick)
{

	if (FSlateApplication::IsInitialized() && GetWorld())
	{
		const float CursorRadius = GetDefault<UCursorSettings>()->GetAnalogCursorRadius();

		if (!IsCursorValid())
		{
			Cursor = MakeShareable(new FExtendedAnalogCursor(GetLocalPlayer(), GetWorld(), CursorRadius));

			const EAnalogStick Stick = bUseLeftStick ? EAnalogStick::Left : EAnalogStick::Right;
			Cursor->SetStick(Stick);
		}

		if (!ContainsGamepadCursorInputProcessor())
		{
			FSlateApplication::Get().RegisterInputPreProcessor(Cursor);
		}
		FSlateApplication::Get().SetCursorRadius(CursorRadius);
	}
}

void UVirtualCursorManager::DisableAnalogCursor()
{
	if (FSlateApplication::IsInitialized())
	{

		if (ContainsGamepadCursorInputProcessor())
		{
			FSlateApplication::Get().UnregisterInputPreProcessor(Cursor);
		}
		FSlateApplication::Get().SetCursorRadius(0.0f);
	}
}

void UVirtualCursorManager::ToggleCursorDebug()
{
	if (IsCursorValid())
	{
		Cursor->bDebugging = !Cursor->bDebugging;
		const FString BoolResult = ((Cursor->bDebugging) ? "true" : "false");
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, "Cursor Debug: " + BoolResult);
	}
}

void UVirtualCursorManager::ToggleAnalogDebug()
{
	if (IsCursorValid())
	{
		Cursor->bAnalogDebug = !Cursor->bAnalogDebug;
		const FString BoolResult = ((Cursor->bAnalogDebug) ? "true" : "false");
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, "Analog Debug: " + BoolResult);
	}
}

bool UVirtualCursorManager::IsCursorDebugActive() const
{
	if (IsCursorValid())
	{
		return Cursor->bDebugging;
	}
	return false;
}

bool UVirtualCursorManager::IsAnalogDebugActive() const
{
	if (IsCursorValid())
	{
		return Cursor->bAnalogDebug;
	}
	return false;
}

bool UVirtualCursorManager::IsCursorOverInteractableWidget() const
{
	if (IsCursorValid())
	{
		return Cursor->IsHovered();
	}
	return false;
}

bool UVirtualCursorManager::IsCursorValid() const
{
	return Cursor.IsValid();
}

bool UVirtualCursorManager::ContainsGamepadCursorInputProcessor() const
{
	if (FSlateApplication::IsInitialized())
	{

		if (IsCursorValid())
		{

			const int32 FoundIndex = FSlateApplication::Get().FindInputPreProcessor(Cursor);
			return (FoundIndex > -1);
		}
	}
	return false;
}
