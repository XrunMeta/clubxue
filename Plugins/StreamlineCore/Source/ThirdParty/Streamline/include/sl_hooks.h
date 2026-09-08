

#pragma once

#include "sl.h"

struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkInstance_T;

using VkPhysicalDevice = VkPhysicalDevice_T*;
using VkDevice = VkDevice_T*;
using VkInstance = VkInstance_T*;

namespace sl
{

enum class FunctionHookID : uint32_t
{

    eIDXGIFactory_CreateSwapChain,
    eIDXGIFactory_CreateSwapChainForHwnd,
    eIDXGIFactory_CreateSwapChainForCoreWindow,

    eIDXGISwapChain_Present,
    eIDXGISwapChain_Present1,
    eIDXGISwapChain_GetBuffer,
    eIDXGISwapChain_GetDesc,
    eIDXGISwapChain_ResizeBuffers,
    eIDXGISwapChain_ResizeBuffers1,
    eIDXGISwapChain_GetCurrentBackBufferIndex,
    eIDXGISwapChain_SetFullscreenState,

    eIDXGISwapChain_Destroyed,

    eID3D12Device_CreateCommandQueue,

    eVulkan_Present,
    eVulkan_CreateSwapchainKHR,
    eVulkan_DestroySwapchainKHR,
    eVulkan_GetSwapchainImagesKHR,
    eVulkan_AcquireNextImageKHR,
    eVulkan_DeviceWaitIdle,
    eVulkan_CreateWin32SurfaceKHR,
    eVulkan_DestroySurfaceKHR,

    eMaxNum
};

#ifndef SL_CASE_STR
#define SL_CASE_STR(a) case a : return #a;
#endif

inline const char* getFunctionHookIDAsStr(FunctionHookID v)
{
    switch (v)
    {
        SL_CASE_STR(FunctionHookID::eIDXGIFactory_CreateSwapChain);
        SL_CASE_STR(FunctionHookID::eIDXGIFactory_CreateSwapChainForHwnd);
        SL_CASE_STR(FunctionHookID::eIDXGIFactory_CreateSwapChainForCoreWindow);
        SL_CASE_STR(FunctionHookID::eIDXGISwapChain_Present);
        SL_CASE_STR(FunctionHookID::eIDXGISwapChain_Present1);
        SL_CASE_STR(FunctionHookID::eIDXGISwapChain_GetBuffer);
        SL_CASE_STR(FunctionHookID::eIDXGISwapChain_GetDesc);
        SL_CASE_STR(FunctionHookID::eIDXGISwapChain_ResizeBuffers);
        SL_CASE_STR(FunctionHookID::eIDXGISwapChain_ResizeBuffers1);
        SL_CASE_STR(FunctionHookID::eIDXGISwapChain_GetCurrentBackBufferIndex);
        SL_CASE_STR(FunctionHookID::eIDXGISwapChain_SetFullscreenState);
        SL_CASE_STR(FunctionHookID::eIDXGISwapChain_Destroyed);
        SL_CASE_STR(FunctionHookID::eID3D12Device_CreateCommandQueue);
        SL_CASE_STR(FunctionHookID::eVulkan_Present);
        SL_CASE_STR(FunctionHookID::eVulkan_CreateSwapchainKHR);
        SL_CASE_STR(FunctionHookID::eVulkan_DestroySwapchainKHR);
        SL_CASE_STR(FunctionHookID::eVulkan_GetSwapchainImagesKHR);
        SL_CASE_STR(FunctionHookID::eVulkan_AcquireNextImageKHR);
        SL_CASE_STR(FunctionHookID::eVulkan_DeviceWaitIdle);
        SL_CASE_STR(FunctionHookID::eVulkan_CreateWin32SurfaceKHR);
        SL_CASE_STR(FunctionHookID::eVulkan_DestroySurfaceKHR);
        case FunctionHookID::eMaxNum: break;
    };
    return "Unknown";
}

} 
