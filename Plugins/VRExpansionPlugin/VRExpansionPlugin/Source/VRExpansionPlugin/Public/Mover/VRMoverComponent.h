

#pragma once
#include "CoreMinimal.h"
#include "MoverComponent.h"
#include "MoverTypes.h"
#include "GameFramework/Pawn.h"
#include "LayeredMove.h"
#include "VRMoverComponent.generated.h"

class UCurveVector;
class UCurveFloat;

DECLARE_LOG_CATEGORY_EXTERN(LogVRMoverComponent, Log, All);

USTRUCT(BlueprintType)
struct VREXPANSIONPLUGIN_API FLayeredMove_VRMovement : public FLayeredMoveBase
{
	GENERATED_USTRUCT_BODY()

	FLayeredMove_VRMovement();

	virtual ~FLayeredMove_VRMovement() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mover)
		FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mover)
		TObjectPtr<UCurveFloat> MagnitudeOverTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mover)
		uint8 SettingsFlags;

	virtual bool IsFinished(double CurrentSimTimeMs) const;

	virtual bool GenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, const UMoverComponent* MoverComp, UMoverBlackboard* SimBlackboard, FProposedMove& OutProposedMove) override;

	virtual FLayeredMoveBase* Clone() const override;

	virtual void NetSerialize(FArchive& Ar) override;

	virtual UScriptStruct* GetScriptStruct() const override;

	virtual FString ToSimpleString() const override;

	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;
};

template<>
struct TStructOpsTypeTraits< FLayeredMove_VRMovement > : public TStructOpsTypeTraitsBase2< FLayeredMove_VRMovement >
{
	enum
	{

		WithCopy = true
	};
};

USTRUCT(BlueprintType)
struct VREXPANSIONPLUGIN_API FVRMoverHMDSyncState : public FMoverDataStructBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
		FVector MoveDirectionIntent;

public:

	FVRMoverHMDSyncState()
	{

		MoveDirectionIntent = FVector::ZeroVector;
	}

	virtual ~FVRMoverHMDSyncState() {}

	virtual FMoverDataStructBase* Clone() const override;

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override
	{
		return false;
	}

	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override
	{
		return;
	}

};

template<>
struct TStructOpsTypeTraits< FVRMoverHMDSyncState > : public TStructOpsTypeTraitsBase2< FVRMoverHMDSyncState >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

UCLASS()
class VREXPANSIONPLUGIN_API AVRMoverBasePawn : public APawn, public IMoverInputProducerInterface
{
	GENERATED_BODY()

public:

	AVRMoverBasePawn(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = Mover)
		UMoverComponent* GetMoverComponent() const { return CharacterMotionComponent; }

	virtual void BeginPlay() override;

protected:

	virtual UPrimitiveComponent* GetMovementBase() const override;

	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

	virtual void OnProduceInput(float DeltaMs, FMoverInputCmdContext& InputCmdResult);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Produce Input", meta = (ScriptName = "OnProduceInput"))
		FMoverInputCmdContext OnProduceInputInBlueprint(float DeltaMs, FMoverInputCmdContext InputCmd);

protected:
	UPROPERTY(Category = Movement, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UMoverComponent> CharacterMotionComponent;

	uint8 bHasProduceInputinBpFunc : 1;
};