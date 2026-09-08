#pragma once

#include "Framework/Application/AnalogCursor.h"

class VIRTUALCURSOR_API FExtendedAnalogCursor : public FAnalogCursor
{
public:

	typedef FAnalogCursor Super;

	FExtendedAnalogCursor(ULocalPlayer* InLocalPlayer, UWorld* InWorld, float _Radius);
	FExtendedAnalogCursor(class APlayerController* PlayerController, float _Radius);

	virtual ~FExtendedAnalogCursor()
	{
	}

	virtual int32 GetOwnerUserIndex() const override;

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;

	FORCEINLINE FName GetHoveredWidgetName() const
	{
		return HoveredWidgetName;
	}

	FORCEINLINE bool IsHovered() const
	{
		return HoveredWidgetName != NAME_None;
	}

	FORCEINLINE FVector2D GetCurrentPosition() const
	{
		return CurrentPosition;
	}

	FORCEINLINE FVector2D GetVelocity() const
	{
		return Velocity;
	}

	FORCEINLINE bool GetIsUsingAnalogCursor() const
	{
		return bIsUsingAnalogCursor;
	}

	FORCEINLINE FVector2D GetLastCursorDirection() const
	{
		return LastCursorDirection;
	}

	FORCEINLINE float GetRadius() const
	{
		return Radius;
	}

	FORCEINLINE void SetStick(const EAnalogStick CursorMovementStick)
	{
		AnalogStick = CursorMovementStick;
	}

	uint8 bDebugging : 1;

	uint8 bAnalogDebug : 1;

protected:

private:

	FVector2D GetAnalogCursorAccelerationValue(const FVector2D& InAnalogValues, float DPIScale) const;

	bool IsCursorStickInput(const FAnalogInputEvent& AnalogInputEvent) const;

	FVector2D Velocity;

	FVector2D CurrentPosition;

	FVector2D LastCursorDirection;

	FName HoveredWidgetName;

	bool bIsUsingAnalogCursor;

	float Radius;

	FLocalPlayerContext PlayerContext;

	TSet<FKey> PressedKeys;

	EAnalogStick AnalogStick = EAnalogStick::Left;
};
