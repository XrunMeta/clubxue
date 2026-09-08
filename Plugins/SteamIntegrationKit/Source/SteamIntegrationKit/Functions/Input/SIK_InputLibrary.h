

#pragma once

#include "CoreMinimal.h"
#include "SIK_SharedFile.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SIK_InputLibrary.generated.h"

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_InputLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void ActivateActionSet(FSIK_InputHandle InputHandle, FSIK_InputActionSetHandle ActionSetHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void ActivateActionSetLayer(FSIK_InputHandle InputHandle, FSIK_InputActionSetHandle ActionSetLayerHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void DeactivateActionSetLayer(FSIK_InputHandle InputHandle, FSIK_InputActionSetHandle ActionSetLayerHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void DeactivateAllActionSetLayers(FSIK_InputHandle InputHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static int64 GetActiveActionSetLayers(FSIK_InputHandle InputHandle, TArray<FSIK_InputActionSetHandle>& ActionSetHandles);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FSIK_InputActionSetHandle GetActionSetHandle(FString ActionSetName);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FSIK_InputAnalogActionData GetAnalogActionData(FSIK_InputHandle InputHandle, FSIK_InputAnalogActionHandle AnalogActionHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FSIK_InputAnalogActionHandle GetAnalogActionHandle(FString AnalogActionName);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static int64 GetAnalogActionOrigins(FSIK_InputHandle InputHandle, FSIK_InputActionSetHandle ActionSetHandle, FSIK_InputAnalogActionHandle AnalogActionHandle, TArray<TEnumAsByte<ESIK_InputActionOrigin>>& OriginsOut);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static int64 GetConnectedControllers(TArray<FSIK_InputHandle>& HandlesOut);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FSIK_InputHandle GetControllerForGamepadIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FSIK_InputActionSetHandle GetCurrentActionSet(FSIK_InputHandle InputHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FSIK_InputDigitalActionData GetDigitalActionData(FSIK_InputHandle InputHandle, FSIK_InputDigitalActionHandle DigitalActionHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FSIK_InputDigitalActionHandle GetDigitalActionHandle(FString DigitalActionName);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static int64 GetDigitalActionOrigins(FSIK_InputHandle InputHandle, FSIK_InputActionSetHandle ActionSetHandle, FSIK_InputDigitalActionHandle DigitalActionHandle, TArray<TEnumAsByte<ESIK_InputActionOrigin>>& OriginsOut);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static int64 GetGamepadIndexForController(FSIK_InputHandle InputHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FString GetGlyphForActionOrigin(TEnumAsByte<ESIK_InputActionOrigin> Origin);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static ESIK_SteamInputType GetInputTypeForHandle(FSIK_InputHandle InputHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FSIK_InputMotionData GetMotionData(FSIK_InputHandle InputHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static FString GetStringForActionOrigin(TEnumAsByte<ESIK_InputActionOrigin> Origin);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static bool Init(bool bExplicitlyCallRunFrame);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void RunFrame();

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void SetDualSenseTriggerEffect(FSIK_InputHandle InputHandle, FSIK_ScePadTriggerEffectCommand L2, FSIK_ScePadTriggerEffectCommand R2);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void SetLEDColor(FSIK_InputHandle InputHandle, FLinearColor Color, TEnumAsByte<ESIK_SteamControllerLEDFlag> LEDFlag);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static bool ShowBindingPanel(FSIK_InputHandle InputHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static bool Shutdown();

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void StopAnalogActionMomentum(FSIK_InputHandle InputHandle, FSIK_InputAnalogActionHandle AnalogActionHandle);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void TriggerHapticPulse(FSIK_InputHandle InputHandle, TEnumAsByte<ESIK_SteamControllerPad> TargetPad, uint8 DurationMicroSec);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void TriggerRepeatedHapticPulse(FSIK_InputHandle InputHandle, TEnumAsByte<ESIK_SteamControllerPad> TargetPad, uint8 DurationMicroSec, uint8 OffMicroSec, uint8 Repeat, uint8 Flags);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void TriggerVibration(FSIK_InputHandle InputHandle, uint8 LeftSpeed, uint8 RightSpeed);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static void TriggerVibrationExtended(FSIK_InputHandle InputHandle, uint8 LeftSpeed, uint8 RightSpeed, uint8 LeftTriggerSpeed, uint8 RightTriggerSpeed);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static TEnumAsByte<ESIK_InputActionOrigin> GetActionOriginFromXboxOrigin(FSIK_InputHandle InputHandle, TEnumAsByte<ESIK_XboxOrigin> XboxOrigin);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static TEnumAsByte<ESIK_InputActionOrigin> TranslateActionOrigin(TEnumAsByte<ESIK_SteamInputType> eDestinationInputType, TEnumAsByte<ESIK_InputActionOrigin> eSourceActionOrigin);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static bool GetDeviceBindingRevision(FSIK_InputHandle InputHandle, int32& Major, int32& Minor);

	UFUNCTION(BlueprintCallable, Category = "Steam Integration Kit || SDK Functions || Input")
	static int64 GetRemotePlaySessionID(FSIK_InputHandle InputHandle);

};
