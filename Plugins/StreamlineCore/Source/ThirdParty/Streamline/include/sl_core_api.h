

#pragma once

#include <limits.h>

#include "sl_struct.h"
#include "sl_consts.h"
#include "sl_version.h"
#include "sl_result.h"
#include "sl_appidentity.h"
#include "sl_device_wrappers.h"

#include "sl_core_types.h"

#if defined(SL_INTERPOSER)
    #if defined(_WIN32)
        #define SL_API extern "C" __declspec(dllexport)
    #else
        #error Unsupported Platform!
    #endif
#else
#define SL_API extern "C"
#endif

#pragma region SL_API

using PFun_slInit = sl::Result(const sl::Preferences& pref, uint64_t sdkVersion);
using PFun_slShutdown = sl::Result();
using PFun_slIsFeatureSupported = sl::Result(sl::Feature feature, const sl::AdapterInfo& adapterInfo);
using PFun_slIsFeatureLoaded = sl::Result(sl::Feature feature, bool& loaded);
using PFun_slSetFeatureLoaded = sl::Result(sl::Feature feature, bool loaded);
using PFun_slEvaluateFeature = sl::Result(sl::Feature feature, const sl::FrameToken& frame, const sl::BaseStructure** inputs, uint32_t numInputs, sl::CommandBuffer* cmdBuffer);
using PFun_slAllocateResources = sl::Result(sl::CommandBuffer* cmdBuffer, sl::Feature feature, const sl::ViewportHandle& viewport);
using PFun_slFreeResources = sl::Result(sl::Feature feature, const sl::ViewportHandle& viewport);
using PFun_slSetTag
#if __cplusplus >= 201402L
[[deprecated("Use the version of this function that takes a sl::FrameToken instead - slSetTagForFrame and set sl::PreferenceFlags::eUseFrameBasedResourceTagging.")]]
#endif
= sl::Result(const sl::ViewportHandle& viewport, const sl::ResourceTag* tags, uint32_t numTags, sl::CommandBuffer* cmdBuffer);
using PFun_slSetTagForFrame = sl::Result(const sl::FrameToken& frame, const sl::ViewportHandle& viewport, const sl::ResourceTag* tags, uint32_t numTags, sl::CommandBuffer* cmdBuffer);
using PFun_slGetFeatureRequirements = sl::Result(sl::Feature feature, sl::FeatureRequirements& requirements);
using PFun_slGetFeatureVersion = sl::Result(sl::Feature feature, sl::FeatureVersion& version);
using PFun_slUpgradeInterface = sl::Result(void** baseInterface);
using PFun_slSetConstants = sl::Result(const sl::Constants& values, const sl::FrameToken& frame, const sl::ViewportHandle& viewport);
using PFun_slGetNativeInterface = sl::Result(void* proxyInterface, void** baseInterface);
using PFun_slGetFeatureFunction = sl::Result(sl::Feature feature, const char* functionName, void*& function);
using PFun_slGetNewFrameToken = sl::Result(sl::FrameToken*& token, const uint32_t* frameIndex);
using PFun_slSetD3DDevice = sl::Result(void* d3dDevice);

SL_API sl::Result slInit(const sl::Preferences &pref, uint64_t sdkVersion = sl::kSDKVersion);

SL_API sl::Result slShutdown();

SL_API sl::Result slIsFeatureSupported(sl::Feature feature, const sl::AdapterInfo& adapterInfo);

SL_API sl::Result slIsFeatureLoaded(sl::Feature feature, bool& loaded);

SL_API sl::Result slSetFeatureLoaded(sl::Feature feature, bool loaded);

SL_API sl::Result slSetTagForFrame(const sl::FrameToken& frame, const sl::ViewportHandle& viewport, const sl::ResourceTag* resources, uint32_t numResources, sl::CommandBuffer* cmdBuffer);

SL_API
#if __cplusplus >= 201402L
[[deprecated("Use the version of this function that takes a sl::FrameToken instead - slSetTagForFrame and set sl::PreferenceFlags::eUseFrameBasedResourceTagging.")]]
#endif
sl::Result slSetTag(const sl::ViewportHandle& viewport, const sl::ResourceTag* tags, uint32_t numTags, sl::CommandBuffer* cmdBuffer);

SL_API sl::Result slSetConstants(const sl::Constants& values, const sl::FrameToken& frame, const sl::ViewportHandle& viewport);

SL_API sl::Result slGetFeatureRequirements(sl::Feature feature, sl::FeatureRequirements& requirements);

SL_API sl::Result slGetFeatureVersion(sl::Feature feature, sl::FeatureVersion& version);

SL_API sl::Result slAllocateResources(sl::CommandBuffer* cmdBuffer, sl::Feature feature, const sl::ViewportHandle& viewport);

SL_API sl::Result slFreeResources(sl::Feature feature, const sl::ViewportHandle& viewport);

SL_API sl::Result slEvaluateFeature(sl::Feature feature, const sl::FrameToken& frame, const sl::BaseStructure** inputs, uint32_t numInputs, sl::CommandBuffer* cmdBuffer);

SL_API sl::Result slUpgradeInterface(void** baseInterface);

SL_API sl::Result slGetNativeInterface(void* proxyInterface, void** baseInterface);

SL_API sl::Result slGetFeatureFunction(sl::Feature feature, const char* functionName, void*& function);

SL_API sl::Result slGetNewFrameToken(sl::FrameToken*& token, const uint32_t* frameIndex = nullptr);

SL_API sl::Result slSetD3DDevice(void* d3dDevice);

#pragma endregion SL_API
