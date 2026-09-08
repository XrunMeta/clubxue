

#pragma once

#include "Components/BillboardComponent.h"
#include "MapEnums.h"
#include "MapIconComponent.generated.h"

class UMapTrackerComponent;
class UMapViewComponent;
class UMapRendererComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapIconMaterialChangedSignature, UMapIconComponent*, MapIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapIconMaterialInstancesChangedSignature, UMapIconComponent*, MapIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapIconAppearanceChangedSignature, UMapIconComponent*, MapIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMapIconEnteredViewSignature, UMapIconComponent*, MapIcon, UMapViewComponent*, View);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMapIconLeftViewSignature, UMapIconComponent*, MapIcon, UMapViewComponent*, View);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapIconDestroyedSignature, UMapIconComponent*, MapIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapIconHoverStartSignature, UMapIconComponent*, MapIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapIconHoverEndSignature, UMapIconComponent*, MapIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMapIconClickedSignature, UMapIconComponent*, MapIcon, bool, bIsLeftMouse);

UCLASS(ClassGroup=(MinimapPlugin), hidecategories=(Sprite), meta=(BlueprintSpawnableComponent))
class MINIMAPPLUGIN_API UMapIconComponent : public UBillboardComponent
{
	GENERATED_BODY()

public:	
	UMapIconComponent();

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;
#endif

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconMaterialForUMG(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UMaterialInterface* GetIconMaterialForUMG() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UMaterialInterface* GetObjectiveArrowMaterialForUMG() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void ResetIconMaterialForUMG();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void GetIconMaterialInstancesForUMG(TArray<UMaterialInstanceDynamic*>& MaterialInstances);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void RegisterMaterialInstanceFromUMG(UUserWidget* IconWidget, UMaterialInstanceDynamic* MatInst);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconMaterialForCanvas(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UMaterialInterface* GetIconMaterialForCanvas() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UMaterialInterface* GetObjectiveArrowMaterialForCanvas() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void ResetIconMaterialForCanvas();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void GetIconMaterialInstancesForCanvas(TArray<UMaterialInstanceDynamic*>& MaterialInstances);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconTexture(UTexture2D* NewIcon);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UTexture2D* GetIconTexture() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconTooltipText(FName NewIconName);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	FName GetIconTooltipText() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconVisible(const bool bNewVisible);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsIconVisible() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconInteractable(const bool bNewInteractable);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsIconInteractable() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconRotates(const bool bNewRotates);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool DoesIconRotate() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconSize(const float NewIconSize, const EIconSizeUnit NewIconSizeUnit);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetIconSize() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	EIconSizeUnit GetIconSizeUnit() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconDrawColor(const FLinearColor& NewDrawColor);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	FLinearColor GetIconDrawColor() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconZOrder(const int32 NewZOrder);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	int32 GetIconZOrder() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetObjectiveArrowEnabled(const bool bNewObjectiveArrowEnabled);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsObjectiveArrowEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetObjectiveArrowTexture(UTexture2D* NewTexture);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	UTexture2D* GetObjectiveArrowTexture() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetObjectiveArrowRotates(const bool bNewRotates);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool DoesObjectiveArrowRotate() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetObjectiveArrowSize(const float NewObjectiveArrowSize);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetObjectiveArrowSize() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconBackgroundInteraction(const EIconBackgroundInteraction NewBackgroundInteraction);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	EIconBackgroundInteraction GetIconBackgroundInteraction() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconFogInteraction(const EIconFogInteraction NewFogInteraction);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	EIconFogInteraction GetIconFogInteraction() const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetIconFogRevealThreshold(const float NewFogRevealThreshold);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetIconFogRevealThreshold() const;

	UMaterialInstanceDynamic* GetIconMaterialInstanceForCanvas(UMapRendererComponent* Renderer);

	UMaterialInstanceDynamic* GetObjectiveArrowMaterialInstanceForCanvas(UMapRendererComponent* Renderer);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	bool MarkRenderedInView(UMapViewComponent* View, const bool bNewIsRendered);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsRenderedInView(UMapViewComponent* View) const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void ReceiveHoverStart();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void ReceiveHoverEnd();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void ReceiveClicked(const bool bIsLeftMouseButton);

private:

	void RefreshPreviewSprite();

	void UnmarkRenderedFromAllViews();

public:

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconAppearanceChangedSignature OnIconAppearanceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconMaterialChangedSignature OnIconMaterialChanged;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconMaterialInstancesChangedSignature OnIconMaterialInstancesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconEnteredViewSignature OnIconEnteredView;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconLeftViewSignature OnIconLeftView;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconDestroyedSignature OnIconDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconHoverStartSignature OnIconHoverStart;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconHoverEndSignature OnIconHoverEnd;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconClickedSignature OnIconClicked;

protected:

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering", BlueprintReadOnly)
	FName IconCategory = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering")
	UTexture2D* IconTexture;

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering")
	UMaterialInterface* IconMaterial_UMG;

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering")
	UMaterialInterface* IconMaterial_Canvas;

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering")
	bool bIconVisible = true;

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering")
	bool bIconRotates = false;

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering")
	EIconSizeUnit IconSizeUnit = EIconSizeUnit::ScreenSpace;

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering", meta = (ClampMin = "1.0"))
	float IconSize = 32.0f;

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering")
	FLinearColor IconDrawColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Minimap Icon Rendering")
	int32 IconZOrder = 0;

	UPROPERTY(EditAnywhere, Category = "Minimap Objective Arrow")
	bool bObjectiveArrowEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Minimap Objective Arrow", meta = (EditCondition = "bObjectiveArrowEnabled"))
	UTexture2D* ObjectiveArrowTexture;

	UPROPERTY(EditAnywhere, Category = "Minimap Objective Arrow")
	UMaterialInterface* ObjectiveArrowMaterial_UMG;

	UPROPERTY(EditAnywhere, Category = "Minimap Objective Arrow")
	UMaterialInterface* ObjectiveArrowMaterial_Canvas;

	UPROPERTY(EditAnywhere, Category = "Minimap Objective Arrow", meta = (EditCondition = "bObjectiveArrowEnabled"))
	bool bObjectiveArrowRotates = true;

	UPROPERTY(EditAnywhere, Category = "Minimap Objective Arrow", meta = (EditCondition = "bObjectiveArrowEnabled", ClampMin = "1.0"))
	float ObjectiveArrowSize = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Minimap Mouse Interaction")
	bool bIconInteractable = true;

	UPROPERTY(EditAnywhere, Category = "Minimap Mouse Interaction", meta = (EditCondition = "bIconInteractable"))
	FName IconTooltipText = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Minimap Environment Interaction")
	EIconBackgroundInteraction IconBackgroundInteraction = EIconBackgroundInteraction::AlwaysRender;

	UPROPERTY(EditAnywhere, Category = "Minimap Environment Interaction")
	EIconFogInteraction IconFogInteraction = EIconFogInteraction::AlwaysRenderUnderFog;

	UPROPERTY(EditAnywhere, Category = "Minimap Environment Interaction")
	float IconFogRevealThreshold = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Minimap Environment Interaction")
	bool bHideOwnerInsideFog = false;

private:

	UPROPERTY(Transient)
	TMap<UMapViewComponent*, bool> IsRenderedPerView;

	UPROPERTY(Transient)
	UMaterialInterface* InitialIconMaterial_UMG;

	UPROPERTY(Transient)
	UMaterialInterface* InitialIconMaterial_Canvas;

	UPROPERTY(Transient)
	TMap<UUserWidget*, UMaterialInstanceDynamic*> IconMaterialInstances_UMG;

	UPROPERTY(Transient)
	TMap<UMapRendererComponent*, UMaterialInstanceDynamic*> IconMaterialInstances_Canvas;
	UPROPERTY(Transient)
	TMap<UMapRendererComponent*, UMaterialInstanceDynamic*> ObjectiveArrowMaterialInstances_Canvas;

	float MaterialEffectStartTime = 0;

	bool bMouseOverStarted = false;

};
