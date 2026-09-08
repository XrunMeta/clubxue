

#pragma once

#include <cassert>

#include "sl.h"

namespace sl
{

enum class PCLHotKey: int16_t
{
    eUsePingMessage = 0,
    eVK_F13 = 0x7C,
    eVK_F14 = 0x7D,
    eVK_F15 = 0x7E,
};

SL_STRUCT_BEGIN(PCLOptions, StructType({ 0xcfa32f9b, 0x023c, 0x420e, { 0x90, 0x56, 0x68, 0x32, 0xb7, 0x4f, 0x89, 0xb4 } }), kStructVersion1)

    PCLHotKey virtualKey = PCLHotKey::eUsePingMessage;

    uint32_t idThread = 0;

SL_STRUCT_END()

SL_STRUCT_BEGIN(PCLState, StructType({ 0xcfa32f9b, 0x023c, 0x420e, { 0x90, 0x56, 0x68, 0x32, 0xb7, 0x4f, 0x89, 0xb5 } }), kStructVersion1)

    uint32_t statsWindowMessage;

SL_STRUCT_END()

enum class PCLMarker: uint32_t
{
    eSimulationStart = 0,
    eSimulationEnd = 1,
    eRenderSubmitStart = 2,
    eRenderSubmitEnd = 3,
    ePresentStart = 4,
    ePresentEnd = 5,

    eTriggerFlash = 7,
    ePCLatencyPing = 8,
    eOutOfBandRenderSubmitStart = 9,
    eOutOfBandRenderSubmitEnd = 10,
    eOutOfBandPresentStart = 11,
    eOutOfBandPresentEnd = 12,
    eControllerInputSample = 13,
    eDeltaTCalculation = 14,
    eLateWarpPresentStart = 15,
    eLateWarpPresentEnd = 16,
    eCameraConstructed = 17,
    eLateWarpRenderSubmitStart = 18,
    eLateWarpRenderSubmitEnd = 19,
    eVendorInternalAsyncPresentStart = 20,
    eVendorInternalAsyncPresentEnd = 21,
    eNumPresentsInBatch = 22,

    eMaximum
};

#if __cplusplus == 202302L
using to_underlying = std::to_underlying;
#else

template<class T>
constexpr auto to_underlying(T value)
{
    return std::underlying_type_t<T>(value);
}
#endif

SL_STRUCT_BEGIN(PCLHelper, StructType({ 0xcfa32f9b, 0x023c, 0x420e, { 0x90, 0x56, 0x68, 0x32, 0xb7, 0x4f, 0x89, 0xb6 } }), kStructVersion1)
    PCLHelper(PCLMarker m) : BaseStructure(PCLHelper::s_structType, kStructVersion1), marker(m) {};
    PCLMarker get() const { return marker; };
private:
    PCLMarker marker;
SL_STRUCT_END()

}

using PFun_slPCLGetState = sl::Result(sl::PCLState& state);

using PFun_slPCLSetMarker = sl::Result(sl::PCLMarker marker, const sl::FrameToken& frame);

using PFun_slPCLSetOptions = sl::Result(const sl::PCLOptions& options);

inline sl::Result slPCLGetState(sl::PCLState& state)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeaturePCL, slPCLGetState);
    return s_slPCLGetState(state);
}

inline sl::Result slPCLSetMarker(sl::PCLMarker marker, const sl::FrameToken& frame)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeaturePCL, slPCLSetMarker);
    return s_slPCLSetMarker(marker, frame);
}

inline sl::Result slPCLSetOptions(const sl::PCLOptions& options)
{
    SL_FEATURE_FUN_IMPORT_STATIC(sl::kFeaturePCL, slPCLSetOptions);
    return s_slPCLSetOptions(options);
}
