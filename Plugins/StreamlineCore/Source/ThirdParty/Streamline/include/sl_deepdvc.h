

#pragma once

#include "sl.h"
#include "sl_helpers.h"

namespace sl
{

enum class DeepDVCMode : uint32_t
{
    eOff,
    eOn,    
    eCount
};

SL_STRUCT_BEGIN(DeepDVCOptions, StructType({ 0x23288aad, 0x7e7e, 0xbe2a, { 0x91, 0x67, 0x27, 0xda, 0x30, 0xa3, 0x04, 0x6b } }), kStructVersion1)

    DeepDVCMode mode = DeepDVCMode::eOff;

    float intensity = 0.5f;

    float saturationBoost = 0.25f;
SL_STRUCT_END()

SL_STRUCT_BEGIN(DeepDVCState, StructType({ 0x934fd3d3, 0xb34c, 0x70a7, { 0xa1, 0x39, 0xf1, 0x9f, 0xe0, 0x4d, 0x91, 0xd3 } }), kStructVersion1)

    uint64_t estimatedVRAMUsageInBytes {};
SL_STRUCT_END()

}

using PFun_slDeepDVCSetOptions = sl::Result(const sl::ViewportHandle& viewport, const sl::DeepDVCOptions& options);

using PFun_slDeepDVCGetState = sl::Result(const sl::ViewportHandle& viewport, sl::DeepDVCState& state);

inline sl::Result slDeepDVCSetOptions(const sl::ViewportHandle& viewport, const sl::DeepDVCOptions& options)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDeepDVC, slDeepDVCSetOptions);
    return s_slDeepDVCSetOptions(viewport, options);
}

inline sl::Result slDeepDVCGetState(const sl::ViewportHandle& viewport, sl::DeepDVCState& state)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDeepDVC, slDeepDVCGetState);
    return s_slDeepDVCGetState(viewport, state);
}

inline const char* getDeepDVCModeAsStr(sl::DeepDVCMode v)
{
    switch (v)
    {
        SL_CASE_STR(sl::DeepDVCMode::eOff);
        SL_CASE_STR(sl::DeepDVCMode::eOn);
    };
    return "Unknown";
}
