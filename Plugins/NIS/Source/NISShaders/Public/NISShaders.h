

#pragma once

#include "Modules/ModuleManager.h"
#include "PostProcess/PostProcessUpscale.h"
class FNISViewExtension;

DECLARE_LOG_CATEGORY_EXTERN(LogNIS, Log, All);

class FNISShadersModule final: public IModuleInterface
{
public:

	virtual void StartupModule();

	virtual void ShutdownModule();

private:
};

NISSHADERS_API ERHIFeatureLevel::Type GetNISMinRequiredFeatureLevel();

NISSHADERS_API FScreenPassTexture AddSharpenOrUpscalePass(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	const ISpatialUpscaler::FInputs& Inputs
);

