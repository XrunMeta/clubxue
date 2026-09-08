

#define FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH      0
#if FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE
#define FSR2_BIND_SRV_DILATED_DEPTH_MOTION_VECTORS_INPUT_LUMA 1
#define FSR2_BIND_SRV_PREV_DILATED_DEPTH_MOTION_VECTORS_INPUT_LUMA 2
#else
#define FSR2_BIND_SRV_DILATED_MOTION_VECTORS                1
#define FSR2_BIND_SRV_DILATED_DEPTH                         2
#define FSR2_BIND_SRV_REACTIVE_MASK                         3
#define FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK     4
#define FSR2_BIND_SRV_PREVIOUS_DILATED_MOTION_VECTORS       5
#endif
#define FSR2_BIND_SRV_INPUT_MOTION_VECTORS                  6
#define FSR2_BIND_SRV_INPUT_COLOR                           7
#define FSR2_BIND_SRV_INPUT_DEPTH                           8
#if !FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE
#define FSR2_BIND_SRV_INPUT_EXPOSURE                        9
#endif

#define FSR2_BIND_CB_FSR2                                   0

#include "ffxm_fsr2_callbacks_hlsl.h"
#include "ffxm_fsr2_common.h"
#include "ffxm_fsr2_sample.h"
#include "ffxm_fsr2_depth_clip.h"

struct VertexOut
{
	float4 position : SV_POSITION;
};

struct DepthClipOutputsFS
{
    FfxFloat32x2 fDilatedReactiveMasks    : SV_TARGET0;
#if !FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE
    FfxFloat32x4 fTonemapped              : SV_TARGET1;
#endif
};

DepthClipOutputsFS main(float4 SvPosition : SV_POSITION)
{
    uint2 uPixelCoord = uint2(SvPosition.xy);
    DepthClipOutputs result = DepthClip(uPixelCoord);
    DepthClipOutputsFS output = (DepthClipOutputsFS)0;
    output.fDilatedReactiveMasks = result.fDilatedReactiveMasks;
#if !FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE
    output.fTonemapped = result.fTonemapped;
#endif
    return output;
}
