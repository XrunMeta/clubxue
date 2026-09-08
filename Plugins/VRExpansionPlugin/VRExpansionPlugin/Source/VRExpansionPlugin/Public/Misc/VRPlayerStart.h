#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Components/SceneComponent.h"
#include "VRPlayerStart.generated.h"

UCLASS(Blueprintable, ClassGroup = Common, hidecategories = Collision)
class VREXPANSIONPLUGIN_API AVRPlayerStart : public APlayerStart
{
	GENERATED_BODY()
private:
	UPROPERTY()
    TObjectPtr<USceneComponent> VRRootComp;
public:

	AVRPlayerStart(const FObjectInitializer& ObjectInitializer);

	class USceneComponent* GetVRRootComponent() const { return VRRootComp; }

	virtual void GetSimpleCollisionCylinder(float& CollisionRadius, float& CollisionHalfHeight) const override;
	virtual void FindBase() override;
	virtual void Validate() override;
};