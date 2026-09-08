

#include "MapFunctionLibrary.h"
#include "MinimapPluginPrivatePCH.h"
#include "MapTrackerComponent.h"
#include "MapViewComponent.h"
#include "MapIconComponent.h"
#include "MapBackground.h"
#include "MapFog.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameState.h"
#include "Engine/Engine.h"
#include "EngineGlobals.h"

UMapTrackerComponent* UMapFunctionLibrary::GetMapTracker(const UObject* WorldContextObject)
{

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 16
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
#else
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
#endif
	if (!World)
		return nullptr;

	if (World->GetNetMode() == ENetMode::NM_DedicatedServer)
		return nullptr;

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 13
	AGameState* GS = World->GetGameState();
#else
	AGameStateBase* GS = World->GetGameState();
#endif

	if (!GS)
		return nullptr;

	UMapTrackerComponent* MapTracker = Cast<UMapTrackerComponent>(GS->GetComponentByClass(UMapTrackerComponent::StaticClass()));
	if (MapTracker)
	{

		return MapTracker;
	}
	else
	{

		MapTracker = NewObject<UMapTrackerComponent>(GS, TEXT("MapTracker"));
		return MapTracker;
	}
}

AMapBackground* UMapFunctionLibrary::GetFirstMapBackground(const UObject* WorldContextObject)
{

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 16
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
#else
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
#endif
	for (TActorIterator<AMapBackground> Itr(World); Itr; ++Itr)
		return *Itr;

	return nullptr;
}

UMapViewComponent* UMapFunctionLibrary::FindMapView(UObject* WorldContextObject, const EMapViewSearchOption MapViewSearchOption)
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 16
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
#else
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
#endif

	bool bConsiderPlayer = false;
	bool bConsiderMapBackground = false;
	bool bConsiderMapFog = false;
	bool bConsiderAllActors = false;
	switch (MapViewSearchOption)
	{
	case EMapViewSearchOption::Any:
		bConsiderPlayer = true;
		bConsiderMapBackground = true;
		bConsiderMapFog = true;
		bConsiderAllActors = true;
		break;
	case EMapViewSearchOption::OnPlayer:
		bConsiderPlayer = true;
		break;
	case EMapViewSearchOption::OnMapBackground:
		bConsiderMapBackground = true;
		break;
	case EMapViewSearchOption::OnMapFog:
		bConsiderMapFog = true;
		break;
	case EMapViewSearchOption::Disabled:
		return nullptr;
	}

	if (bConsiderPlayer)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
		UMapViewComponent* MapView = PlayerPawn ? PlayerPawn->FindComponentByClass<UMapViewComponent>() : nullptr;
		if (MapView)
			return MapView;

		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
		MapView = PlayerController ? PlayerController->FindComponentByClass<UMapViewComponent>() : nullptr;
		if (MapView)
			return MapView;
	}

	if (bConsiderMapBackground)
		for (TActorIterator<AMapBackground> Itr(World); Itr; ++Itr)
			return Itr->GetMapView();

	if (bConsiderMapFog)
		for (TActorIterator<AMapFog> Itr(World); Itr; ++Itr)
			return Itr->GetMapView();

	if (bConsiderAllActors)
		for (TObjectIterator<UMapViewComponent> Itr; Itr; ++Itr)
			if (Itr->GetWorld() == World)
				return *Itr;

	return nullptr;
}

bool UMapFunctionLibrary::DetectIsInView(const FVector2D& UV, const FVector2D& OuterRadiusUV, const bool bIsCircular)
{
	if (bIsCircular)
	{

		return FMath::Pow(UV.X - 0.5f, 2) + FMath::Pow(UV.Y - 0.5f, 2) < FMath::Pow(0.5f + OuterRadiusUV.X, 2);
	}
	else
	{

		return UV.X > -OuterRadiusUV.X && UV.X < 1 + OuterRadiusUV.X && UV.Y > -OuterRadiusUV.Y && UV.Y < 1 + OuterRadiusUV.Y;
	}
}

FVector2D UMapFunctionLibrary::ClampIntoView(const FVector2D& UV, const float OuterRadiusUV, const bool bIsCircular)
{
	FVector2D UVOut;
	if (bIsCircular)
	{

		const float Angle = FMath::Atan2(UV.Y - 0.5f, UV.X - 0.5f);
		const float Radius = 0.5f - OuterRadiusUV;
		UVOut.X = 0.5f + FMath::Cos(Angle) * Radius;
		UVOut.Y = 0.5f + FMath::Sin(Angle) * Radius;
	}
	else
	{

		const float CenteredX = UV.X - 0.5f;
		const float CenteredY = UV.Y - 0.5f;
		const float Angle = FMath::Atan2(CenteredY, CenteredX);
		if (FMath::Abs(CenteredX) > FMath::Abs(CenteredY))
		{

			const float ClampedX = FMath::Sign(CenteredX) * (0.5f - OuterRadiusUV);
			UVOut.X = 0.5f + ClampedX;
			UVOut.Y = 0.5f + FMath::Tan(Angle) * ClampedX;
		}
		else
		{

			const float ClampedY = FMath::Sign(CenteredY) * (0.5f - OuterRadiusUV);
			UVOut.X = 0.5f + ClampedY / FMath::Tan(Angle);
			UVOut.Y = 0.5f + ClampedY;
		}
	}

	return UVOut;
}

TArray<UMapIconComponent*> UMapFunctionLibrary::BoxSelectInView(const FVector2D& StartUV, const FVector2D& EndUV, UMapViewComponent* MapView, const bool bIsCircular)
{
	UMapTrackerComponent* MapTracker = UMapFunctionLibrary::GetMapTracker(MapView);
	if (!MapTracker)
		return TArray<UMapIconComponent*>();

	const FVector2D UVMin(FMath::Min(StartUV.X, EndUV.X), FMath::Min(StartUV.Y, EndUV.Y));
	const FVector2D UVMax(FMath::Max(StartUV.X, EndUV.X), FMath::Max(StartUV.Y, EndUV.Y));

	float U, V;
	TArray<UMapIconComponent*> Results;
	TArray<UMapIconComponent*> AllIcons = MapTracker->GetMapIcons();
	for (UMapIconComponent* MapIcon : AllIcons)
	{

		if (!MapIcon->IsIconVisible())
			continue;

		if (!MapIcon->IsRenderedInView(MapView))
			continue;

		if (MapView->GetViewCoordinates(MapIcon->GetComponentLocation(), bIsCircular, U, V) && DetectIsInView(FVector2D(U, V), FVector2D::ZeroVector, bIsCircular) && U >= UVMin.X && U <= UVMax.X && V >= UVMin.Y && V <= UVMax.Y)
			Results.Add(MapIcon);
	}

	return Results;
}

bool UMapFunctionLibrary::ComputeViewFrustum(const UObject* WorldContextObject, UMapViewComponent* MapView, const bool bIsCircular, TArray<FVector2D>& CornerUVs, float FloorDistance )
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 16
    const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
#else
    const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
#endif
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
		return false;

	FVector ViewPos;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewPos, ViewRot);
	const FVector ViewDir = ViewRot.Vector();
	while (ViewRot.Pitch > 180)
		ViewRot.Pitch -= 360;

	if (ViewRot.Pitch >= -40.0f)
		return false;

	int32 Width, Height;
	PC->GetViewportSize(Width, Height);
	TArray<FVector2D> ViewportCorners2D;
	ViewportCorners2D.Add(FVector2D(0, 0));
	ViewportCorners2D.Add(FVector2D(Width, 0));
	ViewportCorners2D.Add(FVector2D(Width, Height));
	ViewportCorners2D.Add(FVector2D(0, Height));

	const float FloorZ = ViewPos.Z - FloorDistance;
	const FPlane FloorPlane(FVector::UpVector, FloorZ);

	TArray<FVector> WorldCorners;
	for (const auto& P : ViewportCorners2D)
	{

		FVector WorldPos, WorldDir;
		UGameplayStatics::DeprojectScreenToWorld(PC, P, WorldPos, WorldDir);

		if (WorldDir.Z >= 0)
			return false;

		float T; FVector FloorPos;
		if (!UKismetMathLibrary::LinePlaneIntersection(WorldPos, WorldPos + 100000.0f * WorldDir, FloorPlane, T, FloorPos))
			return false;

		WorldCorners.Add(FloorPos);
	}

	for (uint8 i = 0; i < 4; ++i)
	{
		float U, V;
		MapView->GetViewCoordinates(WorldCorners[i], bIsCircular, U, V);
		CornerUVs.Add(FVector2D(U, V));
	}
	return true;
}
