

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "VRInteractibleFunctionLibrary.h"
#include "VRButtonComponent.generated.h"

UENUM(Blueprintable)
enum class EVRButtonType : uint8
{
	Btn_Press,
	Btn_Toggle_Return,
	Btn_Toggle_Stay
};

UENUM(Blueprintable)
enum class EVRStateChangeAuthorityType : uint8
{

	CanChangeState_All,

	CanChangeState_Server,

	CanChangeState_Owner
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVRButtonStateChangedSignature, bool, ButtonState, AActor *, InteractingActor, UPrimitiveComponent *, InteractingComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVRButtonStartedInteractionSignature, AActor *, InteractingActor, UPrimitiveComponent *, InteractingComponent);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRButtonComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UVRButtonComponent(const FObjectInitializer& ObjectInitializer);

	~UVRButtonComponent();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "VRButtonComponent")
		bool IsButtonInUse();

	UFUNCTION(BlueprintCallable, Category = "VRButtonComponent")
		void ResetInitialButtonLocation();

	UFUNCTION(BlueprintCallable, Category = "VRButtonComponent")
		void SetButtonState(bool bNewButtonState, bool bCallButtonChangedEvent = true, bool bSnapIntoPosition = false);

	UFUNCTION(BlueprintCallable, Category = "VRButtonComponent")
		void SetButtonToRestingPosition(bool bLerpToPosition = false);

	UPROPERTY(BlueprintAssignable, Category = "VRButtonComponent")
		FVRButtonStateChangedSignature OnButtonStateChanged;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Button State Changed"))
		void ReceiveButtonStateChanged(bool bCurButtonState, AActor * LastInteractingActor, UPrimitiveComponent * InteractingComponent);

	UPROPERTY(BlueprintAssignable, Category = "VRButtonComponent")
		FVRButtonStartedInteractionSignature OnButtonBeginInteraction;

	UPROPERTY(BlueprintAssignable, Category = "VRButtonComponent")
		FVRButtonStartedInteractionSignature OnButtonEndInteraction;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Button Started Interaction"))
		void ReceiveButtonBeginInteraction(AActor * InteractingActor, UPrimitiveComponent * InteractingComponent);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Button Ended Interaction"))
		void ReceiveButtonEndInteraction(AActor * LastInteractingActor, UPrimitiveComponent * LastInteractingComponent);

	UPROPERTY(BlueprintReadOnly, Category = "VRButtonComponent")
		TObjectPtr<UPrimitiveComponent> LocalInteractingComponent;

	UPROPERTY(BlueprintReadOnly, Category = "VRButtonComponent")
		TObjectPtr<AActor> LocalLastInteractingActor;

	UPROPERTY(BlueprintReadOnly, Category = "VRButtonComponent")
		TObjectPtr<UPrimitiveComponent> LocalLastInteractingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRButtonComponent")
	bool bIsEnabled;

protected:

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Replicated, Category = "VRButtonComponent")
	bool bButtonState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "VRButtonComponent|Replication")
		EVRStateChangeAuthorityType StateChangeAuthorityType;
public:
	bool GetButtonState() { return bButtonState; }
	void SetButtonState(bool bNewButtonState);
	EVRStateChangeAuthorityType GetStateChangeAuthorityType() { return StateChangeAuthorityType; }
	void SetStateChangeAuthorityType(EVRStateChangeAuthorityType NewStateChangeAuthorityType);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRButtonComponent")
	float DepressSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRButtonComponent")
	float DepressDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRButtonComponent")
	EVRButtonType ButtonType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRButtonComponent")
	EVRInteractibleAxis ButtonAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRButtonComponent")
	float ButtonEngageDepth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRButtonComponent")
	float MinTimeBetweenEngaging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRButtonComponent")
		bool bSkipOverlapFiltering;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRButtonComponent")
	bool IsValidOverlap(UPrimitiveComponent * OverlapComponent);

	void SetLastInteractingActor();

	virtual FVector GetTargetRelativeLocation();

	protected:

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface|Replication")
		bool bReplicateMovement;
	public:
		bool GetReplicateMovement() { return bReplicateMovement; }
		void SetReplicateMovement(bool bNewReplicateMovement);

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

	virtual void OnRegister() override;

	protected:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InitialRelativeTransform, Category = "VRButtonComponent")
		FTransform_NetQuantize InitialRelativeTransform;
	public:

		FTransform GetInitialRelativeTransform() { return InitialRelativeTransform; }

	UFUNCTION()
		virtual void OnRep_InitialRelativeTransform()
	{
		SetButtonToRestingPosition();
	}

protected:

	FVector InitialLocation;
	bool bToggledThisTouch;
	FVector InitialComponentLoc;
	float LastToggleTime;

	float GetAxisValue(FVector CheckLocation);

	FVector SetAxisValue(float SetValue);

};