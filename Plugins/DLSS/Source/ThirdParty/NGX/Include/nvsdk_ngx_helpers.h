

#ifndef NVSDK_NGX_HELPERS_H
#define NVSDK_NGX_HELPERS_H
#pragma once

#include "nvsdk_ngx.h"
#include "nvsdk_ngx_defs.h"

typedef NVSDK_NGX_Result(NVSDK_CONV *PFN_NVSDK_NGX_DLSS_GetStatsCallback)(NVSDK_NGX_Parameter *InParams);

static inline NVSDK_NGX_Result NGX_DLSS_GET_STATS_2(
    NVSDK_NGX_Parameter *pInParams,
    unsigned long long *pVRAMAllocatedBytes,
    unsigned int *pOptLevel, unsigned int *IsDevSnippetBranch)
{
    void *Callback = NULL;
    NVSDK_NGX_Parameter_GetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSGetStatsCallback, &Callback);
    if (!Callback)
    {

        return NVSDK_NGX_Result_FAIL_OutOfDate;
    }

    NVSDK_NGX_Result Res = NVSDK_NGX_Result_Success;
    PFN_NVSDK_NGX_DLSS_GetStatsCallback PFNCallback = (PFN_NVSDK_NGX_DLSS_GetStatsCallback)Callback;
    Res = PFNCallback(pInParams);
    if (NVSDK_NGX_FAILED(Res))
    {
        return Res;
    }
    NVSDK_NGX_Parameter_GetULL(pInParams, NVSDK_NGX_Parameter_SizeInBytes, pVRAMAllocatedBytes);
    NVSDK_NGX_Parameter_GetUI(pInParams, NVSDK_NGX_EParameter_OptLevel, pOptLevel);
    NVSDK_NGX_Parameter_GetUI(pInParams, NVSDK_NGX_EParameter_IsDevSnippetBranch, IsDevSnippetBranch);
    return Res;
}

static inline NVSDK_NGX_Result NGX_DLSS_GET_STATS_1(
    NVSDK_NGX_Parameter *pInParams,
    unsigned long long *pVRAMAllocatedBytes,
    unsigned int *pOptLevel)
{
    unsigned int dummy = 0;
    return NGX_DLSS_GET_STATS_2(pInParams, pVRAMAllocatedBytes, pOptLevel, &dummy);
}

static inline NVSDK_NGX_Result NGX_DLSS_GET_STATS(
    NVSDK_NGX_Parameter *pInParams,
    unsigned long long *pVRAMAllocatedBytes)
{
    unsigned int dummy = 0;
    return NGX_DLSS_GET_STATS_2(pInParams, pVRAMAllocatedBytes, &dummy, &dummy);
}

typedef NVSDK_NGX_Result(NVSDK_CONV *PFN_NVSDK_NGX_DLSS_GetOptimalSettingsCallback)(NVSDK_NGX_Parameter *InParams);

static inline NVSDK_NGX_Result NGX_DLSS_GET_OPTIMAL_SETTINGS(
    NVSDK_NGX_Parameter *pInParams,
    unsigned int InUserSelectedWidth,
    unsigned int InUserSelectedHeight,
    NVSDK_NGX_PerfQuality_Value InPerfQualityValue,
    unsigned int *pOutRenderOptimalWidth,
    unsigned int *pOutRenderOptimalHeight,
    unsigned int *pOutRenderMaxWidth,
    unsigned int *pOutRenderMaxHeight,
    unsigned int *pOutRenderMinWidth,
    unsigned int *pOutRenderMinHeight,
    float *pOutSharpness)
{
    void *Callback = NULL;
    NVSDK_NGX_Parameter_GetVoidPointer(pInParams, NVSDK_NGX_Parameter_DLSSOptimalSettingsCallback, &Callback);
    if (!Callback)
    {

        return NVSDK_NGX_Result_FAIL_OutOfDate;
    }

    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, InUserSelectedWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, InUserSelectedHeight);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_PerfQualityValue, InPerfQualityValue);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_RTXValue, false); 

    NVSDK_NGX_Result Res = NVSDK_NGX_Result_Success;
    PFN_NVSDK_NGX_DLSS_GetOptimalSettingsCallback PFNCallback = (PFN_NVSDK_NGX_DLSS_GetOptimalSettingsCallback)Callback;
    Res = PFNCallback(pInParams);
    if (NVSDK_NGX_FAILED(Res))
    {
        return Res;
    }
    NVSDK_NGX_Parameter_GetUI(pInParams, NVSDK_NGX_Parameter_OutWidth, pOutRenderOptimalWidth);
    NVSDK_NGX_Parameter_GetUI(pInParams, NVSDK_NGX_Parameter_OutHeight, pOutRenderOptimalHeight);

    *pOutRenderMaxWidth  = *pOutRenderOptimalWidth;
    *pOutRenderMaxHeight = *pOutRenderOptimalHeight;
    *pOutRenderMinWidth  = *pOutRenderOptimalWidth;
    *pOutRenderMinHeight = *pOutRenderOptimalHeight;
    NVSDK_NGX_Parameter_GetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Width,  pOutRenderMaxWidth);
    NVSDK_NGX_Parameter_GetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Height, pOutRenderMaxHeight);
    NVSDK_NGX_Parameter_GetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Width,  pOutRenderMinWidth);
    NVSDK_NGX_Parameter_GetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Height, pOutRenderMinHeight);
    NVSDK_NGX_Parameter_GetF(pInParams, NVSDK_NGX_Parameter_Sharpness, pOutSharpness);
    return Res;
}

typedef struct NVSDK_NGX_D3D11_Feature_Eval_Params
{
    ID3D11Resource*  pInColor;
    ID3D11Resource*  pInOutput;

    float       InSharpness;
} NVSDK_NGX_D3D11_Feature_Eval_Params;

typedef struct NVSDK_NGX_CUDA_Feature_Eval_Params
{
    CUtexObject* pInColor;
    CUtexObject* pInOutput;

    float       InSharpness;
} NVSDK_NGX_CUDA_Feature_Eval_Params;

typedef struct NVSDK_NGX_D3D11_GBuffer
{
    ID3D11Resource* pInAttrib[NVSDK_NGX_GBUFFERTYPE_NUM];
} NVSDK_NGX_D3D11_GBuffer;

typedef struct NVSDK_NGX_D3D11_DLSS_Eval_Params
{
    NVSDK_NGX_D3D11_Feature_Eval_Params Feature;
    ID3D11Resource*                     pInDepth;
    ID3D11Resource*                     pInMotionVectors;
    float                               InJitterOffsetX;     
    float                               InJitterOffsetY;
    NVSDK_NGX_Dimensions                InRenderSubrectDimensions;

    int                                 InReset;             
    float                               InMVScaleX;          
    float                               InMVScaleY;
    ID3D11Resource*                     pInTransparencyMask; 
    ID3D11Resource*                     pInExposureTexture;
    ID3D11Resource*                     pInBiasCurrentColorMask;
    NVSDK_NGX_Coordinates               InColorSubrectBase;
    NVSDK_NGX_Coordinates               InDepthSubrectBase;
    NVSDK_NGX_Coordinates               InMVSubrectBase;
    NVSDK_NGX_Coordinates               InTranslucencySubrectBase;
    NVSDK_NGX_Coordinates               InBiasCurrentColorSubrectBase;
    NVSDK_NGX_Coordinates               InOutputSubrectBase;
    float                               InPreExposure;
    float                               InExposureScale;
    int                                 InIndicatorInvertXAxis;
    int                                 InIndicatorInvertYAxis;

    NVSDK_NGX_D3D11_GBuffer             GBufferSurface;
    NVSDK_NGX_ToneMapperType            InToneMapperType;
    ID3D11Resource*                     pInMotionVectors3D;
    ID3D11Resource*                     pInIsParticleMask; 
    ID3D11Resource*                     pInAnimatedTextureMask; 
    ID3D11Resource*                     pInDepthHighRes;
    ID3D11Resource*                     pInPositionViewSpace;
    float                               InFrameTimeDeltaInMsec; 
    ID3D11Resource*                     pInRayTracingHitDistance; 
    ID3D11Resource*                     pInMotionVectorsReflections; 
} NVSDK_NGX_D3D11_DLSS_Eval_Params;

typedef struct NVSDK_NGX_D3D11_DLISP_Eval_Params
{
    NVSDK_NGX_D3D11_Feature_Eval_Params Feature;

    unsigned int                    InRectX;
    unsigned int                    InRectY;
    unsigned int                    InRectW;
    unsigned int                    InRectH;
    float                           InDenoise;
} NVSDK_NGX_D3D11_DLISP_Eval_Params;

typedef struct NVSDK_NGX_CUDA_DLISP_Eval_Params
{
    NVSDK_NGX_CUDA_Feature_Eval_Params Feature;

    unsigned int                    InRectX;
    unsigned int                    InRectY;
    unsigned int                    InRectW;
    unsigned int                    InRectH;
    float                           InDenoise;
} NVSDK_NGX_CUDA_DLISP_Eval_Params;

static inline NVSDK_NGX_Result NGX_D3D11_CREATE_DLSS_EXT(
    ID3D11DeviceContext *pInCtx,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_DLSS_Create_Params *pInDlssCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, pInDlssCreateParams->Feature.InWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, pInDlssCreateParams->Feature.InHeight);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutWidth, pInDlssCreateParams->Feature.InTargetWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutHeight, pInDlssCreateParams->Feature.InTargetHeight);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_PerfQualityValue, pInDlssCreateParams->Feature.InPerfQualityValue);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, pInDlssCreateParams->InFeatureCreateFlags);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Enable_Output_Subrects, pInDlssCreateParams->InEnableOutputSubrects ? 1 : 0);

    return NVSDK_NGX_D3D11_CreateFeature(pInCtx, NVSDK_NGX_Feature_SuperSampling, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_D3D11_EVALUATE_DLSS_EXT(
    ID3D11DeviceContext *pInCtx,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_D3D11_DLSS_Eval_Params *pInDlssEvalParams)
{
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_Color, pInDlssEvalParams->Feature.pInColor);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_Output, pInDlssEvalParams->Feature.pInOutput);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_Depth, pInDlssEvalParams->pInDepth);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_MotionVectors, pInDlssEvalParams->pInMotionVectors);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Jitter_Offset_X, pInDlssEvalParams->InJitterOffsetX);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Jitter_Offset_Y, pInDlssEvalParams->InJitterOffsetY);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Sharpness, pInDlssEvalParams->Feature.InSharpness);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_Reset, pInDlssEvalParams->InReset);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_MV_Scale_X, pInDlssEvalParams->InMVScaleX == 0.0f ? 1.0f : pInDlssEvalParams->InMVScaleX);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_MV_Scale_Y, pInDlssEvalParams->InMVScaleY == 0.0f ? 1.0f : pInDlssEvalParams->InMVScaleY);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_TransparencyMask, pInDlssEvalParams->pInTransparencyMask);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_ExposureTexture, pInDlssEvalParams->pInExposureTexture);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, pInDlssEvalParams->pInBiasCurrentColorMask);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Albedo, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_ALBEDO]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Roughness, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_ROUGHNESS]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Metallic, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_METALLIC]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Specular, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SPECULAR]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Subsurface, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SUBSURFACE]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Normals, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_NORMALS]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_ShadingModelId, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SHADINGMODELID]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_MaterialId, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_MATERIALID]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_8, pInDlssEvalParams->GBufferSurface.pInAttrib[8]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_9, pInDlssEvalParams->GBufferSurface.pInAttrib[9]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_10, pInDlssEvalParams->GBufferSurface.pInAttrib[10]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_11, pInDlssEvalParams->GBufferSurface.pInAttrib[11]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_12, pInDlssEvalParams->GBufferSurface.pInAttrib[12]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_13, pInDlssEvalParams->GBufferSurface.pInAttrib[13]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_14, pInDlssEvalParams->GBufferSurface.pInAttrib[14]);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_15, pInDlssEvalParams->GBufferSurface.pInAttrib[15]);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_TonemapperType, pInDlssEvalParams->InToneMapperType);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_MotionVectors3D, pInDlssEvalParams->pInMotionVectors3D);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_IsParticleMask, pInDlssEvalParams->pInIsParticleMask);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_AnimatedTextureMask, pInDlssEvalParams->pInAnimatedTextureMask);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_DepthHighRes, pInDlssEvalParams->pInDepthHighRes);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_Position_ViewSpace, pInDlssEvalParams->pInPositionViewSpace);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, pInDlssEvalParams->InFrameTimeDeltaInMsec);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_RayTracingHitDistance, pInDlssEvalParams->pInRayTracingHitDistance);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_MotionVectorsReflection, pInDlssEvalParams->pInMotionVectorsReflections);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X, pInDlssEvalParams->InColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y, pInDlssEvalParams->InColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_X, pInDlssEvalParams->InDepthSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_Y, pInDlssEvalParams->InDepthSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_X, pInDlssEvalParams->InMVSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_Y, pInDlssEvalParams->InMVSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_X, pInDlssEvalParams->InTranslucencySubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_Y, pInDlssEvalParams->InTranslucencySubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_X, pInDlssEvalParams->InBiasCurrentColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_Y, pInDlssEvalParams->InBiasCurrentColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_X, pInDlssEvalParams->InOutputSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_Y, pInDlssEvalParams->InOutputSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width , pInDlssEvalParams->InRenderSubrectDimensions.Width);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, pInDlssEvalParams->InRenderSubrectDimensions.Height);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DLSS_Pre_Exposure, pInDlssEvalParams->InPreExposure == 0.0f ? 1.0f : pInDlssEvalParams->InPreExposure);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DLSS_Exposure_Scale, pInDlssEvalParams->InExposureScale == 0.0f ? 1.0f : pInDlssEvalParams->InExposureScale);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Indicator_Invert_X_Axis, pInDlssEvalParams->InIndicatorInvertXAxis);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Indicator_Invert_Y_Axis, pInDlssEvalParams->InIndicatorInvertYAxis);

    return NVSDK_NGX_D3D11_EvaluateFeature_C(pInCtx, pInHandle, pInParams, NULL);
}

static inline NVSDK_NGX_Result NGX_D3D11_CREATE_DLISP_EXT(
    ID3D11DeviceContext *pInCtx,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_Feature_Create_Params *pDlispCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, pDlispCreateParams->InWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, pDlispCreateParams->InHeight);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutWidth, pDlispCreateParams->InTargetWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutHeight, pDlispCreateParams->InTargetHeight);

    return NVSDK_NGX_D3D11_CreateFeature(pInCtx, NVSDK_NGX_Feature_ImageSignalProcessing, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_CUDA_CREATE_DLISP_EXT(
    NVSDK_NGX_Handle** ppOutHandle,
    NVSDK_NGX_Parameter* pInParams,
    NVSDK_NGX_Feature_Create_Params* pDlispCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, pDlispCreateParams->InWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, pDlispCreateParams->InHeight);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutWidth, pDlispCreateParams->InTargetWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutHeight, pDlispCreateParams->InTargetHeight);

    return NVSDK_NGX_CUDA_CreateFeature(NVSDK_NGX_Feature_ImageSignalProcessing, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_D3D11_EVALUATE_DLISP_EXT(
    ID3D11DeviceContext *pInCtx,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_D3D11_DLISP_Eval_Params *pDlispEvalParams)
{
    if (pDlispEvalParams->Feature.InSharpness < 0.0f || pDlispEvalParams->Feature.InSharpness > 1.0f || pDlispEvalParams->InDenoise < 0.0f || pDlispEvalParams->InDenoise > 1.0f)
    {
        return NVSDK_NGX_Result_FAIL_InvalidParameter;
    }
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_Color, pDlispEvalParams->Feature.pInColor);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_Output, pDlispEvalParams->Feature.pInOutput);

    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Sharpness, pDlispEvalParams->Feature.InSharpness);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Denoise, pDlispEvalParams->InDenoise);

    if (pDlispEvalParams->InRectW)
    {
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_X, pDlispEvalParams->InRectX);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_Y, pDlispEvalParams->InRectY);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_W, pDlispEvalParams->InRectW);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_H, pDlispEvalParams->InRectH);
    }

    return NVSDK_NGX_D3D11_EvaluateFeature_C(pInCtx, pInHandle, pInParams, NULL);
}

static inline NVSDK_NGX_Result NGX_CUDA_EVALUATE_DLISP_EXT(
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_CUDA_DLISP_Eval_Params *pDlispEvalParams)
{
    if (pDlispEvalParams->Feature.InSharpness < 0.0f || pDlispEvalParams->Feature.InSharpness > 1.0f || pDlispEvalParams->InDenoise < 0.0f || pDlispEvalParams->InDenoise > 1.0f)
    {
        return NVSDK_NGX_Result_FAIL_InvalidParameter;
    }
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_Color, pDlispEvalParams->Feature.pInColor);
    NVSDK_NGX_Parameter_SetVoidPointer(pInParams, NVSDK_NGX_Parameter_Output, pDlispEvalParams->Feature.pInOutput);

    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Sharpness, pDlispEvalParams->Feature.InSharpness);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Denoise, pDlispEvalParams->InDenoise);

    if (pDlispEvalParams->InRectW)
    {
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_X, pDlispEvalParams->InRectX);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_Y, pDlispEvalParams->InRectY);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_W, pDlispEvalParams->InRectW);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_H, pDlispEvalParams->InRectH);
    }

    return NVSDK_NGX_CUDA_EvaluateFeature_C(pInHandle, pInParams, NULL);
}

static inline NVSDK_NGX_Result NGX_D3D11_CREATE_DLRESOLVE_EXT(
    ID3D11DeviceContext *pInCtx,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_Feature_Create_Params *pDlresolveCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, pDlresolveCreateParams->InWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, pDlresolveCreateParams->InHeight);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutWidth, pDlresolveCreateParams->InTargetWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutHeight, pDlresolveCreateParams->InTargetHeight);

    return NVSDK_NGX_D3D11_CreateFeature(pInCtx, NVSDK_NGX_Feature_DeepResolve, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_D3D11_EVALUATE_DLRESOLVE_EXT(
    ID3D11DeviceContext *pInCtx,
    NVSDK_NGX_Handle *InHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_D3D11_Feature_Eval_Params *pDlresolveEvalParams)
{
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_Color, pDlresolveEvalParams->pInColor);
    NVSDK_NGX_Parameter_SetD3d11Resource(pInParams, NVSDK_NGX_Parameter_Output, pDlresolveEvalParams->pInOutput);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Sharpness, pDlresolveEvalParams->InSharpness);

    return NVSDK_NGX_D3D11_EvaluateFeature_C(pInCtx, InHandle, pInParams, NULL);
}

typedef struct NVSDK_NGX_D3D12_Feature_Eval_Params
{
    ID3D12Resource*  pInColor;
    ID3D12Resource*  pInOutput;

    float       InSharpness;
} NVSDK_NGX_D3D12_Feature_Eval_Params;

typedef struct NVSDK_NGX_D3D12_GBuffer
{
    ID3D12Resource* pInAttrib[NVSDK_NGX_GBUFFERTYPE_NUM];
} NVSDK_NGX_D3D12_GBuffer;

typedef struct NVSDK_NGX_D3D12_DLSS_Eval_Params
{
    NVSDK_NGX_D3D12_Feature_Eval_Params Feature;
    ID3D12Resource*                     pInDepth;
    ID3D12Resource*                     pInMotionVectors;
    float                               InJitterOffsetX;     
    float                               InJitterOffsetY;
    NVSDK_NGX_Dimensions                InRenderSubrectDimensions;

    int                                 InReset;             
    float                               InMVScaleX;          
    float                               InMVScaleY;
    ID3D12Resource*                     pInTransparencyMask; 
    ID3D12Resource*                     pInExposureTexture;
    ID3D12Resource*                     pInBiasCurrentColorMask;
    NVSDK_NGX_Coordinates               InColorSubrectBase;
    NVSDK_NGX_Coordinates               InDepthSubrectBase;
    NVSDK_NGX_Coordinates               InMVSubrectBase;
    NVSDK_NGX_Coordinates               InTranslucencySubrectBase;
    NVSDK_NGX_Coordinates               InBiasCurrentColorSubrectBase;
    NVSDK_NGX_Coordinates               InOutputSubrectBase;
    float                               InPreExposure;
    float                               InExposureScale;
    int                                 InIndicatorInvertXAxis;
    int                                 InIndicatorInvertYAxis;

    NVSDK_NGX_D3D12_GBuffer             GBufferSurface;
    NVSDK_NGX_ToneMapperType            InToneMapperType;
    ID3D12Resource*                     pInMotionVectors3D;
    ID3D12Resource*                     pInIsParticleMask; 
    ID3D12Resource*                     pInAnimatedTextureMask; 
    ID3D12Resource*                     pInDepthHighRes;
    ID3D12Resource*                     pInPositionViewSpace;
    float                               InFrameTimeDeltaInMsec; 
    ID3D12Resource*                     pInRayTracingHitDistance; 
    ID3D12Resource*                     pInMotionVectorsReflections; 
} NVSDK_NGX_D3D12_DLSS_Eval_Params;

typedef struct NVSDK_NGX_D3D12_DLISP_Eval_Params
{
    NVSDK_NGX_D3D12_Feature_Eval_Params Feature;

    unsigned int                        InRectX;
    unsigned int                        InRectY;
    unsigned int                        InRectW;
    unsigned int                        InRectH;
    float                               InDenoise;
} NVSDK_NGX_D3D12_DLISP_Eval_Params;

static inline NVSDK_NGX_Result NGX_D3D12_CREATE_DLSS_EXT(
    ID3D12GraphicsCommandList *pInCmdList,
    unsigned int InCreationNodeMask,
    unsigned int InVisibilityNodeMask,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_DLSS_Create_Params *pInDlssCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_CreationNodeMask, InCreationNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_VisibilityNodeMask, InVisibilityNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, pInDlssCreateParams->Feature.InWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, pInDlssCreateParams->Feature.InHeight);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutWidth, pInDlssCreateParams->Feature.InTargetWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutHeight, pInDlssCreateParams->Feature.InTargetHeight);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_PerfQualityValue, pInDlssCreateParams->Feature.InPerfQualityValue);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, pInDlssCreateParams->InFeatureCreateFlags);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Enable_Output_Subrects, pInDlssCreateParams->InEnableOutputSubrects ? 1 : 0);

    return NVSDK_NGX_D3D12_CreateFeature(pInCmdList, NVSDK_NGX_Feature_SuperSampling, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_D3D12_EVALUATE_DLSS_EXT(
    ID3D12GraphicsCommandList *pInCmdList,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_D3D12_DLSS_Eval_Params *pInDlssEvalParams)
{
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_Color, pInDlssEvalParams->Feature.pInColor);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_Output, pInDlssEvalParams->Feature.pInOutput);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_Depth, pInDlssEvalParams->pInDepth);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_MotionVectors, pInDlssEvalParams->pInMotionVectors);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Jitter_Offset_X, pInDlssEvalParams->InJitterOffsetX);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Jitter_Offset_Y, pInDlssEvalParams->InJitterOffsetY);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Sharpness, pInDlssEvalParams->Feature.InSharpness);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_Reset, pInDlssEvalParams->InReset);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_MV_Scale_X, pInDlssEvalParams->InMVScaleX == 0.0f ? 1.0f : pInDlssEvalParams->InMVScaleX);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_MV_Scale_Y, pInDlssEvalParams->InMVScaleY == 0.0f ? 1.0f : pInDlssEvalParams->InMVScaleY);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_TransparencyMask, pInDlssEvalParams->pInTransparencyMask);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_ExposureTexture, pInDlssEvalParams->pInExposureTexture);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, pInDlssEvalParams->pInBiasCurrentColorMask);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Albedo, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_ALBEDO]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Roughness, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_ROUGHNESS]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Metallic, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_METALLIC]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Specular, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SPECULAR]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Subsurface, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SUBSURFACE]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Normals, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_NORMALS]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_ShadingModelId, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_SHADINGMODELID]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_MaterialId, pInDlssEvalParams->GBufferSurface.pInAttrib[NVSDK_NGX_GBUFFER_MATERIALID]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_8, pInDlssEvalParams->GBufferSurface.pInAttrib[8]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_9, pInDlssEvalParams->GBufferSurface.pInAttrib[9]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_10, pInDlssEvalParams->GBufferSurface.pInAttrib[10]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_11, pInDlssEvalParams->GBufferSurface.pInAttrib[11]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_12, pInDlssEvalParams->GBufferSurface.pInAttrib[12]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_13, pInDlssEvalParams->GBufferSurface.pInAttrib[13]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_14, pInDlssEvalParams->GBufferSurface.pInAttrib[14]);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_GBuffer_Atrrib_15, pInDlssEvalParams->GBufferSurface.pInAttrib[15]);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_TonemapperType, pInDlssEvalParams->InToneMapperType);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_MotionVectors3D, pInDlssEvalParams->pInMotionVectors3D);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_IsParticleMask, pInDlssEvalParams->pInIsParticleMask);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_AnimatedTextureMask, pInDlssEvalParams->pInAnimatedTextureMask);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_DepthHighRes, pInDlssEvalParams->pInDepthHighRes);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_Position_ViewSpace, pInDlssEvalParams->pInPositionViewSpace);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, pInDlssEvalParams->InFrameTimeDeltaInMsec);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_RayTracingHitDistance, pInDlssEvalParams->pInRayTracingHitDistance);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_MotionVectorsReflection, pInDlssEvalParams->pInMotionVectorsReflections);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X, pInDlssEvalParams->InColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y, pInDlssEvalParams->InColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_X, pInDlssEvalParams->InDepthSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_Y, pInDlssEvalParams->InDepthSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_X, pInDlssEvalParams->InMVSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_Y, pInDlssEvalParams->InMVSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_X, pInDlssEvalParams->InTranslucencySubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_Y, pInDlssEvalParams->InTranslucencySubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_X, pInDlssEvalParams->InBiasCurrentColorSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_Y, pInDlssEvalParams->InBiasCurrentColorSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_X, pInDlssEvalParams->InOutputSubrectBase.X);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_Y, pInDlssEvalParams->InOutputSubrectBase.Y);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width , pInDlssEvalParams->InRenderSubrectDimensions.Width);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, pInDlssEvalParams->InRenderSubrectDimensions.Height);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DLSS_Pre_Exposure, pInDlssEvalParams->InPreExposure == 0.0f ? 1.0f : pInDlssEvalParams->InPreExposure);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_DLSS_Exposure_Scale, pInDlssEvalParams->InExposureScale == 0.0f ? 1.0f : pInDlssEvalParams->InExposureScale);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Indicator_Invert_X_Axis, pInDlssEvalParams->InIndicatorInvertXAxis);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_DLSS_Indicator_Invert_Y_Axis, pInDlssEvalParams->InIndicatorInvertYAxis);

    return NVSDK_NGX_D3D12_EvaluateFeature_C(pInCmdList, pInHandle, pInParams, NULL);
}

static inline NVSDK_NGX_Result NGX_D3D12_CREATE_DLISP_EXT(
    ID3D12GraphicsCommandList *InCmdList,
    unsigned int InCreationNodeMask,
    unsigned int InVisibilityNodeMask,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_Feature_Create_Params *pDlispCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_CreationNodeMask, InCreationNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_VisibilityNodeMask, InVisibilityNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, pDlispCreateParams->InWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, pDlispCreateParams->InHeight);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutWidth, pDlispCreateParams->InTargetWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutHeight, pDlispCreateParams->InTargetHeight);
    NVSDK_NGX_Parameter_SetI(pInParams, NVSDK_NGX_Parameter_PerfQualityValue, pDlispCreateParams->InPerfQualityValue);

    return NVSDK_NGX_D3D12_CreateFeature(InCmdList, NVSDK_NGX_Feature_ImageSignalProcessing, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_D3D12_EVALUATE_DLISP_EXT(
    ID3D12GraphicsCommandList *pInCmdList,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_D3D12_DLISP_Eval_Params *pDlispEvalParams)
{
    if (pDlispEvalParams->Feature.InSharpness < 0.0f || pDlispEvalParams->Feature.InSharpness > 1.0f || pDlispEvalParams->InDenoise < 0.0f || pDlispEvalParams->InDenoise > 1.0f)
    {
        return NVSDK_NGX_Result_FAIL_InvalidParameter;
    }
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_Color, pDlispEvalParams->Feature.pInColor);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_Output, pDlispEvalParams->Feature.pInOutput);

    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Sharpness, pDlispEvalParams->Feature.InSharpness);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Denoise, pDlispEvalParams->InDenoise);

    if (pDlispEvalParams->InRectW)
    {
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_X, pDlispEvalParams->InRectX);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_Y, pDlispEvalParams->InRectY);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_W, pDlispEvalParams->InRectW);
        NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Rect_H, pDlispEvalParams->InRectH);
    }
    return NVSDK_NGX_D3D12_EvaluateFeature_C(pInCmdList, pInHandle, pInParams, NULL);
}

static inline NVSDK_NGX_Result NGX_D3D12_CREATE_DLRESOLVE_EXT(
    ID3D12GraphicsCommandList *pInCmdList,
    unsigned int InCreationNodeMask,
    unsigned int InVisibilityNodeMask,
    NVSDK_NGX_Handle **ppOutHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_Feature_Create_Params *pDlresolveCreateParams)
{
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_CreationNodeMask, InCreationNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_VisibilityNodeMask, InVisibilityNodeMask);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Width, pDlresolveCreateParams->InWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_Height, pDlresolveCreateParams->InHeight);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutWidth, pDlresolveCreateParams->InTargetWidth);
    NVSDK_NGX_Parameter_SetUI(pInParams, NVSDK_NGX_Parameter_OutHeight, pDlresolveCreateParams->InTargetHeight);

    return NVSDK_NGX_D3D12_CreateFeature(pInCmdList, NVSDK_NGX_Feature_DeepResolve, pInParams, ppOutHandle);
}

static inline NVSDK_NGX_Result NGX_D3D12_EVALUATE_DLRESOLVE_EXT(
    ID3D12GraphicsCommandList *pInCmdList,
    NVSDK_NGX_Handle *pInHandle,
    NVSDK_NGX_Parameter *pInParams,
    NVSDK_NGX_D3D12_Feature_Eval_Params *pDlresolveEvalParams)
{

    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_Color, pDlresolveEvalParams->pInColor);
    NVSDK_NGX_Parameter_SetD3d12Resource(pInParams, NVSDK_NGX_Parameter_Output, pDlresolveEvalParams->pInOutput);
    NVSDK_NGX_Parameter_SetF(pInParams, NVSDK_NGX_Parameter_Sharpness, pDlresolveEvalParams->InSharpness);

    return NVSDK_NGX_D3D12_EvaluateFeature_C(pInCmdList, pInHandle, pInParams, NULL);
}

#endif
