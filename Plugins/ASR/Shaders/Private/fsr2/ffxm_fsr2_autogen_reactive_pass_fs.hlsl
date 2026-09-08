

#define FSR2_BIND_SRV_INPUT_OPAQUE_ONLY                     0
#define FSR2_BIND_SRV_INPUT_COLOR                           1

#define FSR2_BIND_CB_FSR2                                   0
#define FSR2_BIND_CB_REACTIVE                               1

#include "ffxm_fsr2_callbacks_hlsl.h"
#include "ffxm_fsr2_common.h"

struct GenReactiveMaskOutputs
{
    FfxFloat32 fReactiveMask : SV_TARGET0;
};

struct VertexOut
{
	float4 position : SV_POSITION;
};

GenReactiveMaskOutputs main(float4 SvPosition : SV_POSITION)
{
    uint2 uPixelCoord = uint2(SvPosition.xy);

    float3 ColorPreAlpha    = LoadOpaqueOnly( FFXM_MIN16_I2(uPixelCoord) ).rgb;
    float3 ColorPostAlpha   = LoadInputColor(uPixelCoord).rgb;

    if (GenReactiveFlags() & FFXM_FSR2_AUTOREACTIVEFLAGS_APPLY_TONEMAP)
    {
        ColorPreAlpha = Tonemap(ColorPreAlpha);
        ColorPostAlpha = Tonemap(ColorPostAlpha);
    }

    if (GenReactiveFlags() & FFXM_FSR2_AUTOREACTIVEFLAGS_APPLY_INVERSETONEMAP)
    {
        ColorPreAlpha = InverseTonemap(ColorPreAlpha);
        ColorPostAlpha = InverseTonemap(ColorPostAlpha);
    }

    float out_reactive_value = 0.f;
    float3 delta = abs(ColorPostAlpha - ColorPreAlpha);

    out_reactive_value = (GenReactiveFlags() & FFXM_FSR2_AUTOREACTIVEFLAGS_USE_COMPONENTS_MAX) ? max(delta.x, max(delta.y, delta.z)) : length(delta);
    out_reactive_value *= GenReactiveScale();

    out_reactive_value = (GenReactiveFlags() & FFXM_FSR2_AUTOREACTIVEFLAGS_APPLY_THRESHOLD) ? (out_reactive_value < GenReactiveThreshold() ? 0 : GenReactiveBinaryValue()) : out_reactive_value;

    GenReactiveMaskOutputs results = (GenReactiveMaskOutputs)0;
    results.fReactiveMask = out_reactive_value;

    return results;
}
