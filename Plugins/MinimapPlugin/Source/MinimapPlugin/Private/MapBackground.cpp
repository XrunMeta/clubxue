

#include "MapBackground.h"
#include "MinimapPluginPrivatePCH.h"
#include "MapTrackerComponent.h"
#include "MapViewComponent.h"
#include "MapFunctionLibrary.h"

#include "NavMesh/NavMeshRenderingComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

TArray<FString> AMapBackground::HiddenShowFlagNames = { "LocalExposure", "Lighting", "PostProcessing"};

AMapBackground::AMapBackground()
{

	CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent2D"));
	CaptureComponent2D->SetupAttachment(GetRootComponent());

	CaptureComponent2D->SetWorldRotation(FRotator(-90, -90, 0));
	CaptureComponent2D->ProjectionType = ECameraProjectionMode::Type::Orthographic;
	CaptureComponent2D->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;

	CaptureComponent2D->bCaptureEveryFrame = false;
	CaptureComponent2D->bCaptureOnMovement = false;

	CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	for (const FString& ShowFlagToHide : HiddenShowFlagNames)
	{
		if (FEngineShowFlagsSetting* ExistingSetting = CaptureComponent2D->ShowFlagSettings.FindByPredicate([ShowFlagToHide](const FEngineShowFlagsSetting& Setting) { return Setting.ShowFlagName == ShowFlagToHide; }))
		{
			ExistingSetting->Enabled = false;
		}
		else
		{
			FEngineShowFlagsSetting NewSetting;
			NewSetting.ShowFlagName = ShowFlagToHide;
			NewSetting.Enabled = false;
			CaptureComponent2D->ShowFlagSettings.Add(NewSetting);
		}
	}

#if WITH_EDITOR

	if (FProperty* ShowFlagsProp = USceneCaptureComponent2D::StaticClass()->FindPropertyByName(FName("ShowFlagSettings")))
	{
		FPropertyChangedEvent ShowFlagsChangeEvent(ShowFlagsProp);
		CaptureComponent2D->PostEditChangeProperty(ShowFlagsChangeEvent);
	}
#endif

	CaptureComponent2D->ShowFlags.SetFog(false);

	CaptureComponent2D->ShowFlags.SetNavigation(true);
	CaptureComponent2D->ShowFlags.SetDirectionalLights(false);

	CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_SceneColorSceneDepth;

	NavMeshRenderingComponent = CreateDefaultSubobject<UNavMeshRenderingComponent>(TEXT("NavMeshRenderer"));
	NavMeshRenderingComponent->SetupAttachment(GetRootComponent());
	NavMeshRenderingComponent->SetHiddenInGame(true);
	NavMeshRenderingComponent->bIsEditorOnly = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder_UMG(TEXT("/MinimapPlugin/Materials/Background/M_UMG_Background"));
	BackgroundMaterial_UMG = MaterialFinder_UMG.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder_Canvas(TEXT("/MinimapPlugin/Materials/Background/M_Canvas_Background"));
	BackgroundMaterial_Canvas = MaterialFinder_Canvas.Object;

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTargetAssetFinder(TEXT("/MinimapPlugin/Textures/RT_MinimapSnapshot"));

	FMapBackgroundLevel DefaultBackgroundLevel;
	DefaultBackgroundLevel.RenderTarget = RenderTargetAssetFinder.Object;
	BackgroundLevels.Empty();
	BackgroundLevels.Add(DefaultBackgroundLevel);

	HiddenActorClasses.Add(APawn::StaticClass());
}

#if WITH_EDITOR
void AMapBackground::PostLoad()
{
	Super::PostLoad();
	NormalizeScale();
	VisualizeLevelsInEditor();
}

void AMapBackground::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	NormalizeScale();
	VisualizeLevelsInEditor();
}

void AMapBackground::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	NormalizeScale();
	ApplyBackgroundTexture(false);
	VisualizeLevelsInEditor();
}
#endif

void AMapBackground::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	VisualizeLevelsInEditor();
#endif
}

void AMapBackground::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
		return;

	if (BackgroundLevels.Num() == 0)
		BackgroundLevels.Add(FMapBackgroundLevel());

	InitializeDynamicRenderTargets();

	ApplyBackgroundTexture(false);

	UMapTrackerComponent* Tracker = UMapFunctionLibrary::GetMapTracker(this);
	if (Tracker)
		Tracker->RegisterMapBackground(this);

	AnimStartTime = GetWorld()->GetTimeSeconds();
}

void AMapBackground::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
		return;

	UMapTrackerComponent* Tracker = UMapFunctionLibrary::GetMapTracker(this);
	if (Tracker)
		Tracker->UnregisterMapBackground(this);
}

void AMapBackground::SetBackgroundMaterialForUMG(UMaterialInterface* NewMaterial)
{

	BackgroundMaterial_UMG = NewMaterial;
	OnMapBackgroundMaterialChanged.Broadcast(this);
	OnMapBackgroundAppearanceChanged.Broadcast(this);
}

UMaterialInterface* AMapBackground::GetBackgroundMaterialForUMG() const
{

	return BackgroundMaterial_UMG;
}

void AMapBackground::SetBackgroundMaterialForCanvas(UMaterialInterface* NewMaterial)
{

	BackgroundMaterial_Canvas = NewMaterial;
	OnMapBackgroundMaterialChanged.Broadcast(this);
	OnMapBackgroundAppearanceChanged.Broadcast(this);

	MaterialInstances.Empty();

	AnimStartTime = GetWorld()->GetTimeSeconds();
}

UMaterialInstanceDynamic* AMapBackground::GetBackgroundMaterialInstanceForCanvas(UMapRendererComponent* Renderer)
{
	if (!Renderer || !BackgroundMaterial_Canvas)
		return nullptr;

	if (!MaterialInstances.Contains(Renderer))
	{

		UMaterialInstanceDynamic* NewInst = UMaterialInstanceDynamic::Create(BackgroundMaterial_Canvas, this);
		NewInst->SetTextureParameterValue(TEXT("Texture"), GetBackgroundTexture());
		MaterialInstances.Add(Renderer, NewInst);
	}

	UMaterialInstanceDynamic* MatInst = MaterialInstances[Renderer];
	MatInst->SetScalarParameterValue(TEXT("Time"), GetWorld()->GetTimeSeconds() - AnimStartTime);
	return MatInst;
}

void AMapBackground::SetBackgroundVisible(const bool bNewVisible)
{
	bBackgroundVisible = bNewVisible;
	OnMapBackgroundAppearanceChanged.Broadcast(this);
}

bool AMapBackground::IsBackgroundVisible() const
{
	return bBackgroundVisible;
}

void AMapBackground::SetBackgroundPriority(const int32 NewBackgroundPriority)
{
	BackgroundPriority = NewBackgroundPriority;
	OnMapBackgroundAppearanceChanged.Broadcast(this);
}

int32 AMapBackground::GetBackgroundPriority() const
{
	return BackgroundPriority;
}

void AMapBackground::SetBackgroundZOrder(const int32 NewBackgroundZOrder)
{
	BackgroundZOrder = NewBackgroundZOrder;
	OnMapBackgroundAppearanceChanged.Broadcast(this);
}

int32 AMapBackground::GetBackgroundZOrder() const
{
	return BackgroundZOrder;
}

bool AMapBackground::IsMultiLevel() const
{
	return BackgroundLevels.Num() > 1;
}

void AMapBackground::SetBackgroundTexture(const int32 Level, UTexture2D* NewBackgroundTexture)
{
	if (BackgroundLevels.IsValidIndex(Level) && NewBackgroundTexture != BackgroundLevels[Level].BackgroundTexture)
	{
		BackgroundLevels[Level].BackgroundTexture = NewBackgroundTexture;
		ApplyBackgroundTexture(false);
	}
}

UTexture* AMapBackground::GetBackgroundTexture(const int32 Level) const
{
	if (BackgroundLevels.Num() == 0)
		return nullptr;

	const int32 Index = FMath::Clamp(Level, 0, BackgroundLevels.Num() - 1);
	if (BackgroundLevels[Index].BackgroundTexture)
		return BackgroundLevels[Index].BackgroundTexture;
	else
		return BackgroundLevels[Index].RenderTarget;
}

void AMapBackground::SetBackgroundOverlay(const int32 Level , UTextureRenderTarget2D* NewOverlay )
{
	if (BackgroundLevels.IsValidIndex(Level) && NewOverlay != BackgroundLevels[Level].Overlay)
	{
		BackgroundLevels[Level].Overlay = NewOverlay;
		OnMapBackgroundOverlayChanged.Broadcast(this, Level, NewOverlay);
	}
}

UTextureRenderTarget2D* AMapBackground::GetBackgroundOverlay(const int32 Level ) const
{
	return BackgroundLevels.IsValidIndex(Level) ? BackgroundLevels[Level].Overlay : nullptr;
}

int32 AMapBackground::GetLevelAtHeight(const float WorldZ) const
{
	if (BackgroundLevels.Num() == 0)
		return INDEX_NONE;

	const float ScaledBoxExtentZ = GetAreaBounds()->GetScaledBoxExtent().Z;
	const float MinZ = GetAreaBounds()->GetComponentLocation().Z - ScaledBoxExtentZ;
	const float RelativeZ = WorldZ - MinZ;

	int32 LevelIndex = 0;
	float LevelCeiling = 0;
	for (int32 i = 0; i < BackgroundLevels.Num() && RelativeZ > LevelCeiling; ++i)
	{

		LevelIndex = i;

		LevelCeiling += BackgroundLevels[i].LevelHeight;
	}

	return LevelIndex;
}

UTexture* AMapBackground::GetBackgroundTextureAtHeight(const float WorldZ) const
{
	const int32 LevelIndex = GetLevelAtHeight(WorldZ);
	if (BackgroundLevels.IsValidIndex(LevelIndex))
	{
		if (BackgroundLevels[LevelIndex].BackgroundTexture)
			return BackgroundLevels[LevelIndex].BackgroundTexture;
		else
			return BackgroundLevels[LevelIndex].RenderTarget;
	}
	return nullptr;
}

void AMapBackground::RerenderBackground()
{

	ApplyBackgroundTexture(true);
}

void AMapBackground::CaptureBackground()
{
	RerenderBackground();
}

FVector2D AMapBackground::CorrectUVs(const int32 Level, const FVector2D& InUV) const
{
	if (!BackgroundLevels.IsValidIndex(Level))
		return InUV;

	FVector2D OutUV;
	const FMapBackgroundLevel BackgroundLevel = BackgroundLevels[Level];
	if (BackgroundLevel.BackgroundTexture)
	{

		OutUV.X = InUV.X;
		OutUV.Y = InUV.Y;
	}
	else if (BackgroundLevel.RenderTarget)
	{

		const float RatioUL = BackgroundLevels[Level].SamplingResolution.X / BackgroundLevel.RenderTarget->GetSurfaceWidth();
		const float RatioVL = BackgroundLevels[Level].SamplingResolution.Y / BackgroundLevel.RenderTarget->GetSurfaceHeight();
		OutUV.X = (InUV.X - 0.5f) * RatioUL + 0.5f;
		OutUV.Y = (InUV.Y - 0.5f) * RatioVL + 0.5f;
	}
	else
	{

		OutUV = InUV;
	}

	return OutUV;
}

#if WITH_EDITOR
void AMapBackground::VisualizeLevelsInEditor()
{
	const int32 NumSeparators = FMath::Max(0, BackgroundLevels.Num() - 1);
	if (LevelVisualizers.Num() != NumSeparators)
	{

		for (UBoxComponent* Box : LevelVisualizers)
			Box->DestroyComponent();
		LevelVisualizers.Empty();

		for (int32 i = 0; i < NumSeparators; ++i)
		{
			UBoxComponent* NewBox = NewObject<UBoxComponent>(this);
			NewBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			NewBox->SetupAttachment(GetAreaBounds());
			LevelVisualizers.Add(NewBox);
		}
	}

	const float RelativeBottomZ = -GetAreaBounds()->GetUnscaledBoxExtent().Z;
	float RelativeWorldHeight = 0;
	for (int32 i = 0; i < NumSeparators; ++i)
	{
		RelativeWorldHeight += BackgroundLevels[i].LevelHeight;
		LevelVisualizers[i]->SetRelativeLocation(FVector(0, 0, RelativeBottomZ));
		LevelVisualizers[i]->AddWorldOffset(FVector(0, 0, RelativeWorldHeight));
		LevelVisualizers[i]->SetBoxExtent(GetAreaBounds()->GetUnscaledBoxExtent() * FVector(1, 1, 0));
	}
}
#endif

void AMapBackground::NormalizeScale()
{
	const FVector ActorScale = GetActorScale3D();
	const FVector CompScale = GetAreaBounds()->GetComponentScale();
	if (!(ActorScale - FVector::OneVector).IsNearlyZero())
	{
		const FVector OldUnscaledBoxExtent = GetAreaBounds()->GetUnscaledBoxExtent();
		const FVector NewUnscaledBoxExtent = ActorScale * CompScale * OldUnscaledBoxExtent;
		SetActorScale3D(FVector::OneVector);
		GetAreaBounds()->SetWorldScale3D(FVector::OneVector);
		GetAreaBounds()->SetBoxExtent(NewUnscaledBoxExtent, false);
	}
}

void AMapBackground::InitializeDynamicRenderTargets()
{
	for (FMapBackgroundLevel& BackgroundLevel : BackgroundLevels)
	{
		if (!BackgroundLevel.BackgroundTexture && !BackgroundLevel.RenderTarget)
		{

			const int32 RenderTargetSize = FMath::Max(128, DynamicRenderTargetSize);
			BackgroundLevel.RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize, RenderTargetSize);
		}
	}
}

void AMapBackground::ApplyBackgroundTexture(bool bForceRerender)
{
	NormalizeScale();

	const float ScaledBoxExtentZ = GetAreaBounds()->GetScaledBoxExtent().Z;
	float RelativeHeight = -ScaledBoxExtentZ;
	for (int32 i = 0; i < BackgroundLevels.Num(); ++i)
	{
		FMapBackgroundLevel& BackgroundLevel = BackgroundLevels[i];

		if (BackgroundLevel.BackgroundTexture)
		{

			BackgroundLevel.SamplingResolution.X = BackgroundLevel.BackgroundTexture->GetSurfaceWidth();
			BackgroundLevel.SamplingResolution.Y = BackgroundLevel.BackgroundTexture->GetSurfaceHeight();
		}

		if (BackgroundLevel.RenderTarget && (!BackgroundLevel.BackgroundTexture || bForceRerender))
		{

			if (i != BackgroundLevels.Num() - 1)
				RelativeHeight += BackgroundLevel.LevelHeight;
			else
				RelativeHeight = 2 * ScaledBoxExtentZ;

			GenerateSnapshot(BackgroundLevel.RenderTarget, RelativeHeight);

			OnMapBackgroundRendered.Broadcast(this, i, BackgroundLevel.RenderTarget);

			const float AspectRatio = GetMapAspectRatio();
			const float TexSizeX = BackgroundLevel.RenderTarget->GetSurfaceWidth();
			const float TexSizeY = BackgroundLevel.RenderTarget->GetSurfaceHeight();
			BackgroundLevel.SamplingResolution.X = (AspectRatio >= 1.0f) ? TexSizeX : TexSizeY * AspectRatio;
			BackgroundLevel.SamplingResolution.Y = (AspectRatio > 1.0f) ? TexSizeX / AspectRatio : TexSizeY;
		}
	}

	for (auto KVP : MaterialInstances)
		KVP.Value->SetTextureParameterValue(TEXT("Texture"), GetBackgroundTexture());

	OnMapBackgroundTextureChanged.Broadcast(this);
	OnMapBackgroundAppearanceChanged.Broadcast(this);
}

void AMapBackground::GenerateSnapshot(UTextureRenderTarget2D* RenderTarget, float RelativeHeight)
{
	CaptureComponent2D->TextureTarget = RenderTarget;

	CaptureComponent2D->SetWorldScale3D(FVector(1.0f));

	const FVector ScaledBoxExtent = GetAreaBounds()->GetScaledBoxExtent();
	const float SnapshotRadius = FMath::Max(ScaledBoxExtent.X, ScaledBoxExtent.Y);
	CaptureComponent2D->OrthoWidth = 2.0f * SnapshotRadius;

	CaptureComponent2D->SetRelativeLocation(FVector(0, 0, RelativeHeight));

	CaptureComponent2D->HiddenActors.Empty();
	CaptureComponent2D->HiddenComponents.Empty();

	HiddenActorClasses.Remove(nullptr);
	for (TSubclassOf<AActor> HiddenActorClass : HiddenActorClasses)
	{
		TArray<AActor*> ActorsOfClass;
		UGameplayStatics::GetAllActorsOfClass(this, HiddenActorClass, ActorsOfClass);
		for (AActor* FoundActor : ActorsOfClass)
			CaptureComponent2D->HideActorComponents(FoundActor);
	}

	HiddenActors.Remove(nullptr);
	for (AActor* HiddenActor : HiddenActors)
	{
		CaptureComponent2D->HideActorComponents(HiddenActor);
	}

	FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	UGameViewportClient* GameViewport = WorldContext ? WorldContext->GameViewport : nullptr;
#if UE_ENABLE_DEBUG_DRAWING
	const bool bShouldRenderNavMeshInBuild = bRenderNavigationMesh;
#else
	const bool bShouldRenderNavMeshInBuild = false;
#endif
	if (GameViewport && bShouldRenderNavMeshInBuild)
	{

		const bool OldNavigationShown = GameViewport->EngineShowFlags.Navigation;
		GameViewport->EngineShowFlags.SetNavigation(true);
		NavMeshRenderingComponent->SetHiddenInGame(false);

		CaptureComponent2D->CaptureScene();

		GameViewport->EngineShowFlags.SetNavigation(OldNavigationShown);
		NavMeshRenderingComponent->SetHiddenInGame(true);
	}
	else
	{

		CaptureComponent2D->CaptureScene();
	}
}
