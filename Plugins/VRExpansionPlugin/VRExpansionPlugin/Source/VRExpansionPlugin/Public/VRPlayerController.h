

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "VRPlayerController.generated.h"

UCLASS()
class VREXPANSIONPLUGIN_API AVRBasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnCameraManagerCreated"), Category = Actor)
		void OnCameraManagerCreated(APlayerCameraManager* CameraManager);

	virtual void SpawnPlayerCameraManager() override
	{
		Super::SpawnPlayerCameraManager();

		if (PlayerCameraManager != NULL && IsLocalController())
		{
			OnCameraManagerCreated(PlayerCameraManager);
		}
	}

};

UCLASS()
class VREXPANSIONPLUGIN_API AVRPlayerController : public AVRBasePlayerController
{
	GENERATED_BODY()

public:
	AVRPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRPlayerController")
		bool bDisableServerUpdateCamera;

	virtual void SpawnPlayerCameraManager() override;

	FRotator LastRotationInput;

	virtual void PlayerTick(float DeltaTime) override;
};

UCLASS(Blueprintable, meta = (ShortTooltip = "Utility class, when set as the default local player it will spawn the target PlayerController class instead as the pending one"))
class VREXPANSIONPLUGIN_API UVRLocalPlayer : public ULocalPlayer
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LocalPlayer")
	TSubclassOf<class APlayerController> OverridePendingLevelPlayerControllerClass;

	virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
	{
		if (OverridePendingLevelPlayerControllerClass)
		{
			PendingLevelPlayerControllerClass = OverridePendingLevelPlayerControllerClass;
		}

		return Super::SpawnPlayActor(URL, OutError, InWorld);
	}
};