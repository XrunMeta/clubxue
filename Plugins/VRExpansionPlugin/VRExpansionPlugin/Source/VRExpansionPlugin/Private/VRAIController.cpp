

#include "VRAIController.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRAIController)

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "VRBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "NetworkingDistanceConstants.h" 
#include "Navigation/CrowdFollowingComponent.h"

FVector AVRAIController::GetFocalPointOnActor(const AActor *Actor) const
{
	if (const AVRBaseCharacter * VRChar = Cast<const AVRBaseCharacter>(Actor))
	{
		return VRChar->GetVRLocation_Inline();
	}
	else
		return Super::GetFocalPointOnActor(Actor); 
}

bool AVRAIController::LineOfSightTo(const AActor* Other, FVector ViewPoint, bool bAlternateChecks) const
{
	if (Other == nullptr)
	{
		return false;
	}

	if (ViewPoint.IsZero())
	{
		FRotator ViewRotation;
		GetActorEyesViewPoint(ViewPoint, ViewRotation);

		if (ViewPoint.IsZero())
		{
			return false;
		}
	}

	static FName NAME_LineOfSight = FName(TEXT("LineOfSight"));
	FVector TargetLocation = Other->GetTargetLocation(GetPawn());

	FCollisionQueryParams CollisionParams(NAME_LineOfSight, true, this->GetPawn());
	CollisionParams.AddIgnoredActor(Other);

	bool bHit = GetWorld()->LineTraceTestByChannel(ViewPoint, TargetLocation, ECC_Visibility, CollisionParams);
	if (!bHit)
	{
		return true;
	}

	const APawn * OtherPawn = Cast<const APawn>(Other);
	if (!OtherPawn && Cast<UCapsuleComponent>(Other->GetRootComponent()) == NULL)
	{
		return false;
	}

	const AVRBaseCharacter * VRChar = Cast<const AVRBaseCharacter>(Other);
	const FVector OtherActorLocation = VRChar != nullptr ? VRChar->GetVRLocation_Inline() : Other->GetActorLocation();

	const FVector::FReal DistSq = (OtherActorLocation - ViewPoint).SizeSquared();
	if (DistSq > FARSIGHTTHRESHOLDSQUARED)
	{
		return false;
	}

	if (!OtherPawn && (DistSq > NEARSIGHTTHRESHOLDSQUARED))
	{
		return false;
	}

	float OtherRadius, OtherHeight;
	Other->GetSimpleCollisionCylinder(OtherRadius, OtherHeight);

	if (!bAlternateChecks || !bLOSflag)
	{

		bHit = GetWorld()->LineTraceTestByChannel(ViewPoint, OtherActorLocation + FVector(0.f, 0.f, OtherHeight), ECC_Visibility, CollisionParams);
		if (!bHit)
		{
			return true;
		}
	}

	if (!bSkipExtraLOSChecks && (!bAlternateChecks || bLOSflag))
	{

		if (OtherRadius * OtherRadius / (OtherActorLocation - ViewPoint).SizeSquared() < 0.0001f)
		{
			return false;
		}

		FVector Points[4];
		Points[0] = OtherActorLocation - FVector(OtherRadius, -1 * OtherRadius, 0);
		Points[1] = OtherActorLocation + FVector(OtherRadius, OtherRadius, 0);
		Points[2] = OtherActorLocation - FVector(OtherRadius, OtherRadius, 0);
		Points[3] = OtherActorLocation + FVector(OtherRadius, -1 * OtherRadius, 0);
		int32 IndexMin = 0;
		int32 IndexMax = 0;
		FVector::FReal CurrentMax = (Points[0] - ViewPoint).SizeSquared();
		FVector::FReal CurrentMin = CurrentMax;
		for (int32 PointIndex = 1; PointIndex < 4; PointIndex++)
		{
			const FVector::FReal NextSize = (Points[PointIndex] - ViewPoint).SizeSquared();
			if (NextSize > CurrentMin)
			{
				CurrentMin = NextSize;
				IndexMax = PointIndex;
			}
			else if (NextSize < CurrentMax)
			{
				CurrentMax = NextSize;
				IndexMin = PointIndex;
			}
		}

		for (int32 PointIndex = 0; PointIndex < 4; PointIndex++)
		{
			if ((PointIndex != IndexMin) && (PointIndex != IndexMax))
			{
				bHit = GetWorld()->LineTraceTestByChannel(ViewPoint, Points[PointIndex], ECC_Visibility, CollisionParams);
				if (!bHit)
				{
					return true;
				}
			}
		}
	}
	return false;
}

AVRDetourCrowdAIController::AVRDetourCrowdAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{

}
