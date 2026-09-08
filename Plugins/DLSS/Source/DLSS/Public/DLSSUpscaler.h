

#pragma once

#include "CoreMinimal.h"
#include "CustomResourcePool.h"

#define UE_API DLSS_API

struct FDLSSOptimalSettings;
class FSceneViewFamily;
class NGXRHI;

enum class EDLSSQualityMode
{
	MinValue = -2,
	UltraPerformance = -2,
	Performance = -1,
	Balanced = 0,
	Quality = 1,
	UltraQuality = 2,
	DLAA = 3,
	MaxValue = DLAA,
	NumValues = 6
};

class FDLSSUpscaler final : public ICustomResourcePool
{

	friend class FDLSSModule;
public:
	UE_NONCOPYABLE(FDLSSUpscaler)

	void SetupViewFamily(FSceneViewFamily& ViewFamily);

	UE_API float GetOptimalResolutionFractionForQuality(EDLSSQualityMode Quality) const;

	UE_API float GetMinResolutionFractionForQuality(EDLSSQualityMode Quality) const;
	UE_API float GetMaxResolutionFractionForQuality(EDLSSQualityMode Quality) const;
	UE_API bool IsFixedResolutionFraction(EDLSSQualityMode Quality) const;

	const NGXRHI* GetNGXRHI() const
	{
		return NGXRHIExtensions;
	}

	virtual void Tick(FRHICommandListImmediate& RHICmdList) override;

	UE_API bool IsQualityModeSupported(EDLSSQualityMode InQualityMode) const;
	uint32 GetNumRuntimeQualityModes() const
	{
		return NumRuntimeQualityModes;
	}

	bool IsDLSSActive() const;

	UE_API TOptional<EDLSSQualityMode> GetAutoQualityModeFromPixels(int PixelCount) const;

	static void ReleaseStaticResources();

	static float GetMinUpsampleResolutionFraction()
	{
		return MinDynamicResolutionFraction;
	}

	static float GetMaxUpsampleResolutionFraction()
	{
		return MaxDynamicResolutionFraction;
	}

private:
	FDLSSUpscaler(NGXRHI* InNGXRHIExtensions);

	bool EnableDLSSInPlayInEditorViewports() const;

	UE_API static NGXRHI* NGXRHIExtensions;
	UE_API static float MinDynamicResolutionFraction;
	UE_API static float MaxDynamicResolutionFraction;

	static uint32 NumRuntimeQualityModes;
	static TArray<FDLSSOptimalSettings> ResolutionSettings;
	float PreviousResolutionFraction;

	friend class FDLSSUpscalerViewExtension;
	friend class FDLSSSceneViewFamilyUpscaler;
};

#undef UE_API
