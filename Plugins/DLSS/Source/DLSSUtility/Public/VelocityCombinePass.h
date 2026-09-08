

#pragma once

#include "CoreMinimal.h"
#include "RendererInterface.h"
#include "Runtime/Launch/Resources/Version.h"
#include "SceneTexturesConfig.h"

extern DLSSUTILITY_API FRDGTextureRef AddVelocityCombinePass(
	FRDGBuilder& GraphBuilder,
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	const FSceneView& View,
#else
	const FViewInfo& View,
#endif
	FRDGTextureRef InSceneDepthTexture,
	FRDGTextureRef InVelocityTexture,
	FRDGTextureRef AlternateMotionVectorTexture,
	FIntRect InputViewRect,
	FIntRect DLSSOutputViewRect,
	FVector2f TemporalJitterPixels
);
