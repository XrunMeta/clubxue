

#pragma once

#include "CoreMinimal.h"
#include "StreamlineCore.h"
class FStreamlineRHI;
void RegisterStreamlineDLSSGHooks(FStreamlineRHI* InStreamlineRHI);
void UnregisterStreamlineDLSSGHooks();

bool IsDLSSGActive();

extern STREAMLINECORE_API Streamline::EStreamlineFeatureSupport QueryStreamlineDLSSGSupport();
extern STREAMLINECORE_API bool IsStreamlineDLSSGSupported();

extern STREAMLINECORE_API int32 GetStreamlineDLSSGNumFramesToGenerate();
extern STREAMLINECORE_API void GetStreamlineDLSSGMinMaxGeneratedFrames(int32& MinGeneratedFrames, int32& MaxGeneratedFrames);
extern STREAMLINECORE_API bool IsStreamlineDynamicDLSSGAvailable();
extern STREAMLINECORE_API bool IsStreamlineVsyncSupportAvailable();

extern STREAMLINECORE_API void GetStreamlineDLSSGFrameTiming(float& FrameRateInHertz, int32& FramesPresented);

class FRHICommandListImmediate;
struct FRHIStreamlineArguments;
class FSceneViewFamily;
class FRDGBuilder;
void AddStreamlineDLSSGStateRenderPass(FRDGBuilder& GraphBuilder, uint32 ViewID, const FIntRect& SecondaryViewRect);
void BeginRenderViewFamilyDLSSG(FSceneViewFamily& InViewFamily);
void GetDLSSGStatusFromStreamline(bool bQueryOncePerAppLifetimeValues = false);