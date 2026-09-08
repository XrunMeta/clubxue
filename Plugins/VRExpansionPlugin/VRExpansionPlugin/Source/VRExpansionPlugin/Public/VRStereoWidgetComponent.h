

#pragma once

#include "CoreMinimal.h"

#include "Components/StereoLayerComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/TextureRenderTarget2D.h"

#include "VRStereoWidgetComponent.generated.h"

class FWidgetRenderer;
class UUserWidget;
class UTextureRenderTarget2D;
class UStereoLayerShape;
struct FVRStereoWidgetComponentInstanceData;

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin), HideCategories = ("Stereoscopic Properties", Collision))
class VREXPANSIONPLUGIN_API UVRStereoWidgetRenderComponent : public UStereoLayerComponent
{
	GENERATED_BODY()

public:
	UVRStereoWidgetRenderComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetSettings", meta = (ExposeOnSpawn = true))
		TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(BlueprintReadWrite, Transient, DuplicateTransient, Category = "WidgetSettings")
		TObjectPtr<UUserWidget> Widget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetSettings", meta = (ExposeOnSpawn = true))
		bool bDrawAtDesiredSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetSettings", meta = (ExposeOnSpawn = true))
		float WidgetRenderScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetSettings", meta = (ExposeOnSpawn = true))
		float WidgetRenderGamma;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetSettings", meta = (ExposeOnSpawn = true))
	bool bUseGammaCorrection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetSettings", meta = (ExposeOnSpawn = true))
		FLinearColor RenderTargetClearColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetSettings", meta = (ExposeOnSpawn = true))
		bool bDrawWithoutStereo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetSettings", meta = (ExposeOnSpawn = true))
		float DrawRate;

	float DrawCounter;

	TSharedPtr<SWidget> SlateWidget;

	class FWidgetRenderer* WidgetRenderer;

	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category = "WidgetSettings")
		TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	TSharedPtr<class SVirtualWindow> SlateWindow;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void DestroyComponent(bool bPromoteChildren) override;

	UFUNCTION(BlueprintCallable, Category = "WidgetSettings")
	void SetWidgetAndInit(TSubclassOf<UUserWidget> NewWidgetClass);

	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);
	void InitWidget();
	void RenderWidget(float DeltaTime);
	void ReleaseResources();
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRStereoWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UVRStereoWidgetComponent(const FObjectInitializer& ObjectInitializer);

	friend class FStereoLayerComponentVisualizer;

	~UVRStereoWidgetComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, NoClear, Instanced, Category = "StereoLayer", DisplayName = "Stereo Layer Shape")
		TObjectPtr<UStereoLayerShape> Shape;

	void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void OnUnregister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void DrawWidgetToRenderTarget(float DeltaTime) override;
	virtual TStructOnScope<FActorComponentInstanceData>  GetComponentInstanceData() const override;
	void ApplyVRComponentInstanceData(struct FVRStereoWidgetComponentInstanceData* WidgetInstanceData);

	virtual void UpdateRenderTarget(FIntPoint DesiredRenderTargetSize) override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		bool bAlwaysVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		bool bRenderBothStereoAndWorld;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		bool bDrawWithoutStereo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		bool bUseEpicsWorldLockedStereo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		bool bDelayForRenderThread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		bool bIsSleeping;

	UFUNCTION(BlueprintCallable, Category = "Components|Stereo Layer")
		void SetPriority(int32 InPriority);

	UFUNCTION(BlueprintCallable, Category = "Components|Stereo Layer")
		int32 GetPriority() const { return Priority; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		uint32 bSupportsDepth : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		uint32 bNoAlphaChannel : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		uint32 bQuadPreserveTextureRatio : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StereoLayer", Meta = (GetOptions = "EditorFlagCollector.GetFlagNames"))
		TArray<FName> AdditionalFlags;

	UPROPERTY()
	TObjectPtr<class UTexture2D> TextureRef = nullptr;

protected:

public:

		FBox2D UVRect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "StereoLayer")
		int32 Priority;

	uint32 LayerId;

	bool bShouldCreateProxy;

private:

	bool bIsDirty;
	bool bDirtyRenderTarget;

	bool bTextureNeedsUpdate;

	FTransform LastTransform;

	bool bLastVisible;

};

USTRUCT()
struct FVRStereoWidgetComponentInstanceData : public FActorComponentInstanceData
{
	GENERATED_BODY()
public:
	FVRStereoWidgetComponentInstanceData(const UVRStereoWidgetComponent* SourceComponent)
		: FActorComponentInstanceData(SourceComponent)
		, RenderTarget(SourceComponent->GetRenderTarget())
	{
	}

	FVRStereoWidgetComponentInstanceData()
		: FActorComponentInstanceData()
	{
	}

	virtual void ApplyToComponent(UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override
	{
		FActorComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);
		CastChecked<UVRStereoWidgetComponent>(Component)->ApplyVRComponentInstanceData(this);
	}

public:
	UTextureRenderTarget2D* RenderTarget;
};