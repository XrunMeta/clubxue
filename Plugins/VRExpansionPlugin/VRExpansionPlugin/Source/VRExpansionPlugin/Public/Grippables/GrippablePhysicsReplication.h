

#pragma once

#include "CoreMinimal.h"
#include "Physics/PhysicsInterfaceUtils.h"
#include "PhysicsReplication.h"

#include "Engine/ReplicatedState.h"
#include "PhysicsReplicationInterface.h"
#include "Physics/PhysicsInterfaceDeclares.h"
#include "PhysicsProxy/SingleParticlePhysicsProxyFwd.h"
#include "Chaos/PhysicsObject.h"
#include "Chaos/SimCallbackObject.h"
#include "Physics/NetworkPhysicsSettingsComponent.h"

#include "GrippablePhysicsReplication.generated.h"

#pragma region FPhysicsReplicationAsync

class FPhysicsReplicationAsyncVR : public IPhysicsReplicationAsync,
	public Chaos::TSimCallbackObject<
	FPhysicsReplicationAsyncInput,
	Chaos::FSimCallbackNoOutput,
	Chaos::ESimCallbackOptions::Presimulate | Chaos::ESimCallbackOptions::PostIntegrate | Chaos::ESimCallbackOptions::PostSolve | Chaos::ESimCallbackOptions::PhysicsObjectUnregister>
{
	virtual FName GetFNameForStatId() const override;
	virtual void OnPostInitialize_Internal() override;
	virtual void OnPreSimulate_Internal() override;
	virtual void OnPostIntegrate_Internal() override;
	virtual void OnPostSolve_Internal() override;
	virtual void OnPhysicsObjectUnregistered_Internal(Chaos::FConstPhysicsObjectHandle PhysicsObject) override;

	virtual void ApplyTargetStatesAsync(const float DeltaSeconds);
	UE_DEPRECATED(5.6, "Deprecated, call the function with just @param DeltaSeconds instead.")
		virtual void ApplyTargetStatesAsync(const float DeltaSeconds, const FPhysicsRepErrorCorrectionData& ErrorCorrection, const TArray<FPhysicsRepAsyncInputData>& TargetStates) { ApplyTargetStatesAsync(DeltaSeconds); };

	virtual void DefaultReplication_DEPRECATED(Chaos::FRigidBodyHandle_Internal* Handle, const FPhysicsRepAsyncInputData& State, const float DeltaSeconds, const FPhysicsRepErrorCorrectionData& ErrorCorrection);
	virtual bool DefaultReplication(Chaos::FPBDRigidParticleHandle* Handle, FReplicatedPhysicsTargetAsync& Target, const float DeltaSeconds);
	virtual bool PredictiveInterpolation(Chaos::FPBDRigidParticleHandle* Handle, FReplicatedPhysicsTargetAsync& Target, const float DeltaSeconds);
	virtual bool ResimulationReplication(Chaos::FPBDRigidParticleHandle* Handle, FReplicatedPhysicsTargetAsync& Target, const float DeltaSeconds);

public:

	virtual void RegisterSettings(Chaos::FConstPhysicsObjectHandle PhysicsObject, TWeakPtr<const FNetworkPhysicsSettingsData> InSettings) override;
	virtual void AddResimulationRequest_Internal(const float DeltaSeconds) override;

	virtual int32 GetNetworkPhysicsTickOffset_Internal() const override { return NetworkPhysicsTickOffset; }
	virtual TWeakPtr<FParticleSimDecaySettings> FindOrAddParticleSimDecaySettings(Chaos::FConstPhysicsObjectHandle PhysicsObject) override;
	virtual void RemoveParticleSimDecaySettings(Chaos::FConstPhysicsObjectHandle PhysicsObject) override;

private:
	float LatencyOneWay;
	FRigidBodyErrorCorrection ErrorCorrectionDefault;
	FNetworkPhysicsSettingsData SettingsCurrent;
	FNetworkPhysicsSettingsData SettingsDefault;
	TMap<Chaos::FConstPhysicsObjectHandle, FReplicatedPhysicsTargetAsync> ObjectToTarget;
	TArray<Chaos::FConstPhysicsObjectHandle> PendingDeleteFromObjectToTarget;
	TMap<Chaos::FConstPhysicsObjectHandle, TWeakPtr<const FNetworkPhysicsSettingsData>> ObjectToSettings;
	TMap<Chaos::FConstPhysicsObjectHandle, TSharedPtr<FParticleSimDecaySettings>> ParticleSimDecaySettings;
	TArray<const Chaos::Private::FPBDIsland*> ResimIslands;
	TArray<const Chaos::FGeometryParticleHandle*> ResimIslandsParticles;
	TArray<int32> ParticlesInResimIslands;
	TArray<Chaos::FParticleID> ReplicatedParticleIDs;

	int32 ResimOutOfBoundsCounter = 0;
	float ResimErrorLogTimer = 0.0f;

	bool NetworkPhysicsTickOffsetAssigned = false;
	int32 NetworkPhysicsTickOffset = 0;

private:
	FReplicatedPhysicsTargetAsync* AddObjectToReplication(Chaos::FConstPhysicsObjectHandle PhysicsObject);
	void RemoveObjectFromReplication(Chaos::FConstPhysicsObjectHandle PhysicsObject);
	void UpdateAsyncTarget(const FPhysicsRepAsyncInputData& Input, Chaos::FPBDRigidsSolver* RigidsSolver);
	void UpdateRewindDataTarget(const FPhysicsRepAsyncInputData& Input);
	void CacheResimInteractions();

	void FetchObjectSettings(Chaos::FConstPhysicsObjectHandle PhysicsObject);
	bool UsePhysicsReplicationLOD();
	void CheckTargetResimValidity(FReplicatedPhysicsTargetAsync& Target);
	void ApplyPhysicsReplicationLOD(Chaos::FConstPhysicsObjectHandle PhysicsObjectHandle, FReplicatedPhysicsTargetAsync& Target, const uint32 LODFlags);
	void DebugDrawReplicationMode(Chaos::FConstPhysicsObjectHandle PhysicsObjectHandle, const FReplicatedPhysicsTargetAsync& Target);

	bool ShouldResimulateParticle(const Chaos::FPBDRigidParticleHandle* Handle, const FReplicatedPhysicsTargetAsync& Target, const float DeltaSeconds) const;

	bool IsTargetValidForResim(const FReplicatedPhysicsTargetAsync& Target) const;

	Chaos::FRewindData* GetOrEnableRewindData(Chaos::FPBDRigidsSolver* RigidsSolver);

	static void ExtrapolateTarget(FReplicatedPhysicsTargetAsync& Target, const int32 ExtrapolateFrames, const float DeltaSeconds);

	static void ExtrapolateTarget(FReplicatedPhysicsTargetAsync& Target, const float ExtrapolationTime);

public:
	void Setup(FRigidBodyErrorCorrection ErrorCorrection)
	{
		ErrorCorrectionDefault = ErrorCorrection;
	}
};

#pragma endregion 

class FPhysicsReplicationVR : public FPhysicsReplication
{
public:

	FPhysScene* PhysSceneVR;

	FPhysicsReplicationVR(FPhysScene* PhysScene);
	~FPhysicsReplicationVR();
	static bool IsInitialized();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnTick(float DeltaSeconds, TMap<TWeakObjectPtr<UPrimitiveComponent>, FReplicatedPhysicsTarget>& ComponentsToTargets) override;

	virtual bool ApplyRigidBodyState(float DeltaSeconds, FBodyInstance* BI, FReplicatedPhysicsTarget& PhysicsTarget, const FRigidBodyErrorCorrection& ErrorCorrection, const float PingSecondsOneWay, int32 LocalFrame, int32 NumPredictedFrames) override;
	virtual bool ApplyRigidBodyState(float DeltaSeconds, FBodyInstance* BI, FReplicatedPhysicsTarget& PhysicsTarget, const FRigidBodyErrorCorrection& ErrorCorrection, const float PingSecondsOneWay, bool* bDidHardSnap = nullptr) override;

	virtual void SetReplicatedTarget(UPrimitiveComponent* Component, FName BoneName, const FRigidBodyState& ReplicatedTarget, int32 ServerFrame) override;

	void SetReplicatedTargetVR(Chaos::FConstPhysicsObjectHandle PhysicsObject, const FRigidBodyState& ReplicatedTarget, int32 ServerFrame, EPhysicsReplicationMode ReplicationMode = EPhysicsReplicationMode::Default);

	virtual void RemoveReplicatedTarget(UPrimitiveComponent* Component) override;

	void RemoveReplicatedTargetVR(Chaos::FConstPhysicsObjectHandle PhysicsObject);

	TMap<TWeakObjectPtr<UPrimitiveComponent>, FReplicatedPhysicsTarget> ComponentToTargetsVR_DEPRECATED; 
	TArray<FReplicatedPhysicsTarget> ReplicatedTargetsQueueVR;
	FPhysicsReplicationAsyncVR* PhysicsReplicationAsyncVR;
	FPhysicsReplicationAsyncInput* AsyncInputVR;	
	TWeakObjectPtr<UNetworkPhysicsSettingsComponent> SettingsCurrent;
	TArray<TWeakObjectPtr<UPrimitiveComponent>> PendingDeleteFromComponentsToTargetsVR;

	bool NetworkPhysicsTickOffsetAssignedVR = false;
	int32 NetworkPhysicsTickOffsetVR = 0;

	virtual int32 GetNetworkPhysicsTickOffset() const override { return NetworkPhysicsTickOffsetVR; }

	void PrepareAsyncData_ExternalVR(const FRigidBodyErrorCorrection& ErrorCorrection);	
};

class IPhysicsReplicationFactoryVR : public IPhysicsReplicationFactory
{
public:

	virtual TUniquePtr<IPhysicsReplication> CreatePhysicsReplication(FPhysScene* OwningPhysScene) override
	{
		return TUniquePtr<IPhysicsReplication>(new FPhysicsReplicationVR(OwningPhysScene));
	}

};

USTRUCT()
struct VREXPANSIONPLUGIN_API FRepMovementVR : public FRepMovement
{
	GENERATED_USTRUCT_BODY()
public:

	FRepMovementVR();
	FRepMovementVR(FRepMovement& other);
	void CopyTo(FRepMovement& other) const;
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	bool GatherActorsMovement(AActor* OwningActor);
};

template<>
struct TStructOpsTypeTraits<FRepMovementVR> : public TStructOpsTypeTraitsBase2<FRepMovementVR>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

USTRUCT(BlueprintType)
struct VREXPANSIONPLUGIN_API FVRClientAuthReplicationData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRReplication")
		bool bUseClientAuthThrowing;

	UPROPERTY(EditAnywhere, NotReplicated, BlueprintReadOnly, Category = "VRReplication", meta = (ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100"))
		int32 UpdateRate;

	FTimerHandle ResetReplicationHandle;
	FTransform LastActorTransform;
	float TimeAtInitialThrow;
	bool bIsCurrentlyClientAuth;

	FVRClientAuthReplicationData() :
		bUseClientAuthThrowing(false),
		UpdateRate(30),
		LastActorTransform(FTransform::Identity),
		TimeAtInitialThrow(0.0f),
		bIsCurrentlyClientAuth(false)
	{

	}
};