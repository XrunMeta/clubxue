

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Layout/Margin.h"
#include "MapEnums.h"
#include "MapFunctionLibrary.generated.h"

class UCanvas;
class UMapTrackerComponent;
class UMapViewComponent;
class AMapBackground;

UCLASS()
class MINIMAPPLUGIN_API UMapFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Minimap", meta=(WorldContext="WorldContextObject"))
	static UMapTrackerComponent* GetMapTracker(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Minimap", meta=(WorldContext="WorldContextObject"))
	static AMapBackground* GetFirstMapBackground(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Minimap", meta = (WorldContext = "WorldContextObject"))
	static UMapViewComponent* FindMapView(UObject* WorldContextObject, const EMapViewSearchOption MapViewSearchOption);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	static bool DetectIsInView(const FVector2D& UV, const FVector2D& OuterRadiusUV, const bool bIsCircular);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	static FVector2D ClampIntoView(const FVector2D& UV, const float OuterRadiusUV, const bool bIsCircular);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	static TArray<UMapIconComponent*> BoxSelectInView(const FVector2D& StartUV, const FVector2D& EndUV, UMapViewComponent* MapView, const bool bIsCircular);

	UFUNCTION(BlueprintCallable, Category = "Minimap", meta = (WorldContext = "WorldContextObject"))
	static bool ComputeViewFrustum(const UObject* WorldContextObject, UMapViewComponent* MapView, const bool bIsCircular, TArray<FVector2D>& CornerUVs, const float FloorDistance = 600.0f);

};
