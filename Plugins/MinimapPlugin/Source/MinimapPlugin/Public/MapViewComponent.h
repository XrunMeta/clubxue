

#pragma once

#include "Components/BoxComponent.h"
#include "MapEnums.h"
#include "MapViewComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapViewCategoriesChangedSignature, UMapViewComponent*, MapView);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapViewSizeChangedSignature, UMapViewComponent*, MapView);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapViewDestroyedSignature, UMapViewComponent*, MapView);

class UMapIconComponent;
class AMapBackground;

UCLASS(ClassGroup=(MinimapPlugin), meta=(BlueprintSpawnableComponent))
class MINIMAPPLUGIN_API UMapViewComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	UMapViewComponent();

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconCategoryVisible(FName IconCategory, const bool bNewVisible);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsIconCategoryVisible(FName IconCategory) const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetViewExtent(const float NewViewExtentX, const float NewViewExtentY);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	void GetViewExtent(float& ViewExtentX, float& ViewExtentY) const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetZoomScale(const float NewZoomScale);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetZoomScale() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetViewAspectRatio() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	TArray<FVector> GetWorldCorners();

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool ViewContains(const FVector& WorldPos, const float WorldRadius) const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	bool GetViewCoordinates(const FVector& WorldPos, bool bForceRectangular, float& U, float& V);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void GetViewYaw(const float WorldYaw, float& Yaw);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	void DeprojectViewToWorld(const float U, const float V, FVector& WorldPos);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	int32 GetActiveBackgroundPriority(bool& IsInsideAnyBackground);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	int32 GetActiveBackgroundLevel(const AMapBackground* MapBackground);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsSameBackgroundLevel(const UMapIconComponent* MapIcon);

private:
	UFUNCTION()
	void RegisterMultiLevelMapBackground(AMapBackground* MapBackground);
	UFUNCTION()
	void UnregisterMultiLevelMapBackground(AMapBackground* MapBackground);

	void UpdateViewSize();

	void UpdateTransformCache();

	void UpdateBackgroundCache();

public:

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapViewCategoriesChangedSignature OnVisibleCategoriesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapViewSizeChangedSignature OnViewSizeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapViewDestroyedSignature OnViewDestroyed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	EMapViewRotationMode RotationMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	FRotator FixedRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float InheritedYawOffset = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	bool bSupportZooming = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	USceneComponent* HeightProxy;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	float BackgoundLevelCacheLifetime = 0.05f;

private:

	FTransform LastTransform;
	FTransform CachedTransform;
	FTransform CachedInverseTransform;
	FVector2D CachedInverseViewSize;
	float InverseViewRadius;

	float LastBackgroundLevelComputeTime = 0.0f;

	int32 BackgroundPriority = INT_MIN;
	bool bInsideMultiLevelBackground = false;
	bool bInsideAnyBackground = false;

	UPROPERTY(Transient)
	TSet<AMapBackground*> MapBackgrounds;

	UPROPERTY(Transient)
	TMap<AMapBackground*, int32> PositionOnMultiLevelBackgrounds;

	TSet<FName> HiddenIconCategories;

};
