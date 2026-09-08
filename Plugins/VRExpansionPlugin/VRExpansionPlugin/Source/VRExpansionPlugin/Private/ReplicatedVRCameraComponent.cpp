

#include "ReplicatedVRCameraComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ReplicatedVRCameraComponent)

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "VRBaseCharacter.h"
#include "VRCharacter.h"
#include "VRRootComponent.h"
#include "IXRTrackingSystem.h"
#include "IXRCamera.h"
#include "Rendering/MotionVectorSimulation.h"

#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

UReplicatedVRCameraComponent::UReplicatedVRCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(true);
	SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	NetUpdateRate = 100.0f; 
	NetUpdateCount = 0.0f;

	bUsePawnControlRotation = false;
	bAutoSetLockToHmd = true;
	bScaleTracking = false;
	TrackingScaler = FVector(1.0f);

	bLimitMinHeight = false;
	MinimumHeightAllowed = 0.0f;
	bLimitMaxHeight = false;
	MaxHeightAllowed = 300.f;
	bLimitBounds = false;

	MaximumTrackedBounds = 1028;

	bSetPositionDuringTick = false;
	bLerpingPosition = false;
	bReppedOnce = false;

	OverrideSendTransform = nullptr;

	LastRelativePosition = FTransform::Identity;
	bSampleVelocityInWorldSpace = false;
	bHadValidFirstVelocity = false;

}

void UReplicatedVRCameraComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeLocation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeRotation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_SkipOwner, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UReplicatedVRCameraComponent, ReplicatedCameraTransform, PushModelParamsWithCondition);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UReplicatedVRCameraComponent, NetUpdateRate, PushModelParams);

}

void UReplicatedVRCameraComponent::Server_SendCameraTransform_Implementation(FBPVRComponentPosRep NewTransform)
{

	ReplicatedCameraTransform = NewTransform;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UReplicatedVRCameraComponent, ReplicatedCameraTransform, this);
#endif

	if (!bHasAuthority)
	{
		OnRep_ReplicatedCameraTransform();
	}
}

bool UReplicatedVRCameraComponent::Server_SendCameraTransform_Validate(FBPVRComponentPosRep NewTransform)
{
	return true;

}

void UReplicatedVRCameraComponent::OnAttachmentChanged()
{
	if (AVRCharacter* CharacterOwner = Cast<AVRCharacter>(this->GetOwner()))
	{
		AttachChar = CharacterOwner;
	}
	else
	{
		AttachChar = nullptr;
	}

	Super::OnAttachmentChanged();
}

bool UReplicatedVRCameraComponent::HasTrackingParameters()
{
	return  bScaleTracking || bLimitMaxHeight || bLimitMinHeight || bLimitBounds || (AttachChar && !AttachChar->bRetainRoomscale);
}

void UReplicatedVRCameraComponent::ApplyTrackingParameters(FVector &OriginalPosition, bool bSkipLocZero)
{

	if (!bSkipLocZero && (AttachChar && !AttachChar->bRetainRoomscale))
	{
		OriginalPosition.X = 0;
		OriginalPosition.Y = 0;	
	}

	if (bLimitBounds)
	{
		OriginalPosition.X = FMath::Clamp(OriginalPosition.X, -MaximumTrackedBounds, MaximumTrackedBounds);
		OriginalPosition.Y = FMath::Clamp(OriginalPosition.Y, -MaximumTrackedBounds, MaximumTrackedBounds);
	}

	if (bScaleTracking)
	{
		OriginalPosition *= TrackingScaler;
	}

	if (bLimitMaxHeight)
	{
		OriginalPosition.Z = FMath::Min(MaxHeightAllowed, OriginalPosition.Z);
	}

	if (bLimitMinHeight)
	{
		OriginalPosition.Z = FMath::Max(MinimumHeightAllowed, OriginalPosition.Z);
	}
}

void UReplicatedVRCameraComponent::UpdateTracking(float DeltaTime)
{
	bHasAuthority = IsLocallyControlled();

	if (bHasAuthority)
	{

		if (bSetPositionDuringTick && bLockToHmd && GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowedForWorld(*GetWorld()))
		{

			FQuat Orientation;
			FVector Position;
			if (GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, Orientation, Position))
			{
				if (HasTrackingParameters())
				{
					ApplyTrackingParameters(Position, true);
				}

				ReplicatedCameraTransform.Position = Position;
				ReplicatedCameraTransform.Rotation = Orientation.Rotator();

				if (IsValid(AttachChar) && !AttachChar->bRetainRoomscale)
				{	

					Position.X = 0.0f;
					Position.Y = 0.0f;

					FRotator StoredCameraRotOffset = FRotator::ZeroRotator;
					if (AttachChar->VRMovementReference && AttachChar->VRMovementReference->GetReplicatedMovementMode() == EVRConjoinedMovementModes::C_VRMOVE_Seated)
					{

					}
					else
					{
						StoredCameraRotOffset = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(Orientation.Rotator());
					}

					Position += StoredCameraRotOffset.RotateVector(FVector(-AttachChar->VRRootReference->VRCapsuleOffset.X, -AttachChar->VRRootReference->VRCapsuleOffset.Y, 0.0f));

				}

				SetRelativeTransform(FTransform(Orientation, Position));
			}
		}
	}
	else
	{

		RunNetworkedSmoothing(DeltaTime);
	}

	if(bHadValidFirstVelocity || !LastRelativePosition.Equals(FTransform::Identity))
	{ 
		bHadValidFirstVelocity = true;
		ComponentVelocity = ((bSampleVelocityInWorldSpace ? GetComponentLocation() : GetRelativeLocation()) - LastRelativePosition.GetTranslation()) / DeltaTime;
	}

	LastRelativePosition = bSampleVelocityInWorldSpace ? this->GetComponentTransform() : this->GetRelativeTransform();
}

void UReplicatedVRCameraComponent::RunNetworkedSmoothing(float DeltaTime)
{
	FVector RetainPositionOffset(0.0f, 0.0f, ReplicatedCameraTransform.Position.Z);

	if (AttachChar && !AttachChar->bRetainRoomscale)
	{
		FRotator StoredCameraRotOffset = FRotator::ZeroRotator;
		if (AttachChar->VRMovementReference && AttachChar->VRMovementReference->GetReplicatedMovementMode() == EVRConjoinedMovementModes::C_VRMOVE_Seated)
		{

		}
		else
		{
			StoredCameraRotOffset = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(ReplicatedCameraTransform.Rotation);
		}

		RetainPositionOffset += StoredCameraRotOffset.RotateVector(FVector(-AttachChar->VRRootReference->VRCapsuleOffset.X, -AttachChar->VRRootReference->VRCapsuleOffset.Y, 0.0f));
	}

	if (bLerpingPosition)
	{
		if (!bUseExponentialSmoothing)
		{
			NetUpdateCount += DeltaTime;
			float LerpVal = FMath::Clamp(NetUpdateCount / (1.0f / NetUpdateRate), 0.0f, 1.0f);

			if (LerpVal >= 1.0f)
			{
				if (AttachChar && !AttachChar->bRetainRoomscale)
				{
					SetRelativeLocationAndRotation(RetainPositionOffset, ReplicatedCameraTransform.Rotation);
				}
				else
				{
					SetRelativeLocationAndRotation(ReplicatedCameraTransform.Position, ReplicatedCameraTransform.Rotation);
				}

				bLerpingPosition = false;
				NetUpdateCount = 0.0f;
			}
			else
			{

				if (AttachChar && !AttachChar->bRetainRoomscale)
				{

					SetRelativeLocationAndRotation(
						FMath::Lerp(LastUpdatesRelativePosition, RetainPositionOffset, LerpVal),
						FMath::Lerp(LastUpdatesRelativeRotation, ReplicatedCameraTransform.Rotation, LerpVal)
					);
				}
				else
				{

					SetRelativeLocationAndRotation(
						FMath::Lerp(LastUpdatesRelativePosition, (FVector)ReplicatedCameraTransform.Position, LerpVal),
						FMath::Lerp(LastUpdatesRelativeRotation, ReplicatedCameraTransform.Rotation, LerpVal)
					);
				}
			}
		}
		else 
		{
			if (InterpolationSpeed <= 0.f)
			{
				if (AttachChar && !AttachChar->bRetainRoomscale)
				{
					SetRelativeLocationAndRotation(RetainPositionOffset, ReplicatedCameraTransform.Rotation);
				}
				else
				{
					SetRelativeLocationAndRotation((FVector)ReplicatedCameraTransform.Position, ReplicatedCameraTransform.Rotation);
				}

				bLerpingPosition = false;
				return;
			}

			const float Alpha = FMath::Clamp(DeltaTime * InterpolationSpeed, 0.f, 1.f);

			FTransform NA = FTransform(GetRelativeRotation(), GetRelativeLocation(), FVector(1.0f));
			FTransform NB = FTransform::Identity;

			if (AttachChar && !AttachChar->bRetainRoomscale)
			{
				NB = FTransform(ReplicatedCameraTransform.Rotation, RetainPositionOffset, FVector(1.0f));
			}
			else
			{
				NB = FTransform(ReplicatedCameraTransform.Rotation, (FVector)ReplicatedCameraTransform.Position, FVector(1.0f));
			}

			NA.NormalizeRotation();
			NB.NormalizeRotation();

			NA.Blend(NA, NB, Alpha);

			if (NA.EqualsNoScale(NB))
			{
				if (AttachChar && !AttachChar->bRetainRoomscale)
				{
					SetRelativeLocationAndRotation(RetainPositionOffset, ReplicatedCameraTransform.Rotation);
				}
				else
				{ 
					SetRelativeLocationAndRotation(ReplicatedCameraTransform.Position, ReplicatedCameraTransform.Rotation);
				}
			}
			else 
			{
				SetRelativeLocationAndRotation(NA.GetTranslation(), NA.Rotator());
			}
		}
	}
}

void UReplicatedVRCameraComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bUpdateInCharacterMovement || !IsValid(AttachChar))
	{
		UpdateTracking(DeltaTime);
	}
	else
	{
		UCharacterMovementComponent* CharMove = AttachChar->GetCharacterMovement();
		if (!CharMove || !CharMove->IsComponentTickEnabled() || !CharMove->IsActive() || (!CharMove->PrimaryComponentTick.bTickEvenWhenPaused && GetWorld()->IsPaused()))
		{

			UpdateTracking(DeltaTime);
		}
	}

	if (bHasAuthority)
	{

		if (this->GetIsReplicated())
		{
			FRotator RelativeRot = GetRelativeRotation();
			FVector RelativeLoc = GetRelativeLocation();

			if (!RelativeLoc.Equals(LastUpdatesRelativePosition) || !RelativeRot.Equals(LastUpdatesRelativeRotation))
			{
				NetUpdateCount += DeltaTime;

				if (NetUpdateCount >= (1.0f / NetUpdateRate))
				{
					NetUpdateCount = 0.0f;

					if (bFPSDebugMode)
					{
						ReplicatedCameraTransform.Position = RelativeLoc;
						ReplicatedCameraTransform.Rotation = RelativeRot;
					}

#if WITH_PUSH_MODEL
					MARK_PROPERTY_DIRTY_FROM_NAME(UReplicatedVRCameraComponent, ReplicatedCameraTransform, this);
#endif

					if (GetNetMode() == NM_Client)
					{
						AVRBaseCharacter* OwningChar = Cast<AVRBaseCharacter>(GetOwner());
						if (OverrideSendTransform != nullptr && OwningChar != nullptr)
						{
							(OwningChar->* (OverrideSendTransform))(ReplicatedCameraTransform);
						}
						else
						{

							Server_SendCameraTransform(ReplicatedCameraTransform);
						}
					}

					LastUpdatesRelativeRotation = RelativeRot;
					LastUpdatesRelativePosition = RelativeLoc;
				}
			}
		}
	}
}

void UReplicatedVRCameraComponent::HandleXRCamera(float DeltaTime)
{
	bool bIsLocallyControlled = IsLocallyControlled();

	if (bAutoSetLockToHmd)
	{
		if (bIsLocallyControlled)
			bLockToHmd = true;
		else
			bLockToHmd = false;
	}

	if (bIsLocallyControlled && GEngine && GEngine->XRSystem.IsValid() && GetWorld() && GetWorld()->WorldType != EWorldType::Editor)
	{
		IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
		auto XRCamera = XRSystem->GetXRCamera();

		if (XRCamera.IsValid())
		{
			if (XRSystem->IsHeadTrackingAllowedForWorld(*GetWorld()))
			{
				const FTransform ParentWorld = CalcNewComponentToWorld(FTransform());
				XRCamera->SetupLateUpdate(ParentWorld, this, bLockToHmd == 0);

				if (bLockToHmd)
				{
					FQuat Orientation;
					FVector Position;
					if (XRCamera->UpdatePlayerCamera(Orientation, Position, DeltaTime))
					{
						if (HasTrackingParameters())
						{
							ApplyTrackingParameters(Position, true);
						}

						ReplicatedCameraTransform.Position = Position;
						ReplicatedCameraTransform.Rotation = Orientation.Rotator();

						if (IsValid(AttachChar) && !AttachChar->bRetainRoomscale)
						{

							Position.X = 0.0f;
							Position.Y = 0.0f;

							if (AttachChar->VRMovementReference && AttachChar->VRMovementReference->GetReplicatedMovementMode() != EVRConjoinedMovementModes::C_VRMOVE_Seated)
							{

								FRotator StoredCameraRotOffset = FRotator::ZeroRotator;
								if (AttachChar->VRMovementReference->GetReplicatedMovementMode() == EVRConjoinedMovementModes::C_VRMOVE_Seated)
								{

								}
								else
								{
									StoredCameraRotOffset = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(Orientation.Rotator());
								}

								Position += StoredCameraRotOffset.RotateVector(FVector(-AttachChar->VRRootReference->VRCapsuleOffset.X, -AttachChar->VRRootReference->VRCapsuleOffset.Y, 0.0f));
							}
						}

						SetRelativeTransform(FTransform(Orientation, Position));
					}
					else
					{
						SetRelativeScale3D(FVector(1.0f));

					}
				}

				XRCamera->OverrideFOV(this->FieldOfView);
			}
		}
	}
}

FTransform UReplicatedVRCameraComponent::GetHMDTrackingTransform()
{
	return FTransform(ReplicatedCameraTransform.Rotation, ReplicatedCameraTransform.Position);
}

void UReplicatedVRCameraComponent::OnRep_ReplicatedCameraTransform()
{
    if (GetNetMode() < ENetMode::NM_Client && HasTrackingParameters())
    {

        ApplyTrackingParameters(ReplicatedCameraTransform.Position, true);
    }

	FVector CameraPosition = ReplicatedCameraTransform.Position;
	if (AttachChar && !AttachChar->bRetainRoomscale)
	{
		CameraPosition.X = 0;
		CameraPosition.Y = 0;

		FRotator StoredCameraRotOffset = FRotator::ZeroRotator;
		if (AttachChar->VRMovementReference && AttachChar->VRMovementReference->GetReplicatedMovementMode() == EVRConjoinedMovementModes::C_VRMOVE_Seated)
		{

		}
		else
		{
			StoredCameraRotOffset = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(ReplicatedCameraTransform.Rotation);
		}

		CameraPosition += StoredCameraRotOffset.RotateVector(FVector(-AttachChar->VRRootReference->VRCapsuleOffset.X, -AttachChar->VRRootReference->VRCapsuleOffset.Y, 0.0f));

	}

    if (bSmoothReplicatedMotion)
    {
        if (bReppedOnce)
        {
            bLerpingPosition = true;
            NetUpdateCount = 0.0f;
            LastUpdatesRelativePosition = this->GetRelativeLocation();
            LastUpdatesRelativeRotation = this->GetRelativeRotation();

			if (bUseExponentialSmoothing)
			{
				FVector OldToNewVector = CameraPosition - LastUpdatesRelativePosition;
				float NewDistance = OldToNewVector.SizeSquared();

				if (NewDistance >= FMath::Square(NetworkNoSmoothUpdateDistance))
				{
					SetRelativeLocationAndRotation(CameraPosition, ReplicatedCameraTransform.Rotation);
					bLerpingPosition = false;
				}

				else if (NewDistance >= FMath::Square(NetworkMaxSmoothUpdateDistance))
				{
					FVector Offset = (OldToNewVector.Size() - NetworkMaxSmoothUpdateDistance) * OldToNewVector.GetSafeNormal();
					SetRelativeLocation(LastUpdatesRelativePosition + Offset);
				}
			}
        }
        else
        {
            SetRelativeLocationAndRotation(CameraPosition, ReplicatedCameraTransform.Rotation);
            bReppedOnce = true;
        }
    }
    else
        SetRelativeLocationAndRotation(CameraPosition, ReplicatedCameraTransform.Rotation);
}

void UReplicatedVRCameraComponent::SetNetUpdateRate(float NewNetUpdateRate)
{
	NetUpdateRate = NewNetUpdateRate;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UReplicatedVRCameraComponent, NetUpdateRate, this);
#endif
}

bool UReplicatedVRCameraComponent::IsLocallyControlled() const
{

	const AActor* MyOwner = GetOwner();
	return MyOwner->HasLocalNetOwner();
}