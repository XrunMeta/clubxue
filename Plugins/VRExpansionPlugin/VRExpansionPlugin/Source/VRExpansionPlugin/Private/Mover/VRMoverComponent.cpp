
#include "Mover/VRMoverComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRMoverComponent)

#include "VRBPDatatypes.h"
#include "DefaultMovementSet/LayeredMoves/BasicLayeredMoves.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Curves/CurveFloat.h" 
#include "ReplicatedVRCameraComponent.h"

DEFINE_LOG_CATEGORY(LogVRMoverComponent);

static const FName Name_CharacterMotionComponent(TEXT("MoverComponent"));

FLayeredMove_VRMovement::FLayeredMove_VRMovement()
	: Velocity(FVector::ZeroVector)
	, MagnitudeOverTime(nullptr)
	, SettingsFlags(0)
{

	DurationMs = -1.0f;
}

bool FLayeredMove_VRMovement::IsFinished(double CurrentSimTimeMs) const
{

	return false;
}

bool FLayeredMove_VRMovement::GenerateMove(const FMoverTickStartData& SimState, const FMoverTimeStep& TimeStep, const UMoverComponent* MoverComp, UMoverBlackboard* SimBlackboard, FProposedMove& OutProposedMove)
{
	const FMoverDefaultSyncState* SyncState = SimState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	check(SyncState);

	const float DeltaSeconds = TimeStep.StepMs * 0.001f;

	if (SettingsFlags & (uint8)ELayeredMove_ConstantVelocitySettingsFlags::VelocityStartRelative &&
		StartSimTimeMs == TimeStep.BaseSimTimeMs)
	{
		SettingsFlags &= ~(uint8)ELayeredMove_ConstantVelocitySettingsFlags::VelocityStartRelative;
		Velocity = SyncState->GetOrientation_WorldSpace().RotateVector(Velocity);
	}

	FVector VelocityThisFrame = Velocity;

	if (SettingsFlags & (uint8)ELayeredMove_ConstantVelocitySettingsFlags::VelocityAlwaysRelative)
	{
		VelocityThisFrame = SyncState->GetOrientation_WorldSpace().RotateVector(Velocity);
	}

	if (MagnitudeOverTime && DurationMs > 0)
	{
		const float TimeValue = DurationMs > 0.f ? FMath::Clamp((TimeStep.BaseSimTimeMs - StartSimTimeMs) / DurationMs, 0.f, 1.f) : TimeStep.BaseSimTimeMs;
		const float TimeFactor = MagnitudeOverTime->GetFloatValue(TimeValue);
		VelocityThisFrame *= TimeFactor;
	}

	OutProposedMove.LinearVelocity = VelocityThisFrame;

	return true;
}

FLayeredMoveBase* FLayeredMove_VRMovement::Clone() const
{
	FLayeredMove_VRMovement* CopyPtr = new FLayeredMove_VRMovement(*this);
	return CopyPtr;
}

void FLayeredMove_VRMovement::NetSerialize(FArchive& Ar)
{
	Super::NetSerialize(Ar);

	SerializePackedVector<10, 16>(Velocity, Ar);
	Ar << SettingsFlags;
	Ar << MagnitudeOverTime;
}

UScriptStruct* FLayeredMove_VRMovement::GetScriptStruct() const
{
	return FLayeredMove_VRMovement::StaticStruct();
}

FString FLayeredMove_VRMovement::ToSimpleString() const
{
	return FString::Printf(TEXT("LinearVelocity"));
}

void FLayeredMove_VRMovement::AddReferencedObjects(class FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);
}

FMoverDataStructBase* FVRMoverHMDSyncState::Clone() const
{

	FVRMoverHMDSyncState* CopyPtr = new FVRMoverHMDSyncState(*this);
	return CopyPtr;
}

bool FVRMoverHMDSyncState::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Super::NetSerialize(Ar, Map, bOutSuccess);

	SerializeFixedVector<2, 8>(MoveDirectionIntent, Ar);
	bOutSuccess = true;
	return true;
}

AVRMoverBasePawn::AVRMoverBasePawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	CharacterMotionComponent = CreateDefaultSubobject<UMoverComponent>(Name_CharacterMotionComponent);
	ensure(CharacterMotionComponent);

	PrimaryActorTick.bCanEverTick = true;

	SetReplicatingMovement(false);	

	auto IsImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};

	static FName ProduceInputBPFuncName = FName(TEXT("OnProduceInputInBlueprint"));
	UFunction* ProduceInputFunction = GetClass()->FindFunctionByName(ProduceInputBPFuncName);
	bHasProduceInputinBpFunc = IsImplementedInBlueprint(ProduceInputFunction);
}

void AVRMoverBasePawn::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(CharacterMotionComponent))
	{ 

		CharacterMotionComponent->PersistentSyncStateDataTypes.Add(FMoverDataPersistence(FVRMoverHMDSyncState::StaticStruct(), true));

		if (HasLocalNetOwner())
		{

			TSharedPtr<FLayeredMove_VRMovement> VRMoveLayer = MakeShared<FLayeredMove_VRMovement>();
			CharacterMotionComponent->QueueLayeredMove(VRMoveLayer);
		}
	}
}

UPrimitiveComponent* AVRMoverBasePawn::GetMovementBase() const
{
	return CharacterMotionComponent ? CharacterMotionComponent->GetMovementBase() : nullptr;
}

void AVRMoverBasePawn::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	OnProduceInput((float)SimTimeMs, InputCmdResult);

	if (bHasProduceInputinBpFunc)
	{
		InputCmdResult = OnProduceInputInBlueprint((float)SimTimeMs, InputCmdResult);
	}
}

void AVRMoverBasePawn::OnProduceInput(float DeltaMs, FMoverInputCmdContext& OutInputCmd)
{

}
