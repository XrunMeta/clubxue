

#pragma once

#include "SceneRendering.h"

#include <array>
#include <cmath>

#define FFXM_FSR2_RESOURCE_IDENTIFIER_SCENE_LUMINANCE                                29 
#define FFXM_FSR2_RESOURCE_IDENTIFIER_SCENE_LUMINANCE_MIPMAP_4                       33

#define FFXM_FSR2_RESOURCE_IDENTIFIER_SCENE_LUMINANCE_MIPMAP_SHADING_CHANGE          FFXM_FSR2_RESOURCE_IDENTIFIER_SCENE_LUMINANCE_MIPMAP_4
#define FFXM_FSR2_SHADING_CHANGE_MIP_LEVEL                                           (FFXM_FSR2_RESOURCE_IDENTIFIER_SCENE_LUMINANCE_MIPMAP_SHADING_CHANGE - FFXM_FSR2_RESOURCE_IDENTIFIER_SCENE_LUMINANCE)
#define FFXM_FSR2_SHADING_CHANGE_MIPMAP_5 (5)

const float ARM_ASR_EPSILON = 1e-06f;
const float ARM_ASR_PI = 3.141592653589793f;

static const int32_t ARM_ASR_MAX_BIAS_TEXTURE_WIDTH = 16;
static const int32_t ARM_ASR_MAX_BIAS_TEXTURE_HEIGHT = 16;
static const int32_t ARM_ASR_MAX_BIAS_TEXTURE_SIZE = ARM_ASR_MAX_BIAS_TEXTURE_WIDTH * ARM_ASR_MAX_BIAS_TEXTURE_HEIGHT;

static const float ARM_ASR_MAX_BIAS_VALUES[] = {
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.876f, 1.809f, 1.772f, 1.753f, 1.748f,
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.869f, 1.801f, 1.764f, 1.745f, 1.739f,
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.976f, 1.841f, 1.774f, 1.737f, 1.716f, 1.71f,
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.914f, 1.784f, 1.716f, 1.673f, 1.649f, 1.641f,
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.793f, 1.676f, 1.604f, 1.562f, 1.54f,  1.533f,
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.802f, 1.619f, 1.536f, 1.492f, 1.467f, 1.454f, 1.449f,
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.812f, 1.575f, 1.496f, 1.456f, 1.432f, 1.416f, 1.408f, 1.405f,
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.555f, 1.479f, 1.438f, 1.413f, 1.398f, 1.387f, 1.381f, 1.379f,
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.812f, 1.555f, 1.474f, 1.43f,  1.404f, 1.387f, 1.376f, 1.368f, 1.363f, 1.362f,
	2.0f,   2.0f,   2.0f,   2.0f,   2.0f,   1.802f, 1.575f, 1.479f, 1.43f,  1.401f, 1.382f, 1.369f, 1.36f,  1.354f, 1.351f, 1.35f,
	2.0f,   2.0f,   1.976f, 1.914f, 1.793f, 1.619f, 1.496f, 1.438f, 1.404f, 1.382f, 1.367f, 1.357f, 1.349f, 1.344f, 1.341f, 1.34f,
	1.876f, 1.869f, 1.841f, 1.784f, 1.676f, 1.536f, 1.456f, 1.413f, 1.387f, 1.369f, 1.357f, 1.347f, 1.341f, 1.336f, 1.333f, 1.332f,
	1.809f, 1.801f, 1.774f, 1.716f, 1.604f, 1.492f, 1.432f, 1.398f, 1.376f, 1.36f,  1.349f, 1.341f, 1.335f, 1.33f,  1.328f, 1.327f,
	1.772f, 1.764f, 1.737f, 1.673f, 1.562f, 1.467f, 1.416f, 1.387f, 1.368f, 1.354f, 1.344f, 1.336f, 1.33f,  1.326f, 1.323f, 1.323f,
	1.753f, 1.745f, 1.716f, 1.649f, 1.54f,  1.454f, 1.408f, 1.381f, 1.363f, 1.351f, 1.341f, 1.333f, 1.328f, 1.323f, 1.321f, 1.32f,
	1.748f, 1.739f, 1.71f,  1.641f, 1.533f, 1.449f, 1.405f, 1.379f, 1.362f, 1.35f,  1.34f,  1.332f, 1.327f, 1.323f, 1.32f,  1.319f,

};

struct TextureBulkData final : public FResourceBulkDataInterface
{
	TextureBulkData()
		: Data(nullptr)
		, DataSize(0)
	{
	}

	TextureBulkData(const void* InData, uint32 InDataSize)
		: Data(InData)
		, DataSize(InDataSize)
	{
	}

	const void* GetResourceBulkData() const { return Data; }
	uint32 GetResourceBulkDataSize() const { return DataSize; }

	void Discard() {}

	const void* Data = nullptr;
	uint32 DataSize = 0;
};

inline FVector4f SetupDeviceDepthToViewSpaceDepthParams(const FViewInfo& View)
{
	checkf(static_cast<int32>(ERHIZBuffer::IsInverted) != 0, TEXT("ZBuffer should be inverted."));
	return FVector4f(
		-FLT_EPSILON,
		View.NearClippingDistance,
		View.ViewMatrices.GetInvProjectionMatrix().M[0][0],
		View.ViewMatrices.GetInvProjectionMatrix().M[1][1]
	);
}

inline int32_t GetJitterPhaseCount(int32_t RenderWidth, int32_t DisplayWidth)
{
	const float BasePhaseCount = 8.0f;
	const int32_t JitterPhaseCount = static_cast<int32_t>(
		BasePhaseCount * std::pow((static_cast<float>(DisplayWidth) / RenderWidth), 2.0f));
	return JitterPhaseCount;
}

inline void SetCommonParameters(
	FArmASRPassParameters* ArmASRPassParameters,
	int32_t FrameIndex,
	float PrevPreExposure,
	const FIntPoint& InputExtents,
	const FIntPoint& OutputExtents,
	const FViewInfo& ViewInfo,
	const FIntPoint& ResourceDimensions)
{
	ArmASRPassParameters->iRenderSize = FIntPoint(InputExtents.X, InputExtents.Y);
	ArmASRPassParameters->iMaxRenderSize = FIntPoint(InputExtents.X, InputExtents.Y);
	ArmASRPassParameters->iDisplaySize = FIntPoint(OutputExtents.X, OutputExtents.Y);
	ArmASRPassParameters->iInputColorResourceDimensions = ResourceDimensions;

	ArmASRPassParameters->iLumaMipLevelToUse = FFXM_FSR2_SHADING_CHANGE_MIP_LEVEL;
	const float MipDiv = static_cast<float>(2 << ArmASRPassParameters->iLumaMipLevelToUse);
	ArmASRPassParameters->iLumaMipDimensions = FIntPoint(
		ArmASRPassParameters->iMaxRenderSize[0] / MipDiv,
		ArmASRPassParameters->iMaxRenderSize[1] / MipDiv
	);

	ArmASRPassParameters->iFrameIndex = FrameIndex;

	ArmASRPassParameters->fDeviceToViewDepth = SetupDeviceDepthToViewSpaceDepthParams(ViewInfo);

	ArmASRPassParameters->fJitter = FVector2f(ViewInfo.TemporalJitterPixels.X, ViewInfo.TemporalJitterPixels.Y);

	FVector2f MotionVectorScale;
	MotionVectorScale[0] = 1.0;
	MotionVectorScale[1] = 1.0;
	ArmASRPassParameters->fMotionVectorScale = MotionVectorScale;

	FVector2f DownscaleFactor;
	DownscaleFactor[0] = static_cast<float>(InputExtents.X) / static_cast<float>(OutputExtents.X);
	DownscaleFactor[1] = static_cast<float>(InputExtents.Y) / static_cast<float>(OutputExtents.Y);
	ArmASRPassParameters->fDownscaleFactor = DownscaleFactor;

	ArmASRPassParameters->fMotionVectorJitterCancellation = FVector2f(0.0, 0.0);

	ArmASRPassParameters->fPreExposure = ViewInfo.PreExposure;

	ArmASRPassParameters->fPreviousFramePreExposure = PrevPreExposure;

	const float AspectRatio = static_cast<float>(InputExtents.X) / static_cast<float>(InputExtents.Y);
	const float CameraFovAngleVertical = ViewInfo.ViewMatrices.ComputeHalfFieldOfViewPerAxis().Y * 2.0f;
	const float CameraAngleHorizontal = std::atan(std::tan(CameraFovAngleVertical / 2) * AspectRatio) * 2;
	ArmASRPassParameters->fTanHalfFOV = std::tanf(CameraAngleHorizontal * 0.5f);

	const int32_t JitterPhaseCount = GetJitterPhaseCount(InputExtents.X, OutputExtents.X);
	ArmASRPassParameters->fJitterSequenceLength = JitterPhaseCount;

	const float FrameTimeDelta = ViewInfo.Family->Time.GetDeltaWorldTimeSeconds() * 1000.0f;
	const float DeltaTime = std::max(0.0f, std::min(1.0f, FrameTimeDelta / 1000.0f));
	ArmASRPassParameters->fDeltaTime = DeltaTime;

	ArmASRPassParameters->fViewSpaceToMetersFactor = 1.0f;

	ArmASRPassParameters->fDynamicResChangeFactor = 0.0;
}

struct SpdConfig
{
	void Setup(const std::array<uint32_t, 4>& RectInfo, const int32_t Mips = -1)
	{

		WorkGroupOffset[0] = RectInfo[0] / 64;
		WorkGroupOffset[1] = RectInfo[1] / 64;

		uint32_t EndIndexX = (RectInfo[0] + RectInfo[2] - 1) / 64;  
		uint32_t EndIndexY = (RectInfo[1] + RectInfo[3] - 1) / 64;  

		DispatchThreadGroupCountXY[0] = EndIndexX + 1 - WorkGroupOffset[0];
		DispatchThreadGroupCountXY[1] = EndIndexY + 1 - WorkGroupOffset[1];

		NumWorkGroupsAndMips[0] = (DispatchThreadGroupCountXY[0]) * (DispatchThreadGroupCountXY[1]);

		if (Mips >= 0)
		{
			NumWorkGroupsAndMips[1] = static_cast<uint32_t>(Mips);
		}
		else
		{

			uint32_t Resolution = std::max(RectInfo[2], RectInfo[3]);
			NumWorkGroupsAndMips[1] = static_cast<uint32_t>(
				(std::min(std::floor(std::log2(static_cast<float_t>(Resolution))), static_cast<float_t>(12))));
		}
	}

	std::array<uint32_t, 2> DispatchThreadGroupCountXY = { 0, 0 };
	std::array<uint32_t, 2> WorkGroupOffset = { 0, 0 };
	std::array<uint32_t, 2> NumWorkGroupsAndMips = { 0, 0 };
};

static float Lanczos2(float Value)
{
	return std::abs(Value) < ARM_ASR_EPSILON ? 1.f :
		(std::sinf(ARM_ASR_PI * Value) / (ARM_ASR_PI * Value)) * (std::sinf(0.5f * ARM_ASR_PI * Value) / (0.5f * ARM_ASR_PI * Value));
}
