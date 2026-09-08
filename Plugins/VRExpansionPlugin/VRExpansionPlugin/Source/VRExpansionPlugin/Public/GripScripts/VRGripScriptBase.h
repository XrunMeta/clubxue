

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"
#include "VRBPDatatypes.h"
#include "Tickable.h"

#include "VRGripScriptBase.generated.h"

class UGripMotionControllerComponent;
class UVRGripInterface;
class UPrimitiveComponent;
class AActor;

UENUM(Blueprintable)
enum class EGSTransformOverrideType : uint8
{

	None,

	OverridesWorldTransform,

	ModifiesWorldTransform
};

UCLASS(NotBlueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, Abstract, ClassGroup = (VRExpansionPlugin), HideCategories = DefaultSettings)
class VREXPANSIONPLUGIN_API UVRGripScriptBase : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:

	UVRGripScriptBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "VRGripScript|Functions", meta = (WorldContext = "WorldContextObject", bIgnoreSelf = "true", DisplayName = "GetGripScriptByClass", ExpandEnumAsExecs = "Result"))
		static UVRGripScriptBase* GetGripScriptByClass(UObject* WorldContextObject, TSubclassOf<UVRGripScriptBase> GripScriptClass, EBPVRResultSwitch& Result);

	bool IsSupportedForNetworking() const override
	{
		return true;

	}

	bool IsScriptActive();

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GSSettings")
	bool bIsActive;

private:

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "GSSettings|Replication", meta = (AllowPrivateAccess = "true"))
	bool bReplicates = false;
public:

	UFUNCTION(BlueprintCallable, Category = "Components")
		void SetIsReplicated(bool ShouldReplicate);

	UFUNCTION(BlueprintCallable, Category = "Components")
	bool GetIsReplicated() const
	{
		return bReplicates;
	}

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GSSettings|Replication")
	TEnumAsByte<ELifetimeCondition> ReplicationCondition = ELifetimeCondition::COND_None;

	EGSTransformOverrideType GetWorldTransformOverrideType();

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GSSettings")
	EGSTransformOverrideType WorldTransformOverrideType;

	FORCEINLINE bool Wants_DenyAutoDrop()
	{
		return bDenyAutoDrop;
	}

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GSSettings")
		bool bDenyAutoDrop;

	FORCEINLINE bool Wants_ToForceDrop()
	{
		return bForceDrop;
	}

	UPROPERTY(BlueprintReadWrite, Category = "GSSettings")
		bool bForceDrop;

	UFUNCTION(BlueprintCallable, Category = "VRGripScript")
	void ForceGripToDrop()
	{
		bForceDrop = true;
	}

	FORCEINLINE bool Wants_DenyLateUpdates()
	{
		return bDenyLateUpdates;
	}

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GSSettings")
		bool bDenyLateUpdates;

	FORCEINLINE bool InjectPrePhysicsHandle()
	{
		return bInjectPrePhysicsHandle;
	}

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GSSettings")
		bool bInjectPrePhysicsHandle;

	virtual void HandlePrePhysicsHandle(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation &GripInfo, FBPActorPhysicsHandleInformation * HandleInfo, FTransform & KinPose);

	FORCEINLINE bool InjectPostPhysicsHandle()
	{
		return bInjectPostPhysicsHandle;
	}

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GSSettings")
		bool bInjectPostPhysicsHandle;

	virtual void HandlePostPhysicsHandle(UGripMotionControllerComponent* GrippingController, FBPActorPhysicsHandleInformation * HandleInfo);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripScript")
	bool Wants_DenyTeleport(UGripMotionControllerComponent * Controller);
	virtual bool Wants_DenyTeleport_Implementation(UGripMotionControllerComponent* Controller);

#if UE_WITH_IRIS

	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif 

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;

	virtual bool CallRemoteFunction(UFunction * Function, void * Parms, FOutParmRec * OutParms, FFrame * Stack) override;
	virtual int32 GetFunctionCallspace(UFunction * Function, FFrame * Stack) override;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "TickSettings")
		bool bCanEverTick;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TickSettings")
		bool bAllowTicking;

	UFUNCTION(BlueprintCallable, Category = "TickSettings")
		void SetTickEnabled(bool bTickEnabled);

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual bool IsTickableInEditor() const;
	virtual bool IsTickableWhenPaused() const override;
	virtual ETickableTickType GetTickableTickType() const;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintPure, Category = "VRGripScript")
		FTransform GetGripTransform(const FBPActorGripInformation &Grip, const FTransform & ParentTransform);

	UFUNCTION(BlueprintPure, Category = "VRGripScript")
		FTransform GetParentTransform(bool bGetWorldTransform = true, FName BoneName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "VRGripScript")
		USceneComponent* GetParentSceneComp();

	FBodyInstance * GetParentBodyInstance(FName OptionalBoneName = NAME_None);

	UFUNCTION(BlueprintPure, Category = "VRGripScript")
		UObject * GetParent();

	UFUNCTION(BlueprintPure, Category = "VRGripScript")
		AActor * GetOwner();

	UFUNCTION(BlueprintPure, Category = "VRGripScript")
		bool HasAuthority();

	UFUNCTION(BlueprintPure, Category = "VRGripScript")
		bool IsServer();

	virtual void BeginDestroy();
	void EndPlay(const EEndPlayReason::Type EndPlayReason);

	UFUNCTION(BlueprintNativeEvent, Category = "VRGripScript")
		void OnEndPlay(const EEndPlayReason::Type EndPlayReason);
	virtual void OnEndPlay_Implementation(const EEndPlayReason::Type EndPlayReason);

	void BeginPlay(UObject * CallingOwner);
	bool bAlreadyNotifiedPlay = false;
	virtual void PostInitProperties() override;

	UFUNCTION(BlueprintNativeEvent, Category = "VRGripScript")
		void OnBeginPlay(UObject * CallingOwner);
		virtual void OnBeginPlay_Implementation(UObject * CallingOwner);

	UFUNCTION(BlueprintNativeEvent, Category = "VRGripScript")
		bool GetWorldTransform(UGripMotionControllerComponent * GrippingController, float DeltaTime, UPARAM(ref) FTransform & WorldTransform, const FTransform &ParentTransform, UPARAM(ref) FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport);
		virtual bool GetWorldTransform_Implementation(UGripMotionControllerComponent * OwningController, float DeltaTime, FTransform & WorldTransform, const FTransform &ParentTransform, FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport);

	UFUNCTION(BlueprintNativeEvent, Category = "VRGripScript")
		void OnGrip(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation);
		virtual void OnGrip_Implementation(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation);

	UFUNCTION(BlueprintNativeEvent, Category = "VRGripScript")
	void OnGripRelease(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed = false);
	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed = false);

	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
	void OnSecondaryGrip(UGripMotionControllerComponent * Controller, USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation);
	virtual void OnSecondaryGrip_Implementation(UGripMotionControllerComponent * Controller, USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation);

	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
	void OnSecondaryGripRelease(UGripMotionControllerComponent * Controller, USceneComponent * ReleasingSecondaryGripComponent, const FBPActorGripInformation & GripInformation);
	virtual void OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent * Controller, USceneComponent * ReleasingSecondaryGripComponent, const FBPActorGripInformation & GripInformation);

	virtual bool CallCorrect_GetWorldTransform(UGripMotionControllerComponent * OwningController, float DeltaTime, FTransform & WorldTransform, const FTransform &ParentTransform, FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport)
	{
		return GetWorldTransform_Implementation(OwningController, DeltaTime, WorldTransform, ParentTransform, Grip, actor, root, bRootHasInterface, bActorHasInterface, bIsForTeleport);
	}
};

UCLASS(Blueprintable, Abstract, ClassGroup = (VRExpansionPlugin), ShowCategories = DefaultSettings)
class VREXPANSIONPLUGIN_API UVRGripScriptBaseBP : public UVRGripScriptBase
{
	GENERATED_BODY()
public:

	virtual bool CallCorrect_GetWorldTransform(UGripMotionControllerComponent * OwningController, float DeltaTime, FTransform & WorldTransform, const FTransform &ParentTransform, FBPActorGripInformation &Grip, AActor * actor, UPrimitiveComponent * root, bool bRootHasInterface, bool bActorHasInterface, bool bIsForTeleport) override
	{
		return GetWorldTransform(OwningController, DeltaTime, WorldTransform, ParentTransform, Grip, actor, root, bRootHasInterface, bActorHasInterface, bIsForTeleport);
	}

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Tick"))
		void ReceiveTick(float DeltaSeconds);
};