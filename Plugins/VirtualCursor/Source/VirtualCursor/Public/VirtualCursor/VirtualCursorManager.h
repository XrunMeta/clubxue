#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "VirtualCursorManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVirtualCursorManager, Log, All);

class FExtendedAnalogCursor;

UCLASS(Blueprintable, BlueprintType)
class VIRTUALCURSOR_API UVirtualCursorManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	FORCEINLINE TSharedPtr<FExtendedAnalogCursor> GetCursor() const { return Cursor; }

	UFUNCTION(BlueprintCallable, Category = "Cursor")
	void EnableAnalogCursor(bool bUseLeftStick = true);

	UFUNCTION(BlueprintCallable, Category = "Cursor")
	void DisableAnalogCursor();

	UFUNCTION(BlueprintCallable, Category = "Cursor")
	void ToggleCursorDebug();

	UFUNCTION(BlueprintCallable, Category = "Cursor")
	void ToggleAnalogDebug();

	UFUNCTION(BlueprintPure, Category = "Cursor")
	bool IsCursorDebugActive() const;

	UFUNCTION(BlueprintPure, Category = "Cursor")
	bool IsAnalogDebugActive() const;

	UFUNCTION(BlueprintPure, Category = "Cursor")
	bool IsCursorOverInteractableWidget() const;

	UFUNCTION(BlueprintPure, Category = "Cursor")
	bool IsCursorValid() const;

	UFUNCTION(BlueprintPure, Category = "Cursor")
	bool ContainsGamepadCursorInputProcessor() const;

private:

	TSharedPtr<FExtendedAnalogCursor> Cursor;
};
