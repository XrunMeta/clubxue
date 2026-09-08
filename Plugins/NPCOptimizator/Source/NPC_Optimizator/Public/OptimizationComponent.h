

#pragma once

#include "CoreMinimal.h"
#include "NPC_Optimizator_Types.h"
#include "Components/ActorComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/ThreadSafeBool.h"
#include "OptimizationComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOptimizationChangeWave, EOptimizationWave, NewWave);

UCLASS(BlueprintType, Blueprintable, Category = "NPC Optimizator", ClassGroup="NPCOptimizator", DisplayName="NPC Optimization Component", meta=(BlueprintSpawnableComponent),
	HideCategories=(ComponentReplication, Activation, Replication, Collision, Sockets, Tags))
class NPC_OPTIMIZATOR_API UOptimizationProxyComponent : public UActorComponent, public IOptimizationProxy
{
	GENERATED_BODY()
public:
	UOptimizationProxyComponent();
public:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:

	virtual EOptimizationBaseType GetOptimizationBasedType() const override;
	virtual AActor*               GetOwnerActor() override;
	virtual float                 GetMaxVisibleDistance() const override;
	virtual float                 GetDistToWave(EOptimizationWave Wave) const override;
	virtual bool                  IsIgnoreCameraSightOnSmallDistance() const override;
	virtual bool                  IsForceOptimizationWaveEnabled() const override;

public:
	int32                    GetOptimizationHandle() const { return OptimizationHandle; }
	EOptimizationWave        GetOptimizationWave() const;

	void                     SetOptimizationHandle(int32 Handle) { OptimizationHandle = Handle; }
	void                     SetOptimizationWave(EOptimizationWave Wave);

	void                     SetIsNeedToBeOptimized(bool NeedToOptimize);
	bool                     IsNeedToBeOptimized() const;

	void                     OptimizeComponent();
public:

	UFUNCTION(BlueprintCallable, Category = "NPC Optimizator")
	void                     DisableAllOptimizations();

	UFUNCTION(BlueprintCallable, Category = "NPC Optimizator")
	void                     EnableOptimizations();

	UFUNCTION(BlueprintCallable, Category = "NPC Optimizator")
	void                     EnableForceOptimizationWave(EOptimizationWave Wave, bool Enable);
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName="Distance to first wave")
	float DistanceToFirstOptimization = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName="Distance to second wave")
	float DistanceToSecondOptimization = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName="Distance to third wave")
	float DistanceToThirdOptimization = 3500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName="Max visible distance")
	float MaxVisibleDistance = 8000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName="Optimization type")
	EOptimizationBaseType OptimizationBasedOn = EOptimizationBaseType::PlayerPawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName="Use optimization by tag")
	bool UseOptimizationByTag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName="Optimization tag", meta=(EditCondition=UseOptimizationByTag))
	FName OptimizationTag = "NeedOptimization";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName="Ignore optimization tag")
	FName IgnoreOptimizationTag = "IgnoreOptimization";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Disable movement when invisible")
	bool DisableMovementWhenNotVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Ignore camera sight")
	bool IgnoreCameraSight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Ignore camera sight in 'no optimization' wave")
	bool IgnoreCameraSightOnSmallDistance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Disable skeletal mesh tick when invisible")
	bool DisableSkeletalMeshTickWhenNotVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Disable groom tick when invisible")
	bool DisableGroomTickWhenNotVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Hide all attached static meshes")
	bool HideAllStaticMeshes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Disable optimization on listen server")
	bool bDisableOptimizationOnListenServer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Disable optimization on dedicated server")
	bool bDisableOptimizationOnDedicatedServer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Disable optimization in simulation mode")
	bool bDisableOptimizationInSimulationMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Optimize AI controller")
	bool bOptimizeAIController = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", DisplayName = "Optimize path follow component")
	bool bOptimizePathFollowComponent = true; 
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Always check floor")
	bool FirstWave_AlwaysCheckFloor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Enable physics interaction")
	bool FirstWave_EnablePhysicsInteraction = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Max simulation time step")
	float FirstWave_MaxSimulationTimeStep = 0.025f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Max simulation iterations")
	int32 FirstWave_MaxSimulationIterations = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Run physics with no controller")
	bool FirstWave_RunPhysicsWithNoController = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Movement mode")
	TEnumAsByte<EMovementMode> FirstWave_MovementMode = EMovementMode::MOVE_NavWalking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Sweep while nav walking")
	bool FirstWave_SweepWhileNavWalking = true;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Fixed movement tick")
	float FirstWave_OptimizatedMovementTick = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Min movement random tick")
	float FirstWave_OptimizatedMovementTickMin = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Max movement random tick")
	float FirstWave_OptimizatedMovementTickMax = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Movement", DisplayName = "Use random movement tick")
	bool FirstWave_UseRandomOptimizationTickForMovement = true;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Hide shadows")
	bool FirstWave_HideShadows = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Groom", DisplayName = "Hide shadows")
	bool FirstWaveGroom_HideShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Groom", DisplayName = "Hide groom")
	bool FirstWaveGroom_Hide = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Disable mesh collision")
	bool FirstWave_DisableMeshCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Groom", DisplayName = "Disable mesh collision")
	bool FirstWaveGroom_DisableMeshCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Need hide static meshes")
	bool FirstWave_NeedHideStaticMeshes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Use URO")
	bool FirstWave_UseUpdateRateOptimizations = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Use per bone motion blur")
	bool FirstWave_UsePerBoneMotionBlur = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Disable cloth simulation")
	bool FirstWave_DisableClothSimulation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Disable morph target")
	bool FirstWave_DisableMorphTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Skip kinematic update when interpolating")
	bool FirstWave_SkipKinematicUpdateWhenInterpolating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Skip bounds update when interpolating")
	bool FirstWave_SkipBoundsUpdateWhenInterpolating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Allow rigid body anim node")
	bool FirstWave_AllowRigidBodyAnimNode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Generate overlap events")
	bool FirstWave_GenerateOverlapEvents = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Groom", DisplayName = "Generate overlap events")
	bool FirstWaveGroom_GenerateOverlapEvents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|First wave|Skeletal meshes", DisplayName = "Visibility Based Anim Tick Option")
	EVisibilityBasedAnimTickOption FirstWave_VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Always check floor")
	bool SecondWave_AlwaysCheckFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Enable physics interaction")
	bool SecondWave_EnablePhysicsInteraction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Max simulation time step")
	float SecondWave_MaxSimulationTimeStep = 0.035f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Max simulation iterations")
	int32 SecondWave_MaxSimulationIterations = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Run physics with no controller")
	bool SecondWave_RunPhysicsWithNoController = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Movement mode")
	TEnumAsByte<EMovementMode> SecondWave_MovementMode = EMovementMode::MOVE_NavWalking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Sweep while nav walking")
	bool SecondWave_SweepWhileNavWalking = false;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Fixed movement tick")
	float SecondWave_OptimizatedMovementTick = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Min movement random tick")
	float SecondWave_OptimizatedMovementTickMin = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Max movement random tick")
	float SecondWave_OptimizatedMovementTickMax = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Movement", DisplayName = "Use random movement tick")
	bool SecondWave_UseRandomOptimizationTickForMovement = true;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Hide shadows")
	bool SecondWave_HideShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Groom", DisplayName = "Hide shadows")
	bool SecondWaveGroom_HideShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Groom", DisplayName = "Hide groom")
	bool SecondWaveGroom_Hide = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Disable mesh collision")
	bool SecondWave_DisableMeshCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Groom", DisplayName = "Disable mesh collision")
	bool SecondWaveGroom_DisableMeshCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Need hide static meshes")
	bool SecondWave_NeedHideStaticMeshes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Use URO")
	bool SecondWave_UseUpdateRateOptimizations = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Use per bone motion blur")
	bool SecondWave_UsePerBoneMotionBlur = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Disable cloth simulation")
	bool SecondWave_DisableClothSimulation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Disable morph target")
	bool SecondWave_DisableMorphTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Skip kinematic update when interpolating")
	bool SecondWave_SkipKinematicUpdateWhenInterpolating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Skip bounds update when interpolating")
	bool SecondWave_SkipBoundsUpdateWhenInterpolating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Allow rigid body anim node")
	bool SecondWave_AllowRigidBodyAnimNode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Generate overlap events")
	bool SecondWave_GenerateOverlapEvents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Groom", DisplayName = "Generate overlap events")
	bool SecondWaveGroom_GenerateOverlapEvents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Second wave|Skeletal meshes", DisplayName = "Visibility Based Anim Tick Option")
	EVisibilityBasedAnimTickOption SecondWave_VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Always check floor")
	bool ThirdWave_AlwaysCheckFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Enable physics interaction")
	bool ThirdWave_EnablePhysicsInteraction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Max simulation time step")
	float ThirdWave_MaxSimulationTimeStep = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Max simulation iterations")
	int32 ThirdWave_MaxSimulationIterations = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Run physics with no controller")
	bool ThirdWave_RunPhysicsWithNoController = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Movement mode")
	TEnumAsByte<EMovementMode> ThirdWave_MovementMode = EMovementMode::MOVE_NavWalking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Sweep while nav walking")
	bool ThirdWave_SweepWhileNavWalking = false;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Fixed movement tick")
	float ThirdWave_OptimizatedMovementTick = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Min movement random tick")
	float ThirdWave_OptimizatedMovementTickMin = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Max movement random tick")
	float ThirdWave_OptimizatedMovementTickMax = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Movement", DisplayName = "Use random movement tick")
	bool ThirdWave_UseRandomOptimizationTickForMovement = true;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Hide shadows")
	bool ThirdWave_HideShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Groom", DisplayName = "Hide shadows")
	bool ThirdWaveGroom_HideShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Groom", DisplayName = "Hide groom")
	bool ThirdWaveGroom_Hide = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Disable mesh collision")
	bool ThirdWave_DisableMeshCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Groom", DisplayName = "Disable mesh collision")
	bool ThirdWaveGroom_DisableMeshCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Need hide static meshes")
	bool ThirdWave_NeedHideStaticMeshes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Use URO")
	bool ThirdWave_UseUpdateRateOptimizations = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Use per bone motion blur")
	bool ThirdWave_UsePerBoneMotionBlur = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Disable cloth simulation")
	bool ThirdWave_DisableClothSimulation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Disable morph target")
	bool ThirdWave_DisableMorphTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Skip kinematic update when interpolating")
	bool ThirdWave_SkipKinematicUpdateWhenInterpolating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Skip bounds update when interpolating")
	bool ThirdWave_SkipBoundsUpdateWhenInterpolating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Allow rigid body anim node")
	bool ThirdWave_AllowRigidBodyAnimNode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Generate overlap events")
	bool ThirdWave_GenerateOverlapEvents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Groom", DisplayName = "Generate overlap events")
	bool ThirdWaveGroom_GenerateOverlapEvents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Third wave|Skeletal meshes", DisplayName = "Visibility Based Anim Tick Option")
	EVisibilityBasedAnimTickOption ThirdWave_VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Always check floor")
	bool Invisible_AlwaysCheckFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Enable physics interaction")
	bool Invisible_EnablePhysicsInteraction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Max simulation time step")
	float Invisible_MaxSimulationTimeStep = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Max simulation iterations")
	int32 Invisible_MaxSimulationIterations = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Run physics with no controller")
	bool Invisible_RunPhysicsWithNoController = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Movement mode")
	TEnumAsByte<EMovementMode> Invisible_MovementMode = EMovementMode::MOVE_NavWalking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Sweep while nav walking")
	bool Invisible_SweepWhileNavWalking = false;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Fixed movement tick")
	float Invisible_OptimizatedMovementTick = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Min movement random tick")
	float Invisible_OptimizatedMovementTickMin = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Max movement random tick")
	float Invisible_OptimizatedMovementTickMax = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Movement", DisplayName = "Use random movement tick")
	bool Invisible_UseRandomOptimizationTickForMovement = true;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Hide skeletal mesh")
	bool Invisible_HideSkeletalMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Groom", DisplayName = "Hide groom")
	bool InvisibleGroom_Hide = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Hide shadows")
	bool Invisible_HideShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Groom", DisplayName = "Hide shadows")
	bool InvisibleGroom_HideShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Disable mesh collision")
	bool Invisible_DisableMeshCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Groom", DisplayName = "Disable mesh collision")
	bool InvisibleGroom_DisableMeshCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Need hide static meshes")
	bool Invisible_NeedHideStaticMeshes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Use URO")
	bool Invisible_UseUpdateRateOptimizations = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Use per bone motion blur")
	bool Invisible_UsePerBoneMotionBlur = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Disable cloth simulation")
	bool Invisible_DisableClothSimulation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Disable morph target")
	bool Invisible_DisableMorphTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Skip kinematic update when interpolating")
	bool Invisible_SkipKinematicUpdateWhenInterpolating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Skip bounds update when interpolating")
	bool Invisible_SkipBoundsUpdateWhenInterpolating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Visibility Based Anim Tick Option")
	EVisibilityBasedAnimTickOption Invisible_VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Allow rigid body anim node")
	bool Invisible_AllowRigidBodyAnimNode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Skeletal meshes", DisplayName = "Generate overlap events")
	bool Invisible_GenerateOverlapEvents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Invisible|Groom", DisplayName = "Generate overlap events")
	bool InvisibleGroom_GenerateOverlapEvents = false;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Debug draw")
	bool NoOptimization_DrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Debug draw")
	bool FirstWave_DrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Debug draw")
	bool SecondWave_DrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Debug draw")
	bool ThirdWave_DrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|Debug draw")
	bool Invisible_DrawDebug = false;
public:

	UPROPERTY(BlueprintAssignable, Category = "Optimization")
	FOptimizationChangeWave  OnChangeWave;
protected:
	void OptimizeSkeletalMeshes(EOptimizationWave Wave);
	void OptimizeMovement(EOptimizationWave Wave) const;
	void OptimizeGrooms(EOptimizationWave Wave);
public:
	FSDefaultMovementSettings     MovementSettings;
private:
	mutable FCriticalSection      LockCS;
	int32                         OptimizationHandle = INDEX_NONE;
	EOptimizationWave             OptimizationWave = EOptimizationWave::NoOptimization;
	EOptimizationWave             PreviousWave = EOptimizationWave::NoOptimization;
	FThreadSafeBool               bIsNeedToBeOptimized = false;
	bool                          bIsForceOptimizationWaveEnabled = false;
	TArray<FSOptimizedComponent>  ComponentsForOptimization;
};