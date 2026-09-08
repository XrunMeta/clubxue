

#pragma once

#include "CoreMinimal.h"
#include "RendererInterface.h"
#include "ScreenPass.h"
#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION == 4  || ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 1
#define FTextureRHIRef FTexture2DRHIRef
#endif

extern STREAMLINESHADERS_API FRDGTextureRef AddStreamlineUIHintExtractionPass(
	FRDGBuilder& GraphBuilder,

	const float InAlphaThresholdValue,
	const FTextureRHIRef& InBackBuffer

);
