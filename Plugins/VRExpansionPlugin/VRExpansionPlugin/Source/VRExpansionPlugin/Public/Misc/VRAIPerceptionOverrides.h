

#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "VRBaseCharacter.h"
#include "GenericTeamAgentInterface.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig.h"
#include "WorldCollision.h"
#include "Misc/MTAccessDetector.h"

#include "VRAIPerceptionOverrides.generated.h"

class IAISightTargetInterface;
class UAISense_Sight_VR;

class FGameplayDebuggerCategory;
class UAIPerceptionComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogAIPerceptionVR, Warning, All);

UCLASS(meta = (DisplayName = "AI Sight VR config"))
class VREXPANSIONPLUGIN_API UAISenseConfig_Sight_VR : public UAISenseConfig
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", NoClear, config)
		TSubclassOf<UAISense_Sight_VR> Implementation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", config, meta = (UIMin = 0.0, ClampMin = 0.0))
		float SightRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", config, meta = (UIMin = 0.0, ClampMin = 0.0))
		float LoseSightRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", config, meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 180.0, ClampMax = 180.0, DisplayName = "PeripheralVisionHalfAngleDegrees"))
		float PeripheralVisionAngleDegrees;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", config)
		FAISenseAffiliationFilter DetectionByAffiliation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", config)
		float AutoSuccessRangeFromLastSeenLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", config, meta = (UIMin = 0.0, ClampMin = 0.0))
		float PointOfViewBackwardOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", config, meta = (UIMin = 0.0, ClampMin = 0.0))
		float NearClippingRadius;

	virtual TSubclassOf<UAISense> GetSenseImplementation() const override;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif 

#if WITH_GAMEPLAY_DEBUGGER
	virtual void DescribeSelfToGameplayDebugger(const UAIPerceptionComponent* PerceptionComponent, FGameplayDebuggerCategory* DebuggerCategory) const;
#endif 

};

namespace ESightPerceptionEventNameVR
{
	enum Type
	{
		Undefined,
		GainedSight,
		LostSight
	};
}

USTRUCT()
struct VREXPANSIONPLUGIN_API FAISightEventVR
{
	GENERATED_USTRUCT_BODY()

		typedef UAISense_Sight_VR FSenseClass;

	float Age;
	ESightPerceptionEventNameVR::Type EventType;

	UPROPERTY()
		TObjectPtr<AActor> SeenActor;

	UPROPERTY()
		TObjectPtr<AActor> Observer;

	FAISightEventVR() : SeenActor(nullptr), Observer(nullptr) {}

	FAISightEventVR(AActor* InSeenActor, AActor* InObserver, ESightPerceptionEventNameVR::Type InEventType)
		: Age(0.f), EventType(InEventType), SeenActor(InSeenActor), Observer(InObserver)
	{
	}
};

struct FAISightTargetVR
{
	typedef uint32 FTargetId;
	static const FTargetId InvalidTargetId;

	TWeakObjectPtr<AActor> Target;
	TWeakInterfacePtr<IAISightTargetInterface> WeakSightTargetInterface;
	FGenericTeamId TeamId;
	FTargetId TargetId;

	FAISightTargetVR(AActor* InTarget = NULL, FGenericTeamId InTeamId = FGenericTeamId::NoTeam);

	FORCEINLINE FVector GetLocationSimple() const
	{

		const AVRBaseCharacter * VRChar = Cast<const AVRBaseCharacter>(Target);
		return Target.IsValid() ? (VRChar != nullptr ? VRChar->GetVRLocation_Inline() : Target->GetActorLocation()) : FVector::ZeroVector;
	}

	FORCEINLINE const AActor* GetTargetActor() const { return Target.Get(); }
};

struct FAISightQueryVR
{
	FPerceptionListenerID ObserverId;
	FAISightTargetVR::FTargetId TargetId;

	float Score;
	float Importance;

	FVector LastSeenLocation;

	mutable int32 UserData;

	union
	{

		struct
		{
			uint64 bLastResult : 1;
			uint64 LastProcessedFrameNumber : 63;
		} FrameInfo;

		struct
		{
			uint32 bLastResult : 1;
			uint32 Index : 31;
			uint32 FrameNumber;
		} TraceInfo;
	};

	FAISightQueryVR(FPerceptionListenerID ListenerId = FPerceptionListenerID::InvalidID(), FAISightTargetVR::FTargetId Target = FAISightTargetVR::InvalidTargetId)
		: ObserverId(ListenerId), TargetId(Target), Score(0), Importance(0), LastSeenLocation(FAISystem::InvalidLocation), UserData(0)
	{
		FrameInfo.bLastResult = false;
		FrameInfo.LastProcessedFrameNumber = GFrameCounter;
	}

	float GetAge() const
	{
		return (float)(GFrameCounter - FrameInfo.LastProcessedFrameNumber);
	}

	void RecalcScore()
	{
		Score = GetAge() + Importance;
	}

	void OnProcessed()
	{
		FrameInfo.LastProcessedFrameNumber = GFrameCounter;
	}

	void ForgetPreviousResult()
	{
		LastSeenLocation = FAISystem::InvalidLocation;
		SetLastResult(false);
	}

	bool GetLastResult() const
	{
		return FrameInfo.bLastResult;
	}

	void SetLastResult(const bool bValue)
	{
		FrameInfo.bLastResult = bValue;
	}

	void SetTraceInfo(const FTraceHandle& TraceHandle)
	{
		check((TraceHandle._Data.Index & (static_cast<uint32>(1) << 31)) == 0);
		TraceInfo.Index = TraceHandle._Data.Index;
		TraceInfo.FrameNumber = TraceHandle._Data.FrameNumber;
	}

	class FSortPredicate
	{
	public:
		FSortPredicate()
		{}

		bool operator()(const FAISightQueryVR& A, const FAISightQueryVR& B) const
		{
			return A.Score > B.Score;
		}
	};
};

struct FAISightQueryIDVR
{
	FPerceptionListenerID ObserverId;
	FAISightTargetVR::FTargetId TargetId;

	FAISightQueryIDVR(FPerceptionListenerID ListenerId = FPerceptionListenerID::InvalidID(), FAISightTargetVR::FTargetId Target = FAISightTargetVR::InvalidTargetId)
		: ObserverId(ListenerId), TargetId(Target)
	{
	}

	FAISightQueryIDVR(const FAISightQueryVR& Query)
		: ObserverId(Query.ObserverId), TargetId(Query.TargetId)
	{
	}
};

DECLARE_DELEGATE_FiveParams(FOnPendingVisibilityQueryProcessedDelegateVR, const FAISightQueryID&, const bool, const float, const FVector&, const TOptional<int32>&);

UCLASS(ClassGroup = AI, config = Game)
class VREXPANSIONPLUGIN_API UAISense_Sight_VR : public UAISense
{
	GENERATED_UCLASS_BODY()

public:
	struct FDigestedSightProperties
	{
		float PeripheralVisionAngleCos;
		float SightRadiusSq;
		float AutoSuccessRangeSqFromLastSeenLocation;
		float LoseSightRadiusSq;
		float PointOfViewBackwardOffset;
		float NearClippingRadiusSq;
		uint8 AffiliationFlags;

		FDigestedSightProperties();
		FDigestedSightProperties(const UAISenseConfig_Sight_VR& SenseConfig);
	};

	typedef TMap<FAISightTargetVR::FTargetId, FAISightTargetVR> FTargetsContainer;
	FTargetsContainer ObservedTargets;
	TMap<FPerceptionListenerID, FDigestedSightProperties> DigestedProperties;

	int32 NextOutOfRangeIndex = 0;
	bool bSightQueriesOutOfRangeDirty = true;
	TArray<FAISightQueryVR> SightQueriesOutOfRange;
	TArray<FAISightQueryVR> SightQueriesInRange;
	TArray<FAISightQueryVR> SightQueriesPending;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AI Perception", config)
		int32 MaxTracesPerTick;

	UPROPERTY(EditDefaultsOnly, Category = "AI Perception", config)
		int32 MaxAsyncTracesPerTick;

	UPROPERTY(EditDefaultsOnly, Category = "AI Perception", config)
		int32 MinQueriesPerTimeSliceCheck;

	UPROPERTY(EditDefaultsOnly, Category = "AI Perception", config)
		double MaxTimeSlicePerTick;

	UPROPERTY(EditDefaultsOnly, Category = "AI Perception", config)
		float HighImportanceQueryDistanceThreshold;

	float HighImportanceDistanceSquare;

	UPROPERTY(EditDefaultsOnly, Category = "AI Perception", config)
		float MaxQueryImportance;

	UPROPERTY(EditDefaultsOnly, Category = "AI Perception", config)
		float SightLimitQueryImportance;

	UPROPERTY(EditDefaultsOnly, Category = "AI Perception", config)
		float PendingQueriesBudgetReductionRatio;

	UPROPERTY(EditDefaultsOnly, Category = "AI Perception", config)
		bool bUseAsynchronousTraceForDefaultSightQueries;

	ECollisionChannel DefaultSightCollisionChannel;

	FOnPendingVisibilityQueryProcessedDelegateVR OnPendingCanBeSeenQueryProcessedDelegate;
	FTraceDelegate OnPendingTraceQueryProcessedDelegate;

	UE_MT_DECLARE_TS_RW_ACCESS_DETECTOR(QueriesListAccessDetector);

public:

	virtual void PostInitProperties() override;

	void RegisterEvent(const FAISightEventVR& Event);

	virtual void RegisterSource(AActor& SourceActors) override;
	virtual void UnregisterSource(AActor& SourceActor) override;

	virtual void OnListenerForgetsActor(const FPerceptionListener& Listener, AActor& ActorToForget) override;
	virtual void OnListenerForgetsAll(const FPerceptionListener& Listener) override;

#if WITH_GAMEPLAY_DEBUGGER_MENU
	virtual void DescribeSelfToGameplayDebugger(const UAIPerceptionSystem& PerceptionSystem, FGameplayDebuggerCategory& DebuggerCategory) const override;
#endif 

protected:
	virtual float Update() override;

	UAISense_Sight::EVisibilityResult ComputeVisibility(UWorld* World, FAISightQueryVR& SightQuery, FPerceptionListener& Listener, const AActor* ListenerActor, FAISightTargetVR& Target, AActor* TargetActor, const FDigestedSightProperties& PropDigest, float& OutStimulusStrength, FVector& OutSeenLocation, int32& OutNumberOfLoSChecksPerformed, int32& OutNumberOfAsyncLosCheckRequested) const;
	virtual bool ShouldAutomaticallySeeTarget(const FDigestedSightProperties& PropDigest, FAISightQueryVR* SightQuery, FPerceptionListener& Listener, AActor* TargetActor, float& OutStimulusStrength) const;
	UE_DEPRECATED(5.3, "Please use the UpdateQueryVisibilityStatus version which takes an Actor& instead.")
		void UpdateQueryVisibilityStatus(FAISightQueryVR& SightQuery, FPerceptionListener& Listener, const bool bIsVisible, const FVector& SeenLocation, const float StimulusStrength, AActor* TargetActor, const FVector& TargetLocation) const;

	void UpdateQueryVisibilityStatus(FAISightQueryVR& SightQuery, FPerceptionListener& Listener, const bool bIsVisible, const FVector& SeenLocation, const float StimulusStrength, AActor& TargetActor, const FVector& TargetLocation) const;

	void OnPendingCanBeSeenQueryProcessed(const FAISightQueryID& QueryID, const bool bIsVisible, const float StimulusStrength, const FVector& SeenLocation, const TOptional<int32>& UserData);
	void OnPendingTraceQueryProcessed(const FTraceHandle& TraceHandle, FTraceDatum& TraceDatum);
	void OnPendingQueryProcessed(const int32 SightQueryIndex, const bool bIsVisible, const float StimulusStrength, const FVector& SeenLocation, const TOptional<int32>& UserData, const TOptional<AActor*> InTargetActor = NullOpt);

	void OnNewListenerImpl(const FPerceptionListener& NewListener);
	void OnListenerUpdateImpl(const FPerceptionListener& UpdatedListener);
	void OnListenerRemovedImpl(const FPerceptionListener& RemovedListener);
	virtual void OnListenerConfigUpdated(const FPerceptionListener& UpdatedListener) override;

	void GenerateQueriesForListener(const FPerceptionListener& Listener, const FDigestedSightProperties& PropertyDigest, const TFunction<void(FAISightQueryVR&)>& OnAddedFunc = nullptr);

	void RemoveAllQueriesByListener(const FPerceptionListener& Listener, const TFunction<void(const FAISightQueryVR&)>& OnRemoveFunc = nullptr);
	void RemoveAllQueriesToTarget(const FAISightTargetVR::FTargetId& TargetId, const TFunction<void(const FAISightQueryVR&)>& OnRemoveFunc = nullptr);

	void RemoveAllQueriesToTarget_Internal(const FAISightTargetVR::FTargetId& TargetId, const TFunction<void(const FAISightQueryVR&)>& OnRemoveFunc = nullptr);

	bool RegisterTarget(AActor& TargetActor, const TFunction<void(FAISightQueryVR&)>& OnAddedFunc = nullptr);

	float CalcQueryImportance(const FPerceptionListener& Listener, const FVector& TargetLocation, const float SightRadiusSq) const;
	bool RegisterNewQuery(const FPerceptionListener& Listener, const IGenericTeamAgentInterface* ListenersTeamAgent, const AActor& TargetActor, const FAISightTargetVR::FTargetId& TargetId, const FVector& TargetLocation, const FDigestedSightProperties& PropDigest, const TFunction<void(FAISightQueryVR&)>& OnAddedFunc);

public:
	UE_DEPRECATED(4.25, "Not needed anymore done automatically at the beginning of each update.")
		FORCEINLINE void SortQueries() {}

	enum FQueriesOperationPostProcess
	{
		DontSort,
		Sort
	};
};
