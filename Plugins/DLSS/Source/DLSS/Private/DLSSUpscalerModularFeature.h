

#pragma once

#include "DLSSUpscaler.h"

#include "CoreMinimal.h"
#if ENGINE_SUPPORTS_UPSCALER_MODULAR_FEATURE
#include "IUpscalerModularFeature.h"
#include "Misc/Optional.h"
#include "StructUtils/PropertyBag.h"
#include "Templates/SharedPointer.h"
#endif

#include "DLSSUpscalerModularFeature.generated.h"

class ISceneViewExtension;
struct FSceneViewExtensionContext;

#if ENGINE_SUPPORTS_UPSCALER_MODULAR_FEATURE
using namespace UE::VirtualProduction;
#endif

UENUM()
enum class EDLSSUpscalerModularFeatureQuality : uint8
{
	Auto             UMETA(DisplayName = "Auto", ToolTip = "Use Auto to select best quality setting for a given resolution"),
	UltraQuality     UMETA(DisplayName = "Ultra Quality"),
	Quality          UMETA(DisplayName = "Quality"),
	Balanced         UMETA(DisplayName = "Balanced"),
	Performance      UMETA(DisplayName = "Performance"),
	UltraPerformance UMETA(DisplayName = "Ultra Performance"),
	DLAA             UMETA(DisplayName = "DLAA"),
	Count            UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FDLSSUpscalerModularFeatureSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLSS", meta = (DisplayName = "Quality"))
	EDLSSUpscalerModularFeatureQuality Quality = EDLSSUpscalerModularFeatureQuality::Auto;
};

#if ENGINE_SUPPORTS_UPSCALER_MODULAR_FEATURE

class FDLSSTemporalUpscalerModularFeature
	: public IUpscalerModularFeature
	, public TSharedFromThis<FDLSSTemporalUpscalerModularFeature, ESPMode::ThreadSafe>
{
public:
	virtual ~FDLSSTemporalUpscalerModularFeature() = default;

public:

	static const FDLSSTemporalUpscalerModularFeature* Get()
	{
		return ModularFeatureSingleton.Get();
	}

	static void RegisterModularFeature();
	static void UnregisterModularFeature();

public:

	virtual const FName& GetName() const override;
	virtual const FText& GetDisplayName() const override;
	virtual const FText& GetTooltipText() const override;

	virtual bool IsFeatureEnabled() const override;

	virtual bool AddSceneViewExtensionIsActiveFunctor(const FSceneViewExtensionIsActiveFunctor& IsActiveFunction) override;
	virtual bool RemoveSceneViewExtensionIsActiveFunctor(const FGuid& FunctorGuid) override;

	virtual bool GetSettings(FInstancedPropertyBag& OutSettings) const override;

	virtual void SetupSceneView(
		const FInstancedPropertyBag& InUpscalerSettings,
		FSceneView& InOutView) override;

	virtual bool PostConfigureViewFamily(
		const FInstancedPropertyBag& InUpscalerSettings,
		const FUpscalerModularFeatureParameters& InUpscalerParam,
		FSceneViewFamilyContext& InOutViewFamily) override;

public:

	TOptional<bool> SceneViewExtensionIsActive(const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context) const;

	const FInstancedPropertyBag* GetCustomSettings(const FSceneView& View) const;

	const FInstancedPropertyBag* GetCustomSettings_RenderThread(const FSceneView& View) const;

	static TOptional<EDLSSQualityMode> GetQualityMode(const FInstancedPropertyBag& InSettings, int32 PixelCount);

private:

	TArray<FSceneViewExtensionIsActiveFunctor> IsActiveThisFrameFunctions;

	TMap<uint32, FInstancedPropertyBag> CustomSettings;

	TMap<uint32, FInstancedPropertyBag> CustomSettings_RenderThread;

	uint64 LastFrameCounter = 0;

	static TSharedPtr<FDLSSTemporalUpscalerModularFeature, ESPMode::ThreadSafe> ModularFeatureSingleton;
};
#endif 

