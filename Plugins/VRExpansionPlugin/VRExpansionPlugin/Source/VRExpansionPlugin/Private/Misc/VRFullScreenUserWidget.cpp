

#include "Misc/VRFullScreenUserWidget.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRFullScreenUserWidget)

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/UserInterfaceSettings.h"
#include "GameFramework/WorldSettings.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/Package.h"
#include "HAL/PlatformApplicationMisc.h"

#include "Framework/Application/SlateApplication.h"
#include "Input/HittestGrid.h"
#include "Layout/Visibility.h"
#include "Slate/SceneViewport.h"
#include "Slate/WidgetRenderer.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/SViewport.h"

#include "IXRTrackingSystem.h"
#include "IHeadMountedDisplay.h"

#if WITH_EDITOR
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "SLevelViewport.h"
#endif

#define LOCTEXT_NAMESPACE "VRFullScreenUserWidget"

namespace
{
	const FName NAME_LevelEditorName = "LevelEditor";

	EVisibility ConvertWindowVisibilityToVisibility(EWindowVisibility visibility)
	{
		switch (visibility)
		{
		case EWindowVisibility::Visible:
			return EVisibility::Visible;
		case EWindowVisibility::SelfHitTestInvisible:
			return EVisibility::SelfHitTestInvisible;
		default:
			checkNoEntry();
			return EVisibility::SelfHitTestInvisible;
		}
	}

	namespace VPVRFullScreenUserWidgetPrivate
	{

		class FWorldCleanupListener
		{
		public:

			static FWorldCleanupListener* Get()
			{
				static FWorldCleanupListener Instance;
				return &Instance;
			}

			UE_NONCOPYABLE(FWorldCleanupListener);

			~FWorldCleanupListener()
			{
				FWorldDelegates::OnWorldCleanup.RemoveAll(this);
			}

			void AddWidget(UVRFullScreenUserWidget* InWidget)
			{
				WidgetsToHide.AddUnique(InWidget);
			}

			void RemoveWidget(UVRFullScreenUserWidget* InWidget)
			{
				WidgetsToHide.RemoveSingleSwap(InWidget, EAllowShrinking::No);
			}

		private:

			FWorldCleanupListener()
			{
				FWorldDelegates::OnWorldCleanup.AddRaw(this, &FWorldCleanupListener::OnWorldCleanup);
			}

			void OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources)
			{
				for (auto WeakWidgetIter = WidgetsToHide.CreateIterator(); WeakWidgetIter; ++WeakWidgetIter)
				{
					TWeakObjectPtr<UVRFullScreenUserWidget>& WeakWidget = *WeakWidgetIter;
					if (UVRFullScreenUserWidget* Widget = WeakWidget.Get())
					{
						if (Widget->IsDisplayed()
							&& Widget->GetWidget()
							&& (Widget->GetWidget()->GetWorld() == InWorld))
						{

							WeakWidgetIter.RemoveCurrent();
							Widget->Hide();
						}
					}
					else
					{
						WeakWidgetIter.RemoveCurrent();
					}
				}
			}

		private:

			TArray<TWeakObjectPtr<UVRFullScreenUserWidget>> WidgetsToHide;
		};
	}
}

class FVRWidgetPostProcessHitTester : public ICustomHitTestPath
{
public:
	FVRWidgetPostProcessHitTester(UWorld* InWorld, TSharedPtr<SVirtualWindow> InSlateWindow, TAttribute<float> GetDPIAttribute)
		: World(InWorld)
		, VirtualSlateWindow(InSlateWindow)
		, GetDPIAttribute(MoveTemp(GetDPIAttribute))
		, WidgetDrawSize(FIntPoint::ZeroValue)
		, LastLocalHitLocation(FVector2D::ZeroVector)
	{}

	virtual TArray<FWidgetAndPointer> GetBubblePathAndVirtualCursors(const FGeometry& InGeometry, FVector2D DesktopSpaceCoordinate, bool bIgnoreEnabledStatus) const override
	{

		TArray<FWidgetAndPointer> ArrangedWidgets;
		if (TSharedPtr<SVirtualWindow> SlateWindowPin = VirtualSlateWindow.Pin())
		{

			const float DPI = GetDPIAttribute.Get();
			const FVector2D LocalMouseCoordinate = DPI * InGeometry.AbsoluteToLocal(DesktopSpaceCoordinate);

			constexpr float CursorRadius = 0.f;
			ArrangedWidgets = SlateWindowPin->GetHittestGrid().GetBubblePath(LocalMouseCoordinate, CursorRadius, bIgnoreEnabledStatus);

			const FVirtualPointerPosition VirtualMouseCoordinate(LocalMouseCoordinate, LastLocalHitLocation);
			LastLocalHitLocation = LocalMouseCoordinate;

			for (FWidgetAndPointer& ArrangedWidget : ArrangedWidgets)
			{
				ArrangedWidget.SetPointerPosition(VirtualMouseCoordinate);
			}
		}

		return ArrangedWidgets;
	}

	virtual void ArrangeCustomHitTestChildren(FArrangedChildren& ArrangedChildren) const override
	{

		if (TSharedPtr<SVirtualWindow> SlateWindowPin = VirtualSlateWindow.Pin())
		{
			FGeometry WidgetGeom;
			ArrangedChildren.AddWidget(FArrangedWidget(SlateWindowPin.ToSharedRef(), WidgetGeom.MakeChild(WidgetDrawSize, FSlateLayoutTransform())));
		}
	}

	virtual TOptional<FVirtualPointerPosition> TranslateMouseCoordinateForCustomHitTestChild(const SWidget& ChildWidget, const FGeometry& MyGeometry, const FVector2D ScreenSpaceMouseCoordinate, const FVector2D LastScreenSpaceMouseCoordinate) const override
	{
		return TOptional<FVirtualPointerPosition>();
	}

	void SetWidgetDrawSize(FIntPoint NewWidgetDrawSize)
	{
		WidgetDrawSize = NewWidgetDrawSize;
	}

private:
	TWeakObjectPtr<UWorld> World;
	TWeakPtr<SVirtualWindow> VirtualSlateWindow;
	TAttribute<float> GetDPIAttribute;
	FIntPoint WidgetDrawSize;
	mutable FVector2D LastLocalHitLocation;
};

bool FVRFullScreenUserWidget_Viewport::Display(UWorld* World, UUserWidget* Widget, TAttribute<float> InDPIScale)
{
	const TSharedPtr<SConstraintCanvas> FullScreenWidgetPinned = FullScreenCanvasWidget.Pin();
	if (Widget == nullptr || World == nullptr || FullScreenWidgetPinned.IsValid())
	{
		return false;
	}

	const TSharedRef<SConstraintCanvas> FullScreenCanvas = SNew(SConstraintCanvas);
	FullScreenCanvas->AddSlot()
		.Offset(FMargin(0, 0, 0, 0))
		.Anchors(FAnchors(0, 0, 1, 1))
		.Alignment(FVector2D(0, 0))
		[
			SNew(SDPIScaler)
			.DPIScale(MoveTemp(InDPIScale))
		[
			Widget->TakeWidget()
		]
		];

	UGameViewportClient* ViewportClient = World->GetGameViewport();
	const bool bCanUseGameViewport = ViewportClient && World->IsGameWorld();
	if (bCanUseGameViewport)
	{
		FullScreenCanvasWidget = FullScreenCanvas;
		ViewportClient->AddViewportWidgetContent(FullScreenCanvas);
		return true;
	}

#if WITH_EDITOR

	const TSharedPtr<FSceneViewport> PinnedTargetViewport = EditorTargetViewport.Pin();
	for (FLevelEditorViewportClient* Client : GEditor->GetLevelViewportClients())
	{
		const TSharedPtr<SLevelViewport> LevelViewport = StaticCastSharedPtr<SLevelViewport>(Client->GetEditorViewportWidget());
		if (LevelViewport.IsValid() && LevelViewport->GetSceneViewport() == PinnedTargetViewport)
		{

			LevelViewport->AddOverlayWidget(FullScreenCanvas);
			FullScreenCanvasWidget = FullScreenCanvas;
			OverlayWidgetLevelViewport = LevelViewport;
			return true;
		}

	}
#endif
	return false;
}

void FVRFullScreenUserWidget_Viewport::Hide(UWorld* World)
{
	TSharedPtr<SConstraintCanvas> FullScreenWidgetPinned = FullScreenCanvasWidget.Pin();
	if (FullScreenWidgetPinned.IsValid())
	{

		UGameViewportClient* ViewportClient = World ? World->GetGameViewport() : nullptr;
		if (ViewportClient)
		{
			ViewportClient->RemoveViewportWidgetContent(FullScreenWidgetPinned.ToSharedRef());
		}

#if WITH_EDITOR
		if (const TSharedPtr<SLevelViewport> OverlayWidgetLevelViewportPinned = OverlayWidgetLevelViewport.Pin())
		{
			OverlayWidgetLevelViewportPinned->RemoveOverlayWidget(FullScreenWidgetPinned.ToSharedRef());
		}
		OverlayWidgetLevelViewport.Reset();
#endif

		FullScreenCanvasWidget.Reset();
	}
}

FVRFullScreenUserWidget_PostProcess::FVRFullScreenUserWidget_PostProcess()
	:

	bWidgetDrawSize(false)
	, WidgetDrawSize(FIntPoint(640, 360))
	, bWindowFocusable(true)
	, WindowVisibility(EWindowVisibility::SelfHitTestInvisible)
	, bReceiveHardwareInput(true)
	, RenderTargetBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
	, RenderTargetBlendMode(EWidgetBlendMode::Masked)
	, WidgetRenderTarget(nullptr)

	, WidgetRenderer(nullptr)
	, CurrentWidgetDrawSize(FIntPoint::ZeroValue)
{
	bRenderToTextureOnly = true;
	bDrawToVRPreview = true;
	VRDisplayType = ESpectatorScreenMode::TexturePlusEye;
	PostVRDisplayType = ESpectatorScreenMode::SingleEye;
}

bool FVRFullScreenUserWidget_PostProcess::Display(UWorld* World, UUserWidget* Widget, bool bInRenderToTextureOnly, TAttribute<float> InDPIScale)
{
	TAttribute<float> PostProcessDPIScale = 1.0f;
	bool bOk = CreateRenderer(World, Widget, MoveTemp(PostProcessDPIScale));

	if (bRenderToTextureOnly && IsValid(WidgetRenderTarget) && bDrawToVRPreview)
	{
		IHeadMountedDisplay* HMD = GEngine->XRSystem.IsValid() ? GEngine->XRSystem->GetHMDDevice() : nullptr;
		ISpectatorScreenController* Controller = nullptr;
		if (HMD)
		{
			Controller = HMD->GetSpectatorScreenController();
		}

		if (Controller)
		{
			if (VRDisplayType == ESpectatorScreenMode::TexturePlusEye)
			{
				if (Controller->GetSpectatorScreenMode() != ESpectatorScreenMode::TexturePlusEye)
				{
					Controller->SetSpectatorScreenMode(ESpectatorScreenMode::TexturePlusEye);
				}

				FSpectatorScreenModeTexturePlusEyeLayout Layout;
				Layout.bClearBlack = true;
				Layout.bDrawEyeFirst = true;
				Layout.bUseAlpha = true;
				Layout.EyeRectMin = FVector2D(0.f, 0.f);
				Layout.EyeRectMax = FVector2D(1.f, 1.f);
				Layout.TextureRectMin = FVector2D(0.f, 0.f);
				Layout.TextureRectMax = FVector2D(1.f, 1.f);
				Controller->SetSpectatorScreenModeTexturePlusEyeLayout(Layout);
				Controller->SetSpectatorScreenTexture(WidgetRenderTarget);
			}
			else if (VRDisplayType == ESpectatorScreenMode::Texture)
			{
				if (Controller->GetSpectatorScreenMode() != ESpectatorScreenMode::TexturePlusEye)
				{
					Controller->SetSpectatorScreenMode(ESpectatorScreenMode::Texture);
				}

				Controller->SetSpectatorScreenTexture(WidgetRenderTarget);
			}
		}
	}

	if (!bRenderToTextureOnly)
	{

	}

	return bOk;
}

void FVRFullScreenUserWidget_PostProcess::Hide(UWorld* World)
{
	if (!bRenderToTextureOnly)
	{

	}

	if (bRenderToTextureOnly && bDrawToVRPreview)
	{
		IHeadMountedDisplay* HMD = GEngine->XRSystem.IsValid() ? GEngine->XRSystem->GetHMDDevice() : nullptr;
		ISpectatorScreenController* Controller = nullptr;
		if (HMD)
		{
			Controller = HMD->GetSpectatorScreenController();
		}

		if (Controller)
		{
			if (Controller->GetSpectatorScreenMode() == ESpectatorScreenMode::TexturePlusEye || Controller->GetSpectatorScreenMode() == ESpectatorScreenMode::Texture)
			{
				Controller->SetSpectatorScreenMode(PostVRDisplayType);
				Controller->SetSpectatorScreenTexture(nullptr);
			}
		}
	}

	ReleaseRenderer();
}

void FVRFullScreenUserWidget_PostProcess::Tick(UWorld* World, float DeltaSeconds)
{
	TickRenderer(World, DeltaSeconds);
}

TSharedPtr<SVirtualWindow> FVRFullScreenUserWidget_PostProcess::GetSlateWindow() const
{
	return SlateWindow;
}

bool FVRFullScreenUserWidget_PostProcess::CreateRenderer(UWorld* World, UUserWidget* Widget, TAttribute<float> InDPIScale)
{
	ReleaseRenderer();

	if (World && Widget)
	{
		constexpr bool bApplyGammaCorrection = true;
		WidgetRenderer = new FWidgetRenderer(bApplyGammaCorrection);
		WidgetRenderer->SetIsPrepassNeeded(true);

		checkf(CurrentWidgetDrawSize == FIntPoint::ZeroValue, TEXT("Expected ReleaseRenderer to reset CurrentWidgetDrawSize."));
		SlateWindow = SNew(SVirtualWindow).Size(CurrentWidgetDrawSize);
		SlateWindow->SetIsFocusable(bWindowFocusable);
		SlateWindow->SetVisibility(ConvertWindowVisibilityToVisibility(WindowVisibility));
		SlateWindow->SetContent(

			SNew(SDPIScaler)
			.DPIScale(InDPIScale)
			[
				Widget->TakeWidget()
			]
		);

		RegisterHitTesterWithViewport(World);

		if (!Widget->IsDesignTime() && World->IsGameWorld())
		{
			UGameInstance* GameInstance = World->GetGameInstance();
			UGameViewportClient* GameViewportClient = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;
			if (GameViewportClient)
			{
				SlateWindow->AssignParentWidget(GameViewportClient->GetGameViewportWidget());
			}
		}

		FLinearColor ActualBackgroundColor = RenderTargetBackgroundColor;
		switch (RenderTargetBlendMode)
		{
		case EWidgetBlendMode::Opaque:
			ActualBackgroundColor.A = 1.0f;
			break;
		case EWidgetBlendMode::Masked:
			ActualBackgroundColor.A = 0.0f;
			break;
		}

		checkf(CurrentWidgetDrawSize == FIntPoint::ZeroValue, TEXT("Expected ReleaseRenderer to reset CurrentWidgetDrawSize."));

		WidgetRenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), NAME_None, RF_Transient);
		WidgetRenderTarget->ClearColor = ActualBackgroundColor;
	}

	return WidgetRenderer && WidgetRenderTarget;
}

void FVRFullScreenUserWidget_PostProcess::ReleaseRenderer()
{
	if (WidgetRenderer)
	{
		BeginCleanup(WidgetRenderer);
		WidgetRenderer = nullptr;
	}
	UnRegisterHitTesterWithViewport();

	SlateWindow.Reset();
	WidgetRenderTarget = nullptr;
	CurrentWidgetDrawSize = FIntPoint::ZeroValue;
}

void FVRFullScreenUserWidget_PostProcess::TickRenderer(UWorld* World, float DeltaSeconds)
{
	check(World);
	if (IsValid(WidgetRenderTarget))
	{

		if (bRenderToTextureOnly && bDrawToVRPreview)
		{
			IHeadMountedDisplay* HMD = GEngine->XRSystem.IsValid() ? GEngine->XRSystem->GetHMDDevice() : nullptr;
			ISpectatorScreenController* Controller = nullptr;
			if (HMD)
			{
				Controller = HMD->GetSpectatorScreenController();
			}

			if (Controller)
			{
				if (Controller->GetSpectatorScreenMode() != ESpectatorScreenMode::TexturePlusEye)
				{
					Controller->SetSpectatorScreenMode(ESpectatorScreenMode::TexturePlusEye);
					FSpectatorScreenModeTexturePlusEyeLayout Layout;
					Layout.bClearBlack = true;
					Layout.bDrawEyeFirst = true;
					Layout.bUseAlpha = true;
					Layout.EyeRectMin = FVector2D(0.f, 0.f);
					Layout.EyeRectMax = FVector2D(1.f, 1.f);
					Layout.TextureRectMin = FVector2D(0.f, 0.f);
					Layout.TextureRectMax = FVector2D(1.f, 1.f);
					Controller->SetSpectatorScreenModeTexturePlusEyeLayout(Layout);
					Controller->SetSpectatorScreenTexture(WidgetRenderTarget);
				}
			}
		}

		const float DrawScale = 1.0f;

		const FIntPoint NewCalculatedWidgetSize = CalculateWidgetDrawSize(World);
		if (NewCalculatedWidgetSize != CurrentWidgetDrawSize)
		{
			if (IsTextureSizeValid(NewCalculatedWidgetSize))
			{
				CurrentWidgetDrawSize = NewCalculatedWidgetSize;
				WidgetRenderTarget->InitCustomFormat(CurrentWidgetDrawSize.X, CurrentWidgetDrawSize.Y, PF_B8G8R8A8, false);
				WidgetRenderTarget->UpdateResourceImmediate();
				SlateWindow->Resize(CurrentWidgetDrawSize);
				if (CustomHitTestPath)
				{
					CustomHitTestPath->SetWidgetDrawSize(CurrentWidgetDrawSize);
				}
			}
			else
			{
				Hide(World);
			}
		}

		if (WidgetRenderer && CurrentWidgetDrawSize != FIntPoint::ZeroValue)
		{
			WidgetRenderer->DrawWindow(
				WidgetRenderTarget,
				SlateWindow->GetHittestGrid(),
				SlateWindow.ToSharedRef(),
				DrawScale,
				CurrentWidgetDrawSize,
				DeltaSeconds);
		}
	}
}

FIntPoint FVRFullScreenUserWidget_PostProcess::CalculateWidgetDrawSize(UWorld* World)
{
	if (bWidgetDrawSize)
	{
		return WidgetDrawSize;
	}

	if (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE)
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{

			const float SmallWidgetSize = 16.f;
			FVector2D OutSize = FVector2D(SmallWidgetSize, SmallWidgetSize);
			OutSize = ViewportClient->GetWindow()->GetSizeInScreen(); 
			if (OutSize.X < UE_SMALL_NUMBER)
			{
				OutSize = FVector2D(SmallWidgetSize, SmallWidgetSize);
			}
			return OutSize.IntPoint();
		}

		return FIntPoint::ZeroValue;
	}
#if WITH_EDITOR

	if (const TSharedPtr<FSceneViewport> SharedActiveViewport = EditorTargetViewport.Pin())
	{
		return SharedActiveViewport->GetSize();
	}

#endif

	return FIntPoint::ZeroValue;
}

bool FVRFullScreenUserWidget_PostProcess::IsTextureSizeValid(FIntPoint Size) const
{
	const int32 MaxAllowedDrawSize = GetMax2DTextureDimension();
	return Size.X > 0 && Size.Y > 0 && Size.X <= MaxAllowedDrawSize && Size.Y <= MaxAllowedDrawSize;
}

void FVRFullScreenUserWidget_PostProcess::RegisterHitTesterWithViewport(UWorld* World)
{
	if (!bReceiveHardwareInput && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().RegisterVirtualWindow(SlateWindow.ToSharedRef());
	}
	const TSharedPtr<SViewport> EngineViewportWidget = GetViewport(World);
	if (EngineViewportWidget && bReceiveHardwareInput)
	{
		if (EngineViewportWidget->GetCustomHitTestPath())
		{

		}
		else
		{
			ViewportWidget = EngineViewportWidget;
			CustomHitTestPath = MakeShared<FVRWidgetPostProcessHitTester>(World, SlateWindow, TAttribute<float>::CreateRaw(this, &FVRFullScreenUserWidget_PostProcess::GetDPIScaleForPostProcessHitTester, TWeakObjectPtr<UWorld>(World)));
			CustomHitTestPath->SetWidgetDrawSize(CurrentWidgetDrawSize);
			EngineViewportWidget->SetCustomHitTestPath(CustomHitTestPath);
		}
	}
}

void FVRFullScreenUserWidget_PostProcess::UnRegisterHitTesterWithViewport()
{
	if (SlateWindow.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterVirtualWindow(SlateWindow.ToSharedRef());
	}

	if (TSharedPtr<SViewport> ViewportWidgetPin = ViewportWidget.Pin())
	{
		if (ViewportWidgetPin->GetCustomHitTestPath() == CustomHitTestPath)
		{
			ViewportWidgetPin->SetCustomHitTestPath(nullptr);
		}
	}

	ViewportWidget.Reset();
	CustomHitTestPath.Reset();
}

TSharedPtr<SViewport> FVRFullScreenUserWidget_PostProcess::GetViewport(UWorld* World) const
{
	if (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE)
	{
		return GEngine->GetGameViewportWidget();
	}

#if WITH_EDITOR
	if (const TSharedPtr<FSceneViewport> TargetViewportPin = EditorTargetViewport.Pin())
	{
		return TargetViewportPin->GetViewportWidget().Pin();
	}
#endif

	return nullptr;
}

float FVRFullScreenUserWidget_PostProcess::GetDPIScaleForPostProcessHitTester(TWeakObjectPtr<UWorld> World) const
{
	FSceneViewport* Viewport = nullptr;
	if (ensure(World.IsValid()) && World->IsGameWorld())
	{
		UGameViewportClient* ViewportClient = World->GetGameViewport();		
		Viewport = ViewportClient ? ViewportClient->GetGameViewport() : nullptr;
	}

#if WITH_EDITOR
	const TSharedPtr<FSceneViewport> ViewportPin = EditorTargetViewport.Pin();
	Viewport = Viewport ? Viewport : ViewportPin.Get();
#endif

	const bool bCanScale = Viewport && !Viewport->HasFixedSize();
	if (!bCanScale)
	{
		return 1.f;
	}

	const TSharedPtr<SWindow> ViewportWindow = Viewport->FindWindow();
	return ViewportWindow ? ViewportWindow->GetDPIScaleFactor() : 1.f;
}

UVRFullScreenUserWidget::UVRFullScreenUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentDisplayType(EVRWidgetDisplayType::Inactive)
	, bDisplayRequested(false)
{

}

void UVRFullScreenUserWidget::BeginDestroy()
{
	Hide();
	Super::BeginDestroy();
}

bool UVRFullScreenUserWidget::ShouldDisplay(UWorld* InWorld) const
{
#if UE_SERVER
	return false;
#else
	if (GUsingNullRHI || HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject) || IsRunningDedicatedServer())
	{
		return false;
	}

	return GetDisplayType(InWorld) != EVRWidgetDisplayType::Inactive;
#endif 
}

EVRWidgetDisplayType UVRFullScreenUserWidget::GetDisplayType(UWorld* InWorld) const
{
	if (InWorld)
	{
		if (InWorld->WorldType == EWorldType::Game)
		{
			return GameDisplayType;
		}
#if WITH_EDITOR
		else if (InWorld->WorldType == EWorldType::PIE)
		{
			return PIEDisplayType;
		}
		else if (InWorld->WorldType == EWorldType::Editor)
		{
			return EditorDisplayType;
		}
#endif 
	}
	return EVRWidgetDisplayType::Inactive;
}

bool UVRFullScreenUserWidget::IsDisplayed() const
{
	return CurrentDisplayType != EVRWidgetDisplayType::Inactive;
}

bool UVRFullScreenUserWidget::Display(UWorld* InWorld)
{
	bDisplayRequested = true;
	World = InWorld;

#if WITH_EDITOR
	if (!EditorTargetViewport.IsValid() && !World->IsGameWorld())
	{

			if (FModuleManager::Get().IsModuleLoaded(NAME_LevelEditorName))
			{
				FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(NAME_LevelEditorName);
				const TSharedPtr<SLevelViewport> ActiveLevelViewport = LevelEditorModule.GetFirstActiveLevelViewport();
				EditorTargetViewport = ActiveLevelViewport ? ActiveLevelViewport->GetSharedActiveViewport() : nullptr;
			}

		if (!EditorTargetViewport.IsValid())
		{

				return false;
		}
	}

	SetEditorTargetViewport(EditorTargetViewport);
#endif

	bool bWasAdded = false;
	if (InWorld && WidgetClass && ShouldDisplay(InWorld) && CurrentDisplayType == EVRWidgetDisplayType::Inactive)
	{
		const bool bCreatedWidget = InitWidget();
		if (!bCreatedWidget)
		{

			return false;
		}

		CurrentDisplayType = GetDisplayType(InWorld);

		TAttribute<float> GetDpiScaleAttribute = TAttribute<float>::CreateLambda([WeakThis = TWeakObjectPtr<UVRFullScreenUserWidget>(this)]()
		{
			return WeakThis.IsValid() ? WeakThis->GetViewportDPIScale() : 1.f;

		});

		if (CurrentDisplayType == EVRWidgetDisplayType::Viewport)
		{
			bWasAdded = ViewportDisplayType.Display(InWorld, Widget, MoveTemp(GetDpiScaleAttribute));
		}
		else if (CurrentDisplayType == EVRWidgetDisplayType::PostProcess )
		{
			bWasAdded = PostProcessDisplayType.Display(InWorld, Widget, true, MoveTemp(GetDpiScaleAttribute));
		}

		if (bWasAdded)
		{
			FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &UVRFullScreenUserWidget::OnLevelRemovedFromWorld);
			FWorldDelegates::OnWorldCleanup.AddUObject(this, &UVRFullScreenUserWidget::OnWorldCleanup);
			VPVRFullScreenUserWidgetPrivate::FWorldCleanupListener::Get()->AddWidget(this);

		}
	}

	return bWasAdded;
}

void UVRFullScreenUserWidget::Hide()
{
	bDisplayRequested = false;

	if (CurrentDisplayType != EVRWidgetDisplayType::Inactive)
	{
		ReleaseWidget();

		if (CurrentDisplayType == EVRWidgetDisplayType::Viewport)
		{
			ViewportDisplayType.Hide(World.Get());
		}
		else if (CurrentDisplayType == EVRWidgetDisplayType::PostProcess )
		{
			PostProcessDisplayType.Hide(World.Get());
		}
		CurrentDisplayType = EVRWidgetDisplayType::Inactive;
	}

	FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
	VPVRFullScreenUserWidgetPrivate::FWorldCleanupListener::Get()->RemoveWidget(this);
	World.Reset();
}

void UVRFullScreenUserWidget::Tick(float DeltaSeconds)
{
	if (CurrentDisplayType != EVRWidgetDisplayType::Inactive)
	{
		UWorld* CurrentWorld = World.Get();
		if (CurrentWorld == nullptr)
		{
			Hide();
		}
		else if (CurrentDisplayType == EVRWidgetDisplayType::PostProcess )
		{
			PostProcessDisplayType.Tick(CurrentWorld, DeltaSeconds);
		}
	}
}

void UVRFullScreenUserWidget::SetDisplayTypes(EVRWidgetDisplayType InEditorDisplayType, EVRWidgetDisplayType InGameDisplayType, EVRWidgetDisplayType InPIEDisplayType)
{
	EditorDisplayType = InEditorDisplayType;
	GameDisplayType = InGameDisplayType;
	PIEDisplayType = InPIEDisplayType;
}

void UVRFullScreenUserWidget::SetOverrideWidget(UUserWidget* InWidget)
{
	if (ensureMsgf(!IsDisplayed(), TEXT("For simplicity of API you can only override the widget before displaying.")))
	{
		Widget = InWidget;
	}
}

#if WITH_EDITOR
void UVRFullScreenUserWidget::SetEditorTargetViewport(TWeakPtr<FSceneViewport> InTargetViewport)
{
	EditorTargetViewport = InTargetViewport;
	ViewportDisplayType.EditorTargetViewport = InTargetViewport;
	PostProcessDisplayType.EditorTargetViewport = InTargetViewport;
}

void UVRFullScreenUserWidget::ResetEditorTargetViewport()
{
	EditorTargetViewport.Reset();
	ViewportDisplayType.EditorTargetViewport.Reset();
	PostProcessDisplayType.EditorTargetViewport.Reset();
}
#endif

bool UVRFullScreenUserWidget::InitWidget()
{
	const bool bCanCreate = !Widget && WidgetClass && ensure(World.Get()) && FSlateApplication::IsInitialized();
	if (!bCanCreate)
	{
		return false;
	}

	Widget = CreateWidget(World.Get(), WidgetClass);

		if (Widget)
		{
			Widget->SetFlags(RF_Transient);
		}

	return Widget != nullptr;
}

void UVRFullScreenUserWidget::ReleaseWidget()
{
	Widget = nullptr;
}

void UVRFullScreenUserWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{

	if (InLevel == nullptr && InWorld && InWorld == World.Get())
	{
		Hide();
	}
}

void UVRFullScreenUserWidget::OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources)
{

	if (IsDisplayed() && World == InWorld)
	{
		Hide();
	}
}

FVector2D UVRFullScreenUserWidget::FindSceneViewportSize()
{
	ensure(World.IsValid());

	const UWorld* CurrentWorld = World.Get();
	const bool bIsPlayWorld = CurrentWorld && (CurrentWorld->WorldType == EWorldType::Game || CurrentWorld->WorldType == EWorldType::PIE);
	if (bIsPlayWorld)
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			FVector2D OutSize;
			ViewportClient->GetViewportSize(OutSize);
			return OutSize;
		}
	}

#if WITH_EDITOR

	if (const TSharedPtr<FSceneViewport> TargetViewportPin = EditorTargetViewport.Pin())
	{
		return TargetViewportPin->GetSize();
	}
#endif

	ensureMsgf(false, TEXT(
		"FindSceneViewportSize failed. Likely Hide() was called (making World = nullptr) or EditorTargetViewport "
		"reset externally (possibly as part of Hide()). After Hide() is called all widget code should stop calling "
		"FindSceneViewportSize. Investigate whether something was not cleaned up correctly!"
	)
	);
	return FVector2d::ZeroVector;
}

float UVRFullScreenUserWidget::GetViewportDPIScale()
{
	float UIScale = 1.0f;
	float PlatformScale = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(10.0f, 10.0f);

	UWorld* CurrentWorld = World.Get();
	if ((CurrentDisplayType == EVRWidgetDisplayType::Viewport) && CurrentWorld && (CurrentWorld->WorldType == EWorldType::Game || CurrentWorld->WorldType == EWorldType::PIE))
	{

		UIScale = PlatformScale;
	}
	else
	{

		const FIntPoint ViewportSize = FindSceneViewportSize().IntPoint();
		UIScale = GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(ViewportSize);
	}

	return UIScale;
}

#if WITH_EDITOR

void UVRFullScreenUserWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FProperty* Property = PropertyChangedEvent.MemberProperty;

	if (Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		static FName NAME_WidgetClass = GET_MEMBER_NAME_CHECKED(UVRFullScreenUserWidget, WidgetClass);
		static FName NAME_EditorDisplayType = GET_MEMBER_NAME_CHECKED(UVRFullScreenUserWidget, EditorDisplayType);

		static FName NAME_WidgetDrawSize = GET_MEMBER_NAME_CHECKED(FVRFullScreenUserWidget_PostProcess, WidgetDrawSize);
		static FName NAME_WindowFocusable = GET_MEMBER_NAME_CHECKED(FVRFullScreenUserWidget_PostProcess, bWindowFocusable);
		static FName NAME_WindowVisibility = GET_MEMBER_NAME_CHECKED(FVRFullScreenUserWidget_PostProcess, WindowVisibility);
		static FName NAME_ReceiveHardwareInput = GET_MEMBER_NAME_CHECKED(FVRFullScreenUserWidget_PostProcess, bReceiveHardwareInput);
		static FName NAME_RenderTargetBackgroundColor = GET_MEMBER_NAME_CHECKED(FVRFullScreenUserWidget_PostProcess, RenderTargetBackgroundColor);
		static FName NAME_RenderTargetBlendMode = GET_MEMBER_NAME_CHECKED(FVRFullScreenUserWidget_PostProcess, RenderTargetBlendMode);

		static FName NAME_DrawToVRPreview = GET_MEMBER_NAME_CHECKED(FVRFullScreenUserWidget_PostProcess, bDrawToVRPreview);
		static FName NAME_VRDisplayType = GET_MEMBER_NAME_CHECKED(FVRFullScreenUserWidget_PostProcess, VRDisplayType);
		static FName NAME_PostVRDisplayType = GET_MEMBER_NAME_CHECKED(FVRFullScreenUserWidget_PostProcess, PostVRDisplayType);

		if (Property->GetFName() == NAME_WidgetClass
			|| Property->GetFName() == NAME_EditorDisplayType

			|| Property->GetFName() == NAME_WidgetDrawSize
			|| Property->GetFName() == NAME_WindowFocusable
			|| Property->GetFName() == NAME_WindowVisibility
			|| Property->GetFName() == NAME_ReceiveHardwareInput
			|| Property->GetFName() == NAME_RenderTargetBackgroundColor
			|| Property->GetFName() == NAME_RenderTargetBlendMode
			|| Property->GetFName() == NAME_DrawToVRPreview
			|| Property->GetFName() == NAME_VRDisplayType
			|| Property->GetFName() == NAME_PostVRDisplayType)

		{
			bool bWasRequestedDisplay = bDisplayRequested;
			UWorld* CurrentWorld = World.Get();
			Hide();
			if (bWasRequestedDisplay && CurrentWorld)
			{
				Display(CurrentWorld);
			}
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

AVRFullScreenUserWidgetActor::AVRFullScreenUserWidgetActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
#if WITH_EDITOR
	, bEditorDisplayRequested(false)
#endif 
{
	ScreenUserWidget = CreateDefaultSubobject<UVRFullScreenUserWidget>(TEXT("ScreenUserWidget"));

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bAllowTickBeforeBeginPlay = false;
	SetActorTickEnabled(false);

	bShowOnInit = false;
}

void AVRFullScreenUserWidgetActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

#if WITH_EDITOR
	bEditorDisplayRequested = true;
#endif 
}

void AVRFullScreenUserWidgetActor::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	bEditorDisplayRequested = true;
#endif 
}

void AVRFullScreenUserWidgetActor::PostActorCreated()
{
	Super::PostActorCreated();

#if WITH_EDITOR
	bEditorDisplayRequested = true;
#endif 
}

void AVRFullScreenUserWidgetActor::Destroyed()
{
	if (ScreenUserWidget)
	{
		ScreenUserWidget->Hide();
	}
	Super::Destroyed();
}

void AVRFullScreenUserWidgetActor::BeginPlay()
{
	if (ScreenUserWidget && bShowOnInit)
	{
		RequestGameDisplay();
	}

	Super::BeginPlay();
}

void AVRFullScreenUserWidgetActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (ScreenUserWidget)
	{
		UWorld* ActorWorld = GetWorld();
		if (ActorWorld && (ActorWorld->WorldType == EWorldType::Game || ActorWorld->WorldType == EWorldType::PIE))
		{
			ScreenUserWidget->Hide();
		}
	}
}

void AVRFullScreenUserWidgetActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if WITH_EDITOR
	if (bEditorDisplayRequested)
	{
		bEditorDisplayRequested = false;
		RequestEditorDisplay();
	}
#endif 

	if (ScreenUserWidget && ScreenUserWidget->IsDisplayRequested())
	{
		ScreenUserWidget->Tick(DeltaSeconds);
	}
}

void AVRFullScreenUserWidgetActor::RequestEditorDisplay()
{
#if WITH_EDITOR
	UWorld* ActorWorld = GetWorld();
	if (ScreenUserWidget && ActorWorld && ActorWorld->WorldType == EWorldType::Editor)
	{
		ScreenUserWidget->Display(ActorWorld);
	}
#endif 
}

void AVRFullScreenUserWidgetActor::RequestGameDisplay()
{
	UWorld* ActorWorld = GetWorld();
	if (ScreenUserWidget && ActorWorld && (ActorWorld->WorldType == EWorldType::Game || ActorWorld->WorldType == EWorldType::PIE))
	{
		ScreenUserWidget->Display(ActorWorld);
		SetActorTickEnabled(true);
	}
}

void AVRFullScreenUserWidgetActor::SetWidgetVisible(bool bIsVisible)
{
	if (ScreenUserWidget)
	{
 		if (!bIsVisible)
		{
			ScreenUserWidget->Hide();
			SetActorTickEnabled(false);
		}
		else
		{
			RequestGameDisplay();
		}
	}
}

UVRFullScreenUserWidget* AVRFullScreenUserWidgetActor::GetPreviewWidgetComp()
{
	return ScreenUserWidget;
}

UUserWidget* AVRFullScreenUserWidgetActor::GetWidget()
{
	if (ScreenUserWidget)
	{
		return ScreenUserWidget->GetWidget();
	}

	return nullptr;
}

UTextureRenderTarget2D* AVRFullScreenUserWidgetActor::GetPostProcessRenderTarget()
{
	if (ScreenUserWidget)
	{
		return ScreenUserWidget->GetPostProcessRenderTarget();
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
