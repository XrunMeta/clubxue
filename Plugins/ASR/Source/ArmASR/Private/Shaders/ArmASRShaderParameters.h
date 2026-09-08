

#pragma once

#include "GlobalShader.h"

class FArmASR_ApplyBalancedOpt : SHADER_PERMUTATION_BOOL("FFXM_FSR2_OPTION_SHADER_OPT_BALANCED");
class FArmASR_ApplyPerfOpt : SHADER_PERMUTATION_BOOL("FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE");
class FArmASR_ApplyUltraPerfOpt : SHADER_PERMUTATION_BOOL("FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE");

class FArmASRGlobalShader : public FGlobalShader
{
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
};

BEGIN_UNIFORM_BUFFER_STRUCT(FArmASRPassParameters, )
	SHADER_PARAMETER(FIntPoint, iRenderSize)
	SHADER_PARAMETER(FIntPoint, iMaxRenderSize)
	SHADER_PARAMETER(FIntPoint, iDisplaySize)
	SHADER_PARAMETER(FIntPoint, iInputColorResourceDimensions)
	SHADER_PARAMETER(FIntPoint, iLumaMipDimensions)
	SHADER_PARAMETER(int, iLumaMipLevelToUse)
	SHADER_PARAMETER(int, iFrameIndex)
	SHADER_PARAMETER(FVector4f, fDeviceToViewDepth)
	SHADER_PARAMETER(FVector2f, fJitter)
	SHADER_PARAMETER(FVector2f, fMotionVectorScale)
	SHADER_PARAMETER(FVector2f, fDownscaleFactor)
	SHADER_PARAMETER(FVector2f, fMotionVectorJitterCancellation)
	SHADER_PARAMETER(float, fPreExposure)
	SHADER_PARAMETER(float, fPreviousFramePreExposure)
	SHADER_PARAMETER(float, fTanHalfFOV)
	SHADER_PARAMETER(float, fJitterSequenceLength)
	SHADER_PARAMETER(float, fDeltaTime)
	SHADER_PARAMETER(float, fDynamicResChangeFactor)
	SHADER_PARAMETER(float, fViewSpaceToMetersFactor)
END_UNIFORM_BUFFER_STRUCT()

BEGIN_UNIFORM_BUFFER_STRUCT(FArmASRComputeLuminanceParameters, )
	SHADER_PARAMETER(uint32, mips)
	SHADER_PARAMETER(uint32, numWorkGroups)
	SHADER_PARAMETER(FUintVector2, workGroupOffset)
	SHADER_PARAMETER(FUintVector2, renderSize)
END_UNIFORM_BUFFER_STRUCT()

BEGIN_UNIFORM_BUFFER_STRUCT(FArmASRRCASParameters, )
	SHADER_PARAMETER(FUintVector4, rcasConfig)
END_UNIFORM_BUFFER_STRUCT()
