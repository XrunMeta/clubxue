

#pragma once

#include "CoreMinimal.h"
#include "RendererInterface.h"
#include "ScreenPass.h"

STREAMLINESHADERS_API FRDGTextureRef AddStreamlineVelocityCombinePass(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FRDGTextureRef InSceneDepthTexture,
	FRDGTextureRef InVelocityTexture,
	FRDGTextureRef AlternateMotionVectorTexture,
	bool bDilateMotionVectors
);
