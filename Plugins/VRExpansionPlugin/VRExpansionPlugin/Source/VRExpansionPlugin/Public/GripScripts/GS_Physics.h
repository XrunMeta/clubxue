#pragma once

#include "CoreMinimal.h"

#include "VRGripScriptBase.h"
#include "GameFramework/WorldSettings.h"
#include "GripScripts/GS_Default.h"
#include "GS_Physics.generated.h"

UCLASS(NotBlueprintable, ClassGroup = (VRExpansionPlugin), hideCategories = TickSettings)
class VREXPANSIONPLUGIN_API UGS_Physics : public UGS_Default
{
	GENERATED_BODY()
public:

	UGS_Physics(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
		FBPAdvancedPhysicsHandleSettings SingleHandPhysicsSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
		FBPAdvancedPhysicsHandleSettings MultiHandPhysicsSettings;

	void UpdateDualHandInfo(UGripMotionControllerComponent* GrippingController = nullptr, bool bRecreate = true);

	virtual void HandlePostPhysicsHandle(UGripMotionControllerComponent* GrippingController, FBPActorPhysicsHandleInformation* HandleInfo) override;

	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;
	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;
};
