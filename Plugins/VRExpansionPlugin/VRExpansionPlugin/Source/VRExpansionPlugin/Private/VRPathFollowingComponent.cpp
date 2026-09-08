

#include "VRPathFollowingComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRPathFollowingComponent)

#include "AI/Navigation/NavigationTypes.h"
#include "AI/Navigation/PathFollowingAgentInterface.h"

#include "CoreMinimal.h"
#include "Engine/World.h"

#include "Navigation/MetaNavMeshPath.h"
#include "NavLinkCustomInterface.h"

DEFINE_LOG_CATEGORY(LogPathFollowingVR);

void UVRPathFollowingComponent::SetNavMovementInterface(INavMovementInterface* NavMoveInterface)
{
	Super::SetNavMovementInterface(NavMoveInterface);

	if (NavMovementInterface.IsValid() && NavMovementInterface->GetOwnerAsObject())
	{
		if (AVRBaseCharacter* VRMovement = Cast<AVRBaseCharacter>(NavMovementInterface->GetOwnerAsObject()))
		{ 
			if (IsValid(VRMovement->VRMovementReference))
			{
				OnRequestFinished.AddUObject(VRMovement->VRMovementReference, &UVRBaseCharacterMovementComponent::OnMoveCompleted);
			}
		}
	}
}

void UVRPathFollowingComponent::GetDebugStringTokens(TArray<FString>& Tokens, TArray<EPathFollowingDebugTokens::Type>& Flags) const
{
	Tokens.Add(GetStatusDesc());
	Flags.Add(EPathFollowingDebugTokens::Description);

	if (GetStatus() != EPathFollowingStatus::Moving)
	{
		return;
	}

	FString& StatusDesc = Tokens[0];
	if (Path.IsValid())
	{
		const int32 NumMoveSegments = (Path.IsValid() && Path->IsValid()) ? Path->GetPathPoints().Num() : -1;
		const bool bIsDirect = (Path->CastPath<FAbstractNavigationPath>() != NULL);
		const bool bIsCustomLink = CurrentCustomLinkOb.IsValid();

		if (!bIsDirect)
		{
			StatusDesc += FString::Printf(TEXT(" (%d..%d/%d)%s"), MoveSegmentStartIndex + 1, MoveSegmentEndIndex + 1, NumMoveSegments,
				bIsCustomLink ? TEXT(" (custom NavLink)") : TEXT(""));
		}
		else
		{
			StatusDesc += TEXT(" (direct)");
		}
	}
	else
	{
		StatusDesc += TEXT(" (invalid path)");
	}

	float CurrentDot = 0.0f, CurrentDistance = 0.0f, CurrentHeight = 0.0f;
	uint8 bFailedDot = 0, bFailedDistance = 0, bFailedHeight = 0;
	DebugReachTest(CurrentDot, CurrentDistance, CurrentHeight, bFailedHeight, bFailedDistance, bFailedHeight);

	Tokens.Add(TEXT("dot"));
	Flags.Add(EPathFollowingDebugTokens::ParamName);
	Tokens.Add(FString::Printf(TEXT("%.2f"), CurrentDot));
	Flags.Add(bFailedDot ? EPathFollowingDebugTokens::FailedValue : EPathFollowingDebugTokens::PassedValue);

	Tokens.Add(TEXT("dist2D"));
	Flags.Add(EPathFollowingDebugTokens::ParamName);
	Tokens.Add(FString::Printf(TEXT("%.0f"), CurrentDistance));
	Flags.Add(bFailedDistance ? EPathFollowingDebugTokens::FailedValue : EPathFollowingDebugTokens::PassedValue);

	Tokens.Add(TEXT("distZ"));
	Flags.Add(EPathFollowingDebugTokens::ParamName);
	Tokens.Add(FString::Printf(TEXT("%.0f"), CurrentHeight));
	Flags.Add(bFailedHeight ? EPathFollowingDebugTokens::FailedValue : EPathFollowingDebugTokens::PassedValue);
}

