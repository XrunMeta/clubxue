

#ifndef NVSDK_NGX_H
#define NVSDK_NGX_H

#include <stddef.h> 

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

typedef struct IUnknown IUnknown;

typedef struct IDXGIAdapter              IDXGIAdapter;
typedef struct ID3D11Device              ID3D11Device;
typedef struct ID3D11Resource            ID3D11Resource;
typedef struct ID3D11DeviceContext       ID3D11DeviceContext;
typedef struct D3D11_TEXTURE2D_DESC      D3D11_TEXTURE2D_DESC;
typedef struct D3D11_BUFFER_DESC         D3D11_BUFFER_DESC;
typedef struct ID3D11Buffer              ID3D11Buffer;
typedef struct ID3D11Texture2D           ID3D11Texture2D;

typedef struct ID3D12Device              ID3D12Device;
typedef struct ID3D12Resource            ID3D12Resource;
typedef struct ID3D12GraphicsCommandList ID3D12GraphicsCommandList;
typedef struct D3D12_RESOURCE_DESC       D3D12_RESOURCE_DESC;
typedef struct CD3DX12_HEAP_PROPERTIES   CD3DX12_HEAP_PROPERTIES;

typedef struct NVSDK_NGX_CUDADevice      NVSDK_NGX_CUDADevice;

typedef void (NVSDK_CONV *PFN_NVSDK_NGX_D3D12_ResourceAllocCallback)(D3D12_RESOURCE_DESC *InDesc, int InState, CD3DX12_HEAP_PROPERTIES *InHeap, ID3D12Resource **OutResource);
typedef void (NVSDK_CONV *PFN_NVSDK_NGX_D3D11_BufferAllocCallback)(D3D11_BUFFER_DESC *InDesc, ID3D11Buffer **OutResource);
typedef void (NVSDK_CONV *PFN_NVSDK_NGX_D3D11_Tex2DAllocCallback)(D3D11_TEXTURE2D_DESC *InDesc, ID3D11Texture2D **OutResource);
typedef void (NVSDK_CONV *PFN_NVSDK_NGX_ResourceReleaseCallback)(IUnknown *InResource);

typedef unsigned long long CUtexObject;

#if defined(NGX_SNIPPET_BUILD)
#ifdef __cplusplus
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D11Device *InDevice, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D11Device *InDevice, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API, const NVSDK_NGX_Parameter* InParameters = nullptr);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D12Device *InDevice, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D12Device *InDevice, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API, const NVSDK_NGX_Parameter* InParameters = nullptr);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init_Ext(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API, const NVSDK_NGX_Parameter* InParameters = nullptr);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init_Ext1(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, NVSDK_NGX_CUDADevice *InDevice, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API, const NVSDK_NGX_Parameter* InParameters = nullptr);
#else
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D11Device *InDevice, NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D11Device *InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_Parameter* InParameters);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D12Device *InDevice, NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D12Device *InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_Parameter* InParameters);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init_Ext(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_Parameter* InParameters);
NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init_Ext1(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, NVSDK_NGX_CUDADevice *InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_Parameter* InParameters);
#endif
#else
#ifdef __cplusplus
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D11Device *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D12Device *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init1(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, NVSDK_NGX_CUDADevice *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
#else
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D11Device *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, ID3D12Device *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init1(unsigned long long InApplicationId, const wchar_t *InApplicationDataPath, NVSDK_NGX_CUDADevice *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
#endif
#endif 

#if defined(NGX_SNIPPET_BUILD)

#else
#ifdef __cplusplus
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D11_Init_with_ProjectID(const char *InProjectId, NVSDK_NGX_EngineType InEngineType, const char *InEngineVersion, const wchar_t *InApplicationDataPath, ID3D11Device *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D12_Init_with_ProjectID(const char *InProjectId, NVSDK_NGX_EngineType InEngineType, const char *InEngineVersion, const wchar_t *InApplicationDataPath, ID3D12Device *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init_with_ProjectID(const char *InProjectId, NVSDK_NGX_EngineType InEngineType, const char *InEngineVersion, const wchar_t *InApplicationDataPath, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo = nullptr, NVSDK_NGX_Version InSDKVersion = NVSDK_NGX_Version_API);
#else
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D11_Init_with_ProjectID(const char *InProjectId, NVSDK_NGX_EngineType InEngineType, const char *InEngineVersion, const wchar_t *InApplicationDataPath, ID3D11Device *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_D3D12_Init_with_ProjectID(const char *InProjectId, NVSDK_NGX_EngineType InEngineType, const char *InEngineVersion, const wchar_t *InApplicationDataPath, ID3D12Device *InDevice, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_CUDA_Init_with_ProjectID(const char *InProjectId, NVSDK_NGX_EngineType InEngineType, const char *InEngineVersion, const wchar_t *InApplicationDataPath, const NVSDK_NGX_FeatureCommonInfo *InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
#endif
#endif 

#ifdef NGX_ENABLE_DEPRECATED_SHUTDOWN
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown(void);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_Shutdown(void);
#endif
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown1(ID3D11Device *InDevice);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_Shutdown1(ID3D12Device *InDevice);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_Shutdown(void);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_Shutdown1(NVSDK_NGX_CUDADevice *InDevice);

#ifdef NGX_ENABLE_DEPRECATED_GET_PARAMETERS

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter **OutParameters);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_GetParameters(NVSDK_NGX_Parameter **OutParameters);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_GetParameters(NVSDK_NGX_Parameter **OutParameters);
#endif 

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_AllocateParameters(NVSDK_NGX_Parameter** OutParameters);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_AllocateParameters(NVSDK_NGX_Parameter** OutParameters);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_AllocateParameters(NVSDK_NGX_Parameter** OutParameters);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_DestroyParameters(NVSDK_NGX_Parameter* InParameters);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_DestroyParameters(NVSDK_NGX_Parameter* InParameters);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_DestroyParameters(NVSDK_NGX_Parameter* InParameters);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter *InParameters, size_t *OutSizeInBytes);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter *InParameters, size_t *OutSizeInBytes);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter *InParameters, size_t *OutSizeInBytes);

#if defined(NGX_SNIPPET_BUILD)
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_CreateFeature(ID3D11DeviceContext *InDevCtx, NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_CreateFeature(ID3D12GraphicsCommandList *InCmdList, NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_CreateFeature(NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_CreateFeature1(NVSDK_NGX_CUDADevice *InDevice, NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
#else
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_CreateFeature(ID3D11DeviceContext *InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_CreateFeature(ID3D12GraphicsCommandList *InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_CreateFeature(NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_CreateFeature1(NVSDK_NGX_CUDADevice *InDevice, NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter *InParameters, NVSDK_NGX_Handle **OutHandle);

#endif 

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_ReleaseFeature(NVSDK_NGX_Handle *InHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_ReleaseFeature(NVSDK_NGX_Handle *InHandle);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_ReleaseFeature(NVSDK_NGX_Handle *InHandle);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_GetFeatureRequirements(IDXGIAdapter *Adapter,
                                                                                 const NVSDK_NGX_FeatureDiscoveryInfo *FeatureDiscoveryInfo,
                                                                                 NVSDK_NGX_FeatureRequirement *OutSupported);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_GetFeatureRequirements(IDXGIAdapter *Adapter,
                                                                                 const NVSDK_NGX_FeatureDiscoveryInfo *FeatureDiscoveryInfo,
                                                                                 NVSDK_NGX_FeatureRequirement *OutSupported);

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_GetFeatureRequirements(int CudaDevice,
                                                                                const NVSDK_NGX_FeatureDiscoveryInfo *FeatureDiscoveryInfo,
                                                                                NVSDK_NGX_FeatureRequirement *OutSupported);

#ifdef __cplusplus
typedef void (NVSDK_CONV *PFN_NVSDK_NGX_ProgressCallback)(float InCurrentProgress, bool &OutShouldCancel);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_EvaluateFeature(ID3D11DeviceContext *InDevCtx, const NVSDK_NGX_Handle *InFeatureHandle, const NVSDK_NGX_Parameter *InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback = NULL);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_EvaluateFeature(ID3D12GraphicsCommandList *InCmdList, const NVSDK_NGX_Handle *InFeatureHandle, const NVSDK_NGX_Parameter *InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback = NULL);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_EvaluateFeature(const NVSDK_NGX_Handle *InFeatureHandle, const NVSDK_NGX_Parameter *InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback = NULL);
#endif

typedef void (NVSDK_CONV *PFN_NVSDK_NGX_ProgressCallback_C)(float InCurrentProgress, bool *OutShouldCancel);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_EvaluateFeature_C(ID3D11DeviceContext *InDevCtx, const NVSDK_NGX_Handle *InFeatureHandle, const NVSDK_NGX_Parameter *InParameters, PFN_NVSDK_NGX_ProgressCallback_C InCallback);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_EvaluateFeature_C(ID3D12GraphicsCommandList *InCmdList, const NVSDK_NGX_Handle *InFeatureHandle, const NVSDK_NGX_Parameter *InParameters, PFN_NVSDK_NGX_ProgressCallback_C InCallback);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_CUDA_EvaluateFeature_C(const NVSDK_NGX_Handle *InFeatureHandle, const NVSDK_NGX_Parameter *InParameters, PFN_NVSDK_NGX_ProgressCallback_C InCallback);

#if defined(NGX_SNIPPET_BUILD)

#else
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_UpdateFeature(const NVSDK_NGX_Application_Identifier *ApplicationId, const NVSDK_NGX_Feature FeatureID);
#endif 

const wchar_t* NVSDK_CONV GetNGXResultAsString(NVSDK_NGX_Result InNGXResult);

#ifdef __cplusplus
} 
#endif

#endif 
