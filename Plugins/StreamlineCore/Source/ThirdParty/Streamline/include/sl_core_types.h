

#pragma once

#include <limits.h>
#include <vector>

#include "sl_struct.h"
#include "sl_consts.h"
#include "sl_version.h"
#include "sl_result.h"
#include "sl_appidentity.h"
#include "sl_device_wrappers.h"

typedef struct ID3D11Resource   ID3D11Resource;
typedef struct ID3D11Buffer     ID3D11Buffer;
typedef struct ID3D11Texture2D  ID3D11Texture2D;
typedef struct ID3D12Resource   ID3D12Resource;

#ifdef VK_VERSION_1_0
using SL_VKResult = VkResult;
#else
using SL_VKResult = int;
#endif
using HRESULT = long;

namespace sl {

using CommandBuffer = void;
using Device = void;

using BufferType = uint32_t;

namespace test
{
constexpr void AbiValidation();
}

constexpr BufferType kBufferTypeDepth = 0;

constexpr BufferType kBufferTypeMotionVectors = 1;

constexpr BufferType kBufferTypeHUDLessColor = 2;

constexpr BufferType kBufferTypeScalingInputColor = 3;

constexpr BufferType kBufferTypeScalingOutputColor = 4;

constexpr BufferType kBufferTypeNormals = 5;

constexpr BufferType kBufferTypeRoughness = 6;

constexpr BufferType kBufferTypeAlbedo = 7;

constexpr BufferType kBufferTypeSpecularAlbedo = 8;

constexpr BufferType kBufferTypeIndirectAlbedo = 9;

constexpr BufferType kBufferTypeSpecularMotionVectors = 10;

constexpr BufferType kBufferTypeDisocclusionMask = 11;

constexpr BufferType kBufferTypeEmissive = 12;

constexpr BufferType kBufferTypeExposure = 13;

constexpr BufferType kBufferTypeNormalRoughness = 14;

constexpr BufferType kBufferTypeDiffuseHitNoisy = 15;

constexpr BufferType kBufferTypeDiffuseHitDenoised = 16;

constexpr BufferType kBufferTypeSpecularHitNoisy = 17;

constexpr BufferType kBufferTypeSpecularHitDenoised = 18;

constexpr BufferType kBufferTypeShadowNoisy = 19;

constexpr BufferType kBufferTypeShadowDenoised = 20;

constexpr BufferType kBufferTypeAmbientOcclusionNoisy = 21;

constexpr BufferType kBufferTypeAmbientOcclusionDenoised = 22;

constexpr BufferType kBufferTypeUIColorAndAlpha = 23;

constexpr BufferType kBufferTypeShadowHint = 24;

constexpr BufferType kBufferTypeReflectionHint = 25;

constexpr BufferType kBufferTypeParticleHint = 26;

constexpr BufferType kBufferTypeTransparencyHint = 27;

constexpr BufferType kBufferTypeAnimatedTextureHint = 28;

constexpr BufferType kBufferTypeBiasCurrentColorHint = 29;

constexpr BufferType kBufferTypeRaytracingDistance = 30;

constexpr BufferType kBufferTypeReflectionMotionVectors = 31;

constexpr BufferType kBufferTypePosition = 32;

constexpr BufferType kBufferTypeInvalidDepthMotionHint = 33;

constexpr BufferType kBufferTypeAlpha = 34;

constexpr BufferType kBufferTypeOpaqueColor = 35;

constexpr BufferType kBufferTypeReactiveMaskHint = 36;

constexpr BufferType kBufferTypeTransparencyAndCompositionMaskHint = 37;

constexpr BufferType kBufferTypeReflectedAlbedo = 38;

constexpr BufferType kBufferTypeColorBeforeParticles = 39;

constexpr BufferType kBufferTypeColorBeforeTransparency = 40;

constexpr BufferType kBufferTypeColorBeforeFog = 41;

constexpr BufferType kBufferTypeSpecularHitDistance = 42;

constexpr BufferType kBufferTypeSpecularRayDirectionHitDistance = 43;

constexpr BufferType kBufferTypeSpecularRayDirection = 44;

constexpr BufferType kBufferTypeDiffuseHitDistance = 45;

constexpr BufferType kBufferTypeDiffuseRayDirectionHitDistance = 46;

constexpr BufferType kBufferTypeDiffuseRayDirection = 47;

constexpr BufferType kBufferTypeHiResDepth = 48;

constexpr BufferType kBufferTypeLinearDepth = 49;

constexpr BufferType kBufferTypeBidirectionalDistortionField = 50;

constexpr BufferType kBufferTypeTransparencyLayer = 51;

constexpr BufferType kBufferTypeTransparencyLayerOpacity = 52;

constexpr BufferType kBufferTypeBackbuffer = 53;

constexpr BufferType kBufferTypeNoWarpMask = 54;

constexpr BufferType kBufferTypeColorAfterParticles = 55;

constexpr BufferType kBufferTypeColorAfterTransparency = 56;

constexpr BufferType kBufferTypeColorAfterFog = 57;

constexpr BufferType kBufferTypeScreenSpaceSubsurfaceScatteringGuide = 58;

constexpr BufferType kBufferTypeColorBeforeScreenSpaceSubsurfaceScattering = 59;

constexpr BufferType kBufferTypeColorAfterScreenSpaceSubsurfaceScattering = 60;

constexpr BufferType kBufferTypeScreenSpaceRefractionGuide = 61;

constexpr BufferType kBufferTypeColorBeforeScreenSpaceRefraction = 62;

constexpr BufferType kBufferTypeColorAfterScreenSpaceRefraction = 63;

constexpr BufferType kBufferTypeDepthOfFieldGuide = 64;

constexpr BufferType kBufferTypeColorBeforeDepthOfField = 65;

constexpr BufferType kBufferTypeColorAfterDepthOfField = 66;

constexpr BufferType kBufferTypeScalingOutputAlpha  = 67;

constexpr BufferType kBufferTypeUIAlpha = 68;

using Feature = uint32_t;

constexpr Feature kFeatureDLSS = 0;

constexpr Feature kFeatureNRD_INVALID = 1;

constexpr Feature kFeatureNIS = 2;

constexpr Feature kFeatureReflex = 3;

constexpr Feature kFeaturePCL = 4;

constexpr Feature kFeatureDeepDVC = 5;

constexpr Feature kFeatureLatewarp = 6;

constexpr Feature kFeatureDLSS_G = 1000;

constexpr Feature kFeatureDLSS_RR = 1001;

constexpr Feature kFeatureNvPerf = 1002;

constexpr Feature kFeatureDirectSR = 1003;

constexpr Feature kFeatureImGUI = 9999;

constexpr Feature kFeatureCommon = UINT_MAX;

enum class LogLevel : uint32_t
{

    eOff,

    eDefault,

    eVerbose,

    eCount
};

enum class ResourceType : char
{
    eTex2d,
    eBuffer,
    eCommandQueue,
    eCommandBuffer,
    eCommandPool,
    eFence,
    eSwapchain,
    eHostFence,

    eUnknown,
    eCount
};

SL_STRUCT_BEGIN(ResourceAllocationDesc, StructType({ 0xbb57e5, 0x49a2, 0x4c23, { 0xa5, 0x19, 0xab, 0x92, 0x86, 0xe7, 0x40, 0x14 } }), kStructVersion1)
    ResourceAllocationDesc(ResourceType _type, void* _desc, uint32_t _state, void* _heap) : BaseStructure(ResourceAllocationDesc::s_structType, kStructVersion1), type(_type),desc(_desc),state(_state),heap(_heap){};

    ResourceType type = ResourceType::eTex2d;

    void* desc{};

    uint32_t state = 0;

    void* heap{};

SL_STRUCT_END()

SL_STRUCT_BEGIN(SubresourceRange, StructType({ 0x8d4c316c, 0xd402, 0x4524, { 0x89, 0xa7, 0x14, 0xe7, 0x9e, 0x63, 0x8e, 0x3a } }), kStructVersion1)

    uint32_t aspectMask;

    uint32_t baseMipLevel;

    uint32_t levelCount;

    uint32_t baseArrayLayer;

    uint32_t layerCount;
SL_STRUCT_END()

SL_STRUCT_BEGIN(Resource, StructType({ 0x3a9d70cf, 0x2418, 0x4b72, { 0x83, 0x91, 0x13, 0xf8, 0x72, 0x1c, 0x72, 0x61 } }), kStructVersion1)

    Resource(ResourceType _type, void* _native, void* _mem, void* _view, uint32_t _state = UINT_MAX) : BaseStructure(Resource::s_structType, kStructVersion1), type(_type), native(_native), memory(_mem), view(_view), state(_state){};
    Resource(ResourceType _type, void* _native, uint32_t _state = UINT_MAX) : BaseStructure(Resource::s_structType, kStructVersion1), type(_type), native(_native), state(_state) {};

    inline operator ID3D12Resource* () { return reinterpret_cast<ID3D12Resource*>(native); }
    inline operator ID3D11Resource* () { return reinterpret_cast<ID3D11Resource*>(native); }
    inline operator ID3D11Buffer* () { return reinterpret_cast<ID3D11Buffer*>(native); }
    inline operator ID3D11Texture2D* () { return reinterpret_cast<ID3D11Texture2D*>(native); }

    ResourceType type = ResourceType::eTex2d;

    void* native{};

    void* memory{};

    void* view{};

    uint32_t state = UINT_MAX;

    uint32_t width{};

    uint32_t height{};

    uint32_t nativeFormat{};

    uint32_t mipLevels{};

    uint32_t arrayLayers{};

    uint64_t gpuVirtualAddress{};

    uint32_t flags;

    uint32_t usage{};

    uint32_t reserved{};

SL_STRUCT_END()

enum ResourceLifecycle
{

    eOnlyValidNow,

    eValidUntilPresent,

    eValidUntilEvaluate
};

SL_STRUCT_BEGIN(ResourceTag, StructType({ 0x4c6a5aad, 0xb445, 0x496c, { 0x87, 0xff, 0x1a, 0xf3, 0x84, 0x5b, 0xe6, 0x53 } }), kStructVersion1)
    ResourceTag(Resource* r, BufferType t, ResourceLifecycle l, const Extent* e = nullptr)
        : BaseStructure(ResourceTag::s_structType, kStructVersion1), resource(r), type(t), lifecycle(l)
    {
        if (e) extent = *e;
    };

    Resource* resource{};

    BufferType type{};

    ResourceLifecycle lifecycle{};

    Extent extent{};

SL_STRUCT_END()

SL_STRUCT_BEGIN(PrecisionInfo, StructType({ 0x98f6e9ba, 0x8d16, 0x4831, { 0xa8, 0x2, 0x4d, 0x3b, 0x52, 0xff, 0x26, 0xbf } }), kStructVersion1)

    enum PrecisionFormula : uint32_t
    {
        eNoTransform = 0,           
        eLinearTransform,           
    };

    PrecisionInfo(PrecisionInfo::PrecisionFormula formula, float bias, float scale)
        : BaseStructure(PrecisionInfo::s_structType, kStructVersion1), conversionFormula(formula), bias(bias), scale(scale) {};

    static std::string getPrecisionFormulaAsStr(PrecisionFormula formula)
    {
        switch (formula)
        {
        case eNoTransform:
            return "eNoTransform";
        case eLinearTransform:
            return "eLinearTransform";
        default:
            assert("Invalid PrecisionFormula" && false);
            return "Unknown";
        }
    };

    PrecisionFormula conversionFormula{ eNoTransform };
    float bias{ 0.0f };
    float scale{ 1.0f };

    inline operator bool() const { return conversionFormula != eNoTransform; }
    inline bool operator==(const PrecisionInfo& rhs) const
    {
        return conversionFormula == rhs.conversionFormula && bias == rhs.bias && scale == rhs.scale;
    }
    inline bool operator!=(const PrecisionInfo& rhs) const
    {
        return !operator==(rhs);
    }
SL_STRUCT_END()

using PFun_ResourceAllocateCallback = Resource(const ResourceAllocationDesc* desc, void* device);
using PFun_ResourceReleaseCallback = void(Resource* resource, void* device);

enum class LogType : uint32_t
{

    eInfo,

    eWarn,
    eError,

    eCount
};

using PFun_LogMessageCallback = void(LogType type, const char* msg);

struct APIError
{
    union
    {
        HRESULT hres;
        SL_VKResult vkRes;
    };
};

using PFunOnAPIErrorCallback = void(const APIError& lastError);

enum class PreferenceFlags : uint64_t
{

    eDisableCLStateTracking = 1 << 0,

    eDisableDebugText = 1 << 1,

    eUseManualHooking = 1 << 2,

    eAllowOTA = 1 << 3,

    eBypassOSVersionCheck = 1 << 4,

    eUseDXGIFactoryProxy = 1 << 5,

    eLoadDownloadedPlugins = 1 << 6,

    eUseFrameBasedResourceTagging = 1 << 7,

    eAll = eDisableCLStateTracking | eDisableDebugText | eUseManualHooking | eAllowOTA | eBypassOSVersionCheck | eUseDXGIFactoryProxy | eLoadDownloadedPlugins | eUseFrameBasedResourceTagging
};

SL_ENUM_OPERATORS_64(PreferenceFlags)

SL_STRUCT_BEGIN(Preferences, StructType({ 0x1ca10965, 0xbf8e, 0x432b, { 0x8d, 0xa1, 0x67, 0x16, 0xd8, 0x79, 0xfb, 0x14 } }), kStructVersion1)

    bool showConsole = false;

    LogLevel logLevel = LogLevel::eDefault;

    const wchar_t** pathsToPlugins{};

    uint32_t numPathsToPlugins = 0;

    const wchar_t* pathToLogsAndData{};

    PFun_ResourceAllocateCallback* allocateCallback{};

    PFun_ResourceReleaseCallback* releaseCallback{};

    PFun_LogMessageCallback* logMessageCallback{};

    PreferenceFlags flags = PreferenceFlags::eDisableCLStateTracking | PreferenceFlags::eAllowOTA | PreferenceFlags::eLoadDownloadedPlugins;

    const Feature* featuresToLoad{};

    uint32_t numFeaturesToLoad{};

    uint32_t applicationId{};

    EngineType engine = EngineType::eCustom;

    const char* engineVersion{};

    const char* projectId{};

    RenderAPI renderAPI = RenderAPI::eD3D12;

SL_STRUCT_END()

SL_STRUCT_PROTECTED_BEGIN(FrameToken, StructType({ 0x830a0f35, 0xdb84, 0x4171, { 0xa8, 0x4, 0x59, 0xb2, 0x6, 0x49, 0x9b, 0x18 } }), kStructVersion1)

    virtual operator uint32_t() const = 0;
SL_STRUCT_END()

SL_STRUCT_BEGIN(ViewportHandle, StructType({ 0x171b6435, 0x9b3c, 0x4fc8, { 0x99, 0x94, 0xfb, 0xe5, 0x25, 0x69, 0xaa, 0xa4 } }), kStructVersion1)
    ViewportHandle(uint32_t v) : BaseStructure(ViewportHandle::s_structType, kStructVersion1), value(v) {}
    ViewportHandle(int32_t v) : BaseStructure(ViewportHandle::s_structType, kStructVersion1), value(v) {}
    operator uint32_t() const { return value; }
private:
    uint32_t value = UINT_MAX;
    friend constexpr void sl::test::AbiValidation();
SL_STRUCT_END()

enum class FeatureRequirementFlags : uint32_t
{

    eD3D11Supported = 1 << 0,
    eD3D12Supported = 1 << 1,
    eVulkanSupported = 1 << 2,

    eVSyncOffRequired = 1 << 3,

    eHardwareSchedulingRequired = 1 << 4,

    eAll = eD3D11Supported | eD3D12Supported | eVulkanSupported | eVSyncOffRequired | eHardwareSchedulingRequired
};

SL_ENUM_OPERATORS_32(FeatureRequirementFlags);

SL_STRUCT_BEGIN(FeatureRequirements, StructType({ 0x66714097, 0xac6d, 0x4bc6, { 0x89, 0x15, 0x1e, 0xf, 0x55, 0xa6, 0xb6, 0x1f } }), kStructVersion2)

    FeatureRequirementFlags flags {};

    uint32_t maxNumCPUThreads{};

    uint32_t maxNumViewports{};

    uint32_t numRequiredTags{};
    const BufferType* requiredTags{};

    Version osVersionDetected{};
    Version osVersionRequired{};
    Version driverVersionDetected{};
    Version driverVersionRequired{};

    uint32_t vkNumComputeQueuesRequired{};
    uint32_t vkNumGraphicsQueuesRequired{};

    uint32_t vkNumDeviceExtensions{};
    const char** vkDeviceExtensions{};

    uint32_t vkNumInstanceExtensions{};
    const char** vkInstanceExtensions{};

    uint32_t vkNumFeatures12{};
    const char** vkFeatures12{};

    uint32_t vkNumFeatures13{};
    const char** vkFeatures13{};

    uint32_t vkNumOpticalFlowQueuesRequired{};

SL_STRUCT_END()

SL_STRUCT_BEGIN(FeatureVersion, StructType({ 0x6d5b51f0, 0x76b, 0x486d, { 0x99, 0x95, 0x5a, 0x56, 0x10, 0x43, 0xf5, 0xc1 } }), kStructVersion1)

    Version versionSL{};

    Version versionNGX{};

SL_STRUCT_END()

SL_STRUCT_BEGIN(AdapterInfo, StructType({ 0x677315f, 0xa746, 0x4492, { 0x9f, 0x42, 0xcb, 0x61, 0x42, 0xc9, 0xc3, 0xd4 } }), kStructVersion1)

    uint8_t* deviceLUID {};

    uint32_t deviceLUIDSizeInBytes{};

    void* vkPhysicalDevice{};

SL_STRUCT_END()

struct IAllocator
{
    virtual ~IAllocator() = default;
    virtual void *allocate(uint32_t nBytes) = 0;
    virtual void free(void *p) = 0;
};
template <class T>
struct Array
{
    inline Array() {}
    inline ~Array() { destroy(); }
    inline uint32_t size() const { return m_size; }
    inline void copyFrom(IAllocator *pAllocator, const std::vector<T>& src)
    {

        assert(pAllocator || src.size() == 0);
        destroy();
        if (src.size() == 0) return;
        m_pAllocator = pAllocator;
        m_size = static_cast<uint32_t>(src.size());
        m_pData = (T*)m_pAllocator->allocate(m_size * sizeof(T));
        for (uint32_t i = 0; i < m_size; ++i)
            m_pData[i] = src[i];
    }
    inline void copyTo(std::vector<T>& dst)
    {
        dst.resize(m_size);
        for (uint32_t i = 0; i < m_size; ++i)
            dst[i] = m_pData[i];
    }
    inline T& operator[](uint32_t index)
    {
        assert(index < m_size);
        return m_pData[index];
    }
    inline const T& operator[](uint32_t index) const
    {
        assert(index < m_size);
        return m_pData[index];
    }
    inline void destroy()
    {
        if (m_pData) m_pAllocator->free(m_pData);
        m_pAllocator = nullptr;
        m_pData = nullptr;
        m_size = 0;
    }

    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;
  private:
    T* m_pData{};
    uint32_t m_size{};

    IAllocator *m_pAllocator{};
};

}
