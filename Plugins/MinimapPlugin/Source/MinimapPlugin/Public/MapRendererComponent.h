

#pragma once

#include "Components/ActorComponent.h"
#include "Types/SlateEnums.h"
#include "Layout/Margin.h"
#include "MapEnums.h"
#include "MapRendererComponent.generated.h"

class UMapTrackerComponent;
class UMapViewComponent;
class UMapIconComponent;
class UCanvas;
class UMaterialInterface;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMapClickedSignature, FVector, WorldLocation, bool, bIsLeftMouseButton);

UCLASS(ClassGroup=(MinimapPlugin), meta=(BlueprintSpawnableComponent))
class MINIMAPPLUGIN_API UMapRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMapRendererComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetAutoLocateMapView(const EMapViewSearchOption InAutoLocateMapView);

	void DrawToCanvas(UCanvas* Canvas);

	bool HandleClick(const FVector2D& ScreenPosition, const bool bIsLeftClick);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetMapView(UMapViewComponent* InMapView);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIsCircular(const bool bNewIsCircular);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsCircular() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIsRendered(const bool bNewIsRendered);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsRendered() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetDrawFrustum(const bool bNewDrawFrustum);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool GetDrawFrustum() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetFrustumFloorDistance(const float NewFrustumFloorDistance);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetFrustumFloorDistance() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetBackgroundFillColor(const FLinearColor& NewBackgroundFillColor);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	FLinearColor GetBackgroundFillColor() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetMargin(const int32 Left, const int32 Top, const int32 Right, const int32 Bottom);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetSize(const int32 Width, const int32 Height);

private:
	void AutoRelocateMapView();

	void TickHoverEvents();

	void ClearHoverEvents();

	void MarkOnHoverStart(UMapIconComponent* MapIcon);

	void MarkOnHoverEnd(UMapIconComponent* MapIcon);

	void ComputeCanvasRect(UCanvas* Canvas, FVector2D& MapTopLeft, FVector2D& MapSize);

	void ComputeRenderRegion(const FVector2D& MapTopLeft, const FVector2D& MapSize, FVector2D& RenderRegionTopLeft, FVector2D& RenderRegionSize);

	void RenderToCanvas(UCanvas* Canvas, const FVector2D& MapTopLeft, const FVector2D& MapSize);

	void DrawBackground(UCanvas* Canvas, const FVector2D& RenderRegionTopLeft, const FVector2D& RenderRegionSize);
	void DrawFog(UCanvas* Canvas, const FVector2D& RenderRegionTopLeft, const FVector2D& RenderRegionSize);
	void DrawIcons(UCanvas* Canvas, const FVector2D& RenderRegionTopLeft, const FVector2D& RenderRegionSize, const bool bAboveFog);
	void DrawBoundary(UCanvas* Canvas, const FVector2D& RenderRegionTopLeft, const FVector2D& RenderRegionSize);
	void DrawFrustum(UCanvas* Canvas, const FVector2D& RenderRegionTopLeft, const FVector2D& RenderRegionSize);

public:

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapClickedSignature OnMapClicked;

protected:

	UPROPERTY(EditAnywhere, Category = "Minimap")
	EMapViewSearchOption AutoLocateMapView = EMapViewSearchOption::Any;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	bool bIsCircular = false;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	bool bIsRendered = true;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	bool bDrawFrustum = false;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	float FrustumFloorDistance = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	FLinearColor BackgroundFillColor = FLinearColor(0, 0, 0, 1);

	UPROPERTY(EditAnywhere, Category = "Minimap")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	FMargin Margin = FMargin(0, 0, 0, 0);

	UPROPERTY(EditAnywhere, Category = "Minimap")
	FVector2D Size = FVector2D(200, 200);

	UPROPERTY(EditAnywhere, Category = "Minimap")
	UMaterialInterface* FillMaterial;

private:

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* FillMaterialInstance;

	UPROPERTY(Transient)
	UMapTrackerComponent* MapTracker;

	UPROPERTY(Transient)
	UMapViewComponent* MapView;

	UPROPERTY(Transient)
	TSet<UMapIconComponent*> HoveringIcons;

	UPROPERTY(Transient)
	TArray<UMapIconComponent*> BufferedHoverStartEvents;

	UPROPERTY(Transient)
	TArray<UMapIconComponent*> BufferedHoverEndEvents;

	UPROPERTY(Transient)
	UCanvas* LastCanvas;

};
