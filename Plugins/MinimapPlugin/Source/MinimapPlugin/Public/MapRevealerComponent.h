

#pragma once

#include "Components/BoxComponent.h"
#include "MapEnums.h"
#include "MapRevealerComponent.generated.h"

class AMapFog;
class UCanvas;

UCLASS(ClassGroup=(MinimapPlugin), meta=(BlueprintSpawnableComponent))
class MINIMAPPLUGIN_API UMapRevealerComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	UMapRevealerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	EMapFogRevealMode GetRevealMode() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetRevealMode(const EMapFogRevealMode NewRevealMode);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	void GetRevealExtent(float& RevealExtentX, float& RevealExtentY) const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetRevealExtent(const float NewRevealExtentX, const float NewRevealExtentY);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetRevealDropOffDistance() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetRevealDropOffDistance(const float NewRevealDropOffDistance);

	virtual void UpdateMapFog(AMapFog* MapFog, UCanvas* Canvas);

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	UMaterialInterface* RevealMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	EMapFogRevealMode RevealMode = EMapFogRevealMode::Temporary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	float RevealDropOffDistance = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap", EditFixedSize)
	bool bTempEngineBugWorkaround = true;

private:
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* RevealMaterialInstance;

};
