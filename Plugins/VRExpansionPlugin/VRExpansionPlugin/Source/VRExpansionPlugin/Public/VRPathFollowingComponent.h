

#pragma once
#include "CoreMinimal.h"
#include "VRBPDatatypes.h"
#include "VRRootComponent.h"
#include "VRBaseCharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AbstractNavData.h"
#include "Runtime/Launch/Resources/Version.h"

#include "VRPathFollowingComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPathFollowingVR, Warning, All);

UCLASS()
class VREXPANSIONPLUGIN_API UVRPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()

public:

	virtual void SetNavMovementInterface(INavMovementInterface* NavMoveInterface) override;

	virtual void GetDebugStringTokens(TArray<FString>& Tokens, TArray<EPathFollowingDebugTokens::Type>& Flags) const override;

};