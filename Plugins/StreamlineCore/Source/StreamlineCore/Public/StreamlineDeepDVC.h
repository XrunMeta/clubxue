

#pragma once

#include "CoreMinimal.h"
#include "RenderGraphDefinitions.h"
#include "StreamlineCore.h"
class FStreamlineRHI;

bool IsDeepDVCActive();

extern STREAMLINECORE_API Streamline::EStreamlineFeatureSupport QueryStreamlineDeepDVCSupport();
extern STREAMLINECORE_API bool IsStreamlineDeepDVCSupported();

class FRHICommandListImmediate;
struct FRHIStreamlineArguments;
class FSceneViewFamily;
class FRDGBuilder;
void AddStreamlineDeepDVCStateRenderPass(FRDGBuilder& GraphBuilder, uint32 ViewID, const FIntRect& SecondaryViewRect);
void AddStreamlineDeepDVCEvaluateRenderPass(FStreamlineRHI* StreamlineRHIExtensions, FRDGBuilder& GraphBuilder, uint32 ViewID, const FIntRect& SecondaryViewRect, FRDGTextureRef SLSceneColorWithoutHUD);
void BeginRenderViewFamilyDeepDVC(FSceneViewFamily& InViewFamily);
void GetDeepDVCStatusFromStreamline();
