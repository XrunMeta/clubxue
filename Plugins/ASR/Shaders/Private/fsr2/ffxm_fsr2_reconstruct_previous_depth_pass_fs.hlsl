

#define FSR2_BIND_SRV_INPUT_MOTION_VECTORS                  0
#define FSR2_BIND_SRV_INPUT_DEPTH                           1
#define FSR2_BIND_SRV_INPUT_COLOR                           2

#if FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE
#define FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH      3
#else
#define FSR2_BIND_SRV_INPUT_EXPOSURE                        3
#define FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH      4
#endif

#define FSR2_BIND_CB_FSR2                                   0

#include "ffxm_fsr2_callbacks_hlsl.h"
#include "ffxm_fsr2_common.h"
#include "ffxm_fsr2_sample.h"
#include "ffxm_fsr2_reconstruct_dilated_velocity_and_previous_depth.h"

struct VertexOut
{
	float4 position : SV_POSITION;
};

struct ReconstructPrevDepthOutputsFS
{
#if FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE
    FfxFloat32x4 fDepthMotionVectorLuma: SV_TARGET0;
#else
    FfxFloat32 fDepth           : SV_TARGET0;
    FfxFloat32x2 fMotionVector  : SV_TARGET1;
    FfxFloat32 fLuma            : SV_TARGET2;
#endif
};

ReconstructPrevDepthOutputsFS main(float4 SvPosition : SV_POSITION)
{
    uint2 uPixelCoord = uint2(SvPosition.xy);
    ReconstructPrevDepthOutputs result = ReconstructAndDilate(uPixelCoord);
    ReconstructPrevDepthOutputsFS output = (ReconstructPrevDepthOutputsFS)0;
#if FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE
    output.fDepthMotionVectorLuma = FfxFloat32x4(result.fDepth, result.fMotionVector, result.fLuma);
#else
    output.fDepth = result.fDepth;
    output.fMotionVector = result.fMotionVector;
    output.fLuma = result.fLuma;
#endif
    return output;
}
