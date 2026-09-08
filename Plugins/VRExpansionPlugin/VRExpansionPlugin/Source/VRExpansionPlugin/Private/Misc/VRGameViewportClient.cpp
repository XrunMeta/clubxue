

#include "Misc/VRGameViewportClient.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRGameViewportClient)

#include "CoreMinimal.h"

UVRGameViewportClient::UVRGameViewportClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameInputMethod = EVRGameInputMethod::GameInput_Default;
	bAlsoChangeGamepPadInput = false;
}

bool UVRGameViewportClient::IsValidGamePadKey(const FKey & InputKey)
{
	if (!bAlsoChangeGamepPadInput)
		return false;

	FName KeyCategory = InputKey.GetMenuCategory();

	return GamepadInputCategories.Contains(KeyCategory);
}

bool UVRGameViewportClient::EventWindowClosing()
{
	if (BPOnWindowCloseRequested.IsBound())
	{
		BPOnWindowCloseRequested.Broadcast();
	}

	if (bIgnoreWindowCloseCommands)
	{
		return false;
	}

	return true;
}

void UVRGameViewportClient::PostInitProperties()
{
	Super::PostInitProperties();

	if (GamepadInputCategories.Num() < 1)
	{
		GamepadInputCategories.Add(FName(TEXT("Gamepad")));
		GamepadInputCategories.Add(FName(TEXT("PS4")));
		GamepadInputCategories.Add(FName(TEXT("XBox One")));
		GamepadInputCategories.Add(FName(TEXT("Touch")));
		GamepadInputCategories.Add(FName(TEXT("Gesture")));
	}

	OnWindowCloseRequested().BindUObject(this, &UVRGameViewportClient::EventWindowClosing);
}

bool UVRGameViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();

	if(GameInputMethod == EVRGameInputMethod::GameInput_Default || IgnoreInput() || (EventArgs.IsGamepad() && !IsValidGamePadKey(EventArgs.Key)))
		return Super::InputKey(EventArgs);

	const int32 NumLocalPlayers = World->GetGameInstance()->GetNumLocalPlayers();

	if (NumLocalPlayers < 2)
		return Super::InputKey(EventArgs);

	FInputKeyEventArgs NewStruct = EventArgs;

	if (GameInputMethod == EVRGameInputMethod::GameInput_KeyboardAndMouseToPlayer2)
	{

		NewStruct.ControllerId++;

		FPlatformUserId UserId = PLATFORMUSERID_NONE;
		FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
		DeviceMapper.RemapControllerIdToPlatformUserAndDevice(NewStruct.ControllerId, UserId, NewStruct.InputDevice);

		return Super::InputKey(NewStruct);
	}
	else 
	{
		bool bRetVal = false;
		for (int32 i = 0; i < NumLocalPlayers; i++)
		{
			NewStruct.ControllerId = i;

			FPlatformUserId UserId = PLATFORMUSERID_NONE;
			FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
			DeviceMapper.RemapControllerIdToPlatformUserAndDevice(NewStruct.ControllerId, UserId, NewStruct.InputDevice);

			bRetVal = Super::InputKey(NewStruct) || bRetVal;
		}

		return bRetVal;
	}
}

bool UVRGameViewportClient::InputAxis(const FInputKeyEventArgs& Args)
{		

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();

	const int32 NumLocalPlayers = World->GetGameInstance()->GetNumLocalPlayers();

	if (((!Args.Key.IsMouseButton() && !Args.IsGamepad()) || (Args.IsGamepad() && !IsValidGamePadKey(Args.Key))) || NumLocalPlayers < 2 || GameInputMethod == EVRGameInputMethod::GameInput_Default || IgnoreInput())
		return Super::InputAxis(Args);

	if (GameInputMethod == EVRGameInputMethod::GameInput_KeyboardAndMouseToPlayer2)
	{

		int32 ControllerId = 1;

		FPlatformUserId UserId = PLATFORMUSERID_NONE;
		FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
		DeviceMapper.RemapControllerIdToPlatformUserAndDevice(ControllerId, UserId, DeviceId);

		FInputKeyEventArgs NewArgs = Args;
		NewArgs.InputDevice = DeviceId;
		return Super::InputAxis(NewArgs);
	}
	else 
	{
		bool bRetVal = false;
		for (int32 i = 0; i < NumLocalPlayers; i++)
		{
			FPlatformUserId UserId = PLATFORMUSERID_NONE;
			FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
			DeviceMapper.RemapControllerIdToPlatformUserAndDevice(i, UserId, DeviceId);

			FInputKeyEventArgs NewArgs = Args;
			NewArgs.InputDevice = DeviceId;
			bRetVal = Super::InputAxis(NewArgs) || bRetVal;
		}

		return bRetVal;
	}

}