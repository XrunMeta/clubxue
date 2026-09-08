

#include "MapIconComponent.h"
#include "MinimapPluginPrivatePCH.h"
#include "MapTrackerComponent.h"
#include "MapFunctionLibrary.h"

UMapIconComponent::UMapIconComponent()
{

	SetHiddenInGame(true);
	PrimaryComponentTick.bCanEverTick = true;

	SetRelativeLocation(FVector(0, 0, 256));

	static ConstructorHelpers::FObjectFinder<UTexture2D> DefaultIcon(TEXT("/MinimapPlugin/Textures/Icons/T_Icon_Placeholder"));
	IconTexture = DefaultIcon.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> DefaultEdgeIcon(TEXT("/MinimapPlugin/Textures/Icons/T_Icon_ObjectiveArrow"));
	ObjectiveArrowTexture = DefaultEdgeIcon.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterial_UMG(TEXT("/MinimapPlugin/Materials/Icons/M_UMG_MapIcon"));
	IconMaterial_UMG = DefaultMaterial_UMG.Object;
	ObjectiveArrowMaterial_UMG = DefaultMaterial_UMG.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterial_Canvas(TEXT("/MinimapPlugin/Materials/Icons/M_Canvas_MapIcon"));
	IconMaterial_Canvas = DefaultMaterial_Canvas.Object;
	ObjectiveArrowMaterial_Canvas = DefaultMaterial_Canvas.Object;
}

#if WITH_EDITOR
void UMapIconComponent::PostInitProperties()
{
	Super::PostInitProperties();
	RefreshPreviewSprite();
}

void UMapIconComponent::PostLoad()
{
	Super::PostLoad();
	RefreshPreviewSprite();
}

void UMapIconComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshPreviewSprite();
}
#endif

void UMapIconComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		SetComponentTickEnabled(false);
		return;
	}

	UMapTrackerComponent* Tracker = UMapFunctionLibrary::GetMapTracker(this);
	if (Tracker)
		Tracker->RegisterMapIcon(this);

	InitialIconMaterial_UMG = IconMaterial_UMG;
	InitialIconMaterial_Canvas = IconMaterial_Canvas;

	MaterialEffectStartTime = GetWorld()->GetTimeSeconds();

	SetComponentTickEnabled(Tracker && bHideOwnerInsideFog);
}

void UMapIconComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
		return;

	UMapTrackerComponent* Tracker = UMapFunctionLibrary::GetMapTracker(this);
	if (Tracker && Tracker->HasMapFog())
	{
		const FVector Location = GetComponentLocation();
		bool bIsInsideFog;
		GetOwner()->SetActorHiddenInGame(Tracker->GetFogRevealedFactor(Location, IconFogInteraction == EIconFogInteraction::OnlyRenderWhenRevealing, bIsInsideFog) < IconFogRevealThreshold && bIsInsideFog);
	}
}

void UMapIconComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
		return;

	UMapTrackerComponent* Tracker = UMapFunctionLibrary::GetMapTracker(this);
	if (Tracker)
		Tracker->UnregisterMapIcon(this);

	UnmarkRenderedFromAllViews();

	OnIconDestroyed.Broadcast(this);
}

void UMapIconComponent::SetIconMaterialForUMG(UMaterialInterface* NewMaterial)
{
	if (NewMaterial == IconMaterial_UMG)
		return;

	IconMaterialInstances_UMG.Empty();
	OnIconMaterialInstancesChanged.Broadcast(this);

	IconMaterial_UMG = NewMaterial;
	OnIconMaterialChanged.Broadcast(this);
}

UMaterialInterface* UMapIconComponent::GetIconMaterialForUMG() const
{
	return IconMaterial_UMG;
}

UMaterialInterface* UMapIconComponent::GetObjectiveArrowMaterialForUMG() const
{
	return ObjectiveArrowMaterial_UMG;
}

void UMapIconComponent::ResetIconMaterialForUMG()
{
	SetIconMaterialForUMG(InitialIconMaterial_UMG);
}

void UMapIconComponent::GetIconMaterialInstancesForUMG(TArray<UMaterialInstanceDynamic*>& MaterialInstances)
{
	IconMaterialInstances_UMG.GenerateValueArray(MaterialInstances);
}

void UMapIconComponent::RegisterMaterialInstanceFromUMG(UUserWidget* IconWidget, UMaterialInstanceDynamic* MatInst)
{
	IconMaterialInstances_UMG.Remove(IconWidget);
	IconMaterialInstances_UMG.Add(IconWidget, MatInst);
	OnIconMaterialInstancesChanged.Broadcast(this);
}

void UMapIconComponent::SetIconMaterialForCanvas(UMaterialInterface* NewMaterial)
{
	if (NewMaterial == IconMaterial_Canvas)
		return;

	IconMaterialInstances_Canvas.Empty();
	OnIconMaterialInstancesChanged.Broadcast(this);

	IconMaterial_Canvas = NewMaterial;
	OnIconMaterialChanged.Broadcast(this);

	MaterialEffectStartTime = GetWorld()->GetTimeSeconds();
}

UMaterialInterface* UMapIconComponent::GetIconMaterialForCanvas() const
{
	return IconMaterial_Canvas;
}

UMaterialInterface* UMapIconComponent::GetObjectiveArrowMaterialForCanvas() const
{
	return ObjectiveArrowMaterial_Canvas;
}

void UMapIconComponent::ResetIconMaterialForCanvas()
{
	SetIconMaterialForCanvas(InitialIconMaterial_Canvas);
}

void UMapIconComponent::GetIconMaterialInstancesForCanvas(TArray<UMaterialInstanceDynamic*>& MaterialInstances)
{
	IconMaterialInstances_Canvas.GenerateValueArray(MaterialInstances);
}

void UMapIconComponent::SetIconTexture(UTexture2D* NewIcon)
{
	IconTexture = NewIcon;
	OnIconAppearanceChanged.Broadcast(this);
}

UTexture2D* UMapIconComponent::GetIconTexture() const
{
	return IconTexture;
}

void UMapIconComponent::SetIconTooltipText(FName NewIconName)
{
	IconTooltipText = NewIconName;
	OnIconAppearanceChanged.Broadcast(this);
}

FName UMapIconComponent::GetIconTooltipText() const
{
	return IconTooltipText;
}

void UMapIconComponent::SetIconVisible(const bool bNewVisible)
{
	if (bNewVisible == bIconVisible)
		return;
	bIconVisible = bNewVisible;
	OnIconAppearanceChanged.Broadcast(this);

	if (!bNewVisible)
		UnmarkRenderedFromAllViews();
}

bool UMapIconComponent::IsIconVisible() const
{
	return bIconVisible;
}

void UMapIconComponent::SetIconInteractable(const bool bNewInteractable)
{
	if (bNewInteractable == bIconInteractable)
		return;
	bIconInteractable = bNewInteractable;
	OnIconAppearanceChanged.Broadcast(this);
}

bool UMapIconComponent::IsIconInteractable() const
{
	return bIconInteractable;
}

void UMapIconComponent::SetIconRotates(const bool bNewRotates)
{
	bIconRotates = bNewRotates;
	OnIconAppearanceChanged.Broadcast(this);
}

bool UMapIconComponent::DoesIconRotate() const
{
	return bIconRotates;
}

void UMapIconComponent::SetIconSize(const float NewIconSize, const EIconSizeUnit NewIconSizeUnit)
{
	IconSize = NewIconSize;
	IconSizeUnit = NewIconSizeUnit;
	OnIconAppearanceChanged.Broadcast(this);
}

float UMapIconComponent::GetIconSize() const
{
	return IconSize;
}

EIconSizeUnit UMapIconComponent::GetIconSizeUnit() const
{
	return IconSizeUnit;
}

void UMapIconComponent::SetIconDrawColor(const FLinearColor& NewDrawColor)
{
	IconDrawColor = NewDrawColor;
	OnIconAppearanceChanged.Broadcast(this);
}

FLinearColor UMapIconComponent::GetIconDrawColor() const
{
	return IconDrawColor;
}

void UMapIconComponent::SetIconZOrder(const int32 NewZOrder)
{
	IconZOrder = NewZOrder;
	OnIconAppearanceChanged.Broadcast(this);
}

int32 UMapIconComponent::GetIconZOrder() const
{
	return IconZOrder;
}

void UMapIconComponent::SetObjectiveArrowEnabled(const bool bNewObjectiveArrowEnabled)
{
	bObjectiveArrowEnabled = bNewObjectiveArrowEnabled;
	OnIconAppearanceChanged.Broadcast(this);
}

bool UMapIconComponent::IsObjectiveArrowEnabled() const
{
	return bObjectiveArrowEnabled;
}

void UMapIconComponent::SetObjectiveArrowTexture(UTexture2D* NewTexture)
{
	ObjectiveArrowTexture = NewTexture;
	OnIconAppearanceChanged.Broadcast(this);
}

UTexture2D* UMapIconComponent::GetObjectiveArrowTexture() const
{
	return ObjectiveArrowTexture;
}

void UMapIconComponent::SetObjectiveArrowRotates(const bool bNewRotates)
{
	bObjectiveArrowRotates = bNewRotates;
	OnIconAppearanceChanged.Broadcast(this);
}

bool UMapIconComponent::DoesObjectiveArrowRotate() const
{
	return bObjectiveArrowRotates;
}

void UMapIconComponent::SetObjectiveArrowSize(const float NewObjectiveArrowSize)
{
	ObjectiveArrowSize = NewObjectiveArrowSize;
	OnIconAppearanceChanged.Broadcast(this);
}

float UMapIconComponent::GetObjectiveArrowSize() const
{
	return ObjectiveArrowSize;
}

void UMapIconComponent::SetIconBackgroundInteraction(const EIconBackgroundInteraction NewBackgroundInteraction)
{
	IconBackgroundInteraction = NewBackgroundInteraction;
	OnIconAppearanceChanged.Broadcast(this);
}

EIconBackgroundInteraction UMapIconComponent::GetIconBackgroundInteraction() const
{
	return IconBackgroundInteraction;
}

void UMapIconComponent::SetIconFogInteraction(const EIconFogInteraction NewFogInteraction)
{
	IconFogInteraction = NewFogInteraction;
	OnIconAppearanceChanged.Broadcast(this);
}

EIconFogInteraction UMapIconComponent::GetIconFogInteraction() const
{
	return IconFogInteraction;
}

void UMapIconComponent::SetIconFogRevealThreshold(const float NewFogRevealThreshold)
{
	IconFogRevealThreshold = NewFogRevealThreshold;
	OnIconAppearanceChanged.Broadcast(this);
}

float UMapIconComponent::GetIconFogRevealThreshold() const
{
	return IconFogRevealThreshold;
}

UMaterialInstanceDynamic* UMapIconComponent::GetIconMaterialInstanceForCanvas(UMapRendererComponent* Renderer)
{
	if (!Renderer || !IconMaterial_Canvas)
		return nullptr;
	if (!IconMaterialInstances_Canvas.Contains(Renderer))
	{

		UMaterialInstanceDynamic* NewInst = UMaterialInstanceDynamic::Create(IconMaterial_Canvas, this);
		IconMaterialInstances_Canvas.Add(Renderer, NewInst);

		OnIconMaterialInstancesChanged.Broadcast(this);
	}

	UMaterialInstanceDynamic* MatInst = IconMaterialInstances_Canvas[Renderer];
	MatInst->SetScalarParameterValue(TEXT("Time"), GetWorld()->GetTimeSeconds() - MaterialEffectStartTime);
	return MatInst;
}

UMaterialInstanceDynamic* UMapIconComponent::GetObjectiveArrowMaterialInstanceForCanvas(UMapRendererComponent* Renderer)
{
	if (!Renderer)
		return nullptr;

	if (!ObjectiveArrowMaterial_Canvas)
		return GetIconMaterialInstanceForCanvas(Renderer);

	if (!ObjectiveArrowMaterialInstances_Canvas.Contains(Renderer))
	{

		UMaterialInstanceDynamic* NewInst = UMaterialInstanceDynamic::Create(ObjectiveArrowMaterial_Canvas, this);
		ObjectiveArrowMaterialInstances_Canvas.Add(Renderer, NewInst);

		OnIconMaterialInstancesChanged.Broadcast(this);
	}

	UMaterialInstanceDynamic* MatInst = ObjectiveArrowMaterialInstances_Canvas[Renderer];
	MatInst->SetScalarParameterValue(TEXT("Time"), GetWorld()->GetTimeSeconds() - MaterialEffectStartTime);
	return MatInst;
}

bool UMapIconComponent::MarkRenderedInView(UMapViewComponent* View, const bool bNewIsRendered)
{
	if (!IsRenderedPerView.Contains(View))
	{

		IsRenderedPerView.Add(View, bNewIsRendered);
	}
	else if (IsRenderedPerView[View] != bNewIsRendered)
	{

		IsRenderedPerView[View] = bNewIsRendered;
	}
	else
	{

		return false;
	}

	if (bNewIsRendered)
		OnIconEnteredView.Broadcast(this, View);
	else
		OnIconLeftView.Broadcast(this, View);
	return true;
}

bool UMapIconComponent::IsRenderedInView(UMapViewComponent* View) const
{
	return IsRenderedPerView.Contains(View) ? IsRenderedPerView[View] : false;
}

void UMapIconComponent::ReceiveHoverStart()
{
	if (!bMouseOverStarted)
	{

		OnIconHoverStart.Broadcast(this);
		bMouseOverStarted = true;
	}
}

void UMapIconComponent::ReceiveHoverEnd()
{
	if (bMouseOverStarted)
	{

		OnIconHoverEnd.Broadcast(this);
		bMouseOverStarted = false;
	}
}

void UMapIconComponent::ReceiveClicked(const bool bIsLeftMouseButton)
{

	OnIconClicked.Broadcast(this, bIsLeftMouseButton);
}

void UMapIconComponent::RefreshPreviewSprite()
{
	if (IconTexture)
	{

		const float IconTextureSize = FMath::Max(IconTexture->GetSurfaceWidth(), IconTexture->GetSurfaceHeight());
		if (IconTextureSize > 0)
			SetRelativeScale3D(FVector(160.0f / IconTextureSize));
		else
			SetRelativeScale3D(FVector(1, 1, 1));
		SetSprite(IconTexture);
	}
}

void UMapIconComponent::UnmarkRenderedFromAllViews()
{

	TArray<UMapViewComponent*> Views;
	IsRenderedPerView.GetKeys(Views);
	for (UMapViewComponent* View : Views)
		MarkRenderedInView(View, false);
}

