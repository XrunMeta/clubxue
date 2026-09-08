

#include "Grippables/GrippablePhysicsReplication.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GrippablePhysicsReplication)

#include "CoreMinimal.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsReplicationLOD.h"
#include "Physics/PhysicsReplicationQuantization.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "DrawDebugHelpers.h"
#include "Chaos/ChaosMarshallingManager.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "Chaos/PhysicsObjectInternalInterface.h"

#include "PBDRigidsSolver.h"
#include "Chaos/PBDRigidsEvolutionGBF.h"
#include "VRGlobalSettings.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "Chaos/DebugDrawQueue.h"
#include "Chaos/Particles.h"

#include "Misc/ScopeRWLock.h"
#include "RewindData.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Player.h"

namespace VRPhysicsReplicationStatics
{
	static bool bHasVRPhysicsReplication = false;
}

#if WITH_EDITOR

namespace RenderInterpolationCVars
{
	bool bRenderInterpDebugDrawResimTrigger = false;
	float RenderInterpDebugDrawResimBoxScale = 1.0f;
}

namespace PhysicsReplicationLODCVars
{

	 int32 TransitionExtrapFrameMin = 3;
	 float TransitionExtrapFraction = 0.3f;
	 bool bTransitionModeDebugLog = false;
}

namespace PhysicsReplicationCVars
{
	int32 SkipSkeletalRepOptimization = 1;
#if !UE_BUILD_SHIPPING

#endif

	int32 EnableDefaultReplication = 0;
	int32 DebugDrawShowRepMode = 0;
	float DebugDrawLifeTime = 3.0f;

	namespace DefaultReplicationCVars
	{
		bool bHardsnapLegacyInPT = false;
		bool bCorrectConnectedBodies = false;
		bool bCorrectConnectedBodiesFriction = true;
	}

	namespace ResimulationCVars
	{

		bool bResimulateSleepDesync = true;
		bool bRuntimeCorrectionEnabled = false;
		bool bRuntimeVelocityCorrection = false;
		bool bRuntimeCorrectConnectedBodies = true;
		bool bDisableReplicationOnInteraction = false;
		bool bKeepResimStateForNonResimReplicatedObjects = false;
		float PosStabilityMultiplier = 0.5f;
		float RotStabilityMultiplier = 1.0f;
		float VelStabilityMultiplier = 0.5f;
		float AngVelStabilityMultiplier = 0.5f;
		bool bDrawDebug = false;
		float LogOutOfBoundsTimeLimit = 5.0f;

		bool bDynamicInputBufferScalingEnabled = true;
		bool bApplyInputDecaySimProxyInputAtRuntime = false;
		float InputDecaySimProxyInputAtRuntime = 0.25f;
		bool bEnableLagScalingSimProxyRuntimeInputDecay = false;
		bool bEnableLagScalingInputDecay = false;
		float InputDecayReferenceLagMs = 100.0f;

		int32 RedundantInputs = 2;
		int32 RedundantRemoteInputs = 1;
		int32 RedundantStates = 0;
		bool bAllowRewindToClosestState = true;
		bool bCompareStateToTriggerRewind = false;
		bool bCompareStateToTriggerRewindIncludeSimProxies = false;
		bool bCompareInputToTriggerRewind = false;
		bool bEnableUnreliableFlow = true;
		bool bEnableReliableFlow = false;

		bool bTriggerResimOnInputReceive = false;
		bool bApplyInputDecayOverSetTime = false;
		float InputDecaySetTime = 0.15f;

		bool bApplyDataInsteadOfMergeData = false;
		bool bAllowInputExtrapolation = true;
		bool bValidateDataOnGameThread = false;
		bool bApplySimProxyStateAtRuntime = false;
		bool bApplySimProxyInputAtRuntime = true;
		bool bApplyPredictiveInterpolationWhenBehindServer = true;
	}

	namespace PredictiveInterpolationCVars
	{
		float PosCorrectionTimeBase = 0.0f;
		float PosCorrectionTimeMin = 0.1f;
		float PosCorrectionTimeMultiplier = 1.0f;
		float RotCorrectionTimeBase = 0.0f;
		float RotCorrectionTimeMin = 0.1f;
		float RotCorrectionTimeMultiplier = 1.0f;
		float PosInterpolationTimeMultiplier = 1.1f;
		float RotInterpolationTimeMultiplier = 1.25f;
		float AverageReceiveIntervalSmoothing = 3.0f;
		float ExtrapolationTimeMultiplier = 3.0f;
		float ExtrapolationMinTime = 0.75f;
		float MinExpectedDistanceCovered = 0.5f;
		float ErrorAccumulationDecreaseMultiplier = 0.5f;
		float ErrorAccumulationSeconds = 3.0f;
		bool bDisableErrorVelocityLimits = false;
		float ErrorAccLinVelMaxLimit = 50.0f;
		float ErrorAccAngVelMaxLimit = 1.5f;
		float SoftSnapPosStrength = 0.5f;
		float SoftSnapRotStrength = 0.5f;
		bool bSoftSnapToSource = false;
		float EarlyOutDistanceSqr = 1.0f;
		float EarlyOutAngle = 1.5f;
		bool bEarlyOutWithVelocity = true;
		bool bSkipVelocityRepOnPosEarlyOut = true;
		bool bPostResimWaitForUpdate = false;
		bool bVelocityBased = true;

		bool bCorrectionAsVelocity = false;
		bool bCorrectConnectedBodies = false;
		bool bCorrectConnectedBodiesFriction = true;
		bool bSleepConnectedBodies = true;
		bool bKinematicPrediction = true;
		bool bKinematicHardSnap = false;

		bool bDisableSoftSnap = false;
		bool bAlwaysHardSnap = false;
		bool bSkipReplication = false;
		bool bDontClearTarget = false;
		bool bDrawDebugTargets = false;
		bool bDrawDebugVectors = false;
		float DrawDebugZOffset = 50.0f;
		float SleepSecondsClearTarget = 15.0f;
		int32 TargetTickAlignmentClampMultiplier = 2;
		int32 TeleportDetectionEnabled = 1;
		float TeleportDetectionMinDistance = 200.0f;
		float TeleportDetectionVelocityMultiplier = 1.3f;
	}

}
#endif

namespace Chaos
{
	extern CHAOS_API int32 RewindBeforeAdvance;
}

FPhysicsReplicationVR::~FPhysicsReplicationVR()
{
	if (PhysicsReplicationAsyncVR)
	{
		if (auto* Solver = PhysSceneVR->GetSolver())
		{
			Solver->UnregisterAndFreeSimCallbackObject_External(PhysicsReplicationAsyncVR);
		}
	}
}

void ComputeDeltasVR(const FVector& CurrentPos, const FQuat& CurrentQuat, const FVector& TargetPos, const FQuat& TargetQuat, FVector& OutLinDiff, float& OutLinDiffSize,
	FVector& OutAngDiffAxis, float& OutAngDiff, float& OutAngDiffSize)
{
	OutLinDiff = TargetPos - CurrentPos;
	OutLinDiffSize = OutLinDiff.Size();
	const FQuat InvCurrentQuat = CurrentQuat.Inverse();
	const FQuat DeltaQuat = InvCurrentQuat * TargetQuat;
	DeltaQuat.ToAxisAndAngle(OutAngDiffAxis, OutAngDiff);
	OutAngDiff = FMath::RadiansToDegrees(FMath::UnwindRadians(OutAngDiff));
	OutAngDiffSize = FMath::Abs(OutAngDiff);
}

FPhysicsReplicationVR::FPhysicsReplicationVR(FPhysScene* PhysScene) :
	FPhysicsReplication(PhysScene)
{
	PhysSceneVR = PhysScene;
	AsyncInputVR = nullptr;
	PhysicsReplicationAsyncVR = nullptr;
	if (auto* Solver = PhysSceneVR->GetSolver())
	{
		PhysicsReplicationAsyncVR = Solver->CreateAndRegisterSimCallbackObject_External<FPhysicsReplicationAsyncVR>();
		PhysicsReplicationAsyncVR->Setup(UPhysicsSettings::Get()->PhysicErrorCorrection);
	}

	VRPhysicsReplicationStatics::bHasVRPhysicsReplication = true;
}

void FPhysicsReplicationVR::PrepareAsyncData_ExternalVR(const FRigidBodyErrorCorrection& ErrorCorrection)
{

	static const auto CVarLinSet = IConsoleManager::Get().FindConsoleVariable(TEXT("p.PositionLerp"));
	const float PositionLerp = CVarLinSet->GetFloat() >= 0.0f ? CVarLinSet->GetFloat() : ErrorCorrection.PositionLerp;

	static const auto CVarLinLerp = IConsoleManager::Get().FindConsoleVariable(TEXT("p.LinearVelocityCoefficient"));
	const float LinearVelocityCoefficient = CVarLinLerp->GetFloat() >= 0.0f ? CVarLinLerp->GetFloat() : ErrorCorrection.LinearVelocityCoefficient;

	static const auto CVarAngSet = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AngleLerp"));
	const float AngleLerp = CVarAngSet->GetFloat() >= 0.0f ? CVarAngSet->GetFloat() : ErrorCorrection.AngleLerp;

	static const auto CVarAngLerp = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AngularVelocityCoefficient"));
	const float AngularVelocityCoefficient = CVarAngLerp->GetFloat() >= 0.0f ? CVarAngLerp->GetFloat() : ErrorCorrection.AngularVelocityCoefficient;

	AsyncInputVR = PhysicsReplicationAsyncVR->GetProducerInputData_External();
	AsyncInputVR->ErrorCorrection.PositionLerp = PositionLerp;
	AsyncInputVR->ErrorCorrection.AngleLerp = AngleLerp;
	AsyncInputVR->ErrorCorrection.LinearVelocityCoefficient = LinearVelocityCoefficient;
	AsyncInputVR->ErrorCorrection.AngularVelocityCoefficient = AngularVelocityCoefficient;
}

bool FPhysicsReplicationVR::IsInitialized()
{
	return VRPhysicsReplicationStatics::bHasVRPhysicsReplication;
}

bool FPhysicsReplicationVR::ApplyRigidBodyState(float DeltaSeconds, FBodyInstance* BI, FReplicatedPhysicsTarget& PhysicsTarget, const FRigidBodyErrorCorrection& ErrorCorrection, const float InPingSecondsOneWay, int32 LocalFrame, int32 NumPredictedFrames)
{

	if (const UWorld* World = GetOwningWorld())
	{
		if (World->GetNetMode() == ENetMode::NM_Client)
		{
			return FPhysicsReplication::ApplyRigidBodyState(DeltaSeconds, BI, PhysicsTarget, ErrorCorrection, InPingSecondsOneWay, LocalFrame, NumPredictedFrames);
		}
	}

	return ApplyRigidBodyState(DeltaSeconds, BI, PhysicsTarget, ErrorCorrection, InPingSecondsOneWay);
}

void FPhysicsReplicationVR::SetReplicatedTarget(UPrimitiveComponent* Component, FName BoneName, const FRigidBodyState& ReplicatedTarget, int32 ServerFrame)
{

	AActor* Owner = Component->GetOwner();
	static const auto CVarEnableDefaultReplication = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.EnableDefaultReplication"));
	if (Owner && (CVarEnableDefaultReplication->GetInt() || Owner->GetPhysicsReplicationMode() != EPhysicsReplicationMode::Default)) 
	{
		const ENetRole OwnerRole = Owner->GetLocalRole();
		const bool bIsSimulated = OwnerRole == ROLE_SimulatedProxy;
		const bool bIsReplicatedAutonomous = OwnerRole == ROLE_AutonomousProxy && Component->bReplicatePhysicsToAutonomousProxy;
		if (bIsSimulated || bIsReplicatedAutonomous)
		{
			Chaos::FConstPhysicsObjectHandle PhysicsObject = Component->GetPhysicsObjectByName(BoneName);
			SetReplicatedTargetVR(PhysicsObject, ReplicatedTarget, ServerFrame, Owner->GetPhysicsReplicationMode());
			return;
		}
	}

	if (UWorld* OwningWorld = GetOwningWorld())
	{

		TWeakObjectPtr<UPrimitiveComponent> TargetKey(Component);
		FReplicatedPhysicsTarget* Target = ComponentToTargetsVR_DEPRECATED.Find(TargetKey);
		if (!Target)
		{

			Target = &ComponentToTargetsVR_DEPRECATED.Add(TargetKey);
			Target->PrevPos = ReplicatedTarget.Position;
			Target->PrevPosTarget = ReplicatedTarget.Position;
		}

		Target->ServerFrame = ServerFrame;
		Target->TargetState = ReplicatedTarget;
		Target->BoneName = BoneName;
		Target->ArrivedTimeSeconds = OwningWorld->GetTimeSeconds();

		ensure(!Target->PrevPos.ContainsNaN());
		ensure(!Target->PrevPosTarget.ContainsNaN());
		ensure(!Target->TargetState.Position.ContainsNaN());

		OnSetReplicatedTarget(Component, BoneName, ReplicatedTarget, ServerFrame, *Target);
	}
}

void FPhysicsReplicationVR::SetReplicatedTargetVR(Chaos::FConstPhysicsObjectHandle PhysicsObject, const FRigidBodyState& ReplicatedTarget, int32 ServerFrame, EPhysicsReplicationMode ReplicationMode)
{

	if (const UWorld* World = GetOwningWorld())
	{
		if (World->GetNetMode() == ENetMode::NM_Client)
		{
			return FPhysicsReplication::SetReplicatedTarget(PhysicsObject, ReplicatedTarget, ServerFrame);
		}
	}

	if (!PhysicsObject)
	{
		return;
	}

	UWorld* OwningWorld = GetOwningWorld();
	if (OwningWorld == nullptr)
	{
		return;
	}

	FReplicatedPhysicsTarget Target(PhysicsObject);

	Target.ReplicationMode = ReplicationMode;
	Target.ServerFrame = ServerFrame;
	Target.TargetState = ReplicatedTarget;
	Target.ArrivedTimeSeconds = OwningWorld->GetTimeSeconds();

	ensure(!Target.TargetState.Position.ContainsNaN());

	ReplicatedTargetsQueueVR.Add(Target);
}

void FPhysicsReplicationVR::RemoveReplicatedTarget(UPrimitiveComponent* Component)
{

	if (Component == nullptr)
	{
		return;
	}

	ComponentToTargetsVR_DEPRECATED.Remove(Component);

	Chaos::FConstPhysicsObjectHandle PhysicsObject = Component->GetPhysicsObjectByName(NAME_None);

	if (const UWorld* World = GetOwningWorld())
	{
		if (World->GetNetMode() == ENetMode::NM_Client)
		{
			return FPhysicsReplication::RemoveReplicatedTarget(PhysicsObject);
		}
	}

	RemoveReplicatedTargetVR(PhysicsObject);
}

void FPhysicsReplicationVR::RemoveReplicatedTargetVR(Chaos::FConstPhysicsObjectHandle PhysicsObject)
{

	if (!PhysicsObject)
	{
		return;
	}

	FReplicatedPhysicsTarget Target(PhysicsObject); 
	ReplicatedTargetsQueueVR.Add(Target);
}

bool FPhysicsReplicationVR::ApplyRigidBodyState(float DeltaSeconds, FBodyInstance* BI, FReplicatedPhysicsTarget& PhysicsTarget, const FRigidBodyErrorCorrection& ErrorCorrection, const float PingSecondsOneWay, bool* bDidHardSnap)
{

	if (const UWorld* World = GetOwningWorld())
	{
		if (World->GetNetMode() == ENetMode::NM_Client)
		{
			return FPhysicsReplication::ApplyRigidBodyState(DeltaSeconds, BI, PhysicsTarget, ErrorCorrection, PingSecondsOneWay);
		}
	}

	static const auto CVarSkipPhysicsReplication = IConsoleManager::Get().FindConsoleVariable(TEXT("p.SkipPhysicsReplication"));
	if (CVarSkipPhysicsReplication->GetInt())
	{
		return false;
	}

	if (!BI->IsInstanceSimulatingPhysics())
	{
		return false;
	}

	bool bRestoredState = true;
	const FRigidBodyState NewState = PhysicsTarget.TargetState;
	const float NewQuatSizeSqr = NewState.Quaternion.SizeSquared();

	if (!BI->IsInstanceSimulatingPhysics())
	{
		UE_LOGF(LogPhysics, Warning, "Physics replicating on non-simulated body. (%ls)", *BI->GetBodyDebugName());
		return bRestoredState;
	}
	else if (NewQuatSizeSqr < UE_KINDA_SMALL_NUMBER)
	{
		UE_LOGF(LogPhysics, Warning, "Invalid zero quaternion set for body. (%ls)", *BI->GetBodyDebugName());
		return bRestoredState;
	}
	else if (FMath::Abs(NewQuatSizeSqr - 1.f) > UE_KINDA_SMALL_NUMBER)
	{
		UE_LOGF(LogPhysics, Warning, "Quaternion (%f %f %f %f) with non-unit magnitude detected. (%ls)",
			NewState.Quaternion.X, NewState.Quaternion.Y, NewState.Quaternion.Z, NewState.Quaternion.W, *BI->GetBodyDebugName());
		return bRestoredState;
	}

	static const auto CVarNetPingExtrapolation = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetPingExtrapolation"));
	const float NetPingExtrapolation = CVarNetPingExtrapolation->GetFloat() >= 0.0f ? CVarNetPingExtrapolation->GetFloat() : ErrorCorrection.PingExtrapolation;

	static const auto CVarNetPingLimit = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetPingLimit"));
	const float NetPingLimit = CVarNetPingLimit->GetFloat() > 0.0f ? CVarNetPingLimit->GetFloat() : ErrorCorrection.PingLimit;

	static const auto CVarErrorPerLinearDifference = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorPerLinearDifference"));
	const float ErrorPerLinearDiff = CVarErrorPerLinearDifference->GetFloat() >= 0.0f ? CVarErrorPerLinearDifference->GetFloat() : ErrorCorrection.ErrorPerLinearDifference;

	static const auto CVarErrorPerAngularDifference = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorPerAngularDifference"));
	const float ErrorPerAngularDiff = CVarErrorPerAngularDifference->GetFloat() >= 0.0f ? CVarErrorPerAngularDifference->GetFloat() : ErrorCorrection.ErrorPerAngularDifference;

	static const auto CVarMaxRestoredStateError = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxRestoredStateError"));
	const float MaxRestoredStateError = CVarMaxRestoredStateError->GetFloat() >= 0.0f ? CVarMaxRestoredStateError->GetFloat() : ErrorCorrection.MaxRestoredStateError;

	static const auto CVarErrorAccumulation = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorAccumulationSeconds"));
	const float ErrorAccumulationSeconds = CVarErrorAccumulation->GetFloat() >= 0.0f ? CVarErrorAccumulation->GetFloat() : ErrorCorrection.ErrorAccumulationSeconds;

	static const auto CVarErrorAccumulationDistanceSq = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorAccumulationDistanceSq"));
	const float ErrorAccumulationDistanceSq = CVarErrorAccumulationDistanceSq->GetFloat() >= 0.0f ? CVarErrorAccumulationDistanceSq->GetFloat() : ErrorCorrection.ErrorAccumulationDistanceSq;

	static const auto CVarErrorAccumulationSimilarity = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorAccumulationSimilarity"));
	const float ErrorAccumulationSimilarity = CVarErrorAccumulationSimilarity->GetFloat() >= 0.0f ? CVarErrorAccumulationSimilarity->GetFloat() : ErrorCorrection.ErrorAccumulationSimilarity;

	static const auto CVarLinSet = IConsoleManager::Get().FindConsoleVariable(TEXT("p.PositionLerp"));
	const float PositionLerp = CVarLinSet->GetFloat() >= 0.0f ? CVarLinSet->GetFloat() : ErrorCorrection.PositionLerp;

	static const auto CVarLinLerp = IConsoleManager::Get().FindConsoleVariable(TEXT("p.LinearVelocityCoefficient"));
	const float LinearVelocityCoefficient = CVarLinLerp->GetFloat() >= 0.0f ? CVarLinLerp->GetFloat() : ErrorCorrection.LinearVelocityCoefficient;

	static const auto CVarAngSet = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AngleLerp"));
	const float AngleLerp = CVarAngSet->GetFloat() >= 0.0f ? CVarAngSet->GetFloat() : ErrorCorrection.AngleLerp;

	static const auto CVarAngLerp = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AngularVelocityCoefficient"));
	const float AngularVelocityCoefficient = CVarAngLerp->GetFloat() >= 0.0f ? CVarAngLerp->GetFloat() : ErrorCorrection.AngularVelocityCoefficient;

	static const auto CVarMaxLinearHardSnapDistance = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxLinearHardSnapDistance"));
	float MaxLinearHardSnapDistance = CVarMaxLinearHardSnapDistance->GetFloat() >= 0.f ? CVarMaxLinearHardSnapDistance->GetFloat() : ErrorCorrection.MaxLinearHardSnapDistance;

	static const auto CVarHardsnapLegacyInPT = IConsoleManager::Get().FindConsoleVariable(TEXT("p.DefaultReplication.Legacy.HardsnapInPT"));
	bool bHardsnapLegacyInPT = CVarHardsnapLegacyInPT->GetBool();

	static const auto CVarCorrectConnectedBodies = IConsoleManager::Get().FindConsoleVariable(TEXT("p.DefaultReplication.CorrectConnectedBodies"));
	bool bCorrectConnectedBodies = CVarCorrectConnectedBodies->GetBool();

	static const auto CVarCorrectConnectedBodiesFriction = IConsoleManager::Get().FindConsoleVariable(TEXT("p.DefaultReplication.CorrectConnectedBodiesFriction"));
	bool bCorrectConnectedBodiesFriction = CVarCorrectConnectedBodiesFriction->GetBool();

	if (SettingsCurrent.IsValid())
	{
		const FNetworkPhysicsSettingsData& SettingsData = SettingsCurrent.Pin()->GetSettings();
		MaxLinearHardSnapDistance = SettingsData.DefaultReplicationSettings.GetMaxLinearHardSnapDistance(MaxLinearHardSnapDistance);
		bHardsnapLegacyInPT = SettingsData.DefaultReplicationSettings.GetHardsnapDefaultLegacyInPT();
		bCorrectConnectedBodies = SettingsData.DefaultReplicationSettings.GetCorrectConnectedBodies();
		bCorrectConnectedBodiesFriction = SettingsData.DefaultReplicationSettings.GetCorrectConnectedBodiesFriction();
	}

	FRigidBodyState CurrentState;
	BI->GetRigidBodyState(CurrentState);

	const float PingSeconds = FMath::Clamp(PingSecondsOneWay, 0.f, NetPingLimit);
	const float ExtrapolationDeltaSeconds = PingSeconds * NetPingExtrapolation;
	const FVector ExtrapolationDeltaPos = NewState.LinVel * ExtrapolationDeltaSeconds;
	const FVector_NetQuantize100 TargetPos = NewState.Position + ExtrapolationDeltaPos;
	float NewStateAngVel;
	FVector NewStateAngVelAxis;
	NewState.AngVel.FVector::ToDirectionAndLength(NewStateAngVelAxis, NewStateAngVel);
	NewStateAngVel = FMath::DegreesToRadians(NewStateAngVel);
	const FQuat ExtrapolationDeltaQuaternion = FQuat(NewStateAngVelAxis, NewStateAngVel * ExtrapolationDeltaSeconds);
	FQuat TargetQuat = ExtrapolationDeltaQuaternion * NewState.Quaternion;

	FVector LinDiff;
	float LinDiffSize;
	FVector AngDiffAxis;
	float AngDiff;

	float AngDiffSize;

	ComputeDeltasVR(CurrentState.Position, CurrentState.Quaternion, TargetPos, TargetQuat, LinDiff, LinDiffSize, AngDiffAxis, AngDiff, AngDiffSize);

	const bool bShouldSleep = (NewState.Flags & ERigidBodyFlags::Sleeping) != 0;
	const bool bWasAwake = BI->IsInstanceAwake();
	const bool bAutoWake = false;

	const float Error = (LinDiffSize * ErrorPerLinearDiff) + (AngDiffSize * ErrorPerAngularDiff);
	bRestoredState = Error < MaxRestoredStateError;
	if (bRestoredState)
	{
		PhysicsTarget.AccumulatedErrorSeconds = 0.0f;
	}
	else
	{

		const float PrevProgress = FVector::DotProduct(
			FVector(CurrentState.Position) - PhysicsTarget.PrevPos,
			(PhysicsTarget.PrevPosTarget - PhysicsTarget.PrevPos).GetSafeNormal());

		const float PrevSimilarity = FVector::DotProduct(
			TargetPos - FVector(CurrentState.Position),
			PhysicsTarget.PrevPosTarget - PhysicsTarget.PrevPos);

		if (PrevProgress < ErrorAccumulationDistanceSq &&
			PrevSimilarity > ErrorAccumulationSimilarity)
		{
			PhysicsTarget.AccumulatedErrorSeconds += DeltaSeconds;
		}
		else
		{
			PhysicsTarget.AccumulatedErrorSeconds = FMath::Max(PhysicsTarget.AccumulatedErrorSeconds - DeltaSeconds, 0.0f);
		}

		static const auto CVarAlwaysHardSnap = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AlwaysHardSnap"));
		const bool bHardSnap =
			LinDiffSize > MaxLinearHardSnapDistance ||
			PhysicsTarget.AccumulatedErrorSeconds > ErrorAccumulationSeconds ||
			CVarAlwaysHardSnap->GetInt();

		const FTransform IdealWorldTM(TargetQuat, TargetPos);

		if (bHardSnap)
		{
#if !UE_BUILD_SHIPPING
			if (PhysicsReplicationCVars::LogPhysicsReplicationHardSnaps && GetOwningWorld())
			{
				UE_LOGF(LogTemp, Warning, "Simulated HARD SNAP - \nCurrent Pos - %ls, Target Pos - %ls\n CurrentState.LinVel - %ls, New Lin Vel - %ls\nTarget Extrapolation Delta - %ls, Is Replay? - %d, Is Asleep - %d, Prev Progress - %f, Prev Similarity - %f",
					*CurrentState.Position.ToString(), *TargetPos.ToString(), *CurrentState.LinVel.ToString(), *NewState.LinVel.ToString(),
					*ExtrapolationDeltaPos.ToString(), GetOwningWorld()->IsPlayingReplay(), !BI->IsInstanceAwake(), PrevProgress, PrevSimilarity);
				if (bDidHardSnap)
				{
					*bDidHardSnap = true;
				}
				if (LinDiffSize > MaxLinearHardSnapDistance)
				{
					UE_LOGF(LogTemp, Warning, "Hard snap due to linear difference error");
				}
				else
				{
					UE_LOGF(LogTemp, Warning, "Hard snap due to accumulated error")
				}
			}
#endif

			PhysicsTarget.AccumulatedErrorSeconds = 0.0f;
			bRestoredState = true;

			bool bPTHardSnapSuccess = false;

			if (PhysicsReplicationAsyncVR != nullptr)
			{
				if (bHardsnapLegacyInPT)
				{
					if (Chaos::FSingleParticlePhysicsProxy* Proxy = static_cast<Chaos::FSingleParticlePhysicsProxy*>(BI->GetPhysicsActor()))
					{
						if (Chaos::FPBDRigidsSolver* Solver = Proxy->GetSolver<Chaos::FPBDRigidsSolver>())
						{
							Solver->EnqueueCommandImmediate([Solver, Proxy, IdealWorldTM, NewState, bCorrectConnectedBodies, bCorrectConnectedBodiesFriction]()
								{
									Chaos::FRigidBodyHandle_Internal* Handle = Proxy->GetPhysicsThreadAPI();

									Solver->GetEvolution()->ApplyParticleTransformCorrection(Proxy->GetHandle_LowLevel(), IdealWorldTM.GetLocation(), IdealWorldTM.GetRotation(), bCorrectConnectedBodies, bCorrectConnectedBodiesFriction);

									Handle->SetV(NewState.LinVel);
									Handle->SetW(FMath::DegreesToRadians(NewState.AngVel));
								});

							bPTHardSnapSuccess = true;
						}
					}
				}
			}

			if (!bPTHardSnapSuccess)
			{
				BI->SetBodyTransform(IdealWorldTM, ETeleportType::ResetPhysics, bAutoWake);

				BI->SetLinearVelocity(NewState.LinVel, false, bAutoWake);
				BI->SetAngularVelocityInRadians(FMath::DegreesToRadians(NewState.AngVel), false, bAutoWake);
			}

			BI->SetLinearVelocity(NewState.LinVel, false, bAutoWake);
			BI->SetAngularVelocityInRadians(FMath::DegreesToRadians(NewState.AngVel), false, bAutoWake);
		}
		else
		{

			if (PhysicsReplicationAsyncVR == nullptr)	
			{
				const FVector NewLinVel = FVector(NewState.LinVel) + (LinDiff * LinearVelocityCoefficient * DeltaSeconds);
				const FVector NewAngVel = FVector(NewState.AngVel) + (AngDiffAxis * AngDiff * AngularVelocityCoefficient * DeltaSeconds);

				const FVector NewPos = FMath::Lerp(FVector(CurrentState.Position), FVector(TargetPos), PositionLerp);
				const FQuat NewAng = FQuat::Slerp(CurrentState.Quaternion, TargetQuat, AngleLerp);

				BI->SetBodyTransform(FTransform(NewAng, NewPos), ETeleportType::ResetPhysics);
				BI->SetLinearVelocity(NewLinVel, false);
				BI->SetAngularVelocityInRadians(FMath::DegreesToRadians(NewAngVel), false);
			}
			else
			{

				FPhysicsRepAsyncInputData AsyncInputData(nullptr);
				AsyncInputData.TargetState = NewState;
				AsyncInputData.TargetState.Position = IdealWorldTM.GetLocation();
				AsyncInputData.TargetState.Quaternion = IdealWorldTM.GetRotation();
				AsyncInputData.Proxy = static_cast<Chaos::FSingleParticlePhysicsProxy*>(BI->GetPhysicsActor());
				AsyncInputData.ErrorCorrection = { ErrorCorrection.LinearVelocityCoefficient, ErrorCorrection.AngularVelocityCoefficient, ErrorCorrection.PositionLerp, ErrorCorrection.AngleLerp };

				AsyncInputData.LatencyOneWay = PingSeconds;

				AsyncInputVR->InputData.Add(AsyncInputData);

			}
		}

#if !UE_BUILD_SHIPPING
		static const auto CVarNetShowCorrections = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetShowCorrections"));
		if (CVarNetShowCorrections->GetInt() != 0)
		{
			PhysicsTarget.ErrorHistory.bAutoAdjustMinMax = false;
			PhysicsTarget.ErrorHistory.MinValue = 0.0f;
			PhysicsTarget.ErrorHistory.MaxValue = 1.0f;
			PhysicsTarget.ErrorHistory.AddSample(PhysicsTarget.AccumulatedErrorSeconds / ErrorAccumulationSeconds);
			if (UWorld* OwningWorld = GetOwningWorld())
			{
				FColor Color = FColor::White;
				static const auto CVarNetCorrectionLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetCorrectionLifetime"));
				DrawDebugDirectionalArrow(OwningWorld, CurrentState.Position, TargetPos, 5.0f, Color, false, CVarNetCorrectionLifetime->GetFloat(), 0, 1.5f);
#if 0

				DrawDebugFloatHistory(*OwningWorld, PhysicsTarget.ErrorHistory, NewPos + FVector(0.0f, 0.0f, 100.0f), FVector2D(100.0f, 50.0f), FColor::White);
#endif
			}
		}
#endif
	}

	if (bShouldSleep)
	{

		if (PhysicsReplicationAsyncVR == nullptr)
		{
			BI->PutInstanceToSleep();
		}
	}

	PhysicsTarget.PrevPosTarget = TargetPos;
	PhysicsTarget.PrevPos = FVector(CurrentState.Position);

	return bRestoredState;
}

void FPhysicsReplicationVR::Tick(float DeltaSeconds)
{

	OnTick(DeltaSeconds, ComponentToTargetsVR_DEPRECATED);
}

void FPhysicsReplicationVR::OnTick(float DeltaSeconds, TMap<TWeakObjectPtr<UPrimitiveComponent>, FReplicatedPhysicsTarget>& ComponentsToTargets)
{
	using namespace Chaos;

	if (const UWorld* World = GetOwningWorld())
	{
		if (World->GetNetMode() == ENetMode::NM_Client)
		{
			return FPhysicsReplication::OnTick(DeltaSeconds, ComponentsToTargets);
		}
	}

	if (ShouldSkipPhysicsReplication())
	{
		return;
	}

	if (ComponentsToTargets.Num() == 0 && ReplicatedTargetsQueueVR.Num() == 0)
	{
		return;
	}

	using namespace Chaos;

	NetworkPhysicsTickOffsetAssignedVR = false;
	NetworkPhysicsTickOffsetVR = 0; 

	if (UPhysicsSettings::Get()->PhysicsPrediction.bEnablePhysicsPrediction)
	{
		if (UWorld* World = GetOwningWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				NetworkPhysicsTickOffsetAssignedVR = PlayerController->GetNetworkPhysicsTickOffsetAssigned();
				NetworkPhysicsTickOffsetVR = PlayerController->GetNetworkPhysicsTickOffset();

			}
		}
	}

	const FRigidBodyErrorCorrection& PhysicErrorCorrection = UPhysicsSettings::Get()->PhysicErrorCorrection;
	if (PhysicsReplicationAsyncVR)
	{
		PrepareAsyncData_ExternalVR(PhysicErrorCorrection);
		if (ensure(AsyncInputVR))
		{
			AsyncInputVR->NetworkPhysicsTickOffsetAssigned = NetworkPhysicsTickOffsetAssignedVR;
			AsyncInputVR->NetworkPhysicsTickOffset = NetworkPhysicsTickOffsetVR;
		}
	}

	const float LocalPing = 0.0f;

	for (auto Itr = ComponentsToTargets.CreateIterator(); Itr; ++Itr)
	{
		bool bRemoveItr = false;
		if (UPrimitiveComponent* PrimComp = Itr.Key().Get())
		{		
			if (PrimComp->GetAttachParent() == nullptr)
			{
				if (FBodyInstance* BI = PrimComp->GetBodyInstance(Itr.Value().BoneName))
				{
					FReplicatedPhysicsTarget& PhysicsTarget = Itr.Value();
					FRigidBodyState& UpdatedState = PhysicsTarget.TargetState;
					bool bUpdated = false;
					if (AActor* OwningActor = PrimComp->GetOwner())
					{

						{

							 float OwnerPing = 0.0f;

							const float PingSecondsOneWay = 0.0f;

							if (UpdatedState.Flags & ERigidBodyFlags::NeedsUpdate)
							{
								const int32 LocalFrame = PhysicsTarget.ServerFrame - NetworkPhysicsTickOffsetVR;
								const bool bRestoredState = ApplyRigidBodyState(DeltaSeconds, BI, PhysicsTarget, PhysicErrorCorrection, PingSecondsOneWay, LocalFrame, 0);

								static const auto CVarSkipSkeletalRepOptimization = IConsoleManager::Get().FindConsoleVariable(TEXT("p.SkipSkeletalRepOptimization"));
								if (CVarSkipSkeletalRepOptimization->GetInt() == 0 || Cast<USkeletalMeshComponent>(PrimComp) == nullptr)	
								{
									PrimComp->SyncComponentToRBPhysics();
								}
								if (bRestoredState)
								{
									bRemoveItr = true;
								}
							}
						}
					}
				}
			}
		}

		if (bRemoveItr)
		{
			OnTargetRestored(Itr.Key().Get(), Itr.Value());
			PendingDeleteFromComponentsToTargetsVR.Add(Itr.Key());

		}
	}

	for (TWeakObjectPtr<UPrimitiveComponent>& PrimitiveComponent : PendingDeleteFromComponentsToTargetsVR)
	{
		ComponentsToTargets.Remove(PrimitiveComponent);
	}
	PendingDeleteFromComponentsToTargetsVR.Reset();

	if (AsyncInputVR)
	{

		for (FReplicatedPhysicsTarget& PhysicsTarget : ReplicatedTargetsQueueVR)
		{
			const float PingSecondsOneWay = LocalPing * 0.5f * 0.001f;

			FPhysicsRepAsyncInputData AsyncInputData(PhysicsTarget.PhysicsObject);
			AsyncInputData.TargetState = PhysicsTarget.TargetState;
			AsyncInputData.Proxy = nullptr;
			AsyncInputData.RepMode = PhysicsTarget.ReplicationMode;
			AsyncInputData.ServerFrame = PhysicsTarget.ServerFrame;
			AsyncInputData.LatencyOneWay = PingSecondsOneWay;

			AsyncInputVR->InputData.Add(AsyncInputData);
		}
	}
	ReplicatedTargetsQueueVR.Reset();

	AsyncInputVR = nullptr;
}

FRepMovementVR::FRepMovementVR() : FRepMovement()
{
	LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
	VelocityQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
	RotationQuantizationLevel = ERotatorQuantization::ShortComponents;
}

FRepMovementVR::FRepMovementVR(FRepMovement& other) : FRepMovement()
{
	FRepMovementVR();

	LinearVelocity = other.LinearVelocity;
	AngularVelocity = other.AngularVelocity;
	Location = other.Location;
	Rotation = other.Rotation;
	bSimulatedPhysicSleep = other.bSimulatedPhysicSleep;
	bRepPhysics = other.bRepPhysics;
}

void FRepMovementVR::CopyTo(FRepMovement& other) const
{
	other.LinearVelocity = LinearVelocity;
	other.AngularVelocity = AngularVelocity;
	other.Location = Location;
	other.Rotation = Rotation;
	other.bSimulatedPhysicSleep = bSimulatedPhysicSleep;
	other.bRepPhysics = bRepPhysics;
}

bool FRepMovementVR::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	return FRepMovement::NetSerialize(Ar, Map, bOutSuccess);
}

bool FRepMovementVR::GatherActorsMovement(AActor* OwningActor)
{

	{
		UPrimitiveComponent* RootPrimComp = Cast<UPrimitiveComponent>(OwningActor->GetRootComponent());
		if (RootPrimComp && RootPrimComp->IsSimulatingPhysics())
		{
			FRigidBodyState RBState;
			RootPrimComp->GetRigidBodyState(RBState);

			FillFrom(RBState, OwningActor);

			bRepPhysics = !RootPrimComp->IsWelded();
		}
		else if (RootPrimComp != nullptr)
		{

			if (RootPrimComp->GetAttachParent() != nullptr)
			{
				return false; 

			}
			else
			{
				Location = FRepMovement::RebaseOntoZeroOrigin(RootPrimComp->GetComponentLocation(), OwningActor);
				Rotation = RootPrimComp->GetComponentRotation();
				LinearVelocity = OwningActor->GetVelocity();
				AngularVelocity = FVector::ZeroVector;
			}

			bRepPhysics = false;
		}
	}

	return true;
}

#pragma region FPhysicsReplicationAsync

void FPhysicsReplicationAsyncVR::OnPhysicsObjectUnregistered_Internal(Chaos::FConstPhysicsObjectHandle PhysicsObject)
{
	RemoveObjectFromReplication(PhysicsObject);

	ObjectToSettings.Remove(PhysicsObject);
	RemoveParticleSimDecaySettings(PhysicsObject);

	Chaos::FReadPhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetRead();
	if (Chaos::FGeometryParticleHandle* Handle = Interface.GetParticle(PhysicsObject))
	{
		if (Chaos::FPhysicsSolverBase* SolverBase = GetSolver())
		{
			if (Chaos::FRewindData* RewindData = SolverBase->GetRewindData())
			{

				RewindData->ForceResimAsFollower(Handle, false);
			}
		}
	}
}

void FPhysicsReplicationAsyncVR::RegisterSettings(Chaos::FConstPhysicsObjectHandle PhysicsObject, TWeakPtr<const FNetworkPhysicsSettingsData> InSettings)
{
	if (PhysicsObject != nullptr)
	{
		TWeakPtr<const FNetworkPhysicsSettingsData>& Settings = ObjectToSettings.FindOrAdd(PhysicsObject);
		Settings = InSettings;
	}
}

TWeakPtr<FParticleSimDecaySettings> FPhysicsReplicationAsyncVR::FindOrAddParticleSimDecaySettings(Chaos::FConstPhysicsObjectHandle PhysicsObject)
{
	if (PhysicsObject == nullptr)
	{
		return nullptr;
	}

	TSharedPtr<FParticleSimDecaySettings>& Entry = ParticleSimDecaySettings.FindOrAdd(PhysicsObject);
	if (!Entry.IsValid())
	{
		Entry = MakeShared<FParticleSimDecaySettings>();
	}
	return Entry.ToWeakPtr();
}

void FPhysicsReplicationAsyncVR::RemoveParticleSimDecaySettings(Chaos::FConstPhysicsObjectHandle PhysicsObject)
{
	ParticleSimDecaySettings.Remove(PhysicsObject);
}

void FPhysicsReplicationAsyncVR::FetchObjectSettings(Chaos::FConstPhysicsObjectHandle PhysicsObject)
{
	TWeakPtr<const FNetworkPhysicsSettingsData>* CustomSettings = ObjectToSettings.Find(PhysicsObject);
	SettingsCurrent = (CustomSettings && (*CustomSettings).IsValid()) ? *(*CustomSettings).Pin().Get() : SettingsDefault;
}

void FPhysicsReplicationAsyncVR::OnPostInitialize_Internal()
{
	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (RigidsSolver == nullptr)
	{
		return;
	}

	RigidsSolver->SetPhysicsReplication_Internal(this);
}

void FPhysicsReplicationAsyncVR::AddResimulationRequest_Internal(const float DeltaSeconds)
{

	if (FPhysicsReplicationVR::ShouldSkipPhysicsReplication())
	{
		return;
	}

	const FPhysicsReplicationAsyncInput* AsyncInputVR = GetConsumerInput_Internal();
	if (!AsyncInputVR)
	{
		return;
	}

	NetworkPhysicsTickOffsetAssigned = AsyncInputVR->NetworkPhysicsTickOffsetAssigned;
	NetworkPhysicsTickOffset = AsyncInputVR->NetworkPhysicsTickOffset;

	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	check(RigidsSolver);
	Chaos::FRewindData* RewindData = RigidsSolver->GetRewindData();

	Chaos::FReadPhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetRead();
	const int32 CurrentSolverFrame = RigidsSolver->GetCurrentFrame();

	for (const FPhysicsRepAsyncInputData& InputData : AsyncInputVR->InputData)
	{
		if (InputData.TargetState.Flags == ERigidBodyFlags::None)
		{
			RemoveObjectFromReplication(InputData.PhysicsObject);
			continue;
		}

		UpdateRewindDataTarget(InputData);
		UpdateAsyncTarget(InputData, RigidsSolver);
	}

	for (const FPhysicsRepAsyncInputData& InputData : AsyncInputVR->InputData)
	{
		if (InputData.TargetState.Flags == ERigidBodyFlags::None)
		{
			continue;
		}

		FReplicatedPhysicsTargetAsync* Target = ObjectToTarget.Find(InputData.PhysicsObject);
		if (!Target)
		{
			continue;
		}

		if (Target->LastLODFrame == CurrentSolverFrame)
		{

			continue;
		}

		Target->SimDecayTimeScale = 1.0f;
		ApplyPhysicsReplicationLOD(InputData.PhysicsObject, *Target, EPhysicsReplicationLODFlags::LODFlag_All);

		if (Target->RepMode != EPhysicsReplicationMode::Resimulation)
		{
			continue;
		}

		if (!RewindData)
		{
			RewindData = GetOrEnableRewindData(RigidsSolver);
		}

		if (!RewindData || !IsTargetValidForResim(*Target))
		{
			continue;
		}

		Chaos::FPBDRigidParticleHandle* RigidHandle = Interface.GetRigidParticle(InputData.PhysicsObject);
		if (!RigidHandle)
		{
			continue;

		}

		const int32 LocalFrame = Target->ServerFrame - Target->FrameOffset;
		if (ShouldResimulateParticle(RigidHandle, *Target, DeltaSeconds) && LocalFrame > RewindData->GetBlockedResimFrame())
		{
			RewindData->RequestResimulation(LocalFrame, RigidHandle);
		}
	}
}

void FPhysicsReplicationAsyncVR::OnPreSimulate_Internal()
{
	const FPhysicsReplicationAsyncInput* AsyncInputVR = GetConsumerInput_Internal();

	if (AsyncInputVR)
	{
		NetworkPhysicsTickOffsetAssigned = AsyncInputVR->NetworkPhysicsTickOffsetAssigned;
		NetworkPhysicsTickOffset = AsyncInputVR->NetworkPhysicsTickOffset;
	}

	if (FPhysicsReplication::ShouldSkipPhysicsReplication())
	{
		return;
	}

	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	check(RigidsSolver);

	ResimErrorLogTimer += RigidsSolver->GetAsyncDeltaTime();

	Chaos::FRewindData* RewindData = RigidsSolver->GetRewindData();
	bool bRewindDataExist = RewindData != nullptr;
	if (bRewindDataExist && RewindData->IsResim())
	{

		if (SettingsCurrent.PredictiveInterpolationSettings.GetPostResimWaitForUpdate() && RewindData->IsFinalResim())
		{
			for (auto Itr = ObjectToTarget.CreateIterator(); Itr; ++Itr)
			{
				FReplicatedPhysicsTargetAsync& Target = Itr.Value();

				if (Target.RepMode == EPhysicsReplicationMode::PredictiveInterpolation)
				{
					Target.SetWaiting(RigidsSolver->GetCurrentFrame() + Target.FrameOffset, Target.RepModeOverride);
				}
			}
		}
		return;
	}

	if (AsyncInputVR)
	{
		for (const FPhysicsRepAsyncInputData& Input : AsyncInputVR->InputData)
		{
			if (Input.TargetState.Flags == ERigidBodyFlags::None)
			{
				RemoveObjectFromReplication(Input.PhysicsObject);
				continue;
			}

			if (Input.ServerFrame > 0)
			{
				UE::PhysicsReplicationQuantization::VerifyTargetOnGrid_Internal(
				GetSolver(), Input.PhysicsObject, Input.ServerFrame,
				Input.TargetState.Position, Input.TargetState.Quaternion,
				Input.TargetState.LinVel, Input.TargetState.AngVel);
			}

			if (Chaos::RewindBeforeAdvance == 0)
			{
				UpdateRewindDataTarget(Input);
				UpdateAsyncTarget(Input, RigidsSolver);
			}

			if (Input.Proxy != nullptr)
			{
				Chaos::FSingleParticlePhysicsProxy* Proxy = Input.Proxy;
				Chaos::FRigidBodyHandle_Internal* Handle = Proxy->GetPhysicsThreadAPI();

				const FPhysicsRepErrorCorrectionData& UsedErrorCorrection = Input.ErrorCorrection.IsSet() ? Input.ErrorCorrection.GetValue() : AsyncInputVR->ErrorCorrection;
				DefaultReplication_DEPRECATED(Handle, Input, GetDeltaTime_Internal(), UsedErrorCorrection);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				static const auto CVarDebugDrawShowRepMode = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Net.DebugDraw.ShowRepMode"));
				if (CVarDebugDrawShowRepMode->GetInt())
				{
					static const auto CVarDebugDrawLifeTime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Net.DebugDraw.LifeTime"));
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(Input.TargetState.Position, FVector(10.0f, 10.0f, 10.0f), Input.TargetState.Quaternion, FColor::Green, false, CVarDebugDrawLifeTime->GetFloat(), 0, 1.0f);
				}
#endif
			}
		}
	}

	if (Chaos::FPBDRigidsSolver::IsNetworkPhysicsPredictionEnabled())
	{
		CacheResimInteractions();
	}

	ApplyTargetStatesAsync(GetDeltaTime_Internal());
}

void FPhysicsReplicationAsyncVR::OnPostIntegrate_Internal()
{
	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (!RigidsSolver)
	{
		return;
	}

	Chaos::FRewindData* RewindData = RigidsSolver->GetRewindData();
	const bool bIsResim = RewindData && RewindData->IsResim();

	Chaos::FWritePhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetWrite();

	auto ApplyDecay = [&](Chaos::FConstPhysicsObjectHandle Handle, float TimeScale)
		{
			if (TimeScale >= 1.0f)
			{
				return;
			}

			Chaos::FPBDRigidParticleHandle* Particle = Interface.GetRigidParticle(Handle);
			if (!Particle || !Particle->IsDynamic())
			{
				return;
			}

			const Chaos::FVec3 PrePos = Particle->GetTransformXRCom().GetLocation();
			const Chaos::FRotation3 PreRot = Particle->GetTransformXRCom().GetRotation();
			const Chaos::FVec3 PreVel = Particle->GetPreV();
			const Chaos::FVec3 PreAngVel = Particle->GetPreW();

			const Chaos::FVec3 PostPos = Particle->GetTransformPQCom().GetLocation();
			const Chaos::FRotation3 PostRot = Particle->GetTransformPQCom().GetRotation();
			const Chaos::FVec3 PostVel = Particle->GetV();
			const Chaos::FVec3 PostAngVel = Particle->GetW();

			Particle->SetTransformPQCom(
				PrePos + TimeScale * (PostPos - PrePos),
				Chaos::FRotation3::Slerp(PreRot, PostRot, TimeScale).GetNormalized());
			Particle->SetV(PreVel + TimeScale * (PostVel - PreVel));
			Particle->SetW(PreAngVel + TimeScale * (PostAngVel - PreAngVel));

			static const auto CVarbTransitionModeDebugLog = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ReplicationLOD.TransitionMode.DebugLog"));
			if (CVarbTransitionModeDebugLog->GetBool())
			{
				const double DeltaLen = (PostPos - PrePos).Size();
				const double ScaledLen = DeltaLen * TimeScale;
				UE_LOGF(LogPhysics, Log, "[ReplicationLOD] Decay | Frame=%d Resim=%d TimeScale=%.3f | DeltaLen=%.3f ScaledLen=%.3f | PrePos=%.2f PostPos=%.2f ResultPos=%.2f"
					, RigidsSolver->GetCurrentFrame(), bIsResim ? 1 : 0, TimeScale, DeltaLen, ScaledLen, PrePos.Size(), PostPos.Size(), (PrePos + TimeScale * (PostPos - PrePos)).Size());
			}
		};

	if (bIsResim)
	{

		if (RigidsSolver->GetCurrentFrame() == RewindData->GetResimFrame())
		{
			const float ResimFrames = static_cast<float>(RewindData->GetLatestFrame() - RewindData->GetResimFrame());

			for (const TPair<Chaos::FConstPhysicsObjectHandle, TSharedPtr<FParticleSimDecaySettings>>& Pair : ParticleSimDecaySettings)
			{
				FReplicatedPhysicsTargetAsync* ClampTarget = ObjectToTarget.Find(Pair.Key);
				if (!ClampTarget)
				{
					continue;
				}

				const TSharedPtr<FParticleSimDecaySettings>& Settings = Pair.Value;

				float TimeScale = Settings->StaticTimeScale;
				if (Settings->bUseDynamicTimeScale && ResimFrames > 0.0f)
				{
					TimeScale = FMath::Clamp(
						Settings->DynamicBase + (ResimFrames - Settings->InputPredictionFramesAverage) / ResimFrames,
						Settings->DynamicMin, Settings->DynamicMax);
				}

				ClampTarget->SimDecayTimeScale = FMath::Min(ClampTarget->SimDecayTimeScale, TimeScale);

				static const auto CVarbTransitionModeDebugLog = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ReplicationLOD.TransitionMode.DebugLog"));
				if (CVarbTransitionModeDebugLog->GetBool())
				{
					UE_LOGF(LogPhysics, Log, "[ReplicationLOD] Clamp | ActualResimFrames=%.0f InputPredAvg=%.2f UseDynamic=%d | TimeScale=%.3f SimDecayTimeScale=%.3f (post min-merge)"
						, ResimFrames, Settings->InputPredictionFramesAverage, Settings->bUseDynamicTimeScale ? 1 : 0
						, TimeScale, ClampTarget->SimDecayTimeScale);
				}
			}
		}

		for (const TPair<Chaos::FConstPhysicsObjectHandle, FReplicatedPhysicsTargetAsync>& Pair : ObjectToTarget)
		{
			ApplyDecay(Pair.Key, Pair.Value.SimDecayTimeScale);
		}
	}
	else
	{

		for (const TPair<Chaos::FConstPhysicsObjectHandle, TSharedPtr<FParticleSimDecaySettings>>& Pair : ParticleSimDecaySettings)
		{
			if (!Pair.Value->bApplyDecayAtRuntime)
			{
				continue;
			}

			const FReplicatedPhysicsTargetAsync* Target = ObjectToTarget.Find(Pair.Key);
			if (Target && Target->RepMode != EPhysicsReplicationMode::Resimulation)
			{
				continue;
			}

			ApplyDecay(Pair.Key, Pair.Value->StaticTimeScale);
		}
	}
}

void FPhysicsReplicationAsyncVR::OnPostSolve_Internal()
{
	static const auto CVarKeepResimStateForNonResimReplicatedObjects = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.Resim.KeepResimStateForNonResimReplicatedObjects"));
	if (FPhysicsReplication::ShouldSkipPhysicsReplication() || CVarKeepResimStateForNonResimReplicatedObjects->GetBool())
	{
		return;
	}

	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	check(RigidsSolver);

	Chaos::FRewindData* RewindData = RigidsSolver->GetRewindData();
	if (RewindData && RewindData->IsFinalResim())
	{
		Chaos::FReadPhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetRead();
		for (auto Itr = ObjectToTarget.CreateIterator(); Itr; ++Itr)
		{
			FReplicatedPhysicsTargetAsync& Target = Itr.Value();
			if (Target.RepMode == EPhysicsReplicationMode::Default || Target.RepMode == EPhysicsReplicationMode::PredictiveInterpolation)
			{
				Chaos::FConstPhysicsObjectHandle& PhysicsObject = Itr.Key();
				if (Chaos::FGeometryParticleHandle* GeometryParticle = Interface.GetParticle(PhysicsObject))
				{
					Chaos::FParticleSimpleState* PreResimState = nullptr;
					RewindData->GetPreResimState(GeometryParticle, PreResimState);
					if (PreResimState)
					{
						if (Chaos::FPBDRigidParticleHandle* RigidParticle = GeometryParticle->CastToRigidParticle())
						{

							RigidParticle->SetP(PreResimState->GetX());
							RigidParticle->SetQ(PreResimState->GetR());
							RigidParticle->SetV(PreResimState->GetV());
							RigidParticle->SetW(PreResimState->GetW());
						}
						else if (Chaos::FKinematicGeometryParticleHandle* KinematicParticle = GeometryParticle->CastToKinematicParticle())
						{

							KinematicParticle->SetX(PreResimState->GetX());
							KinematicParticle->SetR(PreResimState->GetR());
							KinematicParticle->SetV(PreResimState->GetV());
							KinematicParticle->SetW(PreResimState->GetW());
						}

						RewindData->RemovePreResimState(GeometryParticle);
					}
				}
			}
		}
	}
}

FReplicatedPhysicsTargetAsync* FPhysicsReplicationAsyncVR::AddObjectToReplication(Chaos::FConstPhysicsObjectHandle PhysicsObject)
{
	if (ensure(PhysicsObject))
	{

		Chaos::FReadPhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetRead();
		if (Chaos::FGeometryParticleHandle* Handle = Interface.GetParticle(PhysicsObject))
		{
			ReplicatedParticleIDs.Add(Handle->ParticleID());
		}

		return &ObjectToTarget.Add(PhysicsObject, FReplicatedPhysicsTargetAsync());
	}
	return nullptr;
}

void FPhysicsReplicationAsyncVR::RemoveObjectFromReplication(Chaos::FConstPhysicsObjectHandle PhysicsObject)
{
	if (PhysicsObject == nullptr)
	{
		return;
	}

	ObjectToTarget.Remove(PhysicsObject);

	Chaos::FReadPhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetRead();
	if (Chaos::FGeometryParticleHandle* Handle = Interface.GetParticle(PhysicsObject))
	{
		ReplicatedParticleIDs.Remove(Handle->ParticleID());
	}
}

void FPhysicsReplicationAsyncVR::UpdateRewindDataTarget(const FPhysicsRepAsyncInputData& Input)
{
	if (Input.PhysicsObject == nullptr)
	{
		return;
	}

	if (NetworkPhysicsTickOffsetAssigned == false)
	{
		return;
	}

	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (RigidsSolver == nullptr)
	{
		return;
	}

	Chaos::FRewindData* RewindData = RigidsSolver->GetRewindData();
	if (RewindData == nullptr)
	{
		return;
	}

	Chaos::FReadPhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetRead();
	if (Chaos::FGeometryParticleHandle* Handle = Interface.GetParticle(Input.PhysicsObject))
	{

		const int32 LocalFrame = Input.ServerFrame - NetworkPhysicsTickOffset;
		RewindData->SetTargetStateAtFrame(*Handle, LocalFrame, Chaos::FFrameAndPhase::EParticleHistoryPhase::PrePushData,
			Input.TargetState.Position, Input.TargetState.Quaternion,
			Input.TargetState.LinVel, FMath::DegreesToRadians(Input.TargetState.AngVel), (Input.TargetState.Flags & ERigidBodyFlags::Sleeping));

		RewindData->ForceResimAsFollower(Handle,  Input.RepMode == EPhysicsReplicationMode::None);
	}
}

void FPhysicsReplicationAsyncVR::UpdateAsyncTarget(const FPhysicsRepAsyncInputData& Input, Chaos::FPBDRigidsSolver* RigidsSolver)
{
	if (Input.PhysicsObject == nullptr)
	{
		return;
	}

	FReplicatedPhysicsTargetAsync* Target = ObjectToTarget.Find(Input.PhysicsObject);
	bool bFirstTarget = Target == nullptr;
	if (bFirstTarget)
	{

		Target = AddObjectToReplication(Input.PhysicsObject);
		Target->PrevPos = Input.TargetState.Position;
		Target->PrevPosTarget = Input.TargetState.Position;
		Target->PrevRotTarget = Input.TargetState.Quaternion;
		Target->PrevLinVel = Input.TargetState.LinVel;
		Target->RepModeOverride = Input.RepMode;
	}
	check(Target);

	if ((bFirstTarget || Input.ServerFrame == 0 || Input.ServerFrame > Target->ServerFrame))
	{

		const int32 CurrentFrame = RigidsSolver->GetCurrentFrame();

		const int32 PrevTickCount = (Target->ServerFrame < 0) ? 0 : Target->TickCount;

		const int32 SendInterval = (Target->ServerFrame <= 0) ? 0 : Input.ServerFrame - Target->ServerFrame;

		const bool bPrevAllowTargetAltering = Target->bAllowTargetAltering;

		const bool bFrameOffsetCorrected = Target->FrameOffset != NetworkPhysicsTickOffset;

		Target->bAllowTargetAltering = !(Target->TargetState.Flags & ERigidBodyFlags::Sleeping) && !(Input.TargetState.Flags & ERigidBodyFlags::Sleeping);

		const FVector PrevLinVel = Target->TargetState.LinVel;

		if (SendInterval > 0)
		{
			Target->ReceiveInterval = SendInterval;
		}
		else
		{
			const int32 PrevReceiveFrame = Target->ReceiveFrame < 0 ? (CurrentFrame - 1) : Target->ReceiveFrame;
			Target->ReceiveInterval = (CurrentFrame - PrevReceiveFrame);
		}

		Target->ServerFrame = Input.ServerFrame;
		Target->ReceiveFrame = CurrentFrame;
		Target->TargetState = Input.TargetState;
		Target->RepMode = Input.RepMode;
		Target->FrameOffset = NetworkPhysicsTickOffset;
		Target->TickCount = 0;
		Target->AccumulatedSleepSeconds = 0.0f;
		Target->SimDecayTimeScale = 1.0f;

		Target->UpdateWaiting(Input.ServerFrame);

		Target->bFrameOffsetCorrectedCached = bFrameOffsetCorrected;
		Target->PrevTickCountCached = PrevTickCount;
		Target->bPrevAllowTargetAlteringCached = bPrevAllowTargetAltering;

		static const auto CVarTeleportDetectionEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.TeleportDetection.Enabled"));
		if (CVarTeleportDetectionEnabled->GetInt() == 1 && !bFirstTarget && SendInterval > 0 && RigidsSolver->IsUsingFixedDt())
		{
			const FVector PosOffset = (Input.TargetState.Position - Target->PrevPosTarget);
			static const auto CVarTeleportDetectionMinDistance = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.TeleportDetection.MinDistance"));
			if (PosOffset.SizeSquared() > (CVarTeleportDetectionMinDistance->GetFloat() * CVarTeleportDetectionMinDistance->GetFloat()))
			{
				const FVector Velocity = Input.TargetState.LinVel.SizeSquared() > PrevLinVel.SizeSquared() ? Input.TargetState.LinVel : PrevLinVel;
				const float DeltaSeconds = (SendInterval * RigidsSolver->GetAsyncDeltaTime());

				static const auto CVarTeleportDetectionVelocityMultiplier = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.TeleportDetection.VelocityMultiplier"));
				const float PossibleDistanceSquared = (Velocity * (DeltaSeconds * CVarTeleportDetectionVelocityMultiplier->GetFloat())).SizeSquared();

				if (PossibleDistanceSquared < PosOffset.SizeSquared())
				{

					static const auto CVarErrorAccumulationSeconds = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorAccumulationSeconds"));
					Target->AccumulatedErrorSeconds = CVarErrorAccumulationSeconds->GetFloat() + 1.0f;
				}
			}
		}

		Target->PrevPosTarget = Input.TargetState.Position;
		Target->PrevRotTarget = Input.TargetState.Quaternion;
	}

	LatencyOneWay = Input.LatencyOneWay;
}

void FPhysicsReplicationAsyncVR::CacheResimInteractions()
{
	static const auto CVarResimDisableReplicationOnInteraction = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.Resim.DisableReplicationOnInteraction"));
	if (!CVarResimDisableReplicationOnInteraction->GetBool())
	{
		ParticlesInResimIslands.Empty();
		return;
	}

	if (UsePhysicsReplicationLOD())
	{

		ParticlesInResimIslands.Empty();
		return;
	}

	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (RigidsSolver == nullptr)
	{
		return;
	}

	ResimIslands.Reset();
	ResimIslandsParticles.Reset();
	ParticlesInResimIslands.Reset();

	Chaos::Private::FPBDIslandManager& IslandManager = RigidsSolver->GetEvolution()->GetIslandManager();
	Chaos::FReadPhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetRead();
	for (auto Itr = ObjectToTarget.CreateIterator(); Itr; ++Itr)
	{
		FReplicatedPhysicsTargetAsync& Target = Itr.Value();
		if (Target.RepMode == EPhysicsReplicationMode::Resimulation)
		{
			Chaos::FConstPhysicsObjectHandle& POHandle = Itr.Key();
			if (Chaos::FGeometryParticleHandle* Handle = Interface.GetParticle(POHandle))
			{

				IslandManager.FindParticleIslands(Handle, OUT ResimIslands);
				IslandManager.FindParticlesInIslands(ResimIslands, OUT ResimIslandsParticles);
				for (const Chaos::FGeometryParticleHandle* InteractParticle : ResimIslandsParticles)
				{
					ParticlesInResimIslands.Add(InteractParticle->GetHandleIdx());
				}
			}
		}
	}
}

void FPhysicsReplicationAsyncVR::ApplyTargetStatesAsync(const float DeltaSeconds)
{
	using namespace Chaos;
	using namespace Chaos;
	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (!RigidsSolver)
	{
		return;
	}
	const int32 CurrentSolverFrame = RigidsSolver->GetCurrentFrame();

	auto RemoveTargetHelper = [this](TMap<Chaos::FConstPhysicsObjectHandle, FReplicatedPhysicsTargetAsync>::TIterator Itr, FGeometryParticleHandle* Handle)
		{
			if (Handle)
			{
				ReplicatedParticleIDs.Remove(Handle->ParticleID());
			}
			PendingDeleteFromObjectToTarget.Add(Itr.Key());
		};

	Chaos::FWritePhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetWrite();
	for (TMap<Chaos::FConstPhysicsObjectHandle, FReplicatedPhysicsTargetAsync>::TIterator Itr = ObjectToTarget.CreateIterator(); Itr; ++Itr)
	{
		bool bRemoveItr = true; 

		Chaos::FConstPhysicsObjectHandle& POHandle = Itr.Key();
		FGeometryParticleHandle* Handle = Interface.GetParticle(POHandle);
		if (!Handle)
		{
			RemoveTargetHelper(Itr, nullptr);
			continue;
		}

		FPBDRigidParticleHandle* RigidHandle = Handle->CastToRigidParticle();
		if (!RigidHandle)
		{
			RemoveTargetHelper(Itr, Handle);
			continue;
		}

		FReplicatedPhysicsTargetAsync& Target = Itr.Value();

		FetchObjectSettings(POHandle);

		const bool bFreshInputThisFrame = (Target.ReceiveFrame == CurrentSolverFrame);

		if (Target.LastLODFrame != CurrentSolverFrame)
		{
			Target.SimDecayTimeScale = 1.0f;
			ApplyPhysicsReplicationLOD(POHandle, Target, EPhysicsReplicationLODFlags::LODFlag_All);
		}

		if (bFreshInputThisFrame)
		{
			CheckTargetResimValidity(Target);
		}

		if (Target.RepMode == EPhysicsReplicationMode::PredictiveInterpolation)
		{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			static const auto CVarbDrawDebugTargets = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.DrawDebugTargets"));
			if (CVarbDrawDebugTargets->GetBool())
			{
				static const auto CVarDrawDebugZOffset = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.DrawDebugZOffset"));
				static const auto CVarDebugdrawLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Net.DebugDraw.LifeTime"));
				const FVector Offset = FVector(0.0f, 0.0f, CVarDrawDebugZOffset->GetFloat());
				Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(
					Target.PrevPosTarget + Offset, FVector(15.0f, 15.0f, 15.0f),
					Target.PrevRotTarget,
					FColor::MakeRandomSeededColor(Target.ServerFrame),
					false, CVarDebugdrawLifetime->GetFloat(), 0, 1.0f);
			}
#endif

			if (bFreshInputThisFrame && Target.TickCount == 0 && !Target.bFrameOffsetCorrectedCached)
			{
				if (Target.bPrevAllowTargetAlteringCached && Target.bAllowTargetAltering && !Target.IsWaiting())
				{
					Target.TickCount = Target.PrevTickCountCached - Target.ReceiveInterval;

					static const auto CVarTargetTickAlignmentClampMultiplier = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.TargetTickAlignmentClampMultiplier"));
					const int32 AdjustedAverageReceiveInterval = FMath::CeilToInt(Target.AverageReceiveInterval) * CVarTargetTickAlignmentClampMultiplier->GetInt();
					const int32 TickAlignment = FMath::Clamp(Target.TickCount, -AdjustedAverageReceiveInterval, AdjustedAverageReceiveInterval);
					FPhysicsReplicationAsyncVR::ExtrapolateTarget(Target, TickAlignment, GetDeltaTime_Internal());
				}
			}
		}

		DebugDrawReplicationMode(POHandle, Target);

		const EPhysicsReplicationMode RepMode = Target.IsWaiting() ? Target.RepModeOverride : Target.RepMode;
		switch (RepMode)
		{
		case EPhysicsReplicationMode::Default:
			bRemoveItr = DefaultReplication(RigidHandle, Target, DeltaSeconds);
			break;

		case EPhysicsReplicationMode::PredictiveInterpolation:
			bRemoveItr = PredictiveInterpolation(RigidHandle, Target, DeltaSeconds);
			break;

		case EPhysicsReplicationMode::Resimulation:
			bRemoveItr = ResimulationReplication(RigidHandle, Target, DeltaSeconds);
			break;

		case EPhysicsReplicationMode::None:
			bRemoveItr = true;
			break;
		}
		Target.TickCount++;

		if (bRemoveItr)
		{
			RemoveTargetHelper(Itr, RigidHandle);
		}
	}

	for (Chaos::FConstPhysicsObjectHandle& PhysicsObject : PendingDeleteFromObjectToTarget)
	{
		ObjectToTarget.Remove(PhysicsObject);
	}
	PendingDeleteFromObjectToTarget.Reset();
}

void FPhysicsReplicationAsyncVR::CheckTargetResimValidity(FReplicatedPhysicsTargetAsync& Target)
{
	if (Target.RepMode != EPhysicsReplicationMode::Resimulation)
	{
		return;
	}

	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (RigidsSolver == nullptr)
	{
		return;
	}

	Chaos::FRewindData* RewindData = GetOrEnableRewindData(RigidsSolver);
	if (RewindData == nullptr)
	{

		Target.RepMode = EPhysicsReplicationMode::PredictiveInterpolation;
		return;
	}

	const int32 LocalFrame = Target.ServerFrame - NetworkPhysicsTickOffset;
	if (!RewindData->IsFrameWithinRewindHistory(LocalFrame))
	{

		static const auto CVarResimApplyPredictiveInterpolationWhenBehindServer = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.Resim.ApplyPredictiveInterpolationWhenBehindServer"));

		if (LocalFrame < RewindData->GetEarliestFrame_Internal())
		{

			Target.RepMode = EPhysicsReplicationMode::PredictiveInterpolation;
		}
		else if (CVarResimApplyPredictiveInterpolationWhenBehindServer->GetBool())
		{

			Target.RepMode = EPhysicsReplicationMode::PredictiveInterpolation;
		}

		if (NetworkPhysicsTickOffsetAssigned)
		{
			if (ResimOutOfBoundsCounter == 0)
			{
				UE_LOGF(LogPhysics, Log, "FPhysicsReplication DESYNCED - received target ClientFrame: %d (ServerFrame: %d - FrameOffset: %d) out of rewind data bounds (%d, %d) - %ls - Target will use %ls"
					, LocalFrame, Target.ServerFrame, NetworkPhysicsTickOffset, RewindData->GetEarliestFrame_Internal(), RewindData->CurrentFrame()
					, (LocalFrame < RewindData->GetEarliestFrame_Internal())
					? TEXT("Client is far ahead of the server, server might be dropping frames.")
					: TEXT("Client is behind the server, client might be dropping frames."), *UEnum::GetValueAsString(Target.RepMode));
			}

			ResimOutOfBoundsCounter++;
			ResimErrorLogTimer = 0;
		}
	}
	else
	{
		static const auto CVarLogOutOfBoundsTimeLimit = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.Resim.LogOutOfBoundsTimeLimit"));

		if (ResimOutOfBoundsCounter > 0 && ResimErrorLogTimer > CVarLogOutOfBoundsTimeLimit->GetFloat())
		{
			UE_LOGF(LogPhysics, Log, "FPhysicsReplication IN-SYNC - Received targets have now been within rewind data bounds again for at least %f seconds", ResimErrorLogTimer);

			ResimOutOfBoundsCounter = 0;
		}
	}
}

Chaos::FRewindData* FPhysicsReplicationAsyncVR::GetOrEnableRewindData(Chaos::FPBDRigidsSolver* RigidsSolver)
{
	if (RigidsSolver == nullptr)
	{
		return nullptr;
	}

	Chaos::FRewindData* RewindData = RigidsSolver->GetRewindData();
	if (!RewindData)
	{
		if (Chaos::FPBDRigidsSolver::IsNetworkPhysicsPredictionEnabled() && RigidsSolver->IsUsingFixedDt())
		{
			RigidsSolver->EnableRewindCapture();
			RewindData = RigidsSolver->GetRewindData();
		}
	}
	return RewindData;
}

void FPhysicsReplicationAsyncVR::ApplyPhysicsReplicationLOD(Chaos::FConstPhysicsObjectHandle PhysicsObjectHandle, FReplicatedPhysicsTargetAsync& Target, const uint32 LODFlags)
{
	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (RigidsSolver == nullptr)
	{
		return;
	}
	const int32 CurrentFrame = RigidsSolver->GetCurrentFrame();

	if (Target.ServerFrame <= 0)
	{
		Target.RepMode = EPhysicsReplicationMode::PredictiveInterpolation;
		Target.SimDecayTimeScale = 1.0f;

		static const auto CVarTargetTickAlignmentClampMultiplier = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.TargetTickAlignmentClampMultiplier"));
		static const auto CVarbTransitionModeDebugLog = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ReplicationLOD.TransitionMode.DebugLog"));
		if (CVarbTransitionModeDebugLog->GetBool())
		{
			Chaos::FReadPhysicsObjectInterface_Internal ReadInterface = Chaos::FPhysicsObjectInternalInterface::GetRead();
			const Chaos::FGeometryParticleHandle* DebugHandle = ReadInterface.GetParticle(PhysicsObjectHandle);
			const FVector ParticlePos = DebugHandle ? DebugHandle->GetX() : FVector::ZeroVector;
			const FVector TargetPos = Target.TargetState.Position;
			UE_LOGF(LogPhysics, Log, "[ReplicationLOD] PreServer | CurrentFrame=%d ServerFrame=%d | ParticlePos=(%.2f,%.2f,%.2f) TargetStatePos=(%.2f,%.2f,%.2f) | RepMode=PredictiveInterpolation"
				, RigidsSolver->GetCurrentFrame(), Target.ServerFrame
				, ParticlePos.X, ParticlePos.Y, ParticlePos.Z
				, TargetPos.X, TargetPos.Y, TargetPos.Z);
		}
		return;
	}

	IPhysicsReplicationLODAsync* PhysRepLod = RigidsSolver->GetPhysicsReplicationLOD_Internal();
	if (!PhysRepLod || !PhysRepLod->IsEnabled())
	{
		return;
	}

	FPhysicsRepLodData* LodData = PhysRepLod->GetLODData_Internal(PhysicsObjectHandle, LODFlags);
	if (!LodData || !LodData->DataAssigned)
	{
		return;
	}

	const int32 LocalFrame = Target.ServerFrame - NetworkPhysicsTickOffset;
	const int32 FullPredictionFrames = CurrentFrame - LocalFrame;
	const float DeltaTime = static_cast<float>(RigidsSolver->GetAsyncDeltaTime());
	const float FullPredictionTime = FullPredictionFrames * DeltaTime;

	const int32 AlignDistance = CurrentFrame - LodData->AlignedFrame;

	if (LodData->AlignedTime >= FullPredictionTime || LodData->AlignedFrame == 0 || FullPredictionFrames <= 0)
	{
		Target.RepMode = EPhysicsReplicationMode::PredictiveInterpolation;
		Target.SimDecayTimeScale = 1.0f;
		Target.LastLODFrame = CurrentFrame;

		static const auto CVarbTransitionModeDebugLog = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ReplicationLOD.TransitionMode.DebugLog"));
		if (CVarbTransitionModeDebugLog->GetBool())
		{
			Chaos::FReadPhysicsObjectInterface_Internal ReadInterface = Chaos::FPhysicsObjectInternalInterface::GetRead();
			const Chaos::FGeometryParticleHandle* DebugHandle = ReadInterface.GetParticle(PhysicsObjectHandle);
			const FVector ParticlePos = DebugHandle ? DebugHandle->GetX() : FVector::ZeroVector;
			const FVector TargetPos = Target.TargetState.Position;
			UE_LOGF(LogPhysics, Log, "[ReplicationLOD] OuterZone | CurrentFrame=%d LocalFrame=%d AlignedFrame=%d | FullPredictionFrames=%d FullPredictionTime=%.4f AlignedTime=%.4f | No extrapolation | ParticlePos=(%.2f,%.2f,%.2f) TargetStatePos=(%.2f,%.2f,%.2f) | RepMode=PredictiveInterpolation"
				, CurrentFrame, LocalFrame, LodData->AlignedFrame
				, FullPredictionFrames, FullPredictionTime, LodData->AlignedTime
				, ParticlePos.X, ParticlePos.Y, ParticlePos.Z
				, TargetPos.X, TargetPos.Y, TargetPos.Z);
		}
		return;
	}

	if (LodData->AlignedTime <= 0.0f)
	{
		Target.RepMode = EPhysicsReplicationMode::Resimulation;
		Target.SimDecayTimeScale = 1.0f;
		Target.LastLODFrame = CurrentFrame;

		static const auto CVarbTransitionModeDebugLog = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ReplicationLOD.TransitionMode.DebugLog"));
		if (CVarbTransitionModeDebugLog->GetBool())
		{
			UE_LOGF(LogPhysics, Log, "[ReplicationLOD] InnerZone | CurrentFrame=%d AlignedFrame=%d AlignedTime=%.4f | RepMode=Resimulation"
				, CurrentFrame, LodData->AlignedFrame, LodData->AlignedTime);
		}
		return;
	}

	static const auto CVarTransitionExtrapFrameMin = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ReplicationLOD.TransitionExtrapFrameMin"));
	static const auto CVarTransitionExtrapFraction = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ReplicationLOD.TransitionExtrapFraction"));

	const int32 ExtrapFramesThreshold = FMath::Max(
		CVarTransitionExtrapFrameMin->GetInt(),
		FMath::CeilToInt(CVarTransitionExtrapFraction->GetFloat() * FullPredictionFrames));
	const int32 ExtrapolationFrameBoundary = LocalFrame + ExtrapFramesThreshold;

	Target.LastLODFrame = CurrentFrame;

	if (LodData->AlignedFrame <= ExtrapolationFrameBoundary)
	{
		Target.RepMode = EPhysicsReplicationMode::PredictiveInterpolation;
		Target.SimDecayTimeScale = 1.0f;

		const bool bShouldSleep = (Target.TargetState.Flags & ERigidBodyFlags::Sleeping) != 0;
		const FVector ServerTargetPos = Target.TargetState.Position; 
		if (!bShouldSleep)
		{
			const float AlignedPredictionTime = FullPredictionTime - LodData->AlignedTime;
			FPhysicsReplicationAsyncVR::ExtrapolateTarget(Target, AlignedPredictionTime);
			Target.TickCount = LodData->AlignedFrame - LocalFrame;
		}

		static const auto CVarbTransitionModeDebugLog = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ReplicationLOD.TransitionMode.DebugLog"));
		if (CVarbTransitionModeDebugLog->GetBool())
		{
			Chaos::FReadPhysicsObjectInterface_Internal ReadInterface = Chaos::FPhysicsObjectInternalInterface::GetRead();
			const Chaos::FGeometryParticleHandle* DebugHandle = ReadInterface.GetParticle(PhysicsObjectHandle);
			const FVector ParticlePos = DebugHandle ? DebugHandle->GetX() : FVector::ZeroVector;
			const FVector ExtrapolatedTargetPos = Target.TargetState.Position;
			const TCHAR* Tag = bShouldSleep ? TEXT("TransitionExtrapolationSleep") : TEXT("TransitionExtrapolation");
			UE_LOGF(LogPhysics, Log, "[ReplicationLOD] %ls | CurrentFrame=%d LocalFrame=%d AlignedFrame=%d | FullPredictionFrames=%d FullPredictionTime=%.4f AlignedTime=%.4f | AlignDistance=%d ExtrapFramesThreshold=%d | ParticlePos=(%.2f,%.2f,%.2f) ServerTargetPos=(%.2f,%.2f,%.2f) ExtrapolatedTargetPos=(%.2f,%.2f,%.2f) | RepMode=PredictiveInterpolation"
				, Tag
				, CurrentFrame, LocalFrame, LodData->AlignedFrame
				, FullPredictionFrames, FullPredictionTime, LodData->AlignedTime
				, AlignDistance, ExtrapFramesThreshold
				, ParticlePos.X, ParticlePos.Y, ParticlePos.Z
				, ServerTargetPos.X, ServerTargetPos.Y, ServerTargetPos.Z
				, ExtrapolatedTargetPos.X, ExtrapolatedTargetPos.Y, ExtrapolatedTargetPos.Z);
		}
		return;
	}

	Target.RepMode = EPhysicsReplicationMode::Resimulation;

	Target.SimDecayTimeScale = FMath::Clamp(1.0f - (LodData->AlignedTime / FullPredictionTime), 0.0f, 1.0f);

	if (Chaos::FRewindData* RewindData = GetOrEnableRewindData(RigidsSolver))
	{

		Chaos::FReadPhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetRead();
		Chaos::FPBDRigidParticleHandle* RigidParticle = Interface.GetRigidParticle(PhysicsObjectHandle);
		RewindData->RequestResimulation(LocalFrame, RigidParticle);
	}

	static const auto CVarbTransitionModeDebugLog = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ReplicationLOD.TransitionMode.DebugLog"));
	if (CVarbTransitionModeDebugLog->GetBool())
	{
		Chaos::FReadPhysicsObjectInterface_Internal ReadInterface = Chaos::FPhysicsObjectInternalInterface::GetRead();
		const Chaos::FGeometryParticleHandle* DebugHandle = ReadInterface.GetParticle(PhysicsObjectHandle);
		const FVector ParticlePos = DebugHandle ? DebugHandle->GetX() : FVector::ZeroVector;
		const FVector ServerTargetPos = Target.TargetState.Position;
		UE_LOGF(LogPhysics, Log, "[ReplicationLOD] TransitionResim | CurrentFrame=%d LocalFrame=%d AlignedFrame=%d | FullPredictionFrames=%d FullPredictionTime=%.4f AlignedTime=%.4f | AlignDistance=%d ExtrapFramesThreshold=%d | SimDecayTimeScale=%.4f | ParticlePos=(%.2f,%.2f,%.2f) ServerTargetPos=(%.2f,%.2f,%.2f) | RepMode=Resimulation"
			, CurrentFrame, LocalFrame, LodData->AlignedFrame
			, FullPredictionFrames, FullPredictionTime, LodData->AlignedTime
			, AlignDistance, ExtrapFramesThreshold, Target.SimDecayTimeScale
			, ParticlePos.X, ParticlePos.Y, ParticlePos.Z
			, ServerTargetPos.X, ServerTargetPos.Y, ServerTargetPos.Z);
	}
}

void FPhysicsReplicationAsyncVR::DefaultReplication_DEPRECATED(Chaos::FRigidBodyHandle_Internal* Handle, const FPhysicsRepAsyncInputData& State, const float DeltaSeconds, const FPhysicsRepErrorCorrectionData& ErrorCorrection)
{
	if (Handle && Handle->CanTreatAsRigid())
	{
		const float LinearVelocityCoefficient = ErrorCorrection.LinearVelocityCoefficient;
		const float AngularVelocityCoefficient = ErrorCorrection.AngularVelocityCoefficient;
		const float PositionLerp = ErrorCorrection.PositionLerp;
		const float AngleLerp = ErrorCorrection.AngleLerp;

		const FVector TargetPos = State.TargetState.Position;
		const FQuat TargetQuat = State.TargetState.Quaternion;

		FRigidBodyState CurrentState;
		CurrentState.Position = Handle->X();
		CurrentState.Quaternion = Handle->R();
		CurrentState.AngVel = Handle->W();
		CurrentState.LinVel = Handle->V();

		FVector LinDiff;
		float LinDiffSize;
		FVector AngDiffAxis;
		float AngDiff;
		float AngDiffSize;
		ComputeDeltasVR(CurrentState.Position, CurrentState.Quaternion, TargetPos, TargetQuat, LinDiff, LinDiffSize, AngDiffAxis, AngDiff, AngDiffSize);

		const FVector NewLinVel = FVector(State.TargetState.LinVel) + (LinDiff * LinearVelocityCoefficient * DeltaSeconds);
		const FVector NewAngVel = FVector(State.TargetState.AngVel) + (AngDiffAxis * AngDiff * AngularVelocityCoefficient * DeltaSeconds);

		const FVector NewPos = FMath::Lerp(FVector(CurrentState.Position), TargetPos, PositionLerp);
		const FQuat NewAng = FQuat::Slerp(CurrentState.Quaternion, TargetQuat, AngleLerp);

		Handle->SetX(NewPos);
		Handle->SetR(NewAng);
		Handle->SetV(NewLinVel);
		Handle->SetW(FMath::DegreesToRadians(NewAngVel));

		if (State.TargetState.Flags & ERigidBodyFlags::Sleeping)
		{

			if (Handle->ObjectState() != Chaos::EObjectStateType::Kinematic)
			{
				Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
				if (RigidsSolver)
				{
					RigidsSolver->GetEvolution()->SetParticleObjectState(Handle->GetProxy()->GetHandle_LowLevel()->CastToRigidParticle(), Chaos::EObjectStateType::Sleeping);	
				}
			}
		}
	}
}

bool FPhysicsReplicationAsyncVR::DefaultReplication(Chaos::FPBDRigidParticleHandle* Handle, FReplicatedPhysicsTargetAsync& Target, const float DeltaSeconds)
{
	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (RigidsSolver == nullptr)
	{
		return true;
	}

	static const auto CVarResimDisableReplicationOnInteraction = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.Resim.DisableReplicationOnInteraction"));
	if (CVarResimDisableReplicationOnInteraction->GetBool() && ParticlesInResimIslands.Contains(Handle->GetHandleIdx()))
	{
		return false;
	}

	bool bRestoredState = true;
	const FRigidBodyState NewState = Target.TargetState;
	const float NewQuatSizeSqr = NewState.Quaternion.SizeSquared();

	const FString ObjectName
#if CHAOS_DEBUG_NAME
		= Handle && Handle->DebugName() ? *Handle->DebugName() : FString();
#else
		= FString();
#endif

	if (Handle == nullptr)
	{
		UE_LOGF(LogPhysics, Warning, "Trying to replicate rigid state for non-rigid particle. (%ls)", *ObjectName);
		return bRestoredState;
	}
	else if (NewQuatSizeSqr < UE_KINDA_SMALL_NUMBER)
	{
		UE_LOGF(LogPhysics, Warning, "Invalid zero quaternion set for body. (%ls)", *ObjectName);
		return bRestoredState;
	}
	else if (FMath::Abs(NewQuatSizeSqr - 1.f) > UE_KINDA_SMALL_NUMBER)
	{
		UE_LOGF(LogPhysics, Warning, "Quaternion (%f %f %f %f) with non-unit magnitude detected. (%ls)",
			NewState.Quaternion.X, NewState.Quaternion.Y, NewState.Quaternion.Z, NewState.Quaternion.W, *ObjectName);
		return bRestoredState;
	}

	static const auto CVarNetPingExtrapolation = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetPingExtrapolation"));
	const float NetPingExtrapolation = CVarNetPingExtrapolation->GetFloat() >= 0.0f ? CVarNetPingExtrapolation->GetFloat() : ErrorCorrectionDefault.PingExtrapolation;

	static const auto CVarNetPingLimit = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetPingLimit"));
	const float NetPingLimit = CVarNetPingLimit->GetFloat() > 0.0f ? CVarNetPingLimit->GetFloat() : ErrorCorrectionDefault.PingLimit;

	static const auto CVarErrorPerLinearDifference = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorPerLinearDifference"));
	const float ErrorPerLinearDiff = CVarErrorPerLinearDifference->GetFloat() >= 0.0f ? CVarErrorPerLinearDifference->GetFloat() : ErrorCorrectionDefault.ErrorPerLinearDifference;

	static const auto CVarErrorPerAngularDifference = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorPerAngularDifference"));
	const float ErrorPerAngularDiff = CVarErrorPerAngularDifference->GetFloat() >= 0.0f ? CVarErrorPerAngularDifference->GetFloat() : ErrorCorrectionDefault.ErrorPerAngularDifference;

	static const auto CVarMaxRestoredStateError = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxRestoredStateError"));
	const float MaxRestoredStateError = CVarMaxRestoredStateError->GetFloat() >= 0.0f ? CVarMaxRestoredStateError->GetFloat() : ErrorCorrectionDefault.MaxRestoredStateError;

	static const auto CVarErrorAccumulation = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorAccumulationSeconds"));
	const float ErrorAccumulationSeconds = CVarErrorAccumulation->GetFloat() >= 0.0f ? CVarErrorAccumulation->GetFloat() : ErrorCorrectionDefault.ErrorAccumulationSeconds;

	static const auto CVarErrorAccumulationDistanceSq = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorAccumulationDistanceSq"));
	const float ErrorAccumulationDistanceSq = CVarErrorAccumulationDistanceSq->GetFloat() >= 0.0f ? CVarErrorAccumulationDistanceSq->GetFloat() : ErrorCorrectionDefault.ErrorAccumulationDistanceSq;

	static const auto CVarErrorAccumulationSimilarity = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorAccumulationSimilarity"));
	const float ErrorAccumulationSimilarity = CVarErrorAccumulationSimilarity->GetFloat() >= 0.0f ? CVarErrorAccumulationSimilarity->GetFloat() : ErrorCorrectionDefault.ErrorAccumulationSimilarity;

	static const auto CVarLinSet = IConsoleManager::Get().FindConsoleVariable(TEXT("p.PositionLerp"));
	const float PositionLerp = CVarLinSet->GetFloat() >= 0.0f ? CVarLinSet->GetFloat() : ErrorCorrectionDefault.PositionLerp;

	static const auto CVarLinLerp = IConsoleManager::Get().FindConsoleVariable(TEXT("p.LinearVelocityCoefficient"));
	const float LinearVelocityCoefficient = CVarLinLerp->GetFloat() >= 0.0f ? CVarLinLerp->GetFloat() : ErrorCorrectionDefault.LinearVelocityCoefficient;

	static const auto CVarAngSet = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AngleLerp"));
	const float AngleLerp = CVarAngSet->GetFloat() >= 0.0f ? CVarAngSet->GetFloat() : ErrorCorrectionDefault.AngleLerp;

	static const auto CVarAngLerp = IConsoleManager::Get().FindConsoleVariable(TEXT("p.AngularVelocityCoefficient"));
	const float AngularVelocityCoefficient = CVarAngLerp->GetFloat() >= 0.0f ? CVarAngLerp->GetFloat() : ErrorCorrectionDefault.AngularVelocityCoefficient;

	static const auto CVarMaxLinearHardSnapDistance = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxLinearHardSnapDistance"));
	float MaxLinearHardSnapDistance = CVarMaxLinearHardSnapDistance->GetFloat() >= 0.f ? CVarMaxLinearHardSnapDistance->GetFloat() : ErrorCorrectionDefault.MaxLinearHardSnapDistance;
	MaxLinearHardSnapDistance = SettingsCurrent.DefaultReplicationSettings.GetMaxLinearHardSnapDistance(MaxLinearHardSnapDistance);

	FRigidBodyState CurrentState;
	CurrentState.Position = Handle->GetX();
	CurrentState.Quaternion = Handle->GetR();
	CurrentState.AngVel = Handle->GetW();
	CurrentState.LinVel = Handle->GetV();

	const float PingSeconds = FMath::Clamp(LatencyOneWay, 0.f, NetPingLimit);
	const float ExtrapolationDeltaSeconds = PingSeconds * NetPingExtrapolation;
	const FVector ExtrapolationDeltaPos = NewState.LinVel * ExtrapolationDeltaSeconds;
	const FVector_NetQuantize100 TargetPos = NewState.Position + ExtrapolationDeltaPos;
	float NewStateAngVel;
	FVector NewStateAngVelAxis;
	NewState.AngVel.FVector::ToDirectionAndLength(NewStateAngVelAxis, NewStateAngVel);
	NewStateAngVel = FMath::DegreesToRadians(NewStateAngVel);
	const FQuat ExtrapolationDeltaQuaternion = FQuat(NewStateAngVelAxis, NewStateAngVel * ExtrapolationDeltaSeconds);
	FQuat TargetQuat = ExtrapolationDeltaQuaternion * NewState.Quaternion;

	FVector LinDiff;
	float LinDiffSize;
	FVector AngDiffAxis;
	float AngDiff;
	float AngDiffSize;
	ComputeDeltasVR(CurrentState.Position, CurrentState.Quaternion, TargetPos, TargetQuat, LinDiff, LinDiffSize, AngDiffAxis, AngDiff, AngDiffSize);

	const bool bShouldSleep = (NewState.Flags & ERigidBodyFlags::Sleeping) != 0;
	const bool bWasAwake = !Handle->Sleeping();
	const bool bAutoWake = false;

	const float Error = (LinDiffSize * ErrorPerLinearDiff) + (AngDiffSize * ErrorPerAngularDiff);

	bRestoredState = Error < MaxRestoredStateError;
	if (bRestoredState)
	{
		Target.AccumulatedErrorSeconds = 0.0f;
	}
	else
	{

		const float PrevProgress = FVector::DotProduct(
			FVector(CurrentState.Position) - Target.PrevPos,
			(Target.PrevPosTarget - Target.PrevPos).GetSafeNormal());

		const float PrevSimilarity = FVector::DotProduct(
			TargetPos - FVector(CurrentState.Position),
			Target.PrevPosTarget - Target.PrevPos);

		if (PrevProgress < ErrorAccumulationDistanceSq &&
			PrevSimilarity > ErrorAccumulationSimilarity)
		{
			Target.AccumulatedErrorSeconds += DeltaSeconds;
		}
		else
		{
			Target.AccumulatedErrorSeconds = FMath::Max(Target.AccumulatedErrorSeconds - DeltaSeconds, 0.0f);
		}

		const bool bHardSnap =
			LinDiffSize > MaxLinearHardSnapDistance ||
			Target.AccumulatedErrorSeconds > ErrorAccumulationSeconds ||
			CharacterMovementCVars::AlwaysHardSnap;

		if (bHardSnap)
		{
#if !UE_BUILD_SHIPPING
			if (Handle == nullptr)
			{
				UE_LOGF(LogPhysics, Warning, "Trying to replicate rigid state for non-rigid particle. (%ls)", *ObjectName);
				return bRestoredState;
			}
			else if (NewQuatSizeSqr < UE_KINDA_SMALL_NUMBER)
			{
				UE_LOGF(LogPhysics, Warning, "Invalid zero quaternion set for body. (%ls)", *ObjectName);
				return bRestoredState;
			}
			else if (FMath::Abs(NewQuatSizeSqr - 1.f) > UE_KINDA_SMALL_NUMBER)
			{
				UE_LOGF(LogPhysics, Warning, "Quaternion (%f %f %f %f) with non-unit magnitude detected. (%ls)",
					NewState.Quaternion.X, NewState.Quaternion.Y, NewState.Quaternion.Z, NewState.Quaternion.W, *ObjectName);
				return bRestoredState;
			}
#endif

			Target.AccumulatedErrorSeconds = 0.0f;
			bRestoredState = true;

			const bool bCorrectConnectedBodies = SettingsCurrent.DefaultReplicationSettings.GetCorrectConnectedBodies();
			const bool bCorrectConnectedBodiesFriction = SettingsCurrent.DefaultReplicationSettings.GetCorrectConnectedBodiesFriction();
			RigidsSolver->GetEvolution()->ApplyParticleTransformCorrection(Handle, TargetPos, TargetQuat, bCorrectConnectedBodies, bCorrectConnectedBodiesFriction, ReplicatedParticleIDs);
			Handle->SetV(NewState.LinVel);
			Handle->SetW(FMath::DegreesToRadians(NewState.AngVel));
		}
		else
		{
			const FVector NewLinVel = FVector(Target.TargetState.LinVel) + (LinDiff * LinearVelocityCoefficient * DeltaSeconds);
			const FVector NewAngVel = FVector(Target.TargetState.AngVel) + (AngDiffAxis * AngDiff * AngularVelocityCoefficient * DeltaSeconds);

			const FVector NewPos = FMath::Lerp(FVector(CurrentState.Position), TargetPos, PositionLerp);
			const FQuat NewAng = FQuat::Slerp(CurrentState.Quaternion, TargetQuat, AngleLerp);

			Handle->SetX(NewPos);
			Handle->SetR(NewAng);
			Handle->SetV(NewLinVel);
			Handle->SetW(FMath::DegreesToRadians(NewAngVel));
		}
	}

	if (bShouldSleep)
	{

		if (Handle->ObjectState() != Chaos::EObjectStateType::Kinematic)
		{
			RigidsSolver->GetEvolution()->SetParticleObjectState(Handle, Chaos::EObjectStateType::Sleeping);
		}
	}

	Target.PrevPosTarget = TargetPos;
	Target.PrevPos = FVector(CurrentState.Position);

	return bRestoredState;
}

bool FPhysicsReplicationAsyncVR::PredictiveInterpolation(Chaos::FPBDRigidParticleHandle* Handle, FReplicatedPhysicsTargetAsync& Target, const float DeltaSeconds)
{
	static const auto CVarSkipReplication = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.SkipReplication"));
	if (CVarSkipReplication->GetBool())
	{
		return true;
	}

	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (RigidsSolver == nullptr)
	{
		return true;
	}

	static const auto CVarResimDisableReplicationOnInteraction = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.Resim.DisableReplicationOnInteraction"));
	if (CVarResimDisableReplicationOnInteraction->GetBool() && ParticlesInResimIslands.Contains(Handle->GetHandleIdx()))
	{

		Target.SetWaiting(RigidsSolver->GetCurrentFrame() + NetworkPhysicsTickOffset, EPhysicsReplicationMode::Resimulation);
		return false;
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	static const auto CVarDrawDebugTargets = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.DrawDebugTargets"));
	if (CVarDrawDebugTargets->GetBool())
	{

		const FVector Offset = FVector(0.0f, 0.0f, 50.0f);
		const FVector StartPos = Target.TargetState.Position + Offset;
		const int32 SizeMultiplier = FMath::Clamp(Target.TickCount, -4, 30);
		static const auto CVarDebugdrawLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Net.DebugDraw.LifeTime"));
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(StartPos, FVector(5.0f + SizeMultiplier * 0.75f, 5.0f + SizeMultiplier * 0.75f, 5.0f + SizeMultiplier * 0.75f), Target.TargetState.Quaternion, FColor::MakeRandomSeededColor(Target.ServerFrame), false, CVarDebugdrawLifetime->GetFloat(), 0, 1.0f);
	}
#endif

	const bool bIsSleeping = Handle->IsSleeping();
	const bool bCanSimulate = Handle->IsDynamic() || bIsSleeping;

	Target.AccumulatedSleepSeconds = bIsSleeping ? (Target.AccumulatedSleepSeconds + DeltaSeconds) : 0.0f;

	auto EndReplicationHelper = [RigidsSolver, Handle, bCanSimulate, bIsSleeping, DeltaSeconds](FReplicatedPhysicsTargetAsync& Target, bool bOkToClear) -> bool
	{
		const bool bShouldSleep = (Target.TargetState.Flags & ERigidBodyFlags::Sleeping) != 0;
		const bool bReplicatingPhysics = (Target.TargetState.Flags & ERigidBodyFlags::RepPhysics) != 0;

		if (bOkToClear && bShouldSleep && bCanSimulate)
		{
			RigidsSolver->GetEvolution()->SetParticleObjectState(Handle, Chaos::EObjectStateType::Sleeping);

			static const auto CVarSleepConnectedBodies = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.SleepConnectedBodies"));
			if (CVarSleepConnectedBodies->GetBool())
			{
				RigidsSolver->GetEvolution()->ApplySleepOnConnectedParticles(Handle);
			}
		}

		static const auto CVarSleepSecondsClearTarget = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.SleepSecondsClearTarget"));
		static const auto CVarDontClearTarget = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.DontClearTarget"));

		const bool bClearTarget =
			((bOkToClear && bShouldSleep && Target.AccumulatedSleepSeconds >= CVarSleepSecondsClearTarget->GetFloat()) 
				|| (bOkToClear && !bReplicatingPhysics) 
				|| (bOkToClear && !bCanSimulate)) 
			&& !CVarDontClearTarget->GetBool();

		if (!bClearTarget && Target.bAllowTargetAltering)
		{
			static const auto CVarExtrapolationTimeMultiplier = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.ExtrapolationTimeMultiplier"));
			static const auto CVarExtrapolationMinTime = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.ExtrapolationMinTime"));
			const int32 ExtrapolationTickLimit = FMath::Max(
				FMath::CeilToInt(Target.AverageReceiveInterval * CVarExtrapolationTimeMultiplier->GetFloat()), 
				FMath::CeilToInt(CVarExtrapolationMinTime->GetFloat() / DeltaSeconds)); 

			if (Target.TickCount <= ExtrapolationTickLimit)
			{
				FPhysicsReplicationAsyncVR::ExtrapolateTarget(Target, 1, DeltaSeconds);
			}
			else
			{

				Target.bAllowTargetAltering = false;
			}
		}

		return bClearTarget;
	};

	if (Target.IsWaiting())
	{
		return EndReplicationHelper(Target, true);
	}

	static const auto CVarEarlyOutWithVelocity = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.EarlyOutWithVelocity"));
	static const auto CVarEarlyOutDistanceSqr = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.EarlyOutDistanceSqr"));

	const bool bXCanEarlyOut = (CVarEarlyOutWithVelocity->GetBool() || Target.TargetState.LinVel.SizeSquared() < UE_KINDA_SMALL_NUMBER) &&
		(Target.PrevPosTarget - Handle->GetX()).SizeSquared() < CVarEarlyOutDistanceSqr->GetFloat();

	if (bXCanEarlyOut)
	{

		const FQuat TargetRotDelta = Target.TargetState.Quaternion * Handle->GetR().Inverse();

		float Angle;
		FVector Axis;
		TargetRotDelta.ToAxisAndAngle(Axis, Angle);
		Angle = FMath::RadiansToDegrees(FMath::UnwindRadians(Angle));
		Angle = FMath::Abs(Angle);

		static const auto CVarEarlyOutAngle = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.EarlyOutAngle"));
		if (Angle < CVarEarlyOutAngle->GetFloat())
		{

			return EndReplicationHelper(Target, true);
		}
	}

	static const auto CVarAverageReceiveIntervalSmoothing = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.AverageReceiveIntervalSmoothing"));

	Target.AverageReceiveInterval = Target.ReceiveInterval == 0 ? Target.AverageReceiveInterval : FMath::Lerp(Target.AverageReceiveInterval, Target.ReceiveInterval, FMath::Clamp((1.0f / (Target.ReceiveInterval * CVarAverageReceiveIntervalSmoothing->GetFloat())), 0.0f, 1.0f));

	FRigidBodyState CurrentState;
	CurrentState.Position = Handle->GetX();
	CurrentState.Quaternion = Handle->GetR();
	CurrentState.LinVel = Handle->GetV();
	CurrentState.AngVel = Handle->GetW(); 

	const FVector TargetPos = FVector(Target.TargetState.Position);
	const FQuat TargetRot = Target.TargetState.Quaternion;
	const FVector TargetLinVel = FVector(Target.TargetState.LinVel);
	const FVector TargetAngVel = FVector(FMath::DegreesToRadians(Target.TargetState.AngVel)); 

	static const auto CVarKinematicHardSnap = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.KinematicHardSnap"));
	static const auto CVarErrorAccumulationSeconds = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ErrorAccumulationSeconds"));
	static const auto CVarAlwaysHardSnap = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.AlwaysHardSnap"));
	const bool bHardSnap = (!bCanSimulate && CVarKinematicHardSnap->GetBool())
		|| Target.AccumulatedErrorSeconds > CVarErrorAccumulationSeconds->GetFloat()
		|| CVarAlwaysHardSnap->GetBool();

	if (bHardSnap)
	{
		Target.AccumulatedErrorSeconds = 0.0f;

		if (Handle->IsKinematic())
		{

			const Chaos::FKinematicTarget KinTarget = Chaos::FKinematicTarget::MakePositionTarget(Target.PrevPosTarget, Target.PrevRotTarget); 
			RigidsSolver->GetEvolution()->SetParticleKinematicTarget(Handle, KinTarget);
		}
		else
		{

			const bool bCorrectConnectedBodies = SettingsCurrent.PredictiveInterpolationSettings.GetCorrectConnectedBodies();
			RigidsSolver->GetEvolution()->ApplyParticleTransformCorrection(Handle, Target.PrevPosTarget, Target.PrevRotTarget, bCorrectConnectedBodies,  true, ReplicatedParticleIDs);
			Handle->SetV(TargetLinVel);
			Handle->SetW(TargetAngVel);
		}

		Target.PrevLinVel = FVector(Target.TargetState.LinVel);

		return EndReplicationHelper(Target, true);
	}

	bool bSoftSnap = false;

	static const auto CVarDisableErrorVelocityLimits = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.DisableErrorVelocityLimits"));
	static const auto CVarErrorAccLinVelMaxLimit = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.ErrorAccLinVelMaxLimit"));
	static const auto CVarErrorAccAngVelMaxLimit = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.ErrorAccAngVelMaxLimit"));
	if ( CVarDisableErrorVelocityLimits->GetBool() ||
		(TargetLinVel.Size() < CVarErrorAccLinVelMaxLimit->GetFloat() && TargetAngVel.Size() < CVarErrorAccAngVelMaxLimit->GetFloat()))
	{
		const FVector PrevDiff = CurrentState.Position - Target.PrevPos;
		const float ExpectedDistance = (Target.PrevLinVel * DeltaSeconds).Size();
		const float CoveredDistance = FVector::DotProduct(PrevDiff, Target.PrevLinVel.GetSafeNormal());
		const float CoveredAplha = FMath::Clamp(CoveredDistance / ExpectedDistance, 0.0f, 1.0f);

		static const auto CVarMinExpectedDistanceCovered = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.MinExpectedDistanceCovered"));
		if (CoveredAplha < CVarMinExpectedDistanceCovered->GetFloat())
		{
			Target.AccumulatedErrorSeconds += DeltaSeconds;
			bSoftSnap = true;
		}
		else if (Target.AccumulatedErrorSeconds > 0.f)
		{
			static const auto CVarErrorAccumulationDecreaseMultiplier = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.ErrorAccumulationDecreaseMultiplier"));
			const float DecreaseTime = DeltaSeconds * CVarErrorAccumulationDecreaseMultiplier->GetFloat();
			Target.AccumulatedErrorSeconds = FMath::Max(Target.AccumulatedErrorSeconds - DecreaseTime, 0.0f);
			bSoftSnap = true;
		}
	}
	else
	{
		Target.AccumulatedErrorSeconds = 0;
	}

	if (SettingsCurrent.PredictiveInterpolationSettings.GetDisableSoftSnap())
	{
		bSoftSnap = false;
	}

	if (Handle->IsKinematic()) 
	{
		static const auto CVarKinematicPrediction = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.KinematicPrediction"));
		const bool bKinematicPrediction = CVarKinematicPrediction->GetBool();
		const float InterpolationTicks = FMath::CeilToInt(Target.AverageReceiveInterval) - (RigidsSolver->GetCurrentFrame() - Target.ReceiveFrame);

		if ((bKinematicPrediction && Target.bAllowTargetAltering) || InterpolationTicks > 0)
		{

			const float Lerp = 1.f / (bKinematicPrediction ? Target.AverageReceiveInterval : InterpolationTicks);

			const FVector KinTargetPos = FMath::Lerp(CurrentState.Position,
				(bKinematicPrediction ? Target.TargetState.Position : Target.PrevPosTarget),
				Lerp);
			const FQuat KinTargetRot = FQuat::Slerp(CurrentState.Quaternion,
				(bKinematicPrediction ? Target.TargetState.Quaternion : Target.PrevRotTarget),
				Lerp);

			const Chaos::FKinematicTarget KinTarget = Chaos::FKinematicTarget::MakePositionTarget(KinTargetPos, KinTargetRot); 
			RigidsSolver->GetEvolution()->SetParticleKinematicTarget(Handle, KinTarget);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

			if (CVarDrawDebugTargets->GetBool())
			{

				static const auto CVarDrawDebugZOffset = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.DrawDebugZOffset"));
				const FVector Offset = FVector(0.0f, 0.0f, CVarDrawDebugZOffset->GetFloat());
				const FVector Pos = KinTargetPos + Offset;
				const int32 SizeMultiplier = FMath::Clamp(Target.TickCount, -4, 30);
				static const auto CVarDebugdrawLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Net.DebugDraw.LifeTime"));
				Chaos::FDebugDrawQueue::GetInstance().DrawDebugSphere(Pos, 3.0f + SizeMultiplier * 0.75f, 8, FColor::MakeRandomSeededColor(Target.ServerFrame), false, CVarDebugdrawLifetime->GetFloat(), 0, 1.0f);
			}
#endif
		}
		else
		{

			return EndReplicationHelper(Target, true);
		}
	}
	else 
	{

		if (bIsSleeping)
		{
			RigidsSolver->GetEvolution()->SetParticleObjectState(Handle, Chaos::EObjectStateType::Dynamic);
		}

		if (SettingsCurrent.PredictiveInterpolationSettings.GetVelocityBased())
		{

			const float AverageReceiveIntervalSeconds = Target.AverageReceiveInterval * DeltaSeconds;
			const float InterpolationTime = AverageReceiveIntervalSeconds * SettingsCurrent.PredictiveInterpolationSettings.GetPosInterpolationTimeMultiplier();

			const float RTT = LatencyOneWay * 2.f;
			const float PosCorrectionTime = FMath::Max(SettingsCurrent.PredictiveInterpolationSettings.GetPosCorrectionTimeBase() + AverageReceiveIntervalSeconds + RTT * SettingsCurrent.PredictiveInterpolationSettings.GetPosCorrectionTimeMultiplier(),
				DeltaSeconds + SettingsCurrent.PredictiveInterpolationSettings.GetPosCorrectionTimeMin());
			const float RotCorrectionTime = FMath::Max(SettingsCurrent.PredictiveInterpolationSettings.GetRotCorrectionTimeBase() + AverageReceiveIntervalSeconds + RTT * SettingsCurrent.PredictiveInterpolationSettings.GetRotCorrectionTimeMultiplier(),
				DeltaSeconds + SettingsCurrent.PredictiveInterpolationSettings.GetRotCorrectionTimeMin());

			FVector CorrectionX = CurrentState.Position;
			if ((bXCanEarlyOut && SettingsCurrent.PredictiveInterpolationSettings.GetSkipVelocityRepOnPosEarlyOut()) == false)
			{	

				const FVector PosDiff = TargetPos - CurrentState.Position;

				const FVector LinVelDiff = -CurrentState.LinVel + TargetLinVel;

				const float VelocityAlpha = FMath::Clamp(DeltaSeconds / InterpolationTime, 0.0f, 1.0f);

				FVector RepLinVel;
				if (SettingsCurrent.PredictiveInterpolationSettings.GetCorrectionAsVelocity())
				{

					const FVector PosDiffVelocity = PosDiff / PosCorrectionTime;

					const FVector BlendedTargetVelocity = LinVelDiff + PosDiffVelocity;

					RepLinVel = CurrentState.LinVel + (BlendedTargetVelocity * VelocityAlpha); 
				}
				else 
				{

					RepLinVel = CurrentState.LinVel + (LinVelDiff * VelocityAlpha); 

					const float CorrectionAlpha = FMath::Clamp(DeltaSeconds / PosCorrectionTime, 0.0f, 1.0f);

					const FVector PosDiffVelocityDelta = PosDiff * CorrectionAlpha; 

					CorrectionX = Handle->GetX() + PosDiffVelocityDelta;
				}

				Handle->SetV(RepLinVel);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				static const auto CVarDrawDebugVectors = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.DrawDebugVectors"));
				if (CVarDrawDebugVectors->GetBool())
				{
					static const auto CVarDrawDebugZOffset = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.PredictiveInterpolation.DrawDebugZOffset"));
					const FVector Offset = FVector(0.0f, 0.0f, CVarDrawDebugZOffset->GetFloat());
					const FVector OffsetAdd = FVector(0.0f, 0.0f, 10.0f);
					const FVector StartPos = TargetPos + Offset;
					FVector Direction = TargetLinVel;
					Direction.Normalize();
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow((StartPos + OffsetAdd * 0), (StartPos + OffsetAdd * 0) + TargetLinVel * 0.5f, 5.0f, FColor::Green, false, -1.0f, 0, 2.0f);
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow((StartPos + OffsetAdd * 1), (StartPos + OffsetAdd * 1) + CurrentState.LinVel * 0.5f, 5.0f, FColor::Blue, false, -1.0f, 0, 2.0f);
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow((StartPos + OffsetAdd * 2), (StartPos + OffsetAdd * 2) + (Target.PrevLinVel - CurrentState.LinVel) * 0.5f, 5.0f, FColor::Red, false, -1.0f, 0, 2.0f);
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow((StartPos + OffsetAdd * 3), (StartPos + OffsetAdd * 3) + RepLinVel * 0.5f, 5.0f, FColor::Magenta, false, -1.0f, 0, 2.0f);
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow((StartPos + OffsetAdd * 4), (StartPos + OffsetAdd * 4) + (Target.PrevLinVel - RepLinVel) * 0.5f, 5.0f, FColor::Orange, false, -1.0f, 0, 2.0f);
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow((StartPos + OffsetAdd * 5), (StartPos + OffsetAdd * 5) + Direction * RTT, 5.0f, FColor::White, false, -1.0f, 0, 2.0f);
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow((StartPos + OffsetAdd * 6), (StartPos + OffsetAdd * 6) + Direction * InterpolationTime, 5.0f, FColor::Yellow, false, -1.0f, 0, 2.0f);
				}
#endif

				Target.PrevLinVel = FVector(RepLinVel);
			}

			FQuat CorrectionR = CurrentState.Quaternion;
			{	

				const FVector AngVelDiff = -CurrentState.AngVel + TargetAngVel;

				const float VelocityAlpha = FMath::Clamp(DeltaSeconds / InterpolationTime, 0.0f, 1.0f);

				FVector RepAngVel;
				if (SettingsCurrent.PredictiveInterpolationSettings.GetCorrectionAsVelocity())
				{

					const FQuat RotDiff = TargetRot * CurrentState.Quaternion.Inverse();

					float WAngle;
					FVector WAxis;
					RotDiff.ToAxisAndAngle(WAxis, WAngle);
					WAngle = FMath::UnwindRadians(WAngle);
					const FVector RotDiffVelocity = FVector(WAxis * (WAngle / RotCorrectionTime));

					const FVector BlendedTargetVelocity = AngVelDiff + RotDiffVelocity;

					RepAngVel = CurrentState.AngVel + (BlendedTargetVelocity * VelocityAlpha); 
				}
				else 
				{

					RepAngVel = CurrentState.AngVel + (AngVelDiff * VelocityAlpha); 

					const float CorrectionAlpha = FMath::Clamp(DeltaSeconds / RotCorrectionTime, 0.0f, 1.0f);

					CorrectionR = FQuat::Slerp(Handle->GetR(), TargetRot, CorrectionAlpha);
				}

				Handle->SetW(RepAngVel);
			}

			Target.PrevPos = FVector(CurrentState.Position);

			if (SettingsCurrent.PredictiveInterpolationSettings.GetCorrectionAsVelocity() == false)
			{
				const bool bCorrectConnectedBodies = SettingsCurrent.PredictiveInterpolationSettings.GetCorrectConnectedBodies();
				const bool bCorrectConnectedBodiesFriction = SettingsCurrent.PredictiveInterpolationSettings.GetCorrectConnectedBodiesFriction();
				RigidsSolver->GetEvolution()->ApplyParticleTransformCorrection(Handle, CorrectionX, CorrectionR, bCorrectConnectedBodies, bCorrectConnectedBodiesFriction, ReplicatedParticleIDs);
			}
		}

		if (bSoftSnap || SettingsCurrent.PredictiveInterpolationSettings.GetVelocityBased() == false)
		{
			const FVector SoftSnapPos = FMath::Lerp(FVector(CurrentState.Position),
				SettingsCurrent.PredictiveInterpolationSettings.GetSoftSnapToSource() ? Target.PrevPosTarget : Target.TargetState.Position,
				FMath::Clamp(SettingsCurrent.PredictiveInterpolationSettings.GetSoftSnapPosStrength(), 0.0f, 1.0f));

			const FQuat SoftSnapRot = FQuat::Slerp(CurrentState.Quaternion,
				SettingsCurrent.PredictiveInterpolationSettings.GetSoftSnapToSource() ? Target.PrevRotTarget : Target.TargetState.Quaternion,
				FMath::Clamp(SettingsCurrent.PredictiveInterpolationSettings.GetSoftSnapRotStrength(), 0.0f, 1.0f));

			const bool bCorrectConnectedBodies = SettingsCurrent.PredictiveInterpolationSettings.GetCorrectConnectedBodies();
			const bool bCorrectConnectedBodiesFriction = SettingsCurrent.PredictiveInterpolationSettings.GetCorrectConnectedBodiesFriction();
			RigidsSolver->GetEvolution()->ApplyParticleTransformCorrection(Handle, SoftSnapPos, SoftSnapRot, bCorrectConnectedBodies, bCorrectConnectedBodiesFriction, ReplicatedParticleIDs);
		}
	}

	return EndReplicationHelper(Target, false);
}

void FPhysicsReplicationAsyncVR::ExtrapolateTarget(FReplicatedPhysicsTargetAsync& Target, const int32 ExtrapolateFrames, const float DeltaSeconds)
{
	const float ExtrapolationTime = DeltaSeconds * static_cast<float>(ExtrapolateFrames);
	FPhysicsReplicationAsyncVR::ExtrapolateTarget(Target, ExtrapolationTime);
}

void FPhysicsReplicationAsyncVR::ExtrapolateTarget(FReplicatedPhysicsTargetAsync& Target, const float ExtrapolationTime)
{

	Target.TargetState.Position = Target.TargetState.Position + Target.TargetState.LinVel * ExtrapolationTime;

	float TargetAngVelSize;
	FVector TargetAngVelAxis;
	Target.TargetState.AngVel.FVector::ToDirectionAndLength(TargetAngVelAxis, TargetAngVelSize);
	TargetAngVelSize = FMath::DegreesToRadians(TargetAngVelSize);
	const FQuat TargetRotExtrapDelta = FQuat(TargetAngVelAxis, TargetAngVelSize * ExtrapolationTime);
	Target.TargetState.Quaternion = TargetRotExtrapDelta * Target.TargetState.Quaternion;
}

bool FPhysicsReplicationAsyncVR::IsTargetValidForResim(const FReplicatedPhysicsTargetAsync& Target) const
{
	const Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<const Chaos::FPBDRigidsSolver*>(GetSolver());
	if (RigidsSolver == nullptr)
	{
		return false;
	}

	const Chaos::FRewindData* RewindData = RigidsSolver->GetRewindData();
	if (RewindData == nullptr)
	{
		return false;
	}

	if (Target.ServerFrame <= 0)
	{
		return false;
	}

	const int32 LocalFrame = Target.ServerFrame - NetworkPhysicsTickOffset;

	if (!RewindData->IsFrameWithinRewindHistory(LocalFrame))
	{
		return false;
	}
	return true;
}

bool FPhysicsReplicationAsyncVR::ShouldResimulateParticle(const Chaos::FPBDRigidParticleHandle* Handle, const FReplicatedPhysicsTargetAsync& Target, const float DeltaSeconds) const
{
	bool bShouldTriggerResim = false;
	if (IsTargetValidForResim(Target))
	{
		const Chaos::FRewindData* RewindData = static_cast<const Chaos::FPBDRigidsSolver*>(GetSolver())->GetRewindData();
		const int32 LocalFrame = Target.ServerFrame - NetworkPhysicsTickOffset;
		static constexpr Chaos::FFrameAndPhase::EParticleHistoryPhase RewindPhase = Chaos::FFrameAndPhase::EParticleHistoryPhase::PrePushData;

		const Chaos::FGeometryParticleState PastState = RewindData->GetPastStateAtFrame(*Handle, LocalFrame, RewindPhase);

		const bool bCompareX = Chaos::FPhysicsSolverBase::GetResimulationErrorPositionThresholdEnabled() || SettingsCurrent.ResimulationSettings.bOverrideResimulationErrorPositionThreshold;
		const bool bCompareR = Chaos::FPhysicsSolverBase::GetResimulationErrorRotationThresholdEnabled() || SettingsCurrent.ResimulationSettings.bOverrideResimulationErrorRotationThreshold;
		const bool bCompareV = Chaos::FPhysicsSolverBase::GetResimulationErrorLinearVelocityThresholdEnabled() || SettingsCurrent.ResimulationSettings.bOverrideResimulationErrorLinearVelocityThreshold;
		const bool bCompareW = Chaos::FPhysicsSolverBase::GetResimulationErrorAngularVelocityThresholdEnabled() || SettingsCurrent.ResimulationSettings.bOverrideResimulationErrorAngularVelocityThreshold;

		static const auto CVarResimulateSleepDesync = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.Resim.ResimulateSleepDesync"));

		const bool bCompareSleep = CVarResimulateSleepDesync->GetBool();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

		FColor DebugColor = FColor::Black;
		bool bResimV = false;
		bool bResimW = false;
#endif

		if (bCompareX)
		{
			const float ResimPositionErrorThreshold = SettingsCurrent.ResimulationSettings.GetResimulationErrorPositionThreshold(Chaos::FPhysicsSolverBase::GetResimulationErrorPositionThreshold());
			bShouldTriggerResim = Chaos::FRewindData::CheckVectorThreshold(Target.TargetState.Position, PastState.GetX(), ResimPositionErrorThreshold);
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if (bShouldTriggerResim)
			{
				DebugColor = FColor::Orange;
			}
#endif
		}

		if (!bShouldTriggerResim && bCompareV)
		{
			const float ResimLinVelocityErrorThreshold = SettingsCurrent.ResimulationSettings.GetResimulationErrorLinearVelocityThreshold(Chaos::FPhysicsSolverBase::GetResimulationErrorLinearVelocityThreshold());
			bShouldTriggerResim = Chaos::FRewindData::CheckVectorThreshold(Target.TargetState.LinVel, PastState.GetV(), ResimLinVelocityErrorThreshold);
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if (bShouldTriggerResim)
			{
				bResimV = true;
			}
#endif
		}

		if (!bShouldTriggerResim && bCompareW)
		{
			const float ResimAngVelocityErrorThreshold = SettingsCurrent.ResimulationSettings.GetResimulationErrorAngularVelocityThreshold(Chaos::FPhysicsSolverBase::GetResimulationErrorAngularVelocityThreshold());
			bShouldTriggerResim = Chaos::FRewindData::CheckVectorThreshold(Target.TargetState.AngVel, FMath::RadiansToDegrees(PastState.GetW()), ResimAngVelocityErrorThreshold);
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if (bShouldTriggerResim)
			{
				bResimW = true;
			}
#endif
		}

		if (!bShouldTriggerResim && bCompareR)
		{
			const float ResimRotationErrorThreshold = SettingsCurrent.ResimulationSettings.GetResimulationErrorRotationThreshold(Chaos::FPhysicsSolverBase::GetResimulationErrorRotationThreshold());
			bShouldTriggerResim = Chaos::FRewindData::CheckQuaternionThreshold(Target.TargetState.Quaternion, PastState.GetR(), ResimRotationErrorThreshold);
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if (bShouldTriggerResim)
			{
				DebugColor = FColor::Magenta;
			}
#endif
		}

		if (!bShouldTriggerResim && bCompareSleep)
		{
			const bool bShouldSleep = (Target.TargetState.Flags & ERigidBodyFlags::Sleeping) != 0;
			const bool bIsSleep = PastState.ObjectState() == Chaos::EObjectStateType::Sleeping;
			bShouldTriggerResim = bIsSleep != bShouldSleep;
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if (bShouldTriggerResim)
			{
				DebugColor = FColor::White;
			}
#endif
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

		if (Chaos::FPhysicsSolverBase::CanDebugNetworkPhysicsPrediction())
		{
			UE_LOGF(LogPhysics, Log, "Apply Rigid body state at local frame %d with offset = %d", LocalFrame, NetworkPhysicsTickOffset);
			UE_LOGF(LogPhysics, Log, "Should Trigger Resim = %ls | Server Frame = %d | Client Frame = %d", (bShouldTriggerResim ? TEXT("True") : TEXT("False")), Target.ServerFrame, LocalFrame);
			UE_LOGF(LogPhysics, Log, "Particle Target Position = %ls | Current Position = %ls", *Target.TargetState.Position.ToString(), *PastState.GetX().ToString());
			UE_LOGF(LogPhysics, Log, "Particle Target Velocity = %ls | Current Velocity = %ls", *Target.TargetState.LinVel.ToString(), *PastState.GetV().ToString());
			UE_LOGF(LogPhysics, Log, "Particle Target Quaternion = %ls | Current Quaternion = %ls", *Target.TargetState.Quaternion.ToString(), *PastState.GetR().ToString());
			UE_LOGF(LogPhysics, Log, "Particle Target Omega = %ls | Current Omega= %ls", *Target.TargetState.AngVel.ToString(), *PastState.GetW().ToString());
		}

		static const auto CVarResimDrawDebug = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.Resim.DrawDebug"));
		static const auto CVarRenderInterpDebugDrawResimTrigger = IConsoleManager::Get().FindConsoleVariable(TEXT("p.RenderInterp.DebugDraw.ResimTrigger"));
		static const auto CVarRenderInterpDebugDrawResimBoxScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.RenderInterp.DebugDraw.ResimBoxScale"));
		if (CVarResimDrawDebug->GetBool() || CVarRenderInterpDebugDrawResimTrigger->GetBool())
		{
			if (bShouldTriggerResim)
			{
				FVector Box = CVarRenderInterpDebugDrawResimTrigger->GetBool() ? FVector(6, 3, 2) : FVector(40, 20, 10);
				Box *= CVarRenderInterpDebugDrawResimBoxScale->GetFloat();
				const float DrawThickness = (CVarRenderInterpDebugDrawResimTrigger->GetBool() ? 0.5f : 1.5f) * CVarRenderInterpDebugDrawResimBoxScale->GetFloat();

				static const auto CVarDebugdrawLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Net.DebugDraw.LifeTime"));
				if (CVarRenderInterpDebugDrawResimTrigger->GetBool()) 
				{
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(PastState.GetX(), Box, PastState.GetR(), FColor::White, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(Target.TargetState.Position, Box, Target.TargetState.Quaternion, DebugColor, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);

					Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(Handle->GetX(), PastState.GetX(), 5.0f, FColor::White, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(PastState.GetX(), Target.TargetState.Position, 5.0f, FColor::Black, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);

					if (bResimV)
					{
						const FVector DiffV = Target.TargetState.LinVel - PastState.GetV();
						Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(Target.TargetState.Position, Target.TargetState.Position + DiffV, 5.0f, FColor::Orange, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);
					}
					if (bResimW)
					{
						const FVector DiffW = Target.TargetState.AngVel - FMath::RadiansToDegrees(PastState.GetW());
						Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(Target.TargetState.Position + DiffW, Target.TargetState.Position, 5.0f, FColor::Magenta, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);
					}
				}
				else 
				{
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(Handle->GetX(), Box, PastState.GetR(), FColor::White, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);
					Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(Handle->GetX() + (Target.TargetState.Position - PastState.GetX()), Box, Target.TargetState.Quaternion, DebugColor, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);

					if (bResimV)
					{
						const FVector DiffV = Target.TargetState.LinVel - PastState.GetV();
						Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(Handle->GetX(), Handle->GetX() + DiffV, 5.0f, FColor::Orange, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);
					}
					if (bResimW)
					{
						const FVector DiffW = Target.TargetState.AngVel - FMath::RadiansToDegrees(PastState.GetW());
						Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(Handle->GetX() + DiffW, Handle->GetX(), 5.0f, FColor::Magenta, false, CVarDebugdrawLifetime->GetFloat(), 0, DrawThickness);
					}
				}
			}
		}
#endif
	}

	return bShouldTriggerResim;
}

bool FPhysicsReplicationAsyncVR::ResimulationReplication(Chaos::FPBDRigidParticleHandle* Handle, FReplicatedPhysicsTargetAsync& Target, const float DeltaSeconds)
{
	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (!RigidsSolver)
	{
		return true;
	}

	Chaos::FRewindData* RewindData = GetOrEnableRewindData(RigidsSolver);

	if (!IsTargetValidForResim(Target) || !Handle)
	{
		return true;
	}

	bool bClearTarget = true;
	if (RewindData)
	{
		if (RigidsSolver->GetEvolution())
		{
			const int32 LocalFrame = Target.ServerFrame - NetworkPhysicsTickOffset;
			static constexpr Chaos::FFrameAndPhase::EParticleHistoryPhase RewindPhase = Chaos::FFrameAndPhase::EParticleHistoryPhase::PrePushData;

			const Chaos::FGeometryParticleState PastState = RewindData->GetPastStateAtFrame(*Handle, LocalFrame, RewindPhase);

			const bool bShouldTriggerResim = (Chaos::RewindBeforeAdvance == 0) ? ShouldResimulateParticle(Handle, Target, DeltaSeconds) : false;

			if (bShouldTriggerResim && Target.TickCount == 0 && LocalFrame > RewindData->GetBlockedResimFrame())
			{

				RewindData->RequestResimulation(LocalFrame, Handle);
			}
			else if (SettingsCurrent.ResimulationSettings.GetRuntimeCorrectionEnabled())
			{
				const int32 NumPredictedFrames = RigidsSolver->GetCurrentFrame() - LocalFrame - Target.TickCount;

				if (Target.TickCount <= NumPredictedFrames && NumPredictedFrames > 0)
				{
					const FVector ErrorOffset = (Target.TargetState.Position - PastState.GetX());

					const float CorrectionAmountX = SettingsCurrent.ResimulationSettings.GetPosStabilityMultiplier() / NumPredictedFrames;
					const FVector PosDiffCorrection = ErrorOffset * CorrectionAmountX; 
					const FVector CorrectedX = Handle->GetX() + PosDiffCorrection;

					const float CorrectionAmountR = SettingsCurrent.ResimulationSettings.GetRotStabilityMultiplier() / NumPredictedFrames;
					const FQuat DeltaQuat = PastState.GetR().Inverse() * Target.TargetState.Quaternion;
					const FQuat TargetCorrectionR = Handle->GetR() * DeltaQuat;
					const FQuat CorrectedR = FQuat::Slerp(Handle->GetR(), TargetCorrectionR, CorrectionAmountR);

					if (SettingsCurrent.ResimulationSettings.GetRuntimeVelocityCorrectionEnabled())
					{

						const FVector LinVelDiff = Target.TargetState.LinVel - PastState.GetV(); 
						const float CorrectionAmountV = SettingsCurrent.ResimulationSettings.GetVelStabilityMultiplier() / NumPredictedFrames;
						const FVector VelCorrection = LinVelDiff * CorrectionAmountV; 
						const FVector CorrectedV = Handle->GetV() + VelCorrection;

						const FVector AngVelDiff = FMath::DegreesToRadians(Target.TargetState.AngVel) - PastState.GetW(); 
						const float CorrectionAmountW = SettingsCurrent.ResimulationSettings.GetAngVelStabilityMultiplier() / NumPredictedFrames;
						const FVector AngVelCorrection = AngVelDiff * CorrectionAmountW; 
						const FVector CorrectedW = Handle->GetW() + AngVelCorrection;

						Handle->SetV(CorrectedV);
						Handle->SetW(CorrectedW);
					}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
					static const auto CVarResimDrawDebug = IConsoleManager::Get().FindConsoleVariable(TEXT("np2.Resim.DrawDebug"));
					static const auto CVarDebugdrawLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Net.DebugDraw.LifeTime"));
					if (CVarResimDrawDebug->GetBool())
					{
						Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(Handle->GetX(), CorrectedX, 5.0f, FColor::MakeRandomSeededColor(LocalFrame), false, CVarDebugdrawLifetime->GetFloat(), 0, 0.5f);
					}
#endif

					RigidsSolver->GetEvolution()->ApplyParticleTransformCorrection(Handle, CorrectedX, CorrectedR, SettingsCurrent.ResimulationSettings.GetRuntimeCorrectConnectedBodies(), true, ReplicatedParticleIDs);
				}

				bClearTarget = Target.TickCount >= NumPredictedFrames;
			}

			const bool bShouldSleep = (Target.TargetState.Flags & ERigidBodyFlags::Sleeping) != 0;
			if (Target.IsWaiting() && !bShouldSleep)
			{

				bClearTarget = false;
			}
		}
	}

	return bClearTarget;
}

void FPhysicsReplicationAsyncVR::DebugDrawReplicationMode(Chaos::FConstPhysicsObjectHandle PhysicsObjectHandle, const FReplicatedPhysicsTargetAsync& Target)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

	static const auto CVarDebugDrawShowRepMode = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Net.DebugDraw.ShowRepMode"));
	if (!CVarDebugDrawShowRepMode->GetInt())
	{
		return;
	}

	if (PhysicsObjectHandle == nullptr)
	{
		return;
	}

	FVector BoxExtent = FVector(10.0f, 10.0f, 10.0f);
	Chaos::FReadPhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetRead();
	if (Chaos::FGeometryParticleHandle* Handle = Interface.GetParticle(PhysicsObjectHandle))
	{
		BoxExtent = Handle->LocalBounds().Extents() * 0.5f;
	}

	FColor DebugColor = FColor::White;
	const EPhysicsReplicationMode RepMode = Target.IsWaiting() ? Target.RepModeOverride : Target.RepMode;
	switch (RepMode)
	{
	case EPhysicsReplicationMode::PredictiveInterpolation:
		DebugColor = FColor::Yellow;
		break;
	case EPhysicsReplicationMode::Resimulation:
		DebugColor = FColor::Red;
		break;
	case EPhysicsReplicationMode::Default:
		DebugColor = FColor::Cyan;
		break;
	case EPhysicsReplicationMode::None:
	default:
		DebugColor = FColor::Black;
		break;
	}

	static const auto CVarDebugdrawLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Net.DebugDraw.LifeTime"));
	Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(Target.TargetState.Position, BoxExtent, Target.TargetState.Quaternion, DebugColor, false, CVarDebugdrawLifetime->GetFloat(), 0, 1.0f);
#endif
}

FName FPhysicsReplicationAsyncVR::GetFNameForStatId() const
{
	const static FLazyName StaticName("FPhysicsReplicationAsyncCallback");
	return StaticName;
}

bool FPhysicsReplicationAsyncVR::UsePhysicsReplicationLOD()
{
	Chaos::FPBDRigidsSolver* RigidsSolver = static_cast<Chaos::FPBDRigidsSolver*>(GetSolver());
	if (RigidsSolver == nullptr)
	{
		return false;
	}

	IPhysicsReplicationLODAsync* PhysRepLod = RigidsSolver->GetPhysicsReplicationLOD_Internal();
	return PhysRepLod && PhysRepLod->IsEnabled();
}

#pragma endregion 