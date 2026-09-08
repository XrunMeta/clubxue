

#pragma once

#include <stdint.h>
#include <assert.h>
#include <string>
#include "sl_struct.h"

#define SL_ENUM_OPERATORS_64(T)                                                         \
inline bool operator&(T a, T b)                                                         \
{                                                                                       \
    return ((uint64_t)a & (uint64_t)b) != 0;                                            \
}                                                                                       \
                                                                                        \
inline T& operator&=(T& a, T b)                                                         \
{                                                                                       \
    a = (T)((uint64_t)a & (uint64_t)b);                                                 \
    return a;                                                                           \
}                                                                                       \
                                                                                        \
inline T operator|(T a, T b)                                                            \
{                                                                                       \
    return (T)((uint64_t)a | (uint64_t)b);                                              \
}                                                                                       \
                                                                                        \
inline T& operator |= (T& lhs, T rhs)                                                   \
{                                                                                       \
    lhs = (T)((uint64_t)lhs | (uint64_t)rhs);                                           \
    return lhs;                                                                         \
}                                                                                       \
                                                                                        \
inline T operator~(T a)                                                                 \
{                                                                                       \
    return (T)~((uint64_t)a);                                                           \
}

#define SL_ENUM_OPERATORS_32(T)                                                         \
inline bool operator&(T a, T b)                                                         \
{                                                                                       \
    return ((uint32_t)a & (uint32_t)b) != 0;                                            \
}                                                                                       \
                                                                                        \
inline T& operator&=(T& a, T b)                                                         \
{                                                                                       \
    a = (T)((uint32_t)a & (uint32_t)b);                                                 \
    return a;                                                                           \
}                                                                                       \
                                                                                        \
inline T operator|(T a, T b)                                                            \
{                                                                                       \
    return (T)((uint32_t)a | (uint32_t)b);                                              \
}                                                                                       \
                                                                                        \
inline T& operator |= (T& lhs, T rhs)                                                   \
{                                                                                       \
    lhs = (T)((uint32_t)lhs | (uint32_t)rhs);                                           \
    return lhs;                                                                         \
}                                                                                       \
                                                                                        \
inline T operator~(T a)                                                                 \
{                                                                                       \
    return (T)~((uint32_t)a);                                                           \
}

namespace sl
{

constexpr float INVALID_FLOAT = 3.40282346638528859811704183484516925440e38f;
constexpr uint32_t INVALID_UINT = 0xffffffff;

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 6;

struct uint3
{
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct float2
{
    float2() : x(INVALID_FLOAT), y(INVALID_FLOAT) {}
    float2(float _x, float _y) : x(_x), y(_y) {}
    float x, y;
};

struct float3
{
    float3() : x(INVALID_FLOAT), y(INVALID_FLOAT), z(INVALID_FLOAT) {}
    float3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    float x, y, z;
};

struct float4
{
    float4() : x(INVALID_FLOAT), y(INVALID_FLOAT), z(INVALID_FLOAT), w(INVALID_FLOAT) {}
    float4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    float x, y, z, w;
};

struct float4x4
{

    inline float4& operator[](uint32_t i) { return row[i]; }
    inline const float4& operator[](uint32_t i) const { return row[i]; }
    inline void setRow(uint32_t i, const float4& v) { row[i] = v; }
    inline const float4& getRow(uint32_t i) { return row[i]; }

    float4 row[4];
};

struct Extent
{
    uint32_t top{};
    uint32_t left{};
    uint32_t width{};
    uint32_t height{};

    inline operator bool() const { return width != 0 && height != 0; }
    inline bool operator==(const Extent& rhs) const 
    { 
        return top == rhs.top && left == rhs.left &&
        width == rhs.width && height == rhs.height;
    }
    inline bool operator!=(const Extent& rhs) const
    {
        return !operator==(rhs);
    }
    inline bool isSameRes(const Extent& rhs) const
    {
        return width == rhs.width && height == rhs.height;
    }

#if defined(_WINDEF_)

    inline operator RECT() const { return RECT { (LONG)left, (LONG)top, (LONG)(left + width), (LONG)(top + height) }; }
#endif
};

enum Boolean : char
{
    eFalse,
    eTrue,
    eInvalid
};

SL_STRUCT_BEGIN(Constants, StructType({ 0xdcd35ad7, 0x4e4a, 0x4bad, { 0xa9, 0xc, 0xe0, 0xc4, 0x9e, 0xb2, 0x3a, 0xfe } }), kStructVersion2)

    float4x4 cameraViewToClip;

    float4x4 clipToCameraView;

    float4x4 clipToLensClip;

    float4x4 clipToPrevClip;

    float4x4 prevClipToClip;

    float2 jitterOffset;

    float2 mvecScale;

    float2 cameraPinholeOffset;

    float3 cameraPos;

    float3 cameraUp;

    float3 cameraRight;

    float3 cameraFwd;

    float cameraNear = INVALID_FLOAT;

    float cameraFar = INVALID_FLOAT;

    float cameraFOV = INVALID_FLOAT;

    float cameraAspectRatio = INVALID_FLOAT;

    float motionVectorsInvalidValue = INVALID_FLOAT;

    Boolean depthInverted = Boolean::eInvalid;

    Boolean cameraMotionIncluded = Boolean::eInvalid;

    Boolean motionVectors3D = Boolean::eInvalid;

    Boolean reset = Boolean::eInvalid;

    Boolean orthographicProjection = Boolean::eFalse;

    Boolean motionVectorsDilated = Boolean::eFalse;

    Boolean motionVectorsJittered = Boolean::eFalse;

    float minRelativeLinearDepthObjectSeparation = 40.0f;

SL_STRUCT_END()

}
