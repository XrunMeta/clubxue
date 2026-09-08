

#pragma once

#include "sl_pcl.h"

namespace sl
{

enum ReflexMode
{
    eOff,
    eLowLatency,
    eLowLatencyWithBoost,

    ReflexMode_eCount
};

SL_STRUCT_BEGIN(ReflexOptions, StructType({ 0xf03af81a, 0x6d0b, 0x4902, { 0xa6, 0x51, 0xc4, 0x96, 0x5e, 0x21, 0x54, 0x34 } }), kStructVersion1)

    ReflexMode mode = ReflexMode::eOff;

    uint32_t frameLimitUs = 0;

    bool useMarkersToOptimize = false;

    uint16_t virtualKey = 0;

    uint32_t idThread = 0;

SL_STRUCT_END()

SL_STRUCT_BEGIN(ReflexReport, StructType({ 0xd569b37, 0xa1c8, 0x4453, { 0xbe, 0x4d, 0x40, 0xf4, 0xde, 0x57, 0x95, 0x2b } }), kStructVersion1)

    uint64_t frameID{};
    uint64_t inputSampleTime{};
    uint64_t simStartTime{};
    uint64_t simEndTime{};
    uint64_t renderSubmitStartTime{};
    uint64_t renderSubmitEndTime{};
    uint64_t presentStartTime{};
    uint64_t presentEndTime{};
    uint64_t driverStartTime{};
    uint64_t driverEndTime{};
    uint64_t osRenderQueueStartTime{};
    uint64_t osRenderQueueEndTime{};
    uint64_t gpuRenderStartTime{};
    uint64_t gpuRenderEndTime{};
    uint32_t gpuActiveRenderTimeUs{};
    uint32_t gpuFrameTimeUs{};

SL_STRUCT_END()

SL_STRUCT_BEGIN(ReflexReport2, StructType({ 0x68bb0632, 0x5e1c, 0x402b, { 0x89, 0x9d, 0xb4, 0x9f, 0x63, 0x3c, 0x56, 0xc2 } }), kStructVersion1)

    uint64_t cameraConstructedTime{};
    uint32_t crossAdapterCopyTimeUs{};

SL_STRUCT_END()

constexpr int kReflexFrameReportCount = 64;

SL_STRUCT_BEGIN(ReflexState, StructType({ 0xf0bb5985, 0xdaf9, 0x4728, { 0xb2, 0xfd, 0xae, 0x80, 0xa2, 0xbd, 0x79, 0x89 } }), kStructVersion2)

    bool lowLatencyAvailable = false;

    bool latencyReportAvailable = false;

    uint32_t statsWindowMessage;

    ReflexReport frameReport[kReflexFrameReportCount];

    bool flashIndicatorDriverControlled = false;

    ReflexReport2 frameReport2[kReflexFrameReportCount];

SL_STRUCT_END()

SL_STRUCT_BEGIN(ReflexCameraData, StructType({ 0xc83cbb02, 0xb4e2, 0x4260, { 0x9c, 0xa2, 0xd0, 0xc3, 0xde, 0x3a, 0x96, 0x84 } }), kStructVersion1)
    float4x4 worldToViewMatrix;
    float4x4 viewToClipMatrix;
    float4x4 prevRenderedWorldToViewMatrix;
    float4x4 prevRenderedViewToClipMatrix;

SL_STRUCT_END()

SL_STRUCT_BEGIN(ReflexPredictedCameraData, StructType({ 0x8b960090, 0xa807, 0x4c85, { 0xb0, 0x2f, 0x10, 0x69, 0x95, 0x0d, 0x06, 0x6c } }), kStructVersion1)
    float4x4 predictedWorldToViewMatrix;
    float4x4 predictedViewToClipMatrix;

SL_STRUCT_END()

using MarkerUnderlying = std::underlying_type_t<PCLMarker>;

SL_STRUCT_BEGIN(ReflexHelper, StructType({ 0xe268b3dc, 0xf963, 0x4c37, { 0x97, 0x76, 0xaf, 0x4, 0x8e, 0x13, 0x26, 0x21 } }), kStructVersion1)
    ReflexHelper(MarkerUnderlying m) : BaseStructure(ReflexHelper::s_structType, kStructVersion1), marker(m) {};
    ReflexHelper(PCLMarker m) : BaseStructure(ReflexHelper::s_structType, kStructVersion1), marker(to_underlying(m)) {};
    operator MarkerUnderlying () const { return marker; };
private:

    MarkerUnderlying marker;
SL_STRUCT_END()

}

using PFun_slReflexGetState = sl::Result(sl::ReflexState& state);

using PFun_slReflexSleep = sl::Result(const sl::FrameToken& frame);

using PFun_slReflexSetOptions = sl::Result(const sl::ReflexOptions& options);

using PFun_slReflexSetCameraData = sl::Result(const sl::ViewportHandle& viewport, const sl::FrameToken& frame, const sl::ReflexCameraData& inCameraData);

using PFun_slReflexGetPredictedCameraData = sl::Result(const sl::ViewportHandle& viewport, const sl::FrameToken& frame, sl::ReflexPredictedCameraData& outCameraData);

inline sl::Result slReflexGetState(sl::ReflexState& state)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureReflex, slReflexGetState);
    return s_slReflexGetState(state);
}

inline sl::Result slReflexSleep(const sl::FrameToken& frame)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureReflex, slReflexSleep);
    return s_slReflexSleep(frame);
}

inline sl::Result slReflexSetOptions(const sl::ReflexOptions& options)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureReflex, slReflexSetOptions);
    return s_slReflexSetOptions(options);
}

inline sl::Result slReflexSetCameraData(const sl::ViewportHandle& viewport, const sl::FrameToken& frame, const sl::ReflexCameraData& inCameraData)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureReflex, slReflexSetCameraData);
    return s_slReflexSetCameraData(viewport, frame, inCameraData);
}

inline sl::Result slReflexGetPredictedCameraData(const sl::ViewportHandle& viewport, const sl::FrameToken& frame, sl::ReflexPredictedCameraData& outCameraData)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureReflex, slReflexGetPredictedCameraData);
    return s_slReflexGetPredictedCameraData(viewport, frame, outCameraData);
}

