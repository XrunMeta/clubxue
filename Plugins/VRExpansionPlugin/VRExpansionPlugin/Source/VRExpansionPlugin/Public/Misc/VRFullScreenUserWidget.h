

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Info.h"
#include "Components/WidgetComponent.h"
#include "ISpectatorScreenController.h"
#include "Templates/NonNullPointer.h"

#include "VRFullScreenUserWidget.generated.h"

class FSceneViewport;
class FWidgetRenderer;
class FVRWidgetPostProcessHitTester;
class SConstraintCanvas;
class SVirtualWindow;
class SViewport;
class SWidget;
class ULevel;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPostProcessComponent;
class UTextureRenderTarget2D;
class UWorld;

#if WITH_EDITOR
class SLevelViewport;
#endif

UENUM(BlueprintType)
enum class EVRWidgetDisplayType : uint8
{

	Inactive,

	Viewport,

	PostProcess,

};

USTRUCT()
struct FVRFullScreenUserWidget_Viewport
{
	GENERATED_BODY()

public:

	bool Display(UWorld* World, UUserWidget* Widget, TAttribute<float> InDPIScale);
	void Hide(UWorld* World);

#if WITH_EDITOR

	TWeakPtr<FSceneViewport> EditorTargetViewport;
#endif

private:

	TWeakPtr<SConstraintCanvas> FullScreenCanvasWidget;

#if WITH_EDITOR

	TWeakPtr<SLevelViewport> OverlayWidgetLevelViewport;
#endif
};

USTRUCT()
struct FVRFullScreenUserWidget_PostProcess
{
	GENERATED_BODY()

	FVRFullScreenUserWidget_PostProcess();
	void SetCustomPostProcessSettingsSource(TWeakObjectPtr<UObject> InCustomPostProcessSettingsSource);
	bool Display(UWorld* World, UUserWidget* Widget, bool bInRenderToTextureOnly, TAttribute<float> InDPIScale);
	void Hide(UWorld* World);
	void Tick(UWorld* World, float DeltaSeconds);

	TSharedPtr<SVirtualWindow> VREXPANSIONPLUGIN_API GetSlateWindow() const;

private:

	bool CreateRenderer(UWorld* World, UUserWidget* Widget, TAttribute<float> InDPIScale);
	void ReleaseRenderer();
	void TickRenderer(UWorld* World, float DeltaSeconds);

	FIntPoint CalculateWidgetDrawSize(UWorld* World);
	bool IsTextureSizeValid(FIntPoint Size) const;

	void RegisterHitTesterWithViewport(UWorld* World);
	void UnRegisterHitTesterWithViewport();

	TSharedPtr<SViewport> GetViewport(UWorld* World) const;
	float GetDPIScaleForPostProcessHitTester(TWeakObjectPtr<UWorld> World) const;
	FPostProcessSettings* GetPostProcessSettings() const;

public:

	UPROPERTY(EditAnywhere, Category = PostProcess, meta=(InlineEditConditionToggle))
	bool bWidgetDrawSize;

	UPROPERTY(EditAnywhere, Category = PostProcess, meta=(EditCondition= bWidgetDrawSize))
	FIntPoint WidgetDrawSize;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
	bool bWindowFocusable;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
	EWindowVisibility WindowVisibility;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category= PostProcess)
	bool bReceiveHardwareInput;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
	FLinearColor RenderTargetBackgroundColor;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
	EWidgetBlendMode RenderTargetBlendMode;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> WidgetRenderTarget;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
		bool bRenderToTextureOnly;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = PostProcess)
		bool bDrawToVRPreview;

	UPROPERTY(EditAnywhere, Category = "User Interface")
		ESpectatorScreenMode VRDisplayType;

	UPROPERTY(EditAnywhere, Category = "User Interface")
		ESpectatorScreenMode PostVRDisplayType;

#if WITH_EDITOR

	TWeakPtr<FSceneViewport> EditorTargetViewport;
#endif
private:

	TSharedPtr<SVirtualWindow> SlateWindow;

	TWeakPtr<SViewport> ViewportWidget;

	FWidgetRenderer* WidgetRenderer;

	FIntPoint CurrentWidgetDrawSize;

	TSharedPtr<FVRWidgetPostProcessHitTester> CustomHitTestPath;

};

UCLASS(BlueprintType, meta=(ShowOnlyInnerProperties))
class VREXPANSIONPLUGIN_API UVRFullScreenUserWidget : public UObject
{
	GENERATED_BODY()

public:
	UVRFullScreenUserWidget(const FObjectInitializer& ObjectInitializer);

	virtual void BeginDestroy() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	bool ShouldDisplay(UWorld* World) const;
	EVRWidgetDisplayType GetDisplayType(UWorld* World) const;
	bool IsDisplayed() const;

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetComp")
	bool IsDisplayRequested()
	{
		return bDisplayRequested;
	}

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetComp")
	virtual bool Display(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetComp")
	virtual void Hide();

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetComp")
		void SetIsHidden(bool bNewHidden)
	{
		if (bNewHidden)
		{
			Hide();
		}
		else
		{
			if (World.IsValid())
			{
				Display(World.Get());
			}
			else
			{
				UWorld* myWorld = this->GetWorld();
				if (myWorld)
				{
					Display(myWorld);
				}
			}
		}
	}

	virtual void Tick(float DeltaTime);

	void SetDisplayTypes(EVRWidgetDisplayType InEditorDisplayType, EVRWidgetDisplayType InGameDisplayType, EVRWidgetDisplayType InPIEDisplayType);
	void SetOverrideWidget(UUserWidget* InWidget);

#if WITH_EDITOR

	void SetEditorTargetViewport(TWeakPtr<FSceneViewport> InTargetViewport);

	void ResetEditorTargetViewport();
#endif

protected:
	bool InitWidget();
	void ReleaseWidget();

	FVector2D FindSceneViewportSize();
	float GetViewportDPIScale();

private:
	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);
	void OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources);

public:

	UPROPERTY(EditAnywhere, Category = "User Interface")
		EVRWidgetDisplayType EditorDisplayType;

	UPROPERTY(EditAnywhere, Category = "User Interface")
		EVRWidgetDisplayType GameDisplayType;

	UPROPERTY(EditAnywhere, Category = "User Interface", meta = (DisplayName = "PIE Display Type"))
		EVRWidgetDisplayType PIEDisplayType;

	UPROPERTY(EditAnywhere, Category = "Viewport", meta = (ShowOnlyInnerProperties))
		FVRFullScreenUserWidget_Viewport ViewportDisplayType;

	UPROPERTY(EditAnywhere, Category = "User Interface")
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, Category = "Post Process", meta = (ShowOnlyInnerProperties))
	FVRFullScreenUserWidget_PostProcess PostProcessDisplayType;

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetComp")
	UUserWidget* GetWidget() const { return Widget; };

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetComp")
		UTextureRenderTarget2D* GetPostProcessRenderTarget() const { return PostProcessDisplayType.WidgetRenderTarget; };

private:

	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<UUserWidget> Widget;

	TWeakObjectPtr<UWorld> World;

	EVRWidgetDisplayType CurrentDisplayType;

	bool bDisplayRequested;

#if WITH_EDITOR

	TWeakPtr<FSceneViewport> EditorTargetViewport;
#endif
};

UCLASS(Blueprintable, ClassGroup = "UserInterface", HideCategories = (Actor, Input, Movement, Collision, Rendering, "Utilities|Transformation", LOD), ShowCategories = ("Input|MouseInput", "Input|TouchInput"))
class VREXPANSIONPLUGIN_API AVRFullScreenUserWidgetActor : public AInfo
{
	GENERATED_BODY()

public:
	AVRFullScreenUserWidgetActor(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual void PostLoad() override;
	virtual void PostActorCreated() override;
	virtual void Destroyed() override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "User Interface")
		bool bShowOnInit;

	UPROPERTY(VisibleAnywhere, Instanced, NoClear, Category = "User Interface", meta = (ShowOnlyInnerProperties))
		TObjectPtr<UVRFullScreenUserWidget> ScreenUserWidget;

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetActor")
		UVRFullScreenUserWidget* GetPreviewWidgetComp();

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetActor")
	void SetPIEDisplayType(EVRWidgetDisplayType NewDisplayType)
	{
		if (IsValid(ScreenUserWidget))
		{
			ScreenUserWidget->PIEDisplayType = NewDisplayType;
			ScreenUserWidget->Hide();
			RequestGameDisplay();
		}
	}

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetActor")
	void SetGameDisplayType(EVRWidgetDisplayType NewDisplayType)
	{
		if (IsValid(ScreenUserWidget))
		{
			ScreenUserWidget->GameDisplayType = NewDisplayType;
			ScreenUserWidget->Hide();
			RequestGameDisplay();
		}
	}

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetActor")
		void SetWidgetVisible(bool bIsVisible);

	virtual void SetActorHiddenInGame(bool bNewHidden) override
	{
		SetWidgetVisible(bNewHidden);
		Super::SetActorHiddenInGame(bNewHidden);
	}

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetActor")
		UUserWidget* GetWidget();

	UFUNCTION(BlueprintCallable, Category = "FullScreenWidgetActor")
		UTextureRenderTarget2D* GetPostProcessRenderTarget();

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
#endif 

private:
	void RequestEditorDisplay();
	void RequestGameDisplay();

protected:

#if WITH_EDITORONLY_DATA

	bool bEditorDisplayRequested;
#endif
};
