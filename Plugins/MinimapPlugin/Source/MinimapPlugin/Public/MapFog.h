

#pragma once

#include "MapAreaBase.h"
#include "MapEnums.h"
#include "MapFog.generated.h"

class UMapRevealerComponent;
class APostProcessVolume;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapFogMaterialChangedSignature, AMapFog*, MapFog);

UCLASS()
class MINIMAPPLUGIN_API AMapFog : public AMapAreaBase
{
	GENERATED_BODY()

public:	
	AMapFog();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool GetFogAtLocation(const FVector& WorldLocation, const bool bRequireCurrentlyRevealing, float& RevealFactor);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UTextureRenderTarget2D* GetDestinationFogRenderTarget() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UTextureRenderTarget2D* GetSourceFogRenderTarget() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetWorldToPixelRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetFogMaterialForUMG(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UMaterialInterface* GetFogMaterialForUMG();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetFogMaterialForCanvas(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UMaterialInstanceDynamic* GetFogMaterialInstanceForCanvas(UMapRendererComponent* Renderer);

private:
	void InitializeWorldFog();

	UFUNCTION()
	void OnMapRevealerRegistered(UMapRevealerComponent* MapRevealer);
	UFUNCTION()
	void OnMapRevealerUnregistered(UMapRevealerComponent* MapRevealer);

public:

	UPROPERTY(BlueprintAssignable, Category = "Minimap Background")
	FMapFogMaterialChangedSignature OnMapFogMaterialChanged;

protected:

	UPROPERTY(EditAnywhere, Category = "Minimap Fog")
	int32 FogRenderTargetSize = 256;

	UPROPERTY(EditAnywhere, Category = "Minimap Fog")
	UMaterialInterface* FogMaterial_UMG = nullptr;

	UPROPERTY(EditAnywhere, Category = "Minimap Fog")
	UMaterialInterface* FogMaterial_Canvas = nullptr;

	UPROPERTY(EditAnywhere, Category = "Minimap Fog", BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimapOpacityHidden = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Minimap Fog", BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimapOpacityExplored = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Minimap Fog", BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimapOpacityRevealing = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Minimap Fog")
	UMaterialInterface* FogCombineMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Gameplay Fog")
	float FogCacheLifetime = 0.05f;

	UPROPERTY(EditAnywhere, Category = "World Fog")
	bool bEnableWorldFog = true;

	UPROPERTY(EditAnywhere, Category = "World Fog", meta = (EditCondition = "bEnableWorldFog"))
	UMaterialInterface* FogPostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "World Fog", BlueprintReadOnly, meta = (EditCondition = "bEnableWorldFog", ClampMin = "0.0", ClampMax = "1.0"))
	float WorldOpacityHidden = 0.5f;

	UPROPERTY(EditAnywhere, Category = "World Fog", BlueprintReadOnly, meta = (EditCondition = "bEnableWorldFog", ClampMin = "0.0", ClampMax = "1.0"))
	float WorldOpacityExplored = 0.8f;

	UPROPERTY(EditAnywhere, Category = "World Fog", BlueprintReadOnly, meta = (EditCondition = "bEnableWorldFog", ClampMin = "0.0", ClampMax = "1.0"))
	float WorldOpacityRevealing = 1.0f;

	UPROPERTY(EditAnywhere, Category = "World Fog", meta = (EditCondition = "bEnableWorldFog"))
	APostProcessVolume* PostProcessVolume = nullptr;

	UPROPERTY(EditAnywhere, Category = "World Fog", meta = (EditCondition = "bEnableWorldFog"))
	EFogPostProcessVolumeOption AutoLocatePostProcessVolume = EFogPostProcessVolumeOption::AutoLocateOrCreate;

private:

	UPROPERTY(Transient)
	UTextureRenderTarget2D* PermanentRevealRT_A = nullptr;
	UPROPERTY(Transient)
	UTextureRenderTarget2D* PermanentRevealRT_B = nullptr;
	UPROPERTY(Transient)
	UTextureRenderTarget2D* RevealRT_Staging = nullptr;
	bool bUseBufferA = true;

	UPROPERTY(Transient)
	TMap<UMapRendererComponent*, UMaterialInstanceDynamic*> MaterialInstances;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* FogCombineMatInst = nullptr;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* FogPostProcessMatInst = nullptr;

	float AnimStartTime = 0.0f;

	bool bPermanentRT_Read = false;
	bool bStagingRT_Read = false;
	float PermanentRT_LastReadTime = 0.0f;
	float StagingRT_LastReadTime = 0.0f;
	TArray<FLinearColor> PermanentRT_Buffer;
	TArray<FLinearColor> StagingRT_Buffer;

	UPROPERTY(Transient)
	TArray<UMapRevealerComponent*> MapRevealers;

};
