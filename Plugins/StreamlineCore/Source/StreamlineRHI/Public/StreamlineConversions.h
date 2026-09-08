

#pragma once

#include "StreamlineRHI.h"
#include "sl.h"
#include "sl_helpers.h"

inline sl::float4x4 ToSL(const FRHIStreamlineArguments::FMatrix44f& InMatrix, bool bIsOrthographicProjection = false)
{

	sl::float4x4 Result;
	for (int i = 0; i < 4; i++)
	{
		Result.setRow(i, { InMatrix.M[i][0], InMatrix.M[i][1], InMatrix.M[i][2], InMatrix.M[i][3] });
	}
	return Result;
};

inline sl::float4 ToSL(const FRHIStreamlineArguments::FVector4f& InVector)
{
	return { InVector.X, InVector.Y, InVector.Z, InVector.W };
};

inline sl::float3 ToSL(const FRHIStreamlineArguments::FVector3f& InVector)
{
	return { InVector.X, InVector.Y, InVector.Z };
};

inline sl::float2 ToSL(const FRHIStreamlineArguments::FVector2f& InVector)
{
	return { InVector.X, InVector.Y };
};

inline sl::Boolean ToSL(bool b)
{
	return b ? sl::eTrue : sl::eFalse;
};

inline sl::Extent ToSL(const FIntRect& InRect)
{
	check(InRect.Min.X >= 0);
	check(InRect.Min.Y >= 0);

	check(InRect.Width() >= 0);
	check(InRect.Height() >= 0);

	return { uint32_t(InRect.Min.Y ), uint32_t(InRect.Min.X ), 
		uint32_t(InRect.Width()), uint32_t(InRect.Height())};
};

inline sl::BufferType ToSL(EStreamlineResource InResourceTag)
{
	switch (InResourceTag)
	{
	default:
		checkf(false, TEXT("unexpected EStreamlineResource enum value %u. This is a UE Streamline plugin developer bug"), InResourceTag);
	case EStreamlineResource::Depth: return sl::kBufferTypeDepth;
	case EStreamlineResource::MotionVectors: return sl::kBufferTypeMotionVectors;
	case EStreamlineResource::HUDLessColor: return sl::kBufferTypeHUDLessColor;
	case EStreamlineResource::UIColorAndAlpha: return sl::kBufferTypeUIColorAndAlpha;
	case EStreamlineResource::Backbuffer: return sl::kBufferTypeBackbuffer;
	case EStreamlineResource::ScalingOutputColor: return sl::kBufferTypeScalingOutputColor;
	}
}

