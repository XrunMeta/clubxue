

#pragma once

#include "sl_dlss.h"

namespace sl
{

enum class DLSSDPreset : uint32_t
{

    eDefault,

    ePresetD = 4,   
    ePresetE = 5,   
    ePresetF = 6,   
    ePresetG = 7,   
    ePresetH = 8,   
    ePresetI = 9,   
    ePresetJ = 10,  
    ePresetK = 11,  
    ePresetL = 12,  
    ePresetM = 13,  
    ePresetN = 14,  
    ePresetO = 15,  

    eCount
};

enum class DLSSDNormalRoughnessMode : uint32_t
{
    eUnpacked,  
    ePacked,    

    eCount
};

SL_STRUCT_BEGIN(DLSSDOptions, StructType({ 0x0ad87504, 0x774e, 0x4bf3, { 0x96, 0x33, 0xa4, 0x4d, 0x1f, 0x7f, 0x9c, 0xb8 } }), kStructVersion3)

    DLSSMode mode = DLSSMode::eOff;

    uint32_t outputWidth = INVALID_UINT;

    uint32_t outputHeight = INVALID_UINT;

    float sharpness = 0.0f;

    float preExposure = 1.0f;

    float exposureScale = 1.0f;

    Boolean colorBuffersHDR = Boolean::eTrue;

    Boolean indicatorInvertAxisX = Boolean::eFalse;

    Boolean indicatorInvertAxisY = Boolean::eFalse;

    DLSSDNormalRoughnessMode normalRoughnessMode = DLSSDNormalRoughnessMode::eUnpacked;

    float4x4 worldToCameraView;

    float4x4 cameraViewToWorld;

    Boolean alphaUpscalingEnabled = Boolean::eFalse;

    DLSSDPreset dlaaPreset = DLSSDPreset::eDefault;
    DLSSDPreset qualityPreset = DLSSDPreset::eDefault;
    DLSSDPreset balancedPreset = DLSSDPreset::eDefault;
    DLSSDPreset performancePreset = DLSSDPreset::eDefault;
    DLSSDPreset ultraPerformancePreset = DLSSDPreset::eDefault;
    DLSSDPreset ultraQualityPreset = DLSSDPreset::eDefault;

SL_STRUCT_END()

SL_STRUCT_BEGIN(DLSSDOptimalSettings, StructType({ 0xfbd0c637, 0xa28f, 0x41f2, { 0xbc, 0x91, 0xb4, 0x21, 0xfa, 0xee, 0x8e, 0x1e } }), kStructVersion1)

    uint32_t optimalRenderWidth{};

    uint32_t optimalRenderHeight{};

    float optimalSharpness{};

    uint32_t renderWidthMin{};

    uint32_t renderHeightMin{};

    uint32_t renderWidthMax{};

    uint32_t renderHeightMax{};

SL_STRUCT_END()

SL_STRUCT_BEGIN(DLSSDState, StructType({ 0x71873c14, 0xf8ca, 0x4767, { 0x9e, 0xaf, 0x3b, 0x43, 0x93, 0xea, 0x98, 0xfa } }), kStructVersion1)

    uint64_t estimatedVRAMUsageInBytes {};

SL_STRUCT_END()

}

using PFun_slDLSSDGetOptimalSettings = sl::Result(const sl::DLSSDOptions & options, sl::DLSSDOptimalSettings & settings);

using PFun_slDLSSDGetState = sl::Result(const sl::ViewportHandle & viewport, sl::DLSSDState & state);

using PFun_slDLSSDSetOptions = sl::Result(const sl::ViewportHandle& viewport, const sl::DLSSDOptions& options);

inline sl::Result slDLSSDGetOptimalSettings(const sl::DLSSDOptions& options, sl::DLSSDOptimalSettings& settings)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDLSS_RR, slDLSSDGetOptimalSettings);
    return s_slDLSSDGetOptimalSettings(options, settings);
}

inline sl::Result slDLSSDGetState(const sl::ViewportHandle& viewport, sl::DLSSDState& state)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDLSS_RR, slDLSSDGetState);
    return s_slDLSSDGetState(viewport, state);
}

inline sl::Result slDLSSDSetOptions(const sl::ViewportHandle& viewport, const sl::DLSSDOptions& options)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDLSS_RR, slDLSSDSetOptions);
    return s_slDLSSDSetOptions(viewport, options);
}
