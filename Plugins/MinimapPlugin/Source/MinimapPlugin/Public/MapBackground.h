

#pragma once

#include "MapAreaBase.h"
#include "MapBackground.generated.h"

class UBoxComponent;
class USceneCaptureComponent2D;
class UNavMeshRenderingComponent;
class UTextureRenderTarget2D;
class UMapViewComponent;
class UMapRendererComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapBackgroundTextureChangedSignature, AMapBackground*, MapBackground);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapBackgroundMaterialChangedSignature, AMapBackground*, MapBackground);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapBackgroundAppearanceChangedSignature, AMapBackground*, MapBackground);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMapBackgroundRenderedSignature, AMapBackground*, MapBackground, int32, Level, UTextureRenderTarget2D*, RenderTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMapBackgroundOverlayChangedSignature, AMapBackground*, MapBackground, int32, Level, UTextureRenderTarget2D*, RenderTarget);

USTRUCT(BlueprintType)
struct FMapBackgroundLevel
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Minimap Snapshot Generation")
	UTexture2D* BackgroundTexture = nullptr;

	UPROPERTY(EditAnywhere, Category = "Minimap Snapshot Generation")
	UTextureRenderTarget2D* RenderTarget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Minimap Snapshot Generation")
	UTextureRenderTarget2D* Overlay = nullptr;

	UPROPERTY(EditAnywhere, Category = "Minimap Snapshot Generation")
	float LevelHeight = 0.0f;

	UPROPERTY(Transient)
	FVector2D SamplingResolution;

};

UCLASS()
class MINIMAPPLUGIN_API AMapBackground : public AMapAreaBase
{
	GENERATED_BODY()

public:

	static TArray<FString> HiddenShowFlagNames;

	AMapBackground();

#if WITH_EDITOR
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetBackgroundMaterialForUMG(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UMaterialInterface* GetBackgroundMaterialForUMG() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetBackgroundMaterialForCanvas(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UMaterialInstanceDynamic* GetBackgroundMaterialInstanceForCanvas(UMapRendererComponent* Renderer);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetBackgroundVisible(const bool bNewVisible);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsBackgroundVisible() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetBackgroundPriority(const int32 NewBackgroundPriority);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	int32 GetBackgroundPriority() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetBackgroundZOrder(const int32 NewBackgroundZOrder);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	int32 GetBackgroundZOrder() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsMultiLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetBackgroundTexture(const int32 Level = 0, UTexture2D* NewBackgroundTexture = nullptr);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UTexture* GetBackgroundTexture(const int32 Level = 0) const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetBackgroundOverlay(const int32 Level = 0, UTextureRenderTarget2D* NewBackgroundOverlay = nullptr);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UTextureRenderTarget2D* GetBackgroundOverlay(const int32 Level = 0) const;

	virtual int32 GetLevelAtHeight(const float WorldZ) const override;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UTexture* GetBackgroundTextureAtHeight(const float WorldZ) const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void RerenderBackground();

	UFUNCTION(CallInEditor, Category = "Minimap Background")
	void CaptureBackground();

protected:

	virtual FVector2D CorrectUVs(const int32 Level, const FVector2D& InUV) const;

private:
#if WITH_EDITOR
	void VisualizeLevelsInEditor();
#endif

	void NormalizeScale();

	void InitializeDynamicRenderTargets();

	void ApplyBackgroundTexture(bool bForceRerender);

	void GenerateSnapshot(UTextureRenderTarget2D* RenderTarget, float RelativeHeight);

public:

	UPROPERTY(BlueprintAssignable, Category = "Minimap Background")
	FMapBackgroundTextureChangedSignature OnMapBackgroundTextureChanged;

	UPROPERTY(BlueprintAssignable, Category = "Minimap Background")
	FMapBackgroundMaterialChangedSignature OnMapBackgroundMaterialChanged;

	UPROPERTY(BlueprintAssignable, Category = "Minimap Background")
	FMapBackgroundAppearanceChangedSignature OnMapBackgroundAppearanceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Minimap Background")
	FMapBackgroundRenderedSignature OnMapBackgroundRendered;

	UPROPERTY(BlueprintAssignable, Category = "Minimap Background")
	FMapBackgroundOverlayChangedSignature OnMapBackgroundOverlayChanged;

protected:

	UPROPERTY(EditAnywhere, Category = "Minimap Background")
	TArray<FMapBackgroundLevel> BackgroundLevels;

	UPROPERTY(EditAnywhere, Category = "Minimap Background")
	UMaterialInterface* BackgroundMaterial_UMG;

	UPROPERTY(EditAnywhere, Category = "Minimap Background")
	UMaterialInterface* BackgroundMaterial_Canvas;

	UPROPERTY(EditAnywhere, Category = "Minimap Background")
	bool bBackgroundVisible = true;

	UPROPERTY(EditAnywhere, Category = "Minimap Background")
	int32 BackgroundPriority;

	UPROPERTY(EditAnywhere, Category = "Minimap Background")
	int32 BackgroundZOrder;

	UPROPERTY(EditAnywhere, Category = "Minimap Background")
	int32 DynamicRenderTargetSize = 1024;

	UPROPERTY(EditAnywhere, Category = "Minimap Background")
	TArray<TSubclassOf<AActor>> HiddenActorClasses;

	UPROPERTY(EditAnywhere, Category = "Minimap Background")
	TArray<AActor*> HiddenActors;

	UPROPERTY(EditAnywhere, Category = "Minimap Background", DisplayName = "Render Navigation Mesh (Editor Only)")
	bool bRenderNavigationMesh = true;

private:

	UPROPERTY(Transient)
	TMap<UMapRendererComponent*, UMaterialInstanceDynamic*> MaterialInstances;

	UPROPERTY(Transient)
	TArray<UBoxComponent*> LevelVisualizers;

	float AnimStartTime;

	UPROPERTY(VisibleAnywhere, Category = "Minimap Background Generation")
	USceneCaptureComponent2D* CaptureComponent2D;

	UPROPERTY(VisibleAnywhere, Category = "Minimap Background Generation")
	UNavMeshRenderingComponent* NavMeshRenderingComponent;

};
