

#pragma once

#include "Components/PrimitiveComponent.h"
#include "MapAreaBase.generated.h"

class UBoxComponent;
class UMapViewComponent;
class UMapAreaPrimitiveComponent;

UCLASS()
class MINIMAPPLUGIN_API UMapAreaPrimitiveComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:

	FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

public:
	FVector ScaledBoxExtent;

};

UCLASS()
class MINIMAPPLUGIN_API AMapAreaBase : public AActor
{
	GENERATED_BODY()

public:	
	AMapAreaBase();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
#endif

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UBoxComponent* GetAreaBounds() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UMapViewComponent* GetMapView() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetMapAspectRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	bool GetMapViewCornerUVs(UMapViewComponent* MapView, TArray<FVector2D>& CornerUVs);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	virtual int32 GetLevelAtHeight(const float WorldZ) const;

protected:

	virtual FVector2D CorrectUVs(const int32 Level, const FVector2D& InUV) const;

private:

	void ApplyAreaBounds();

	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	UBoxComponent* AreaBounds;

	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	UMapAreaPrimitiveComponent* AreaPrimitive;

	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	UMapViewComponent* AreaMapView;

};
