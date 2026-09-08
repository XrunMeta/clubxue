

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GrippableCharacter.generated.h"

class UGrippableSkeletalMeshComponent;

UCLASS()
class VREXPANSIONPLUGIN_API AGrippableCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGrippableCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(Category = GrippableCharacter, VisibleAnywhere, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UGrippableSkeletalMeshComponent> GrippableMeshReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		FName ViewOriginationSocket;

	virtual void GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const override;

};