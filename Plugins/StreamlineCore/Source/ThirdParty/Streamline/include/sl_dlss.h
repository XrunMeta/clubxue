

#pragma once

#if __cplusplus >= 201402L
#define SR_DEPRECATED_SHARPENING [[deprecated("Sharpness is not supported")]]
#else
#define SR_DEPRECATED_SHARPENING
#endif

namespace sl
{

enum class DLSSMode : uint32_t
{
    eOff,
    eMaxPerformance,
    eBalanced,
    eMaxQuality,
    eUltraPerformance,
    eUltraQuality,
    eDLAA,
    eCount,
};

enum class DLSSPreset : uint32_t
{

    eDefault,

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

SL_STRUCT_BEGIN(DLSSOptions, StructType({ 0x6ac826e4, 0x4c61, 0x4101, { 0xa9, 0x2d, 0x63, 0x8d, 0x42, 0x10, 0x57, 0xb8 } }), kStructVersion3)

    DLSSMode mode = DLSSMode::eOff;

    uint32_t outputWidth = INVALID_UINT;

    uint32_t outputHeight = INVALID_UINT;

    float sharpness SR_DEPRECATED_SHARPENING = 0.0f;

    float preExposure = 1.0f;

    float exposureScale = 1.0f;

    Boolean colorBuffersHDR = Boolean::eTrue;

    Boolean indicatorInvertAxisX = Boolean::eFalse;

    Boolean indicatorInvertAxisY = Boolean::eFalse;

    DLSSPreset dlaaPreset = DLSSPreset::eDefault;
    DLSSPreset qualityPreset = DLSSPreset::eDefault;
    DLSSPreset balancedPreset = DLSSPreset::eDefault;
    DLSSPreset performancePreset = DLSSPreset::eDefault;
    DLSSPreset ultraPerformancePreset = DLSSPreset::eDefault;
    DLSSPreset ultraQualityPreset = DLSSPreset::eDefault;

    Boolean useAutoExposure = Boolean::eFalse;

    Boolean alphaUpscalingEnabled = Boolean::eFalse;

SL_STRUCT_END()

SL_STRUCT_BEGIN(DLSSOptimalSettings, StructType({ 0xef1d0957, 0xfd58, 0x4df7, { 0xb5, 0x4, 0x8b, 0x69, 0xd8, 0xaa, 0x6b, 0x76 } }), kStructVersion1)

    uint32_t optimalRenderWidth{};

    uint32_t optimalRenderHeight{};

    float optimalSharpness{};

    uint32_t renderWidthMin{};

    uint32_t renderHeightMin{};

    uint32_t renderWidthMax{};

    uint32_t renderHeightMax{};

SL_STRUCT_END()

SL_STRUCT_BEGIN(DLSSState, StructType({ 0x9366b056, 0x8c01, 0x463c, { 0xbb, 0x91, 0xe6, 0x87, 0x82, 0x63, 0x6c, 0xe9 } }), kStructVersion1)

    uint64_t estimatedVRAMUsageInBytes{};

SL_STRUCT_END()

}

using PFun_slDLSSGetOptimalSettings = sl::Result(const sl::DLSSOptions & options, sl::DLSSOptimalSettings & settings);

using PFun_slDLSSGetState = sl::Result(const sl::ViewportHandle & viewport, sl::DLSSState & state);

using PFun_slDLSSSetOptions = sl::Result(const sl::ViewportHandle& viewport, const sl::DLSSOptions& options);

inline sl::Result slDLSSGetOptimalSettings(const sl::DLSSOptions& options, sl::DLSSOptimalSettings& settings)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDLSS, slDLSSGetOptimalSettings);
    return s_slDLSSGetOptimalSettings(options, settings);
}

inline sl::Result slDLSSGetState(const sl::ViewportHandle& viewport, sl::DLSSState& state)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDLSS, slDLSSGetState);
    return s_slDLSSGetState(viewport, state);
}

inline sl::Result slDLSSSetOptions(const sl::ViewportHandle& viewport, const sl::DLSSOptions& options)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDLSS, slDLSSSetOptions);
    return s_slDLSSSetOptions(viewport, options);
}
