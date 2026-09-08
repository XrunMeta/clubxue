

#pragma once

namespace sl
{

enum class NISMode : uint32_t
{
    eOff,
    eScaler,
    eSharpen,
    eCount
};

enum class NISHDR : uint32_t
{
    eNone,
    eLinear,
    ePQ,
    eCount
};

SL_STRUCT_BEGIN(NISOptions, StructType({ 0x676610e5, 0x9674, 0x4d3a, { 0x9c, 0x8a, 0xf4, 0x95, 0xd0, 0x1b, 0x36, 0xf3 } }), kStructVersion1)

    NISMode mode = NISMode::eScaler;

    NISHDR hdrMode = NISHDR::eNone;

    float sharpness = 0.0f;

SL_STRUCT_END()

SL_STRUCT_BEGIN(NISState, StructType({ 0x71ab4fd0, 0xd959, 0x4c2a, { 0xaf, 0x69, 0xed, 0x48, 0x50, 0xbd, 0x4e, 0x3d } }), kStructVersion1)

    uint64_t estimatedVRAMUsageInBytes {};

SL_STRUCT_END()

}

using PFun_slNISSetOptions = sl::Result(const sl::ViewportHandle& viewport, const sl::NISOptions& options);

using PFun_slNISGetState = sl::Result(const sl::ViewportHandle& viewport, sl::NISState& state);

inline sl::Result slNISSetOptions(const sl::ViewportHandle& viewport, const sl::NISOptions& options)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureNIS, slNISSetOptions);
    return s_slNISSetOptions(viewport, options);
}

inline sl::Result slNISGetState(const sl::ViewportHandle& viewport, sl::NISState& state)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureNIS, slNISGetState);
    return s_slNISGetState(viewport, state);
}
