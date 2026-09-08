

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"

#include "Chaos/SimCallbackObject.h"
#include "Chaos/SimCallbackInput.h"
#include "Chaos/ParticleHandle.h"

#include "CollisionIgnoreSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(VRE_CollisionIgnoreLog, Log, All);

USTRUCT()
struct FChaosParticlePair
{
	GENERATED_BODY()

	Chaos::TPBDRigidParticleHandle<Chaos::FReal, 3>* ParticleHandle0;
	Chaos::TPBDRigidParticleHandle<Chaos::FReal, 3>* ParticleHandle1;

	FChaosParticlePair()
	{
		ParticleHandle0 = nullptr;
		ParticleHandle1 = nullptr;
	}

	FChaosParticlePair(Chaos::TPBDRigidParticleHandle<Chaos::FReal, 3>* pH1, Chaos::TPBDRigidParticleHandle<Chaos::FReal, 3>* pH2)
	{
		ParticleHandle0 = pH1;
		ParticleHandle1 = pH2;
	}

	FORCEINLINE bool operator==(const FChaosParticlePair& Other) const
	{
		return(
			(ParticleHandle0 == Other.ParticleHandle0 || ParticleHandle0 == Other.ParticleHandle1) &&
			(ParticleHandle1 == Other.ParticleHandle1 || ParticleHandle1 == Other.ParticleHandle0)
			);
	}
};

struct FSimCallbackInputVR : public Chaos::FSimCallbackInput
{
	virtual ~FSimCallbackInputVR() {}
	void Reset() 
	{
		ParticlePairs.Empty();
	}

	TArray<FChaosParticlePair> ParticlePairs;

	bool bIsInitialized;
};

struct FSimCallbackNoOutputVR : public Chaos::FSimCallbackOutput
{
	void Reset() {}
};

class FCollisionIgnoreSubsystemAsyncCallback : public Chaos::TSimCallbackObject<FSimCallbackInputVR, FSimCallbackNoOutputVR, Chaos::ESimCallbackOptions::ContactModification>
{

private:

	virtual void OnPreSimulate_Internal() override
	{

	}

	virtual void OnContactModification_Internal(Chaos::FCollisionContactModifier& Modifier) override;

};

USTRUCT()
struct FCollisionPrimPair
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> Prim1;
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> Prim2;

	FCollisionPrimPair()
	{
		Prim1 = nullptr;
		Prim2 = nullptr;
	}

	FORCEINLINE bool operator==(const FCollisionPrimPair& Other) const
	{
		if (!IsValid(Prim1) || !IsValid(Prim2))
			return false;

		if (!IsValid(Other.Prim1) || !IsValid(Other.Prim2))
			return false;

		return(
			(Prim1.Get() == Other.Prim1.Get() || Prim1.Get() == Other.Prim2.Get()) &&
			(Prim2.Get() == Other.Prim1.Get() || Prim2.Get() == Other.Prim2.Get())
			);
	}

	friend uint32 GetTypeHash(const FCollisionPrimPair& InKey)
	{
		return GetTypeHash(InKey.Prim1) ^ GetTypeHash(InKey.Prim2);
	}

};

USTRUCT()
struct FCollisionIgnorePair
{
	GENERATED_BODY()
public:

	FPhysicsActorHandle Actor1;
	UPROPERTY()
	FName BoneName1;
	FPhysicsActorHandle Actor2;
	UPROPERTY()
	FName BoneName2;

	void FlipElements()
	{
		FPhysicsActorHandle tH = Actor1;
		Actor1 = Actor2;
		Actor2 = tH;

		FName tN = BoneName1;
		BoneName1 = BoneName2;
		BoneName2 = tN;
	}

	FORCEINLINE bool operator==(const FCollisionIgnorePair& Other) const
	{
		return (
			(BoneName1 == Other.BoneName1 || BoneName1 == Other.BoneName2) &&
			(BoneName2 == Other.BoneName2 || BoneName2 == Other.BoneName1)
			);
	}

	FORCEINLINE bool operator==(const FName& Other) const
	{
		return (BoneName1 == Other || BoneName2 == Other);
	}
};

USTRUCT()
struct FCollisionIgnorePairArray
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TArray<FCollisionIgnorePair> PairArray;
};

UCLASS()
class VREXPANSIONPLUGIN_API UCollisionIgnoreSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	UCollisionIgnoreSubsystem() :
		Super()
	{
		ContactModifierCallback = nullptr;
	}

	FCollisionIgnoreSubsystemAsyncCallback* ContactModifierCallback;

	void ConstructInput();

	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;

	}

	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
	}

	virtual void Deinitialize() override;

	UPROPERTY()
	TMap<FCollisionPrimPair, FCollisionIgnorePairArray> CollisionTrackedPairs;

	UPROPERTY()
	TMap<FCollisionPrimPair, FCollisionIgnorePairArray> RemovedPairs;

	void UpdateTimer(bool bChangesWereMade);

	UFUNCTION(Category = "Collision")
		void CheckActiveFilters();

	void InitiateIgnore();

	void SetComponentCollisionIgnoreState(bool bIterateChildren1, bool bIterateChildren2, UPrimitiveComponent* Prim1, FName OptionalBoneName1, UPrimitiveComponent* Prim2, FName OptionalBoneName2, bool bIgnoreCollision, bool bCheckFilters = false);
	void RemoveComponentCollisionIgnoreState(UPrimitiveComponent* Prim1);
	bool IsComponentIgnoringCollision(UPrimitiveComponent* Prim1);
	bool AreComponentsIgnoringCollisions(UPrimitiveComponent* Prim1, UPrimitiveComponent* Prim2);
	bool HasCollisionIgnorePairs();
private:

	FTimerHandle UpdateHandle;

};
