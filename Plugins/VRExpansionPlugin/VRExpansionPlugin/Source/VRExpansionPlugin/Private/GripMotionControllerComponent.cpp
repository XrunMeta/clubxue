

#include "GripMotionControllerComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GripMotionControllerComponent)

#include "VRExpansionFunctionLibrary.h"
#include "IHeadMountedDisplay.h"
#include "HeadMountedDisplayTypes.h"
#include "Misc/ScopeLock.h"
#include "Net/UnrealNetwork.h"
#include "PrimitiveSceneInfo.h"
#include "Engine/World.h"
#include "PrimitiveSceneProxy.h"
#include "GameFramework/WorldSettings.h"
#include "IXRSystemAssets.h"
#include "Components/StaticMeshComponent.h"
#include "MotionDelayBuffer.h"
#include "UObject/VRObjectVersion.h"
#include "UObject/UObjectGlobals.h" 
#include "IXRTrackingSystem.h"
#include "IXRSystemAssets.h"
#include "SceneView.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "VRBaseCharacter.h"
#include "VRCharacter.h"
#include "VRRootComponent.h"
#include "VRGlobalSettings.h"
#include "Math/DualQuat.h"
#include "IIdentifiableXRDevice.h" 
#include "XRMotionControllerBase.h" 

#include "Physics/Experimental/PhysScene_Chaos.h"

#include "GripScripts/GS_Default.h"
#include "GripScripts/GS_LerpToHand.h"

#include "PhysicsPublic.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConstraintDrives.h"
#include "PhysicsReplication.h"
#include "PhysicsEngine/PhysicsAsset.h"

#include "Chaos/ParticleHandle.h"
#include "Chaos/KinematicGeometryParticles.h"
#include "Chaos/PBDJointConstraintTypes.h"
#include "Chaos/PBDConstraintBaseData.h"
#include "Chaos/PBDJointConstraintData.h"
#include "Chaos/Sphere.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "Chaos/ChaosConstraintSettings.h"
#include "Chaos/PhysicsObject.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include "Chaos/PhysicsObjectInterface.h"

#include "Misc/CollisionIgnoreSubsystem.h"

#include "Features/IModularFeatures.h"

#if WITH_PUSH_MODEL
#include "Net/Core/PushModel/PushModel.h"
#endif

DEFINE_LOG_CATEGORY(LogVRMotionController);

DECLARE_CYCLE_STAT(TEXT("TickGrip ~ TickingGrip"), STAT_TickGrip, STATGROUP_TickGrip);
DECLARE_CYCLE_STAT(TEXT("GetGripWorldTransform ~ GettingTransform"), STAT_GetGripTransform, STATGROUP_TickGrip);

const float ANGULAR_STIFFNESS_MULTIPLIER = 1.5f;
const float ANGULAR_DAMPING_MULTIPLIER = 1.4f;
const float ANGULAR_STIFFNESS_MULTIPLIER_CHAOS = 0.45f;
const float ANGULAR_DAMPING_MULTIPLIER_CHAOS = 0.45f;

const float HYBRID_PHYSICS_GRIP_MULTIPLIER = 10.0f;

namespace {

	FCriticalSection CritSect;

} 

namespace GripUEMotionController {

	class FScopeLockOptional
	{
	public:
		FScopeLockOptional()
		{
		}

		void Lock(FCriticalSection* InSynchObject)
		{
			SynchObject = InSynchObject;
			SynchObject->Lock();
		}

		~FScopeLockOptional()
		{
			Unlock();
		}

		void Unlock()
		{
			if (SynchObject)
			{
				SynchObject->Unlock();
				SynchObject = nullptr;
			}
		}

	private:

		FScopeLockOptional(const FScopeLockOptional& InScopeLock);

		FScopeLockOptional& operator=(FScopeLockOptional& InScopeLock)
		{
			return *this;
		}

	private:

		FCriticalSection* SynchObject = nullptr;
	};
}

namespace GripMotionControllerCvars
{
	static int32 DrawDebugGripCOM = 0;
	FAutoConsoleVariableRef CVarDrawCOMDebugSpheres(
		TEXT("vr.DrawDebugCenterOfMassForGrips"),
		DrawDebugGripCOM,
		TEXT("When on, will draw debug speheres for physics grips COM.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);
}

UGripMotionControllerComponent::UGripMotionControllerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bTickEvenWhenPaused = true;

	PlayerIndex = 0;
	MotionSource = IMotionController::LeftHandSourceId;

	bDisableLowLatencyUpdate = false;
	bHasAuthority = false;
	bIgnoreTrackingStatus = false;
	bUseWithoutTracking = false;
	ClientAuthConflictResolutionMethod = EVRClientAuthConflictResolutionMode::VRGRIP_CONFLICT_First;
	bAlwaysSendTickGrip = false;
	bAutoActivate = true;

	SetIsReplicatedByDefault(true);

	CurrentTrackingStatus = ETrackingStatus::NotTracked;

	ControllerNetUpdateRate = 100.0f; 
	ControllerNetUpdateCount = 0.0f;
	bReplicateWithoutTracking = false;
	bLerpingPosition = false;
	bSmoothReplicatedMotion = false;
	bReppedOnce = false;
	bScaleTracking = false;
	TrackingScaler = FVector(1.0f);
	bLimitMinHeight = false;
	MinimumHeight = 0.0f;
	bLimitMaxHeight = false;
	MaximumHeight = 240.0f;

	bLeashToHMD = false;
	LeashRange = 300.0f;
	bConstrainToPivot = false;

	bSmoothHandTracking = false;
	bWasSmoothingHand = false;
	bSmoothWithEuroLowPassFunction = true;
	LastSmoothRelativeTransform = FTransform::Identity;
	SmoothingSpeed = 20.0f;
	EuroSmoothingParams.MinCutoff = 0.1f;
	EuroSmoothingParams.DeltaCutoff = 10.f;
	EuroSmoothingParams.CutoffSlope = 10.f;

	bIsPostTeleport = false;

	GripIDIncrementer = INVALID_VRGRIP_ID;

	CustomPivotComponentSocketName = NAME_None;
	bSkipPivotTransformAdjustment = false;

	bOffsetByControllerProfile = true;
	CurrentControllerProfileTransform = FTransform::Identity;

	DefaultGripScript = nullptr;
	DefaultGripScriptClass = UGS_Default::StaticClass();

	VelocityCalculationType = EVRVelocityType::VRLOCITY_Default;
	LastRelativePosition = FTransform::Identity;
	bSampleVelocityInWorldSpace = false;
	VelocitySamples = 30.f;

	bProjectNonSimulatingGrips = false;
	EndPhysicsTickFunction.TickGroup = TG_EndPhysics;
	EndPhysicsTickFunction.bCanEverTick = true;
	EndPhysicsTickFunction.bStartWithTickEnabled = false;
}

void UGripMotionControllerComponent::RegisterEndPhysicsTick(bool bRegister)
{
	if (bRegister != EndPhysicsTickFunction.IsTickFunctionRegistered())
	{
		if (bRegister)
		{
			if (SetupActorComponentTickFunction(&EndPhysicsTickFunction))
			{
				EndPhysicsTickFunction.Target = this;

				UWorld* World = GetWorld();
				if (World != nullptr)
				{
					EndPhysicsTickFunction.AddPrerequisite(World, World->EndPhysicsTickFunction);
				}
			}
		}
		else
		{
			EndPhysicsTickFunction.UnRegisterTickFunction();
		}
	}
}

void UGripMotionControllerComponent::EndPhysicsTickComponent(FGripComponentEndPhysicsTickFunction& ThisTickFunction)
{

	if (!IsValidChecked(this))
		return;

	FTransform baseTrans = this->GetAttachParent()->GetComponentTransform().Inverse();

	for (int i = 0; i < LocallyGrippedObjects.Num(); ++i)
	{
		if (!LocallyGrippedObjects[i].GrippedObject || !IsValid(LocallyGrippedObjects[i].GrippedObject))
			continue; 

		if (LocallyGrippedObjects[i].GrippedObject && IsValid(LocallyGrippedObjects[i].GrippedObject) && LocallyGrippedObjects[i].GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			bool bSampleRelativeTransform = bProjectNonSimulatingGrips;

			if (!bSampleRelativeTransform)
			{
				EGripInterfaceTeleportBehavior TeleportBehavior = IVRGripInterface::Execute_TeleportBehavior(LocallyGrippedObjects[i].GrippedObject);
				bSampleRelativeTransform = TeleportBehavior == EGripInterfaceTeleportBehavior::DeltaTeleportation;
			}

			if (bSampleRelativeTransform)
			{
				switch(LocallyGrippedObjects[i].GripTargetType)
				{
					case EGripTargetType::ActorGrip:
					{
						if (AActor* Actor = Cast<AActor>(LocallyGrippedObjects[i].GrippedObject))
						{
							if (UPrimitiveComponent* root = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
							{
								LocallyGrippedObjects[i].LastWorldTransform = root->GetComponentTransform() * baseTrans;
								LocallyGrippedObjects[i].bSetLastWorldTransform = true;
							}
						}
					}break;
					case EGripTargetType::ComponentGrip:
					{
						if (UPrimitiveComponent* root = Cast<UPrimitiveComponent>(LocallyGrippedObjects[i].GrippedObject))
						{
							LocallyGrippedObjects[i].LastWorldTransform = root->GetComponentTransform() * baseTrans;
							LocallyGrippedObjects[i].bSetLastWorldTransform = true;
						}
					}break;
				}			
			}
		}
	}

	for (int i = 0; i < GrippedObjects.Num(); ++i)
	{
		if (!GrippedObjects[i].GrippedObject || !IsValid(GrippedObjects[i].GrippedObject))
			continue; 

		if (GrippedObjects[i].GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			bool bSampleRelativeTransform = bProjectNonSimulatingGrips;

			if (!bSampleRelativeTransform)
			{
				EGripInterfaceTeleportBehavior TeleportBehavior = IVRGripInterface::Execute_TeleportBehavior(GrippedObjects[i].GrippedObject);
				bSampleRelativeTransform = TeleportBehavior == EGripInterfaceTeleportBehavior::DeltaTeleportation;
			}

			if (bSampleRelativeTransform)
			{
				switch (GrippedObjects[i].GripTargetType)
				{
				case EGripTargetType::ActorGrip:
				{
					if (AActor* Actor = Cast<AActor>(GrippedObjects[i].GrippedObject))
					{
						if (UPrimitiveComponent* root = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
						{
							GrippedObjects[i].LastWorldTransform = root->GetComponentTransform() * baseTrans;
							GrippedObjects[i].bSetLastWorldTransform = true;
						}
					}
				}break;
				case EGripTargetType::ComponentGrip:
				{
					if (UPrimitiveComponent* root = Cast<UPrimitiveComponent>(GrippedObjects[i].GrippedObject))
					{
						GrippedObjects[i].LastWorldTransform = root->GetComponentTransform() * baseTrans;
						GrippedObjects[i].bSetLastWorldTransform = true;
					}
				}break;
				}
			}
		}
	}
}

void FGripComponentEndPhysicsTickFunction::ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	QUICK_SCOPE_CYCLE_COUNTER(FGripComponentEndPhysicsTickFunction_ExecuteTick);
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(Physics);

	if (Target && IsValid(Target))
	{
		FActorComponentTickFunction::ExecuteTickHelper(Target,  false, DeltaTime, TickType, [this](float DilatedTime)
		{
			Target->EndPhysicsTickComponent(*this);
		});
	}
}

FString FGripComponentEndPhysicsTickFunction::DiagnosticMessage()
{
	return TEXT("GripComponentEndPhysicsTickFunction");
}

FName FGripComponentEndPhysicsTickFunction::DiagnosticContext(bool bDetailed)
{
	return FName(TEXT("GripComponentEndPhysicsTick"));
}

UGripMotionControllerComponent::~UGripMotionControllerComponent()
{

}

void UGripMotionControllerComponent::NewControllerProfileLoaded()
{
	GetCurrentProfileTransform(false);
}

void UGripMotionControllerComponent::GetCurrentProfileTransform(bool bBindToNoticationDelegate)
{
	if (bOffsetByControllerProfile)
	{
		UVRGlobalSettings* VRSettings = GetMutableDefault<UVRGlobalSettings>();

		if (VRSettings == nullptr)
			return;

		EControllerHand HandType;
		this->GetHandType(HandType);

		FTransform NewControllerProfileTransform = FTransform::Identity;

		if (HandType == EControllerHand::Left || HandType == EControllerHand::AnyHand || !VRSettings->bUseSeperateHandTransforms)
		{
			NewControllerProfileTransform = VRSettings->CurrentControllerProfileTransform;
		}
		else if (HandType == EControllerHand::Right)
		{
			NewControllerProfileTransform = VRSettings->CurrentControllerProfileTransformRight;
		}

		if (bBindToNoticationDelegate && !NewControllerProfileEvent_Handle.IsValid())
		{
			NewControllerProfileEvent_Handle = VRSettings->OnControllerProfileChangedEvent.AddUObject(this, &UGripMotionControllerComponent::NewControllerProfileLoaded);
		}

		if (!NewControllerProfileTransform.Equals(CurrentControllerProfileTransform))
		{
			FTransform OriginalControllerProfileTransform = CurrentControllerProfileTransform;
			CurrentControllerProfileTransform = NewControllerProfileTransform;

			if (!bTracked && bUseWithoutTracking)
			{
				this->SetRelativeTransform(CurrentControllerProfileTransform * (OriginalControllerProfileTransform.Inverse() * this->GetRelativeTransform()));
			}

			OnControllerProfileTransformChanged.Broadcast(CurrentControllerProfileTransform.Inverse() * OriginalControllerProfileTransform, CurrentControllerProfileTransform);
		}
	}
}

void UGripMotionControllerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (!DefaultGripScript && DefaultGripScriptClass)
		DefaultGripScript = NewObject<UGS_Default>(this, DefaultGripScriptClass); 
	else
		DefaultGripScript = NewObject<UGS_Default>(this, UGS_Default::StaticClass());
}

void UGripMotionControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	RegisterEndPhysicsTick(false);

	if (NewControllerProfileEvent_Handle.IsValid())
	{
		UVRGlobalSettings* VRSettings = GetMutableDefault<UVRGlobalSettings>();
		if (VRSettings != nullptr)
		{
			VRSettings->OnControllerProfileChangedEvent.Remove(NewControllerProfileEvent_Handle);
			NewControllerProfileEvent_Handle.Reset();
		}
	}

	for (int i = 0; i < GrippedObjects.Num(); i++)
	{
		DestroyPhysicsHandle(GrippedObjects[i]);

		if (IsServer())
		{
			DropObjectByInterface(nullptr, GrippedObjects[i].GripID);
		}
		else
		{
			if (IsValid(GrippedObjects[i].GrippedObject))
			{
				bool bSimulateOnDrop = true;
				if (GrippedObjects[i].GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
				{
					bSimulateOnDrop = IVRGripInterface::Execute_SimulateOnDrop(GrippedObjects[i].GrippedObject);
				}

				NotifyDrop(GrippedObjects[i], bSimulateOnDrop);
			}
		}
	}
	GrippedObjects.Empty();

	for (int i = 0; i < LocallyGrippedObjects.Num(); i++)
	{
		DestroyPhysicsHandle(LocallyGrippedObjects[i]);

		if (IsServer())
		{
			DropObjectByInterface(nullptr, LocallyGrippedObjects[i].GripID);
		}
		else
		{
			if (IsValid(LocallyGrippedObjects[i].GrippedObject))
			{
				bool bSimulateOnDrop = true;
				if (LocallyGrippedObjects[i].GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
				{
					bSimulateOnDrop = IVRGripInterface::Execute_SimulateOnDrop(LocallyGrippedObjects[i].GrippedObject);
				}

				NotifyDrop(LocallyGrippedObjects[i], bSimulateOnDrop);
			}
		}
	}
	LocallyGrippedObjects.Empty();

	for (int i = 0; i < PhysicsGrips.Num(); i++)
	{
		DestroyPhysicsHandle(&PhysicsGrips[i]);
	}
	PhysicsGrips.Empty();

	if (UWorld * myWorld = GetWorld())
	{
		myWorld->GetTimerManager().ClearAllTimersForObject(this);
	}

	ObjectsWaitingForSocketUpdate.Empty();
}

void UGripMotionControllerComponent::OnUnregister()
{
	Super::OnUnregister();
}

void UGripMotionControllerComponent::BeginDestroy()
{
	Super::BeginDestroy();

	if (GripViewExtension.IsValid())
	{
		{

			FScopeLock ScopeLock(&CritSect);
			GripViewExtension->MotionControllerComponent = NULL;
		}

		GripViewExtension.Reset();
	}
}

void UGripMotionControllerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGripMotionControllerComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{	

	if (bHasAuthority && !bDisableLowLatencyUpdate && IsActive())
	{
		LateUpdateParams.GripRenderThreadRelativeTransform = GetRelativeTransform();
		LateUpdateParams.GripRenderThreadComponentScale = GetComponentScale();
		LateUpdateParams.GripRenderThreadProfileTransform = CurrentControllerProfileTransform;
		LateUpdateParams.GripRenderThreadLastLocationForLateUpdate = LastLocationForLateUpdate;

		LateUpdateParams.bRenderSmoothHandTracking = bSmoothHandTracking;
		if (LateUpdateParams.bRenderSmoothHandTracking)
		{
			if (UWorld* world = GetWorld())
			{
				LateUpdateParams.RenderLastDeltaTime = world->GetDeltaSeconds();
			}

			LateUpdateParams.bRenderSmoothWithEuroLowPassFunction = bSmoothWithEuroLowPassFunction;

			if (LateUpdateParams.bRenderSmoothWithEuroLowPassFunction)
			{
				LateUpdateParams.RenderEuroSmoothingParams = EuroSmoothingParams;
			}
			else
			{
				LateUpdateParams.RenderSmoothingSpeed = SmoothingSpeed;
				LateUpdateParams.RenderLastSmoothRelativeTransform = LastSmoothRelativeTransform;
			}
		}
	}

	Super::Super::CreateRenderState_Concurrent(Context);
}

void UGripMotionControllerComponent::SendRenderTransform_Concurrent()
{

	if (bHasAuthority && !bDisableLowLatencyUpdate && IsActive())
	{
		struct FPrimitiveUpdateRenderThreadRelativeTransformParams
		{
			FRenderTrackingParams LateUpdateParams;
		};

		FPrimitiveUpdateRenderThreadRelativeTransformParams UpdateParams;
		UpdateParams.LateUpdateParams.GripRenderThreadRelativeTransform = GetRelativeTransform();
		UpdateParams.LateUpdateParams.GripRenderThreadComponentScale = GetComponentScale();
		UpdateParams.LateUpdateParams.GripRenderThreadProfileTransform = CurrentControllerProfileTransform;
		UpdateParams.LateUpdateParams.GripRenderThreadLastLocationForLateUpdate = LastLocationForLateUpdate;

		UpdateParams.LateUpdateParams.bRenderSmoothHandTracking = bSmoothHandTracking;
		if (UpdateParams.LateUpdateParams.bRenderSmoothHandTracking)
		{
			if (UWorld* world = GetWorld())
			{
				UpdateParams.LateUpdateParams.RenderLastDeltaTime = world->GetDeltaSeconds();
			}

			UpdateParams.LateUpdateParams.bRenderSmoothWithEuroLowPassFunction = bSmoothWithEuroLowPassFunction;

			if (UpdateParams.LateUpdateParams.bRenderSmoothWithEuroLowPassFunction)
			{
				UpdateParams.LateUpdateParams.RenderEuroSmoothingParams = EuroSmoothingParams;
			}
			else
			{
				UpdateParams.LateUpdateParams.RenderSmoothingSpeed = SmoothingSpeed;
				UpdateParams.LateUpdateParams.RenderLastSmoothRelativeTransform = LastSmoothRelativeTransform;
			}
		}

		ENQUEUE_RENDER_COMMAND(UpdateRTRelativeTransformCommand)(
			[UpdateParams, this](FRHICommandListImmediate& RHICmdList)
			{
				LateUpdateParams = UpdateParams.LateUpdateParams;
			});
	}

	Super::Super::SendRenderTransform_Concurrent();
}

FBPActorPhysicsHandleInformation * UGripMotionControllerComponent::GetPhysicsGrip(const FBPActorGripInformation & GripInfo)
{
	return PhysicsGrips.FindByKey(GripInfo);
}

FBPActorPhysicsHandleInformation* UGripMotionControllerComponent::GetPhysicsGrip(const uint8 GripID)
{
	return PhysicsGrips.FindByKey(GripID);
}

bool UGripMotionControllerComponent::GetPhysicsGripIndex(const FBPActorGripInformation & GripInfo, int & index)
{
	index = PhysicsGrips.IndexOfByKey(GripInfo);
	return index != INDEX_NONE;
}

FBPActorPhysicsHandleInformation * UGripMotionControllerComponent::CreatePhysicsGrip(const FBPActorGripInformation & GripInfo)
{
	FBPActorPhysicsHandleInformation * HandleInfo = PhysicsGrips.FindByKey(GripInfo);

	if (HandleInfo)
	{
		DestroyPhysicsHandle(HandleInfo);
		return HandleInfo;
	}

	FBPActorPhysicsHandleInformation NewInfo;
	NewInfo.HandledObject = GripInfo.GrippedObject;
	NewInfo.GripID = GripInfo.GripID;

	int index = PhysicsGrips.Add(NewInfo);

	return &PhysicsGrips[index];
}

void UGripMotionControllerComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	 Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	 DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeLocation);
	 DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeRotation);
	 DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D);

	FDoRepLifetimeParams PushModelParams{ COND_None, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UGripMotionControllerComponent, GrippedObjects, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGripMotionControllerComponent, ControllerNetUpdateRate, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGripMotionControllerComponent, bSmoothReplicatedMotion, PushModelParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGripMotionControllerComponent, bReplicateWithoutTracking, PushModelParams);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_SkipOwner, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UGripMotionControllerComponent, ReplicatedControllerTransform, PushModelParamsWithCondition);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGripMotionControllerComponent, LocallyGrippedObjects, PushModelParamsWithCondition);

	FDoRepLifetimeParams PushModelParamsWithConditionOwnerOnly{ COND_OwnerOnly, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(UGripMotionControllerComponent, LocalTransactionBuffer, PushModelParamsWithConditionOwnerOnly);

}

void UGripMotionControllerComponent::Server_SendControllerTransform_Implementation(FBPVRComponentPosRep NewTransform)
{

	ReplicatedControllerTransform = NewTransform;
#if WITH_PUSH_MODEL
		MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, ReplicatedControllerTransform, this);
#endif

	if(!bHasAuthority)
		OnRep_ReplicatedControllerTransform();
}

bool UGripMotionControllerComponent::Server_SendControllerTransform_Validate(FBPVRComponentPosRep NewTransform)
{
	return true;

}

void UGripMotionControllerComponent::FGripViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
	if (!MotionControllerComponent)
	{
		return;
	}

	LateUpdate.Setup(MotionControllerComponent->CalcNewComponentToWorld(FTransform()), MotionControllerComponent, false);
}

void UGripMotionControllerComponent::GetPhysicsVelocities(const FBPActorGripInformation &Grip, FVector &CurAngularVelocity, FVector &CurLinearVelocity)
{
	UPrimitiveComponent * primComp = Grip.GetGrippedComponent();
	AActor * pActor = Grip.GetGrippedActor();

	if (!primComp && pActor)
		primComp = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

	if (!primComp)
	{
		CurAngularVelocity = FVector::ZeroVector;
		CurLinearVelocity = FVector::ZeroVector;
		return;
	}

	if (!primComp->IsSimulatingPhysics())
	{
		CurLinearVelocity = Grip.LinVel;
		CurAngularVelocity = Grip.RotVel;
		return;
	}

	CurAngularVelocity = primComp->GetPhysicsAngularVelocityInDegrees();
	CurLinearVelocity = primComp->GetPhysicsLinearVelocity();
}

bool UGripMotionControllerComponent::GetPhysicsConstraintForce(const FBPActorGripInformation& Grip, FVector& AngularForce, FVector& LinearForce)
{
	if (FBPActorPhysicsHandleInformation * PhysHandle = GetPhysicsGrip(Grip.GripID))
	{
		if (PhysHandle->HandleData2.IsValid())
		{
			FPhysicsInterface::GetForce(PhysHandle->HandleData2, LinearForce, AngularForce);
			return true;
		}
	}

	return false;
}

void UGripMotionControllerComponent::GetGripMass(const FBPActorGripInformation& Grip, float& Mass)
{
	UPrimitiveComponent* primComp = Grip.GetGrippedComponent();
	AActor* pActor = Grip.GetGrippedActor();

	if (!primComp && pActor)
		primComp = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

	if (!primComp || !primComp->IsSimulatingPhysics())
	{
		Mass = 0.f;
		return;
	}

	Mass = primComp->GetMass();
}

FTransform UGripMotionControllerComponent::GetGrippedObjectTransform(const FBPActorGripInformation& Grip)
{
	FTransform returnTrans = FTransform::Identity;

	if (!IsValid(Grip.GrippedObject))
	{
		return returnTrans;
	}

	if (Grip.GripTargetType == EGripTargetType::ActorGrip)
	{
		if (AActor* GrippedActor = Cast<AActor>(Grip.GrippedObject))
		{
			returnTrans = GrippedActor->GetActorTransform();
		}
	}
	else
	{
		if (UPrimitiveComponent* GrippedComp = Cast<UPrimitiveComponent>(Grip.GrippedObject))
		{
			returnTrans = GrippedComp->GetComponentTransform();
		}
	}

	return returnTrans;
}

void UGripMotionControllerComponent::GetGripByActor(FBPActorGripInformation &Grip, AActor * ActorToLookForGrip, EBPVRResultSwitch &Result)
{
	if (!ActorToLookForGrip)
	{
		Result = EBPVRResultSwitch::OnFailed;
		return;
	}

	FBPActorGripInformation * GripInfo = GrippedObjects.FindByKey(ActorToLookForGrip);
	if(!GripInfo)
		GripInfo = LocallyGrippedObjects.FindByKey(ActorToLookForGrip);

	if (GripInfo)
	{
		Grip = *GripInfo;
		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}

	Result = EBPVRResultSwitch::OnFailed;
}

void UGripMotionControllerComponent::GetGripByComponent(FBPActorGripInformation &Grip, UPrimitiveComponent * ComponentToLookForGrip, EBPVRResultSwitch &Result)
{
	if (!ComponentToLookForGrip)
	{
		Result = EBPVRResultSwitch::OnFailed;
		return;
	}

	FBPActorGripInformation * GripInfo = GrippedObjects.FindByKey(ComponentToLookForGrip);
	if(!GripInfo)
		GripInfo = LocallyGrippedObjects.FindByKey(ComponentToLookForGrip);

	if (GripInfo)
	{
		Grip = *GripInfo;
		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}

	Result = EBPVRResultSwitch::OnFailed;
}

void UGripMotionControllerComponent::GetGripByObject(FBPActorGripInformation &Grip, UObject * ObjectToLookForGrip, EBPVRResultSwitch &Result)
{
	if (!ObjectToLookForGrip)
	{
		Result = EBPVRResultSwitch::OnFailed;
		return;
	}

	FBPActorGripInformation * GripInfo = GrippedObjects.FindByKey(ObjectToLookForGrip);
	if(!GripInfo)
		GripInfo = LocallyGrippedObjects.FindByKey(ObjectToLookForGrip);

	if (GripInfo)
	{
		Grip = *GripInfo;
		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}

	Result = EBPVRResultSwitch::OnFailed;
}

FBPActorGripInformation * UGripMotionControllerComponent::GetGripPtrByID(uint8 IDToLookForGrip)
{
	if (IDToLookForGrip == INVALID_VRGRIP_ID)
	{
		return nullptr;
	}

	FBPActorGripInformation* GripInfo = GrippedObjects.FindByKey(IDToLookForGrip);
	if (!GripInfo)
		GripInfo = LocallyGrippedObjects.FindByKey(IDToLookForGrip);

	return GripInfo;
}

void UGripMotionControllerComponent::GetGripByID(FBPActorGripInformation &Grip, uint8 IDToLookForGrip, EBPVRResultSwitch &Result)
{
	if (IDToLookForGrip == INVALID_VRGRIP_ID)
	{
		Result = EBPVRResultSwitch::OnFailed;
		return;
	}

	FBPActorGripInformation * GripInfo = GrippedObjects.FindByKey(IDToLookForGrip);
	if (!GripInfo)
		GripInfo = LocallyGrippedObjects.FindByKey(IDToLookForGrip);

	if (GripInfo)
	{
		Grip = *GripInfo;
		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}

	Result = EBPVRResultSwitch::OnFailed;
}

void UGripMotionControllerComponent::SetGripHybridLock(const FBPActorGripInformation& Grip, EBPVRResultSwitch& Result, bool bIsLocked)
{
	int fIndex = GrippedObjects.Find(Grip);

	FBPActorGripInformation* GripInformation = nullptr;

	if (fIndex != INDEX_NONE)
	{
		GripInformation = &GrippedObjects[fIndex];
	}
	else
	{
		fIndex = LocallyGrippedObjects.Find(Grip);

		if (fIndex != INDEX_NONE)
		{
			GripInformation = &LocallyGrippedObjects[fIndex];
		}
	}

	if (GripInformation != nullptr)
	{
		GripInformation->bLockHybridGrip = bIsLocked;
		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}

	Result = EBPVRResultSwitch::OnFailed;
}

void UGripMotionControllerComponent::SetGripPaused(const FBPActorGripInformation &Grip, EBPVRResultSwitch &Result, bool bIsPaused, bool bNoConstraintWhenPaused)
{
	int fIndex = GrippedObjects.Find(Grip);

	FBPActorGripInformation * GripInformation = nullptr;

	if (fIndex != INDEX_NONE)
	{
		GripInformation = &GrippedObjects[fIndex];
	}
	else
	{
		fIndex = LocallyGrippedObjects.Find(Grip);

		if (fIndex != INDEX_NONE)
		{
			GripInformation = &LocallyGrippedObjects[fIndex];
		}
	}

	if (GripInformation != nullptr)
	{
		if (bNoConstraintWhenPaused)
		{
			if (bIsPaused)
			{
				if (FBPActorPhysicsHandleInformation * PhysHandle = GetPhysicsGrip(*GripInformation))
				{
					DestroyPhysicsHandle(*GripInformation);
				}
			}
			else
			{
				ReCreateGrip(*GripInformation);
			}
		}

		GripInformation->bIsPaused = bIsPaused;
		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}

	Result = EBPVRResultSwitch::OnFailed;
}

void UGripMotionControllerComponent::SetPausedTransform(const FBPActorGripInformation &Grip, const FTransform & PausedTransform, bool bTeleport)
{

	FBPActorGripInformation * GripInformation = nullptr;

	int fIndex = GrippedObjects.Find(Grip);

	if (fIndex != INDEX_NONE)
	{
		GripInformation = &GrippedObjects[fIndex];
	}
	else
	{
		fIndex = LocallyGrippedObjects.Find(Grip);

		if (fIndex != INDEX_NONE)
		{
			GripInformation = &LocallyGrippedObjects[fIndex];
		}
	}

	if (GripInformation != nullptr && GripInformation->GrippedObject != nullptr)
	{
		if (bTeleport)
		{
			FTransform ProxyTrans = PausedTransform;
			TeleportMoveGrip_Impl(*GripInformation, true, true, ProxyTrans);
		}
		else
		{
			if (FBPActorPhysicsHandleInformation * PhysHandle = GetPhysicsGrip(GrippedObjects[fIndex]))
			{
				UpdatePhysicsHandleTransform(*GripInformation, PausedTransform);
			}
			else
			{
				if (GripInformation->GripTargetType == EGripTargetType::ActorGrip)
				{
					GripInformation->GetGrippedActor()->SetActorTransform(PausedTransform);
				}
				else
				{
					GripInformation->GetGrippedComponent()->SetWorldTransform(PausedTransform);
				}
			}
		}
	}
}

void UGripMotionControllerComponent::SetGripCollisionType(const FBPActorGripInformation &Grip, EBPVRResultSwitch &Result, EGripCollisionType NewGripCollisionType)
{
	int fIndex = GrippedObjects.Find(Grip);

	if (fIndex != INDEX_NONE)
	{
		GrippedObjects[fIndex].GripCollisionType = NewGripCollisionType;
		ReCreateGrip(GrippedObjects[fIndex]);
		DIRTY_GRIPPED_OBJECTS();
		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}
	else
	{
		fIndex = LocallyGrippedObjects.Find(Grip);

		if (fIndex != INDEX_NONE)
		{
			LocallyGrippedObjects[fIndex].GripCollisionType = NewGripCollisionType;

			if (IsLocallyControlled() && !IsServer() && !IsTornOff() && LocallyGrippedObjects[fIndex].GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive)
			{
				FBPActorGripInformation GripInfo = LocallyGrippedObjects[fIndex];
				Server_NotifyLocalGripAddedOrChanged(GripInfo);
			}

			ReCreateGrip(LocallyGrippedObjects[fIndex]);
			DIRTY_LOCALLY_GRIPPED_OBJECTS();

			Result = EBPVRResultSwitch::OnSucceeded;
			return;
		}
	}

	Result = EBPVRResultSwitch::OnFailed;
}

void UGripMotionControllerComponent::SetGripLateUpdateSetting(const FBPActorGripInformation &Grip, EBPVRResultSwitch &Result, EGripLateUpdateSettings NewGripLateUpdateSetting)
{
	int fIndex = GrippedObjects.Find(Grip);

	if (fIndex != INDEX_NONE)
	{
		GrippedObjects[fIndex].GripLateUpdateSetting = NewGripLateUpdateSetting;
		DIRTY_GRIPPED_OBJECTS();
		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}
	else
	{
		fIndex = LocallyGrippedObjects.Find(Grip);

		if (fIndex != INDEX_NONE)
		{
			LocallyGrippedObjects[fIndex].GripLateUpdateSetting = NewGripLateUpdateSetting;

			if (IsLocallyControlled() && !IsServer() && !IsTornOff() && LocallyGrippedObjects[fIndex].GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive)
			{
				FBPActorGripInformation GripInfo = LocallyGrippedObjects[fIndex];
				Server_NotifyLocalGripAddedOrChanged(GripInfo);
			}

			DIRTY_LOCALLY_GRIPPED_OBJECTS();
			Result = EBPVRResultSwitch::OnSucceeded;
			return;
		}
	}

	Result = EBPVRResultSwitch::OnFailed;
}

void UGripMotionControllerComponent::SetGripRelativeTransform(
	const FBPActorGripInformation &Grip,
	EBPVRResultSwitch &Result,
	const FTransform & NewRelativeTransform
	)
{
	int fIndex = GrippedObjects.Find(Grip);

	if (fIndex != INDEX_NONE)
	{
		GrippedObjects[fIndex].RelativeTransform = NewRelativeTransform;
		if (FBPActorPhysicsHandleInformation * HandleInfo = GetPhysicsGrip(Grip))
		{
			UpdatePhysicsHandle(Grip.GripID, true);
			NotifyGripTransformChanged(Grip);
		}

		DIRTY_GRIPPED_OBJECTS();

		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}
	else
	{
		fIndex = LocallyGrippedObjects.Find(Grip);

		if (fIndex != INDEX_NONE)
		{
			LocallyGrippedObjects[fIndex].RelativeTransform = NewRelativeTransform;
			if (FBPActorPhysicsHandleInformation * HandleInfo = GetPhysicsGrip(Grip))
			{
				UpdatePhysicsHandle(Grip.GripID, true);
				NotifyGripTransformChanged(Grip);
			}

			if (IsLocallyControlled() && !IsServer() && !IsTornOff() && LocallyGrippedObjects[fIndex].GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive)
			{
				FBPActorGripInformation GripInfo = LocallyGrippedObjects[fIndex];
				Server_NotifyLocalGripAddedOrChanged(GripInfo);
			}

			DIRTY_LOCALLY_GRIPPED_OBJECTS();

			Result = EBPVRResultSwitch::OnSucceeded;
			return;
		}
	}

	Result = EBPVRResultSwitch::OnFailed;
}

void UGripMotionControllerComponent::SetGripAdditionTransform(
	const FBPActorGripInformation &Grip,
	EBPVRResultSwitch &Result,
	const FTransform & NewAdditionTransform, bool bMakeGripRelative
	)
{
	int fIndex = GrippedObjects.Find(Grip);

	if (fIndex != INDEX_NONE)
	{
		GrippedObjects[fIndex].AdditionTransform = CreateGripRelativeAdditionTransform(Grip, NewAdditionTransform, bMakeGripRelative);
		DIRTY_GRIPPED_OBJECTS();

		Result = EBPVRResultSwitch::OnSucceeded;
		return;
	}
	else
	{
		fIndex = LocallyGrippedObjects.Find(Grip);

		if (fIndex != INDEX_NONE)
		{
			LocallyGrippedObjects[fIndex].AdditionTransform = CreateGripRelativeAdditionTransform(Grip, NewAdditionTransform, bMakeGripRelative);
			DIRTY_LOCALLY_GRIPPED_OBJECTS();

			Result = EBPVRResultSwitch::OnSucceeded;
			return;
		}
	}
	Result = EBPVRResultSwitch::OnFailed;
}

void UGripMotionControllerComponent::SetGripStiffnessAndDamping(
	const FBPActorGripInformation &Grip,
	EBPVRResultSwitch &Result,
	float NewStiffness, float NewDamping, bool bAlsoSetAngularValues, float OptionalAngularStiffness, float OptionalAngularDamping
	)
{
	Result = EBPVRResultSwitch::OnFailed;
	int fIndex = GrippedObjects.Find(Grip);

	if (fIndex != INDEX_NONE)
	{
		GrippedObjects[fIndex].Stiffness = NewStiffness;
		GrippedObjects[fIndex].Damping = NewDamping;

		if (bAlsoSetAngularValues)
		{
			GrippedObjects[fIndex].AdvancedGripSettings.PhysicsSettings.AngularStiffness = OptionalAngularStiffness;
			GrippedObjects[fIndex].AdvancedGripSettings.PhysicsSettings.AngularDamping = OptionalAngularDamping;
		}

		DIRTY_GRIPPED_OBJECTS();

		Result = EBPVRResultSwitch::OnSucceeded;
		SetGripConstraintStiffnessAndDamping(&GrippedObjects[fIndex]);

	}
	else
	{
		fIndex = LocallyGrippedObjects.Find(Grip);

		if (fIndex != INDEX_NONE)
		{
			LocallyGrippedObjects[fIndex].Stiffness = NewStiffness;
			LocallyGrippedObjects[fIndex].Damping = NewDamping;

			if (bAlsoSetAngularValues)
			{
				LocallyGrippedObjects[fIndex].AdvancedGripSettings.PhysicsSettings.AngularStiffness = OptionalAngularStiffness;
				LocallyGrippedObjects[fIndex].AdvancedGripSettings.PhysicsSettings.AngularDamping = OptionalAngularDamping;
			}

			if (IsLocallyControlled() && !IsServer() && !IsTornOff() && LocallyGrippedObjects[fIndex].GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive)
			{
				FBPActorGripInformation GripInfo = LocallyGrippedObjects[fIndex];
				Server_NotifyLocalGripAddedOrChanged(GripInfo);
			}

			DIRTY_LOCALLY_GRIPPED_OBJECTS();

			Result = EBPVRResultSwitch::OnSucceeded;
			SetGripConstraintStiffnessAndDamping(&LocallyGrippedObjects[fIndex]);

		}
	}
}

void UGripMotionControllerComponent::SetGripAdvancedGripSettings(
	const FBPActorGripInformation& Grip,
	EBPVRResultSwitch& Result,
	uint8 GripPriority,
	bool bSetOwnerOnGrip,
	bool bDisallowLerping,
	bool bDisallowSettingPositionOnClientAuthDrop
)
{
	Result = EBPVRResultSwitch::OnFailed;
	int fIndex = GrippedObjects.Find(Grip);

	if (fIndex != INDEX_NONE)
	{
		GrippedObjects[fIndex].AdvancedGripSettings.GripPriority = GripPriority;
		GrippedObjects[fIndex].AdvancedGripSettings.bSetOwnerOnGrip = bSetOwnerOnGrip;
		GrippedObjects[fIndex].AdvancedGripSettings.bDisallowLerping = bDisallowLerping;
		GrippedObjects[fIndex].AdvancedGripSettings.bDisallowSettingPositionOnClientAuthDrop = bDisallowSettingPositionOnClientAuthDrop;

		DIRTY_GRIPPED_OBJECTS();

		Result = EBPVRResultSwitch::OnSucceeded;
	}
	else
	{
		fIndex = LocallyGrippedObjects.Find(Grip);

		if (fIndex != INDEX_NONE)
		{
			GrippedObjects[fIndex].AdvancedGripSettings.GripPriority = GripPriority;
			GrippedObjects[fIndex].AdvancedGripSettings.bSetOwnerOnGrip = bSetOwnerOnGrip;
			GrippedObjects[fIndex].AdvancedGripSettings.bDisallowLerping = bDisallowLerping;
			GrippedObjects[fIndex].AdvancedGripSettings.bDisallowSettingPositionOnClientAuthDrop = bDisallowSettingPositionOnClientAuthDrop;

			if (IsLocallyControlled() && !IsServer() && !IsTornOff() && LocallyGrippedObjects[fIndex].GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive)
			{
				FBPActorGripInformation GripInfo = LocallyGrippedObjects[fIndex];
				Server_NotifyLocalGripAddedOrChanged(GripInfo);
			}

			DIRTY_LOCALLY_GRIPPED_OBJECTS();

			Result = EBPVRResultSwitch::OnSucceeded;
		}
	}

}

FTransform UGripMotionControllerComponent::CreateGripRelativeAdditionTransform_BP(
	const FBPActorGripInformation &GripToSample,
	const FTransform & AdditionTransform,
	bool bGripRelative
)
{
	return CreateGripRelativeAdditionTransform(GripToSample, AdditionTransform, bGripRelative);
}

bool UGripMotionControllerComponent::GripObject(
	UObject * ObjectToGrip,
	const FTransform &WorldOffset,
	bool bWorldOffsetIsRelative,
	FName OptionalSnapToSocketName,
	FName OptionalBoneToGripName,
	EGripCollisionType GripCollisionType,
	EGripLateUpdateSettings GripLateUpdateSetting,
	EGripMovementReplicationSettings GripMovementReplicationSetting,
	float GripStiffness,
	float GripDamping,
	bool bIsSlotGrip)
{

	if (IsTravelingOrNullWorld())
		return false;

	if (UPrimitiveComponent * PrimComp = Cast<UPrimitiveComponent>(ObjectToGrip))
	{
		return GripComponent(PrimComp, WorldOffset, bWorldOffsetIsRelative, OptionalSnapToSocketName, OptionalBoneToGripName, GripCollisionType,GripLateUpdateSetting,GripMovementReplicationSetting,GripStiffness,GripDamping, bIsSlotGrip);
	}
	else if (AActor * Actor = Cast<AActor>(ObjectToGrip))
	{
		return GripActor(Actor, WorldOffset, bWorldOffsetIsRelative, OptionalSnapToSocketName, OptionalBoneToGripName, GripCollisionType, GripLateUpdateSetting, GripMovementReplicationSetting, GripStiffness, GripDamping, bIsSlotGrip);
	}

	return false;
}

bool UGripMotionControllerComponent::DropObject(
	UObject* ObjectToDrop,
	uint8 GripIDToDrop,
	bool bSimulate,
	FVector OptionalAngularVelocity,
	FVector OptionalLinearVelocity)
{
	if (IsValid(ObjectToDrop))
	{
		FBPActorGripInformation * GripInfo = GrippedObjects.FindByKey(ObjectToDrop);
		if (!GripInfo)
			GripInfo = LocallyGrippedObjects.FindByKey(ObjectToDrop);

		if (GripInfo != nullptr && IsValid(GripInfo->GrippedObject))
		{
			return DropGrip_Implementation(*GripInfo, bSimulate, OptionalAngularVelocity, OptionalLinearVelocity);
		}
	}
	else if (GripIDToDrop != INVALID_VRGRIP_ID)
	{
		FBPActorGripInformation * GripInfo = GrippedObjects.FindByKey(GripIDToDrop);
		if (!GripInfo)
			GripInfo = LocallyGrippedObjects.FindByKey(GripIDToDrop);

		if (GripInfo != nullptr && IsValid(GripInfo->GrippedObject))
		{
			return DropGrip_Implementation(*GripInfo, bSimulate, OptionalAngularVelocity, OptionalLinearVelocity);
		}
	}

	return false;
}

bool UGripMotionControllerComponent::GripObjectByInterface(UObject* ObjectToGrip, const FTransform &WorldOffset, bool bWorldOffsetIsRelative, FName OptionalBoneToGripName, FName OptionalSnapToSocketName, bool bIsSlotGrip)
{

	if (IsTravelingOrNullWorld())
		return false;

	if (!IsValid(ObjectToGrip))
	{
		return false;
	}

	if (UPrimitiveComponent * PrimComp = Cast<UPrimitiveComponent>(ObjectToGrip))
	{
		AActor * Owner = PrimComp->GetOwner();

		if (!IsValid(Owner))
			return false;

		if (PrimComp->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			EGripCollisionType CollisionType = IVRGripInterface::Execute_GetPrimaryGripType(PrimComp, bIsSlotGrip);

			float Stiffness;
			float Damping;
			IVRGripInterface::Execute_GetGripStiffnessAndDamping(PrimComp, Stiffness, Damping);

			return GripComponent(PrimComp, WorldOffset, bWorldOffsetIsRelative, OptionalSnapToSocketName,
				OptionalBoneToGripName,
				CollisionType,
				IVRGripInterface::Execute_GripLateUpdateSetting(PrimComp),
				IVRGripInterface::Execute_GripMovementReplicationType(PrimComp),
				Stiffness,
				Damping,
				bIsSlotGrip
				);
		}
		else if (Owner->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			EGripCollisionType CollisionType = IVRGripInterface::Execute_GetPrimaryGripType(Owner, bIsSlotGrip);

			float Stiffness;
			float Damping;
			IVRGripInterface::Execute_GetGripStiffnessAndDamping(Owner, Stiffness, Damping);

			return GripActor(Owner, WorldOffset, bWorldOffsetIsRelative, OptionalSnapToSocketName,
				OptionalBoneToGripName,
				CollisionType,
				IVRGripInterface::Execute_GripLateUpdateSetting(Owner),
				IVRGripInterface::Execute_GripMovementReplicationType(Owner),
				Stiffness,
				Damping,
				bIsSlotGrip
			);

		}
		else
		{

			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController GripObjectByInterface was called on an object that doesn't implement the interface and doesn't have a parent that implements the interface!");
			return false;
		}
	}
	else if (AActor * Actor = Cast<AActor>(ObjectToGrip))
	{
		UPrimitiveComponent * root = Cast<UPrimitiveComponent>(Actor->GetRootComponent());

		if (!IsValid(root))
			return false;

		if (root->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			EGripCollisionType CollisionType = IVRGripInterface::Execute_GetPrimaryGripType(root, bIsSlotGrip);

			float Stiffness;
			float Damping;
			IVRGripInterface::Execute_GetGripStiffnessAndDamping(root, Stiffness, Damping);

			return GripComponent(root, WorldOffset, bWorldOffsetIsRelative, OptionalSnapToSocketName,
				OptionalBoneToGripName,
				CollisionType,
				IVRGripInterface::Execute_GripLateUpdateSetting(root),
				IVRGripInterface::Execute_GripMovementReplicationType(root),
				Stiffness,
				Damping,
				bIsSlotGrip
				);

		}
		else if (Actor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			EGripCollisionType CollisionType = IVRGripInterface::Execute_GetPrimaryGripType(Actor, bIsSlotGrip);

			float Stiffness;
			float Damping;
			IVRGripInterface::Execute_GetGripStiffnessAndDamping(Actor, Stiffness, Damping);

			return GripActor(Actor, WorldOffset, bWorldOffsetIsRelative, OptionalSnapToSocketName,
				OptionalBoneToGripName,
				CollisionType,
				IVRGripInterface::Execute_GripLateUpdateSetting(Actor),
				IVRGripInterface::Execute_GripMovementReplicationType(Actor),
				Stiffness,
				Damping,
				bIsSlotGrip
				);
		}
		else
		{

			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController GripObjectByInterface was called on an object that doesn't implement the interface and doesn't have a parent that implements the interface!");
			return false;
		}
	}

	return false;
}

bool UGripMotionControllerComponent::DropObjectByInterface(UObject* ObjectToDrop, uint8 GripIDToDrop, FVector OptionalAngularVelocity, FVector OptionalLinearVelocity)
{
	return DropObjectByInterface_Implementation(ObjectToDrop, GripIDToDrop, OptionalAngularVelocity, OptionalLinearVelocity, false);
}

bool UGripMotionControllerComponent::DropObjectByInterface_Implementation(UObject* ObjectToDrop, uint8 GripIDToDrop, FVector OptionalAngularVelocity, FVector OptionalLinearVelocity, bool bSkipNotify)
{

	FBPActorGripInformation * GripInfo = nullptr;
	if (IsValid(ObjectToDrop))
	{
		GripInfo = GrippedObjects.FindByKey(ObjectToDrop);
		if (!GripInfo)
			GripInfo = LocallyGrippedObjects.FindByKey(ObjectToDrop);
	}
	else if (GripIDToDrop != INVALID_VRGRIP_ID)
	{
		GripInfo = GrippedObjects.FindByKey(GripIDToDrop);
		if (!GripInfo)
			GripInfo = LocallyGrippedObjects.FindByKey(GripIDToDrop);
	}

	if (GripInfo == nullptr || !IsValid(GripInfo->GrippedObject))
	{
		return false;
	}

	if (UPrimitiveComponent * PrimComp = Cast<UPrimitiveComponent>(GripInfo->GrippedObject))
	{
		AActor * Owner = PrimComp->GetOwner();

		if (!Owner)
			return false;

		if (PrimComp->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			return DropGrip_Implementation(*GripInfo, IVRGripInterface::Execute_SimulateOnDrop(PrimComp), OptionalAngularVelocity, OptionalLinearVelocity, bSkipNotify);

		}
		else if (Owner->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			return DropGrip_Implementation(*GripInfo, IVRGripInterface::Execute_SimulateOnDrop(Owner), OptionalAngularVelocity, OptionalLinearVelocity, bSkipNotify);

		}
		else
		{

			return DropGrip_Implementation(*GripInfo, true, OptionalAngularVelocity, OptionalLinearVelocity, bSkipNotify);

		}
	}
	else if (AActor * Actor = Cast<AActor>(GripInfo->GrippedObject))
	{
		UPrimitiveComponent * root = Cast<UPrimitiveComponent>(Actor->GetRootComponent());

		if (!IsValid(root))
			return false;

		if (root->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			return DropGrip_Implementation(*GripInfo, IVRGripInterface::Execute_SimulateOnDrop(root), OptionalAngularVelocity, OptionalLinearVelocity, bSkipNotify);
		}
		else if (Actor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			return DropGrip_Implementation(*GripInfo, IVRGripInterface::Execute_SimulateOnDrop(Actor), OptionalAngularVelocity, OptionalLinearVelocity, bSkipNotify);
		}
		else
		{

			return DropGrip_Implementation(*GripInfo, true, OptionalAngularVelocity, OptionalLinearVelocity, bSkipNotify);
		}
	}

	return false;
}

bool UGripMotionControllerComponent::GripActor(
	AActor* ActorToGrip, 
	const FTransform &WorldOffset, 
	bool bWorldOffsetIsRelative,
	FName OptionalSnapToSocketName, 
	FName OptionalBoneToGripName,
	EGripCollisionType GripCollisionType, 
	EGripLateUpdateSettings GripLateUpdateSetting,
	EGripMovementReplicationSettings GripMovementReplicationSetting,
	float GripStiffness, 
	float GripDamping,
	bool bIsSlotGrip)
{

	if (IsTravelingOrNullWorld())
		return false;

	bool bIsLocalGrip = (GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive || GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep);

	if (!IsServer() && !bIsLocalGrip)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController grab function was called on the client side as a replicated grip");
		return false;
	}

	if (!ActorToGrip || !IsValid(ActorToGrip))
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController grab function was passed an invalid or pending kill actor");
		return false;
	}

	if (GetIsObjectHeld(ActorToGrip))
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController grab function was passed an already gripped actor");
		return false;
	}

	UPrimitiveComponent *root = Cast<UPrimitiveComponent>(ActorToGrip->GetRootComponent());

	if (!root)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController tried to grip an actor without a UPrimitiveComponent Root");
		return false; 
	}

	if (root->Mobility != EComponentMobility::Movable && (GripCollisionType != EGripCollisionType::CustomGrip && GripCollisionType != EGripCollisionType::EventsOnly))
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController tried to grip an actor set to static mobility not with a Custom Grip");
		return false; 
	}

	FBPAdvGripSettings AdvancedGripSettings;
	UObject * ObjectToCheck = NULL; 

	TArray<FBPGripPair> HoldingControllers;
	bool bIsHeld = false;
	bool bHadOriginalSettings = false;
	bool bOriginalGravity = false;
	bool bOriginalReplication = false;

	if (root->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		if(IVRGripInterface::Execute_DenyGripping(root, this))
			return false; 

		IVRGripInterface::Execute_IsHeld(root, HoldingControllers, bIsHeld);
		bool bAllowMultipleGrips = IVRGripInterface::Execute_AllowsMultipleGrips(root);
		if (bIsHeld && !bAllowMultipleGrips)
		{
			return false; 
		}
		else if (bIsHeld)
		{

			if (HoldingControllers[0].HoldingController != nullptr)
			{
				FBPActorGripInformation* gripInfo = HoldingControllers[0].HoldingController->GetGripPtrByID(HoldingControllers[0].GripID);

				if (gripInfo != nullptr)
				{
					bHadOriginalSettings = true;
					bOriginalGravity = gripInfo->bOriginalGravity;
					bOriginalReplication = gripInfo->bOriginalReplicatesMovement;
				}
			}
		}

		AdvancedGripSettings = IVRGripInterface::Execute_AdvancedGripSettings(root);
		ObjectToCheck = root;
	}
	else if (ActorToGrip->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		if(IVRGripInterface::Execute_DenyGripping(ActorToGrip, this))
			return false; 

		IVRGripInterface::Execute_IsHeld(ActorToGrip, HoldingControllers, bIsHeld);
		bool bAllowMultipleGrips = IVRGripInterface::Execute_AllowsMultipleGrips(ActorToGrip);
		if (bIsHeld && !bAllowMultipleGrips)
		{
			return false; 
		}
		else if (bIsHeld)
		{

			if (HoldingControllers[0].HoldingController != nullptr)
			{
				FBPActorGripInformation* gripInfo = HoldingControllers[0].HoldingController->GetGripPtrByID(HoldingControllers[0].GripID);

				if (gripInfo != nullptr)
				{
					bHadOriginalSettings = true;
					bOriginalGravity = gripInfo->bOriginalGravity;
					bOriginalReplication = gripInfo->bOriginalReplicatesMovement;
				}
			}
		}

		AdvancedGripSettings = IVRGripInterface::Execute_AdvancedGripSettings(ActorToGrip);
		ObjectToCheck = ActorToGrip;
	}

	ActorToGrip->AddTickPrerequisiteComponent(this);

	FBPActorGripInformation newActorGrip;
	newActorGrip.GripID = GetNextGripID(bIsLocalGrip);
	newActorGrip.GripCollisionType = GripCollisionType;
	newActorGrip.GrippedObject = ActorToGrip;
	if (bHadOriginalSettings)
	{
		newActorGrip.bOriginalReplicatesMovement = bOriginalReplication;
		newActorGrip.bOriginalGravity = bOriginalGravity;
	}
	else
	{
		newActorGrip.bOriginalReplicatesMovement = ActorToGrip->IsReplicatingMovement();
		newActorGrip.bOriginalGravity = root->IsGravityEnabled();
	}
	newActorGrip.Stiffness = GripStiffness;
	newActorGrip.Damping = GripDamping;
	newActorGrip.AdvancedGripSettings = AdvancedGripSettings;
	newActorGrip.ValueCache.bWasInitiallyRepped = true; 
	newActorGrip.bIsSlotGrip = bIsSlotGrip;
	newActorGrip.GrippedBoneName = OptionalBoneToGripName;
	newActorGrip.SlotName = OptionalSnapToSocketName;

	switch(newActorGrip.GripCollisionType)
	{
	case EGripCollisionType::ManipulationGrip:
	case EGripCollisionType::ManipulationGripWithWristTwist:
	{
		newActorGrip.GripLateUpdateSetting = EGripLateUpdateSettings::LateUpdatesAlwaysOff; 
	}break;

	default:
	{
		newActorGrip.GripLateUpdateSetting = GripLateUpdateSetting;
	}break;
	}

	if (GripMovementReplicationSetting == EGripMovementReplicationSettings::KeepOriginalMovement)
	{
		if (ActorToGrip->IsReplicatingMovement())
		{
			newActorGrip.GripMovementReplicationSetting = EGripMovementReplicationSettings::ForceServerSideMovement;
		}
		else
		{
			newActorGrip.GripMovementReplicationSetting = EGripMovementReplicationSettings::ForceClientSideMovement;
		}
	}
	else
		newActorGrip.GripMovementReplicationSetting = GripMovementReplicationSetting;

	newActorGrip.GripTargetType = EGripTargetType::ActorGrip;

	if (OptionalSnapToSocketName.IsValid() && WorldOffset.Equals(FTransform::Identity) && root->DoesSocketExist(OptionalSnapToSocketName))
	{

		FTransform sockTrans = root->GetSocketTransform(OptionalSnapToSocketName, ERelativeTransformSpace::RTS_Component);
		sockTrans.SetScale3D(FVector(1.f) / root->GetComponentScale()); 
		newActorGrip.RelativeTransform = sockTrans.Inverse();
		newActorGrip.bIsSlotGrip = true; 

		ObjectToCheck = NULL; 

		newActorGrip.SlotName = OptionalSnapToSocketName;
	}
	else if (bWorldOffsetIsRelative)
	{
		if (bSkipPivotTransformAdjustment && IsValid(CustomPivotComponent) && !bIsSlotGrip)
		{
			newActorGrip.RelativeTransform = (WorldOffset * this->GetComponentTransform()).GetRelativeTransform(CustomPivotComponent->GetComponentTransform());
		}
		else
		{
			newActorGrip.RelativeTransform = WorldOffset;
		}
	}
	else
	{
		newActorGrip.RelativeTransform = WorldOffset.GetRelativeTransform(GetPivotTransform());
	}

	if (!bIsLocalGrip)
	{
		int32 Index = GrippedObjects.Add(newActorGrip);
		DIRTY_GRIPPED_OBJECTS();

		if (Index != INDEX_NONE)
			NotifyGrip(GrippedObjects[Index]);

	}
	else
	{
		if (!IsLocallyControlled())
		{
			LocalTransactionBuffer.Add(newActorGrip);
#if WITH_PUSH_MODEL
			MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, LocalTransactionBuffer, this);
#endif
		}

		int32 Index = LocallyGrippedObjects.Add(newActorGrip);
		DIRTY_LOCALLY_GRIPPED_OBJECTS();

		if (Index != INDEX_NONE)
		{
			if (!IsLocallyControlled())
			{
				if (!HandleGripReplication(LocallyGrippedObjects[Index]))
				{
					return true;
				}
			}
			else
			{
				if (!NotifyGrip(LocallyGrippedObjects[Index]))
				{
					return true;
				}
			}

			if (bIsLocalGrip && IsLocallyControlled() && !IsServer() && !IsTornOff() && newActorGrip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive)
			{
				Index = LocallyGrippedObjects.IndexOfByKey(newActorGrip.GripID);
				if (Index != INDEX_NONE)
				{
					FBPActorGripInformation GripInfo = LocallyGrippedObjects[Index];
					Server_NotifyLocalGripAddedOrChanged(GripInfo);
				}
			}
		}
	}

	return true;
}

bool UGripMotionControllerComponent::DropActor(AActor* ActorToDrop, bool bSimulate, FVector OptionalAngularVelocity, FVector OptionalLinearVelocity)
{
	if (!ActorToDrop)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop function was passed an invalid actor");
		return false;
	}

	FBPActorGripInformation * GripToDrop = LocallyGrippedObjects.FindByKey(ActorToDrop);

	if(GripToDrop)
		return DropGrip_Implementation(*GripToDrop, bSimulate, OptionalAngularVelocity, OptionalLinearVelocity);

	if (!IsServer())
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop function was called on the client side with a replicated grip");
		return false;
	}

	GripToDrop = GrippedObjects.FindByKey(ActorToDrop);
	if (GripToDrop)
		return DropGrip_Implementation(*GripToDrop, bSimulate, OptionalAngularVelocity, OptionalLinearVelocity);

	return false;
}

bool UGripMotionControllerComponent::GripComponent(
	UPrimitiveComponent* ComponentToGrip, 
	const FTransform &WorldOffset, 
	bool bWorldOffsetIsRelative, 
	FName OptionalSnapToSocketName, 
	FName OptionalBoneToGripName,
	EGripCollisionType GripCollisionType,
	EGripLateUpdateSettings GripLateUpdateSetting,
	EGripMovementReplicationSettings GripMovementReplicationSetting,
	float GripStiffness, 
	float GripDamping,
	bool bIsSlotGrip
	)
{

	if (IsTravelingOrNullWorld())
		return false;

	bool bIsLocalGrip = (GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive || GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep);

	if (!IsServer() && !bIsLocalGrip)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController grab function was called on the client side with a replicating grip");
		return false;
	}

	if (!ComponentToGrip || !IsValid(ComponentToGrip))
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController grab function was passed an invalid or pending kill component");
		return false;
	}

	if (GetIsObjectHeld(ComponentToGrip))
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController grab function was passed an already gripped component");
		return false;
	}

	if (ComponentToGrip->Mobility != EComponentMobility::Movable && (GripCollisionType != EGripCollisionType::CustomGrip && GripCollisionType != EGripCollisionType::EventsOnly))
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController tried to grip a component set to static mobility not in CustomGrip mode");
		return false; 
	}

	FBPAdvGripSettings AdvancedGripSettings;
	UObject * ObjectToCheck = NULL;

	TArray<FBPGripPair> HoldingControllers;
	bool bIsHeld = false;
	bool bHadOriginalSettings = false;
	bool bOriginalGravity = false;
	bool bOriginalReplication = false;

	if (ComponentToGrip->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		if(IVRGripInterface::Execute_DenyGripping(ComponentToGrip, this))
			return false; 

		IVRGripInterface::Execute_IsHeld(ComponentToGrip, HoldingControllers, bIsHeld);
		bool bAllowMultipleGrips = IVRGripInterface::Execute_AllowsMultipleGrips(ComponentToGrip);
		if (bIsHeld && !bAllowMultipleGrips)
		{
			return false; 
		}
		else if(bIsHeld)
		{

			if (HoldingControllers[0].HoldingController != nullptr)
			{
				FBPActorGripInformation gripInfo;
				EBPVRResultSwitch result;
				HoldingControllers[0].HoldingController->GetGripByID(gripInfo, HoldingControllers[0].GripID, result);

				if (result != EBPVRResultSwitch::OnFailed)
				{
					bHadOriginalSettings = true;
					bOriginalGravity = gripInfo.bOriginalGravity;
					bOriginalReplication = gripInfo.bOriginalReplicatesMovement;
				}
			}
		}

		AdvancedGripSettings = IVRGripInterface::Execute_AdvancedGripSettings(ComponentToGrip);
		ObjectToCheck = ComponentToGrip;
	}

	ComponentToGrip->AddTickPrerequisiteComponent(this);

	FBPActorGripInformation newComponentGrip;
	newComponentGrip.GripID = GetNextGripID(bIsLocalGrip);
	newComponentGrip.GripCollisionType = GripCollisionType;
	newComponentGrip.GrippedObject = ComponentToGrip;

	if (bHadOriginalSettings)
	{
		newComponentGrip.bOriginalReplicatesMovement = bOriginalReplication;
		newComponentGrip.bOriginalGravity = bOriginalGravity;
	}
	else
	{
		if (ComponentToGrip->GetOwner())
			newComponentGrip.bOriginalReplicatesMovement = ComponentToGrip->GetOwner()->IsReplicatingMovement();

		newComponentGrip.bOriginalGravity = ComponentToGrip->IsGravityEnabled();
	}
	newComponentGrip.Stiffness = GripStiffness;
	newComponentGrip.Damping = GripDamping;
	newComponentGrip.AdvancedGripSettings = AdvancedGripSettings;
	newComponentGrip.GripTargetType = EGripTargetType::ComponentGrip;
	newComponentGrip.ValueCache.bWasInitiallyRepped = true; 
	newComponentGrip.bIsSlotGrip = bIsSlotGrip;
	newComponentGrip.GrippedBoneName = OptionalBoneToGripName;
	newComponentGrip.SlotName = OptionalSnapToSocketName;

	switch (newComponentGrip.GripCollisionType)
	{
	case EGripCollisionType::ManipulationGrip:
	case EGripCollisionType::ManipulationGripWithWristTwist:
	{
		newComponentGrip.GripLateUpdateSetting = EGripLateUpdateSettings::LateUpdatesAlwaysOff; 
	}break;

	default:
	{
		newComponentGrip.GripLateUpdateSetting = GripLateUpdateSetting;
	}break;
	}

	if (GripMovementReplicationSetting == EGripMovementReplicationSettings::KeepOriginalMovement)
	{
		if (ComponentToGrip->GetOwner())
		{
			if (ComponentToGrip->GetOwner()->IsReplicatingMovement())
			{
				newComponentGrip.GripMovementReplicationSetting = EGripMovementReplicationSettings::ForceServerSideMovement;
			}
			else
			{
				newComponentGrip.GripMovementReplicationSetting = EGripMovementReplicationSettings::ForceClientSideMovement;
			}
		}
		else
			newComponentGrip.GripMovementReplicationSetting = EGripMovementReplicationSettings::ForceClientSideMovement;
	}
	else
		newComponentGrip.GripMovementReplicationSetting = GripMovementReplicationSetting;

	if (OptionalSnapToSocketName.IsValid() && WorldOffset.Equals(FTransform::Identity) && ComponentToGrip->DoesSocketExist(OptionalSnapToSocketName))
	{

		FTransform sockTrans = ComponentToGrip->GetSocketTransform(OptionalSnapToSocketName, ERelativeTransformSpace::RTS_Component);
		sockTrans.SetScale3D(FVector(1.f) / ComponentToGrip->GetComponentScale()); 
		newComponentGrip.RelativeTransform = sockTrans.Inverse();
		newComponentGrip.bIsSlotGrip = true; 

		ObjectToCheck = NULL; 
	}
	else if (bWorldOffsetIsRelative)
	{
		if (bSkipPivotTransformAdjustment && IsValid(CustomPivotComponent) && !bIsSlotGrip)
		{
			newComponentGrip.RelativeTransform = (WorldOffset * this->GetComponentTransform()).GetRelativeTransform(CustomPivotComponent->GetComponentTransform());
		}
		else
		{
			newComponentGrip.RelativeTransform = WorldOffset;
		}
	}
	else
	{
		newComponentGrip.RelativeTransform = WorldOffset.GetRelativeTransform(GetPivotTransform());
	}

	if (!bIsLocalGrip)
	{
		int32 Index = GrippedObjects.Add(newComponentGrip);
		DIRTY_GRIPPED_OBJECTS();

		if (Index != INDEX_NONE)
			NotifyGrip(GrippedObjects[Index]);

	}
	else
	{
		if (!IsLocallyControlled())
		{
			LocalTransactionBuffer.Add(newComponentGrip);
#if WITH_PUSH_MODEL
			MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, LocalTransactionBuffer, this);
#endif
		}

		int32 Index = LocallyGrippedObjects.Add(newComponentGrip);
		DIRTY_LOCALLY_GRIPPED_OBJECTS();

		if (Index != INDEX_NONE)
		{
			if (!IsLocallyControlled())
			{		
				if (!HandleGripReplication(LocallyGrippedObjects[Index]))
				{
					return true;
				}
			}
			else
			{
				if (!NotifyGrip(LocallyGrippedObjects[Index]))
				{
					return true;
				}
			}

			if (bIsLocalGrip && IsLocallyControlled() && !IsServer() && !IsTornOff() && newComponentGrip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive)
			{
				Index = LocallyGrippedObjects.IndexOfByKey(newComponentGrip.GripID);
				if (Index != INDEX_NONE)
				{
					FBPActorGripInformation GripInfo = LocallyGrippedObjects[Index];
					Server_NotifyLocalGripAddedOrChanged(GripInfo);
				}
			}
		}
	}

	return true;
}

bool UGripMotionControllerComponent::DropComponent(UPrimitiveComponent * ComponentToDrop, bool bSimulate, FVector OptionalAngularVelocity, FVector OptionalLinearVelocity)
{
	FBPActorGripInformation *GripInfo;

	GripInfo = LocallyGrippedObjects.FindByKey(ComponentToDrop);

	if (GripInfo != nullptr)
	{
		return DropGrip_Implementation(*GripInfo, bSimulate, OptionalAngularVelocity, OptionalLinearVelocity);
	}

	if (!IsServer())
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop function was called on the client side for a replicated grip");
		return false;
	}

	GripInfo = GrippedObjects.FindByKey(ComponentToDrop);

	if (GripInfo != nullptr)
	{
		return DropGrip_Implementation(*GripInfo, bSimulate, OptionalAngularVelocity, OptionalLinearVelocity);
	}
	else
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop function was passed an invalid component");
		return false;
	}

}

bool UGripMotionControllerComponent::DropGrip(const FBPActorGripInformation& Grip, bool bSimulate, FVector OptionalAngularVelocity, FVector OptionalLinearVelocity)
{
	return DropGrip_Implementation(Grip, bSimulate, OptionalAngularVelocity, OptionalLinearVelocity);
}

bool UGripMotionControllerComponent::DropGrip_Implementation(const FBPActorGripInformation &Grip, bool bSimulate, FVector OptionalAngularVelocity, FVector OptionalLinearVelocity, bool bSkipNotify)
{
	int FoundIndex = 0;
	bool bIsServer = IsServer();
	bool bWasLocalGrip = false;
	if (!LocallyGrippedObjects.Find(Grip, FoundIndex)) 
	{
		if (!bIsServer)
		{
			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop function was called on the client side for a replicated grip");
			return false;
		}

		if (!GrippedObjects.Find(Grip, FoundIndex)) 
		{
			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop function was passed an invalid drop");
			return false;
		}

		bWasLocalGrip = false;
	}
	else
		bWasLocalGrip = true;

	if (bWasLocalGrip && bIsServer)
	{
		for (int i = LocalTransactionBuffer.Num() - 1; i >= 0; i--)
		{
			if (LocalTransactionBuffer[i].GripID == Grip.GripID)
				LocalTransactionBuffer.RemoveAt(i);
		}
#if WITH_PUSH_MODEL
		MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, LocalTransactionBuffer, this);
#endif
	}

	UPrimitiveComponent * PrimComp = nullptr;

	AActor * pActor = nullptr;
	if (bWasLocalGrip)
	{
		PrimComp = LocallyGrippedObjects[FoundIndex].GetGrippedComponent();
		pActor = LocallyGrippedObjects[FoundIndex].GetGrippedActor();
	}
	else
	{
		PrimComp = GrippedObjects[FoundIndex].GetGrippedComponent();
		pActor = GrippedObjects[FoundIndex].GetGrippedActor();
	}

	if (!PrimComp && pActor)
		PrimComp = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

	if(!PrimComp)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop function was passed an invalid drop or CleanUpBadGrip wascalled");

	}
	else
	{
		if (bSimulate && (!OptionalLinearVelocity.IsNearlyZero() || !OptionalAngularVelocity.IsNearlyZero()))
		{
			if (Grip.GripCollisionType != EGripCollisionType::EventsOnly)
			{

				if (!Grip.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings || !Grip.AdvancedGripSettings.PhysicsSettings.bSkipSettingSimulating)
				{
					if (PrimComp->IsSimulatingPhysics() != bSimulate)
					{
						PrimComp->SetSimulatePhysics(bSimulate);
					}
				}
			}

			if (PrimComp->IsSimulatingPhysics())
			{
				PrimComp->SetPhysicsLinearVelocity(OptionalLinearVelocity);
				PrimComp->SetPhysicsAngularVelocityInDegrees(OptionalAngularVelocity);
			}
		}
	}

	if (bWasLocalGrip)
	{

		FBPActorGripInformation GripInfo = LocallyGrippedObjects[FoundIndex];

		if (IsLocallyControlled() && !IsServer()) 
		{
			if (!IsTornOff())
			{
				FTransform_NetQuantize TransformAtDrop = FTransform::Identity;

				switch (GripInfo.GripTargetType)
				{
				case EGripTargetType::ActorGrip:
				{
					if (AActor * GrippedActor = GripInfo.GetGrippedActor())
					{
						TransformAtDrop = GrippedActor->GetActorTransform();
					}
				}; break;
				case EGripTargetType::ComponentGrip:
				{
					if (UPrimitiveComponent * GrippedPrim = GripInfo.GetGrippedComponent())
					{
						TransformAtDrop = GrippedPrim->GetComponentTransform();
					}
				}break;
				default:break;
				}

				if(!bSkipNotify)
					Server_NotifyLocalGripRemoved(GripInfo.GripID, TransformAtDrop, OptionalAngularVelocity, OptionalLinearVelocity);
			}

			if (LocallyGrippedObjects.Num() > 0 && LocallyGrippedObjects.Find(GripInfo, FoundIndex))
			{

				Drop_Implementation(GripInfo, bSimulate);
			}
		}
		else 
		{
			NotifyDrop(GripInfo, bSimulate);
		}
	}
	else
		NotifyDrop(GrippedObjects[FoundIndex], bSimulate);

	return true;
}

bool UGripMotionControllerComponent::DropAndSocketObject(const FTransform_NetQuantize & RelativeTransformToParent, UObject * ObjectToDrop, uint8 GripIDToDrop, USceneComponent * SocketingParent, FName OptionalSocketName, bool bWeldBodies)
{
	if (!SocketingParent)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop and socket function was passed an invalid socketing parent");
		return false;
	}

	if (!ObjectToDrop && GripIDToDrop == INVALID_VRGRIP_ID)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop and socket function was passed an invalid object");
		return false;
	}

	bool bWasLocalGrip = false;
	FBPActorGripInformation * GripInfo = nullptr;

	if (ObjectToDrop)
		GripInfo = LocallyGrippedObjects.FindByKey(ObjectToDrop);
	else if (GripIDToDrop != INVALID_VRGRIP_ID)
		GripInfo = LocallyGrippedObjects.FindByKey(GripIDToDrop);

	if(GripInfo) 
	{
		bWasLocalGrip = true;
	}
	else
	{
		if (!IsServer())
		{
			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop and socket function was called on the client side for a replicated grip");
			return false;
		}

		if(ObjectToDrop)
			GripInfo = GrippedObjects.FindByKey(ObjectToDrop);
		else if(GripIDToDrop != INVALID_VRGRIP_ID)
			GripInfo = GrippedObjects.FindByKey(GripIDToDrop);

		if(GripInfo) 
		{
			bWasLocalGrip = false;
		}
		else
		{
			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop and socket function was passed an invalid drop");
			return false;
		}
	}

	if(GripInfo)
		return DropAndSocketGrip_Implementation(*GripInfo, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);

	return false;
}

bool UGripMotionControllerComponent::DropAndSocketGrip(const FBPActorGripInformation& GripToDrop, USceneComponent* SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize& RelativeTransformToParent, bool bWeldBodies)
{
	return DropAndSocketGrip_Implementation(GripToDrop, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);
}

bool UGripMotionControllerComponent::DropAndSocketGrip_Implementation(const FBPActorGripInformation & GripToDrop, USceneComponent * SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize & RelativeTransformToParent, bool bWeldBodies, bool bSkipServerNotify)
{
	if (!SocketingParent || !IsValid(SocketingParent))
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop and socket function was passed an invalid socketing parent");
		return false;
	}

	bool bWasLocalGrip = false;
	FBPActorGripInformation * GripInfo = nullptr;

	GripInfo = LocallyGrippedObjects.FindByKey(GripToDrop);
	if (GripInfo) 
	{
		bWasLocalGrip = true;
	}
	else
	{
		if (!IsServer())
		{
			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop and socket function was called on the client side for a replicated grip");
			return false;
		}

		GripInfo = GrippedObjects.FindByKey(GripToDrop);

		if (GripInfo) 
		{
			bWasLocalGrip = false;
		}
		else
		{
			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop and socket function was passed an invalid drop");
			return false;
		}
	}

	UPrimitiveComponent * PrimComp = nullptr;

	AActor * pActor = nullptr;

	PrimComp = GripInfo->GetGrippedComponent();
	pActor = GripInfo->GetGrippedActor();

	if (!PrimComp && pActor)
		PrimComp = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

	if (!PrimComp)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop and socket function was passed an invalid drop or CleanUpBadGrip wascalled");

	}

	UObject * GrippedObject = GripInfo->GrippedObject;

	if (!GrippedObject || !IsValid(GrippedObject))
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController drop and socket function was passed an invalid or pending kill gripped object");
		return false;
	}

	int PhysicsHandleIndex = INDEX_NONE;
	GetPhysicsGripIndex(*GripInfo, PhysicsHandleIndex);

	if (bWasLocalGrip)
	{
		if (IsLocallyControlled() && !IsServer())
		{
			if (!IsTornOff() && !bSkipServerNotify)
			{
				Server_NotifyDropAndSocketGrip(GripInfo->GripID, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);
			}

			OnSocketingObject.Broadcast(*GripInfo, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);
			Socket_Implementation(GrippedObject, (PhysicsHandleIndex != INDEX_NONE), SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);

			DropAndSocket_Implementation(*GripInfo);
		}
		else 
		{

			NotifyDropAndSocket(*GripInfo, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);
		}
	}
	else
	{

		NotifyDropAndSocket(*GripInfo, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);
	}

	return true;
}

void UGripMotionControllerComponent::SetSocketTransform(UObject* ObjectToSocket,  const FTransform_NetQuantize RelativeTransformToParent)
{
	if (ObjectsWaitingForSocketUpdate.RemoveSingle(ObjectToSocket) < 1)
	{

		for (int i = ObjectsWaitingForSocketUpdate.Num() - 1; i >= 0; --i)
		{
			if (ObjectsWaitingForSocketUpdate[i] == nullptr)
				ObjectsWaitingForSocketUpdate.RemoveAt(i);
		}

		return;
	}

	if (!ObjectToSocket || !IsValid(ObjectToSocket))
		return;

	if (UPrimitiveComponent * root = Cast<UPrimitiveComponent>(ObjectToSocket))
	{

		if(root->GetAttachParent())
			root->SetRelativeTransform(RelativeTransformToParent);
	}
	else if (AActor * pActor = Cast<AActor>(ObjectToSocket))
	{

		if(pActor->GetAttachParentActor())
			pActor->SetActorRelativeTransform(RelativeTransformToParent);
	}
}

bool UGripMotionControllerComponent::Server_NotifyDropAndSocketGrip_Validate(uint8 GripID, USceneComponent * SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize & RelativeTransformToParent, bool bWeldBodies)
{
	return true;
}

void UGripMotionControllerComponent::Server_NotifyDropAndSocketGrip_Implementation(uint8 GripID, USceneComponent * SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize & RelativeTransformToParent, bool bWeldBodies)
{
	FBPActorGripInformation FoundGrip;
	EBPVRResultSwitch Result;

	GetGripByID(FoundGrip, GripID, Result);

	if (Result == EBPVRResultSwitch::OnFailed)
		return;

	int PhysicsHandleIndex = INDEX_NONE;
	GetPhysicsGripIndex(FoundGrip, PhysicsHandleIndex);

	if (FoundGrip.GrippedObject)
	{
		OnSocketingObject.Broadcast(FoundGrip, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);
		Socket_Implementation(FoundGrip.GrippedObject, (PhysicsHandleIndex != INDEX_NONE), SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);
	}

	if (!DropAndSocketGrip_Implementation(FoundGrip, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies, true))
	{
		DropGrip_Implementation(FoundGrip, false, FVector::ZeroVector, FVector::ZeroVector, true);
	}

}

void UGripMotionControllerComponent::Socket_Implementation(UObject * ObjectToSocket, bool bWasSimulating, USceneComponent * SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize & RelativeTransformToParent, bool bWeldBodies)
{

	if (!SocketingParent || !SocketingParent->IsValidLowLevelFast() || !ObjectToSocket || !ObjectToSocket->IsValidLowLevelFast())
	{
		if (!SocketingParent || !SocketingParent->IsValidLowLevelFast())
		{
			UE_LOGF(LogVRMotionController, Error, "VRGripMotionController Socket_Implementation was called with an invalid Socketing Parent object");
		}
		else
		{
			UE_LOGF(LogVRMotionController, Error, "VRGripMotionController Socket_Implementation was called with an invalid Object to Socket");
		}
		return;
	}

	FAttachmentTransformRules TransformRule = FAttachmentTransformRules::KeepWorldTransform;
	TransformRule.bWeldSimulatedBodies = bWeldBodies;

	if (UPrimitiveComponent * root = Cast<UPrimitiveComponent>(ObjectToSocket))
	{
		if (FBodyInstance* rBodyInstance = root->GetBodyInstance())
		{
			if (rBodyInstance->OnRecalculatedMassProperties().IsBoundToObject(this))
			{
				rBodyInstance->OnRecalculatedMassProperties().RemoveAll(this);
			}
		}

		if (bWasSimulating || root->IsSimulatingPhysics())
		{
			root->SetSimulatePhysics(false);
			bWasSimulating = true;
		}

		root->AttachToComponent(SocketingParent, TransformRule, OptionalSocketName);
		root->SetRelativeTransform(RelativeTransformToParent);
	}
	else if (AActor * pActor = Cast<AActor>(ObjectToSocket))
	{

		if (UPrimitiveComponent * rootComp = Cast<UPrimitiveComponent>(pActor->GetRootComponent()))
		{
			if (FBodyInstance* rBodyInstance = rootComp->GetBodyInstance())
			{
				if (rBodyInstance->OnRecalculatedMassProperties().IsBoundToObject(this))
				{
					rBodyInstance->OnRecalculatedMassProperties().RemoveAll(this);
				}
			}

			if (bWasSimulating || rootComp->IsSimulatingPhysics())
			{

				rootComp->SetSimulatePhysics(false);
				bWasSimulating = true;
			}
		}

		pActor->AttachToComponent(SocketingParent, TransformRule, OptionalSocketName);
		pActor->SetActorRelativeTransform(RelativeTransformToParent);

	}

	if (bWasSimulating)
	{
		ObjectsWaitingForSocketUpdate.Add(ObjectToSocket);
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UGripMotionControllerComponent::SetSocketTransform, ObjectToSocket, RelativeTransformToParent));
	}
}

void UGripMotionControllerComponent::NotifyDropAndSocket_Implementation(const FBPActorGripInformation &NewDrop, USceneComponent* SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize& RelativeTransformToParent, bool bWeldBodies)
{

	if ((NewDrop.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive ||
		NewDrop.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep) &&
		IsLocallyControlled() &&
		!IsServer())
	{

		if (FBPActorGripInformation * GripInfo = GetGripPtrByID(NewDrop.GripID))
		{
			DropAndSocketGrip_Implementation(*GripInfo, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies, true);
		}
		return;
	}

	int PhysicsHandleIndex = INDEX_NONE;
	GetPhysicsGripIndex(NewDrop, PhysicsHandleIndex);

	if (NewDrop.GrippedObject)
	{
		OnSocketingObject.Broadcast(NewDrop, SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);
		Socket_Implementation(NewDrop.GrippedObject, (PhysicsHandleIndex != INDEX_NONE), SocketingParent, OptionalSocketName, RelativeTransformToParent, bWeldBodies);
	}

	DropAndSocket_Implementation(NewDrop);
}

void UGripMotionControllerComponent::DropAndSocket_Implementation(const FBPActorGripInformation &NewDrop)
{
	UGripMotionControllerComponent * HoldingController = nullptr;
	bool bIsHeld = false;

	DestroyPhysicsHandle(NewDrop);

	bool bHadGripAuthority = HasGripAuthority(NewDrop);

	UPrimitiveComponent *root = NULL;
	AActor * pActor = NULL;

	switch (NewDrop.GripTargetType)
	{
	case EGripTargetType::ActorGrip:

	{
		pActor = NewDrop.GetGrippedActor();

		if (pActor)
		{
			root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

			pActor->RemoveTickPrerequisiteComponent(this);

			if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
			{
				OwningPawn->MoveIgnoreActorRemove(pActor);

			}

			if (root)
			{

				if ((NewDrop.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && NewDrop.AdvancedGripSettings.PhysicsSettings.bTurnOffGravityDuringGrip) ||
					(NewDrop.GripMovementReplicationSetting == EGripMovementReplicationSettings::ForceServerSideMovement && !IsServer()))
					root->SetEnableGravity(NewDrop.bOriginalGravity);

				root->SetSimulatePhysics(false);
			}

			if (IsServer()) 
			{
				pActor->SetReplicateMovement(NewDrop.bOriginalReplicatesMovement);
			}

			if (pActor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				IVRGripInterface::Execute_SetHeld(pActor, this, NewDrop.GripID, false);

				if(NewDrop.SecondaryGripInfo.bHasSecondaryAttachment || SecondaryGripIDs.Contains(NewDrop.GripID))
				{
					IVRGripInterface::Execute_OnSecondaryGripRelease(pActor, this, NewDrop.SecondaryGripInfo.SecondaryAttachment, NewDrop);
					OnSecondaryGripRemoved.Broadcast(NewDrop);
				}

				SecondaryGripIDs.Remove(NewDrop.GripID);

				TArray<UVRGripScriptBase*> GripScripts;
				if (IVRGripInterface::Execute_GetGripScripts(pActor, GripScripts))
				{
					for (UVRGripScriptBase* Script : GripScripts)
					{
						if (Script)
						{
							if (NewDrop.SecondaryGripInfo.bHasSecondaryAttachment)
								Script->OnSecondaryGripRelease(this, NewDrop.SecondaryGripInfo.SecondaryAttachment, NewDrop);

							Script->OnGripRelease(this, NewDrop, true);
						}
					}
				}

				IVRGripInterface::Execute_OnGripRelease(pActor, this, NewDrop, true);
				if (IVRGripInterface* GripInterface = Cast<IVRGripInterface>(pActor))
				{
					GripInterface->Native_NotifyThrowGripDelegates(this, false, NewDrop, true);
				}

			}
		}
	}break;

	case EGripTargetType::ComponentGrip:

	{
		root = NewDrop.GetGrippedComponent();
		if (root)
		{
			pActor = root->GetOwner();

			root->RemoveTickPrerequisiteComponent(this);

			if ((NewDrop.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && NewDrop.AdvancedGripSettings.PhysicsSettings.bTurnOffGravityDuringGrip) ||
				(NewDrop.GripMovementReplicationSetting == EGripMovementReplicationSettings::ForceServerSideMovement && !IsServer()))
				root->SetEnableGravity(NewDrop.bOriginalGravity);

			root->SetSimulatePhysics(false);

			if (pActor)
			{
				if (IsServer() && root == pActor->GetRootComponent()) 
				{
					pActor->SetReplicateMovement(NewDrop.bOriginalReplicatesMovement);
				}

				if (pActor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
				{
					IVRGripInterface::Execute_OnChildGripRelease(pActor, this, NewDrop, true);
				}
			}

			if (root->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				IVRGripInterface::Execute_SetHeld(root, this, NewDrop.GripID, false);

				if (NewDrop.SecondaryGripInfo.bHasSecondaryAttachment || SecondaryGripIDs.Contains(NewDrop.GripID))
				{
					IVRGripInterface::Execute_OnSecondaryGripRelease(root, this, NewDrop.SecondaryGripInfo.SecondaryAttachment, NewDrop);
					OnSecondaryGripRemoved.Broadcast(NewDrop);
				}

				SecondaryGripIDs.Remove(NewDrop.GripID);

				TArray<UVRGripScriptBase*> GripScripts;
				if (IVRGripInterface::Execute_GetGripScripts(root, GripScripts))
				{
					for (UVRGripScriptBase* Script : GripScripts)
					{
						if (Script)
						{
							if (NewDrop.SecondaryGripInfo.bHasSecondaryAttachment)
								Script->OnSecondaryGripRelease(this, NewDrop.SecondaryGripInfo.SecondaryAttachment, NewDrop);

							Script->OnGripRelease(this, NewDrop, true);
						}
					}
				}

				IVRGripInterface::Execute_OnGripRelease(root, this, NewDrop, true);
				if (IVRGripInterface* GripInterface = Cast<IVRGripInterface>(root))
				{
					GripInterface->Native_NotifyThrowGripDelegates(this, false, NewDrop, true);
				}
			}

			if (root->GetAttachParent() && root->GetAttachParent()->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				IVRGripInterface::Execute_OnChildGripRelease(root->GetAttachParent(), this, NewDrop, true);
			}
		}
	}break;
	}

	FBPActorGripInformation DropBroadcastData = NewDrop;

	int fIndex = 0;
	if (LocallyGrippedObjects.Find(NewDrop, fIndex))
	{
		DIRTY_LOCALLY_GRIPPED_OBJECTS();

		if (HasGripAuthority(NewDrop) || IsServer())
		{
			LocallyGrippedObjects.RemoveAt(fIndex);
		}
		else
		{
			LocallyGrippedObjects[fIndex].bIsPendingKill = true;
			LocallyGrippedObjects[fIndex].bIsPaused = true; 
		}
	}
	else
	{
		fIndex = 0;
		if (GrippedObjects.Find(NewDrop, fIndex))
		{
			DIRTY_GRIPPED_OBJECTS();
			if (HasGripAuthority(NewDrop) || IsServer())
			{
				GrippedObjects.RemoveAt(fIndex);
			}
			else
			{
				GrippedObjects[fIndex].bIsPendingKill = true;
				GrippedObjects[fIndex].bIsPaused = true; 
			}
		}
	}

	OnDroppedObject.Broadcast(DropBroadcastData, true);
}

bool UGripMotionControllerComponent::NotifyGrip(FBPActorGripInformation &NewGrip, bool bIsReInit)
{
	UPrimitiveComponent *root = NULL;
	AActor *pActor = NULL;

	bool bRootHasInterface = false;
	bool bActorHasInterface = false;

	if (!NewGrip.GrippedObject || !NewGrip.GrippedObject->IsValidLowLevelFast())
		return false;

	if (!NewGrip.AdvancedGripSettings.bDisallowLerping && !bIsReInit && NewGrip.GripCollisionType != EGripCollisionType::EventsOnly && NewGrip.GripCollisionType != EGripCollisionType::CustomGrip)
	{

		InitializeLerpToHand(NewGrip);
	}

	switch (NewGrip.GripTargetType)
	{
	case EGripTargetType::ActorGrip:

	{
		pActor = NewGrip.GetGrippedActor();

		if (pActor)
		{
			root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

			if (root->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				bRootHasInterface = true;
			}
			if (pActor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{

				bActorHasInterface = true;
			}

			if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
			{
				if (NewGrip.GripCollisionType != EGripCollisionType::EventsOnly)
				{
					OwningPawn->MoveIgnoreActorAdd(pActor);
				}

				if (NewGrip.AdvancedGripSettings.bSetOwnerOnGrip)
				{
					if (IsServer())
					{
						pActor->SetOwner(OwningPawn);
					}
				}
			}

			if (!bIsReInit && bActorHasInterface)
			{
				IVRGripInterface::Execute_SetHeld(pActor, this, NewGrip.GripID, true);

				TArray<UVRGripScriptBase*> GripScripts;
				if (IVRGripInterface::Execute_GetGripScripts(pActor, GripScripts))
				{
					for (UVRGripScriptBase* Script : GripScripts)
					{
						if (Script)
						{
							Script->OnGrip(this, NewGrip);
						}
					}
				}

				uint8 GripID = NewGrip.GripID;
				IVRGripInterface::Execute_OnGrip(pActor, this, NewGrip);
				if (!LocallyGrippedObjects.Contains(GripID) && !GrippedObjects.Contains(GripID))
				{
					return false;
				}

				if (IVRGripInterface* GripInterface = Cast<IVRGripInterface>(pActor))
				{
					GripInterface->Native_NotifyThrowGripDelegates(this, true, NewGrip, false);

					if (!LocallyGrippedObjects.Contains(GripID) && !GrippedObjects.Contains(GripID))
					{
						return false;
					}
				}

			}

			if (root)
			{
				if (NewGrip.GripCollisionType != EGripCollisionType::EventsOnly)
				{

					if ((NewGrip.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && NewGrip.AdvancedGripSettings.PhysicsSettings.bTurnOffGravityDuringGrip) ||
						(NewGrip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ForceServerSideMovement && !IsServer()))
						root->SetEnableGravity(false);
				}

			}

		}
		else
			return false;

		if (bActorHasInterface && !EndPhysicsTickFunction.IsTickFunctionRegistered())
		{
			if (bProjectNonSimulatingGrips)
			{
				RegisterEndPhysicsTick(true);
			}
			else
			{
				EGripInterfaceTeleportBehavior TeleportBehavior = IVRGripInterface::Execute_TeleportBehavior(pActor);

				if (TeleportBehavior == EGripInterfaceTeleportBehavior::DeltaTeleportation)
				{
					RegisterEndPhysicsTick(true);
				}
			}
		}

	}break;

	case EGripTargetType::ComponentGrip:

	{
		root = NewGrip.GetGrippedComponent();

		if (root)
		{
			pActor = root->GetOwner();

			if (root->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				bRootHasInterface = true;
			}
			if (pActor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{

				bActorHasInterface = true;
			}

			if (!bIsReInit && bRootHasInterface)
			{
				IVRGripInterface::Execute_SetHeld(root, this, NewGrip.GripID, true);

				TArray<UVRGripScriptBase*> GripScripts;
				if (IVRGripInterface::Execute_GetGripScripts(root, GripScripts))
				{
					for (UVRGripScriptBase* Script : GripScripts)
					{
						if (Script)
						{
							Script->OnGrip(this, NewGrip);
						}
					}
				}

				uint8 GripID = NewGrip.GripID;
				IVRGripInterface::Execute_OnGrip(root, this, NewGrip);
				if (!LocallyGrippedObjects.Contains(GripID) && !GrippedObjects.Contains(GripID))
				{
					return false;
				}

				if (IVRGripInterface* GripInterface = Cast<IVRGripInterface>(root))
				{

					GripInterface->Native_NotifyThrowGripDelegates(this, true, NewGrip, false);

					if (!LocallyGrippedObjects.Contains(GripID) && !GrippedObjects.Contains(GripID))
					{
						return false;
					}

				}

			}

			if (pActor)
			{

				if (!bIsReInit && bActorHasInterface)
				{
					uint8 GripID = NewGrip.GripID;
					IVRGripInterface::Execute_OnChildGrip(pActor, this, NewGrip);
					if (!LocallyGrippedObjects.Contains(GripID) && !GrippedObjects.Contains(GripID))
					{
						return false;
					}
				}

			}

			if (!bIsReInit && root->GetAttachParent() && root->GetAttachParent()->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				uint8 GripID = NewGrip.GripID;
				IVRGripInterface::Execute_OnChildGrip(root->GetAttachParent(), this, NewGrip);
				if (!LocallyGrippedObjects.Contains(GripID) && !GrippedObjects.Contains(GripID))
				{
					return false;
				}
			}

			if (NewGrip.GripCollisionType != EGripCollisionType::EventsOnly)
			{
				if ((NewGrip.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && NewGrip.AdvancedGripSettings.PhysicsSettings.bTurnOffGravityDuringGrip) ||
					(NewGrip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ForceServerSideMovement && !IsServer()))
					root->SetEnableGravity(false);
			}

		}
		else
			return false;

		if (bRootHasInterface && !EndPhysicsTickFunction.IsTickFunctionRegistered())
		{
			if (bProjectNonSimulatingGrips)
			{
				RegisterEndPhysicsTick(true);
			}
			else
			{
				EGripInterfaceTeleportBehavior TeleportBehavior = IVRGripInterface::Execute_TeleportBehavior(root);

				if (TeleportBehavior == EGripInterfaceTeleportBehavior::DeltaTeleportation)
				{
					RegisterEndPhysicsTick(true);
				}
			}
		}

	}break;
	}

	switch (NewGrip.GripMovementReplicationSetting)
	{
	case EGripMovementReplicationSettings::ForceClientSideMovement:
	case EGripMovementReplicationSettings::ClientSide_Authoritive:
	case EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep:
	{
		if (NewGrip.GripCollisionType != EGripCollisionType::EventsOnly)
		{
			if (IsServer() && pActor && ((NewGrip.GripTargetType == EGripTargetType::ActorGrip) || (root && root == pActor->GetRootComponent())))
			{
				pActor->SetReplicateMovement(false);
			}
			if (root)
			{

				if (UWorld* World = GetWorld())
				{
					if (FPhysScene* PhysScene = World->GetPhysicsScene())
					{
						if (IPhysicsReplication* PhysicsReplication = PhysScene->GetPhysicsReplication())
						{
							FBodyInstance* BI = root->GetBodyInstance(NewGrip.GrippedBoneName);
							if (BI && BI->IsInstanceSimulatingPhysics())
							{
								PhysicsReplication->RemoveReplicatedTarget(root);

							}
						}
					}
				}
			}
		}

	}break; 

	case EGripMovementReplicationSettings::ForceServerSideMovement:
	{
		if (NewGrip.GripCollisionType != EGripCollisionType::EventsOnly)
		{
			if (IsServer() && pActor && ((NewGrip.GripTargetType == EGripTargetType::ActorGrip) || (root && root == pActor->GetRootComponent())))
			{
				pActor->SetReplicateMovement(true);
			}
		}
	}break;

	case EGripMovementReplicationSettings::KeepOriginalMovement:
	default:
	{}break;
	}

	bool bHasMovementAuthority = HasGripMovementAuthority(NewGrip);

	switch (NewGrip.GripCollisionType)
	{
	case EGripCollisionType::InteractiveCollisionWithPhysics:
	case EGripCollisionType::LockedConstraint:
	case EGripCollisionType::ManipulationGrip:
	case EGripCollisionType::ManipulationGripWithWristTwist:
	{
		if (bHasMovementAuthority)
		{
			SetUpPhysicsHandle(NewGrip);
		}
	} break;

	case EGripCollisionType::InteractiveHybridCollisionWithPhysics:
	{
		if (bHasMovementAuthority)
		{
			SetUpPhysicsHandle(NewGrip);
		}
	} break;

	case EGripCollisionType::EventsOnly:
	case EGripCollisionType::CustomGrip:
	{		

	} break;

	case EGripCollisionType::AttachmentGrip:
	{
		if (IsValid(root))
			root->SetSimulatePhysics(false);

		if (bHasMovementAuthority)
		{
			if (!NewGrip.bIsLerping)
			{
				TeleportMoveGrip(NewGrip);
			}
		}

		if (bHasMovementAuthority || IsServer())
		{
			FName BoneName = IsValid(CustomPivotComponent) ? CustomPivotComponentSocketName : NAME_None;
			root->AttachToComponent(IsValid(CustomPivotComponent) ? CustomPivotComponent.Get() : this, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), BoneName);
		}

	}break;

	case EGripCollisionType::PhysicsOnly:
	case EGripCollisionType::SweepWithPhysics:
	case EGripCollisionType::InteractiveHybridCollisionWithSweep:
	case EGripCollisionType::InteractiveCollisionWithSweep:
	default: 
	{

		if (IsValid(root))
		{
			if (root->IsSimulatingPhysics())
			{
				root->SetSimulatePhysics(false);
			}

			else
			{
				root->SetSimulatePhysics(true); 
				root->SetSimulatePhysics(false); 
			}

			if(root->GetAttachParent())
			{
				root->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			}
		}

		if (bHasMovementAuthority)
		{
			if (!NewGrip.bIsLerping)
			{
				TeleportMoveGrip(NewGrip);
			}
		}
	} break;

	}

	if (!bIsReInit)
	{

		if (IsValid(root))
		{

			NewGrip.LastVelWorldTrans = root->GetComponentTransform();
		}

		OnGrippedObject.Broadcast(NewGrip);
		if (!LocallyGrippedObjects.Contains(NewGrip.GripID) && !GrippedObjects.Contains(NewGrip.GripID))
		{
			return false;
		}
	}

	return true;
}

void UGripMotionControllerComponent::InitializeLerpToHand(FBPActorGripInformation & GripInformation)
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

	if (!VRSettings.bUseGlobalLerpToHand || VRSettings.LerpDuration <= 0.f)
		return;

	if (VRSettings.bSkipLerpToHandIfHeld && GripInformation.GrippedObject && GripInformation.GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		bool bIsHeld = false;
		TArray<FBPGripPair> HoldingControllers;

		IVRGripInterface::Execute_IsHeld(GripInformation.GrippedObject, HoldingControllers, bIsHeld);

		if (HoldingControllers.Num() > 0)
		{
			for (FBPGripPair& ControllerPair : HoldingControllers)
			{
				if (ControllerPair.HoldingController && ControllerPair.HoldingController != this)
				{
					FBPActorGripInformation Grip;
					EBPVRResultSwitch Result;
					ControllerPair.HoldingController->GetGripByID(Grip, ControllerPair.GripID, Result);

					if (Result != EBPVRResultSwitch::OnFailed)
					{
						if (Grip.IsValid())
						{

							GripInformation.bIsLerping = false;
							return;
						}
					}
				}
			}
		}
	}

	EBPVRResultSwitch Result;
	TSubclassOf<class UVRGripScriptBase> Class = UGS_LerpToHand::StaticClass();
	UVRGripScriptBase::GetGripScriptByClass(GripInformation.GrippedObject, Class, Result);
	if (Result == EBPVRResultSwitch::OnSucceeded)
	{
		return;
	}

	if (USceneComponent* PrimParent = Cast<USceneComponent>(GripInformation.GrippedObject))
	{
		if (GripInformation.GrippedBoneName != NAME_None)
		{
			GripInformation.OnGripTransform = PrimParent->GetSocketTransform(GripInformation.GrippedBoneName);
		}
		else
		{
			GripInformation.OnGripTransform = PrimParent->GetComponentTransform();
		}
	}
	else if (AActor* ParentActor = Cast<AActor>(GripInformation.GrippedObject))
	{
		GripInformation.OnGripTransform = ParentActor->GetActorTransform();
	}

	FTransform TargetTransform = GripInformation.RelativeTransform * this->GetPivotTransform();
	float Distance = FVector::Dist(GripInformation.OnGripTransform.GetLocation(), TargetTransform.GetLocation());
	if (VRSettings.MinDistanceForLerp > 0.0f && Distance < VRSettings.MinDistanceForLerp)
	{

		GripInformation.bIsLerping = false;

		return;
	}
	else
	{
		float LerpScaler = 1.0f;
		float DistanceToSpeed = Distance / VRSettings.LerpDuration;
		if (DistanceToSpeed < VRSettings.MinSpeedForLerp)
		{
			LerpScaler = VRSettings.MinSpeedForLerp / DistanceToSpeed;
		}
		else if (VRSettings.MaxSpeedForLerp > 0.f && DistanceToSpeed > VRSettings.MaxSpeedForLerp)
		{
			LerpScaler = VRSettings.MaxSpeedForLerp / DistanceToSpeed;
		}
		else
		{
			LerpScaler = 1.0f;
		}

		GripInformation.LerpSpeed = ((1.f / VRSettings.LerpDuration) * LerpScaler);
		GripInformation.bIsLerping = true;
		GripInformation.CurrentLerpTime = 0.0f;
	}

	GripInformation.CurrentLerpTime = 0.0f;
}

void UGripMotionControllerComponent::HandleGlobalLerpToHand(FBPActorGripInformation& GripInformation, FTransform& WorldTransform, float DeltaTime)
{
	UVRGlobalSettings* VRSettings = GetMutableDefault<UVRGlobalSettings>();

	if (!VRSettings->bUseGlobalLerpToHand || !GripInformation.bIsLerping)
		return;

	EBPVRResultSwitch Result;
	TSubclassOf<class UVRGripScriptBase> Class = UGS_LerpToHand::StaticClass();
	UVRGripScriptBase * LerpScript = UVRGripScriptBase::GetGripScriptByClass(GripInformation.GrippedObject, Class, Result);
	if (Result == EBPVRResultSwitch::OnSucceeded && LerpScript && LerpScript->IsScriptActive())
	{
		return;
	}

	if (VRSettings->LerpDuration <= 0.f)
	{
		GripInformation.bIsLerping = false;
		GripInformation.CurrentLerpTime = 0.f;
		OnLerpToHandFinished.Broadcast(GripInformation);
		return;
	}

	FTransform NA = GripInformation.OnGripTransform;
	float Alpha = 0.0f;

	GripInformation.CurrentLerpTime += DeltaTime * GripInformation.LerpSpeed;
	float OrigAlpha = FMath::Clamp(GripInformation.CurrentLerpTime, 0.f, 1.0f);
	Alpha = OrigAlpha;

	if (VRSettings->bUseCurve)
	{
		if (FRichCurve* richCurve = VRSettings->OptionalCurveToFollow.GetRichCurve())
		{

			{
				Alpha = FMath::Clamp(richCurve->Eval(Alpha), 0.f, 1.f);

			}
		}
	}

	FTransform NB = WorldTransform;
	NA.NormalizeRotation();
	NB.NormalizeRotation();

	if (VRSettings->LerpInterpolationMode == EVRLerpInterpolationMode::QuatInterp)
	{
		WorldTransform.Blend(NA, NB, Alpha);
	}

	else if (VRSettings->LerpInterpolationMode == EVRLerpInterpolationMode::EulerInterp)
	{
		WorldTransform.SetTranslation(FMath::Lerp(NA.GetTranslation(), NB.GetTranslation(), Alpha));
		WorldTransform.SetScale3D(FMath::Lerp(NA.GetScale3D(), NB.GetScale3D(), Alpha));

		FRotator A = NA.Rotator();
		FRotator B = NB.Rotator();
		WorldTransform.SetRotation(FQuat(A + (Alpha * (B - A))));
	}

	else
	{
		if ((NB.GetRotation() | NA.GetRotation()) < 0.0f)
		{
			NB.SetRotation(NB.GetRotation() * -1.0f);
		}
		WorldTransform = (FDualQuat(NA) * (1 - Alpha) + FDualQuat(NB) * Alpha).Normalized().AsFTransform(FMath::Lerp(NA.GetScale3D(), NB.GetScale3D(), Alpha));
	}

	if (OrigAlpha >= 1.0f)
	{
		GripInformation.CurrentLerpTime = 0.0f;
		GripInformation.bIsLerping = false;

		if (bConstrainToPivot)
		{
			DestroyPhysicsHandle(GripInformation, false);
			SetUpPhysicsHandle(GripInformation);
		}

		OnLerpToHandFinished.Broadcast(GripInformation);
	}
}

void UGripMotionControllerComponent::CancelGlobalLerpToHand(uint8 GripID)
{
	FBPActorGripInformation* GripToUse = nullptr;
	if (GripID != INVALID_VRGRIP_ID)
	{
		GripToUse = GrippedObjects.FindByKey(GripID);
		if (!GripToUse)
		{
			GripToUse = LocallyGrippedObjects.FindByKey(GripID);
		}

		if (GripToUse)
		{
			GripToUse->bIsLerping = false;

			if (bConstrainToPivot)
			{
				DestroyPhysicsHandle(*GripToUse, false);
				SetUpPhysicsHandle(*GripToUse);
			}

			GripToUse->CurrentLerpTime = 0.0f;
			OnLerpToHandFinished.Broadcast(*GripToUse);
		}
	}
}

void UGripMotionControllerComponent::NotifyDrop_Implementation(const FBPActorGripInformation &NewDrop, bool bSimulate)
{

	if ((NewDrop.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive || 
		NewDrop.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep) && 
		IsLocallyControlled() && 
		!IsServer())
	{

		if (FBPActorGripInformation * GripInfo = GetGripPtrByID(NewDrop.GripID))
		{
			DropGrip_Implementation(*GripInfo, bSimulate, FVector::ZeroVector, FVector::ZeroVector, true);
		}

		return;
	}

	Drop_Implementation(NewDrop, bSimulate);
}

void UGripMotionControllerComponent::Drop_Implementation(const FBPActorGripInformation &NewDrop, bool bSimulate)
{
	bool bSkipFullDrop = false;
	bool bHadAnotherSelfGrip = false;
	TArray<FBPGripPair> HoldingControllers;
	bool bIsHeld = false;

	if(NewDrop.GrippedObject && NewDrop.GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		IVRGripInterface::Execute_IsHeld(NewDrop.GrippedObject, HoldingControllers, bIsHeld);

	if (bIsHeld && (!HoldingControllers.Contains(this) || HoldingControllers.Num() > 1))
	{

		bSkipFullDrop = true;
	}	
	else 
	{
		for (int i = 0; i < LocallyGrippedObjects.Num(); ++i)
		{
			if (LocallyGrippedObjects[i].GrippedObject == NewDrop.GrippedObject && LocallyGrippedObjects[i].GripID != NewDrop.GripID)
			{
				bSkipFullDrop = true;
				bHadAnotherSelfGrip = true;
			}
		}
		for (int i = 0; i < GrippedObjects.Num(); ++i)
		{
			if (GrippedObjects[i].GrippedObject == NewDrop.GrippedObject && GrippedObjects[i].GripID != NewDrop.GripID)
			{
				bSkipFullDrop = true;
				bHadAnotherSelfGrip = true;
			}
		}
	}

	DestroyPhysicsHandle(NewDrop, bHadAnotherSelfGrip);

	bool bHadGripAuthority = HasGripAuthority(NewDrop);

	UPrimitiveComponent *root = NULL;
	AActor * pActor = NULL;

	switch (NewDrop.GripTargetType)
	{
	case EGripTargetType::ActorGrip:

	{
		pActor = NewDrop.GetGrippedActor();

		if (pActor)
		{
			root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

			if (!bSkipFullDrop)
			{

				pActor->RemoveTickPrerequisiteComponent(this);

				if (NewDrop.GripCollisionType != EGripCollisionType::EventsOnly)
				{
					if (APawn * OwningPawn = Cast<APawn>(GetOwner()))
					{
						OwningPawn->MoveIgnoreActorRemove(pActor);

					}
				}

				if (root)
				{

					if (NewDrop.GripCollisionType == EGripCollisionType::AttachmentGrip && (HasGripAuthority(NewDrop) || IsServer()))
						root->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

					if (NewDrop.GripCollisionType != EGripCollisionType::EventsOnly)
					{
						if (IsServer() || bHadGripAuthority || !NewDrop.bOriginalReplicatesMovement || !pActor->GetIsReplicated())
						{
							if (!NewDrop.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings || !NewDrop.AdvancedGripSettings.PhysicsSettings.bSkipSettingSimulating)
							{
								if (root->IsSimulatingPhysics() != bSimulate)
								{
									root->SetSimulatePhysics(bSimulate);
								}

								if (bSimulate)
									root->WakeAllRigidBodies();
							}
						}

						root->UpdateComponentToWorld(); 
					}

					if (NewDrop.GripCollisionType != EGripCollisionType::EventsOnly)
					{
						if ((NewDrop.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && NewDrop.AdvancedGripSettings.PhysicsSettings.bTurnOffGravityDuringGrip) ||
							(NewDrop.GripMovementReplicationSetting == EGripMovementReplicationSettings::ForceServerSideMovement && !IsServer()))
							root->SetEnableGravity(NewDrop.bOriginalGravity);
					}
				}
			}

			if (IsServer() && !bSkipFullDrop)
			{
				pActor->SetReplicateMovement(NewDrop.bOriginalReplicatesMovement);
			}

			if (pActor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				IVRGripInterface::Execute_SetHeld(pActor, this, NewDrop.GripID, false);

				if (NewDrop.SecondaryGripInfo.bHasSecondaryAttachment || SecondaryGripIDs.Contains(NewDrop.GripID))
				{
					IVRGripInterface::Execute_OnSecondaryGripRelease(pActor, this, NewDrop.SecondaryGripInfo.SecondaryAttachment, NewDrop);
					OnSecondaryGripRemoved.Broadcast(NewDrop);
				}

				SecondaryGripIDs.Remove(NewDrop.GripID);

				TArray<UVRGripScriptBase*> GripScripts;
				if (IVRGripInterface::Execute_GetGripScripts(pActor, GripScripts))
				{
					for (UVRGripScriptBase* Script : GripScripts)
					{
						if (Script)
						{
							if (NewDrop.SecondaryGripInfo.bHasSecondaryAttachment)
								Script->OnSecondaryGripRelease(this, NewDrop.SecondaryGripInfo.SecondaryAttachment, NewDrop);

							Script->OnGripRelease(this, NewDrop, false);
						}
					}
				}

				IVRGripInterface::Execute_OnGripRelease(pActor, this, NewDrop, false);
				if (IVRGripInterface* GripInterface = Cast<IVRGripInterface>(pActor))
				{

					GripInterface->Native_NotifyThrowGripDelegates(this, false, NewDrop, false);
				}
			}
		}
	}break;

	case EGripTargetType::ComponentGrip:

	{
		root = NewDrop.GetGrippedComponent();
		if (root)
		{
			pActor = root->GetOwner();

			if (!bSkipFullDrop)
			{
				root->RemoveTickPrerequisiteComponent(this);

				if (NewDrop.GripCollisionType == EGripCollisionType::AttachmentGrip && (HasGripAuthority(NewDrop) || IsServer()))
					root->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				if (NewDrop.GripCollisionType != EGripCollisionType::EventsOnly)
				{

					if (IsServer() || bHadGripAuthority || !NewDrop.bOriginalReplicatesMovement || (pActor && (pActor->GetRootComponent() != root || !pActor->GetIsReplicated())))
					{
						if (!NewDrop.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings || !NewDrop.AdvancedGripSettings.PhysicsSettings.bSkipSettingSimulating)
						{
							if (root->IsSimulatingPhysics() != bSimulate)
							{
								root->SetSimulatePhysics(bSimulate);
							}

							if (bSimulate)
								root->WakeAllRigidBodies();
						}
					}

					root->UpdateComponentToWorld(); 
				}

				if (NewDrop.GripCollisionType != EGripCollisionType::EventsOnly)
				{
					if ((NewDrop.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && NewDrop.AdvancedGripSettings.PhysicsSettings.bTurnOffGravityDuringGrip) ||
						(NewDrop.GripMovementReplicationSetting == EGripMovementReplicationSettings::ForceServerSideMovement && !IsServer()))
						root->SetEnableGravity(NewDrop.bOriginalGravity);
				}
			}

			if (pActor)
			{
				if (IsServer() && root == pActor->GetRootComponent() && !bSkipFullDrop)
				{
					pActor->SetReplicateMovement(NewDrop.bOriginalReplicatesMovement);
				}

				if (pActor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
				{
					IVRGripInterface::Execute_OnChildGripRelease(pActor, this, NewDrop, false);
				}

			}

			if (root->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				IVRGripInterface::Execute_SetHeld(root, this, NewDrop.GripID, false);

				if (NewDrop.SecondaryGripInfo.bHasSecondaryAttachment || SecondaryGripIDs.Contains(NewDrop.GripID))
				{
					IVRGripInterface::Execute_OnSecondaryGripRelease(root, this, NewDrop.SecondaryGripInfo.SecondaryAttachment, NewDrop);
					OnSecondaryGripRemoved.Broadcast(NewDrop);
				}

				SecondaryGripIDs.Remove(NewDrop.GripID);

				TArray<UVRGripScriptBase*> GripScripts;
				if (IVRGripInterface::Execute_GetGripScripts(root, GripScripts))
				{
					for (UVRGripScriptBase* Script : GripScripts)
					{
						if (Script)
						{
							if (NewDrop.SecondaryGripInfo.bHasSecondaryAttachment)
								Script->OnSecondaryGripRelease(this, NewDrop.SecondaryGripInfo.SecondaryAttachment, NewDrop);

							Script->OnGripRelease(this, NewDrop, false);
						}
					}
				}

				IVRGripInterface::Execute_OnGripRelease(root, this, NewDrop, false);
				if (IVRGripInterface* GripInterface = Cast<IVRGripInterface>(root))
				{
					GripInterface->Native_NotifyThrowGripDelegates(this, false, NewDrop, false);
				}

			}

			if (root->GetAttachParent() && root->GetAttachParent()->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				IVRGripInterface::Execute_OnChildGripRelease(root->GetAttachParent(), this, NewDrop, false);
			}
		}
	}break;
	}

	switch (NewDrop.GripMovementReplicationSetting)
	{
	case EGripMovementReplicationSettings::ForceClientSideMovement:
	case EGripMovementReplicationSettings::ClientSide_Authoritive:
	case EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep:
	{
		if (NewDrop.GripCollisionType != EGripCollisionType::EventsOnly)
		{
			if (root)
			{

				if (UWorld * World = GetWorld())
				{
					if (FPhysScene * PhysScene = World->GetPhysicsScene())
					{
						if (IPhysicsReplication* PhysicsReplication = PhysScene->GetPhysicsReplication())
						{
							FBodyInstance* BI = root->GetBodyInstance(NewDrop.GrippedBoneName);
							if (BI && BI->IsInstanceSimulatingPhysics())
							{
								PhysicsReplication->RemoveReplicatedTarget(root);

							}
						}
					}
				}
			}
		}

	}break;

	};

	FBPActorGripInformation DropBroadcastData = NewDrop;

	int fIndex = 0;
	if (LocallyGrippedObjects.Find(NewDrop, fIndex))
	{
		DIRTY_LOCALLY_GRIPPED_OBJECTS();

		if (HasGripAuthority(NewDrop) || IsServer())
		{
			LocallyGrippedObjects.RemoveAt(fIndex);
		}
		else
		{
			LocallyGrippedObjects[fIndex].bIsPendingKill = true;
			LocallyGrippedObjects[fIndex].bIsPaused = true; 
		}
	}
	else
	{
		fIndex = 0;
		if (GrippedObjects.Find(NewDrop, fIndex))
		{
			DIRTY_GRIPPED_OBJECTS();
			if (HasGripAuthority(NewDrop) || IsServer())
			{
				GrippedObjects.RemoveAt(fIndex);
			}
			else
			{
				GrippedObjects[fIndex].bIsPendingKill = true;
				GrippedObjects[fIndex].bIsPaused = true; 
			}
		}
	}

	OnDroppedObject.Broadcast(DropBroadcastData, false);

	if (EndPhysicsTickFunction.IsTickFunctionRegistered())
	{
		bool bNeedsPhysicsTick = false;

		if (LocallyGrippedObjects.Num() > 0 || GrippedObjects.Num() > 0)
		{
			if (bProjectNonSimulatingGrips)
			{
				bNeedsPhysicsTick = true;
			}
			else
			{

				for (int i = 0; i < LocallyGrippedObjects.Num(); ++i)
				{
					if (IsValid(LocallyGrippedObjects[i].GrippedObject) && LocallyGrippedObjects[i].GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
					{
						if (LocallyGrippedObjects[i].GripCollisionType != EGripCollisionType::CustomGrip && LocallyGrippedObjects[i].GripCollisionType != EGripCollisionType::EventsOnly)
						{
							EGripInterfaceTeleportBehavior TeleportBehavior = IVRGripInterface::Execute_TeleportBehavior(LocallyGrippedObjects[i].GrippedObject);
							if (TeleportBehavior == EGripInterfaceTeleportBehavior::DeltaTeleportation)
							{
								bNeedsPhysicsTick = true;
								break;
							}
						}
					}
				}

				if (!bNeedsPhysicsTick)
				{
					for (int i = 0; i < GrippedObjects.Num(); ++i)
					{
						if (IsValid(GrippedObjects[i].GrippedObject) && GrippedObjects[i].GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
						{
							if (GrippedObjects[i].GripCollisionType != EGripCollisionType::CustomGrip && GrippedObjects[i].GripCollisionType != EGripCollisionType::EventsOnly)
							{
								EGripInterfaceTeleportBehavior TeleportBehavior = IVRGripInterface::Execute_TeleportBehavior(GrippedObjects[i].GrippedObject);
								if (TeleportBehavior == EGripInterfaceTeleportBehavior::DeltaTeleportation)
								{
									bNeedsPhysicsTick = true;
									break;
								}
							}
						}
					}
				}
			}
		}

		if (!bNeedsPhysicsTick)
		{
			RegisterEndPhysicsTick(false);
		}
	}
}

bool UGripMotionControllerComponent::BP_IsLocallyControlled()
{
	return IsLocallyControlled();
}

bool UGripMotionControllerComponent::BP_HasGripAuthority(const FBPActorGripInformation &Grip)
{
	return HasGripAuthority(Grip);
}

bool UGripMotionControllerComponent::BP_HasGripAuthorityForObject(const UObject * ObjToCheck)
{
	return HasGripAuthority(ObjToCheck);
}

bool UGripMotionControllerComponent::BP_HasGripMovementAuthority(const FBPActorGripInformation &Grip)
{
	return HasGripMovementAuthority(Grip);
}

bool UGripMotionControllerComponent::AddSecondaryAttachmentPoint(UObject * GrippedObjectToAddAttachment, USceneComponent * SecondaryPointComponent, const FTransform & OriginalTransform, bool bTransformIsAlreadyRelative, float LerpToTime, bool bIsSlotGrip, FName SecondarySlotName)
{
	if (!GrippedObjectToAddAttachment || !SecondaryPointComponent || (!GrippedObjects.Num() && !LocallyGrippedObjects.Num()))
		return false;

	FBPActorGripInformation * GripToUse = nullptr;

	GripToUse = LocallyGrippedObjects.FindByKey(GrippedObjectToAddAttachment);

	if (!GripToUse)
	{

		if (!IsServer())
		{
			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController add secondary attachment function was called on the client side with a replicated grip");
			return false;
		}

		GripToUse = GrippedObjects.FindByKey(GrippedObjectToAddAttachment);
	}

	if (GripToUse)
	{
		return AddSecondaryAttachmentToGrip(*GripToUse, SecondaryPointComponent, OriginalTransform, bTransformIsAlreadyRelative, LerpToTime, bIsSlotGrip, SecondarySlotName);
	}

	return false;
}

bool UGripMotionControllerComponent::AddSecondaryAttachmentToGripByID(const uint8 GripID, USceneComponent* SecondaryPointComponent, const FTransform& OriginalTransform, bool bTransformIsAlreadyRelative, float LerpToTime, bool bIsSlotGrip, FName SecondarySlotName)
{
	FBPActorGripInformation* GripToUse = nullptr;
	if (GripID != INVALID_VRGRIP_ID)
	{
		GripToUse = GrippedObjects.FindByKey(GripID);
		if (!GripToUse)
		{
			GripToUse = LocallyGrippedObjects.FindByKey(GripID);
		}

		if (GripToUse)
		{
			return AddSecondaryAttachmentToGrip(*GripToUse, SecondaryPointComponent, OriginalTransform, bTransformIsAlreadyRelative, LerpToTime, bIsSlotGrip, SecondarySlotName);
		}
	}

	return false;
}

bool UGripMotionControllerComponent::AddSecondaryAttachmentToGrip(const FBPActorGripInformation & GripToAddAttachment, USceneComponent * SecondaryPointComponent, const FTransform &OriginalTransform, bool bTransformIsAlreadyRelative, float LerpToTime, bool bIsSlotGrip, FName SecondarySlotName)
{
	if (!SecondaryPointComponent)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController add secondary attachment function was called with a bad secondary component target!");
		return false;
	}

	FBPActorGripInformation* GripToUse = nullptr;
	bool bWasLocal = false;
	if (GripToAddAttachment.GrippedObject && GripToAddAttachment.GripID != INVALID_VRGRIP_ID)
	{
		GripToUse = GrippedObjects.FindByKey(GripToAddAttachment.GripID);
		if (!GripToUse)
		{
			GripToUse = LocallyGrippedObjects.FindByKey(GripToAddAttachment.GripID);
			bWasLocal = true;
		}
	}

	if (!GripToUse || GripToUse->GripID == INVALID_VRGRIP_ID)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController add secondary attachment function was called with a bad grip! It was not valid / found.");
		return false;
	}

	if (!GripToUse->GrippedObject)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController add secondary attachment function was called with a bad grip (gripped object invalid)!");
		return false;
	}

	if (!bWasLocal && !IsServer())
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController add secondary attachment function was called on the client side with a replicated grip");
		return false;
	}

	bool bGrippedObjectIsInterfaced = GripToUse->GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass());

	if (bGrippedObjectIsInterfaced)
	{
		ESecondaryGripType SecondaryType = IVRGripInterface::Execute_SecondaryGripType(GripToUse->GrippedObject);

		if (SecondaryType == ESecondaryGripType::SG_None)
		{
			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController add secondary attachment function was called on an interface object set to SG_None!");
			return false;
		}
	}

	UPrimitiveComponent * root = nullptr;

	switch (GripToUse->GripTargetType)
	{
	case EGripTargetType::ActorGrip:
	{
		AActor * pActor = GripToUse->GetGrippedActor();

		if (pActor)
		{
			root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());
		}
	}
	break;
	case EGripTargetType::ComponentGrip:
	{
		root = GripToUse->GetGrippedComponent();
	}
	break;
	}

	if (!root)
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController add secondary attachment function was unable to get root component or gripped component.");
		return false;
	}

	if (bTransformIsAlreadyRelative)
		GripToUse->SecondaryGripInfo.SecondaryRelativeTransform = OriginalTransform;
	else
		GripToUse->SecondaryGripInfo.SecondaryRelativeTransform = OriginalTransform.GetRelativeTransform(root->GetComponentTransform());

	GripToUse->SecondaryGripInfo.SecondaryAttachment = SecondaryPointComponent;
	GripToUse->SecondaryGripInfo.bHasSecondaryAttachment = true;
	GripToUse->SecondaryGripInfo.SecondaryGripDistance = 0.0f;
	GripToUse->SecondaryGripInfo.SecondarySlotName = SecondarySlotName;

	GripToUse->SecondaryGripInfo.bIsSlotGrip = bIsSlotGrip;

	if (GripToUse->SecondaryGripInfo.GripLerpState == EGripLerpState::EndLerp)
		LerpToTime = 0.0f;

	if (LerpToTime > 0.0f)
	{
		GripToUse->SecondaryGripInfo.LerpToRate = LerpToTime;
		GripToUse->SecondaryGripInfo.GripLerpState = EGripLerpState::StartLerp;
		GripToUse->SecondaryGripInfo.curLerp = LerpToTime;
	}

	if (bGrippedObjectIsInterfaced)
	{
		SecondaryGripIDs.Add(GripToUse->GripID);

		IVRGripInterface::Execute_OnSecondaryGrip(GripToUse->GrippedObject, this, SecondaryPointComponent, *GripToUse);

		TArray<UVRGripScriptBase*> GripScripts;
		if (IVRGripInterface::Execute_GetGripScripts(GripToUse->GrippedObject, GripScripts))
		{
			for (UVRGripScriptBase* Script : GripScripts)
			{
				if (Script)
				{
					Script->OnSecondaryGrip(this, SecondaryPointComponent, *GripToUse);
				}
			}
		}
	}

	if (GripToUse->GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive && !IsServer() && !IsTornOff())
	{
		Server_NotifySecondaryAttachmentChanged(GripToUse->GripID, GripToUse->SecondaryGripInfo);
	}

	if (bWasLocal)
	{
		DIRTY_LOCALLY_GRIPPED_OBJECTS();
	}
	else
	{
		DIRTY_GRIPPED_OBJECTS();
	}

	OnSecondaryGripAdded.Broadcast(*GripToUse);
	GripToUse = nullptr;

	return true;
}

bool UGripMotionControllerComponent::RemoveSecondaryAttachmentPoint(UObject * GrippedObjectToRemoveAttachment, float LerpToTime)
{
	if (!GrippedObjectToRemoveAttachment || (!GrippedObjects.Num() && !LocallyGrippedObjects.Num()))
		return false;

	FBPActorGripInformation * GripToUse = nullptr;

	GripToUse = LocallyGrippedObjects.FindByKey(GrippedObjectToRemoveAttachment);

	if (!GripToUse)
	{
		if (!IsServer())
		{
			UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController remove secondary attachment function was called on the client side for a replicating grip");
			return false;
		}

		GripToUse = GrippedObjects.FindByKey(GrippedObjectToRemoveAttachment);
	}

	if (GripToUse && GripToUse->GrippedObject)
	{
		return RemoveSecondaryAttachmentFromGrip(*GripToUse, LerpToTime);
	}

	return false;
}
bool UGripMotionControllerComponent::RemoveSecondaryAttachmentFromGripByID(const uint8 GripID, float LerpToTime)
{
	FBPActorGripInformation* GripToUse = nullptr;
	if (GripID != INVALID_VRGRIP_ID)
	{
		GripToUse = GrippedObjects.FindByKey(GripID);
		if (!GripToUse)
		{
			GripToUse = LocallyGrippedObjects.FindByKey(GripID);
		}

		if (GripToUse)
		{
			return RemoveSecondaryAttachmentFromGrip(*GripToUse, LerpToTime);
		}
	}

	return false;
}

bool UGripMotionControllerComponent::RemoveSecondaryAttachmentFromGrip(const FBPActorGripInformation & GripToRemoveAttachment, float LerpToTime)
{
	FBPActorGripInformation* GripToUse = nullptr;
	bool bWasLocal = false;
	if (GripToRemoveAttachment.GrippedObject && GripToRemoveAttachment.GripID != INVALID_VRGRIP_ID)
	{
		GripToUse = GrippedObjects.FindByKey(GripToRemoveAttachment.GripID);
		if (!GripToUse)
		{
			GripToUse = LocallyGrippedObjects.FindByKey(GripToRemoveAttachment.GripID);
			bWasLocal = true;
		}
	}

	if (GripToUse && !bWasLocal && !IsServer())
	{
		UE_LOGF(LogVRMotionController, Warning, "VRGripMotionController remove secondary attachment function was called on the client side for a replicating grip");
		return false;
	}

	if (GripToUse && GripToUse->GrippedObject && GripToUse->GripID != INVALID_VRGRIP_ID)
	{
		SecondaryGripIDs.Remove(GripToUse->GripID);

		if (GripToUse->SecondaryGripInfo.GripLerpState == EGripLerpState::StartLerp)
			LerpToTime = 0.0f;

		UPrimitiveComponent * primComp = nullptr;

		switch (GripToUse->GripTargetType)
		{
		case EGripTargetType::ComponentGrip:
		{
			primComp = GripToUse->GetGrippedComponent();
		}break;
		case EGripTargetType::ActorGrip:
		{
			AActor * pActor = GripToUse->GetGrippedActor();
			if (pActor)
				primComp = Cast<UPrimitiveComponent>(pActor->GetRootComponent());
		} break;
		}

		bool bGripObjectHasInterface = GripToUse->GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass());

		ESecondaryGripType SecondaryType = ESecondaryGripType::SG_None;
		if (bGripObjectHasInterface)
		{
			SecondaryType = IVRGripInterface::Execute_SecondaryGripType(GripToUse->GrippedObject);

		}

		if (primComp)
		{
			switch (SecondaryType)
			{

			case ESecondaryGripType::SG_FreeWithScaling_Retain:
			case ESecondaryGripType::SG_SlotOnlyWithScaling_Retain:
			case ESecondaryGripType::SG_Free_Retain:
			case ESecondaryGripType::SG_SlotOnly_Retain:
			case ESecondaryGripType::SG_ScalingOnly:
			{
				GripToUse->RelativeTransform = primComp->GetComponentTransform().GetRelativeTransform(GetPivotTransform());
				GripToUse->SecondaryGripInfo.LerpToRate = 0.0f;
				GripToUse->SecondaryGripInfo.GripLerpState = EGripLerpState::NotLerping;
			}break;
			default:
			{
				if (LerpToTime > 0.0f)
				{

					GripToUse->SecondaryGripInfo.LerpToRate = LerpToTime;
					GripToUse->SecondaryGripInfo.GripLerpState = EGripLerpState::EndLerp;
					GripToUse->SecondaryGripInfo.curLerp = LerpToTime;
				}
			}break;
			}

		}
		else
		{
			GripToUse->SecondaryGripInfo.LerpToRate = 0.0f;
			GripToUse->SecondaryGripInfo.GripLerpState = EGripLerpState::NotLerping;
		}

		if (bGripObjectHasInterface)
		{
			IVRGripInterface::Execute_OnSecondaryGripRelease(GripToUse->GrippedObject, this, GripToUse->SecondaryGripInfo.SecondaryAttachment, *GripToUse);

			TArray<UVRGripScriptBase*> GripScripts;
			if (IVRGripInterface::Execute_GetGripScripts(GripToUse->GrippedObject, GripScripts))
			{
				for (UVRGripScriptBase* Script : GripScripts)
				{
					if (Script)
					{
						Script->OnSecondaryGripRelease(this, GripToUse->SecondaryGripInfo.SecondaryAttachment, *GripToUse);
					}
				}
			}
		}

		GripToUse->SecondaryGripInfo.SecondaryAttachment = nullptr;
		GripToUse->SecondaryGripInfo.bHasSecondaryAttachment = false;

		if (GripToUse->GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive && !IsServer())
		{
			switch (SecondaryType)
			{

			case ESecondaryGripType::SG_FreeWithScaling_Retain:
			case ESecondaryGripType::SG_SlotOnlyWithScaling_Retain:
			case ESecondaryGripType::SG_Free_Retain:
			case ESecondaryGripType::SG_SlotOnly_Retain:
			case ESecondaryGripType::SG_ScalingOnly:
			{
				if (!IsTornOff())
					Server_NotifySecondaryAttachmentChanged_Retain(GripToUse->GripID, GripToUse->SecondaryGripInfo, GripToUse->RelativeTransform);
			}break;
			default:
			{
				if (!IsTornOff())
					Server_NotifySecondaryAttachmentChanged(GripToUse->GripID, GripToUse->SecondaryGripInfo);
			}break;
			}

		}

		if (bWasLocal)
		{
			DIRTY_LOCALLY_GRIPPED_OBJECTS();
		}
		else
		{
			DIRTY_GRIPPED_OBJECTS();
		}

		SecondaryGripIDs.Remove(GripToUse->GripID);
		OnSecondaryGripRemoved.Broadcast(*GripToUse);
		GripToUse = nullptr;
		return true;
	}

	return false;
}

bool UGripMotionControllerComponent::TeleportMoveGrippedActor(AActor * GrippedActorToMove, bool bTeleportPhysicsGrips)
{
	if (!GrippedActorToMove || (!GrippedObjects.Num() && !LocallyGrippedObjects.Num()))
		return false;

	FBPActorGripInformation * GripInfo = LocallyGrippedObjects.FindByKey(GrippedActorToMove);
	if (!GripInfo)
		GripInfo = GrippedObjects.FindByKey(GrippedActorToMove);

	if (GripInfo)
	{
		return TeleportMoveGrip(*GripInfo, bTeleportPhysicsGrips);
	}

	return false;
}

bool UGripMotionControllerComponent::TeleportMoveGrippedComponent(UPrimitiveComponent * ComponentToMove, bool bTeleportPhysicsGrips)
{
	if (!ComponentToMove || (!GrippedObjects.Num() && !LocallyGrippedObjects.Num()))
		return false;

	FBPActorGripInformation * GripInfo = LocallyGrippedObjects.FindByKey(ComponentToMove);
	if (!GripInfo)
		GripInfo = GrippedObjects.FindByKey(ComponentToMove);

	if (GripInfo)
	{
		return TeleportMoveGrip(*GripInfo, bTeleportPhysicsGrips);
	}

	return false;
}

void UGripMotionControllerComponent::TeleportMoveGrips(bool bTeleportPhysicsGrips, bool bIsForPostTeleport)
{
	FTransform EmptyTransform = FTransform::Identity;
	for (FBPActorGripInformation& GripInfo : LocallyGrippedObjects)
	{
		TeleportMoveGrip_Impl(GripInfo, bTeleportPhysicsGrips, bIsForPostTeleport, EmptyTransform);
	}

	for (FBPActorGripInformation& GripInfo : GrippedObjects)
	{
		TeleportMoveGrip_Impl(GripInfo, bTeleportPhysicsGrips, bIsForPostTeleport, EmptyTransform);
	}
}

bool UGripMotionControllerComponent::TeleportMoveGrip(FBPActorGripInformation &Grip, bool bTeleportPhysicsGrips, bool bIsForPostTeleport)
{
	FTransform EmptyTransform = FTransform::Identity;
	return TeleportMoveGrip_Impl(Grip, bTeleportPhysicsGrips, bIsForPostTeleport, EmptyTransform);
}

bool UGripMotionControllerComponent::TeleportMoveGrip_Impl(FBPActorGripInformation &Grip, bool bTeleportPhysicsGrips, bool bIsForPostTeleport, FTransform & OptionalTransform)
{
	bool bHasMovementAuthority = HasGripMovementAuthority(Grip);

	if (!bHasMovementAuthority)
		return false;

	UPrimitiveComponent * PrimComp = NULL;
	AActor * actor = NULL;

	bool bRootHasInterface = false;
	bool bActorHasInterface = false;

	switch (Grip.GripTargetType)
	{
	case EGripTargetType::ActorGrip:

	{
		actor = Grip.GetGrippedActor();
		if (actor)
		{
			PrimComp = Cast<UPrimitiveComponent>(actor->GetRootComponent());
			if (actor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				bActorHasInterface = true;
			}
		}
	}break;

	case EGripTargetType::ComponentGrip:

	{
		PrimComp = Grip.GetGrippedComponent();

		if (PrimComp)
		{
			actor = PrimComp->GetOwner();
			if (PrimComp->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				bRootHasInterface = true;
			}
		}

	}break;

	}

	if (!PrimComp || !actor || !IsValid(actor) || !IsValid(PrimComp))
		return false;

	EGripInterfaceTeleportBehavior TeleportBehavior = EGripInterfaceTeleportBehavior::TeleportAllComponents;
	bool bSimulateOnDrop = false;

	if (bRootHasInterface)
	{
		TeleportBehavior = IVRGripInterface::Execute_TeleportBehavior(PrimComp);
		bSimulateOnDrop = IVRGripInterface::Execute_SimulateOnDrop(PrimComp);
	}
	else if (bActorHasInterface)
	{

		TeleportBehavior = IVRGripInterface::Execute_TeleportBehavior(actor);
		bSimulateOnDrop = IVRGripInterface::Execute_SimulateOnDrop(actor);
	}

	if (bIsForPostTeleport)
	{
		if (TeleportBehavior == EGripInterfaceTeleportBehavior::OnlyTeleportRootComponent)
		{
			if (AActor * owner = PrimComp->GetOwner())
			{
				if (PrimComp != owner->GetRootComponent())
				{
					return false;
				}
			}
		}
		else if (TeleportBehavior == EGripInterfaceTeleportBehavior::DropOnTeleport)
		{
			if (IsServer() ||
				Grip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive ||
				Grip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep)
			{
				DropObjectByInterface(nullptr, Grip.GripID);
			}

			return false; 
		}
		else if (TeleportBehavior == EGripInterfaceTeleportBehavior::DontTeleport)
		{
			return false; 
		}
	}
	else
	{
		switch (TeleportBehavior)
		{
		case EGripInterfaceTeleportBehavior::DontTeleport:
		case EGripInterfaceTeleportBehavior::DropOnTeleport:
		{
			return false;
		}break;
		default:break;
		}
	}

	if (Grip.GripCollisionType == EGripCollisionType::AttachmentGrip || Grip.GripCollisionType == EGripCollisionType::EventsOnly)
	{
		return false;
	}

	FTransform WorldTransform;
	FTransform ParentTransform = GetPivotTransform();

	FBPActorGripInformation copyGrip = Grip;

	if (!OptionalTransform.Equals(FTransform::Identity))
	{
		WorldTransform = OptionalTransform;
	}
	else
	{
		TArray<UVRGripScriptBase*> Scripts;

		if (bRootHasInterface)
		{
			IVRGripInterface::Execute_GetGripScripts(PrimComp, Scripts);
		}
		else if (bActorHasInterface)
		{
			IVRGripInterface::Execute_GetGripScripts(actor, Scripts);
		}

		bool bForceADrop = false;
		bool bHadValidWorldTransform = GetGripWorldTransform(Scripts, 0.0f, WorldTransform, ParentTransform, copyGrip, actor, PrimComp, bRootHasInterface, bActorHasInterface, true, bForceADrop);

		if (!bHadValidWorldTransform)
			return false;
	}

	if (!WorldTransform.IsValid())
	{
		UE_LOGF(LogVRMotionController, Warning, "Something went wrong, TeleportGrip_Impl's target transform contained NAN.");
		return false;
	}

	FTransform physicsTrans = WorldTransform;
	if (TeleportBehavior == EGripInterfaceTeleportBehavior::DeltaTeleportation && !Grip.LastWorldTransform.Equals(FTransform::Identity))
	{
		FTransform baseTrans = this->GetAttachParent()->GetComponentTransform();
		WorldTransform = Grip.LastWorldTransform * baseTrans;

		if (!Grip.bSkipNextTeleportCheck && (bRootHasInterface || bActorHasInterface))
		{
			TArray<FBPGripPair> HoldingControllers;
			bool bIsHeld = false;
			IVRGripInterface::Execute_IsHeld(Grip.GrippedObject, HoldingControllers, bIsHeld);

			for (FBPGripPair pair : HoldingControllers)
			{
				if (pair.HoldingController && pair.HoldingController != this && pair.HoldingController->bIsPostTeleport)
				{
					FBPActorGripInformation* pGrip = pair.HoldingController->GetGripPtrByID(pair.GripID);

					if (pGrip)
					{
						pGrip->bSkipNextTeleportCheck = true;
					}
				}
			}
		}
	}

	if (!WorldTransform.IsValid())
	{
		if (WorldTransform.ContainsNaN())
		{
			UE_LOGF(LogVRMotionController, Error, "Failed to teleport grip, bad transform, NaN detected with object: %ls", *Grip.GrippedObject->GetName());
			return false;
		}
		else if (!WorldTransform.GetRotation().IsNormalized())
		{
			WorldTransform.NormalizeRotation();

			if (!WorldTransform.IsValid())
			{
				UE_LOGF(LogVRMotionController, Error, "Failed to teleport grip, bad transform, rotation normalization issue: %ls", *Grip.GrippedObject->GetName());
				return false;
			}
			else
			{
				UE_LOGF(LogVRMotionController, Error, "Error during teleport grip, rotation not normalized for object: %ls", *Grip.GrippedObject->GetName());
			}
		}
	}

	FBPActorPhysicsHandleInformation * Handle = GetPhysicsGrip(Grip);

	if (!Handle)
	{
		PrimComp->SetWorldTransform(WorldTransform, bSweepGripTeleports, nullptr, ETeleportType::TeleportPhysics);
	}
	else if (Handle && FPhysicsInterface::IsValid(Handle->KinActorData2) && bTeleportPhysicsGrips)
	{

		if (HasGripAuthority(Grip))
			Grip.bSkipNextConstraintLengthCheck = true;

		if (Grip.bSkipNextTeleportCheck)
		{
			Grip.bSkipNextTeleportCheck = false;
		}
		else
		{
			PrimComp->SetWorldTransform(WorldTransform, bSweepGripTeleports, nullptr, ETeleportType::TeleportPhysics);
		}

		physicsTrans.SetScale3D(FVector(1.0f));

		if (Grip.bIsLerping || !bConstrainToPivot)
		{
			FBodyInstance* pInstance = PrimComp->GetBodyInstance();
			FPhysicsActorHandle ActorHandle = Handle->KinActorData2;
			FTransform newTrans = Handle->COMPosition * (Handle->RootBoneRotation * physicsTrans);
			if (pInstance && pInstance->IsValidBodyInstance())
			{
				if (FPhysScene* PhysicalScene = pInstance->GetPhysicsScene())
				{

					FPhysicsCommand::ExecuteWrite(PhysicalScene, [&]()
					{
						if (FPhysicsInterface::IsValid(ActorHandle))
						{
							FPhysicsInterface::SetKinematicTarget_AssumesLocked(ActorHandle, newTrans);
							FPhysicsInterface::SetGlobalPose_AssumesLocked(ActorHandle, newTrans);
						}
					});
				}
			}
		}
	}

	Grip.LastVelWorldTrans = PrimComp->GetComponentTransform();

	return true;
}

void UGripMotionControllerComponent::PostTeleportMoveGrippedObjects()
{
	if (!GrippedObjects.Num() && !LocallyGrippedObjects.Num())
		return;

	this->bIsPostTeleport = true;
}

void UGripMotionControllerComponent::Deactivate()
{
	Super::Deactivate();

	if (IsActive() == false && GripViewExtension.IsValid())
	{
		{

			FScopeLock ScopeLock(&CritSect);
			GripViewExtension->MotionControllerComponent = NULL;
		}

		GripViewExtension.Reset();
	}
}

void UGripMotionControllerComponent::OnAttachmentChanged()
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

void UGripMotionControllerComponent::OnRep_ReplicatedControllerTransform()
{

	if (IsServer() && HasTrackingParameters())
	{

		ApplyTrackingParameters(ReplicatedControllerTransform.Position, true, false);
	}

	if (bSmoothReplicatedMotion)
	{
		if (bReppedOnce)
		{
			bLerpingPosition = true;
			ControllerNetUpdateCount = 0.0f;
			LastUpdatesRelativePosition = this->GetRelativeLocation();
			LastUpdatesRelativeRotation = this->GetRelativeRotation();

			if (bUseExponentialSmoothing)
			{
				FVector OldToNewVector = ReplicatedControllerTransform.Position - LastUpdatesRelativePosition;
				float NewDistance = OldToNewVector.SizeSquared();

				if (NewDistance >= FMath::Square(NetworkNoSmoothUpdateDistance))
				{
					SetRelativeLocationAndRotation(ReplicatedControllerTransform.Position, ReplicatedControllerTransform.Rotation);
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
			SetRelativeLocationAndRotation(ReplicatedControllerTransform.Position, ReplicatedControllerTransform.Rotation);
			bReppedOnce = true;
		}
	}
	else
		SetRelativeLocationAndRotation(ReplicatedControllerTransform.Position, ReplicatedControllerTransform.Rotation);
}

void UGripMotionControllerComponent::UpdateTracking(float DeltaTime)
{

	if (bHasAuthority)
	{
		if (bOffsetByControllerProfile && !NewControllerProfileEvent_Handle.IsValid())
		{
			GetCurrentProfileTransform(true);
		}

		FVector Position = GetRelativeLocation();
		FRotator Orientation = GetRelativeRotation();

		if (!bUseWithoutTracking)
		{
			if (!GripViewExtension.IsValid() && GEngine)
			{
				GripViewExtension = FSceneViewExtensions::NewExtension<FGripViewExtension>(this);
			}

			float WorldToMeters = GetWorld() ? GetWorld()->GetWorldSettings()->WorldToMeters : 100.0f;
			ETrackingStatus LastTrackingStatus = CurrentTrackingStatus;
			const bool bNewTrackedState = GripPollControllerState_GameThread(Position, Orientation, bProvidedLinearVelocity, LinearVelocity, bProvidedAngularVelocity, AngularVelocityAsAxisAndLength, bProvidedLinearAcceleration, LinearAcceleration, WorldToMeters);

			if (!bTracked && bNewTrackedState)
			{
				OnActivateVisualizationComponent.Broadcast(true);
			}

			bTracked = bNewTrackedState && (bIgnoreTrackingStatus || CurrentTrackingStatus != ETrackingStatus::NotTracked);
			if (bTracked)
			{
				if (bSmoothHandTracking)
				{
					FTransform CalcedTransform = FTransform(Orientation, Position, this->GetRelativeScale3D());

					if (bSmoothWithEuroLowPassFunction)
					{
						SetRelativeTransform(EuroSmoothingParams.RunFilterSmoothing(CalcedTransform, DeltaTime));
					}
					else
					{
						if (SmoothingSpeed <= 0.f || LastSmoothRelativeTransform.Equals(FTransform::Identity))
						{
							SetRelativeTransform(CalcedTransform);
							LastSmoothRelativeTransform = CalcedTransform;
						}
						else
						{
							const float Alpha = FMath::Clamp(DeltaTime * SmoothingSpeed, 0.f, 1.f);
							LastSmoothRelativeTransform.Blend(LastSmoothRelativeTransform, CalcedTransform, Alpha);
							SetRelativeTransform(LastSmoothRelativeTransform);
						}
					}

					bWasSmoothingHand = true;
				}
				else
				{
					if (bWasSmoothingHand)
					{

						LastSmoothRelativeTransform = FTransform::Identity;
						EuroSmoothingParams.ResetSmoothingFilter();

						bWasSmoothingHand = false;
					}

					SetRelativeTransform(FTransform(Orientation, Position, this->GetRelativeScale3D()));
				}
			}

			if (LastTrackingStatus != CurrentTrackingStatus)
			{
				OnTrackingChanged.Broadcast(CurrentTrackingStatus);
			}
		}

		if (!bTracked && !bUseWithoutTracking)
			return; 

		if (GetIsReplicated() && (bTracked || bReplicateWithoutTracking))
		{
			FVector RelLoc = GetRelativeLocation();
			FRotator RelRot = GetRelativeRotation();

			if (!RelLoc.Equals(ReplicatedControllerTransform.Position) || !RelRot.Equals(ReplicatedControllerTransform.Rotation))
			{
				ControllerNetUpdateCount += DeltaTime;
				if (ControllerNetUpdateCount >= (1.0f / ControllerNetUpdateRate))
				{
					ControllerNetUpdateCount = 0.0f;

					ReplicatedControllerTransform.Position = RelLoc;
					ReplicatedControllerTransform.Rotation = RelRot;

#if WITH_PUSH_MODEL
					MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, ReplicatedControllerTransform, this);
#endif

					if (!IsServer())
					{
						AVRBaseCharacter* OwningChar = Cast<AVRBaseCharacter>(GetOwner());
						if (OverrideSendTransform != nullptr && OwningChar != nullptr)
						{
							(OwningChar->* (OverrideSendTransform))(ReplicatedControllerTransform);
						}
						else
							Server_SendControllerTransform(ReplicatedControllerTransform);
					}
				}
			}
		}
	}
	else
	{

		if (GripViewExtension.IsValid())
		{
			{

				FScopeLock ScopeLock(&CritSect);
				GripViewExtension->MotionControllerComponent = NULL;
			}

			GripViewExtension.Reset();
		}

		RunNetworkedSmoothing(DeltaTime);
	}
}

void UGripMotionControllerComponent::RunNetworkedSmoothing(float DeltaTime)
{
	if (bLerpingPosition)
	{
		if (!bUseExponentialSmoothing)
		{
			ControllerNetUpdateCount += DeltaTime;
			float LerpVal = FMath::Clamp(ControllerNetUpdateCount / (1.0f / ControllerNetUpdateRate), 0.0f, 1.0f);

			if (LerpVal >= 1.0f)
			{
				SetRelativeLocationAndRotation(ReplicatedControllerTransform.Position, ReplicatedControllerTransform.Rotation);

				bLerpingPosition = false;
				ControllerNetUpdateCount = 0.0f;
			}
			else
			{

				SetRelativeLocationAndRotation(
					FMath::Lerp(LastUpdatesRelativePosition, (FVector)ReplicatedControllerTransform.Position, LerpVal),
					FMath::Lerp(LastUpdatesRelativeRotation, ReplicatedControllerTransform.Rotation, LerpVal)
				);
			}
		}
		else 
		{
			if (InterpolationSpeed <= 0.f)
			{
				SetRelativeLocationAndRotation((FVector)ReplicatedControllerTransform.Position, ReplicatedControllerTransform.Rotation);
				bLerpingPosition = false;
				return;
			}

			const float Alpha = FMath::Clamp(DeltaTime * InterpolationSpeed, 0.f, 1.f);

			FTransform NA = FTransform(GetRelativeRotation(), GetRelativeLocation(), FVector(1.0f));
			FTransform NB = FTransform(ReplicatedControllerTransform.Rotation, (FVector)ReplicatedControllerTransform.Position, FVector(1.0f));
			NA.NormalizeRotation();
			NB.NormalizeRotation();

			NA.Blend(NA, NB, Alpha);

			if (NA.EqualsNoScale(NB))
			{
				SetRelativeLocationAndRotation(ReplicatedControllerTransform.Position, ReplicatedControllerTransform.Rotation);
				bLerpingPosition = false;
			}
			else 
			{
				SetRelativeLocationAndRotation(NA.GetTranslation(), NA.Rotator());
			}			
		}
	}
}

void UGripMotionControllerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{

	if (IsTravelingOrNullWorld())
		return;

	Super::Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsActive())
		return;

	bHasAuthority = IsLocallyControlled();

	UpdateTracking(DeltaTime);

	TickGrip(DeltaTime);
}

bool UGripMotionControllerComponent::GetGripWorldTransform(TArray<UVRGripScriptBase*>& GripScripts, float DeltaTime, FTransform & WorldTransform, const FTransform &ParentTransform, FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport, bool &bForceADrop)
{
	SCOPE_CYCLE_COUNTER(STAT_GetGripTransform);

	bool bHasValidTransform = true;

	if (GripScripts.Num())
	{
		bool bGetDefaultTransform = true;

		for (UVRGripScriptBase* Script: GripScripts)
		{
			if (Script && Script->IsScriptActive() && Script->GetWorldTransformOverrideType() == EGSTransformOverrideType::OverridesWorldTransform)
			{

				bGetDefaultTransform = false;
				break;
			}
		}

		if (bGetDefaultTransform && DefaultGripScript)
		{		
			bHasValidTransform = DefaultGripScript->CallCorrect_GetWorldTransform(this, DeltaTime, WorldTransform, ParentTransform, Grip, actor, root, bRootHasInterface, bActorHasInterface, bIsForTeleport);
			bForceADrop = DefaultGripScript->Wants_ToForceDrop();
		}

		for (UVRGripScriptBase* Script : GripScripts)
		{
			if (Script && Script->IsScriptActive() && Script->GetWorldTransformOverrideType() != EGSTransformOverrideType::None)
			{
				bHasValidTransform = Script->CallCorrect_GetWorldTransform(this, DeltaTime, WorldTransform, ParentTransform, Grip, actor, root, bRootHasInterface, bActorHasInterface, bIsForTeleport);
				bForceADrop = Script->Wants_ToForceDrop();

				if (!bHasValidTransform || bForceADrop)
					break;
			}
		}
	}
	else
	{
		if (DefaultGripScript)
		{
			bHasValidTransform = DefaultGripScript->CallCorrect_GetWorldTransform(this, DeltaTime, WorldTransform, ParentTransform, Grip, actor, root, bRootHasInterface, bActorHasInterface, bIsForTeleport);
			bForceADrop = DefaultGripScript->Wants_ToForceDrop();
		}
	}

	HandleGlobalLerpToHand(Grip, WorldTransform, DeltaTime);

	if (bHasValidTransform && !WorldTransform.IsValid())
	{
		UE_LOGF(LogVRMotionController, Warning, "Something went wrong, GetGripWorldTransform tried to return NAN!.");
		bHasValidTransform = false;
	}

	return bHasValidTransform;
}

void UGripMotionControllerComponent::TickGrip(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_TickGrip);

	if (PhysicsGrips.Num() > (GrippedObjects.Num() + LocallyGrippedObjects.Num()))
	{
		CleanUpBadPhysicsHandles();
		UE_LOGF(LogVRMotionController, Warning, "Something went wrong, there were too many physics handles for how many grips exist! Cleaned up bad handles.");
	}

	FTransform ParentTransform = GetPivotTransform();

	if(!IsServer())
		CheckTransactionBuffer();

	bool bOriginalPostTeleport = bIsPostTeleport;

	HandleGripArray(GrippedObjects, ParentTransform, DeltaTime, true);
	HandleGripArray(LocallyGrippedObjects, ParentTransform, DeltaTime);

	if (bOriginalPostTeleport)
	{
		if ((GrippedObjects.Num() || LocallyGrippedObjects.Num()))
		{
			OnTeleportedGrips.Broadcast();
		}

		bIsPostTeleport = false;
	}

	FVector newVelocitySample = ((bSampleVelocityInWorldSpace ? GetComponentLocation() : GetRelativeLocation()) - LastRelativePosition.GetTranslation()) / DeltaTime;

	switch (VelocityCalculationType)
	{
	case EVRVelocityType::VRLOCITY_Default:
	{
		ComponentVelocity = newVelocitySample;
	}break;
	case EVRVelocityType::VRLOCITY_RunningAverage:
	{
		UVRExpansionFunctionLibrary::LowPassFilter_RollingAverage(ComponentVelocity, newVelocitySample, ComponentVelocity, VelocitySamples);
	}break;
	case EVRVelocityType::VRLOCITY_SamplePeak:
	{
		if (PeakFilter.VelocitySamples != VelocitySamples)
			PeakFilter.VelocitySamples = VelocitySamples;
		UVRExpansionFunctionLibrary::UpdatePeakLowPassFilter(PeakFilter, newVelocitySample);
	}break;
	}

	LastRelativePosition = bSampleVelocityInWorldSpace ? this->GetComponentTransform() : this->GetRelativeTransform();
}

FVector UGripMotionControllerComponent::GetComponentVelocity() const
{
	if(VelocityCalculationType == EVRVelocityType::VRLOCITY_SamplePeak)
	{ 
		return PeakFilter.GetPeak();
	}

	return Super::GetComponentVelocity();
}

void UGripMotionControllerComponent::CalculateGripVelocity(FBPActorGripInformation& GripToFill, UPrimitiveComponent* ComponentToSample, float DeltaTime)
{
	if (!bHasAuthority && !IsServer())
	{
		return;
	}

	FTransform CurTrans = ComponentToSample->GetComponentTransform();

	GripToFill.LinVel = (CurTrans.GetLocation() - GripToFill.LastVelWorldTrans.GetLocation()) / DeltaTime;
	GripToFill.RotVel = FVector::RadiansToDegrees(((CurTrans.GetRotation().ToRotationVector() - GripToFill.LastVelWorldTrans.GetRotation().ToRotationVector()))) / DeltaTime;

	GripToFill.LastVelWorldTrans = CurTrans;
}

void UGripMotionControllerComponent::HandleGripArray(TArray<FBPActorGripInformation> &GrippedObjectsArray, const FTransform & ParentTransform, float DeltaTime, bool bReplicatedArray)
{
	if (GrippedObjectsArray.Num())
	{
		FTransform WorldTransform;

		for (int i = GrippedObjectsArray.Num() - 1; i >= 0; --i)
		{
			if (!HasGripMovementAuthority(GrippedObjectsArray[i]))
				continue;

			FBPActorGripInformation * Grip = &GrippedObjectsArray[i];

			if (!Grip) 
				continue;

			if (!Grip->ValueCache.bWasInitiallyRepped && !HasGripAuthority(*Grip) && !HandleGripReplication(*Grip))
				continue; 

			if (Grip->IsValid())
			{

				if (Grip->bIsPaused)
					continue;

				if (Grip->GripCollisionType == EGripCollisionType::EventsOnly)
					continue; 

				UPrimitiveComponent *root = NULL;
				AActor *actor = NULL;

				switch (Grip->GripTargetType)
				{
					case EGripTargetType::ActorGrip:

					{
						actor = Grip->GetGrippedActor();
						if(actor)
							root = Cast<UPrimitiveComponent>(actor->GetRootComponent());
					}break;

					case EGripTargetType::ComponentGrip:

					{
						root = Grip->GetGrippedComponent();
						if(root)
							actor = root->GetOwner();
					}break;

				default:break;
				}

				if (!root || !actor || !IsValid(root) || !IsValid(actor))
					continue;

				if (GetWorld()->IsInSeamlessTravel())
				{
					continue;
				}

				bool bRootHasInterface = false;
				bool bActorHasInterface = false;

				if (root->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
				{
					bRootHasInterface = true;
				}
				else if (actor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
				{

					bActorHasInterface = true;
				}

				if (Grip->GripCollisionType == EGripCollisionType::CustomGrip)
				{

					if(bRootHasInterface)
						IVRGripInterface::Execute_TickGrip(root, this, *Grip, DeltaTime);
					else if(bActorHasInterface)
						IVRGripInterface::Execute_TickGrip(actor, this, *Grip, DeltaTime);

					CalculateGripVelocity(*Grip, root, DeltaTime);
					continue;
				}

				bool bRescalePhysicsGrips = false;

				TArray<UVRGripScriptBase*> GripScripts;

				if (bRootHasInterface)
				{
					IVRGripInterface::Execute_GetGripScripts(root, GripScripts);
				}
				else if (bActorHasInterface)
				{
					IVRGripInterface::Execute_GetGripScripts(actor, GripScripts);
				}

				bool bForceADrop = false;

				bool bHasValidWorldTransform = GetGripWorldTransform(GripScripts, DeltaTime, WorldTransform, ParentTransform, *Grip, actor, root, bRootHasInterface, bActorHasInterface, false, bForceADrop);

				if (bForceADrop)
				{
					if (HasGripAuthority(*Grip))
					{
						if (bRootHasInterface)
							DropGrip_Implementation(*Grip, IVRGripInterface::Execute_SimulateOnDrop(root));
						else if (bActorHasInterface)
							DropGrip_Implementation(*Grip, IVRGripInterface::Execute_SimulateOnDrop(actor));
						else
							DropGrip_Implementation(*Grip, true);
					}

					continue;
				}
				else if (!bHasValidWorldTransform)
				{
					continue;
				}

				if (Grip->GrippedBoneName == NAME_None && !root->GetComponentTransform().GetScale3D().Equals(WorldTransform.GetScale3D()))
					bRescalePhysicsGrips = true;

				if (bIsPostTeleport)
				{

					bool bSkipTeleport = false;
					for (UVRGripScriptBase* Script : GripScripts)
					{
						if (Script && Script->IsScriptActive() && Script->Wants_DenyTeleport(this))
						{
							bSkipTeleport = true;
							break;
						}
					}

					if (!bSkipTeleport)
					{
						TeleportMoveGrip_Impl(*Grip, true, true, WorldTransform);
						continue;
					}
				}
				else
				{

				}

				if ((bRootHasInterface || bActorHasInterface) &&
					(
							(Grip->GripCollisionType != EGripCollisionType::AttachmentGrip) &&
							(Grip->GripCollisionType != EGripCollisionType::PhysicsOnly) && 
							(Grip->GripCollisionType != EGripCollisionType::SweepWithPhysics)) &&
							((Grip->GripCollisionType != EGripCollisionType::InteractiveHybridCollisionWithSweep) || ((Grip->GripCollisionType == EGripCollisionType::InteractiveHybridCollisionWithSweep) && Grip->bColliding))
					)
				{

					if (Grip->bSkipNextConstraintLengthCheck)
					{
						Grip->bSkipNextConstraintLengthCheck = false;
					}
					else
					{
						float BreakDistance = 0.0f;
						if (bRootHasInterface)
						{
							BreakDistance = IVRGripInterface::Execute_GripBreakDistance(root);
						}
						else if (bActorHasInterface)
						{

							BreakDistance = IVRGripInterface::Execute_GripBreakDistance(actor);
						}

						FVector CheckDistance;
						if (!GetPhysicsJointLength(*Grip, root, CheckDistance))
						{
							CheckDistance = (WorldTransform.GetLocation() - root->GetComponentLocation());
						}

						Grip->GripDistance = CheckDistance.Size();

						if (BreakDistance > 0.0f)
						{
							if (Grip->GripDistance >= BreakDistance)
							{
								bool bIgnoreDrop = false;
								for (UVRGripScriptBase* Script : GripScripts)
								{
									if (Script && Script->IsScriptActive() && Script->Wants_DenyAutoDrop())
									{
										bIgnoreDrop = true;
										break;
									}
								}

								if (bIgnoreDrop)
								{

								}
								else if (OnGripOutOfRange.IsBound())
								{
									uint8 GripID = Grip->GripID;
									OnGripOutOfRange.Broadcast(*Grip, Grip->GripDistance);

									FBPActorGripInformation GripInfo;
									EBPVRResultSwitch Result;
									GetGripByID(GripInfo, GripID, Result);
									if (Result == EBPVRResultSwitch::OnFailed)
									{

										continue;
									}
								}
								else if(HasGripAuthority(*Grip))
								{
									if(bRootHasInterface)
										DropGrip_Implementation(*Grip, IVRGripInterface::Execute_SimulateOnDrop(root));
									else
										DropGrip_Implementation(*Grip, IVRGripInterface::Execute_SimulateOnDrop(actor));

									continue;
								}
							}
						}
					}
				}

				switch (Grip->GripCollisionType)
				{
					case EGripCollisionType::InteractiveCollisionWithPhysics:
					case EGripCollisionType::LockedConstraint:
					{
						UpdatePhysicsHandleTransform(*Grip, WorldTransform);

						if (bRescalePhysicsGrips)
							root->SetWorldScale3D(WorldTransform.GetScale3D());

						if (
							(bHasAuthority && !this->bDisableLowLatencyUpdate &&
								((Grip->GripLateUpdateSetting == EGripLateUpdateSettings::NotWhenColliding) ||
									(Grip->GripLateUpdateSetting == EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping)))
							)
						{

							FComponentQueryParams Params(NAME_None, this->GetOwner());

							Params.AddIgnoredActor(actor);
							Params.AddIgnoredActors(root->MoveIgnoreActors);

							actor->ForEachAttachedActors([&Params](AActor* Actor)
							{
								Params.AddIgnoredActor(Actor);
								return true;
							});

							TArray<FHitResult> Hits;

							if (
									GetWorld()->ComponentSweepMulti(Hits, root, root->GetComponentLocation(), WorldTransform.GetLocation(), WorldTransform.GetRotation(), Params)
								)
							{

								UCollisionIgnoreSubsystem* CollisionIgnoreSubsystem = GetWorld()->GetSubsystem<UCollisionIgnoreSubsystem>();

								if (CollisionIgnoreSubsystem->HasCollisionIgnorePairs())
								{

									Grip->bColliding = false;

									for (const FHitResult& Hit : Hits)
									{
										if (Hit.bBlockingHit && !CollisionIgnoreSubsystem->AreComponentsIgnoringCollisions(root, Hit.Component.Get()))
										{
											Grip->bColliding = true;
											break;
										}
									}
								}
								else
								{
									if (FHitResult::GetFirstBlockingHit(Hits) != nullptr)
									{
										Grip->bColliding = true;
									}
								}
							}
							else
							{
								Grip->bColliding = false;
							}
						}

					}break;

					case EGripCollisionType::InteractiveCollisionWithSweep:
					{
						FVector OriginalPosition(root->GetComponentLocation());
						FVector NewPosition(WorldTransform.GetTranslation());

						if (!Grip->bIsLocked)
							root->ComponentVelocity = (NewPosition - OriginalPosition) / DeltaTime;

						if (Grip->bIsLocked)
							WorldTransform.SetRotation(Grip->LastLockedRotation);

						FHitResult OutHit;

						if (bProjectNonSimulatingGrips && !Grip->bIsLocked && Grip->bSetLastWorldTransform)
						{
							FScopedMovementUpdate ScopedMovementUpdate(root, EScopedUpdate::DeferredUpdates);
							FTransform baseTrans = this->GetAttachParent()->GetComponentTransform();
							root->SetWorldTransform(Grip->LastWorldTransform * baseTrans, false, nullptr, ETeleportType::None);
							root->SetWorldTransform(WorldTransform, true, &OutHit);
						}
						else
						{
							root->SetWorldTransform(WorldTransform, true, &OutHit);
						}

						if (OutHit.bBlockingHit)
						{
							Grip->bColliding = true;

							if (!Grip->bIsLocked)
							{
								Grip->bIsLocked = true;
								Grip->LastLockedRotation = root->GetComponentQuat();
							}
						}
						else
						{
							Grip->bColliding = false;

							if (Grip->bIsLocked)
								Grip->bIsLocked = false;
						}
					}break;

					case EGripCollisionType::InteractiveHybridCollisionWithPhysics:
					{
						UpdatePhysicsHandleTransform(*Grip, WorldTransform);

						if (bRescalePhysicsGrips)
							root->SetWorldScale3D(WorldTransform.GetScale3D());

						FComponentQueryParams Params(NAME_None, this->GetOwner());

						Params.AddIgnoredActor(actor);
						Params.AddIgnoredActors(root->MoveIgnoreActors);

						actor->ForEachAttachedActors([&Params](AActor* Actor)
						{
							Params.AddIgnoredActor(Actor);
							return true;
						});

						TArray<FHitResult> Hits;

						if (Grip->bLockHybridGrip)
						{
							if (!Grip->bColliding)
							{
								SetGripConstraintStiffnessAndDamping(Grip, false);
							}

							Grip->bColliding = true;
						}
						else if (GetWorld()->ComponentSweepMulti(Hits, root, root->GetComponentLocation(), WorldTransform.GetLocation(), WorldTransform.GetRotation(), Params) && FHitResult::GetFirstBlockingHit(Hits) != nullptr)
						{

							Grip->bColliding = true;

							UCollisionIgnoreSubsystem* CollisionIgnoreSubsystem = GetWorld()->GetSubsystem<UCollisionIgnoreSubsystem>();

							if (CollisionIgnoreSubsystem->HasCollisionIgnorePairs())
							{

								bool bOriginalColliding = Grip->bColliding;

								Grip->bColliding = false;

								for (const FHitResult& Hit : Hits)
								{
									if (Hit.bBlockingHit && !CollisionIgnoreSubsystem->AreComponentsIgnoringCollisions(root, Hit.Component.Get()))
									{
										if (!bOriginalColliding)
										{
											SetGripConstraintStiffnessAndDamping(Grip, false);
										}
										Grip->bColliding = true;
										break;
									}
								}

								if (!Grip->bColliding)
								{
									if (bOriginalColliding)
									{
										SetGripConstraintStiffnessAndDamping(Grip, true);
									}
								}

							}
							else
							{
								if (!Grip->bColliding)
								{
									SetGripConstraintStiffnessAndDamping(Grip, false);
								}

							}
						}
						else
						{
							if (Grip->bColliding)
							{
								SetGripConstraintStiffnessAndDamping(Grip, true);
							}

							Grip->bColliding = false;
						}

					}break;

					case EGripCollisionType::InteractiveHybridCollisionWithSweep:
					{

						FBPActorPhysicsHandleInformation * GripHandle = GetPhysicsGrip(*Grip);

						TArray<FHitResult> Hits;
						FComponentQueryParams Params(NAME_None, this->GetOwner());

						Params.AddIgnoredActor(actor);
						Params.AddIgnoredActors(root->MoveIgnoreActors);

						actor->ForEachAttachedActors([&Params](AActor* Actor)
						{
							Params.AddIgnoredActor(Actor);
							return true;
						});

						FTransform BaseTransform = root->GetComponentTransform();

						if (bProjectNonSimulatingGrips && !Grip->bColliding && Grip->bSetLastWorldTransform)
						{
							FTransform baseTrans = this->GetAttachParent()->GetComponentTransform();
							BaseTransform = Grip->LastWorldTransform * baseTrans;
						}

						bool bWasColliding = Grip->bColliding;
						bool bLerpCollisions = false;
						bool bLerpRotationOnly = false;
						bool bDistanceBasedInterpolation = false;
						float LerpSpeed = 0.0f;
						const UVRGlobalSettings* VRSettings = GetDefault<UVRGlobalSettings>();

						if (VRSettings)
						{
							bLerpCollisions = VRSettings->bLerpHybridWithSweepGrips;
							LerpSpeed = VRSettings->HybridWithSweepLerpDuration;
							bLerpRotationOnly = VRSettings->bOnlyLerpHybridRotation;
							bDistanceBasedInterpolation = VRSettings->bHybridWithSweepUseDistanceBasedLerp;
						}

						if (Grip->bLockHybridGrip)
						{
							Grip->bColliding = true;
						}

						else if (GetWorld()->ComponentSweepMulti(Hits, root, BaseTransform.GetLocation(), WorldTransform.GetLocation(), WorldTransform.GetRotation(), Params) && FHitResult::GetFirstBlockingHit(Hits) != nullptr)
						{

							Grip->bColliding = true;

							UCollisionIgnoreSubsystem* CollisionIgnoreSubsystem = GetWorld()->GetSubsystem<UCollisionIgnoreSubsystem>();
							if (CollisionIgnoreSubsystem->HasCollisionIgnorePairs())
							{

								Grip->bColliding = false;

								for (const FHitResult& Hit : Hits)
								{
									if (Hit.bBlockingHit && !CollisionIgnoreSubsystem->AreComponentsIgnoringCollisions(root, Hit.Component.Get()))
									{
										Grip->bColliding = true;
										break;
									}
								}

								if (bLerpCollisions && !Grip->bColliding)
								{
									if (bLerpCollisions && GetWorld()->ComponentSweepMulti(Hits, root, BaseTransform.GetLocation(), WorldTransform.GetLocation(), root->GetComponentRotation(), Params))
									{
										for (const FHitResult& Hit : Hits)
										{
											if (Hit.bBlockingHit && !CollisionIgnoreSubsystem->AreComponentsIgnoringCollisions(root, Hit.Component.Get()))
											{
												Grip->bColliding = true;
												break;
											}
										}
									}
								}
							}
						}

						else if (bLerpCollisions && GetWorld()->ComponentSweepMulti(Hits, root, BaseTransform.GetLocation(), WorldTransform.GetLocation(), root->GetComponentRotation(), Params) && FHitResult::GetFirstBlockingHit(Hits) != nullptr)
						{

							Grip->bColliding = true;

							UCollisionIgnoreSubsystem* CollisionIgnoreSubsystem = GetWorld()->GetSubsystem<UCollisionIgnoreSubsystem>();

							if (CollisionIgnoreSubsystem->HasCollisionIgnorePairs())
							{

								Grip->bColliding = false;

								for (const FHitResult& Hit : Hits)
								{
									if (Hit.bBlockingHit && !CollisionIgnoreSubsystem->AreComponentsIgnoringCollisions(root, Hit.Component.Get()))
									{
										Grip->bColliding = true;
										break;
									}
								}
							}
						}
						else
						{
							Grip->bColliding = false;
						}

						if (!Grip->bColliding)
						{
							if (GripHandle && !GripHandle->bIsPaused)
							{
								PausePhysicsHandle(GripHandle);

								switch (Grip->GripTargetType)
								{
								case EGripTargetType::ComponentGrip:
								{
									root->SetSimulatePhysics(false);
								}break;
								case EGripTargetType::ActorGrip:
								{
									root->SetSimulatePhysics(false);

								} break;
								}
							}

							if (bLerpCollisions && !Grip->bIsLerping)
							{
								if (bWasColliding && !Grip->bIsLerping)
								{

									Grip->OnGripTransform = root->GetComponentTransform().GetRelativeTransform(this->GetPivotTransform());
									Grip->CurrentLerpTime = 1.0f;	
									Grip->LerpSpeed = (1.f / LerpSpeed);

									if (bDistanceBasedInterpolation)
									{

										Grip->LerpSpeed *= 10.0f;
										Grip->CurrentLerpTime = LerpSpeed;
									}
								}

								if (Grip->CurrentLerpTime > 0.0f)
								{
									FTransform NB = (Grip->OnGripTransform * this->GetPivotTransform());
									float Alpha = 0.0f;

									if (bDistanceBasedInterpolation)
									{
										if (Grip->LerpSpeed <= 0.f)
										{
											Alpha = 1.0f;
											Grip->CurrentLerpTime = 0.0f;
										}
										else
										{
											Grip->CurrentLerpTime = FMath::Clamp(Grip->CurrentLerpTime - DeltaTime, 0.0f, 1.0f);
											Alpha = FMath::Clamp(DeltaTime * Grip->LerpSpeed, 0.f, 1.f);
										}

										Alpha = FMath::Clamp(DeltaTime * Grip->LerpSpeed, 0.f, 1.f);
									}
									else
									{
										Grip->CurrentLerpTime -= DeltaTime * Grip->LerpSpeed;
										float OrigAlpha = FMath::Clamp(1.0f - Grip->CurrentLerpTime, 0.f, 1.0f);
										Alpha = OrigAlpha;
									}

									FTransform NA = WorldTransform;
									NA.NormalizeRotation();
									NB.NormalizeRotation();

									if (!bLerpRotationOnly)
									{
										WorldTransform.Blend(NB, NA, Alpha);
									}
									else
									{
										WorldTransform.SetRotation(FQuat::Slerp(NB.GetRotation(), NA.GetRotation(), Alpha));
									}

									if (bDistanceBasedInterpolation)
									{
										if(NA.Equals(WorldTransform, 0.01f))
										{
											Grip->CurrentLerpTime = 0.0f;
										}
										else
										{

											Grip->OnGripTransform = WorldTransform.GetRelativeTransform(this->GetPivotTransform());
										}
									}
								}
							}

							if (bProjectNonSimulatingGrips && Grip->bSetLastWorldTransform)
							{
								FScopedMovementUpdate ScopedMovementUpdate(root, EScopedUpdate::DeferredUpdates);
								FTransform baseTrans = this->GetAttachParent()->GetComponentTransform();
								root->SetWorldTransform(Grip->LastWorldTransform * baseTrans, false, nullptr, ETeleportType::None);
								root->SetWorldTransform(WorldTransform, false);
							}
							else
							{
								root->SetWorldTransform(WorldTransform, false);
							}

							if (GripHandle)
							{
								UpdatePhysicsHandleTransform(*Grip, WorldTransform);
							}

						}
						else if (Grip->bColliding)
						{
							if (!GripHandle)
							{
								SetUpPhysicsHandle(*Grip, &GripScripts);
							}
							else if (GripHandle->bIsPaused)
							{
								UnPausePhysicsHandle(*Grip, GripHandle);
							}

							if (GripHandle)
							{
								UpdatePhysicsHandleTransform(*Grip, WorldTransform);
								if (bRescalePhysicsGrips)
									root->SetWorldScale3D(WorldTransform.GetScale3D());
							}
						}
						else
						{

							if (GripHandle)
							{
								UpdatePhysicsHandleTransform(*Grip, WorldTransform);
								if (bRescalePhysicsGrips)
										root->SetWorldScale3D(WorldTransform.GetScale3D());
							}
						}

					}break;

					case EGripCollisionType::SweepWithPhysics:
					{

						if (root->IsSimulatingPhysics())
						{
							root->SetSimulatePhysics(false);
						}

						FVector OriginalPosition(root->GetComponentLocation());
						FRotator OriginalOrientation(root->GetComponentRotation());

						FVector NewPosition(WorldTransform.GetTranslation());
						FRotator NewOrientation(WorldTransform.GetRotation());

						root->ComponentVelocity = (NewPosition - OriginalPosition) / DeltaTime;

						if (bUseWithoutTracking || NewPosition != OriginalPosition || NewOrientation != OriginalOrientation)
						{
							FVector move = NewPosition - OriginalPosition;

							const float MinMovementDistSq = (FMath::Square(4.f*UE_KINDA_SMALL_NUMBER));

							if (bUseWithoutTracking || move.SizeSquared() > MinMovementDistSq || NewOrientation != OriginalOrientation)
							{
								if (CheckComponentWithSweep(root, move, OriginalOrientation, false))
								{
									Grip->bColliding = true;
								}
								else
								{
									Grip->bColliding = false;
								}

								TArray<USceneComponent* > PrimChildren;
								root->GetChildrenComponents(true, PrimChildren);
								for (USceneComponent * Prim : PrimChildren)
								{
									if (UPrimitiveComponent * primComp = Cast<UPrimitiveComponent>(Prim))
									{
										CheckComponentWithSweep(primComp, move, primComp->GetComponentRotation(), false);
									}
								}
							}
						}

						if (bProjectNonSimulatingGrips && Grip->bSetLastWorldTransform)
						{
							FScopedMovementUpdate ScopedMovementUpdate(root, EScopedUpdate::DeferredUpdates);
							FTransform baseTrans = this->GetAttachParent()->GetComponentTransform();
							root->SetWorldTransform(Grip->LastWorldTransform * baseTrans, false, nullptr, ETeleportType::None);

							root->SetWorldTransform(WorldTransform, false);
						}
						else
						{

							root->SetWorldTransform(WorldTransform, false);
						}

					}break;

					case EGripCollisionType::PhysicsOnly:
					{

						if (root->IsSimulatingPhysics())
						{
							root->SetSimulatePhysics(false);
						}

						if (bProjectNonSimulatingGrips && Grip->bSetLastWorldTransform)
						{
							FScopedMovementUpdate ScopedMovementUpdate(root, EScopedUpdate::DeferredUpdates);
							FTransform baseTrans = this->GetAttachParent()->GetComponentTransform();
							root->SetWorldTransform(Grip->LastWorldTransform * baseTrans, false, nullptr, ETeleportType::None);

							root->SetWorldTransform(WorldTransform, false);
						}
						else
						{

							root->SetWorldTransform(WorldTransform, false);
						}
					}break;

					case EGripCollisionType::AttachmentGrip:
					{
						FTransform RelativeTrans = WorldTransform.GetRelativeTransform(ParentTransform);

						if (!root->GetAttachParent() || root->IsSimulatingPhysics())
						{
							UE_LOGF(LogVRMotionController, Warning, "Attachment Grip was missing attach parent - Attempting to Re-attach");

							if (HasGripMovementAuthority(*Grip) || IsServer())
							{
								root->SetSimulatePhysics(false);
								if (root->AttachToComponent(IsValid(CustomPivotComponent) ? CustomPivotComponent.Get() : this, FAttachmentTransformRules::KeepWorldTransform))
								{
									UE_LOGF(LogVRMotionController, Warning, "Re-attached");
									if (!root->GetRelativeTransform().Equals(RelativeTrans))
									{
										root->SetRelativeTransform(RelativeTrans);
									}
								}
							}
						}
						else
						{
							if (!root->GetRelativeTransform().Equals(RelativeTrans))
							{
								root->SetRelativeTransform(RelativeTrans);
							}
						}

					}break;

					case EGripCollisionType::ManipulationGrip:
					case EGripCollisionType::ManipulationGripWithWristTwist:
					{
						UpdatePhysicsHandleTransform(*Grip, WorldTransform);
						if (bRescalePhysicsGrips)
							root->SetWorldScale3D(WorldTransform.GetScale3D());

					}break;

					default:
					{}break;
				}

				CalculateGripVelocity(*Grip, root, DeltaTime); 

				if (bAlwaysSendTickGrip)
				{

					if (bRootHasInterface)
					{
						IVRGripInterface::Execute_TickGrip(root, this, *Grip, DeltaTime);
					}

					if (bActorHasInterface)
					{
						IVRGripInterface::Execute_TickGrip(actor, this, *Grip, DeltaTime);
					}
				}
			}
			else
			{

				if (!Grip->bIsPendingKill)
				{
					CleanUpBadGrip(GrippedObjectsArray, i, bReplicatedArray);
				}
			}
		}
	}
}

void UGripMotionControllerComponent::CleanUpBadGrip(TArray<FBPActorGripInformation> &GrippedObjectsArray, int GripIndex, bool bReplicatedArray)
{

	if (!DestroyPhysicsHandle(GrippedObjectsArray[GripIndex]))
	{

		for (int g = PhysicsGrips.Num() - 1; g >= 0; --g)
		{
			if (!PhysicsGrips[g].HandledObject || PhysicsGrips[g].HandledObject == GrippedObjectsArray[GripIndex].GrippedObject || !IsValid(PhysicsGrips[g].HandledObject))
			{

				DestroyPhysicsHandle(&PhysicsGrips[g]);
				PhysicsGrips.RemoveAt(g);
			}
		}
	}

	if (IsServer() || HasGripAuthority(GrippedObjectsArray[GripIndex]))
	{
		DropGrip_Implementation(GrippedObjectsArray[GripIndex], false);
		UE_LOGF(LogVRMotionController, Warning, "Gripped object was null or destroying, auto dropping it");
	}
	else
	{
		GrippedObjectsArray[GripIndex].bIsPendingKill = true;
		GrippedObjectsArray[GripIndex].bIsPaused = true;
	}
}

void UGripMotionControllerComponent::CleanUpBadPhysicsHandles()
{

	for (int g = PhysicsGrips.Num() - 1; g >= 0; --g)
	{
		FBPActorGripInformation * GripInfo = LocallyGrippedObjects.FindByKey(PhysicsGrips[g].GripID);
		if(!GripInfo)
			GripInfo = GrippedObjects.FindByKey(PhysicsGrips[g].GripID);

		if (!GripInfo)
		{

			DestroyPhysicsHandle(&PhysicsGrips[g]);
			PhysicsGrips.RemoveAt(g);
		}
	}
}

bool UGripMotionControllerComponent::UpdatePhysicsHandle(uint8 GripID, bool bFullyRecreate)
{
	FBPActorGripInformation* GripInfo = GrippedObjects.FindByKey(GripID);
	if (!GripInfo)
		GripInfo = LocallyGrippedObjects.FindByKey(GripID);

	if (!GripInfo)
		return false;

	return UpdatePhysicsHandle(*GripInfo, bFullyRecreate);
}

bool UGripMotionControllerComponent::UpdatePhysicsHandle(const FBPActorGripInformation& GripInfo, bool bFullyRecreate)
{
	int HandleIndex = 0;
	FBPActorPhysicsHandleInformation* HandleInfo = GetPhysicsGrip(GripInfo);

	if (!HandleInfo || HandleInfo->bIsPaused || !HandleInfo->bInitiallySetup)
		return false;

	if (bFullyRecreate || !HandleInfo->HandleData2.IsValid() || HandleInfo->bSkipResettingCom)
	{
		return SetUpPhysicsHandle(GripInfo);
	}

	UPrimitiveComponent* root = GripInfo.GetGrippedComponent();
	AActor* pActor = GripInfo.GetGrippedActor();

	if (!root && pActor)
		root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

	if (!root)
		return false;

	FBodyInstance* rBodyInstance = root->GetBodyInstance(GripInfo.GrippedBoneName);
	if (!rBodyInstance || !rBodyInstance->IsValidBodyInstance() || !FPhysicsInterface::IsValid(rBodyInstance->ActorHandle))
	{
		return false;
	}

	check(rBodyInstance->BodySetup->GetCollisionTraceFlag() != CTF_UseComplexAsSimple);

	FPhysicsCommand::ExecuteWrite(rBodyInstance->ActorHandle, [&](const FPhysicsActorHandle& Actor)
	{
			if (HandleInfo)
			{
				if (HandleInfo->KinActorData2 && FPhysicsInterface::IsValid(HandleInfo->KinActorData2))
				{

					Chaos::FConstraintBase* ConstraintHandle = HandleInfo->HandleData2.Constraint;
					if (ConstraintHandle)
					{
						((Chaos::FJointConstraint*)ConstraintHandle)->SetParticleProxies({ HandleInfo->KinActorData2, Actor });
					}

					if (HandleInfo->bSetCOM && !HandleInfo->bSkipResettingCom)
					{
						FTransform localCom = FPhysicsInterface::GetComTransformLocal_AssumesLocked(Actor);

						localCom.SetLocation(HandleInfo->COMPosition.GetTranslation());
						FPhysicsInterface::SetComLocalPose_AssumesLocked(Actor, localCom);
					}
				}
			}
	});

	return true;

}

bool UGripMotionControllerComponent::PausePhysicsHandle(FBPActorPhysicsHandleInformation* HandleInfo)
{
	if (!HandleInfo)
		return false;

	HandleInfo->bIsPaused = true;
	HandleInfo->bInitiallySetup = false;
	FPhysicsInterface::ReleaseConstraint(HandleInfo->HandleData2);
	return true;
}

bool UGripMotionControllerComponent::UnPausePhysicsHandle(FBPActorGripInformation& GripInfo, FBPActorPhysicsHandleInformation* HandleInfo)
{
	if (!HandleInfo)
		return false;

	HandleInfo->bIsPaused = false;
	SetUpPhysicsHandle(GripInfo);

	return true;
}

bool UGripMotionControllerComponent::DestroyPhysicsHandle(FBPActorPhysicsHandleInformation* HandleInfo)
{
	if (!HandleInfo)
		return false;

	FPhysicsInterface::ReleaseConstraint(HandleInfo->HandleData2);

	if (!HandleInfo->bSkipDeletingKinematicActor)
	{
		if (FPhysicsInterface::IsValid(HandleInfo->KinActorData2))
		{
			FPhysicsActorHandle ActorHandle = HandleInfo->KinActorData2;
			FPhysicsCommand::ExecuteWrite(ActorHandle, [&](const FPhysicsActorHandle& Actor)
			{
					FPhysicsInterface::ReleaseActor(HandleInfo->KinActorData2, FPhysicsInterface::GetCurrentScene(HandleInfo->KinActorData2));
			});
		}
	}

	return true;
}

bool UGripMotionControllerComponent::DestroyPhysicsHandle(const FBPActorGripInformation &Grip, bool bSkipUnregistering)
{
	FBPActorPhysicsHandleInformation * HandleInfo = GetPhysicsGrip(Grip);

	if (!HandleInfo)
	{
		return true;
	}

	if (IsValid(Grip.GrippedObject))
	{

		UPrimitiveComponent* root = Grip.GetGrippedComponent();
		AActor* pActor = Grip.GetGrippedActor();

		if (!root && pActor)
			root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

		if (root)
		{
			if (FBodyInstance* rBodyInstance = root->GetBodyInstance(Grip.GrippedBoneName))
			{

				if (!bSkipUnregistering)
				{
					if (rBodyInstance->OnRecalculatedMassProperties().IsBoundToObject(this))
					{
						rBodyInstance->OnRecalculatedMassProperties().RemoveAll(this);
					}
				}

				if (HandleInfo->bSetCOM)
				{

					FVector vel = rBodyInstance->GetUnrealWorldVelocity();
					FVector aVel = rBodyInstance->GetUnrealWorldAngularVelocityInRadians();
					FVector originalCOM = rBodyInstance->GetCOMPosition();

					if (rBodyInstance->IsValidBodyInstance() && rBodyInstance->BodySetup.IsValid())
					{
						rBodyInstance->UpdateMassProperties();
					}

					if (rBodyInstance->IsInstanceSimulatingPhysics())
					{

						vel += FVector::CrossProduct(aVel, rBodyInstance->GetCOMPosition() - originalCOM);
						rBodyInstance->SetLinearVelocity(vel, false);
					}
				}
			}
		}
	}

	DestroyPhysicsHandle(HandleInfo);

	int index;
	if (GetPhysicsGripIndex(Grip, index))
		PhysicsGrips.RemoveAt(index);

	return true;
}

void UGripMotionControllerComponent::OnGripMassUpdated(FBodyInstance* GripBodyInstance)
{
	TArray<FBPActorGripInformation> GripArray;
	this->GetAllGrips(GripArray);
	FBPActorGripInformation NewGrip;

	for (int i = 0; i < GripArray.Num(); i++)
	{
		NewGrip = GripArray[i];

		UPrimitiveComponent *root = NewGrip.GetGrippedComponent();
		AActor * pActor = NewGrip.GetGrippedActor();

		if (!root && pActor)
		{
			if (!IsValid(pActor))
				continue;

			root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());
		}

		if (!root || root != GripBodyInstance->OwnerComponent)
			continue;

		if (IsValid(root))
		{
			UpdatePhysicsHandle(NewGrip, false);
		}
		break;
	}
}

bool UGripMotionControllerComponent::SetUpPhysicsHandle(const FBPActorGripInformation &NewGrip, TArray<UVRGripScriptBase*> * GripScripts)
{
	UPrimitiveComponent *root = NewGrip.GetGrippedComponent();
	AActor * pActor = NewGrip.GetGrippedActor();

	if(!root && pActor)
		root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());

	if (!root)
		return false;

	FBPActorPhysicsHandleInformation* HandleInfo = GetPhysicsGrip(NewGrip);
	if (HandleInfo == nullptr)
	{
		HandleInfo = CreatePhysicsGrip(NewGrip);
	}

	if (HandleInfo->bIsPaused)
	{
		return false;
	}

	HandleInfo->bSetCOM = false; 
	HandleInfo->bSkipDeletingKinematicActor = (bConstrainToPivot && !NewGrip.bIsLerping);

	TArray<UVRGripScriptBase*> LocalGripScripts;
	if (GripScripts == nullptr)
	{
		if (root && root->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			if (IVRGripInterface::Execute_GetGripScripts(root, LocalGripScripts))
			{
				GripScripts = &LocalGripScripts;
			}
		}
		else if (pActor && pActor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			if (IVRGripInterface::Execute_GetGripScripts(pActor, LocalGripScripts))
			{
				GripScripts = &LocalGripScripts;
			}
		}
	}

	if (!NewGrip.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings || !NewGrip.AdvancedGripSettings.PhysicsSettings.bSkipSettingSimulating)
	{
		root->SetSimulatePhysics(true);
	}

	FBodyInstance* rBodyInstance = root->GetBodyInstance(NewGrip.GrippedBoneName);
	Chaos::FPhysicsObject* PhysicsActor = root->GetPhysicsObjectByName(NewGrip.GrippedBoneName);

	bool bUseActorHandle = true;
	if (!rBodyInstance || !rBodyInstance->IsValidBodyInstance() || !FPhysicsInterface::IsValid(rBodyInstance->ActorHandle) || !rBodyInstance->BodySetup.IsValid())
	{	
		if (PhysicsActor)
		{
			bUseActorHandle = false;
		}
		else
		{
			return false;
		}
	}

	if (rBodyInstance && bUseActorHandle)
	{
		check(rBodyInstance->BodySetup->GetCollisionTraceFlag() != CTF_UseComplexAsSimple);

		if (!HandleInfo->bSkipResettingCom && !FPhysicsInterface::IsValid(HandleInfo->KinActorData2) && !rBodyInstance->OnRecalculatedMassProperties().IsBoundToObject(this))
		{

			rBodyInstance->UpdateMassProperties();

		}
	}

	FTransform RootBoneRotation = FTransform::Identity;
	if (NewGrip.GrippedBoneName != NAME_None)
	{

	}
	else
	{

		if (USkeletalMeshComponent* skele = Cast<USkeletalMeshComponent>(root))
		{
			int32 RootBodyIndex = INDEX_NONE;
			if (const UPhysicsAsset* PhysicsAsset = skele->GetPhysicsAsset())
			{
				for (int32 i = 0; i < skele->GetNumBones(); i++)
				{
					if (PhysicsAsset->FindBodyIndex(skele->GetBoneName(i)) != INDEX_NONE)
					{
						RootBodyIndex = i;
						break;
					}
				}
			}

			if (RootBodyIndex != INDEX_NONE)
			{
				RootBoneRotation = FTransform(skele->GetBoneTransform(RootBodyIndex, FTransform::Identity));
				RootBoneRotation.SetScale3D(FVector(1.f));
				RootBoneRotation.NormalizeRotation();
				HandleInfo->RootBoneRotation = RootBoneRotation;
			}
		}
	}

	bool bExecutedPhys = FPhysicsCommand::ExecuteWrite(PhysicsActor, PhysicsActor, [&](Chaos::FPhysicsObject* PhysActor, Chaos::FPhysicsObject* PhysActorNULL)
	{
		const FPhysicsActorHandle& Actor = bUseActorHandle ? rBodyInstance->GetPhysicsActorHandle() : nullptr;
		Chaos::FWritePhysicsObjectInterface_External Interface = FPhysicsObjectExternalInterface::GetWrite_AssumesLocked();
		FChaosScene* PhysScene = PhysicsObjectPhysicsCoreInterface::GetScene({ &PhysActor, 1 });
		FTransform PhysActorTransform = Interface.GetTransform(PhysActor);

		if (!PhysScene)
		{
			return;
		}

		FTransform KinPose;
		FTransform trans = Interface.GetTransform(PhysActor);

		EPhysicsGripCOMType COMType = NewGrip.AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings;

		if (!NewGrip.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings || COMType == EPhysicsGripCOMType::COM_Default)
		{
			if (NewGrip.GripCollisionType == EGripCollisionType::ManipulationGrip || NewGrip.GripCollisionType == EGripCollisionType::ManipulationGripWithWristTwist)
			{
				COMType = EPhysicsGripCOMType::COM_GripAtControllerLoc;
			}
			else
			{
				COMType = EPhysicsGripCOMType::COM_SetAndGripAt;
			}
		}

		if (!bUseActorHandle)
		{
			if (COMType == EPhysicsGripCOMType::COM_SetAndGripAt)
			{
				COMType = EPhysicsGripCOMType::COM_GripAtControllerLoc;
			}
		}

		if (bUseActorHandle && COMType == EPhysicsGripCOMType::COM_SetAndGripAt)
		{

			FTransform ForwardTrans = (RootBoneRotation * NewGrip.RelativeTransform);
			ForwardTrans.NormalizeRotation();
			FVector Loc = (FTransform(ForwardTrans.ToInverseMatrixWithScale())).GetLocation();
			Loc *= root->GetComponentScale();

			FTransform localCom = FPhysicsInterface::GetComTransformLocal_AssumesLocked(Actor);
			localCom.SetLocation(Loc);
			FPhysicsInterface::SetComLocalPose_AssumesLocked(Actor, localCom);

			FVector ComLoc = FPhysicsInterface::GetComTransform_AssumesLocked(Actor).GetLocation(); 
			trans.SetLocation(ComLoc);
			HandleInfo->COMPosition = FTransform(PhysActorTransform.InverseTransformPosition(ComLoc));
			HandleInfo->bSetCOM = true;
		}
		else if (COMType == EPhysicsGripCOMType::COM_GripAtControllerLoc)
		{
			FTransform ObjectTransform = PhysActorTransform;
			ObjectTransform.SetScale3D(root->GetComponentScale());

			FVector ControllerLoc = (FTransform(NewGrip.RelativeTransform.ToInverseMatrixWithScale()) * ObjectTransform).GetLocation();
			trans.SetLocation(ControllerLoc);
			HandleInfo->COMPosition = FTransform(PhysActorTransform.InverseTransformPosition(ControllerLoc));
		}
		else if (COMType != EPhysicsGripCOMType::COM_AtPivot)
		{
			FVector ComLoc = Interface.GetWorldCoM(PhysActor);
			trans.SetLocation(ComLoc);
			HandleInfo->COMPosition = FTransform(PhysActorTransform.InverseTransformPosition(ComLoc));
		}

		KinPose = trans;
		bool bRecreatingConstraint = false;

		if (NewGrip.GripCollisionType == EGripCollisionType::ManipulationGripWithWristTwist)
		{
			FTransform PivTrans = GetPivotTransform();

			FQuat DeltaQuat = (PivTrans.GetRotation().Inverse() * KinPose.GetRotation()).Inverse();

			KinPose.SetRotation(KinPose.GetRotation() * DeltaQuat);
			HandleInfo->COMPosition.SetRotation(HandleInfo->COMPosition.GetRotation()* DeltaQuat);
		}

		if (GripScripts)
		{
			bool bResetCom = false;

			for (UVRGripScriptBase* Script : *GripScripts)
			{
				if (Script && Script->IsScriptActive() && Script->InjectPrePhysicsHandle())
				{
					Script->HandlePrePhysicsHandle(this, NewGrip, HandleInfo, KinPose);
					bResetCom = true;
				}
			}

			if (bUseActorHandle && HandleInfo->bSetCOM && bResetCom)
			{
				FTransform localCom = FPhysicsInterface::GetComTransformLocal_AssumesLocked(Actor);
				localCom.SetLocation(HandleInfo->COMPosition.GetTranslation());

				FPhysicsInterface::SetComLocalPose_AssumesLocked(Actor, localCom);
			}
		}

		if (!NewGrip.bIsLerping && bConstrainToPivot && IsValid(CustomPivotComponent))
		{
			if (UPrimitiveComponent* PivotPrim = Cast<UPrimitiveComponent>(CustomPivotComponent))
			{
				if (FBodyInstance* Bodyinst = PivotPrim->GetBodyInstance(CustomPivotComponentSocketName))
				{
					HandleInfo->KinActorData2 = Bodyinst->GetPhysicsActorHandle();
				}
			}
		}

		if (!FPhysicsInterface::IsValid(HandleInfo->KinActorData2))
		{

			FActorCreationParams ActorParams;
			ActorParams.InitialTM = KinPose;
			ActorParams.DebugName = nullptr;
			ActorParams.bEnableGravity = false;
			ActorParams.bQueryOnly = false;
			ActorParams.bStatic = false;
			ActorParams.Scene = PhysScene;
			FPhysicsInterface::CreateActor(ActorParams, HandleInfo->KinActorData2);

			if (FPhysicsInterface::IsValid(HandleInfo->KinActorData2))
			{
				FPhysicsInterface::SetMass_AssumesLocked(HandleInfo->KinActorData2, 1.0f);
				FPhysicsInterface::SetMassSpaceInertiaTensor_AssumesLocked(HandleInfo->KinActorData2, FVector(1.f));
				FPhysicsInterface::SetIsKinematic_AssumesLocked(HandleInfo->KinActorData2, true);
				FPhysicsInterface::SetMaxDepenetrationVelocity_AssumesLocked(HandleInfo->KinActorData2, MAX_FLT);

			}

			using namespace Chaos;

			HandleInfo->KinActorData2->GetGameThreadAPI().SetGeometry(MakeImplicitObjectPtr<TSphere<FReal, 3>>(TVector<FReal, 3>(0.f), 1000.f));
			HandleInfo->KinActorData2->GetGameThreadAPI().SetObjectState(EObjectStateType::Kinematic);
			FPhysicsInterface::AddActorToSolver(HandleInfo->KinActorData2, ActorParams.Scene->GetSolver());

		}

		if (!HandleInfo->HandleData2.IsValid())
		{

			if (!NewGrip.bIsLerping && bConstrainToPivot)
			{
				FTransform TargetTrans(FTransform(NewGrip.RelativeTransform.ToMatrixNoScale().Inverse()) * HandleInfo->RootBoneRotation.Inverse());
				HandleInfo->HandleData2 = FPhysicsInterface::CreateConstraint(HandleInfo->KinActorData2->GetPhysicsObject(), PhysActor, FTransform::Identity, TargetTrans);
			}
			else
			{
				HandleInfo->HandleData2 = FPhysicsInterface::CreateConstraint(HandleInfo->KinActorData2->GetPhysicsObject(), PhysActor, FTransform::Identity, KinPose.GetRelativeTransform(PhysActorTransform));
			}
		}
		else
		{
			bRecreatingConstraint = true;

			FPhysicsInterface::ReleaseConstraint(HandleInfo->HandleData2);

			if (!NewGrip.bIsLerping && bConstrainToPivot)
			{
				FTransform TargetTrans(NewGrip.RelativeTransform.ToMatrixNoScale().Inverse());
				HandleInfo->HandleData2 = FPhysicsInterface::CreateConstraint(HandleInfo->KinActorData2->GetPhysicsObject(), PhysActor, FTransform::Identity, TargetTrans);
			}
			else
			{
				HandleInfo->HandleData2 = FPhysicsInterface::CreateConstraint(HandleInfo->KinActorData2->GetPhysicsObject(), PhysActor, FTransform::Identity, KinPose.GetRelativeTransform(PhysActorTransform));
			}

		}

		if (HandleInfo->HandleData2.IsValid())
		{
			FPhysicsInterface::SetBreakForces_AssumesLocked(HandleInfo->HandleData2, MAX_FLT, MAX_FLT);

			if (NewGrip.GripCollisionType == EGripCollisionType::LockedConstraint)
			{
				FPhysicsInterface::SetLinearMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::X, ELinearConstraintMotion::LCM_Locked);
				FPhysicsInterface::SetLinearMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Y, ELinearConstraintMotion::LCM_Locked);
				FPhysicsInterface::SetLinearMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Z, ELinearConstraintMotion::LCM_Locked);
				FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Twist, EAngularConstraintMotion::ACM_Locked);
				FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Swing1, EAngularConstraintMotion::ACM_Locked);
				FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Swing2, EAngularConstraintMotion::ACM_Locked);
				FPhysicsInterface::SetProjectionEnabled_AssumesLocked(HandleInfo->HandleData2, true, 0.01f, 0.01f);
			}
			else
			{
				FPhysicsInterface::SetLinearMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::X, ELinearConstraintMotion::LCM_Free);
				FPhysicsInterface::SetLinearMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Y, ELinearConstraintMotion::LCM_Free);
				FPhysicsInterface::SetLinearMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Z, ELinearConstraintMotion::LCM_Free);
				FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Twist, EAngularConstraintMotion::ACM_Free);
				FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Swing1, EAngularConstraintMotion::ACM_Free);
				FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked(HandleInfo->HandleData2, PhysicsInterfaceTypes::ELimitAxis::Swing2, EAngularConstraintMotion::ACM_Free);
				FPhysicsInterface::SetProjectionEnabled_AssumesLocked(HandleInfo->HandleData2, false);
			}

			FPhysicsInterface::SetDrivePosition(HandleInfo->HandleData2, FVector::ZeroVector);

			bool bUseForceDrive = (NewGrip.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && NewGrip.AdvancedGripSettings.PhysicsSettings.PhysicsConstraintType == EPhysicsGripConstraintType::ForceConstraint);

			float Stiffness = NewGrip.Stiffness;
			float Damping = NewGrip.Damping;
			float MaxForce;
			float AngularStiffness;
			float AngularDamping;
			float AngularMaxForce;

			const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

			if (NewGrip.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && NewGrip.AdvancedGripSettings.PhysicsSettings.bUseCustomAngularValues)
			{
				AngularStiffness = NewGrip.AdvancedGripSettings.PhysicsSettings.AngularStiffness;
				AngularDamping = NewGrip.AdvancedGripSettings.PhysicsSettings.AngularDamping;
			}
			else
			{
				AngularStiffness = Stiffness * ANGULAR_STIFFNESS_MULTIPLIER; 
				AngularDamping = Damping * ANGULAR_DAMPING_MULTIPLIER; 

				if (!VRSettings.bUseChaosTranslationScalers)
				{
					AngularStiffness *= ANGULAR_STIFFNESS_MULTIPLIER_CHAOS;
					AngularDamping *= ANGULAR_DAMPING_MULTIPLIER_CHAOS;
				}
			}

			if (VRSettings.bUseChaosTranslationScalers)
			{
				Stiffness *= VRSettings.LinearDriveStiffnessScale;
				Damping *= VRSettings.LinearDriveDampingScale;
				AngularStiffness *= VRSettings.AngularDriveStiffnessScale;
				AngularDamping *= VRSettings.AngularDriveDampingScale;
			}
			else
			{
				auto CVarLinearDriveStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.LinearDriveStiffnessScale"));
				auto CVarLinearDriveDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.LinaearDriveDampingScale"));
				auto CVarAngularDriveStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.AngularDriveStiffnessScale"));
				auto CVarAngularDriveDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.AngularDriveDampingScale"));

				Stiffness *= CVarLinearDriveStiffnessScale->GetFloat();
				Damping *= CVarLinearDriveDampingScale->GetFloat();
				AngularStiffness *= CVarAngularDriveStiffnessScale->GetFloat();
				AngularDamping *= CVarAngularDriveDampingScale->GetFloat();
			}

			AngularMaxForce = (float)FMath::Clamp<double>((double)AngularStiffness * (double)NewGrip.AdvancedGripSettings.PhysicsSettings.AngularMaxForceCoefficient, 0, (double)MAX_FLT);
			MaxForce = (float)FMath::Clamp<double>((double)Stiffness * (double)NewGrip.AdvancedGripSettings.PhysicsSettings.LinearMaxForceCoefficient, 0, (double)MAX_FLT);

			if (NewGrip.GripCollisionType == EGripCollisionType::ManipulationGrip || NewGrip.GripCollisionType == EGripCollisionType::ManipulationGripWithWristTwist)
			{
				if (!bRecreatingConstraint)
				{
					FConstraintDrive NewLinDrive;
					NewLinDrive.bEnablePositionDrive = true;
					NewLinDrive.bEnableVelocityDrive = true;
					NewLinDrive.Damping = Damping;
					NewLinDrive.Stiffness = Stiffness;
					NewLinDrive.MaxForce = MaxForce;

					HandleInfo->LinConstraint.XDrive = NewLinDrive;
					HandleInfo->LinConstraint.YDrive = NewLinDrive;
					HandleInfo->LinConstraint.ZDrive = NewLinDrive;
				}

				if (NewGrip.GripCollisionType == EGripCollisionType::ManipulationGripWithWristTwist)
				{
					if (!bRecreatingConstraint)
					{
						FConstraintDrive NewAngDrive;
						NewAngDrive.bEnablePositionDrive = true;
						NewAngDrive.bEnableVelocityDrive = true;
						NewAngDrive.Damping = AngularDamping;
						NewAngDrive.Stiffness = AngularStiffness;
						NewAngDrive.MaxForce = AngularMaxForce;

						HandleInfo->AngConstraint.AngularDriveMode = EAngularDriveMode::TwistAndSwing;

						HandleInfo->AngConstraint.TwistDrive = NewAngDrive;
					}
				}

				if (GripScripts)
				{

					for (UVRGripScriptBase* Script : *GripScripts)
					{
						if (Script && Script->IsScriptActive() && Script->InjectPostPhysicsHandle())
						{
							Script->HandlePostPhysicsHandle(this, HandleInfo);
						}
					}
				}

				FPhysicsInterface::UpdateLinearDrive_AssumesLocked(HandleInfo->HandleData2, HandleInfo->LinConstraint);
				FPhysicsInterface::UpdateAngularDrive_AssumesLocked(HandleInfo->HandleData2, HandleInfo->AngConstraint);
			}
			else
			{
				if (NewGrip.GripCollisionType == EGripCollisionType::InteractiveHybridCollisionWithPhysics)
				{

					Stiffness *= HYBRID_PHYSICS_GRIP_MULTIPLIER;
					AngularStiffness *= HYBRID_PHYSICS_GRIP_MULTIPLIER;
					AngularMaxForce = (float)FMath::Clamp<double>((double)AngularStiffness * (double)NewGrip.AdvancedGripSettings.PhysicsSettings.AngularMaxForceCoefficient, 0, (double)MAX_FLT);
					MaxForce = (float)FMath::Clamp<double>((double)Stiffness * (double)NewGrip.AdvancedGripSettings.PhysicsSettings.LinearMaxForceCoefficient, 0, (double)MAX_FLT);
				}

				if (!bRecreatingConstraint)
				{
					if (NewGrip.GripCollisionType != EGripCollisionType::LockedConstraint)
					{
						FConstraintDrive NewLinDrive;
						NewLinDrive.bEnablePositionDrive = true;
						NewLinDrive.bEnableVelocityDrive = true;
						NewLinDrive.Damping = Damping;
						NewLinDrive.Stiffness = Stiffness;
						NewLinDrive.MaxForce = MaxForce;

						FConstraintDrive NewAngDrive;
						NewAngDrive.bEnablePositionDrive = true;
						NewAngDrive.bEnableVelocityDrive = true;
						NewAngDrive.Damping = AngularDamping;
						NewAngDrive.Stiffness = AngularStiffness;
						NewAngDrive.MaxForce = AngularMaxForce;

						HandleInfo->LinConstraint.XDrive.bEnablePositionDrive = true;
						HandleInfo->LinConstraint.YDrive.bEnablePositionDrive = true;
						HandleInfo->LinConstraint.ZDrive.bEnablePositionDrive = true;

						HandleInfo->LinConstraint.XDrive = NewLinDrive;
						HandleInfo->LinConstraint.YDrive = NewLinDrive;
						HandleInfo->LinConstraint.ZDrive = NewLinDrive;

						HandleInfo->AngConstraint.AngularDriveMode = EAngularDriveMode::SLERP;
						HandleInfo->AngConstraint.SlerpDrive = NewAngDrive;
					}
				}

				if (GripScripts)
				{

					for (UVRGripScriptBase* Script : *GripScripts)
					{
						if (Script && Script->IsScriptActive() && Script->InjectPostPhysicsHandle())
						{
							Script->HandlePostPhysicsHandle(this, HandleInfo);
						}
					}
				}

				FPhysicsInterface::UpdateLinearDrive_AssumesLocked(HandleInfo->HandleData2, HandleInfo->LinConstraint);
				FPhysicsInterface::UpdateAngularDrive_AssumesLocked(HandleInfo->HandleData2, HandleInfo->AngConstraint);
			}

			if (bUseForceDrive && HandleInfo->HandleData2.IsValid() && HandleInfo->HandleData2.Constraint)
			{
				if (HandleInfo->HandleData2.IsValid() && HandleInfo->HandleData2.Constraint->IsType(Chaos::EConstraintType::JointConstraintType))
				{
					if (Chaos::FJointConstraint* Constraint = static_cast<Chaos::FJointConstraint*>(HandleInfo->HandleData2.Constraint))
					{
						Constraint->SetLinearDriveForceMode(Chaos::EJointForceMode::Force);
						Constraint->SetAngularDriveForceMode(Chaos::EJointForceMode::Force);
					}
				}
			}
		}
	});

	if (!bExecutedPhys)
	{
		return false;
	}

	HandleInfo->bInitiallySetup = true;

	if (bUseActorHandle && !rBodyInstance->OnRecalculatedMassProperties().IsBoundToObject(this))
	{
		rBodyInstance->OnRecalculatedMassProperties().AddUObject(this, &UGripMotionControllerComponent::OnGripMassUpdated);
	}

	return true;
}

bool UGripMotionControllerComponent::SetGripConstraintStiffnessAndDamping(const FBPActorGripInformation* Grip, bool bUseHybridMultiplier)
{
	if (!Grip)
		return false;

	FBPActorPhysicsHandleInformation* HandleInfo = GetPhysicsGrip(*Grip);

	if (HandleInfo)
	{
		if (HandleInfo->HandleData2.IsValid())
		{
			FPhysicsInterface::ExecuteOnUnbrokenConstraintReadWrite(HandleInfo->HandleData2, [&](const FPhysicsConstraintHandle& InUnbrokenConstraint)
				{
					bool bUseForceDrive = (Grip->AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && Grip->AdvancedGripSettings.PhysicsSettings.PhysicsConstraintType == EPhysicsGripConstraintType::ForceConstraint);

					float Stiffness = Grip->Stiffness;
					float Damping = Grip->Damping;
					float MaxForce;
					float AngularStiffness;
					float AngularDamping;
					float AngularMaxForce;

					const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

					if (Grip->AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && Grip->AdvancedGripSettings.PhysicsSettings.bUseCustomAngularValues)
					{
						AngularStiffness = Grip->AdvancedGripSettings.PhysicsSettings.AngularStiffness;
						AngularDamping = Grip->AdvancedGripSettings.PhysicsSettings.AngularDamping;
					}
					else
					{
						AngularStiffness = Stiffness * ANGULAR_STIFFNESS_MULTIPLIER; 
						AngularDamping = Damping * ANGULAR_DAMPING_MULTIPLIER; 

						if (!VRSettings.bUseChaosTranslationScalers)
						{
							AngularStiffness *= ANGULAR_STIFFNESS_MULTIPLIER_CHAOS;
							AngularDamping *= ANGULAR_DAMPING_MULTIPLIER_CHAOS;
						}
					}

					if (VRSettings.bUseChaosTranslationScalers)
					{
						Stiffness *= VRSettings.LinearDriveStiffnessScale;
						Damping *= VRSettings.LinearDriveDampingScale;
						AngularStiffness *= VRSettings.AngularDriveStiffnessScale;
						AngularDamping *= VRSettings.AngularDriveDampingScale;
					}
					else
					{
						auto CVarLinearDriveStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.LinearDriveStiffnessScale"));
						auto CVarLinearDriveDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.LinaearDriveDampingScale"));
						auto CVarAngularDriveStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.AngularDriveStiffnessScale"));
						auto CVarAngularDriveDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.AngularDriveDampingScale"));

						Stiffness *= CVarLinearDriveStiffnessScale->GetFloat();
						Damping *= CVarLinearDriveDampingScale->GetFloat();
						AngularStiffness *= CVarAngularDriveStiffnessScale->GetFloat();
						AngularDamping *= CVarAngularDriveDampingScale->GetFloat();
					}

					AngularMaxForce = (float)FMath::Clamp<double>((double)AngularStiffness * (double)Grip->AdvancedGripSettings.PhysicsSettings.AngularMaxForceCoefficient, 0, (double)MAX_FLT);
					MaxForce = (float)FMath::Clamp<double>((double)Stiffness * (double)Grip->AdvancedGripSettings.PhysicsSettings.LinearMaxForceCoefficient, 0, (double)MAX_FLT);

					if (Grip->GripCollisionType == EGripCollisionType::ManipulationGrip || Grip->GripCollisionType == EGripCollisionType::ManipulationGripWithWristTwist)
					{
						HandleInfo->LinConstraint.XDrive.Damping = Damping;
						HandleInfo->LinConstraint.XDrive.Stiffness = Stiffness;
						HandleInfo->LinConstraint.XDrive.MaxForce = MaxForce;
						HandleInfo->LinConstraint.YDrive.Damping = Damping;
						HandleInfo->LinConstraint.YDrive.Stiffness = Stiffness;
						HandleInfo->LinConstraint.YDrive.MaxForce = MaxForce;
						HandleInfo->LinConstraint.ZDrive.Damping = Damping;
						HandleInfo->LinConstraint.ZDrive.Stiffness = Stiffness;
						HandleInfo->LinConstraint.ZDrive.MaxForce = MaxForce;

						FPhysicsInterface::UpdateLinearDrive_AssumesLocked(HandleInfo->HandleData2, HandleInfo->LinConstraint);
						if (bUseForceDrive && HandleInfo->HandleData2.IsValid() && HandleInfo->HandleData2.Constraint)
						{
							if (HandleInfo->HandleData2.IsValid() && HandleInfo->HandleData2.Constraint->IsType(Chaos::EConstraintType::JointConstraintType))
							{
								if (Chaos::FJointConstraint* Constraint = static_cast<Chaos::FJointConstraint*>(HandleInfo->HandleData2.Constraint))
								{
									Constraint->SetLinearDriveForceMode(Chaos::EJointForceMode::Force);
									Constraint->SetAngularDriveForceMode(Chaos::EJointForceMode::Force);
								}
							}
						}

						if (Grip->GripCollisionType == EGripCollisionType::ManipulationGripWithWristTwist)
						{
							HandleInfo->AngConstraint.TwistDrive.Damping = AngularDamping;
							HandleInfo->AngConstraint.TwistDrive.Stiffness = AngularStiffness;
							HandleInfo->AngConstraint.TwistDrive.MaxForce = AngularMaxForce;

							FPhysicsInterface::UpdateAngularDrive_AssumesLocked(HandleInfo->HandleData2, HandleInfo->AngConstraint);
						}

						FPhysicsInterface::SetDrivePosition(HandleInfo->HandleData2, FVector::ZeroVector);
						FPhysicsInterface::SetDriveOrientation(HandleInfo->HandleData2, FQuat::Identity);
					}
					else
					{
						if (Grip->GripCollisionType == EGripCollisionType::InteractiveHybridCollisionWithPhysics)
						{

							Stiffness *= HYBRID_PHYSICS_GRIP_MULTIPLIER;
							AngularStiffness *= HYBRID_PHYSICS_GRIP_MULTIPLIER;

							AngularMaxForce = (float)FMath::Clamp<double>((double)AngularStiffness * (double)Grip->AdvancedGripSettings.PhysicsSettings.AngularMaxForceCoefficient, 0, (double)MAX_FLT);
							MaxForce = (float)FMath::Clamp<double>((double)Stiffness * (double)Grip->AdvancedGripSettings.PhysicsSettings.LinearMaxForceCoefficient, 0, (double)MAX_FLT);
						}

						HandleInfo->LinConstraint.XDrive.Damping = Damping;
						HandleInfo->LinConstraint.XDrive.Stiffness = Stiffness;
						HandleInfo->LinConstraint.XDrive.MaxForce = MaxForce;
						HandleInfo->LinConstraint.YDrive.Damping = Damping;
						HandleInfo->LinConstraint.YDrive.Stiffness = Stiffness;
						HandleInfo->LinConstraint.YDrive.MaxForce = MaxForce;
						HandleInfo->LinConstraint.ZDrive.Damping = Damping;
						HandleInfo->LinConstraint.ZDrive.Stiffness = Stiffness;
						HandleInfo->LinConstraint.ZDrive.MaxForce = MaxForce;

						FPhysicsInterface::UpdateLinearDrive_AssumesLocked(HandleInfo->HandleData2, HandleInfo->LinConstraint);
						if (bUseForceDrive && HandleInfo->HandleData2.IsValid() && HandleInfo->HandleData2.Constraint)
						{
							if (HandleInfo->HandleData2.IsValid() && HandleInfo->HandleData2.Constraint->IsType(Chaos::EConstraintType::JointConstraintType))
							{
								if (Chaos::FJointConstraint* Constraint = static_cast<Chaos::FJointConstraint*>(HandleInfo->HandleData2.Constraint))
								{
									Constraint->SetLinearDriveForceMode(Chaos::EJointForceMode::Force);
								}
							}
						}

						HandleInfo->AngConstraint.SlerpDrive.Damping = AngularDamping;
						HandleInfo->AngConstraint.SlerpDrive.Stiffness = AngularStiffness;
						HandleInfo->AngConstraint.SlerpDrive.MaxForce = AngularMaxForce;
						FPhysicsInterface::UpdateAngularDrive_AssumesLocked(HandleInfo->HandleData2, HandleInfo->AngConstraint);
						if (bUseForceDrive && HandleInfo->HandleData2.IsValid() && HandleInfo->HandleData2.Constraint)
						{
							if (HandleInfo->HandleData2.IsValid() && HandleInfo->HandleData2.Constraint->IsType(Chaos::EConstraintType::JointConstraintType))
							{
								if (Chaos::FJointConstraint* Constraint = static_cast<Chaos::FJointConstraint*>(HandleInfo->HandleData2.Constraint))
								{
									Constraint->SetAngularDriveForceMode(Chaos::EJointForceMode::Force);
								}
							}
						}
					}

				});

		}
		return true;
	}

	return false;
}

bool UGripMotionControllerComponent::GetPhysicsJointLength(const FBPActorGripInformation &GrippedActor, UPrimitiveComponent * rootComp, FVector & LocOut)
{
	if (!GrippedActor.GrippedObject)
		return false;

	FBPActorPhysicsHandleInformation * HandleInfo = GetPhysicsGrip(GrippedActor);

	if (!HandleInfo || !FPhysicsInterface::IsValid(HandleInfo->KinActorData2))
		return false;

	if (!HandleInfo->HandleData2.IsValid())
		return false;

	bool bUseComLoc = 
		(
			!HandleInfo->bSkipResettingCom &&
			(HandleInfo->bSetCOM || 
			(GrippedActor.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings && GrippedActor.AdvancedGripSettings.PhysicsSettings.PhysicsGripLocationSettings == EPhysicsGripCOMType::COM_GripAt))
		);

	FTransform tran3 = FTransform::Identity;

	FBodyInstance* rBodyInstance = rootComp->GetBodyInstance(GrippedActor.GrippedBoneName);

	if (bUseComLoc && rBodyInstance && rBodyInstance->IsValidBodyInstance())
	{
		tran3 = FTransform(rBodyInstance->GetCOMPosition());
	}
	else
	{
		FTransform rr;
		tran3 = FPhysicsInterface::GetLocalPose(HandleInfo->HandleData2, EConstraintFrame::Frame2);

		if (!rBodyInstance || !rBodyInstance->IsValidBodyInstance())
		{
			rr = rootComp->GetComponentTransform();

			rr.SetScale3D(FVector(1, 1, 1));
		}
		else
			rr = rBodyInstance->GetUnrealWorldTransform();

		tran3 = tran3 * rr;
	}

	FTransform kinPose = FTransform::Identity;
	FPhysicsCommand::ExecuteRead(HandleInfo->KinActorData2, [&](const FPhysicsActorHandle & Actor)
	{
		kinPose = FPhysicsInterface::GetGlobalPose_AssumesLocked(Actor);
	});

	LocOut = FTransform::SubtractTranslations(kinPose, tran3);

	return true;
}

void UGripMotionControllerComponent::UpdatePhysicsHandleTransform(const FBPActorGripInformation &GrippedActor, const FTransform& NewTransform)
{
	if (!GrippedActor.GrippedObject || (bConstrainToPivot && !GrippedActor.bIsLerping) || IsTravelingOrNullWorld())
		return;

	if (!NewTransform.IsValid())
	{
		UE_LOGF(LogVRMotionController, Warning, "Something went wrong, UpdatePhysicsHandeTransforms target transform contained NAN!.");
		return;
	}

	FBPActorPhysicsHandleInformation * HandleInfo = GetPhysicsGrip(GrippedActor);

	if (!HandleInfo || !FPhysicsInterface::IsValid(HandleInfo->KinActorData2))
		return;

	if (!HandleInfo->LastPhysicsTransform.EqualsNoScale(NewTransform))
	{
		HandleInfo->LastPhysicsTransform = NewTransform;
		HandleInfo->LastPhysicsTransform.SetScale3D(FVector(1.0f));
		FPhysicsActorHandle ActorHandle = HandleInfo->KinActorData2;
		FTransform newTrans = HandleInfo->COMPosition * (HandleInfo->RootBoneRotation * HandleInfo->LastPhysicsTransform);
		FPhysicsCommand::ExecuteWrite(ActorHandle, [&](const FPhysicsActorHandle & Actor)
		{
			FPhysicsInterface::SetKinematicTarget_AssumesLocked(Actor, newTrans);
		});
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (GripMotionControllerCvars::DrawDebugGripCOM)
	{
		UPrimitiveComponent* me = Cast<UPrimitiveComponent>(GrippedActor.GripTargetType == EGripTargetType::ActorGrip ? GrippedActor.GetGrippedActor()->GetRootComponent() : GrippedActor.GetGrippedComponent());
		FVector curCOMPosition = me->GetBodyInstance(GrippedActor.GrippedBoneName)->GetCOMPosition();
		DrawDebugSphere(GetWorld(), curCOMPosition, 4, 32, FColor::Red, false);
		FTransform TargetTransform = (HandleInfo->COMPosition * (HandleInfo->RootBoneRotation * HandleInfo->LastPhysicsTransform));
		DrawDebugSphere(GetWorld(), TargetTransform.GetLocation(), 4, 32, FColor::Cyan, false);
		DrawDebugLine(GetWorld(), TargetTransform.GetTranslation(), TargetTransform.GetTranslation() + (TargetTransform.GetRotation().GetForwardVector() * 20.f), FColor::Red);
		DrawDebugLine(GetWorld(), TargetTransform.GetTranslation(), TargetTransform.GetTranslation() + (TargetTransform.GetRotation().GetRightVector() * 20.f), FColor::Green);
		DrawDebugLine(GetWorld(), TargetTransform.GetTranslation(), TargetTransform.GetTranslation() + (TargetTransform.GetRotation().GetUpVector() * 20.f), FColor::Blue);
	}
#endif

}

static void PullBackHitComp(FHitResult& Hit, const FVector& Start, const FVector& End, const float Dist)
{
	const float DesiredTimeBack = FMath::Clamp(0.1f, 0.1f / Dist, 1.f / Dist) + 0.001f;
	Hit.Time = FMath::Clamp(Hit.Time - DesiredTimeBack, 0.f, 1.f);
}

bool UGripMotionControllerComponent::CheckComponentWithSweep(UPrimitiveComponent * ComponentToCheck, FVector Move, FRotator newOrientation, bool bSkipSimulatingComponents)
{
	TArray<FHitResult> Hits;

	FHitResult BlockingHit(NoInit);
	BlockingHit.bBlockingHit = false;
	BlockingHit.Time = 1.f;
	bool bFilledHitResult = false;
	bool bMoved = false;
	bool bIncludesOverlapsAtEnd = false;
	bool bRotationOnly = false;

	UPrimitiveComponent *root = ComponentToCheck;

	if (!root || !root->IsQueryCollisionEnabled())
		return false;

	FVector start(root->GetComponentLocation());

	const bool bCollisionEnabled = root->IsQueryCollisionEnabled();

	if (bCollisionEnabled)
	{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (!root->IsRegistered())
		{
			UE_LOGF(LogVRMotionController, Warning, "MovedComponent %ls not initialized in grip motion controller", *root->GetFullName());
		}
#endif

		UWorld* const MyWorld = GetWorld();
		FComponentQueryParams Params(TEXT("sweep_params"), root->GetOwner());

		FCollisionResponseParams ResponseParam;
		root->InitSweepCollisionParams(Params, ResponseParam);

		FVector end = start + Move;
		bool const bHadBlockingHit = MyWorld->ComponentSweepMulti(Hits, root, start, end, newOrientation.Quaternion(), Params);

		if (Hits.Num() > 0)
		{
			const float DeltaSize = FVector::Dist(start, end);
			for (int32 HitIdx = 0; HitIdx < Hits.Num(); HitIdx++)
			{
				PullBackHitComp(Hits[HitIdx], start, end, DeltaSize);
			}
		}

		if (bHadBlockingHit)
		{
			int32 BlockingHitIndex = INDEX_NONE;
			float BlockingHitNormalDotDelta = UE_BIG_NUMBER;
			for (int32 HitIdx = 0; HitIdx < Hits.Num(); HitIdx++)
			{
				const FHitResult& TestHit = Hits[HitIdx];

				if (TestHit.GetActor() == this->GetOwner() || (bSkipSimulatingComponents && TestHit.Component->IsSimulatingPhysics()))
				{
					if (Hits.Num() == 1)
					{

						return false;
					}
					else
						continue;
				}

				if (TestHit.bBlockingHit && TestHit.IsValidBlockingHit())
				{
					if (TestHit.Time == 0.f)
					{

						const float NormalDotDelta = (TestHit.ImpactNormal | Move);
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

			if (BlockingHitIndex >= 0)
			{
				BlockingHit = Hits[BlockingHitIndex];
				bFilledHitResult = true;
			}
		}
	}

	if (BlockingHit.bBlockingHit && IsValid(root))
	{
		check(bFilledHitResult);
		if (root->IsDeferringMovementUpdates())
		{
			FScopedMovementUpdate* ScopedUpdate = root->GetCurrentScopedMovement();
			ScopedUpdate->AppendBlockingHitAfterMove(BlockingHit);
		}
		else
		{

			if(root->GetOwner())
				root->DispatchBlockingHit(*root->GetOwner(), BlockingHit);
		}

		return true;
	}

	return false;
}

bool UGripMotionControllerComponent::HasTrackingParameters()
{
	return  bScaleTracking || bLeashToHMD || bLimitMinHeight || bLimitMaxHeight || (AttachChar && !AttachChar->bRetainRoomscale);
}

void UGripMotionControllerComponent::ApplyTrackingParameters(FVector& OriginalPosition, bool bIsInGameThread, bool bApplyZeroing)
{
	if (bScaleTracking)
	{
		OriginalPosition *= TrackingScaler;
	}

	if (bLimitMinHeight)
	{
		OriginalPosition.Z = FMath::Max(OriginalPosition.Z, MinimumHeight);
	}

	if (bLimitMaxHeight)
	{
		OriginalPosition.Z = FMath::Min(OriginalPosition.Z, MaximumHeight);
	}

	if (bApplyZeroing && ( bLeashToHMD || (AttachChar && !AttachChar->bRetainRoomscale)))
	{
		if (bIsInGameThread)
		{
			if (IsLocallyControlled() && GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowedForWorld(*GetWorld()))
			{
				FQuat curRot;
				FVector curLoc;
				if (GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, curRot, curLoc))
				{

					LastLocationForLateUpdate = curLoc;

					if (IsValid(AttachChar) && !AttachChar->bRetainRoomscale)
					{
						if (AttachChar->VRMovementReference && AttachChar->VRMovementReference->GetReplicatedMovementMode() == EVRConjoinedMovementModes::C_VRMOVE_Seated)
						{

						}
						else
						{
							FRotator StoredCameraRotOffset = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(curRot.Rotator());
							LastLocationForLateUpdate += StoredCameraRotOffset.RotateVector(FVector(AttachChar->VRRootReference->VRCapsuleOffset.X, AttachChar->VRRootReference->VRCapsuleOffset.Y, 0.0f));
						}
					}
				}
			}
			else
			{
				if (IsValid(AttachChar) && AttachChar->VRReplicatedCamera)
				{

					LastLocationForLateUpdate = AttachChar->VRReplicatedCamera->ReplicatedCameraTransform.Position; 

					if (!AttachChar->bRetainRoomscale && IsLocallyControlled())
					{
						if (AttachChar->VRMovementReference && AttachChar->VRMovementReference->GetReplicatedMovementMode() == EVRConjoinedMovementModes::C_VRMOVE_Seated)
						{

						}
						else
						{
							FRotator StoredCameraRotOffset = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(AttachChar->VRReplicatedCamera->GetRelativeRotation());
							LastLocationForLateUpdate += StoredCameraRotOffset.RotateVector(FVector(AttachChar->VRRootReference->VRCapsuleOffset.X, AttachChar->VRRootReference->VRCapsuleOffset.Y, 0.0f));
						}
					}
				}
			}
		}

		FVector CorrectLastLocation = bIsInGameThread ? LastLocationForLateUpdate : LateUpdateParams.GripRenderThreadLastLocationForLateUpdate;

		if (bLeashToHMD)
		{
			FVector DifferenceVec = OriginalPosition - CorrectLastLocation;

			if (DifferenceVec.SizeSquared() > FMath::Square(LeashRange))
			{
				OriginalPosition = CorrectLastLocation + (DifferenceVec.GetSafeNormal() * LeashRange);
			}
		}

		if ( (AttachChar && !AttachChar->bRetainRoomscale))
		{
			OriginalPosition -= FVector(CorrectLastLocation.X, CorrectLastLocation.Y, 0.0f);
		}
	}
}

void UGripMotionControllerComponent::OnModularFeatureUnregistered(const FName& Type, class IModularFeature* ModularFeature)
{
	FScopeLock Lock(&PolledMotionControllerMutex);

	if (ModularFeature == PolledMotionController_GameThread)
	{
		PolledMotionController_GameThread = nullptr;
	}
	if (ModularFeature == PolledMotionController_RenderThread)
	{
		PolledMotionController_RenderThread = nullptr;
	}
}

bool UGripMotionControllerComponent::GripPollControllerState_GameThread(FVector& Position, FRotator& Orientation, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale)
{

	bool bIsInGameThread = true;

	if (bHasAuthority)
	{
		{
			FScopeLock Lock(&PolledMotionControllerMutex);
			PolledMotionController_GameThread = nullptr;
			bPolledHMD_GameThread = false;
		}

		if (MotionSource == IMotionController::HMDSourceId || MotionSource == IMotionController::HeadSourceId)
		{
			IXRTrackingSystem* TrackingSys = GEngine->XRSystem.Get();
			if (TrackingSys)
			{
				FQuat OrientationQuat;
				if (TrackingSys->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, OrientationQuat, Position))
				{
					Orientation = OrientationQuat.Rotator();
					{
						FScopeLock Lock(&PolledMotionControllerMutex);
						bPolledHMD_GameThread = true;  
					}
					return true;
				}
			}
		}
		else
		{

			TArray<IMotionController*> MotionControllers;
			MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());
			for (auto MotionController : MotionControllers)
			{
				if (MotionController == nullptr)
				{
					continue;
				}

				if (bIsInGameThread)
				{
					EControllerHand HandType;
					GetHandType(HandType);
					FName GripSource = (HandType == EControllerHand::Left) ? FName("LeftGrip") : FName("RightGrip");

					CurrentTrackingStatus = MotionController->GetControllerTrackingStatus(PlayerIndex, MotionSource);
					if (!bIgnoreTrackingStatus && CurrentTrackingStatus == ETrackingStatus::NotTracked)
						continue;
				}

				if (MotionController->GetControllerOrientationAndPosition(PlayerIndex, MotionSource, Orientation, Position, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale))
				{

					if (HasTrackingParameters())
					{
						ApplyTrackingParameters(Position, bIsInGameThread);
					}

					if (bOffsetByControllerProfile)
					{
						FTransform FinalControllerTransform(Orientation, Position);
						if (bIsInGameThread)
						{
							FinalControllerTransform = CurrentControllerProfileTransform * FinalControllerTransform;
						}
						else
						{
							FinalControllerTransform = LateUpdateParams.GripRenderThreadProfileTransform * FinalControllerTransform;
						}

						Orientation = FinalControllerTransform.Rotator();
						Position = FinalControllerTransform.GetTranslation();
					}

					InUseMotionController = MotionController;
					OnMotionControllerUpdated();
					InUseMotionController = nullptr;

					{
						FScopeLock Lock(&PolledMotionControllerMutex);
						PolledMotionController_GameThread = MotionController;  
					}
					return true;
				}

			}
		}
	}
	return false;
}

bool UGripMotionControllerComponent::GripPollControllerState_RenderThread(FVector& Position, FRotator& Orientation, float WorldToMetersScale)
{
	check(IsInRenderingThread());
	bool bIsInGameThread = false;

	if (PolledMotionController_RenderThread)
	{
		EControllerHand HandType;
		GetHandType(HandType);
		FName GripSource = (HandType == EControllerHand::Left) ? FName("LeftGrip") : FName("RightGrip");

		CurrentTrackingStatus = PolledMotionController_RenderThread->GetControllerTrackingStatus(PlayerIndex, GripSource);
		if (PolledMotionController_RenderThread->GetControllerOrientationAndPosition(PlayerIndex, MotionSource, Orientation, Position, WorldToMetersScale))
		{
			if (HasTrackingParameters())
			{
				ApplyTrackingParameters(Position, bIsInGameThread);
			}

			if (bOffsetByControllerProfile)
			{
				FTransform FinalControllerTransform(Orientation, Position);
				if (bIsInGameThread)
				{
					FinalControllerTransform = CurrentControllerProfileTransform * FinalControllerTransform;
				}
				else
				{
					FinalControllerTransform = LateUpdateParams.GripRenderThreadProfileTransform * FinalControllerTransform;
				}

				Orientation = FinalControllerTransform.Rotator();
				Position = FinalControllerTransform.GetTranslation();
			}
			return true;
		}
	}

	if (bPolledHMD_RenderThread)
	{
		IXRTrackingSystem* TrackingSys = GEngine->XRSystem.Get();
		if (TrackingSys)
		{
			FQuat OrientationQuat;
			if (TrackingSys->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, OrientationQuat, Position))
			{
				Orientation = OrientationQuat.Rotator();
				return true;
			}
		}
	}

	return false;
}

UGripMotionControllerComponent::FGripViewExtension::FGripViewExtension(const FAutoRegister& AutoRegister, UGripMotionControllerComponent* InMotionControllerComponent)
	: FSceneViewExtensionBase(AutoRegister)
	, MotionControllerComponent(InMotionControllerComponent)
{
	FSceneViewExtensionIsActiveFunctor IsActiveFunc;
	IsActiveFunc.IsActiveFunction = [this](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
	{
		check(IsInGameThread());

		static const auto CVarEnableMotionControllerLateUpdate = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.EnableMotionControllerLateUpdate"));
		return MotionControllerComponent && !MotionControllerComponent->bDisableLowLatencyUpdate && MotionControllerComponent->bTracked && CVarEnableMotionControllerLateUpdate->GetValueOnGameThread();
	};

	IsActiveThisFrameFunctions.Add(IsActiveFunc);
}

void UGripMotionControllerComponent::FGripViewExtension::PreRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily)
{
	FTransform OldTransform;
	FTransform NewTransform;

	{
		FScopeLock ScopeLock(&CritSect);

		if (!MotionControllerComponent)
			return;

		{
			FScopeLock Lock(&MotionControllerComponent->PolledMotionControllerMutex);
			MotionControllerComponent->PolledMotionController_RenderThread = MotionControllerComponent->PolledMotionController_GameThread;
		}

		float WorldToMetersScale = -1.0f;
		for (const FSceneView* SceneView : InViewFamily.Views)
		{
			if (SceneView && SceneView->PlayerIndex == MotionControllerComponent->PlayerIndex)
			{
				WorldToMetersScale = SceneView->WorldToMetersScale;
				break;
			}
		}

		if (WorldToMetersScale < 0.0f)
		{
			check(InViewFamily.Views.Num() > 0);
			WorldToMetersScale = InViewFamily.Views[0]->WorldToMetersScale;
		}

		FVector Position = MotionControllerComponent->LateUpdateParams.GripRenderThreadRelativeTransform.GetTranslation();
		FRotator Orientation = MotionControllerComponent->LateUpdateParams.GripRenderThreadRelativeTransform.GetRotation().Rotator();

		if (!MotionControllerComponent->GripPollControllerState_RenderThread(Position, Orientation, WorldToMetersScale))
		{
			return;
		}

		if (MotionControllerComponent->LateUpdateParams.bRenderSmoothHandTracking)
		{
			FTransform CalcedTransform = FTransform(Orientation, Position, MotionControllerComponent->LateUpdateParams.GripRenderThreadComponentScale);

			if (MotionControllerComponent->LateUpdateParams.bRenderSmoothWithEuroLowPassFunction)
			{
				CalcedTransform = MotionControllerComponent->LateUpdateParams.RenderEuroSmoothingParams.RunFilterSmoothing(CalcedTransform, MotionControllerComponent->LateUpdateParams.RenderLastDeltaTime);

			}
			else
			{
				if (MotionControllerComponent->LateUpdateParams.RenderSmoothingSpeed <= 0.f || MotionControllerComponent->LateUpdateParams.RenderLastSmoothRelativeTransform.Equals(FTransform::Identity))
				{

				}
				else
				{
					const float Alpha = FMath::Clamp(MotionControllerComponent->LateUpdateParams.RenderLastDeltaTime * MotionControllerComponent->LateUpdateParams.RenderSmoothingSpeed, 0.f, 1.f);
					MotionControllerComponent->LateUpdateParams.RenderLastSmoothRelativeTransform.Blend(MotionControllerComponent->LateUpdateParams.RenderLastSmoothRelativeTransform, CalcedTransform, Alpha);
					CalcedTransform = MotionControllerComponent->LateUpdateParams.RenderLastSmoothRelativeTransform;

				}
			}

			NewTransform = CalcedTransform;
		}
		else
		{
			NewTransform = FTransform(Orientation, Position, MotionControllerComponent->LateUpdateParams.GripRenderThreadComponentScale);
		}

		OldTransform = MotionControllerComponent->LateUpdateParams.GripRenderThreadRelativeTransform;

		MotionControllerComponent->LateUpdateParams.GripRenderThreadRelativeTransform = NewTransform;
	} 

	LateUpdate.Apply_RenderThread(InViewFamily.Scene, OldTransform, NewTransform);

}

bool UGripMotionControllerComponent::K2_GetFirstActiveGrip(FBPActorGripInformation& FirstActiveGrip)
{
	FBPActorGripInformation* FirstGrip = GetFirstActiveGrip();

	if (FirstGrip)
	{
		FirstActiveGrip = *FirstGrip;
		return true;
	}

	return false;
}

FBPActorGripInformation* UGripMotionControllerComponent::GetFirstActiveGrip()
{
	for (FBPActorGripInformation& Grip : GrippedObjects)
	{
		if (Grip.IsValid() && !Grip.bIsPaused)
		{
			return &Grip;
		}
	}

	for (FBPActorGripInformation& LocalGrip : LocallyGrippedObjects)
	{
		if (LocalGrip.IsValid() && !LocalGrip.bIsPaused)
		{
			return &LocalGrip;
		}
	}

	return nullptr;
}

void UGripMotionControllerComponent::GetAllGrips(TArray<FBPActorGripInformation> &GripArray)
{
	GripArray.Append(GrippedObjects);
	GripArray.Append(LocallyGrippedObjects);
}

void UGripMotionControllerComponent::GetGrippedObjects(TArray<UObject*> &GrippedObjectsArray)
{
	for (int i = 0; i < GrippedObjects.Num(); ++i)
	{
		if (GrippedObjects[i].GrippedObject)
			GrippedObjectsArray.Add(GrippedObjects[i].GrippedObject);
	}

	for (int i = 0; i < LocallyGrippedObjects.Num(); ++i)
	{
		if (LocallyGrippedObjects[i].GrippedObject)
			GrippedObjectsArray.Add(LocallyGrippedObjects[i].GrippedObject);
	}

}

void UGripMotionControllerComponent::GetGrippedActors(TArray<AActor*> &GrippedObjectsArray)
{
	for (int i = 0; i < GrippedObjects.Num(); ++i)
	{
		if(GrippedObjects[i].GetGrippedActor())
			GrippedObjectsArray.Add(GrippedObjects[i].GetGrippedActor());
	}

	for (int i = 0; i < LocallyGrippedObjects.Num(); ++i)
	{
		if (LocallyGrippedObjects[i].GetGrippedActor())
			GrippedObjectsArray.Add(LocallyGrippedObjects[i].GetGrippedActor());
	}

}

void UGripMotionControllerComponent::GetGrippedComponents(TArray<UPrimitiveComponent*> &GrippedComponentsArray)
{
	for (int i = 0; i < GrippedObjects.Num(); ++i)
	{
		if (GrippedObjects[i].GetGrippedComponent())
			GrippedComponentsArray.Add(GrippedObjects[i].GetGrippedComponent());
	}

	for (int i = 0; i < LocallyGrippedObjects.Num(); ++i)
	{
		if (LocallyGrippedObjects[i].GetGrippedComponent())
			GrippedComponentsArray.Add(LocallyGrippedObjects[i].GetGrippedComponent());
	}
}

void UGripMotionControllerComponent::Client_NotifyInvalidLocalGrip_Implementation(UObject * LocallyGrippedObject, uint8 GripID, bool bWasAGripConflict)
{
	if (GripID != INVALID_VRGRIP_ID)
	{
		if (FBPActorGripInformation* GripInfo = GetGripPtrByID(GripID))
		{
			DropObjectByInterface(nullptr, GripID);

			if (LocallyGrippedObject && bWasAGripConflict)
			{
				OnClientAuthGripConflict.Broadcast(LocallyGrippedObject, ClientAuthConflictResolutionMethod);
			}

			return;
		}		
	}

	FBPActorGripInformation FoundGrip;
	EBPVRResultSwitch Result;

	GetGripByObject(FoundGrip, LocallyGrippedObject, Result);

	if (Result == EBPVRResultSwitch::OnFailed)
		return;

	DropObjectByInterface(nullptr, FoundGrip.GripID);

	if (LocallyGrippedObject && bWasAGripConflict)
	{
		OnClientAuthGripConflict.Broadcast(LocallyGrippedObject, ClientAuthConflictResolutionMethod);
	}
}

bool UGripMotionControllerComponent::Server_NotifyHandledTransaction_Validate(uint8 GripID)
{
	return true;
}

void UGripMotionControllerComponent::Server_NotifyHandledTransaction_Implementation(uint8 GripID)
{
	for (int i = LocalTransactionBuffer.Num() - 1; i >= 0; i--)
	{
		if(LocalTransactionBuffer[i].GripID == GripID)
			LocalTransactionBuffer.RemoveAt(i);
	}

#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, LocalTransactionBuffer, this);
#endif
}

bool UGripMotionControllerComponent::Server_NotifyLocalGripAddedOrChanged_Validate(const FBPActorGripInformation & newGrip)
{
	return true;
}

void UGripMotionControllerComponent::Server_NotifyLocalGripAddedOrChanged_Implementation(const FBPActorGripInformation & newGrip)
{
	DIRTY_LOCALLY_GRIPPED_OBJECTS();

	if (!newGrip.GrippedObject || newGrip.GripMovementReplicationSetting != EGripMovementReplicationSettings::ClientSide_Authoritive)
	{
		Client_NotifyInvalidLocalGrip(newGrip.GrippedObject, newGrip.GripID);
		return;
	}

	if (!LocallyGrippedObjects.Contains(newGrip))
	{

		bool bImplementsInterface = newGrip.GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass());

		TArray<FBPGripPair> HoldingControllers;
		bool bIsHeld = false;
		if (bImplementsInterface)
		{
			IVRGripInterface::Execute_IsHeld(newGrip.GrippedObject, HoldingControllers, bIsHeld);

			if (bIsHeld && ClientAuthConflictResolutionMethod != EVRClientAuthConflictResolutionMode::VRGRIP_CONFLICT_None)
			{

				if (!IVRGripInterface::Execute_AllowsMultipleGrips(newGrip.GrippedObject))
				{

					for (FBPGripPair& GripPair : HoldingControllers)
					{
						if (!GripPair.HoldingController || GripPair.GripID == INVALID_VRGRIP_ID)
						{
							continue;
						}

						if (GripPair.HoldingController->GetOwner()->GetNetOwner() != this->GetOwner()->GetNetOwner())
						{
							switch (ClientAuthConflictResolutionMethod)
							{
							case EVRClientAuthConflictResolutionMode::VRGRIP_CONFLICT_First:
							{

								Client_NotifyInvalidLocalGrip(newGrip.GrippedObject, newGrip.GripID, true);

								OnClientAuthGripConflict.Broadcast(newGrip.GrippedObject, ClientAuthConflictResolutionMethod);
								return;
							}break;
							case EVRClientAuthConflictResolutionMode::VRGRIP_CONFLICT_Last:
							{				

								GripPair.HoldingController->DropObjectByInterface(newGrip.GrippedObject, GripPair.GripID);
								GripPair.HoldingController->Client_NotifyInvalidLocalGrip(newGrip.GrippedObject, GripPair.GripID, true);
								OnClientAuthGripConflict.Broadcast(newGrip.GrippedObject, ClientAuthConflictResolutionMethod);
							}break;
							case EVRClientAuthConflictResolutionMode::VRGRIP_CONFLICT_DropAll:
							{

								Client_NotifyInvalidLocalGrip(newGrip.GrippedObject, newGrip.GripID, true);
								GripPair.HoldingController->DropObjectByInterface(newGrip.GrippedObject, GripPair.GripID);
								GripPair.HoldingController->Client_NotifyInvalidLocalGrip(newGrip.GrippedObject, GripPair.GripID, true);
								OnClientAuthGripConflict.Broadcast(newGrip.GrippedObject, ClientAuthConflictResolutionMethod);
								return;
							}break;
							case EVRClientAuthConflictResolutionMode::VRGRIP_CONFLICT_None:
							default:
							{
								OnClientAuthGripConflict.Broadcast(newGrip.GrippedObject, ClientAuthConflictResolutionMethod);
							}break;

							}
						}
					}
				}
			}
		}

		UPrimitiveComponent* PrimComp = nullptr;
		AActor* pActor = nullptr;

		PrimComp = newGrip.GetGrippedComponent();
		pActor = newGrip.GetGrippedActor();

		if (!PrimComp && pActor)
		{
			PrimComp = Cast<UPrimitiveComponent>(pActor->GetRootComponent());
		}
		else if (!pActor && PrimComp)
		{
			pActor = PrimComp->GetOwner();
		}

		if (!PrimComp || !pActor)
		{
			Client_NotifyInvalidLocalGrip(newGrip.GrippedObject, newGrip.GripID);
			return;
		}

		bool bHadOriginalSettings = false;
		bool bOriginalGravity = false;
		bool bOriginalReplication = false;

		if (bImplementsInterface)
		{

			if (bIsHeld)
			{

				if (HoldingControllers.Num() > 0 && HoldingControllers[0].HoldingController != nullptr)
				{
					FBPActorGripInformation* gripInfo = HoldingControllers[0].HoldingController->GetGripPtrByID(HoldingControllers[0].GripID);

					if (gripInfo != nullptr)
					{
						bHadOriginalSettings = true;
						bOriginalGravity = gripInfo->bOriginalGravity;
						bOriginalReplication = gripInfo->bOriginalReplicatesMovement;
					}
				}
			}
		}

		int32 NewIndex = LocallyGrippedObjects.Add(newGrip);
		DIRTY_LOCALLY_GRIPPED_OBJECTS();

		if (NewIndex != INDEX_NONE && LocallyGrippedObjects.Num() > 0)
		{
			if (bHadOriginalSettings)
			{
				LocallyGrippedObjects[NewIndex].bOriginalReplicatesMovement = bOriginalReplication;
				LocallyGrippedObjects[NewIndex].bOriginalGravity = bOriginalGravity;
			}
			else
			{
				LocallyGrippedObjects[NewIndex].bOriginalReplicatesMovement = pActor->IsReplicatingMovement();
				LocallyGrippedObjects[NewIndex].bOriginalGravity = PrimComp->IsGravityEnabled();
			}

			HandleGripReplication(LocallyGrippedObjects[NewIndex]);
		}

	}
	else
	{
		int32 IndexFound = INDEX_NONE;
		if (LocallyGrippedObjects.Find(newGrip, IndexFound) && IndexFound != INDEX_NONE)
		{
			FBPActorGripInformation OriginalGrip = LocallyGrippedObjects[IndexFound];
			LocallyGrippedObjects[IndexFound].RepCopy(newGrip);
			HandleGripReplication(LocallyGrippedObjects[IndexFound], &OriginalGrip);
		}
	}

}

bool UGripMotionControllerComponent::Server_NotifyLocalGripRemoved_Validate(uint8 GripID, const FTransform_NetQuantize &TransformAtDrop, FVector_NetQuantize100 OptAngularVelocity, FVector_NetQuantize100 OptLinearVelocity)
{
	return true;
}

void UGripMotionControllerComponent::Server_NotifyLocalGripRemoved_Implementation(uint8 GripID, const FTransform_NetQuantize &TransformAtDrop, FVector_NetQuantize100 OptAngularVelocity, FVector_NetQuantize100 OptLinearVelocity)
{
	DIRTY_LOCALLY_GRIPPED_OBJECTS();

	FBPActorGripInformation FoundGrip;
	EBPVRResultSwitch Result;
	GetGripByID(FoundGrip, GripID, Result);

	if (Result == EBPVRResultSwitch::OnFailed)
		return;

	if (FoundGrip.GripCollisionType != EGripCollisionType::EventsOnly)
	{
		switch (FoundGrip.GripTargetType)
		{
		case EGripTargetType::ActorGrip:
		{
			if (AActor* DroppingActor = FoundGrip.GetGrippedActor())
			{
				if (IsValid(DroppingActor) && TransformAtDrop.IsValid())
				{
					bool bSkipSetTransform = false;
					if (DroppingActor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
					{
						FBPAdvGripSettings AdvSettings = IVRGripInterface::Execute_AdvancedGripSettings(DroppingActor);
						bSkipSetTransform = AdvSettings.bDisallowSettingPositionOnClientAuthDrop;
					}

					if (!bSkipSetTransform)
					{
						DroppingActor->SetActorTransform(TransformAtDrop, false, nullptr, ETeleportType::None);
					}
				}
			}
		}break;
		case EGripTargetType::ComponentGrip:
		{
			if (UPrimitiveComponent* DroppingComp = FoundGrip.GetGrippedComponent())
			{
				if (IsValid(DroppingComp) && TransformAtDrop.IsValid())
				{
					bool bSkipSetTransform = false;
					if (DroppingComp->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
					{
						FBPAdvGripSettings AdvSettings = IVRGripInterface::Execute_AdvancedGripSettings(DroppingComp);
						bSkipSetTransform = AdvSettings.bDisallowSettingPositionOnClientAuthDrop;
					}

					if (!bSkipSetTransform)
					{
						DroppingComp->SetWorldTransform(TransformAtDrop, false, nullptr, ETeleportType::None);
					}
				}
			}
		}break;
		default:break;
		}
	}

	if (!DropObjectByInterface_Implementation(nullptr, FoundGrip.GripID, OptAngularVelocity, OptLinearVelocity, true))
	{
		DropGrip_Implementation(FoundGrip, false, OptAngularVelocity, OptLinearVelocity,true);
	}
}

bool UGripMotionControllerComponent::Server_NotifySecondaryAttachmentChanged_Validate(
	uint8 GripID,
	const FBPSecondaryGripInfo& SecondaryGripInfo)
{
	return true;
}

void UGripMotionControllerComponent::Server_NotifySecondaryAttachmentChanged_Implementation(
	uint8 GripID,
	const FBPSecondaryGripInfo& SecondaryGripInfo)
{
	DIRTY_LOCALLY_GRIPPED_OBJECTS();

	FBPActorGripInformation * GripInfo = LocallyGrippedObjects.FindByKey(GripID);
	if (GripInfo != nullptr)
	{
		FBPActorGripInformation OriginalGrip = *GripInfo;

		GripInfo->SecondaryGripInfo.RepCopy(SecondaryGripInfo);

		HandleGripReplication(*GripInfo, &OriginalGrip);
	}

}

bool UGripMotionControllerComponent::Server_NotifySecondaryAttachmentChanged_Retain_Validate(
	uint8 GripID,
	const FBPSecondaryGripInfo& SecondaryGripInfo, const FTransform_NetQuantize & NewRelativeTransform)
{
	return true;
}

void UGripMotionControllerComponent::Server_NotifySecondaryAttachmentChanged_Retain_Implementation(
	uint8 GripID,
	const FBPSecondaryGripInfo& SecondaryGripInfo, const FTransform_NetQuantize & NewRelativeTransform)
{
	DIRTY_LOCALLY_GRIPPED_OBJECTS();

	FBPActorGripInformation * GripInfo = LocallyGrippedObjects.FindByKey(GripID);
	if (GripInfo != nullptr)
	{
		FBPActorGripInformation OriginalGrip = *GripInfo;

		GripInfo->SecondaryGripInfo.RepCopy(SecondaryGripInfo);
		GripInfo->RelativeTransform = NewRelativeTransform;

		HandleGripReplication(*GripInfo, &OriginalGrip);
	}

}
void UGripMotionControllerComponent::GetControllerDeviceID(FXRDeviceId & DeviceID, EBPVRResultSwitch &Result, bool bCheckOpenVROnly)
{
	EControllerHand ControllerHandIndex;
	if (!IMotionController::GetHandEnumForSourceName(MotionSource, ControllerHandIndex))
	{
		Result = EBPVRResultSwitch::OnFailed;
		return;
	}

	TArray<IXRSystemAssets*> XRAssetSystems = IModularFeatures::Get().GetModularFeatureImplementations<IXRSystemAssets>(IXRSystemAssets::GetModularFeatureName());
	for (IXRSystemAssets* AssetSys : XRAssetSystems)
	{
		if (bCheckOpenVROnly && !AssetSys->GetSystemName().IsEqual(FName(TEXT("SteamVR"))))
			continue;

		const int32 XRID = AssetSys->GetDeviceId(ControllerHandIndex);

		if (XRID != INDEX_NONE)
		{
			DeviceID = FXRDeviceId(AssetSys, XRID);
			Result = EBPVRResultSwitch::OnSucceeded;
			return;
		}
	}

	DeviceID = FXRDeviceId();
	Result = EBPVRResultSwitch::OnFailed;
	return;
}

FExpandedLateUpdateManager::FExpandedLateUpdateManager()
	: LateUpdateGameWriteIndex(0)
	, LateUpdateRenderReadIndex(0)
{
}

void FExpandedLateUpdateManager::Setup(const FTransform& ParentToWorld, UGripMotionControllerComponent* Component, bool bSkipLateUpdate)
{
	if (!Component)
		return;

	check(IsInGameThread());

	UpdateStates[LateUpdateGameWriteIndex].Primitives.Reset();
	UpdateStates[LateUpdateGameWriteIndex].ParentToWorld = ParentToWorld;

	for (UPrimitiveComponent* primComp : Component->AdditionalLateUpdateComponents)
	{
		if (primComp)
			GatherLateUpdatePrimitives(primComp);
	}

	ProcessGripArrayLateUpdatePrimitives(Component, Component->LocallyGrippedObjects);
	ProcessGripArrayLateUpdatePrimitives(Component, Component->GrippedObjects);

	GatherLateUpdatePrimitives(Component);

	UpdateStates[LateUpdateGameWriteIndex].bSkip = bSkipLateUpdate;
	++UpdateStates[LateUpdateGameWriteIndex].TrackingNumber;

	int32 NextFrameRenderReadIndex = LateUpdateGameWriteIndex;
	LateUpdateGameWriteIndex = 1 - LateUpdateGameWriteIndex;

	ENQUEUE_RENDER_COMMAND(UpdateLateUpdateRenderReadIndexCommand)(
		[NextFrameRenderReadIndex, this](FRHICommandListImmediate& RHICmdList)
		{
			LateUpdateRenderReadIndex = NextFrameRenderReadIndex;
		});

}

void FExpandedLateUpdateManager::Apply_RenderThread(FSceneInterface* Scene, const FTransform& OldRelativeTransform, const FTransform& NewRelativeTransform)
{
	FRHICommandListBase& RHICmdList = FRHICommandListImmediate::Get();

	check(IsInRenderingThread());

	if (!UpdateStates[LateUpdateRenderReadIndex].Primitives.Num() || UpdateStates[LateUpdateRenderReadIndex].bSkip)
	{
		return;
	}

	const FTransform OldCameraTransform = OldRelativeTransform * UpdateStates[LateUpdateRenderReadIndex].ParentToWorld;
	const FTransform NewCameraTransform = NewRelativeTransform * UpdateStates[LateUpdateRenderReadIndex].ParentToWorld;
	const FMatrix LateUpdateTransform = (OldCameraTransform.Inverse() * NewCameraTransform).ToMatrixWithScale();

	bool bIndicesHaveChanged = false;

	for (auto& PrimitivePair : UpdateStates[LateUpdateRenderReadIndex].Primitives)
	{
		if (PrimitivePair.Value == -1)
			continue;

		FPrimitiveSceneInfo* RetrievedSceneInfo = Scene->GetPrimitiveSceneInfo(PrimitivePair.Value);
		FPrimitiveSceneInfo* CachedSceneInfo = PrimitivePair.Key;

		if (CachedSceneInfo != RetrievedSceneInfo)
		{
			bIndicesHaveChanged = true;
			break; 
		}
		else if (CachedSceneInfo->Proxy)
		{
			CachedSceneInfo->Proxy->ApplyLateUpdateTransform(RHICmdList, LateUpdateTransform);
			PrimitivePair.Value = -1; 

		}
	}

	if (bIndicesHaveChanged)
	{
		int32 Index = 0;
		FPrimitiveSceneInfo* RetrievedSceneInfo = Scene->GetPrimitiveSceneInfo(Index++);
		while (RetrievedSceneInfo)
		{

			if (RetrievedSceneInfo->Proxy && UpdateStates[LateUpdateRenderReadIndex].Primitives.Contains(RetrievedSceneInfo) && UpdateStates[LateUpdateRenderReadIndex].Primitives[RetrievedSceneInfo] >= 0)
			{
				RetrievedSceneInfo->Proxy->ApplyLateUpdateTransform(RHICmdList, LateUpdateTransform);

			}
			RetrievedSceneInfo = Scene->GetPrimitiveSceneInfo(Index++);
		}
	}
}

void FExpandedLateUpdateManager::CacheSceneInfo(USceneComponent* Component)
{
	ensureMsgf(!Component->IsUsingAbsoluteLocation() && !Component->IsUsingAbsoluteRotation(), TEXT("SceneComponents that use absolute location or rotation are not supported by the LateUpdateManager"));

	UPrimitiveComponent* PrimitiveComponent = dynamic_cast<UPrimitiveComponent*>(Component);
	if (PrimitiveComponent && PrimitiveComponent->SceneProxy)
	{
		FPrimitiveSceneInfo* PrimitiveSceneInfo = PrimitiveComponent->SceneProxy->GetPrimitiveSceneInfo();
		if (PrimitiveSceneInfo && PrimitiveSceneInfo->IsIndexValid())
		{
			UpdateStates[LateUpdateGameWriteIndex].Primitives.Emplace(PrimitiveSceneInfo, PrimitiveSceneInfo->GetIndex());
		}
	}
}

void FExpandedLateUpdateManager::GatherLateUpdatePrimitives(USceneComponent* ParentComponent)
{
	CacheSceneInfo(ParentComponent);
	TArray<USceneComponent*> DirectComponents;

	ParentComponent->GetChildrenComponents(true, DirectComponents);
	for (USceneComponent* Component : DirectComponents)
	{
		if (Component != nullptr)
		{
			CacheSceneInfo(Component);
		}
	}
}

void FExpandedLateUpdateManager::ProcessGripArrayLateUpdatePrimitives(UGripMotionControllerComponent * MotionControllerComponent, TArray<FBPActorGripInformation> & GripArray)
{
	for (FBPActorGripInformation actor : GripArray)
	{

		if (!actor.GrippedObject || actor.GripCollisionType == EGripCollisionType::EventsOnly)
			continue;

		if (actor.GripCollisionType == EGripCollisionType::AttachmentGrip)
		{
			continue;
		}

		if (actor.GripMovementReplicationSetting == EGripMovementReplicationSettings::ForceServerSideMovement && !MotionControllerComponent->IsServer())
			continue;

		if (actor.bIsPaused)
			continue;

		switch (actor.GripLateUpdateSetting)
		{
		case EGripLateUpdateSettings::LateUpdatesAlwaysOff:
		{
			continue;
		}break;
		case EGripLateUpdateSettings::NotWhenColliding:
		{
			if (actor.bColliding && actor.GripCollisionType != EGripCollisionType::SweepWithPhysics && 
				actor.GripCollisionType != EGripCollisionType::PhysicsOnly)
				continue;
		}break;
		case EGripLateUpdateSettings::NotWhenDoubleGripping:
		{
			if (actor.SecondaryGripInfo.bHasSecondaryAttachment)
				continue;
		}break;
		case EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping:
		{
			if (
				(actor.bColliding && actor.GripCollisionType != EGripCollisionType::SweepWithPhysics && actor.GripCollisionType != EGripCollisionType::PhysicsOnly) ||
				(actor.SecondaryGripInfo.bHasSecondaryAttachment)
				)
			{
				continue;
			}
		}break;
		case EGripLateUpdateSettings::LateUpdatesAlwaysOn:
		default:
		{}break;
		}

		if (actor.GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
		{
			TArray<UVRGripScriptBase*> GripScripts;
			if (IVRGripInterface::Execute_GetGripScripts(actor.GrippedObject, GripScripts))
			{
				bool bContinueOn = false;
				for (UVRGripScriptBase* Script : GripScripts)
				{
					if (Script && Script->IsScriptActive() && Script->Wants_DenyLateUpdates())
					{
						bContinueOn = true;
						break;
					}
				}

				if (bContinueOn)
					continue;
			}
		}

		switch (actor.GripTargetType)
		{
		case EGripTargetType::ActorGrip:

		{
			AActor * pActor = actor.GetGrippedActor();
			if (pActor)
			{
				if (USceneComponent * rootComponent = pActor->GetRootComponent())
				{
					GatherLateUpdatePrimitives(rootComponent);
				}
			}

		}break;

		case EGripTargetType::ComponentGrip:

		{
			UPrimitiveComponent * cPrimComp = actor.GetGrippedComponent();
			if (cPrimComp)
			{
				GatherLateUpdatePrimitives(cPrimComp);
			}
		}break;
		}
	}
}

void UGripMotionControllerComponent::GetHandType(EControllerHand& Hand)
{
	if (!IMotionController::GetHandEnumForSourceName(MotionSource, Hand))
	{

		if (MotionSource.Compare(FName(TEXT("RightPalm"))) == 0 || MotionSource.Compare(FName(TEXT("RightWrist"))) == 0)
		{
			Hand = EControllerHand::Right;
		}

		else if (MotionSource.Compare(FName(TEXT("LeftPalm"))) == 0 || MotionSource.Compare(FName(TEXT("LeftWrist"))) == 0)
		{
			Hand = EControllerHand::Left;
		}
		else
		{
			Hand = EControllerHand::Left;
		}
	}
}

void UGripMotionControllerComponent::SetCustomPivotComponent(USceneComponent * NewCustomPivotComponent, FName PivotSocketName)
{
	CustomPivotComponent = NewCustomPivotComponent;
	CustomPivotComponentSocketName = PivotSocketName;
}

FTransform UGripMotionControllerComponent::GetPivotTransform_BP()
{
	return GetPivotTransform();
}

FVector UGripMotionControllerComponent::GetPivotLocation_BP()
{
	return GetPivotLocation();
}

FTransform UGripMotionControllerComponent::ConvertToControllerRelativeTransform(const FTransform & InTransform)
{
	return InTransform.GetRelativeTransform(!bSkipPivotTransformAdjustment && IsValid(CustomPivotComponent) ? CustomPivotComponent->GetSocketTransform(CustomPivotComponentSocketName) : this->GetComponentTransform());
}

FTransform UGripMotionControllerComponent::ConvertToGripRelativeTransform(const FTransform& GrippedActorTransform, const FTransform & InTransform)
{
	return InTransform.GetRelativeTransform(GrippedActorTransform);
}

bool UGripMotionControllerComponent::GetIsObjectHeld(const UObject * ObjectToCheck)
{
	if (!ObjectToCheck)
		return false;

	return (GrippedObjects.FindByKey(ObjectToCheck) || LocallyGrippedObjects.FindByKey(ObjectToCheck));
}

bool UGripMotionControllerComponent::GetIsHeld(const AActor * ActorToCheck)
{
	if (!ActorToCheck)
		return false;

	return (GrippedObjects.FindByKey(ActorToCheck) || LocallyGrippedObjects.FindByKey(ActorToCheck));
}

bool UGripMotionControllerComponent::GetIsComponentHeld(const UPrimitiveComponent * ComponentToCheck)
{
	if (!ComponentToCheck)
		return false;

	return (GrippedObjects.FindByKey(ComponentToCheck) || LocallyGrippedObjects.FindByKey(ComponentToCheck));

}

bool UGripMotionControllerComponent::GetIsSecondaryAttachment(const USceneComponent * ComponentToCheck, FBPActorGripInformation & Grip)
{
	if (!ComponentToCheck)
		return false;

	for (int i = 0; i < GrippedObjects.Num(); ++i)
	{
		if (GrippedObjects[i].SecondaryGripInfo.bHasSecondaryAttachment && GrippedObjects[i].SecondaryGripInfo.SecondaryAttachment == ComponentToCheck)
		{
			Grip = GrippedObjects[i];
			return true;
		}
	}

	for (int i = 0; i < LocallyGrippedObjects.Num(); ++i)
	{
		if (LocallyGrippedObjects[i].SecondaryGripInfo.bHasSecondaryAttachment && LocallyGrippedObjects[i].SecondaryGripInfo.SecondaryAttachment == ComponentToCheck)
		{
			Grip = LocallyGrippedObjects[i];
			return true;
		}
	}

	return false;
}

bool UGripMotionControllerComponent::HasGrippedObjects()
{
	return GrippedObjects.Num() > 0 || LocallyGrippedObjects.Num() > 0;
}

bool UGripMotionControllerComponent::SetUpPhysicsHandle_BP(const FBPActorGripInformation &Grip)
{
	return SetUpPhysicsHandle(Grip);
}

bool UGripMotionControllerComponent::DestroyPhysicsHandle_BP(const FBPActorGripInformation &Grip)
{
	return DestroyPhysicsHandle(Grip);
}

bool UGripMotionControllerComponent::UpdatePhysicsHandle_BP(const FBPActorGripInformation& Grip, bool bFullyRecreate)
{
	return UpdatePhysicsHandle(Grip.GripID, bFullyRecreate);
}

bool UGripMotionControllerComponent::GetPhysicsHandleSettings(const FBPActorGripInformation& Grip, FBPAdvancedPhysicsHandleSettings& PhysicsHandleSettingsOut)
{
	FBPActorPhysicsHandleInformation * HandleInfo = GetPhysicsGrip(Grip);

	if (!HandleInfo)
		return false;

	PhysicsHandleSettingsOut.FillFrom(HandleInfo);
	return true;
}

bool UGripMotionControllerComponent::SetPhysicsHandleSettings(const FBPActorGripInformation& Grip, const FBPAdvancedPhysicsHandleSettings& PhysicsHandleSettingsIn)
{
	FBPActorPhysicsHandleInformation* HandleInfo = GetPhysicsGrip(Grip);

	if (!HandleInfo)
		return false;

	PhysicsHandleSettingsIn.FillTo(HandleInfo);
	return UpdatePhysicsHandle(Grip, true);
}

void UGripMotionControllerComponent::UpdatePhysicsHandleTransform_BP(const FBPActorGripInformation &GrippedActor, const FTransform& NewTransform)
{
	return UpdatePhysicsHandleTransform(GrippedActor, NewTransform);
}

bool UGripMotionControllerComponent::GetGripDistance_BP(FBPActorGripInformation &Grip, FVector ExpectedLocation, float & CurrentDistance)
{
	if (!Grip.GrippedObject)
		return false;

	UPrimitiveComponent * RootComp = nullptr;

	if (Grip.GripTargetType == EGripTargetType::ActorGrip)
	{
		RootComp = Cast<UPrimitiveComponent>(Grip.GetGrippedActor()->GetRootComponent());
	}
	else
		RootComp = Grip.GetGrippedComponent();

	if (!RootComp)
		return false;

	FVector CheckDistance;
	if (!GetPhysicsJointLength(Grip, RootComp, CheckDistance))
	{
		CheckDistance = (ExpectedLocation - RootComp->GetComponentLocation());
	}

	CurrentDistance = CheckDistance.Size();
	return true;
}

bool UGripMotionControllerComponent::GripControllerIsTracked() const
{
	return IsTracked();
}

bool UGripMotionControllerComponent::HasAuthority() const
{
	return bHasAuthority;
}

void UGripMotionControllerComponent::CheckTransactionBuffer()
{
	if (LocalTransactionBuffer.Num())
	{
		for (int i = LocalTransactionBuffer.Num() - 1; i >= 0; --i)
		{
			if (LocalTransactionBuffer[i].ValueCache.bWasInitiallyRepped && LocalTransactionBuffer[i].GripID != LocalTransactionBuffer[i].ValueCache.CachedGripID)
			{

				LocalTransactionBuffer[i].ClearNonReppingItems();
			}

			if (!LocalTransactionBuffer[i].ValueCache.bWasInitiallyRepped && LocalTransactionBuffer[i].GrippedObject->IsValidLowLevelFast())
			{
				LocalTransactionBuffer[i].ValueCache.bWasInitiallyRepped = true;
				LocalTransactionBuffer[i].ValueCache.CachedGripID = LocalTransactionBuffer[i].GripID;

				int32 Index = LocallyGrippedObjects.Add(LocalTransactionBuffer[i]);
				DIRTY_LOCALLY_GRIPPED_OBJECTS();

				if (Index != INDEX_NONE)
				{
					NotifyGrip(LocallyGrippedObjects[Index]);
				}

				Server_NotifyHandledTransaction(LocalTransactionBuffer[i].GripID);
			}
		}
	}
}

void UGripMotionControllerComponent::SetSmoothReplicatedMotion(bool bNewSmoothReplicatedMotion)
{
	bSmoothReplicatedMotion = bNewSmoothReplicatedMotion;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, bSmoothReplicatedMotion, this);
#endif
}

void UGripMotionControllerComponent::SetReplicateWithoutTracking(bool bNewReplicateWithoutTracking)
{
	bReplicateWithoutTracking = bNewReplicateWithoutTracking;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, bReplicateWithoutTracking, this);
#endif
}

void UGripMotionControllerComponent::SetControllerNetUpdateRate(float NewControllerNetUpdateRate)
{
	ControllerNetUpdateRate = NewControllerNetUpdateRate;
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, ControllerNetUpdateRate, this);
#endif
}

void UGripMotionControllerComponent::DIRTY_GRIPPED_OBJECTS()
{
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, GrippedObjects, this);
#endif
}

void UGripMotionControllerComponent::DIRTY_LOCALLY_GRIPPED_OBJECTS()
{
#if WITH_PUSH_MODEL
	MARK_PROPERTY_DIRTY_FROM_NAME(UGripMotionControllerComponent, LocallyGrippedObjects, this);
#endif
}

