

#define FSR2_BIND_SRV_INPUT_EXPOSURE        0
#define FSR2_BIND_SRV_RCAS_INPUT            1

#define FSR2_BIND_CB_FSR2                   0
#define FSR2_BIND_CB_RCAS                   1

#include "ffxm_fsr2_callbacks_hlsl.h"
#include "ffxm_fsr2_common.h"
#include "ffxm_fsr2_rcas.h"

struct VertexOut
{
	float4 position : SV_POSITION;
};

struct RCASOutputsFS
{
    FfxFloat32x3 fUpscaledColor    : SV_TARGET0;
};

RCASOutputsFS main(float4 SvPosition : SV_POSITION)
{
    uint2 uPixelCoord = uint2(SvPosition.xy);
    RCASOutputs result = RCAS(uPixelCoord);
    RCASOutputsFS output = (RCASOutputsFS)0;
    output.fUpscaledColor = result.fUpscaledColor;
    return output;
}
