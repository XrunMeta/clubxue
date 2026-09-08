

#pragma once

#include "CoreMinimal.h"
#include "RendererInterface.h"
#include "Runtime/Launch/Resources/Version.h"
#include "ScreenPass.h"
#include "SceneTexturesConfig.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
#include "TemporalUpscaler.h"
using ITemporalUpscaler = UE::Renderer::Private::ITemporalUpscaler;
#endif

#ifndef SUPPORT_GUIDE_GBUFFER
#define SUPPORT_GUIDE_GBUFFER 0
#endif

#ifndef SUPPORT_GUIDE_SSS_DOF
#define SUPPORT_GUIDE_SSS_DOF 0
#endif

struct FGBufferResolveOutputs
{
	FRDGTextureRef DiffuseAlbedo = nullptr;
	FRDGTextureRef SpecularAlbedo = nullptr;

	FRDGTextureRef Normals = nullptr;
	FRDGTextureRef Roughness = nullptr;

	FRDGTextureRef LinearDepth = nullptr;

#if SUPPORT_GUIDE_GBUFFER
	FRDGTextureRef ReflectionHitDistance = nullptr;
#endif

#if SUPPORT_GUIDE_SSS_DOF
	FRDGTextureRef SubsurfaceScatteringGuide = nullptr;
	FRDGTextureRef DepthOfFieldGuide = nullptr;
#endif

};

extern DLSSUTILITY_API FGBufferResolveOutputs AddGBufferResolvePass(
	FRDGBuilder& GraphBuilder,
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	const FSceneView& View,
	const ITemporalUpscaler::FInputs& PassInputs,
#else
	const FViewInfo& View,
#endif
	FIntRect InputViewRect,
	const bool bComputeDiffuseSpecularAlbedo
);
