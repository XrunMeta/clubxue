

#pragma once

#include "sl.h"
#include "sl_consts.h"
#include "sl_core_types.h"
#include <vector>

namespace sl
{

enum class DLSSGMode : uint32_t
{
    eOff,
    eOn,
    eAuto,
    eDynamic,
    eCount
};

enum class DLSSGFlags : uint32_t
{
    eShowOnlyInterpolatedFrame = 1 << 0,
    eDynamicResolutionEnabled = 1 << 1,
    eRequestVRAMEstimate = 1 << 2,
    eRetainResourcesWhenOff = 1 << 3,
    eEnableFullscreenMenuDetection = 1 << 4,

    eAll = eShowOnlyInterpolatedFrame | eDynamicResolutionEnabled | eRequestVRAMEstimate | eRetainResourcesWhenOff | eEnableFullscreenMenuDetection
};

enum class DLSSGQueueParallelismMode : uint32_t
{

    eBlockPresentingClientQueue,

    eBlockNoClientQueues,
    eCount
};

SL_ENUM_OPERATORS_32(DLSSGFlags)

SL_STRUCT_BEGIN(DLSSGOptions, StructType({ 0xfac5f1cb, 0x2dfd, 0x4f36, { 0xa1, 0xe6, 0x3a, 0x9e, 0x86, 0x52, 0x56, 0xc5 } }), kStructVersion5)

    DLSSGMode mode = DLSSGMode::eOff;

    uint32_t numFramesToGenerate = 1;

    DLSSGFlags flags{};

    uint32_t dynamicResWidth{};

    uint32_t dynamicResHeight{};

    uint32_t numBackBuffers{};

    uint32_t mvecDepthWidth{};

    uint32_t mvecDepthHeight{};

    uint32_t colorWidth{};

    uint32_t colorHeight{};

    uint32_t colorBufferFormat{};

    uint32_t mvecBufferFormat{};

    uint32_t depthBufferFormat{};

    uint32_t hudLessBufferFormat{};

    uint32_t uiBufferFormat{};

    PFunOnAPIErrorCallback* onErrorCallback{};

    Boolean bReserved15 = eInvalid;

    DLSSGQueueParallelismMode queueParallelismMode{};

    Boolean enableUserInterfaceRecomposition = Boolean::eFalse;

    float dynamicTargetFrameRate{};

SL_STRUCT_END()

enum class DLSSGStatus : uint32_t
{

    eOk = 0,

    eFailResolutionTooLow = 1 << 0,

    eFailReflexNotDetectedAtRuntime = 1 << 1,

    eFailHDRFormatNotSupported = 1 << 2,

    eFailCommonConstantsInvalid = 1 << 3,

    eFailGetCurrentBackBufferIndexNotCalled = 1 << 4,

    eReserved5 = 1 << 5,

    eAll = eFailResolutionTooLow | eFailReflexNotDetectedAtRuntime | eFailHDRFormatNotSupported | eFailCommonConstantsInvalid | eFailGetCurrentBackBufferIndexNotCalled | eReserved5
};

SL_ENUM_OPERATORS_32(DLSSGStatus)

SL_STRUCT_BEGIN(DLSSGState, StructType({ 0xcc8ac8e1, 0xa179, 0x44f5, { 0x97, 0xfa, 0xe7, 0x41, 0x12, 0xf9, 0xbc, 0x61 } }), kStructVersion4)

    uint64_t estimatedVRAMUsageInBytes{};

    DLSSGStatus status{};

    uint32_t minWidthOrHeight{};

    uint32_t numFramesActuallyPresented{};

    uint32_t numFramesToGenerateMax{};

    sl::Boolean bReserved4{};

    sl::Boolean bIsVsyncSupportAvailable{};

    void* inputsProcessingCompletionFence{};
    uint64_t lastPresentInputsProcessingCompletionFenceValue{};

    sl::Boolean bIsDynamicMFGSupported{};

SL_STRUCT_END()

}

using PFun_slDLSSGGetState = sl::Result(const sl::ViewportHandle& viewport, sl::DLSSGState& state, const sl::DLSSGOptions* options);

using PFun_slDLSSGSetOptions = sl::Result(const sl::ViewportHandle& viewport, const sl::DLSSGOptions& options);

inline sl::Result slDLSSGGetState(const sl::ViewportHandle& viewport, sl::DLSSGState& state, const sl::DLSSGOptions* options)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDLSS_G, slDLSSGGetState);
    return s_slDLSSGGetState(viewport, state, options);
}

inline sl::Result slDLSSGSetOptions(const sl::ViewportHandle& viewport, const sl::DLSSGOptions& options)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeatureDLSS_G, slDLSSGSetOptions);
    return s_slDLSSGSetOptions(viewport, options);
}
