

#pragma once
#include "CoreMinimal.h"
#include "AIController.h"

#include "VRAIController.generated.h"

UCLASS()
class VREXPANSIONPLUGIN_API AVRAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual FVector GetFocalPointOnActor(const AActor *Actor) const override;

	virtual bool LineOfSightTo(const AActor* Other, FVector ViewPoint = FVector(ForceInit), bool bAlternateChecks = false) const override;

};

UCLASS()
class AVRDetourCrowdAIController : public AVRAIController
{
	GENERATED_BODY()
public:
	AVRDetourCrowdAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};