

#pragma once

#include "CoreMinimal.h"

#include "SceneViewExtension.h"
#include "VRBPDatatypes.h"
#include "MotionControllerComponent.h"
#include "VRGripInterface.h"
#include "GripScripts/VRGripScriptBase.h"
#include "GripMotionControllerComponent.generated.h"

class AVRBaseCharacter;
class AVRCharacter;
struct FXRDeviceId;

#define RESET_REPLIFETIME_CONDITION_PRIVATE_PROPERTY(c,v,cond)  ResetReplicatedLifetimeProperty(StaticClass(), c::StaticClass(), FName(TEXT(#v)), cond, OutLifetimeProps);

DECLARE_LOG_CATEGORY_EXTERN(LogVRMotionController, Log, All);

DECLARE_STATS_GROUP(TEXT("TICKGrip"), STATGROUP_TickGrip, STATCAT_Advanced);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRGripControllerOnTrackingEventSignature, const ETrackingStatus &, NewTrackingStatus);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVROnControllerGripSignature, const FBPActorGripInformation &, GripInformation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVROnControllerDropSignature, const FBPActorGripInformation &, GripInformation, bool, bWasSocketed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FVROnControllerSocketSignature, const FBPActorGripInformation&, GripInformation, const USceneComponent*, NewParentComp, FName, OptionalSocketName, FTransform, RelativeTransformToParent, bool, bWeldingBodies);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVROnControllerTeleportedGripsSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVRGripControllerOnGripOutOfRange, const FBPActorGripInformation &, GripInformation, float, Distance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVRGripControllerOnProfileTransformChanged, const FTransform &, NewRelTransForProcComps, const FTransform &, NewProfileTransform);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVROnClientAuthGripConflict, UObject *, Object, EVRClientAuthConflictResolutionMode, ResolutionMethod);

class VREXPANSIONPLUGIN_API FExpandedLateUpdateManager
{
public:
	FExpandedLateUpdateManager();

	virtual ~FExpandedLateUpdateManager() {}

	void Setup(const FTransform& ParentToWorld, UGripMotionControllerComponent* Component, bool bSkipLateUpdate);

	void Apply_RenderThread(FSceneInterface* Scene, const FTransform& OldRelativeTransform, const FTransform& NewRelativeTransform);

	bool GetSkipLateUpdate_RenderThread() const { return UpdateStates[LateUpdateRenderReadIndex].bSkip; }

public:

	void GatherLateUpdatePrimitives(USceneComponent* ParentComponent);
	void ProcessGripArrayLateUpdatePrimitives(UGripMotionControllerComponent* MotionController, TArray<FBPActorGripInformation> & GripArray);

	void CacheSceneInfo(USceneComponent* Component);

	struct FLateUpdateState
	{
		FLateUpdateState()
			: ParentToWorld(FTransform::Identity)
			, bSkip(false)
			, TrackingNumber(-1)
		{}

		FTransform ParentToWorld;

		TMap<FPrimitiveSceneInfo*, int32> Primitives;

		bool bSkip;

		int64 TrackingNumber;
	};

	FLateUpdateState UpdateStates[2];
	int32 LateUpdateGameWriteIndex;
	int32 LateUpdateRenderReadIndex;
};

USTRUCT()
struct FGripComponentEndPhysicsTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

		UGripMotionControllerComponent* Target;

	virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

	virtual FString DiagnosticMessage() override;

	virtual FName DiagnosticContext(bool bDetailed) override;
};

template<>
struct TStructOpsTypeTraits<FGripComponentEndPhysicsTickFunction> : public TStructOpsTypeTraitsBase2<FGripComponentEndPhysicsTickFunction>
{
	enum
	{
		WithCopy = false
	};
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = MotionController)
class VREXPANSIONPLUGIN_API UGripMotionControllerComponent : public UMotionControllerComponent
{

public:

	FGripComponentEndPhysicsTickFunction EndPhysicsTickFunction;
	friend struct FGripComponentEndPhysicsTickFunction;

	void EndPhysicsTickComponent(FGripComponentEndPhysicsTickFunction& ThisTickFunction);
	void RegisterEndPhysicsTick(bool bRegister);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripMotionController|Advanced")
		bool bProjectNonSimulatingGrips;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripMotionController|Advanced")
		bool bSweepGripTeleports = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripMotionController|Advanced")
		TSubclassOf<class UVRGripScriptBase> DefaultGripScriptClass;

	UPROPERTY(VisibleDefaultsOnly, Transient, BlueprintReadOnly, Category = "GripMotionController|Advanced")
		TObjectPtr<UVRGripScriptBase> DefaultGripScript;

	void InitializeLerpToHand(FBPActorGripInformation& GripInfo);
	void HandleGlobalLerpToHand(FBPActorGripInformation& GripInformation, FTransform& WorldTransform, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		void CancelGlobalLerpToHand(uint8 GripID);

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnControllerGripSignature OnLerpToHandFinished;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Advanced|Tracking")
		bool bScaleTracking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Advanced|Tracking", meta = (ClampMin = "0.1", UIMin = "0.1", EditCondition = "bScaleTracking"))
		FVector TrackingScaler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Advanced|Tracking")
		bool bLimitMinHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Advanced|Tracking", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bLimitMinHeight"))
		float MinimumHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Advanced|Tracking")
		bool bLimitMaxHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Advanced|Tracking", meta = (ClampMin = "0.1", UIMin = "0.1", EditCondition = "bLimitMinHeight"))
		float MaximumHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Advanced|Tracking")
		bool bLeashToHMD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Advanced|Tracking", meta = (ClampMin = "0.1", UIMin = "0.1", EditCondition = "bLeashToHMD"))
		float LeashRange;

	void ApplyTrackingParameters(FVector& OriginalPosition, bool bIsInGameThread, bool bApplyZeroing = true);
	bool HasTrackingParameters();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Advanced")
		bool bConstrainToPivot;

	UPROPERTY()
		TObjectPtr<AVRCharacter> AttachChar;
	void UpdateTracking(float DeltaTime);
	virtual void OnAttachmentChanged() override;

	FVector LastLocationForLateUpdate;
	FTransform LastRelativePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Smoothing")
		bool bSmoothHandTracking;

	bool bWasSmoothingHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Smoothing")
		bool bSmoothWithEuroLowPassFunction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Smoothing")
		float SmoothingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|Smoothing")
		FBPEuroLowPassFilterTrans EuroSmoothingParams;

	FTransform LastSmoothRelativeTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|ComponentVelocity")
		EVRVelocityType VelocityCalculationType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|ComponentVelocity")
		bool bSampleVelocityInWorldSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|ComponentVelocity")
		int32 VelocitySamples;

	FBPLowPassPeakFilter PeakFilter;

	virtual FVector GetComponentVelocity() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController")
		bool bOffsetByControllerProfile;

	FTransform CurrentControllerProfileTransform;

	UPROPERTY(BlueprintAssignable, Category = "GripMotionController")
		FVRGripControllerOnProfileTransformChanged OnControllerProfileTransformChanged;

	UPROPERTY(BlueprintAssignable, Category = "GripMotionController")
		FVRGripControllerOnGripOutOfRange OnGripOutOfRange;

private:

	GENERATED_BODY()

public:
	UGripMotionControllerComponent(const FObjectInitializer& ObjectInitializer);

	~UGripMotionControllerComponent();

	void CalculateGripVelocity(FBPActorGripInformation &GripToFill, UPrimitiveComponent* ComponentToSample, float DeltaTime);

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void InitializeComponent() override;
	virtual void OnUnregister() override;

	virtual void Deactivate() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;
	virtual void BeginPlay() override;

protected:

	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
	virtual void SendRenderTransform_Concurrent() override;

	void OnModularFeatureUnregistered(const FName& Type, class IModularFeature* ModularFeature);

	struct FRenderTrackingParams
	{
		FTransform GripRenderThreadRelativeTransform = FTransform::Identity;
		FVector GripRenderThreadComponentScale = FVector::ZeroVector;
		FTransform GripRenderThreadProfileTransform = FTransform::Identity;
		FVector GripRenderThreadLastLocationForLateUpdate = FVector::ZeroVector;

		bool bRenderSmoothHandTracking = false;
		bool bRenderSmoothWithEuroLowPassFunction = false;
		float RenderSmoothingSpeed = 0.0f;
		FBPEuroLowPassFilterTrans RenderEuroSmoothingParams;
		FTransform RenderLastSmoothRelativeTransform = FTransform::Identity;
		float RenderLastDeltaTime = 0.0f;
	}LateUpdateParams;

	FDelegateHandle NewControllerProfileEvent_Handle;
	UFUNCTION()
	void NewControllerProfileLoaded();
	void GetCurrentProfileTransform(bool bBindToNoticationDelegate);

public:

	UPROPERTY(BlueprintAssignable, Category = "GripMotionController")
		FVRGripControllerOnTrackingEventSignature OnTrackingChanged;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnControllerGripSignature OnGrippedObject;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnControllerDropSignature OnDroppedObject;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnControllerSocketSignature OnSocketingObject;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnControllerTeleportedGripsSignature OnTeleportedGrips;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnControllerGripSignature OnSecondaryGripAdded;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnControllerGripSignature OnSecondaryGripRemoved;

	TArray<uint8> SecondaryGripIDs;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnControllerGripSignature OnGripTransformChanged;

	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "HandType", CompactNodeTitle = "HandType"))
		void GetHandType(EControllerHand& Hand);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GripMotionController|CustomPivot")
		TObjectPtr<USceneComponent> CustomPivotComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GripMotionController|CustomPivot")
		FName CustomPivotComponentSocketName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GripMotionController|CustomPivot")
		bool bSkipPivotTransformAdjustment;

	UFUNCTION(BlueprintCallable, Category = "GripMotionController|CustomPivot")
		void SetCustomPivotComponent(USceneComponent * NewCustomPivotComponent, FName PivotSocketName = NAME_None);

	UFUNCTION(BlueprintPure, Category = "GripMotionController", meta = (DisplayName = "GetPivotTransform"))
		FTransform GetPivotTransform_BP();

	UFUNCTION(BlueprintPure, Category = "GripMotionController", meta = (DisplayName = "GetPivotLocation"))
		FVector GetPivotLocation_BP();

	FORCEINLINE FTransform GetPivotTransform()
	{
		return IsValid(CustomPivotComponent) ? CustomPivotComponent->GetSocketTransform(CustomPivotComponentSocketName) : this->GetComponentTransform();
	}

	FORCEINLINE FVector GetPivotLocation()
	{
		return IsValid(CustomPivotComponent) ? CustomPivotComponent->GetSocketLocation(CustomPivotComponentSocketName) : this->GetComponentLocation();
	}

	uint8 GripIDIncrementer;

	inline uint8 GetNextGripID(bool bIsLocalGrip)
	{

		if (!bIsLocalGrip) 
		{
			if (GripIDIncrementer < 127)
				GripIDIncrementer++;
			else
				GripIDIncrementer = (INVALID_VRGRIP_ID + 1);

			return GripIDIncrementer;
		}
		else 
		{

			if (!IsServer())
			{
				if (GripIDIncrementer < 63)
					GripIDIncrementer++;
				else
					GripIDIncrementer = (INVALID_VRGRIP_ID + 1);

				return GripIDIncrementer + 128;
			}
			else
			{
				if (GripIDIncrementer < 63)
					GripIDIncrementer++;
				else
					GripIDIncrementer = (INVALID_VRGRIP_ID + 1);

				return GripIDIncrementer + 128 + 64;
			}
		}
	}

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GripMotionController", ReplicatedUsing = OnRep_GrippedObjects)
	TArray<FBPActorGripInformation> GrippedObjects;

	void DIRTY_GRIPPED_OBJECTS();

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GripMotionController", ReplicatedUsing = OnRep_LocallyGrippedObjects)
	TArray<FBPActorGripInformation> LocallyGrippedObjects;

	void DIRTY_LOCALLY_GRIPPED_OBJECTS();

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GripMotionController", ReplicatedUsing = OnRep_LocalTransaction)
		TArray<FBPActorGripInformation> LocalTransactionBuffer;

	UFUNCTION(Reliable, Client, Category = "GripMotionController")
	void Client_NotifyInvalidLocalGrip(UObject * LocallyGrippedObject, uint8 GripID, bool bWasAGripConflict = false);

	UFUNCTION(Reliable, Server, WithValidation, Category = "GripMotionController")
	void Server_NotifyLocalGripAddedOrChanged(const FBPActorGripInformation & newGrip);

	UFUNCTION(Reliable, Server, WithValidation)
		void Server_NotifySecondaryAttachmentChanged(
			uint8 GripID,
			const FBPSecondaryGripInfo& SecondaryGripInfo);

	UFUNCTION(Reliable, Server, WithValidation)
		void Server_NotifySecondaryAttachmentChanged_Retain(
			uint8 GripID,
			const FBPSecondaryGripInfo& SecondaryGripInfo, const FTransform_NetQuantize & NewRelativeTransform);

	UFUNCTION(Reliable, Server, WithValidation)
	void Server_NotifyLocalGripRemoved(uint8 GripID, const FTransform_NetQuantize &TransformAtDrop, FVector_NetQuantize100 OptAngularVelocity, FVector_NetQuantize100 OptLinearVelocity);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController|ClientAuth")
		EVRClientAuthConflictResolutionMode ClientAuthConflictResolutionMethod;

	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
		FVROnClientAuthGripConflict OnClientAuthGripConflict;

	UFUNCTION(Reliable, Server, WithValidation, Category = "GripMotionController")
		void Server_NotifyHandledTransaction(uint8 GripID);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController")
	bool bAlwaysSendTickGrip;

	void CleanUpBadGrip(TArray<FBPActorGripInformation> &GrippedObjectsArray, int GripIndex, bool bReplicatedArray);
	void CleanUpBadPhysicsHandles();

	bool UpdatePhysicsHandle(uint8 GripID, bool bFullyRecreate = true);
	bool UpdatePhysicsHandle(const FBPActorGripInformation & GripInfo, bool bFullyRecreate = true);

	inline void NotifyGripTransformChanged(const FBPActorGripInformation & GripInfo)
	{
		if (OnGripTransformChanged.IsBound())
		{
			FBPActorGripInformation CurrentGrip;
			EBPVRResultSwitch Result;
			GetGripByID(CurrentGrip, GripInfo.GripID, Result);
			if (Result == EBPVRResultSwitch::OnSucceeded)
			{
				OnGripTransformChanged.Broadcast(CurrentGrip);
			}
		}
	}

	inline void ReCreateGrip(FBPActorGripInformation & GripInfo)
	{
		int HandleIndex = 0;
		if (GetPhysicsGripIndex(GripInfo, HandleIndex))
		{
			DestroyPhysicsHandle(GripInfo);
		}

		NotifyGrip(GripInfo, true);
	}

	inline bool HandleGripReplication(FBPActorGripInformation & Grip, FBPActorGripInformation * OldGripInfo = nullptr)
	{
		if (Grip.ValueCache.bWasInitiallyRepped && Grip.GripID != Grip.ValueCache.CachedGripID)
		{

			Grip.ClearNonReppingItems();
		}

		if (Grip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep)
		{

			Grip.ValueCache.bWasInitiallyRepped = true;

			Grip.GrippedObject = nullptr;

			Grip.bIsPaused = true;
		}

		if (!Grip.ValueCache.bWasInitiallyRepped) 
		{
			Grip.ValueCache.bWasInitiallyRepped = NotifyGrip(Grip); 

			if (!Grip.ValueCache.bWasInitiallyRepped)
			{

				return false;
			}

			if(Grip.SecondaryGripInfo.bHasSecondaryAttachment)
			{

				Grip.SecondaryGripInfo.SecondaryGripDistance = 0.0f;

				if (FMath::IsNearlyZero(Grip.SecondaryGripInfo.LerpToRate)) 
					Grip.SecondaryGripInfo.GripLerpState = EGripLerpState::NotLerping;
				else
				{
					Grip.SecondaryGripInfo.curLerp = Grip.SecondaryGripInfo.LerpToRate;
					Grip.SecondaryGripInfo.GripLerpState = EGripLerpState::StartLerp;
				}

				if (Grip.GrippedObject && Grip.GrippedObject->IsValidLowLevelFast() && Grip.GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
				{
					SecondaryGripIDs.Add(Grip.GripID);
					IVRGripInterface::Execute_OnSecondaryGrip(Grip.GrippedObject, this, Grip.SecondaryGripInfo.SecondaryAttachment, Grip);

					TArray<UVRGripScriptBase*> GripScripts;
					if (IVRGripInterface::Execute_GetGripScripts(Grip.GrippedObject, GripScripts))
					{
						for (UVRGripScriptBase* Script : GripScripts)
						{
							if (Script)
							{
								Script->OnSecondaryGrip(this, Grip.SecondaryGripInfo.SecondaryAttachment, Grip);
							}
						}
					}
				}

				OnSecondaryGripAdded.Broadcast(Grip);	
			}

		}
		else if(OldGripInfo != nullptr) 
		{

			if ((OldGripInfo->SecondaryGripInfo.bHasSecondaryAttachment != Grip.SecondaryGripInfo.bHasSecondaryAttachment) ||
				(OldGripInfo->SecondaryGripInfo.SecondaryAttachment != Grip.SecondaryGripInfo.SecondaryAttachment) ||
				(!OldGripInfo->SecondaryGripInfo.SecondaryRelativeTransform.Equals(Grip.SecondaryGripInfo.SecondaryRelativeTransform)))
			{

				Grip.SecondaryGripInfo.SecondaryGripDistance = 0.0f;

				if (FMath::IsNearlyZero(Grip.SecondaryGripInfo.LerpToRate)) 
					Grip.SecondaryGripInfo.GripLerpState = EGripLerpState::NotLerping;
				else
				{

					if (Grip.SecondaryGripInfo.bHasSecondaryAttachment)
					{
						Grip.SecondaryGripInfo.curLerp = Grip.SecondaryGripInfo.LerpToRate;
						Grip.SecondaryGripInfo.GripLerpState = EGripLerpState::StartLerp;
					}
					else 
					{
						Grip.SecondaryGripInfo.curLerp = Grip.SecondaryGripInfo.LerpToRate;
						Grip.SecondaryGripInfo.GripLerpState = EGripLerpState::EndLerp;
					}
				}

				bool bSendReleaseEvent = ((!Grip.SecondaryGripInfo.bHasSecondaryAttachment && OldGripInfo->SecondaryGripInfo.bHasSecondaryAttachment) ||
										((Grip.SecondaryGripInfo.bHasSecondaryAttachment && OldGripInfo->SecondaryGripInfo.bHasSecondaryAttachment) &&
										(OldGripInfo->SecondaryGripInfo.SecondaryAttachment != Grip.SecondaryGripInfo.SecondaryAttachment)));

				bool bSendGripEvent =	(Grip.SecondaryGripInfo.bHasSecondaryAttachment && 
										(!OldGripInfo->SecondaryGripInfo.bHasSecondaryAttachment || (OldGripInfo->SecondaryGripInfo.SecondaryAttachment != Grip.SecondaryGripInfo.SecondaryAttachment)));

				if (bSendReleaseEvent)
				{
					if (Grip.GrippedObject && Grip.GrippedObject->IsValidLowLevelFast() && Grip.GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
					{
						IVRGripInterface::Execute_OnSecondaryGripRelease(Grip.GrippedObject, this, OldGripInfo->SecondaryGripInfo.SecondaryAttachment, Grip);

						TArray<UVRGripScriptBase*> GripScripts;
						if (IVRGripInterface::Execute_GetGripScripts(Grip.GrippedObject, GripScripts))
						{
							for (UVRGripScriptBase* Script : GripScripts)
							{
								if (Script)
								{
									Script->OnSecondaryGripRelease(this, OldGripInfo->SecondaryGripInfo.SecondaryAttachment, Grip);
								}
							}
						}
					}

					SecondaryGripIDs.Remove(Grip.GripID);
					OnSecondaryGripRemoved.Broadcast(Grip);
				}

				if (bSendGripEvent)
				{
					if (Grip.GrippedObject && Grip.GrippedObject->IsValidLowLevelFast() && Grip.GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
					{
						SecondaryGripIDs.Add(Grip.GripID);
						IVRGripInterface::Execute_OnSecondaryGrip(Grip.GrippedObject, this, Grip.SecondaryGripInfo.SecondaryAttachment, Grip);

						TArray<UVRGripScriptBase*> GripScripts;
						if (IVRGripInterface::Execute_GetGripScripts(Grip.GrippedObject, GripScripts))
						{
							for (UVRGripScriptBase* Script : GripScripts)
							{
								if (Script)
								{
									Script->OnSecondaryGrip(this, Grip.SecondaryGripInfo.SecondaryAttachment, Grip);
								}
							}
						}
					}

					OnSecondaryGripAdded.Broadcast(Grip);
				}
			}

			if (OldGripInfo->GripCollisionType != Grip.GripCollisionType ||
				OldGripInfo->GripMovementReplicationSetting != Grip.GripMovementReplicationSetting ||
				OldGripInfo->GrippedBoneName != Grip.GrippedBoneName ||
				OldGripInfo->AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings != Grip.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings
				)
			{
				ReCreateGrip(Grip); 
			}
			else 
			{
				bool bTransformChanged = !OldGripInfo->RelativeTransform.Equals(Grip.RelativeTransform);

				if (!FMath::IsNearlyEqual(OldGripInfo->Stiffness, Grip.Stiffness) ||
					!FMath::IsNearlyEqual(OldGripInfo->Damping, Grip.Damping) ||
					OldGripInfo->AdvancedGripSettings.PhysicsSettings != Grip.AdvancedGripSettings.PhysicsSettings ||
					bTransformChanged
					)
				{
					UpdatePhysicsHandle(Grip);

					if (bTransformChanged)
					{
						NotifyGripTransformChanged(Grip);
					}
				}
			}
		}

		Grip.ValueCache.CachedGripID = Grip.GripID;

		return true;
	}

	void CheckTransactionBuffer();

	UFUNCTION()
	virtual void OnRep_LocalTransaction(TArray<FBPActorGripInformation> OriginalArrayState) 
	{
		CheckTransactionBuffer();
	}

	UFUNCTION()
	virtual void OnRep_GrippedObjects(TArray<FBPActorGripInformation> OriginalArrayState) 
	{

		for (int i = GrippedObjects.Num() - 1; i >= 0; --i)
		{
			HandleGripReplication(GrippedObjects[i], OriginalArrayState.FindByKey(GrippedObjects[i].GripID));
		}
	}

	UFUNCTION()
	virtual void OnRep_LocallyGrippedObjects(TArray<FBPActorGripInformation> OriginalArrayState)
	{
		for (int i = LocallyGrippedObjects.Num() - 1; i >= 0; --i)
		{
			HandleGripReplication(LocallyGrippedObjects[i], OriginalArrayState.FindByKey(LocallyGrippedObjects[i].GripID));
		}
	}

	UPROPERTY(BlueprintReadWrite, Category = "GripMotionController")
	TArray<TObjectPtr<UPrimitiveComponent>> AdditionalLateUpdateComponents;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedControllerTransform, Category = "GripMotionController|Networking")
	FBPVRComponentPosRep ReplicatedControllerTransform;

	FVector LastUpdatesRelativePosition;
	FRotator LastUpdatesRelativeRotation;

	bool bLerpingPosition;
	bool bReppedOnce;

	UFUNCTION()
	virtual void OnRep_ReplicatedControllerTransform();

	void RunNetworkedSmoothing(float DeltaTime);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "GripMotionController|Networking", meta = (ClampMin = "0", UIMin = "0"))
	float ControllerNetUpdateRate;

public:
	void SetControllerNetUpdateRate(float NewControllerNetUpdateRate);
	inline float GetControllerNetUpdateRate() { return ControllerNetUpdateRate; };

	float ControllerNetUpdateCount;

	protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "GripMotionController|Networking")
		bool bSmoothReplicatedMotion;

	public:
		void SetSmoothReplicatedMotion(bool NewSmoothReplicatedMotion);
		inline bool GetSmoothReplicatedMotion() { return bSmoothReplicatedMotion; };

	UPROPERTY(EditAnywhere, Category = "GripMotionController|Networking|Smoothing", meta = (editcondition = "bSmoothReplicatedMotion"))
		bool bUseExponentialSmoothing = true;

	UPROPERTY(EditAnywhere, Category = "GripMotionController|Networking|Smoothing", meta = (editcondition = "bUseExponentialSmoothing"))
		float InterpolationSpeed = 25.0f;

	UPROPERTY(EditAnywhere, Category = "GripMotionController|Networking|Smoothing", meta = (editcondition = "bUseExponentialSmoothing"))
		float NetworkMaxSmoothUpdateDistance = 50.f;

	UPROPERTY(EditAnywhere, Category = "GripMotionController|Networking|Smoothing", meta = (editcondition = "bUseExponentialSmoothing"))
		float NetworkNoSmoothUpdateDistance = 100.f;

	protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "GripMotionController|Networking")
		bool bReplicateWithoutTracking;

	public:
		void SetReplicateWithoutTracking(bool NewReplicateWithoutTracking);
		inline bool GetReplicateWithoutTracking() { return bReplicateWithoutTracking; };

	UFUNCTION(Unreliable, Server, WithValidation)
	void Server_SendControllerTransform(FBPVRComponentPosRep NewTransform);

	typedef void (AVRBaseCharacter::*VRBaseCharTransformRPC_Pointer)(FBPVRComponentPosRep NewTransform);
	VRBaseCharTransformRPC_Pointer OverrideSendTransform;

	inline bool IsLocallyControlled() const
	{

		const AActor* MyOwner = GetOwner();
		return MyOwner->HasLocalNetOwner();

	}

	inline bool IsTravelingOrNullWorld() const
	{
		UWorld* myWorld = GetWorld();
		if (IsValid(myWorld))
		{
			return myWorld->IsInSeamlessTravel();
		}

		return true;
	}

	UFUNCTION(BlueprintPure, Category = "GripMotionController", meta = (DisplayName = "IsLocallyControlled"))
		bool BP_IsLocallyControlled();

	inline bool IsTornOff() const
	{
		const AActor* MyOwner = GetOwner();
		return MyOwner ? MyOwner->GetTearOff() : false;
	}

	inline bool IsServer() const
	{
		if (GEngine != nullptr && GWorld != nullptr)
		{
			return GEngine->GetNetMode(GWorld) < NM_Client;
		}

		return false;
	}

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool DropAndSocketObject(const FTransform_NetQuantize & RelativeTransformToParent, UObject * ObjectToDrop = nullptr, uint8 GripIDToDrop = 0, USceneComponent * SocketingParent = nullptr, FName OptionalSocketName = NAME_None, bool bWeldBodies = true);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool DropAndSocketGrip(const FBPActorGripInformation &GripToDrop, USceneComponent * SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize & RelativeTransformToParent, bool bWeldBodies = true);
		bool DropAndSocketGrip_Implementation(const FBPActorGripInformation& GripToDrop, USceneComponent* SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize& RelativeTransformToParent, bool bWeldBodies = true, bool bSkipServerNotify = false);

	UFUNCTION(Reliable, Server, WithValidation, Category = "GripMotionController")
		void Server_NotifyDropAndSocketGrip(uint8 GripID, USceneComponent * SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize & RelativeTransformToParent, bool bWeldBodies = true);

	UFUNCTION(Reliable, NetMulticast)
		void NotifyDropAndSocket(const FBPActorGripInformation &NewDrop, USceneComponent* SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize& RelativeTransformToParent, bool bWeldBodies = true);

	void DropAndSocket_Implementation(const FBPActorGripInformation &NewDrop);
	void Socket_Implementation(UObject * ObjectToSocket, bool bWasSimulating, USceneComponent * SocketingParent, FName OptionalSocketName, const FTransform_NetQuantize & RelativeTransformToParent, bool bWeldBodies = true);

	UPROPERTY()
	TArray<TObjectPtr<UObject>> ObjectsWaitingForSocketUpdate;

	UFUNCTION()
		void SetSocketTransform(UObject* ObjectToSocket,  const FTransform_NetQuantize RelativeTransformToParent);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool GripObject(
			UObject * ObjectToGrip,
			const FTransform &WorldOffset,
			bool bWorldOffsetIsRelative = false,
			FName OptionalSnapToSocketName = NAME_None,
			FName OptionalBoneToGripName = NAME_None,
			EGripCollisionType GripCollisionType = EGripCollisionType::InteractiveCollisionWithPhysics,
			EGripLateUpdateSettings GripLateUpdateSetting = EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping,
			EGripMovementReplicationSettings GripMovementReplicationSetting = EGripMovementReplicationSettings::ForceClientSideMovement,
			float GripStiffness = 2250.0f,
			float GripDamping = 140.0f, bool bIsSlotGrip = false);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool DropObject(
			UObject* ObjectToDrop = nullptr,
			uint8 GripIdToDrop = 0,
			bool bSimulate = false,
			FVector OptionalAngularVelocity = FVector::ZeroVector,
			FVector OptionalLinearVelocity = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool GripObjectByInterface(UObject* ObjectToGrip, const FTransform &WorldOffset, bool bWorldOffsetIsRelative = false, FName OptionalBoneToGripName = NAME_None, FName OptionalSnapToSocketName = NAME_None, bool bIsSlotGrip = false);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool DropObjectByInterface(UObject* ObjectToDrop = nullptr, uint8 GripIDToDrop = 0, FVector OptionalAngularVelocity = FVector::ZeroVector, FVector OptionalLinearVelocity = FVector::ZeroVector);

	bool DropObjectByInterface_Implementation(UObject* ObjectToDrop = nullptr, uint8 GripIDToDrop = 0, FVector OptionalAngularVelocity = FVector::ZeroVector, FVector OptionalLinearVelocity = FVector::ZeroVector, bool bSkipNotify = false);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool GripActor(
		AActor* ActorToGrip, 
		const FTransform &WorldOffset, 
		bool bWorldOffsetIsRelative = false, 
		FName OptionalSnapToSocketName = NAME_None,
		FName OptionalBoneToGripName = NAME_None,
		EGripCollisionType GripCollisionType = EGripCollisionType::InteractiveCollisionWithPhysics, 
		EGripLateUpdateSettings GripLateUpdateSetting = EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping, 
		EGripMovementReplicationSettings GripMovementReplicationSetting = EGripMovementReplicationSettings::ForceClientSideMovement,
		float GripStiffness = 1500.0f, 
		float GripDamping = 200.0f,
		bool bIsSlotGrip = false);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool DropActor(
		AActor* ActorToDrop, 
		bool bSimulate, 
		FVector OptionalAngularVelocity = FVector::ZeroVector, 
		FVector OptionalLinearVelocity = FVector::ZeroVector
		);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool GripComponent(
		UPrimitiveComponent* ComponentToGrip, 
		const FTransform &WorldOffset, bool bWorldOffsetIsRelative = false, 
		FName OptionalsnapToSocketName = NAME_None, 
		FName OptionalBoneToGripName = NAME_None,
		EGripCollisionType GripCollisionType = EGripCollisionType::InteractiveCollisionWithPhysics, 
		EGripLateUpdateSettings GripLateUpdateSetting = EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping,
		EGripMovementReplicationSettings GripMovementReplicationSetting = EGripMovementReplicationSettings::ForceClientSideMovement,
		float GripStiffness = 1500.0f, 
		float GripDamping = 200.0f,
		bool bIsSlotGrip = false);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool DropComponent(
		UPrimitiveComponent* ComponentToDrop,
		bool bSimulate, 
		FVector OptionalAngularVelocity = FVector::ZeroVector, 
		FVector OptionalLinearVelocity = FVector::ZeroVector
		);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool DropGrip(
		const FBPActorGripInformation &Grip, 
		bool bSimulate = false, 
		FVector OptionalAngularVelocity = FVector::ZeroVector, 
		FVector OptionalLinearVelocity = FVector::ZeroVector);

	bool DropGrip_Implementation(
		const FBPActorGripInformation& Grip,
		bool bSimulate = false,
		FVector OptionalAngularVelocity = FVector::ZeroVector,
		FVector OptionalLinearVelocity = FVector::ZeroVector,
		bool bSkipNotify = false);

	bool NotifyGrip(FBPActorGripInformation &NewGrip, bool bIsReInit = false);

	UFUNCTION(Reliable, NetMulticast)
	void NotifyDrop(const FBPActorGripInformation &NewDrop, bool bSimulate);

	void Drop_Implementation(const FBPActorGripInformation &NewDrop, bool bSimulate);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void GetGripByActor(FBPActorGripInformation &Grip, AActor * ActorToLookForGrip, EBPVRResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void GetGripByComponent(FBPActorGripInformation &Grip, UPrimitiveComponent * ComponentToLookForGrip, EBPVRResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
	void GetGripByObject(FBPActorGripInformation &Grip, UObject * ObjectToLookForGrip, EBPVRResultSwitch &Result);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
	void GetGripByID(FBPActorGripInformation &Grip, uint8 IDToLookForGrip, EBPVRResultSwitch &Result);

	FBPActorGripInformation * GetGripPtrByID(uint8 IDToLookForGrip);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		void GetPhysicsVelocities(const FBPActorGripInformation &Grip, FVector &CurAngularVelocity, FVector &CurLinearVelocity);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool GetPhysicsConstraintForce(const FBPActorGripInformation& Grip, FVector& AngularForce, FVector& LinearForce);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		static void GetGripMass(const FBPActorGripInformation& Grip, float& Mass);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		static FTransform GetGrippedObjectTransform(const FBPActorGripInformation& Grip);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void SetGripPaused(
			const FBPActorGripInformation &Grip,
			EBPVRResultSwitch &Result,
			bool bIsPaused = false,
			bool bNoConstraintWhenPaused = false
		);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void SetGripHybridLock(
			const FBPActorGripInformation& Grip,
			EBPVRResultSwitch& Result,
			bool bIsLocked = false
		);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		void SetPausedTransform(
			const FBPActorGripInformation &Grip,
			const FTransform & PausedTransform,
			bool bTeleport = false
		);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void SetGripCollisionType(
			const FBPActorGripInformation &Grip,
			EBPVRResultSwitch &Result,
			EGripCollisionType NewGripCollisionType = EGripCollisionType::InteractiveCollisionWithPhysics
			);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void SetGripLateUpdateSetting(
			const FBPActorGripInformation &Grip, 
			EBPVRResultSwitch &Result,
			EGripLateUpdateSettings NewGripLateUpdateSetting = EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping	
			);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void SetGripRelativeTransform(
			const FBPActorGripInformation &Grip,
			EBPVRResultSwitch &Result,
			const FTransform & NewRelativeTransform
			);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void SetGripAdditionTransform(
			const FBPActorGripInformation &Grip,
			EBPVRResultSwitch &Result,
			const FTransform & NewAdditionTransform, bool bMakeGripRelative = false
			);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void SetGripStiffnessAndDamping(
			const FBPActorGripInformation &Grip,
			EBPVRResultSwitch &Result,
			float NewStiffness, float NewDamping, bool bAlsoSetAngularValues = false, float OptionalAngularStiffness = 0.0f, float OptionalAngularDamping = 0.0f
		);

		UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void SetGripAdvancedGripSettings(
			const FBPActorGripInformation& Grip,
			EBPVRResultSwitch& Result,
			uint8 GripPriority = 0,
			bool bSetOwnerOnGrip = true,
			bool bDisallowLerping = false,
			bool bDisallowSettingPositionOnClientAuthDrop = false
		);

	UFUNCTION(BlueprintPure, Category = "GripMotionController", meta = (DisplayName = "CreateGripRelativeAdditionTransform"))
		FTransform CreateGripRelativeAdditionTransform_BP(
			const FBPActorGripInformation &GripToSample,
			const FTransform & AdditionTransform,
			bool bGripRelative = false
			);

	inline FTransform CreateGripRelativeAdditionTransform(
		const FBPActorGripInformation &GripToSample,
		const FTransform & AdditionTransform,
		bool bGripRelative = false
		);

	inline bool HasGripAuthority(const FBPActorGripInformation &Grip);

	inline bool HasGripAuthority(const UObject * ObjToCheck);

	UFUNCTION(BlueprintPure, Category = "GripMotionController", meta = (DisplayName = "HasGripAuthority"))
		bool BP_HasGripAuthority(const FBPActorGripInformation & Grip);

	UFUNCTION(BlueprintPure, Category = "GripMotionController", meta = (DisplayName = "HasGripAuthorityForObject"))
		bool BP_HasGripAuthorityForObject(const UObject* ObjToCheck);

	inline bool HasGripMovementAuthority(const FBPActorGripInformation & Grip);

	UFUNCTION(BlueprintPure, Category = "GripMotionController", meta = (DisplayName = "HasGripMovementAuthority"))
		bool BP_HasGripMovementAuthority(const FBPActorGripInformation & Grip);

	void TickGrip(float DeltaTime);

	void HandleGripArray(TArray<FBPActorGripInformation> &GrippedObjectsArray, const FTransform & ParentTransform, float DeltaTime, bool bReplicatedArray = false);

	bool GetGripWorldTransform(TArray<UVRGripScriptBase*>& GripScripts, float DeltaTime,FTransform & WorldTransform, const FTransform &ParentTransform, FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport, bool &bForceADrop);

	inline FTransform CalcControllerComponentToWorld(FRotator Orientation, FVector Position)
	{
		return this->CalcNewComponentToWorld(FTransform(Orientation, Position));
	}

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		FTransform ConvertToControllerRelativeTransform(const FTransform & InTransform);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		static FTransform ConvertToGripRelativeTransform(const FTransform& GrippedActorTransform, const FTransform & InTransform);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		bool GetIsObjectHeld(const UObject * ObjectToCheck);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		bool GetIsHeld(const AActor * ActorToCheck);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		bool GetIsComponentHeld(const UPrimitiveComponent * ComponentToCheck);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool GetIsSecondaryAttachment(const USceneComponent * ComponentToCheck, FBPActorGripInformation & Grip);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		bool HasGrippedObjects();

	UFUNCTION(BlueprintCallable, meta = (Keywords = "Grip", DisplayName = "GetFirstActiveGrip", ScriptName = "GetFirstActiveGrip"), Category = "GripMotionController")
		bool K2_GetFirstActiveGrip(FBPActorGripInformation& FirstActiveGrip);

	FBPActorGripInformation* GetFirstActiveGrip();

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		void GetAllGrips(TArray<FBPActorGripInformation> &GripArray);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	void GetGrippedActors(TArray<AActor*> &GrippedActorArray);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	void GetGrippedObjects(TArray<UObject*> &GrippedObjectsArray);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	void GetGrippedComponents(TArray<UPrimitiveComponent*> &GrippedComponentsArray);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	void PostTeleportMoveGrippedObjects();

	bool bIsPostTeleport;

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool TeleportMoveGrippedActor(AActor * GrippedActorToMove, bool bTeleportPhysicsGrips = true);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool TeleportMoveGrippedComponent(UPrimitiveComponent * ComponentToMove, bool bTeleportPhysicsGrips = true);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool TeleportMoveGrip(UPARAM(ref)FBPActorGripInformation &Grip, bool bTeleportPhysicsGrips = true, bool bIsForPostTeleport = false);
	bool TeleportMoveGrip_Impl(FBPActorGripInformation &Grip, bool bTeleportPhysicsGrips, bool bIsForPostTeleport, FTransform & OptionalTransform);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		void TeleportMoveGrips(bool bTeleportPhysicsGrips = true, bool bIsForPostTeleport = false);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool AddSecondaryAttachmentPoint(UObject * GrippedObjectToAddAttachment, USceneComponent * SecondaryPointComponent, const FTransform &OriginalTransform, bool bTransformIsAlreadyRelative = false, float LerpToTime = 0.25f, bool bIsSlotGrip = false, FName SecondarySlotName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool AddSecondaryAttachmentToGrip(const FBPActorGripInformation & GripToAddAttachment, USceneComponent * SecondaryPointComponent, const FTransform &OriginalTransform, bool bTransformIsAlreadyRelative = false, float LerpToTime = 0.25f, bool bIsSlotGrip = false, FName SecondarySlotName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool AddSecondaryAttachmentToGripByID(const uint8 GripID, USceneComponent* SecondaryPointComponent, const FTransform& OriginalTransform, bool bTransformIsAlreadyRelative = false, float LerpToTime = 0.25f, bool bIsSlotGrip = false, FName SecondarySlotName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
	bool RemoveSecondaryAttachmentPoint(UObject * GrippedObjectToRemoveAttachment, float LerpToTime = 0.25f);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool RemoveSecondaryAttachmentFromGrip(const FBPActorGripInformation & GripToRemoveAttachment, float LerpToTime = 0.25f);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController")
		bool RemoveSecondaryAttachmentFromGripByID(const uint8 GripID = 0, float LerpToTime = 0.25f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController")
		bool bIgnoreTrackingStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripMotionController")
	bool bUseWithoutTracking;

	bool CheckComponentWithSweep(UPrimitiveComponent * ComponentToCheck, FVector Move, FRotator newOrientation, bool bSkipSimulatingComponents);

	void OnGripMassUpdated(FBodyInstance* GripBodyInstance);
	bool SetUpPhysicsHandle(const FBPActorGripInformation &NewGrip, TArray<UVRGripScriptBase*> * GripScripts = nullptr);
	bool DestroyPhysicsHandle(const FBPActorGripInformation &Grip, bool bSkipUnregistering = false);
	void UpdatePhysicsHandleTransform(const FBPActorGripInformation &GrippedActor, const FTransform& NewTransform);
	bool SetGripConstraintStiffnessAndDamping(const FBPActorGripInformation *Grip, bool bUseHybridMultiplier = false);	
	bool GetPhysicsJointLength(const FBPActorGripInformation &GrippedActor, UPrimitiveComponent * rootComp, FVector & LocOut);

	TArray<FBPActorPhysicsHandleInformation> PhysicsGrips;
	FBPActorPhysicsHandleInformation * GetPhysicsGrip(const FBPActorGripInformation & GripInfo);
	FBPActorPhysicsHandleInformation * GetPhysicsGrip(const uint8 GripID);
	bool GetPhysicsGripIndex(const FBPActorGripInformation & GripInfo, int & index);
	FBPActorPhysicsHandleInformation * CreatePhysicsGrip(const FBPActorGripInformation & GripInfo);
	bool DestroyPhysicsHandle(FBPActorPhysicsHandleInformation * HandleInfo);
	bool PausePhysicsHandle(FBPActorPhysicsHandleInformation* HandleInfo);
	bool UnPausePhysicsHandle(FBPActorGripInformation& GripInfo, FBPActorPhysicsHandleInformation* HandleInfo);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController|Custom", meta = (DisplayName = "GetPhysicsHandleSettings"))
		bool GetPhysicsHandleSettings(UPARAM(ref)const FBPActorGripInformation & Grip, FBPAdvancedPhysicsHandleSettings& PhysicsHandleSettingsOut);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController|Custom", meta = (DisplayName = "SetPhysicsHandleSettings"))
		bool SetPhysicsHandleSettings(UPARAM(ref)const FBPActorGripInformation& Grip, UPARAM(ref) const FBPAdvancedPhysicsHandleSettings& PhysicsHandleSettingsIn);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController|Custom", meta = (DisplayName = "SetUpPhysicsHandle"))
		bool SetUpPhysicsHandle_BP(UPARAM(ref)const FBPActorGripInformation &Grip);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController|Custom", meta = (DisplayName = "DestroyPhysicsHandle"))
		bool DestroyPhysicsHandle_BP(UPARAM(ref)const FBPActorGripInformation &Grip);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController|Custom", meta = (DisplayName = "UpdatePhysicsHandle"))
		bool UpdatePhysicsHandle_BP(UPARAM(ref)const FBPActorGripInformation& Grip, bool bFullyRecreate = true);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController|Custom", meta = (DisplayName = "UpdatePhysicsHandleTransform"))
		void UpdatePhysicsHandleTransform_BP(UPARAM(ref)const FBPActorGripInformation &GrippedActor, UPARAM(ref)const FTransform& NewTransform);

	UFUNCTION(BlueprintCallable, Category = "GripMotionController|Custom", meta = (DisplayName = "GetGripDistance"))
		bool GetGripDistance_BP(UPARAM(ref)FBPActorGripInformation &Grip, FVector ExpectedLocation, float & CurrentDistance);

	bool GripPollControllerState_GameThread(FVector& Position, FRotator& Orientation, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale);
	bool GripPollControllerState_RenderThread(FVector& Position, FRotator& Orientation, float WorldToMetersScale);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		bool GripControllerIsTracked() const;

	UFUNCTION(BlueprintCallable, Category = "GripMotionController", meta = (ExpandEnumAsExecs = "Result"))
		void GetControllerDeviceID(FXRDeviceId & DeviceID, EBPVRResultSwitch &Result, bool bCheckOpenVROnly = false);

	UFUNCTION(BlueprintPure, Category = "GripMotionController")
		bool HasAuthority() const;

private:

	class FGripViewExtension : public FSceneViewExtensionBase
	{

	public:
		FGripViewExtension(const FAutoRegister& AutoRegister, UGripMotionControllerComponent* InMotionControllerComponent);

		virtual ~FGripViewExtension() = default;

		virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
		virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
		virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;

		virtual void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override {}
		virtual void PreRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override;

		virtual int32 GetPriority() const override { return -10; }

	private:
		friend class UGripMotionControllerComponent;

		UGripMotionControllerComponent* MotionControllerComponent;

		FExpandedLateUpdateManager LateUpdate;
	};
	TSharedPtr< FGripViewExtension, ESPMode::ThreadSafe > GripViewExtension;

};

FTransform inline UGripMotionControllerComponent::CreateGripRelativeAdditionTransform(
	const FBPActorGripInformation &GripToSample,
	const FTransform & AdditionTransform,
	bool bGripRelative
)
{

	FTransform FinalTransform;

	if (bGripRelative)
	{
		FinalTransform = FTransform(AdditionTransform.GetRotation(), GripToSample.RelativeTransform.GetRotation().RotateVector(AdditionTransform.GetLocation()), AdditionTransform.GetScale3D());
	}
	else
	{
		const FTransform PivotToWorld = FTransform(FQuat::Identity, GripToSample.RelativeTransform.GetLocation());
		const FTransform WorldToPivot = FTransform(FQuat::Identity, -GripToSample.RelativeTransform.GetLocation());

		FTransform RotationOffsetTransform(AdditionTransform.GetRotation(), FVector::ZeroVector);
		FinalTransform = FTransform(FQuat::Identity, AdditionTransform.GetLocation(), AdditionTransform.GetScale3D()) * WorldToPivot * RotationOffsetTransform * PivotToWorld;
	}

	return FinalTransform;
}

bool inline UGripMotionControllerComponent::HasGripAuthority(const FBPActorGripInformation &Grip)
{
	if (((Grip.GripMovementReplicationSetting != EGripMovementReplicationSettings::ClientSide_Authoritive &&
		Grip.GripMovementReplicationSetting != EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep) && IsServer()) ||
		((Grip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive ||
			Grip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep) && bHasAuthority))
	{
		return true;
	}

	return false;
}

bool inline UGripMotionControllerComponent::HasGripAuthority(const UObject * ObjToCheck)
{
	if (!ObjToCheck)
		return false;

	if (!ObjToCheck->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()) && IsServer())
		return true;

	EGripMovementReplicationSettings MovementRepType = IVRGripInterface::Execute_GripMovementReplicationType(const_cast<UObject*>(ObjToCheck));

	if (((MovementRepType != EGripMovementReplicationSettings::ClientSide_Authoritive &&
		MovementRepType != EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep) && IsServer()) ||
		((MovementRepType == EGripMovementReplicationSettings::ClientSide_Authoritive ||
			MovementRepType == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep) && bHasAuthority))
	{
		return true;
	}

	return false;
}

bool inline UGripMotionControllerComponent::HasGripMovementAuthority(const FBPActorGripInformation &Grip)
{
	if (IsServer())
	{
		return true;
	}
	else
	{
		if (Grip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ForceClientSideMovement ||
			Grip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive ||
			Grip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep)
		{
			return true;
		}
		else if (Grip.GripMovementReplicationSetting == EGripMovementReplicationSettings::ForceServerSideMovement)
		{
			return false;
		}

		check(Grip.GripMovementReplicationSetting != EGripMovementReplicationSettings::KeepOriginalMovement);
	}

	return false;
}
