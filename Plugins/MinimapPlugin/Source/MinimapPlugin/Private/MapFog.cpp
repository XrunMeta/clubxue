

#include "MapFog.h"
#include "MinimapPluginPrivatePCH.h"
#include "MapFunctionLibrary.h"
#include "MapTrackerComponent.h"
#include "MapRevealerComponent.h"
#include "MapViewComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"

AMapFog::AMapFog()
{

	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultFogMaterial_UMG(TEXT("/MinimapPlugin/Materials/Fog/M_UMG_Fog"));
	FogMaterial_UMG = DefaultFogMaterial_UMG.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultFogMaterial_Canvas(TEXT("/MinimapPlugin/Materials/Fog/M_Canvas_Fog"));
	FogMaterial_Canvas = DefaultFogMaterial_Canvas.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultFogCombineMaterial(TEXT("/MinimapPlugin/Materials/Fog/M_FogCombine"));
	FogCombineMaterial = DefaultFogCombineMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultFogPostProcessMaterial(TEXT("/MinimapPlugin/Materials/Fog/M_WorldFog"));
	FogPostProcessMaterial = DefaultFogPostProcessMaterial.Object;
}

void AMapFog::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		SetActorTickEnabled(false);
		return;
	}

	InitializeWorldFog();

	const int32 RenderTargetSize = FMath::Max(2, FogRenderTargetSize);
	PermanentRevealRT_A = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize, RenderTargetSize);
	PermanentRevealRT_B = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize, RenderTargetSize);
	RevealRT_Staging = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize, RenderTargetSize);

	UMapTrackerComponent* Tracker = UMapFunctionLibrary::GetMapTracker(this);
	if (Tracker)
	{
		Tracker->RegisterMapFog(this);
		Tracker->OnMapRevealerRegistered.AddUniqueDynamic(this, &AMapFog::OnMapRevealerRegistered);
		Tracker->OnMapRevealerUnregistered.AddUniqueDynamic(this, &AMapFog::OnMapRevealerUnregistered);

		TArray<UMapRevealerComponent*> Revealers = Tracker->GetMapRevealers();
		for (UMapRevealerComponent* Revealer : Revealers)
			OnMapRevealerRegistered(Revealer);
	}

	AnimStartTime = GetWorld()->GetTimeSeconds();

	if (FogCombineMaterial)
	{
		FogCombineMatInst = UMaterialInstanceDynamic::Create(FogCombineMaterial, this);
		FogCombineMatInst->SetTextureParameterValue(TEXT("NewFog"), RevealRT_Staging);
	}
}

void AMapFog::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
		return;

	UMapTrackerComponent* Tracker = UMapFunctionLibrary::GetMapTracker(this);
	if (Tracker)
	{
		Tracker->UnregisterMapFog(this);
		Tracker->OnMapRevealerRegistered.RemoveDynamic(this, &AMapFog::OnMapRevealerRegistered);
		Tracker->OnMapRevealerUnregistered.RemoveDynamic(this, &AMapFog::OnMapRevealerUnregistered);
	}
}

void AMapFog::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UKismetRenderingLibrary::ClearRenderTarget2D(this, RevealRT_Staging, FLinearColor::Black);

	UCanvas* Canvas;
	FVector2D Size;
	FDrawToRenderTargetContext RenderContext;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, RevealRT_Staging, Canvas, Size, RenderContext);

	TSet<UMapRevealerComponent*> TemporaryRevealers;
	TSet<UMapRevealerComponent*> PermanentRevealers;
	for (UMapRevealerComponent* Revealer : MapRevealers)
	{
		if (Revealer->GetRevealMode() == EMapFogRevealMode::Off)
			continue;
		Revealer->UpdateMapFog(this, Canvas);
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, RenderContext);

	if (FogCombineMatInst)
	{

		UTextureRenderTarget2D* OldRT = bUseBufferA ? PermanentRevealRT_A : PermanentRevealRT_B;
		UTextureRenderTarget2D* NewRT = bUseBufferA ? PermanentRevealRT_B : PermanentRevealRT_A;
		bUseBufferA = !bUseBufferA;

		FogCombineMatInst->SetTextureParameterValue(TEXT("OldFog"), OldRT);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, NewRT, FogCombineMatInst);

		if (FogPostProcessMatInst)
			FogPostProcessMatInst->SetTextureParameterValue(TEXT("FogRenderTarget"), GetDestinationFogRenderTarget());
	}

	const float Time = GetWorld()->GetTimeSeconds();
	if (Time - StagingRT_LastReadTime > FogCacheLifetime)
		bStagingRT_Read = false;
	if (Time - PermanentRT_LastReadTime > FogCacheLifetime)
		bPermanentRT_Read = false;
}

bool AMapFog::GetFogAtLocation(const FVector& WorldLocation, const bool bRequireCurrentlyRevealing, float& RevealFactor)
{
	float U, V;
	if (FogRenderTargetSize <= 0 || !GetMapView()->GetViewCoordinates(WorldLocation, false, U, V))
		return false;

	bool& bRelevantReadFlag = bRequireCurrentlyRevealing ? bStagingRT_Read : bPermanentRT_Read;
	TArray<FLinearColor>& RelevantBuffer = bRequireCurrentlyRevealing ? StagingRT_Buffer : PermanentRT_Buffer;
	float& RelevantLastReadTime = bRequireCurrentlyRevealing ? StagingRT_LastReadTime : PermanentRT_LastReadTime;
	if (!bRelevantReadFlag)
	{

		UTextureRenderTarget2D* RelevantRenderTarget = bRequireCurrentlyRevealing ? RevealRT_Staging : PermanentRevealRT_A;
		FTextureRenderTarget2DResource* TextureResource = static_cast<FTextureRenderTarget2DResource*>(RelevantRenderTarget->GetResource());
		TextureResource->ReadLinearColorPixels(RelevantBuffer);
		checkf(RelevantBuffer.Num() > 0, TEXT("Expected pixels to be retrieved"));

		bRelevantReadFlag = true;
		RelevantLastReadTime = GetWorld()->GetTimeSeconds();
	}

	const int32 i = FMath::RoundToInt(U * FogRenderTargetSize);
	const int32 j = FMath::RoundToInt(V * FogRenderTargetSize);
	const int32 PixelIndex = FMath::Clamp(j * FogRenderTargetSize + i, 0, RelevantBuffer.Num() - 1);

	const FLinearColor PixelValue = RelevantBuffer[PixelIndex];
	RevealFactor = FMath::Clamp(FMath::Max(PixelValue.R, PixelValue.G), 0.0f, 1.0f);
	return true;
}

UTextureRenderTarget2D* AMapFog::GetDestinationFogRenderTarget() const
{
	return bUseBufferA ? PermanentRevealRT_B : PermanentRevealRT_A;
}

UTextureRenderTarget2D* AMapFog::GetSourceFogRenderTarget() const
{
	return bUseBufferA ? PermanentRevealRT_A : PermanentRevealRT_B;
}

float AMapFog::GetWorldToPixelRatio() const
{
	const float WorldSize = 2.0f * GetAreaBounds()->GetScaledBoxExtent().X;
	return (WorldSize > 0) ? (static_cast<float>(FogRenderTargetSize) / WorldSize) : 1.0f;
}

void AMapFog::SetFogMaterialForUMG(UMaterialInterface* NewMaterial)
{
	FogMaterial_UMG = NewMaterial;
	OnMapFogMaterialChanged.Broadcast(this);
}

UMaterialInterface* AMapFog::GetFogMaterialForUMG()
{
	return FogMaterial_UMG;
}

void AMapFog::SetFogMaterialForCanvas(UMaterialInterface* NewMaterial)
{

	FogMaterial_Canvas = NewMaterial;
	OnMapFogMaterialChanged.Broadcast(this);

	MaterialInstances.Empty();

	AnimStartTime = GetWorld()->GetTimeSeconds();
}

UMaterialInstanceDynamic* AMapFog::GetFogMaterialInstanceForCanvas(UMapRendererComponent* Renderer)
{
	if (!Renderer || !FogMaterial_Canvas)
		return nullptr;

	if (!MaterialInstances.Contains(Renderer))
	{

		UMaterialInstanceDynamic* NewInst = UMaterialInstanceDynamic::Create(FogMaterial_Canvas, this);
		MaterialInstances.Add(Renderer, NewInst);
		NewInst->SetScalarParameterValue(TEXT("OpacityHidden"), MinimapOpacityHidden);
		NewInst->SetScalarParameterValue(TEXT("OpacityExplored"), MinimapOpacityExplored);
		NewInst->SetScalarParameterValue(TEXT("OpacityViewing"), MinimapOpacityRevealing);
	}

	UMaterialInstanceDynamic* MatInst = MaterialInstances[Renderer];
	MatInst->SetScalarParameterValue(TEXT("Time"), GetWorld()->GetTimeSeconds() - AnimStartTime);
	MatInst->SetTextureParameterValue(TEXT("FogRenderTarget"), GetDestinationFogRenderTarget());
	return MatInst;
}

void AMapFog::InitializeWorldFog()
{

	if (!bEnableWorldFog || !FogPostProcessMaterial)
		return;

	bool bAllowAutoLocate = false;
	bool bAllowAutoCreate = false;
	switch (AutoLocatePostProcessVolume)
	{
	case EFogPostProcessVolumeOption::AutoLocate:
		bAllowAutoLocate = true;
		break;
	case EFogPostProcessVolumeOption::AutoLocateOrCreate:
		bAllowAutoLocate = true;
		bAllowAutoCreate = true;
		break;
	default: ;
	}

	if (!PostProcessVolume && bAllowAutoLocate)
	{

		for (TActorIterator<APostProcessVolume> PPItr(GetWorld()); PPItr && !PostProcessVolume; ++PPItr)
			if (PPItr->bUnbound)
				PostProcessVolume = *PPItr;

		for (TActorIterator<APostProcessVolume> PPItr(GetWorld()); PPItr && !PostProcessVolume; ++PPItr)
			PostProcessVolume = *PPItr;
	}

	if (!PostProcessVolume && bAllowAutoCreate)
	{
		PostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>();
		PostProcessVolume->bUnbound = true;
	}

	if (!PostProcessVolume)
		return;

	FogPostProcessMatInst = UMaterialInstanceDynamic::Create(FogPostProcessMaterial, this);

	FogPostProcessMatInst->SetTextureParameterValue(TEXT("FogRenderTarget"), GetDestinationFogRenderTarget());

	const FVector FogLocation = GetActorLocation();
	const FVector FogExtent = GetAreaBounds()->GetScaledBoxExtent();
	const float FogAngle = GetActorRotation().Yaw;
	FLinearColor FogVolumeBounds(FogLocation.X, FogLocation.Y, FogExtent.X, FogExtent.Y);
	FogPostProcessMatInst->SetVectorParameterValue(TEXT("FogVolumeBounds"), FogVolumeBounds);
	FogPostProcessMatInst->SetScalarParameterValue(TEXT("FogVolumeAngle"), FogAngle);
	FogPostProcessMatInst->SetScalarParameterValue(TEXT("OpacityHidden"), WorldOpacityHidden);
	FogPostProcessMatInst->SetScalarParameterValue(TEXT("OpacityExplored"), WorldOpacityExplored);
	FogPostProcessMatInst->SetScalarParameterValue(TEXT("OpacityViewing"), WorldOpacityRevealing);

	PostProcessVolume->AddOrUpdateBlendable(FogPostProcessMatInst);
}

void AMapFog::OnMapRevealerRegistered(UMapRevealerComponent* MapRevealer)
{
	MapRevealers.Add(MapRevealer);
}

void AMapFog::OnMapRevealerUnregistered(UMapRevealerComponent* MapRevealer)
{
	MapRevealers.RemoveSingle(MapRevealer);
}