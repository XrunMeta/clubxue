

#include "VRCharacter.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRCharacter)

#include "NavigationSystem.h"
#include "VRBPDatatypes.h"

#include "VRRootComponent.h"
#include "VRCharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Runtime/Launch/Resources/Version.h"
#include "VRPathFollowingComponent.h"
#include "NavFilters/NavigationQueryFilter.h"

DEFINE_LOG_CATEGORY(LogVRCharacter);

AVRCharacter::AVRCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UVRRootComponent>(ACharacter::CapsuleComponentName).SetDefaultSubobjectClass<UVRCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	VRRootReference = NULL;
	if (GetCapsuleComponent())
	{
		VRRootReference = Cast<UVRRootComponent>(GetCapsuleComponent());
		VRRootReference->SetCapsuleSize(20.0f, 96.0f);

		VRRootReference->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		VRRootReference->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	}

	VRMovementReference = NULL;
	if (GetMovementComponent())
	{
		VRMovementReference = Cast<UVRBaseCharacterMovementComponent>(GetMovementComponent());

	}
}

FVector AVRCharacter::GetTeleportLocation(FVector OriginalLocation)
{
	if (!bRetainRoomscale)
	{
		return OriginalLocation + FVector(0.f, 0.f, VRRootReference->GetScaledCapsuleHalfHeight());
	}
	else
	{
		FVector modifier = VRRootReference->OffsetComponentToWorld.GetLocation() - this->GetActorLocation();
		modifier.Z = 0.0f; 
		return OriginalLocation - modifier;
	}
}

bool AVRCharacter::TeleportTo(const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest, bool bNoCheck)
{
	bool bTeleportSucceeded = Super::TeleportTo(DestLocation, DestRotation, bIsATest, bNoCheck);

	if (bTeleportSucceeded)
	{
		NotifyOfTeleport();
	}

	return bTeleportSucceeded;
}

FVector AVRCharacter::GetNavAgentLocation() const
{
	FVector AgentLocation = FNavigationSystem::InvalidLocation;

	if (GetCharacterMovement() != nullptr)
	{
		if (VRMovementReference)
		{
			AgentLocation = VRMovementReference->GetActorFeetLocationVR();
		}
		else
			AgentLocation = GetCharacterMovement()->GetActorFeetLocation();
	}

	if (FNavigationSystem::IsValidLocation(AgentLocation) == false )
	{
		if (VRRootReference)
		{
			AgentLocation = VRRootReference->OffsetComponentToWorld.GetLocation() - FVector(0, 0, VRRootReference->GetScaledCapsuleHalfHeight());
		}
		else if(GetCapsuleComponent() != nullptr)
			AgentLocation = GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	}

	return AgentLocation;
}

void AVRCharacter::ExtendedSimpleMoveToLocation(const FVector& GoalLocation, float AcceptanceRadius, bool bStopOnOverlap, bool bUsePathfinding, bool bProjectDestinationToNavigation, bool bCanStrafe, TSubclassOf<UNavigationQueryFilter> FilterClass, bool bAllowPartialPaths)
{
	UNavigationSystemV1* NavSys = Controller ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(Controller->GetWorld()) : nullptr;
	if (NavSys == nullptr || Controller == nullptr )
	{
		UE_LOGF(LogVRCharacter, Warning, "UVRCharacter::ExtendedSimpleMoveToLocation called for NavSys:%ls Controller:%ls (if any of these is None then there's your problem",
			*GetNameSafe(NavSys), *GetNameSafe(Controller));
		return;
	}

	UPathFollowingComponent* PFollowComp = nullptr;

	if (Controller)
	{

		PFollowComp = Controller->FindComponentByClass<UPathFollowingComponent>();
		if (PFollowComp == nullptr)
		{
			PFollowComp = NewObject<UVRPathFollowingComponent>(Controller);
			PFollowComp->RegisterComponentWithWorld(Controller->GetWorld());
			PFollowComp->Initialize();
		}
	}

	if (PFollowComp == nullptr)
	{
		UE_LOGF(LogVRCharacter, Warning, "ExtendedSimpleMoveToLocation - No PathFollowingComponent Found");
		return;
	}

	if (!PFollowComp->IsPathFollowingAllowed())
	{
		UE_LOGF(LogVRCharacter, Warning, "ExtendedSimpleMoveToLocation - Path Following Movement Is Not Set To Allowed");
		return;
	}

	EPathFollowingReachMode ReachMode;
	if (bStopOnOverlap)
		ReachMode = EPathFollowingReachMode::OverlapAgent;
	else
		ReachMode = EPathFollowingReachMode::ExactLocation;

	bool bAlreadyAtGoal = false;

	if(UVRPathFollowingComponent * pathcomp = Cast<UVRPathFollowingComponent>(PFollowComp))
		bAlreadyAtGoal = pathcomp->HasReached(GoalLocation, ReachMode);
	else
		bAlreadyAtGoal = PFollowComp->HasReached(GoalLocation, ReachMode);

	if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
	{
		if (GetNetMode() == ENetMode::NM_Client)
		{

			PFollowComp->AbortMove(*NavSys, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest
				, FAIRequestID::AnyRequest, EPathFollowingVelocityMode::Reset );
		}
		else
		{
			PFollowComp->AbortMove(*NavSys, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest
				, FAIRequestID::AnyRequest, bAlreadyAtGoal ? EPathFollowingVelocityMode::Reset : EPathFollowingVelocityMode::Keep);
		}
	}

	if (bAlreadyAtGoal)
	{
		PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
	}
	else
	{
		const ANavigationData* NavData = NavSys->GetNavDataForProps(Controller->GetNavAgentPropertiesRef());
		if (NavData)
		{
			FPathFindingQuery Query(Controller, *NavData, Controller->GetNavAgentLocation(), GoalLocation);
			FPathFindingResult Result = NavSys->FindPathSync(Query);
			if (Result.IsSuccessful())
			{
				FAIMoveRequest MoveReq(GoalLocation);
				MoveReq.SetUsePathfinding(bUsePathfinding);
				MoveReq.SetAllowPartialPath(bAllowPartialPaths);
				MoveReq.SetProjectGoalLocation(bProjectDestinationToNavigation);
				MoveReq.SetNavigationFilter(*FilterClass ? FilterClass : DefaultNavigationFilterClass);
				MoveReq.SetAcceptanceRadius(AcceptanceRadius);
				MoveReq.SetReachTestIncludesAgentRadius(bStopOnOverlap);
				MoveReq.SetCanStrafe(bCanStrafe);
				MoveReq.SetReachTestIncludesGoalRadius(true);

				PFollowComp->RequestMove(MoveReq, Result.Path);
			}
			else if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
			{
				PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
			}
		}
	}
}

void AVRCharacter::ZeroToSeatInformation()
{

	SetSeatRelativeLocationAndRotationVR(FVector::ZeroVector);
	NetSmoother->SetRelativeTransform(FTransform(FQuat::Identity, FVector(0.f, 0.f, this->VRRootReference->GetTargetHeightOffset().Z), FVector(1.0f)));

	NotifyOfTeleport();

}

FVector AVRCharacter::GetTargetHeightOffset()
{
	return bRetainRoomscale ? FVector::ZeroVector : VRRootReference->GetTargetHeightOffset();
}

void AVRCharacter::OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	RecalculateBaseEyeHeight();

	K2_OnStartCrouch(HeightAdjust, ScaledHeightAdjust);
}

void AVRCharacter::OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	RecalculateBaseEyeHeight();

	K2_OnEndCrouch(HeightAdjust, ScaledHeightAdjust);
}

void AVRCharacter::RegenerateOffsetComponentToWorld(bool bUpdateBounds, bool bCalculatePureYaw)
{
	if (VRRootReference)
	{
		VRRootReference->GenerateOffsetToWorld(bUpdateBounds, bCalculatePureYaw);
	}
}

void AVRCharacter::SetCharacterSizeVR(float NewRadius, float NewHalfHeight, bool bUpdateOverlaps)
{
	if (VRRootReference)
	{
		VRRootReference->SetCapsuleSizeVR(NewRadius, NewHalfHeight, bUpdateOverlaps);

		if (GetNetMode() < ENetMode::NM_Client)
			ReplicatedCapsuleHeight.CapsuleHeight = VRRootReference->GetUnscaledCapsuleHalfHeight();
	}
	else
	{
		Super::SetCharacterSizeVR(NewRadius, NewHalfHeight, bUpdateOverlaps);
	}
}

void AVRCharacter::SetCharacterHalfHeightVR(float HalfHeight, bool bUpdateOverlaps)
{
	if (VRRootReference)
	{
		VRRootReference->SetCapsuleHalfHeightVR(HalfHeight, bUpdateOverlaps);

		if (GetNetMode() < ENetMode::NM_Client)
			ReplicatedCapsuleHeight.CapsuleHeight = VRRootReference->GetUnscaledCapsuleHalfHeight();
	}
	else
	{
		Super::SetCharacterHalfHeightVR(HalfHeight, bUpdateOverlaps);
	}
}

FVector AVRCharacter::GetProjectedVRLocation() const
{
	if (VRRootReference)
	{
		return OffsetComponentToWorld.TransformPosition(-FVector(VRRootReference->VRCapsuleOffset.X, VRRootReference->VRCapsuleOffset.Y, 0.0f));
	}
	else
	{
		return AVRBaseCharacter::GetProjectedVRLocation();
	}
}
