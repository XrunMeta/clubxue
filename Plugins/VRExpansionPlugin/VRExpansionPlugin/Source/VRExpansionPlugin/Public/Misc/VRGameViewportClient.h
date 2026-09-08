

#pragma once
#include "Engine/GameViewportClient.h"

#include "CoreMinimal.h"
#include "InputKeyEventArgs.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#include "VRGameViewportClient.generated.h"

UENUM(Blueprintable)
enum class EVRGameInputMethod : uint8
{
	GameInput_Default,
	GameInput_SharedKeyboardAndMouse,
	GameInput_KeyboardAndMouseToPlayer2,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVROnWindowCloseRequested);

UCLASS(Blueprintable)
class VREXPANSIONPLUGIN_API UVRGameViewportClient : public UGameViewportClient
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "VRExpansionPlugin")
		FVROnWindowCloseRequested BPOnWindowCloseRequested;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRExpansionPlugin")
		bool bIgnoreWindowCloseCommands;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRExpansionPlugin")
		EVRGameInputMethod GameInputMethod;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRExpansionPlugin")
		bool bAlsoChangeGamepPadInput;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRExpansionPlugin")
		TArray<FName> GamepadInputCategories;

	bool IsValidGamePadKey(const FKey& InputKey);

	UFUNCTION()
		bool EventWindowClosing();

	virtual void PostInitProperties() override;
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual bool InputAxis(const FInputKeyEventArgs& Args) override;
};