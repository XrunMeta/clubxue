

#include "VRRootComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRRootComponent)

#include "PhysicsPublic.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/ScopedMovementUpdate.h"
#include "SceneManagement.h"
#include "PrimitiveSceneProxy.h"
#include "SceneView.h"

#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "VRCharacter.h"
#include "Engine/OverlapResult.h"
#include "Algo/Copy.h"
#include "AI/Navigation/NavigationRelevantData.h"

#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY(LogVRRootComponent);
#define LOCTEXT_NAMESPACE "VRRootComponent"

DECLARE_CYCLE_STAT(TEXT("VRRootMovement"), STAT_VRRootMovement, STATGROUP_VRRootComponent);
DECLARE_CYCLE_STAT(TEXT("PerformOverlapQueryVR Time"), STAT_PerformOverlapQueryVR, STATGROUP_VRRootComponent);
DECLARE_CYCLE_STAT(TEXT("UpdateOverlapsVRRoot Time"), STAT_UpdateOverlapsVRRoot, STATGROUP_VRRootComponent);

typedef TArray<const FOverlapInfo*, TInlineAllocator<8>> TInlineOverlapPointerArray;

FORCEINLINE_DEBUGGABLE static bool CanComponentsGenerateOverlap(const UPrimitiveComponent* MyComponent,  UPrimitiveComponent* OtherComp)
{
	return OtherComp
		&& OtherComp->GetGenerateOverlapEvents()
		&& MyComponent
		&& MyComponent->GetGenerateOverlapEvents()
		&& MyComponent->GetCollisionResponseToComponent(OtherComp) == ECR_Overlap;
}

struct FPredicateFilterCanOverlap
{
	FPredicateFilterCanOverlap(const UPrimitiveComponent& OwningComponent)
		: MyComponent(OwningComponent)
	{
	}

	bool operator() (const FOverlapInfo& Info) const
	{
		return CanComponentsGenerateOverlap(&MyComponent, Info.OverlapInfo.GetComponent());
	}

private:
	const UPrimitiveComponent& MyComponent;
};

struct FPredicateFilterCannotOverlap
{
	FPredicateFilterCannotOverlap(const UPrimitiveComponent& OwningComponent)
		: MyComponent(OwningComponent)
	{
	}

	bool operator() (const FOverlapInfo& Info) const
	{
		return !CanComponentsGenerateOverlap(&MyComponent, Info.OverlapInfo.GetComponent());
	}

private:
	const UPrimitiveComponent& MyComponent;
};

template <class ElementType, class AllocatorType1, class AllocatorType2>
FORCEINLINE_DEBUGGABLE static void GetPointersToArrayData(TArray<const ElementType*, AllocatorType1>& Pointers, const TArray<ElementType, AllocatorType2>& DataArray)
{
	const int32 NumItems = DataArray.Num();
	Pointers.SetNumUninitialized(NumItems);
	for (int32 i = 0; i < NumItems; i++)
	{
		Pointers[i] = &(DataArray[i]);
	}
}

template <class ElementType, class AllocatorType1>
FORCEINLINE_DEBUGGABLE static void GetPointersToArrayData(TArray<const ElementType*, AllocatorType1>& Pointers, const TArrayView<const ElementType>& DataArray)
{
	const int32 NumItems = DataArray.Num();
	Pointers.SetNumUninitialized(NumItems);
	for (int32 i = 0; i < NumItems; i++)
	{
		Pointers[i] = &(DataArray[i]);
	}
}

template <class ElementType, class AllocatorType1, class AllocatorType2, typename PredicateT>
FORCEINLINE_DEBUGGABLE static void GetPointersToArrayDataByPredicate(TArray<const ElementType*, AllocatorType1>& Pointers, const TArray<ElementType, AllocatorType2>& DataArray, PredicateT Predicate)
{
	Pointers.Reserve(Pointers.Num() + DataArray.Num());
	for (const ElementType& Item : DataArray)
	{
		if (Invoke(Predicate, Item))
		{
			Pointers.Add(&Item);
		}
	}
}

template <class ElementType, class AllocatorType1, typename PredicateT>
FORCEINLINE_DEBUGGABLE static void GetPointersToArrayDataByPredicate(TArray<const ElementType*, AllocatorType1>& Pointers, const TArrayView<const ElementType>& DataArray, PredicateT Predicate)
{
	Pointers.Reserve(Pointers.Num() + DataArray.Num());
	for (const ElementType& Item : DataArray)
	{
		if (Invoke(Predicate, Item))
		{
			Pointers.Add(&Item);
		}
	}
}

#define PERF_MOVECOMPONENT_STATS 0

namespace PrimitiveComponentStatics
{

	static const FName MoveComponentName(TEXT("MoveComponent"));
	static const FName UpdateOverlapsName(TEXT("UpdateOverlaps"));
}

template<class AllocatorType>
FORCEINLINE_DEBUGGABLE int32 IndexOfOverlapFast(const TArray<FOverlapInfo, AllocatorType>& OverlapArray, const FOverlapInfo& SearchItem)
{
	return OverlapArray.IndexOfByPredicate(FFastOverlapInfoCompare(SearchItem));
}

template<class AllocatorType>
FORCEINLINE_DEBUGGABLE int32 IndexOfOverlapFast(const TArray<const FOverlapInfo*, AllocatorType>& OverlapPtrArray, const FOverlapInfo* SearchItem)
{
	return OverlapPtrArray.IndexOfByPredicate(FFastOverlapInfoCompare(*SearchItem));
}

template<class AllocatorType>
FORCEINLINE_DEBUGGABLE void AddUniqueOverlapFast(TArray<FOverlapInfo, AllocatorType>& OverlapArray, FOverlapInfo& NewOverlap)
{
	if (IndexOfOverlapFast(OverlapArray, NewOverlap) == INDEX_NONE)
	{
		OverlapArray.Add(NewOverlap);
	}
}

template<class AllocatorType>
FORCEINLINE_DEBUGGABLE void AddUniqueOverlapFast(TArray<FOverlapInfo, AllocatorType>& OverlapArray, FOverlapInfo&& NewOverlap)
{
	if (IndexOfOverlapFast(OverlapArray, NewOverlap) == INDEX_NONE)
	{
		OverlapArray.Add(NewOverlap);
	}
}

static void PullBackHit(FHitResult& Hit, const FVector& Start, const FVector& End, const float Dist)
{
	const float DesiredTimeBack = FMath::Clamp(0.1f, 0.1f / Dist, 1.f / Dist) + 0.001f;
	Hit.Time = FMath::Clamp(Hit.Time - DesiredTimeBack, 0.f, 1.f);
}

static bool ShouldIgnoreHitResult(const UWorld* InWorld, bool bAllowSimulatingCollision, FHitResult const& TestHit, FVector const& MovementDirDenormalized, const AActor* MovingActor, EMoveComponentFlags MoveFlags)
{
	if (TestHit.bBlockingHit)
	{

		if (!bAllowSimulatingCollision && TestHit.Component.IsValid() && TestHit.Component->IsSimulatingPhysics())
			return true;

		if ((MoveFlags & MOVECOMP_IgnoreBases) && MovingActor)	
		{

			AActor const* const HitActor = TestHit.HitObjectHandle.FetchActor();
			if (HitActor)
			{
				if (MovingActor->IsBasedOnActor(HitActor) || HitActor->IsBasedOnActor(MovingActor))
				{
					return true;
				}
			}
		}

		static const auto CVarHitDistanceTolerance = IConsoleManager::Get().FindConsoleVariable(TEXT("p.HitDistanceTolerance"));
		if ((TestHit.Distance < CVarHitDistanceTolerance->GetFloat() || TestHit.bStartPenetrating) && !(MoveFlags & MOVECOMP_NeverIgnoreBlockingOverlaps))
		{
			static const auto CVarInitialOverlapTolerance = IConsoleManager::Get().FindConsoleVariable(TEXT("p.InitialOverlapTolerance"));
			const float DotTolerance = CVarInitialOverlapTolerance->GetFloat();

			const FVector MovementDir = MovementDirDenormalized.GetSafeNormal();
			const float MoveDot = (TestHit.ImpactNormal | MovementDir);

			const bool bMovingOut = MoveDot > DotTolerance;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

			static const auto CVarShowInitialOverlaps = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ShowInitialOverlaps"));
			if (CVarShowInitialOverlaps->GetInt() != 0)
			{
				UE_LOGF(LogVRRootComponent, Log, "Overlapping %ls Dir %ls Dot %f Normal %ls Depth %f", *GetNameSafe(TestHit.Component.Get()), *MovementDir.ToString(), MoveDot, *TestHit.ImpactNormal.ToString(), TestHit.PenetrationDepth);
				DrawDebugDirectionalArrow(InWorld, TestHit.TraceStart, TestHit.TraceStart + 30.f * TestHit.ImpactNormal, 5.f, bMovingOut ? FColor(64, 128, 255) : FColor(255, 64, 64), true, 4.f);
				if (TestHit.PenetrationDepth > UE_KINDA_SMALL_NUMBER)
				{
					DrawDebugDirectionalArrow(InWorld, TestHit.TraceStart, TestHit.TraceStart + TestHit.PenetrationDepth * TestHit.Normal, 5.f, FColor(64, 255, 64), true, 4.f);
				}
			}

#endif

			if (bMovingOut)
			{
				return true;
			}
		}
	}

	return false;
}
static FORCEINLINE_DEBUGGABLE bool ShouldIgnoreOverlapResult(const UWorld* World, const AActor* ThisActor, const UPrimitiveComponent& ThisComponent, const AActor* OtherActor, const UPrimitiveComponent& OtherComponent, bool bCheckOverlapFlags)
{

	if (&ThisComponent == &OtherComponent)
	{
		return true;
	}

	if (bCheckOverlapFlags)
	{

		if (!ThisComponent.GetGenerateOverlapEvents() || !OtherComponent.GetGenerateOverlapEvents())
		{
			return true;
		}
	}

	if (!ThisActor || !OtherActor)
	{
		return true;
	}

	if (!World || OtherActor == (AActor*)World->GetWorldSettings() || !OtherActor->IsActorInitialized())
	{
		return true;
	}

	return false;
}

UVRRootComponent::UVRRootComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	bWantsInitializeComponent = true;

	this->SetRelativeScale3D(FVector(1.f));
	this->SetRelativeLocation(FVector::ZeroVector);

	VRCapsuleOffset = FVector(-8.0f, 0.0f, 2.15f );

	bCenterCapsuleOnHMD = false;
	bPauseTracking = false;

	ShapeColor = FColor(223, 149, 157, 255);

	CapsuleRadius = 20.0f;
	CapsuleHalfHeight = 96.0f;
	bUseEditorCompositing = true;
	OffsetComponentToWorld = FTransform(FQuat(0.0f,0.0f,0.0f,1.0f), FVector::ZeroVector, FVector(1.0f));

	lastCameraLoc = FVector::ZeroVector;
	lastCameraRot = FRotator::ZeroRotator;
	curCameraRot = FRotator::ZeroRotator;
	curCameraLoc = FVector::ZeroVector;
	StoredCameraRotOffset = FRotator::ZeroRotator;
	TargetPrimitiveComponent = NULL;
	owningVRChar = NULL;

	bAllowSimulatingCollision = false;
	bUseWalkingCollisionOverride = false;
	WalkingCollisionOverride = ECollisionChannel::ECC_Pawn;

	bCalledUpdateTransform = false;

	CanCharacterStepUpOn = ECB_No;

	SetCanEverAffectNavigation(false);
	bDynamicObstacle = true;

	LineThickness = 1.25f;

}

class FDrawVRCylinderSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FDrawVRCylinderSceneProxy(const UVRRootComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, bDrawOnlyIfSelected(InComponent->bDrawOnlyIfSelected)
		, CapsuleRadius(InComponent->GetScaledCapsuleRadius())
		, CapsuleHalfHeight(InComponent->GetScaledCapsuleHalfHeight())
		, ShapeColor(InComponent->ShapeColor)
		, VRCapsuleOffset(InComponent->VRCapsuleOffset)
		, bSimulating(false)

		, LocalToWorld(InComponent->OffsetComponentToWorld.ToMatrixWithScale())
		, LineThickness(InComponent->GetLineThickness())
	{
		bWillEverBeLit = false;
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_GetDynamicMeshElements_DrawDynamicElements);

		const int32 CapsuleSides = FMath::Clamp<int32>(CapsuleRadius / 4.f, 16, 64);

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{

			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				const FLinearColor DrawCapsuleColor = GetViewSelectionColor(ShapeColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

				if (bSimulating)
				{
					DrawWireCapsule(PDI, LocalToWorld.GetOrigin() - FVector(0.f, 0.f, CapsuleHalfHeight), LocalToWorld.GetUnitAxis(EAxis::X), LocalToWorld.GetUnitAxis(EAxis::Y), LocalToWorld.GetUnitAxis(EAxis::Z), DrawCapsuleColor, CapsuleRadius, CapsuleHalfHeight, CapsuleSides, SDPG_World, LineThickness);
				}
				else if (UseEditorCompositing(View))
				{
					DrawWireCapsule(PDI, LocalToWorld.GetOrigin() , LocalToWorld.GetUnitAxis(EAxis::X), LocalToWorld.GetUnitAxis(EAxis::Y), LocalToWorld.GetUnitAxis(EAxis::Z), DrawCapsuleColor, CapsuleRadius, CapsuleHalfHeight, CapsuleSides, SDPG_World, LineThickness);
				}
				else
					DrawWireCapsule(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetUnitAxis(EAxis::X), LocalToWorld.GetUnitAxis(EAxis::Y), LocalToWorld.GetUnitAxis(EAxis::Z), DrawCapsuleColor, CapsuleRadius, CapsuleHalfHeight, CapsuleSides, SDPG_World, LineThickness);
			}
		}
	}

	void UpdateTransform_RenderThread(const FTransform &NewTransform, float NewHalfHeight, bool bIsSimulating)
	{
		check(IsInRenderingThread());
		LocalToWorld = NewTransform.ToMatrixWithScale();

		CapsuleHalfHeight = NewHalfHeight;
		bSimulating = bIsSimulating;
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		const bool bProxyVisible = !bDrawOnlyIfSelected || IsSelected();

		const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = (IsShown(View) && bProxyVisible) || bShowForCollision;
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		return Result;
	}
	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

private:
	const uint32	bDrawOnlyIfSelected : 1;
	const float		CapsuleRadius;
	float		CapsuleHalfHeight;
	FColor	ShapeColor;
	const FVector VRCapsuleOffset;
	bool bSimulating = false;

	FMatrix LocalToWorld;
	const float	LineThickness;
};

FPrimitiveSceneProxy* UVRRootComponent::CreateSceneProxy()
{
	return new FDrawVRCylinderSceneProxy(this);
}

void UVRRootComponent::InitializeComponent()
{
	Super::InitializeComponent();
	GenerateOffsetToWorld();
}

void UVRRootComponent::BeginPlay()
{
	Super::BeginPlay();

	if(AVRBaseCharacter * vrOwner = Cast<AVRBaseCharacter>(this->GetOwner()))
	{
		if (vrOwner->VRReplicatedCamera)
		{
			TargetPrimitiveComponent = vrOwner->VRReplicatedCamera;
			owningVRChar = vrOwner;

			return;
		}
	}
	else
	{
		TArray<USceneComponent*> children = this->GetAttachChildren();

		for (int i = 0; i < children.Num(); i++)
		{
			if (children[i]->IsA(UCameraComponent::StaticClass()))
			{
				TargetPrimitiveComponent = children[i];
				owningVRChar = NULL;
				return;
			}
		}
	}

	TargetPrimitiveComponent = NULL;
	owningVRChar = NULL;
}

void UVRRootComponent::SetTrackingPaused(bool bPaused)
{
	bPauseTracking = bPaused;
}

void UVRRootComponent::UpdateCharacterCapsuleOffset()
{
	if (owningVRChar && !owningVRChar->bRetainRoomscale && owningVRChar->NetSmoother)
	{
		if (bCenterCapsuleOnHMD || !FMath::IsNearlyEqual(LastCapsuleHalfHeight, CapsuleHalfHeight))
		{
			owningVRChar->NetSmoother->SetRelativeLocation(GetTargetHeightOffset(), false, nullptr, ETeleportType::TeleportPhysics);

			LastCapsuleHalfHeight = CapsuleHalfHeight;
		}
	}
}

void UVRRootComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{

	if (this->IsSimulatingPhysics())
	{
		return Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	}

	if (bPauseTracking)
	{
		bHadRelativeMovement = false;
		DifferenceFromLastFrame = FVector::ZeroVector;
		return;
	}

	UVRBaseCharacterMovementComponent * CharMove = nullptr;
	bool bRetainRoomscale = true;

	if (IsValid(owningVRChar))
	{
		CharMove = Cast<UVRBaseCharacterMovementComponent>(owningVRChar->GetCharacterMovement());
		bRetainRoomscale = owningVRChar->bRetainRoomscale;
	}

	if (IsLocallyControlled())
	{
		bool bHadBadTracking = false;

		if (owningVRChar && owningVRChar->bTrackingPaused)
		{
			curCameraLoc = owningVRChar->PausedTrackingLoc;
			curCameraRot = FRotator(0.f, owningVRChar->PausedTrackingRot, 0.f);
		}
		else if (OptionalWaistTrackingParent.IsValid())
		{
			FTransform NewTrans = IVRTrackedParentInterface::Default_GetWaistOrientationAndPosition(OptionalWaistTrackingParent);
			curCameraLoc = NewTrans.GetTranslation();
			curCameraRot = NewTrans.Rotator();
		}
		else if (GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowedForWorld(*GetWorld()))
		{
			FQuat curRot;
			if (!GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, curRot, curCameraLoc))
			{
				curCameraLoc = lastCameraLoc;
				curCameraRot = lastCameraRot;
				bHadBadTracking = true;
			}
			else
			{
				if (owningVRChar && owningVRChar->VRReplicatedCamera)
				{
					owningVRChar->VRReplicatedCamera->ApplyTrackingParameters(curCameraLoc, true);
				}

				curCameraRot = curRot.Rotator();
			}
		}
		else if (TargetPrimitiveComponent)
		{
			curCameraRot = TargetPrimitiveComponent->GetRelativeRotation();
			curCameraLoc = TargetPrimitiveComponent->GetRelativeLocation();
		}
		else
		{
			curCameraRot = FRotator::ZeroRotator;
			curCameraLoc = FVector::ZeroVector;
		}

		StoredCameraRotOffset = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(curCameraRot);

		if (bRetainRoomscale)
		{

			UE::Net::QuantizeVector(100, curCameraLoc);

		}

		if (!bHadBadTracking && (!bRetainRoomscale || (!curCameraLoc.Equals(lastCameraLoc, 0.01f) || !curCameraRot.Equals(lastCameraRot, 0.01f))))
		{

			FVector LastPosition = OffsetComponentToWorld.GetLocation();

			bCalledUpdateTransform = false;

			if (bRetainRoomscale)
			{

				if (!CharMove || !CharMove->IsComponentTickEnabled() || !CharMove->IsActive())
				{
					OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
				}
				else 
				{

					OnUpdateTransform(EUpdateTransformFlags::SkipPhysicsUpdate, ETeleportType::None);
				}
			}

			FHitResult OutHit;
			FCollisionQueryParams Params("RelativeMovementSweep", false, GetOwner());
			FCollisionResponseParams ResponseParam;

			InitSweepCollisionParams(Params, ResponseParam);
			Params.bFindInitialOverlaps = true;
			bool bBlockingHit = false;

			if (bUseWalkingCollisionOverride )
			{
				FVector TargetWorldLocation = FVector::ZeroVector;

				if (bRetainRoomscale)
				{
					TargetWorldLocation = OffsetComponentToWorld.GetLocation();
				}
				else 
				{
					FVector NewLocation = StoredCameraRotOffset.RotateVector(FVector(VRCapsuleOffset.X, VRCapsuleOffset.Y, 0.0f)) + curCameraLoc;
					FVector PlanerLocation = NewLocation - lastCameraLoc;
					PlanerLocation.Z = 0.0f;
					DifferenceFromLastFrame = GetComponentTransform().TransformVector(PlanerLocation);
					TargetWorldLocation = LastPosition + DifferenceFromLastFrame;
				}

				bool bAllowWalkingCollision = false;
				if (CharMove != nullptr)
				{
					if (CharMove->MovementMode == EMovementMode::MOVE_Walking || CharMove->MovementMode == EMovementMode::MOVE_NavWalking)
						bAllowWalkingCollision = true;
				}

				if (bAllowWalkingCollision)
				{
					bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, LastPosition, TargetWorldLocation, FQuat::Identity, WalkingCollisionOverride, GetCollisionShape(), Params, ResponseParam);
				}

				if (bBlockingHit && OutHit.Component.IsValid())
				{
					if (CharMove != nullptr && CharMove->bIgnoreSimulatingComponentsInFloorCheck && OutHit.Component->IsSimulatingPhysics())
						bHadRelativeMovement = false;
					else
						bHadRelativeMovement = true;
				}
				else
					bHadRelativeMovement = false;
			}
			else
				bHadRelativeMovement = true;

			if (bHadRelativeMovement || (owningVRChar && !owningVRChar->bRetainRoomscale))
			{
				if (bRetainRoomscale)
				{
					DifferenceFromLastFrame = OffsetComponentToWorld.GetLocation() - LastPosition;
					lastCameraLoc = curCameraLoc;
					lastCameraRot = curCameraRot;

					UE::Net::QuantizeVector(100, DifferenceFromLastFrame);

				}
				else
				{

					FVector NewLocation = StoredCameraRotOffset.RotateVector(FVector(VRCapsuleOffset.X, VRCapsuleOffset.Y, 0.0f)) + curCameraLoc;
					FVector PlanerLocation = NewLocation - lastCameraLoc;
					PlanerLocation.Z = 0.0f;
					DifferenceFromLastFrame = GetComponentTransform().TransformVector(PlanerLocation);
					lastCameraLoc = NewLocation;
					lastCameraRot = curCameraRot;

					if (!bTickedOnce)
					{
						DifferenceFromLastFrame = FVector::ZeroVector;
						bTickedOnce = true;
					}
					else
					{

						DifferenceFromLastFrame.X = FMath::Clamp(DifferenceFromLastFrame.X, -100.0f, 100.0f);
						DifferenceFromLastFrame.Y = FMath::Clamp(DifferenceFromLastFrame.Y, -100.0f, 100.0f);
						DifferenceFromLastFrame.Z = FMath::Clamp(DifferenceFromLastFrame.Z, -100.0f, 100.0f);
						UE::Net::QuantizeVector(10000, DifferenceFromLastFrame);

					}
				}

			}
			else 
			{
				DifferenceFromLastFrame = FVector::ZeroVector;
				lastCameraLoc = curCameraLoc;
				lastCameraRot = curCameraRot;
			}
		}
		else
		{
			bHadRelativeMovement = false;
			DifferenceFromLastFrame = FVector::ZeroVector;
			lastCameraLoc = curCameraLoc;
			lastCameraRot = curCameraRot;
		}

	}
	else
	{
		if (owningVRChar && owningVRChar->bTrackingPaused)
		{
			curCameraLoc = owningVRChar->PausedTrackingLoc;
			curCameraRot = FRotator(0.f, owningVRChar->PausedTrackingRot, 0.f);
		}
		else if (TargetPrimitiveComponent)
		{
			curCameraRot = TargetPrimitiveComponent->GetRelativeRotation();
			curCameraLoc = TargetPrimitiveComponent->GetRelativeLocation();
		}
		else
		{
			curCameraRot = FRotator(0.0f, 0.0f, 0.0f);
			curCameraLoc = FVector(0.0f, 0.0f, 0.0f);
		}

		StoredCameraRotOffset = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(curCameraRot);

		if (!curCameraLoc.Equals(lastCameraLoc, 0.01f) || !curCameraRot.Equals(lastCameraRot, 0.01f))
		{
			bCalledUpdateTransform = false;

			if (!CharMove || !CharMove->IsActive())
			{
				OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
				if (bNavigationRelevant && bRegistered)
				{
					UpdateNavigationData();
					PostUpdateNavigationData();
				}
			}
			else 
			{

				OnUpdateTransform(EUpdateTransformFlags::SkipPhysicsUpdate, ETeleportType::None);

				if (this->GetOwner()->GetLocalRole() == ENetRole::ROLE_SimulatedProxy)
				{
					if (bNavigationRelevant && bRegistered)
					{
						UpdateNavigationData();
						PostUpdateNavigationData();
					}
				}
			}

			lastCameraRot = curCameraRot;
			lastCameraLoc = curCameraLoc;
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UVRRootComponent::SendPhysicsTransform(ETeleportType Teleport)
{

	BodyInstance.SetBodyTransform(OffsetComponentToWorld, Teleport);
	BodyInstance.UpdateBodyScale(OffsetComponentToWorld.GetScale3D());
}

void UVRRootComponent::SetSimulatePhysics(bool bSimulate)
{
	Super::SetSimulatePhysics(bSimulate);

	if (owningVRChar && !owningVRChar->bRetainRoomscale)
	{
		return Super::SetSimulatePhysics(bSimulate);
	}

	if (bSimulate)
	{
		if (AVRCharacter* OwningCharacter = Cast<AVRCharacter>(GetOwner()))
		{
			if (OwningCharacter->NetSmoother)
			{
				OwningCharacter->NetSmoother->SetRelativeLocation(FVector(0.f,0.f, -this->GetUnscaledCapsuleHalfHeight()));
			}
		}	
		this->AddWorldOffset(this->GetComponentRotation().RotateVector(FVector(0.f, 0.f, this->GetScaledCapsuleHalfHeight())), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		if (AVRCharacter* OwningCharacter = Cast<AVRCharacter>(GetOwner()))
		{
			if (OwningCharacter->NetSmoother)
			{
				OwningCharacter->NetSmoother->SetRelativeLocation(FVector(0.f, 0.f, 0));
			}
		}
		this->AddWorldOffset(this->GetComponentRotation().RotateVector(FVector(0.f, 0.f, -this->GetScaledCapsuleHalfHeight())), false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void UVRRootComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	if (this->IsSimulatingPhysics())
	{
		if (this->ShouldRender() && this->SceneProxy)
		{
			FTransform lOffsetComponentToWorld = OffsetComponentToWorld;
			float lCapsuleHalfHeight = CapsuleHalfHeight;
			bool bIsSimulating = this->IsSimulatingPhysics();
			FDrawVRCylinderSceneProxy* CylinderSceneProxy = (FDrawVRCylinderSceneProxy*)SceneProxy;
			ENQUEUE_RENDER_COMMAND(VRRootComponent_SendNewDebugTransform)(
				[CylinderSceneProxy, lOffsetComponentToWorld, lCapsuleHalfHeight, bIsSimulating](FRHICommandList& RHICmdList)
				{
					CylinderSceneProxy->UpdateTransform_RenderThread(lOffsetComponentToWorld, lCapsuleHalfHeight, bIsSimulating);
				});
		}

		return Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	}

	GenerateOffsetToWorld();

	if (!(UpdateTransformFlags & EUpdateTransformFlags::SkipPhysicsUpdate))
	{
		bCalledUpdateTransform = true;

		if (this->ShouldRender() && this->SceneProxy)
		{

			FTransform lOffsetComponentToWorld = OffsetComponentToWorld;
			float lCapsuleHalfHeight = CapsuleHalfHeight;
			bool bIsSimulating = this->IsSimulatingPhysics();
			FDrawVRCylinderSceneProxy* CylinderSceneProxy = (FDrawVRCylinderSceneProxy*)SceneProxy;
			ENQUEUE_RENDER_COMMAND(VRRootComponent_SendNewDebugTransform)(
				[CylinderSceneProxy, lOffsetComponentToWorld, lCapsuleHalfHeight, bIsSimulating](FRHICommandList& RHICmdList)
			{
				CylinderSceneProxy->UpdateTransform_RenderThread(lOffsetComponentToWorld, lCapsuleHalfHeight, bIsSimulating);
			});

		}

		if (bPhysicsStateCreated)
		{

			const bool bTransformSetDirectly = !(UpdateTransformFlags & EUpdateTransformFlags::PropagateFromParent);
			if (bTransformSetDirectly || !IsWelded())
			{
				SendPhysicsTransform(Teleport);
			}
		}
	}
}

FBoxSphereBounds UVRRootComponent::CalcBounds(const FTransform& LocalToWorld) const
{

	if (owningVRChar && !owningVRChar->bRetainRoomscale)
	{
		return Super::CalcBounds(LocalToWorld);
	}

	FVector BoxPoint = FVector(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight);

	return FBoxSphereBounds(FVector(curCameraLoc.X, curCameraLoc.Y, CapsuleHalfHeight) + StoredCameraRotOffset.RotateVector(VRCapsuleOffset), BoxPoint, BoxPoint.Size()).TransformBy(LocalToWorld);
}

void UVRRootComponent::GetNavigationData(FNavigationRelevantData& Data) const
{
	if (bDynamicObstacle)
	{
		Data.Modifiers.CreateAreaModifiers(this, GetDesiredAreaClass());
	}
}

#if WITH_EDITOR
void UVRRootComponent::PreEditChange(FProperty* PropertyThatWillChange)
{

	if (this->GetOwner()->IsA(AVRCharacter::StaticClass()))
		return;	
	else
		Super::PreEditChange(PropertyThatWillChange);
}

void UVRRootComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UVRRootComponent, CapsuleHalfHeight))
	{
		CapsuleHalfHeight = FMath::Max3(0.f, CapsuleHalfHeight, CapsuleRadius);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UVRRootComponent, CapsuleRadius))
	{
		CapsuleRadius = FMath::Clamp(CapsuleRadius, 0.f, CapsuleHalfHeight);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UVRRootComponent, VRCapsuleOffset))
	{
	}

	if (!IsTemplate())
	{

	}

	return;

}
#endif	

bool UVRRootComponent::MoveComponentImpl(const FVector& Delta, const FQuat& NewRotationQuat, bool bSweep, FHitResult* OutHit, EMoveComponentFlags MoveFlags, ETeleportType Teleport)
{

	SCOPE_CYCLE_COUNTER(STAT_VRRootMovement);

	if (!IsValidChecked(this) || (this->Mobility == EComponentMobility::Static && IsRegistered()))
	{
		if (OutHit)
		{
			OutHit->Init();
		}
		return false;
	}

	const bool bSkipPhysicsMove = ((MoveFlags & MOVECOMP_SkipPhysicsMove) != MOVECOMP_NoFlags);

	if (!this->IsSimulatingPhysics() && bSkipPhysicsMove)
	{

		return false;
	}

	ConditionalUpdateComponentToWorld();

	const FVector TraceStart = OffsetComponentToWorld.GetLocation();
	const FVector TraceEnd = TraceStart + Delta;

	float DeltaSizeSq = (TraceEnd - TraceStart).SizeSquared();				

	const FQuat InitialRotationQuat = GetComponentTransform().GetRotation();

	const float MinMovementDistSq = (bSweep ? FMath::Square(4.f*UE_KINDA_SMALL_NUMBER) : 0.f);
	if (DeltaSizeSq <= MinMovementDistSq)
	{

		if (NewRotationQuat.Equals(InitialRotationQuat, SCENECOMPONENT_QUAT_TOLERANCE))
		{

			if (OutHit)
			{
				OutHit->Init(TraceStart, TraceEnd);
			}
			return true;
		}
		DeltaSizeSq = 0.f;
	}

	FHitResult BlockingHit(NoInit);
	BlockingHit.bBlockingHit = false;
	BlockingHit.Time = 1.f;
	bool bFilledHitResult = false;
	bool bMoved = false;
	bool bIncludesOverlapsAtEnd = false;
	bool bRotationOnly = false;
	TInlineOverlapInfoArray PendingOverlaps;
	AActor* const Actor = GetOwner();
	FVector OrigLocation = GetComponentLocation();

	if (!bSweep)
	{

		bMoved = InternalSetWorldLocationAndRotation(OrigLocation + Delta, NewRotationQuat, bSkipPhysicsMove, Teleport);
		GenerateOffsetToWorld();
		bRotationOnly = (DeltaSizeSq == 0);
		bIncludesOverlapsAtEnd = bRotationOnly && (AreSymmetricRotations(InitialRotationQuat, NewRotationQuat, GetComponentScale())) && IsCollisionEnabled();
	}
	else
	{
		TArray<FHitResult> Hits;
		FVector NewLocation = OrigLocation;

		const bool bCollisionEnabled = IsQueryCollisionEnabled();
		UWorld* const MyWorld = GetWorld();
		if (MyWorld && bCollisionEnabled && (DeltaSizeSq > 0.f))
		{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if (!IsRegistered() && !MyWorld->bIsTearingDown)
			{
				if (Actor)
				{
					ensureMsgf(IsRegistered(), TEXT("%s MovedComponent %s not registered during sweep (IsValid %d)"), *Actor->GetName(), *GetName(), IsValid(Actor));
				}
				else
				{ 
					ensureMsgf(IsRegistered(), TEXT("Non-actor MovedComponent %s not registered during sweep"), *GetFullName());
				}
			}
#endif
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) && PERF_MOVECOMPONENT_STATS
			MoveTimer.bDidLineCheck = true;
#endif 
			static const FName TraceTagName = TEXT("MoveComponent");
			const bool bForceGatherOverlaps = !ShouldCheckOverlapFlagToQueueOverlaps(*this);		
			FComponentQueryParams Params(SCENE_QUERY_STAT(MoveComponent), Actor);
			FCollisionResponseParams ResponseParam;
			InitSweepCollisionParams(Params, ResponseParam);
			Params.bIgnoreTouches |= !(GetGenerateOverlapEvents() || bForceGatherOverlaps);
			Params.TraceTag = TraceTagName;
			bool const bHadBlockingHit = MyWorld->ComponentSweepMulti(Hits, this, TraceStart, TraceEnd, InitialRotationQuat, Params);

			if (Hits.Num() > 0)
			{
				const float DeltaSize = FMath::Sqrt(DeltaSizeSq);
				for (int32 HitIdx = 0; HitIdx < Hits.Num(); HitIdx++)
				{
					PullBackHit(Hits[HitIdx], TraceStart, TraceEnd, DeltaSize);
				}
			}

			int32 FirstNonInitialOverlapIdx = INDEX_NONE;
			if (bHadBlockingHit || (GetGenerateOverlapEvents() || bForceGatherOverlaps))
			{
				int32 BlockingHitIndex = INDEX_NONE;
				float BlockingHitNormalDotDelta = UE_BIG_NUMBER;
				for (int32 HitIdx = 0; HitIdx < Hits.Num(); HitIdx++)
				{
					const FHitResult& TestHit = Hits[HitIdx];

					if (TestHit.bBlockingHit)
					{
						if (!ShouldIgnoreHitResult(MyWorld, bAllowSimulatingCollision, TestHit, Delta, Actor, MoveFlags) && !ShouldComponentIgnoreHitResult(TestHit, MoveFlags))
						{
							if (TestHit.bStartPenetrating)
							{

								const float NormalDotDelta = (TestHit.ImpactNormal | Delta);
								if (NormalDotDelta < BlockingHitNormalDotDelta)
								{
									BlockingHitNormalDotDelta = NormalDotDelta;
									BlockingHitIndex = HitIdx;
								}
							}
							else if (BlockingHitIndex == INDEX_NONE)
							{

								BlockingHitIndex = HitIdx;
								break;
							}
						}
					}
					else if (GetGenerateOverlapEvents() || bForceGatherOverlaps)
					{
						UPrimitiveComponent* OverlapComponent = TestHit.Component.Get();
						if (OverlapComponent && (OverlapComponent->GetGenerateOverlapEvents() || bForceGatherOverlaps))
						{
							if (!ShouldIgnoreOverlapResult(MyWorld, Actor, *this, TestHit.HitObjectHandle.FetchActor(), *OverlapComponent,!bForceGatherOverlaps))
							{

								if (BlockingHitIndex >= 0 && TestHit.Time > Hits[BlockingHitIndex].Time)
								{
									break;
								}

								if (FirstNonInitialOverlapIdx == INDEX_NONE && TestHit.Time > 0.f)
								{

									FirstNonInitialOverlapIdx = PendingOverlaps.Num();
								}

								AddUniqueOverlapFast(PendingOverlaps, FOverlapInfo(TestHit));
							}
						}
					}
				}

				if (BlockingHitIndex >= 0)
				{
					BlockingHit = Hits[BlockingHitIndex];
					bFilledHitResult = true;
				}
			}

			if (!BlockingHit.bBlockingHit)
			{
				NewLocation += (TraceEnd - TraceStart);
			}
			else
			{
				check(bFilledHitResult);
				NewLocation += (BlockingHit.Time * (TraceEnd - TraceStart));

				const FVector ToNewLocation = (NewLocation - OrigLocation);
				if (ToNewLocation.SizeSquared() <= MinMovementDistSq)
				{

					NewLocation = OrigLocation;
					BlockingHit.Time = 0.f;

					if (FirstNonInitialOverlapIdx != INDEX_NONE)
					{
						PendingOverlaps.SetNum(FirstNonInitialOverlapIdx, EAllowShrinking::No);
					}
				}
			}

			bIncludesOverlapsAtEnd = AreSymmetricRotations(InitialRotationQuat, NewRotationQuat, GetComponentScale());
		}
		else if (DeltaSizeSq > 0.f)
		{

			NewLocation += Delta;
			bIncludesOverlapsAtEnd = false;
		}
		else if (DeltaSizeSq == 0.f && bCollisionEnabled)
		{
			bIncludesOverlapsAtEnd = AreSymmetricRotations(InitialRotationQuat, NewRotationQuat, GetComponentScale());
			bRotationOnly = true;
		}

		bMoved = InternalSetWorldLocationAndRotation(NewLocation, NewRotationQuat, bSkipPhysicsMove, Teleport);
		GenerateOffsetToWorld();
	}

	if (bMoved)
	{
		if (IsDeferringMovementUpdates())
		{

			FScopedMovementUpdate* ScopedUpdate = GetCurrentScopedMovement();
			if (bRotationOnly && bIncludesOverlapsAtEnd)
			{
				ScopedUpdate->KeepCurrentOverlapsAfterRotation(bSweep);
			}
			else
			{
				ScopedUpdate->AppendOverlapsAfterMove(PendingOverlaps, bSweep, bIncludesOverlapsAtEnd);
			}
		}
		else
		{
			if (bIncludesOverlapsAtEnd)
			{
				TInlineOverlapInfoArray OverlapsAtEndLocation;
				bool bHasEndOverlaps = false;
				if (bRotationOnly)
				{

				}
				else
				{		

				}
				TOverlapArrayView PendingOverlapsView(PendingOverlaps);
				TOverlapArrayView OverlapsAtEndView(OverlapsAtEndLocation);
				UpdateOverlaps(&PendingOverlapsView, true, bHasEndOverlaps ? &OverlapsAtEndView : nullptr);
			}
			else
			{
				TOverlapArrayView PendingOverlapsView(PendingOverlaps);
				UpdateOverlaps(&PendingOverlapsView, true, nullptr);
			}
		}
	}

	const bool bAllowHitDispatch = !BlockingHit.bStartPenetrating || !(MoveFlags & MOVECOMP_DisableBlockingOverlapDispatch);
	if (BlockingHit.bBlockingHit && bAllowHitDispatch && IsValidChecked(this))
	{
		check(bFilledHitResult);
		if (IsDeferringMovementUpdates())
		{
			FScopedMovementUpdate* ScopedUpdate = GetCurrentScopedMovement();
			ScopedUpdate->AppendBlockingHitAfterMove(BlockingHit);
		}
		else
		{
			DispatchBlockingHit(*Actor, BlockingHit);
		}
	}

	if (OutHit)
	{
		if (bFilledHitResult)
		{
			*OutHit = BlockingHit;
		}
		else
		{
			OutHit->Init(TraceStart, TraceEnd);
		}
	}

	return bMoved;
}

bool UVRRootComponent::UpdateOverlapsImpl(const TOverlapArrayView* NewPendingOverlaps, bool bDoNotifies, const TOverlapArrayView* OverlapsAtEndLocation)
{

	SCOPE_CYCLE_COUNTER(STAT_UpdateOverlapsVRRoot);
	SCOPE_CYCLE_UOBJECT(ComponentScope, this);

	const AActor* const MyActor = GetOwner();
	if (MyActor && !MyActor->HasActorBegunPlay() && !MyActor->IsActorBeginningPlay())
	{
		return false;
	}

	bool bCanSkipUpdateOverlaps = true;

	if (GetGenerateOverlapEvents() && IsQueryCollisionEnabled())	
	{
		bCanSkipUpdateOverlaps = false;

		if (MyActor)
		{
			const FTransform PrevTransform = GetComponentTransform();

			const bool bIgnoreChildren = (MyActor->GetRootComponent() == this);

			if (NewPendingOverlaps)
			{	 

				const int32 NumNewPendingOverlaps = NewPendingOverlaps->Num();
				for (int32 Idx = 0; Idx < NumNewPendingOverlaps; ++Idx)
				{
					BeginComponentOverlap((*NewPendingOverlaps)[Idx], bDoNotifies);
				}
			}

			const TOverlapArrayView* OverlapsAtEndLocationPtr = OverlapsAtEndLocation;

			TArray<FOverlapInfo> OverlapsAtEnd;
			TOverlapArrayView OverlapsAtEndLoc;
			if ( NewPendingOverlaps && NewPendingOverlaps->Num() > 0)
			{
				ConvertSweptOverlapsToCurrentOverlapsVR(OverlapsAtEnd, *NewPendingOverlaps, -1, OffsetComponentToWorld.GetLocation(), GetComponentQuat());
				OverlapsAtEndLoc = TOverlapArrayView(OverlapsAtEnd);
				OverlapsAtEndLocationPtr = &OverlapsAtEndLoc;
			}

			TInlineOverlapInfoArray OverlapMultiResult;
			TInlineOverlapPointerArray NewOverlappingComponentPtrs;

			if (IsValidChecked(this) && GetGenerateOverlapEvents())
			{

				static const auto CVarAllowCachedOverlaps = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AllowCachedOverlaps"));

				if (OverlapsAtEndLocationPtr != nullptr && CVarAllowCachedOverlaps->GetInt() > 0 && PrevTransform.Equals(GetComponentTransform()))
				{
					UE_LOGF(LogVRRootComponent, VeryVerbose, "%ls->%ls Skipping overlap test!", *GetNameSafe(GetOwner()), *GetName());
					const bool bCheckForInvalid = (NewPendingOverlaps && NewPendingOverlaps->Num() > 0);
					if (bCheckForInvalid)
					{

						GetPointersToArrayDataByPredicate(NewOverlappingComponentPtrs, *OverlapsAtEndLocationPtr, FPredicateFilterCanOverlap(*this));
					}
					else
					{
						GetPointersToArrayData(NewOverlappingComponentPtrs, *OverlapsAtEndLocationPtr);
					}
				}
				else
				{
					SCOPE_CYCLE_COUNTER(STAT_PerformOverlapQueryVR);
					UE_LOGF(LogVRRootComponent, VeryVerbose, "%ls->%ls Performing overlaps!", *GetNameSafe(GetOwner()), *GetName());
					UWorld* const MyWorld = GetWorld();
					TArray<FOverlapResult> Overlaps;

					FComponentQueryParams Params(SCENE_QUERY_STAT(UpdateOverlaps), bIgnoreChildren ? MyActor : nullptr); 

					Params.bIgnoreBlocks = true;	
					FCollisionResponseParams ResponseParam;
					InitSweepCollisionParams(Params, ResponseParam);
					ComponentOverlapMulti(Overlaps, MyWorld, OffsetComponentToWorld.GetTranslation(), GetComponentQuat(), GetCollisionObjectType(), Params);

					for (int32 ResultIdx = 0; ResultIdx < Overlaps.Num(); ResultIdx++)
					{
						const FOverlapResult& Result = Overlaps[ResultIdx];

						UPrimitiveComponent* const HitComp = Result.Component.Get();
						if (HitComp && (HitComp != this) && HitComp->GetGenerateOverlapEvents())
						{
							const bool bCheckOverlapFlags = false; 
							if (!ShouldIgnoreOverlapResult(MyWorld, MyActor, *this, Result.OverlapObjectHandle.FetchActor(), *HitComp, bCheckOverlapFlags))
							{
								OverlapMultiResult.Emplace(HitComp, Result.GetItemIndex());		
							}
						}
					}

					GetPointersToArrayData(NewOverlappingComponentPtrs, OverlapMultiResult);
				}
			}

			if (OverlappingComponents.Num() > 0)
			{
				TInlineOverlapPointerArray OldOverlappingComponentPtrs;
				if (bIgnoreChildren)
				{
					GetPointersToArrayDataByPredicate(OldOverlappingComponentPtrs, OverlappingComponents, FPredicateOverlapHasDifferentActor(*MyActor));
				}
				else
				{
					GetPointersToArrayData(OldOverlappingComponentPtrs, OverlappingComponents);
				}

				for (int32 CompIdx = 0; CompIdx < OldOverlappingComponentPtrs.Num() && NewOverlappingComponentPtrs.Num() > 0; ++CompIdx)
				{

					const FOverlapInfo* SearchItem = OldOverlappingComponentPtrs[CompIdx];
					const int32 NewElementIdx = IndexOfOverlapFast(NewOverlappingComponentPtrs, SearchItem);
					if (NewElementIdx != INDEX_NONE)
					{
						NewOverlappingComponentPtrs.RemoveAtSwap(NewElementIdx, 1, EAllowShrinking::No);
						OldOverlappingComponentPtrs.RemoveAtSwap(CompIdx, 1, EAllowShrinking::No);
						--CompIdx;
					}
				}

				const int32 NumOldOverlaps = OldOverlappingComponentPtrs.Num();
				if (NumOldOverlaps > 0)
				{

					TInlineOverlapInfoArray OldOverlappingComponents;
					OldOverlappingComponents.SetNumUninitialized(NumOldOverlaps);
					for (int32 i = 0; i < NumOldOverlaps; i++)
					{
						OldOverlappingComponents[i] = *(OldOverlappingComponentPtrs[i]);
					}

					for (const FOverlapInfo& OtherOverlap : OldOverlappingComponents)
					{
						if (OtherOverlap.OverlapInfo.Component.IsValid())
						{
							EndComponentOverlap(OtherOverlap, bDoNotifies, false);
						}
						else
						{

							const bool bAllowShrinking = (OverlappingComponents.Max() >= 24);
							const int32 StaleElementIndex = IndexOfOverlapFast(OverlappingComponents, OtherOverlap);
							if (StaleElementIndex != INDEX_NONE)
							{
								OverlappingComponents.RemoveAtSwap(StaleElementIndex, 1, bAllowShrinking ? EAllowShrinking::Yes : EAllowShrinking::No);
							}
						}
					}
				}
			}

			static_assert(sizeof(OverlapMultiResult) != 0, "Variable must be in this scope");
			static_assert(sizeof(*OverlapsAtEndLocation) != 0, "Variable must be in this scope");

			for (const FOverlapInfo* NewOverlap : NewOverlappingComponentPtrs)
			{
				BeginComponentOverlap(*NewOverlap, bDoNotifies);
			}
		}
	}
	else
	{

		if (OverlappingComponents.Num() > 0)
		{
			const bool bSkipNotifySelf = false;
			ClearComponentOverlaps(bDoNotifies, bSkipNotifySelf);
		}
	}

	TInlineComponentArray<USceneComponent*> AttachedChildren;
	AttachedChildren.Append(GetAttachChildren());

	for (USceneComponent* const ChildComp : AttachedChildren)
	{
		if (ChildComp)
		{

			bCanSkipUpdateOverlaps &= ChildComp->UpdateOverlaps(nullptr, bDoNotifies, nullptr);
		}
	}

	if (GetShouldUpdatePhysicsVolume())
	{
		UpdatePhysicsVolume(bDoNotifies);
		bCanSkipUpdateOverlaps = false;
	}

	return bCanSkipUpdateOverlaps;
}

bool UVRRootComponent::IsLocallyControlled() const
{

	const AActor* MyOwner = GetOwner();
	return MyOwner->HasLocalNetOwner();

}

 void UVRRootComponent::SetCapsuleSizeVR(float NewRadius, float NewHalfHeight, bool bUpdateOverlaps)
{
	SCOPE_CYCLE_COUNTER(STAT_VRRootSetCapsuleSize);

	FScopedMovementUpdate ScopedNetSmootherMovementUpdate(owningVRChar ? owningVRChar->NetSmoother : nullptr, EScopedUpdate::DeferredUpdates);

	if (FMath::IsNearlyEqual(NewRadius, CapsuleRadius) && FMath::IsNearlyEqual(NewHalfHeight, CapsuleHalfHeight))
	{
		return;
	}

	float OldCapsuleHalfHeight = CapsuleHalfHeight;

	CapsuleHalfHeight = FMath::Max3(0.f, NewHalfHeight, NewRadius);
	CapsuleRadius = FMath::Max(0.f, NewRadius);

	UpdateBounds();
	UpdateBodySetup();
	MarkRenderStateDirty();
	GenerateOffsetToWorld();

	if (bPhysicsStateCreated)
	{

		BodyInstance.UpdateBodyScale(GetComponentTransform().GetScale3D(), true);

		if (bUpdateOverlaps && IsCollisionEnabled() && GetOwner())
		{
			UpdateOverlaps();
		}
	}

	if (owningVRChar)
	{
		if (GetNetMode() < ENetMode::NM_Client)
		{
			if (owningVRChar->GetVRReplicateCapsuleHeight())
			{
				owningVRChar->ReplicatedCapsuleHeight.CapsuleHeight = CapsuleHalfHeight;
			}
		}

		float Offset = (CapsuleHalfHeight - OldCapsuleHalfHeight);

		if (!owningVRChar->bRetainRoomscale && (owningVRChar->GetNetMode() < ENetMode::NM_Client || IsLocallyControlled()))
		{
			MoveComponent(this->GetComponentQuat().GetUpVector() * (Offset * this->GetComponentScale().Z), GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		}

		if (!owningVRChar->bRetainRoomscale && !IsLocallyControlled() && !IsNetMode(NM_DedicatedServer))
		{

			FNetworkPredictionData_Client_Character* ClientData = owningVRChar->GetCharacterMovement()->GetPredictionData_Client_Character();
			if (ClientData)
			{
				ClientData->MeshTranslationOffset.Z += (Offset * this->GetComponentScale().Z);
				ClientData->OriginalMeshTranslationOffset.Z = ClientData->MeshTranslationOffset.Z;
			}
		}
	}

}

void UVRRootComponent::UpdatePhysicsVolume(bool bTriggerNotifiers)
{
	if (GetShouldUpdatePhysicsVolume() && IsValidChecked(this))
	{

		if (UWorld * MyWorld = GetWorld())
		{
			if (MyWorld->GetNonDefaultPhysicsVolumeCount() == 0)
			{
				SetPhysicsVolume(MyWorld->GetDefaultPhysicsVolume(), bTriggerNotifiers);
			}
			else if (GetGenerateOverlapEvents() && IsQueryCollisionEnabled())
			{
				APhysicsVolume* BestVolume = MyWorld->GetDefaultPhysicsVolume();
				int32 BestPriority = BestVolume->Priority;

				for (auto CompIt = OverlappingComponents.CreateIterator(); CompIt; ++CompIt)
				{
					const FOverlapInfo& Overlap = *CompIt;
					UPrimitiveComponent* OtherComponent = Overlap.OverlapInfo.Component.Get();
					if (OtherComponent && OtherComponent->GetGenerateOverlapEvents())
					{
						APhysicsVolume* V = Cast<APhysicsVolume>(OtherComponent->GetOwner());
						if (V && V->Priority > BestPriority)
						{

							if (AreWeOverlappingVolume(V))
							{
								BestPriority = V->Priority;
								BestVolume = V;
							}
						}
					}
				}

				SetPhysicsVolume(BestVolume, bTriggerNotifiers);
			}
			else
			{
				Super::UpdatePhysicsVolume(bTriggerNotifiers);
			}
		}
	}
}

template<typename AllocatorType>
bool UVRRootComponent::ConvertSweptOverlapsToCurrentOverlapsVR(
	TArray<FOverlapInfo, AllocatorType>& OverlapsAtEndLocation, const TOverlapArrayView& SweptOverlaps, int32 SweptOverlapsIndex,
	const FVector& EndLocation, const FQuat& EndRotationQuat)
{
	if (SweptOverlapsIndex == -1)
	{
		SweptOverlapsIndex = 0;
	}
	else
	{
		return false;
	}

	checkSlow(SweptOverlapsIndex >= 0);

	FVector EndLocationVR = OffsetComponentToWorld.GetLocation();

	bool bResult = false;
	const bool bForceGatherOverlaps = !ShouldCheckOverlapFlagToQueueOverlaps(*this);

	static const auto CVarAllowCachedOverlaps = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AllowCachedOverlaps"));
	if ((GetGenerateOverlapEvents() || bForceGatherOverlaps) && CVarAllowCachedOverlaps->GetInt())
	{
		const AActor* Actor = GetOwner();
		if (Actor && Actor->GetRootComponent() == this)
		{

			static const auto CVarEnableFastOverlapCheck = IConsoleManager::Get().FindConsoleVariable(TEXT("p.EnableFastOverlapCheck"));

			if (CVarAllowCachedOverlaps->GetInt())
			{

				const FCollisionQueryParams UnusedQueryParams(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				const int32 NumSweptOverlaps = SweptOverlaps.Num();
				OverlapsAtEndLocation.Reserve(OverlapsAtEndLocation.Num() + NumSweptOverlaps);
				for (int32 Index = SweptOverlapsIndex; Index < NumSweptOverlaps; ++Index)
				{
					const FOverlapInfo& OtherOverlap = SweptOverlaps[Index];
					UPrimitiveComponent* OtherPrimitive = OtherOverlap.OverlapInfo.GetComponent();
					if (OtherPrimitive && (OtherPrimitive->GetGenerateOverlapEvents() || bForceGatherOverlaps))
					{
						if (OtherPrimitive->bMultiBodyOverlap)
						{

							return false;
						}
						else if (Cast<USkeletalMeshComponent>(OtherPrimitive) )
						{

							return false;
						}
						else if (OtherPrimitive->ComponentOverlapComponent(this, EndLocationVR, EndRotationQuat, UnusedQueryParams))
						{
							OverlapsAtEndLocation.Add(OtherOverlap);
						}
					}
				}

				checkfSlow(OverlapsAtEndLocation.FindByPredicate(FPredicateOverlapHasSameActor(*Actor)) == nullptr,
					TEXT("Child overlaps should not be included in the SweptOverlaps() array in UPrimitiveComponent::ConvertSweptOverlapsToCurrentOverlaps()."));

				bResult = true;
			}
			else
			{
				if (SweptOverlaps.Num() == 0 && AreAllCollideableDescendantsRelative())
				{

					GetOverlapsWithActor_TemplateVR(Actor, OverlapsAtEndLocation);
					bResult = true;
				}
			}
		}
	}

	return bResult;
}

template<typename AllocatorType>
bool UVRRootComponent::ConvertRotationOverlapsToCurrentOverlapsVR(TArray<FOverlapInfo, AllocatorType>& OutOverlapsAtEndLocation, const TOverlapArrayView& CurrentOverlaps)
{
	bool bResult = false;
	const bool bForceGatherOverlaps = !ShouldCheckOverlapFlagToQueueOverlaps(*this);

	static const auto CVarAllowCachedOverlaps = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AllowCachedOverlaps"));

	if ((GetGenerateOverlapEvents() || bForceGatherOverlaps) &&  CVarAllowCachedOverlaps->GetInt())
	{
		const AActor* Actor = GetOwner();
		if (Actor && Actor->GetRootComponent() == this)
		{
			static const auto CVarEnableFastOverlapCheck = IConsoleManager::Get().FindConsoleVariable(TEXT("p.EnableFastOverlapCheck"));

			if (CVarAllowCachedOverlaps->GetInt())
			{

				OutOverlapsAtEndLocation.Reserve(OutOverlapsAtEndLocation.Num() + CurrentOverlaps.Num());
				Algo::CopyIf(CurrentOverlaps, OutOverlapsAtEndLocation, FPredicateOverlapHasDifferentActor(*Actor));
				bResult = true;
			}
		}
	}

	return bResult;
}

template<typename AllocatorType>
bool UVRRootComponent::GetOverlapsWithActor_TemplateVR(const AActor* Actor, TArray<FOverlapInfo, AllocatorType>& OutOverlaps) const
{
	const int32 InitialCount = OutOverlaps.Num();
	if (Actor)
	{
		for (int32 OverlapIdx = 0; OverlapIdx < OverlappingComponents.Num(); ++OverlapIdx)
		{
			UPrimitiveComponent const* const PrimComp = OverlappingComponents[OverlapIdx].OverlapInfo.Component.Get();
			if (PrimComp && (PrimComp->GetOwner() == Actor))
			{
				OutOverlaps.Add(OverlappingComponents[OverlapIdx]);
			}
		}
	}

	return InitialCount != OutOverlaps.Num();
}
#undef LOCTEXT_NAMESPACE