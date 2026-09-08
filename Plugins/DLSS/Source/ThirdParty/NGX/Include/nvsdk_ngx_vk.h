

#ifndef NVSDK_NGX_VK_H
#define NVSDK_NGX_VK_H

#include "nvsdk_ngx_defs.h"
#include "nvsdk_ngx_params.h"
#ifndef __cplusplus
#include <stdbool.h> 
#include <wchar.h> 
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct NVSDK_NGX_ImageViewInfo_VK {
    VkImageView ImageView;
    VkImage Image;
    VkImageSubresourceRange SubresourceRange;
    VkFormat Format;
    unsigned int Width;
    unsigned int Height;
} NVSDK_NGX_ImageViewInfo_VK;

typedef struct NVSDK_NGX_BufferInfo_VK {
    VkBuffer Buffer;
    unsigned int SizeInBytes;
} NVSDK_NGX_BufferInfo_VK;

typedef struct NVSDK_NGX_Resource_VK {
    union {
        NVSDK_NGX_ImageViewInfo_VK ImageViewInfo;
        NVSDK_NGX_BufferInfo_VK BufferInfo;
    } Resource;
    NVSDK_NGX_Resource_VK_Type Type;
    bool ReadWrite;
} NVSDK_NGX_Resource_VK;

NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_RequiredExtensions(unsigned int *OutInstanceExtCount, const char ***OutInstanceExts, unsigned int *OutDeviceExtCount, const char ***OutDeviceExts);

#if defined(NGX_SNIPPET_BUILD)
#ifdef __cplusplus
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init_Ext(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API, const NVSDK_NGX_Parameter *InParameters = nullptr);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init_Ext2(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA = nullptr, PFN_vkGetDeviceProcAddr InGDPA = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API, const NVSDK_NGX_Parameter *InParameters = nullptr);
#else
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init_Ext(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_Parameter *InParameters);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init_Ext2(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_Parameter *InParameters);
#endif
#else
#ifdef __cplusplus
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA = nullptr, PFN_vkGetDeviceProcAddr InGDPA = nullptr, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
#else
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
#endif
#endif 

#if defined(NGX_SNIPPET_BUILD)

#else
#ifdef __cplusplus
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init_with_ProjectID(const char *InProjectId, NVSDK_NGX_EngineType InEngineType, const char *InEngineVersion, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA = nullptr, PFN_vkGetDeviceProcAddr InGDPA = nullptr, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
#else
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_Init_with_ProjectID(const char *InProjectId, NVSDK_NGX_EngineType InEngineType, const char *InEngineVersion, const wchar_t *InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
#endif
#endif 

#ifdef NGX_ENABLE_DEPRECATED_SHUTDOWN
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_Shutdown(void);
#endif
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_Shutdown1(VkDevice InDevice);

#ifdef NGX_ENABLE_DEPRECATED_GET_PARAMETERS

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetParameters(NVSDK_NGX_Parameter **OutParameters);
#endif 

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_AllocateParameters(NVSDK_NGX_Parameter** OutParameters);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_DestroyParameters(NVSDK_NGX_Parameter* InParameters);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter *InParameters, size_t *OutSizeInBytes);

#if defined(NGX_SNIPPET_BUILD)
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_CreateFeature(VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_CreateFeature1(VkDevice InDevice, VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
#else
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_CreateFeature(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_CreateFeature1(VkDevice InDevice, VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
#endif 

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_ReleaseFeature(NVSDK_NGX_Handle *InHandle);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetFeatureRequirements(const VkInstance Instance,
                                                                                  const VkPhysicalDevice PhysicalDevice,
                                                                                  const NVSDK_NGX_FeatureDiscoveryInfo *FeatureDiscoveryInfo,
                                                                                  NVSDK_NGX_FeatureRequirement *OutSupported);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements(const NVSDK_NGX_FeatureDiscoveryInfo *FeatureDiscoveryInfo,
                                                                                                   uint32_t *OutExtensionCount,
                                                                                                   VkExtensionProperties **OutExtensionProperties);

#if defined(NGX_SNIPPET_BUILD)
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements(const VkInstance Instance,
                                                                                                 const VkPhysicalDevice PhysicalDevice,
                                                                                                 const NVSDK_NGX_FeatureDiscoveryInfo *FeatureDiscoveryInfo,
                                                                                                 uint32_t *OutExtensionCount,
                                                                                                 VkExtensionProperties **OutExtensionProperties);
#else
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements(VkInstance Instance,
                                                                                                 VkPhysicalDevice PhysicalDevice,
                                                                                                 const NVSDK_NGX_FeatureDiscoveryInfo *FeatureDiscoveryInfo,
                                                                                                 uint32_t *OutExtensionCount,
                                                                                                 VkExtensionProperties **OutExtensionProperties);
#endif 

#ifdef __cplusplus
typedef void (NVSDK_CONV *PFN_NVSDK_NGX_ProgressCallback)(float InCurrentProgress, bool &OutShouldCancel);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_EvaluateFeature(VkCommandBuffer InCmdList, const NVSDK_NGX_Handle *InFeatureHandle, const NVSDK_NGX_Parameter *InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback = NULL);
#endif
typedef void (NVSDK_CONV *PFN_NVSDK_NGX_ProgressCallback_C)(float InCurrentProgress, bool *OutShouldCancel);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_EvaluateFeature_C(VkCommandBuffer InCmdList, const NVSDK_NGX_Handle *InFeatureHandle, const NVSDK_NGX_Parameter *InParameters, PFN_NVSDK_NGX_ProgressCallback_C InCallback);

#if defined(NGX_SNIPPET_BUILD)

#else

const wchar_t* NVSDK_CONV GetNGXResultAsString(NVSDK_NGX_Result InNGXResult);
#endif

#ifdef __cplusplus
} 
#endif

#endif 
