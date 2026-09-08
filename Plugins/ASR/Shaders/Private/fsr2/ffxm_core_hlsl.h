

#define FFXM_GROUPSHARED groupshared

#define FFXM_GROUP_MEMORY_BARRIER() GroupMemoryBarrierWithGroupSync()

#define FFXM_ATOMIC_ADD(x, y) InterlockedAdd(x, y)

#define FFXM_STATIC static

#define FFXM_UNROLL [unroll]

#define FFXM_GREATER_THAN(x, y) x > y

#define FFXM_GREATER_THAN_EQUAL(x, y) x >= y

#define FFXM_LESS_THAN(x, y) x < y

#define FFXM_LESS_THAN_EQUAL(x, y) x <= y

#define FFXM_EQUAL(x, y) x == y

#define FFXM_NOT_EQUAL(x, y) x != y

#define FFXM_MATRIX_MULTIPLY(a, b) mul(a, b)

#define FFXM_TRANSFORM_VECTOR(a, b) mul(a, b)

#define FFXM_MODULO(a, b) (fmod(a, b))

#define FFXM_BROADCAST_FLOAT32(x) FfxFloat32(x)

#define FFXM_BROADCAST_FLOAT32X2(x) FfxFloat32(x)

#define FFXM_BROADCAST_FLOAT32X3(x) FfxFloat32(x)

#define FFXM_BROADCAST_FLOAT32X4(x) FfxFloat32(x)

#define FFXM_BROADCAST_UINT32(x) FfxUInt32(x)

#define FFXM_BROADCAST_UINT32X2(x) FfxUInt32(x)

#define FFXM_BROADCAST_UINT32X3(x) FfxUInt32(x)

#define FFXM_BROADCAST_UINT32X4(x) FfxUInt32(x)

#define FFXM_BROADCAST_INT32(x) FfxInt32(x)

#define FFXM_BROADCAST_INT32X2(x) FfxInt32(x)

#define FFXM_BROADCAST_INT32X3(x) FfxInt32(x)

#define FFXM_BROADCAST_INT32X4(x) FfxInt32(x)

#define FFXM_BROADCAST_MIN_FLOAT16(a)   FFXM_MIN16_F(a)

#define FFXM_BROADCAST_MIN_FLOAT16X2(a) FFXM_MIN16_F(a)

#define FFXM_BROADCAST_MIN_FLOAT16X3(a) FFXM_MIN16_F(a)

#define FFXM_BROADCAST_MIN_FLOAT16X4(a) FFXM_MIN16_F(a)

#define FFXM_BROADCAST_MIN_UINT16(a)   FFXM_MIN16_U(a)

#define FFXM_BROADCAST_MIN_UINT16X2(a) FFXM_MIN16_U(a)

#define FFXM_BROADCAST_MIN_UINT16X3(a) FFXM_MIN16_U(a)

#define FFXM_BROADCAST_MIN_UINT16X4(a) FFXM_MIN16_U(a)

#define FFXM_BROADCAST_MIN_INT16(a)   FFXM_MIN16_I(a)

#define FFXM_BROADCAST_MIN_INT16X2(a) FFXM_MIN16_I(a)

#define FFXM_BROADCAST_MIN_INT16X3(a) FFXM_MIN16_I(a)

#define FFXM_BROADCAST_MIN_INT16X4(a) FFXM_MIN16_I(a)

FfxUInt32 packHalf2x16(FfxFloat32x2 value)
{
    return f32tof16(value.x) | (f32tof16(value.y) << 16);
}

FfxFloat32x2 ffxBroadcast2(FfxFloat32 value)
{
    return FfxFloat32x2(value, value);
}

FfxFloat32x3 ffxBroadcast3(FfxFloat32 value)
{
    return FfxFloat32x3(value, value, value);
}

FfxFloat32x4 ffxBroadcast4(FfxFloat32 value)
{
    return FfxFloat32x4(value, value, value, value);
}

FfxInt32x2 ffxBroadcast2(FfxInt32 value)
{
    return FfxInt32x2(value, value);
}

FfxUInt32x3 ffxBroadcast3(FfxInt32 value)
{
    return FfxUInt32x3(value, value, value);
}

FfxInt32x4 ffxBroadcast4(FfxInt32 value)
{
    return FfxInt32x4(value, value, value, value);
}

FfxUInt32x2 ffxBroadcast2(FfxUInt32 value)
{
    return FfxUInt32x2(value, value);
}

FfxUInt32x3 ffxBroadcast3(FfxUInt32 value)
{
    return FfxUInt32x3(value, value, value);
}

FfxUInt32x4 ffxBroadcast4(FfxUInt32 value)
{
    return FfxUInt32x4(value, value, value, value);
}

FfxUInt32 bitfieldExtract(FfxUInt32 src, FfxUInt32 off, FfxUInt32 bits)
{
    FfxUInt32 mask = (1u << bits) - 1;
    return (src >> off) & mask;
}

FfxUInt32 bitfieldInsert(FfxUInt32 src, FfxUInt32 ins, FfxUInt32 mask)
{
    return (ins & mask) | (src & (~mask));
}

FfxUInt32 bitfieldInsertMask(FfxUInt32 src, FfxUInt32 ins, FfxUInt32 bits)
{
    FfxUInt32 mask = (1u << bits) - 1;
    return (ins & mask) | (src & (~mask));
}

FfxUInt32 ffxAsUInt32(FfxFloat32 x)
{
    return asuint(x);
}

FfxUInt32x2 ffxAsUInt32(FfxFloat32x2 x)
{
    return asuint(x);
}

FfxUInt32x3 ffxAsUInt32(FfxFloat32x3 x)
{
    return asuint(x);
}

FfxUInt32x4 ffxAsUInt32(FfxFloat32x4 x)
{
    return asuint(x);
}

FfxFloat32 ffxAsFloat(FfxUInt32 x)
{
    return asfloat(x);
}

FfxFloat32x2 ffxAsFloat(FfxUInt32x2 x)
{
    return asfloat(x);
}

FfxFloat32x3 ffxAsFloat(FfxUInt32x3 x)
{
    return asfloat(x);
}

FfxFloat32x4 ffxAsFloat(FfxUInt32x4 x)
{
    return asfloat(x);
}

FfxFloat32 ffxLerp(FfxFloat32 x, FfxFloat32 y, FfxFloat32 t)
{
    return lerp(x, y, t);
}

FfxFloat32x2 ffxLerp(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32 t)
{
    return lerp(x, y, t);
}

FfxFloat32x2 ffxLerp(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 t)
{
    return lerp(x, y, t);
}

FfxFloat32x3 ffxLerp(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32 t)
{
    return lerp(x, y, t);
}

FfxFloat32x3 ffxLerp(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 t)
{
    return lerp(x, y, t);
}

FfxFloat32x4 ffxLerp(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32 t)
{
    return lerp(x, y, t);
}

FfxFloat32x4 ffxLerp(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 t)
{
    return lerp(x, y, t);
}

FfxFloat32 ffxSaturate(FfxFloat32 x)
{
    return saturate(x);
}

FfxFloat32x2 ffxSaturate(FfxFloat32x2 x)
{
    return saturate(x);
}

FfxFloat32x3 ffxSaturate(FfxFloat32x3 x)
{
    return saturate(x);
}

FfxFloat32x4 ffxSaturate(FfxFloat32x4 x)
{
    return saturate(x);
}

FfxFloat32 ffxFract(FfxFloat32 x)
{
    return x - floor(x);
}

FfxFloat32x2 ffxFract(FfxFloat32x2 x)
{
    return x - floor(x);
}

FfxFloat32x3 ffxFract(FfxFloat32x3 x)
{
    return x - floor(x);
}

FfxFloat32x4 ffxFract(FfxFloat32x4 x)
{
    return x - floor(x);
}

FfxFloat32 ffxMax3(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return max(x, max(y, z));
}

FfxFloat32x2 ffxMax3(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return max(x, max(y, z));
}

FfxFloat32x3 ffxMax3(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return max(x, max(y, z));
}

FfxFloat32x4 ffxMax3(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return max(x, max(y, z));
}

FfxUInt32 ffxMax3(FfxUInt32 x, FfxUInt32 y, FfxUInt32 z)
{
    return max(x, max(y, z));
}

FfxUInt32x2 ffxMax3(FfxUInt32x2 x, FfxUInt32x2 y, FfxUInt32x2 z)
{
    return max(x, max(y, z));
}

FfxUInt32x3 ffxMax3(FfxUInt32x3 x, FfxUInt32x3 y, FfxUInt32x3 z)
{
    return max(x, max(y, z));
}

FfxUInt32x4 ffxMax3(FfxUInt32x4 x, FfxUInt32x4 y, FfxUInt32x4 z)
{
    return max(x, max(y, z));
}

FfxFloat32 ffxMed3(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return max(min(x, y), min(max(x, y), z));
}

FfxFloat32x2 ffxMed3(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return max(min(x, y), min(max(x, y), z));
}

FfxFloat32x3 ffxMed3(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return max(min(x, y), min(max(x, y), z));
}

FfxFloat32x4 ffxMed3(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return max(min(x, y), min(max(x, y), z));
}

FfxInt32 ffxMed3(FfxInt32 x, FfxInt32 y, FfxInt32 z)
{
    return max(min(x, y), min(max(x, y), z));

}

FfxInt32x2 ffxMed3(FfxInt32x2 x, FfxInt32x2 y, FfxInt32x2 z)
{
    return max(min(x, y), min(max(x, y), z));

}

FfxInt32x3 ffxMed3(FfxInt32x3 x, FfxInt32x3 y, FfxInt32x3 z)
{
    return max(min(x, y), min(max(x, y), z));
}

FfxInt32x4 ffxMed3(FfxInt32x4 x, FfxInt32x4 y, FfxInt32x4 z)
{
    return max(min(x, y), min(max(x, y), z));
}

FfxFloat32 ffxMin3(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return min(x, min(y, z));
}

FfxFloat32x2 ffxMin3(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return min(x, min(y, z));
}

FfxFloat32x3 ffxMin3(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return min(x, min(y, z));
}

FfxFloat32x4 ffxMin3(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return min(x, min(y, z));
}

FfxUInt32 ffxMin3(FfxUInt32 x, FfxUInt32 y, FfxUInt32 z)
{
    return min(x, min(y, z));
}

FfxUInt32x2 ffxMin3(FfxUInt32x2 x, FfxUInt32x2 y, FfxUInt32x2 z)
{
    return min(x, min(y, z));
}

FfxUInt32x3 ffxMin3(FfxUInt32x3 x, FfxUInt32x3 y, FfxUInt32x3 z)
{
    return min(x, min(y, z));
}

FfxUInt32x4 ffxMin3(FfxUInt32x4 x, FfxUInt32x4 y, FfxUInt32x4 z)
{
    return min(x, min(y, z));
}

FfxUInt32 AShrSU1(FfxUInt32 a, FfxUInt32 b)
{
    return FfxUInt32(FfxInt32(a) >> FfxInt32(b));
}

FfxUInt32 ffxPackF32(FfxFloat32x2 v){
    FfxUInt32x2 p = FfxUInt32x2(f32tof16(FfxFloat32x2(v).x), f32tof16(FfxFloat32x2(v).y));
	return p.x | (p.y << 16);
}

FfxFloat32x2 ffxUnpackF32(FfxUInt32 a){
    return f16tof32(FfxUInt32x2(a & 0xFFFF, a >> 16));
}

FFXM_MIN16_F2 ffxUint32ToFloat16x2(FfxUInt32 x)
{
	FfxFloat32x2 t = f16tof32(FfxUInt32x2(x & 0xFFFF, x >> 16));
	return FFXM_MIN16_F2(t);
}
FFXM_MIN16_F4 ffxUint32x2ToFloat16x4(FfxUInt32x2 x)
{
	return FFXM_MIN16_F4(ffxUint32ToFloat16x2(x.x), ffxUint32ToFloat16x2(x.y));
}
FFXM_MIN16_U2 ffxUint32ToUint16x2(FfxUInt32 x)
{
	FfxUInt32x2 t = FfxUInt32x2(x & 0xFFFF, x >> 16);
	return FFXM_MIN16_U2(t);
}
FFXM_MIN16_U4 ffxUint32x2ToUint16x4(FfxUInt32x2 x)
{
	return FFXM_MIN16_U4(ffxUint32ToUint16x2(x.x), ffxUint32ToUint16x2(x.y));
}

FfxFloat32 ffxInvertSafe(FfxFloat32 v){
    FfxFloat32 s = sign(v);
    FfxFloat32 s2 = s*s;
    return s2/(v + s2 - 1.0);
}

FfxFloat32x2 ffxInvertSafe(FfxFloat32x2 v){
    FfxFloat32x2 s = sign(v);
    FfxFloat32x2 s2 = s*s;
    return s2/(v + s2 - FfxFloat32x2(1.0, 1.0));
}

FfxFloat32x3 ffxInvertSafe(FfxFloat32x3 v){
    FfxFloat32x3 s = sign(v);
    FfxFloat32x3 s2 = s*s;
    return s2/(v + s2 - FfxFloat32x3(1.0, 1.0, 1.0));
}

FfxFloat32x4 ffxInvertSafe(FfxFloat32x4 v){
    FfxFloat32x4 s = sign(v);
    FfxFloat32x4 s2 = s*s;
    return s2/(v + s2 - FfxFloat32x4(1.0, 1.0, 1.0, 1.0));
}

#define FFXM_UINT32_TO_FLOAT16X2(x) ffxUint32ToFloat16x2(FfxUInt32(x))
#if FFXM_HALF

#define FFXM_UINT32X2_TO_FLOAT16X4(x) ffxUint32x2ToFloat16x4(FfxUInt32x2(x))
#define FFXM_UINT32_TO_UINT16X2(x) ffxUint32ToUint16x2(FfxUInt32(x))
#define FFXM_UINT32X2_TO_UINT16X4(x) ffxUint32x2ToUint16x4(FfxUInt32x2(x))

FfxUInt32 ffxPackF16(FfxFloat16x2 v){
    FfxUInt32x2 p = FfxUInt32x2(f32tof16(FfxFloat32x2(v).x), f32tof16(FfxFloat32x2(v).y));
	return p.x | (p.y << 16);
}

FfxFloat16x2 ffxUnpackF16(FfxUInt32 a){
    return FfxFloat16x2(f16tof32(FfxUInt32x2(a & 0xFFFF, a >> 16)));
}

FfxUInt32 FFXM_MIN16_F2ToUint32(FFXM_MIN16_F2 x)
{
	return f32tof16(x.x) + (f32tof16(x.y) << 16);
}
FfxUInt32x2 FFXM_MIN16_F4ToUint32x2(FFXM_MIN16_F4 x)
{
	return FfxUInt32x2(FFXM_MIN16_F2ToUint32(x.xy), FFXM_MIN16_F2ToUint32(x.zw));
}
FfxUInt32 FFXM_MIN16_U2ToUint32(FFXM_MIN16_U2 x)
{
	return FfxUInt32(x.x) + (FfxUInt32(x.y) << 16);
}
FfxUInt32x2 FFXM_MIN16_U4ToUint32x2(FFXM_MIN16_U4 x)
{
	return FfxUInt32x2(FFXM_MIN16_U2ToUint32(x.xy), FFXM_MIN16_U2ToUint32(x.zw));
}
#define FFXM_FLOAT16X2_TO_UINT32(x) FFXM_MIN16_F2ToUint32(FFXM_MIN16_F2(x))
#define FFXM_FLOAT16X4_TO_UINT32X2(x) FFXM_MIN16_F4ToUint32x2(FFXM_MIN16_F4(x))
#define FFXM_UINT16X2_TO_UINT32(x) FFXM_MIN16_U2ToUint32(FFXM_MIN16_U2(x))
#define FFXM_UINT16X4_TO_UINT32X2(x) FFXM_MIN16_U4ToUint32x2(FFXM_MIN16_U4(x))

#if (FFXM_HLSL_6_2) && !defined(FFXM_NO_16_BIT_CAST)
#define FFXM_TO_UINT16(x) asuint16(x)
#define FFXM_TO_UINT16X2(x) asuint16(x)
#define FFXM_TO_UINT16X3(x) asuint16(x)
#define FFXM_TO_UINT16X4(x) asuint16(x)
#else
#define FFXM_TO_UINT16(a) FFXM_MIN16_U(f32tof16(FfxFloat32(a)))
#define FFXM_TO_UINT16X2(a) FFXM_MIN16_U2(FFXM_TO_UINT16((a).x), FFXM_TO_UINT16((a).y))
#define FFXM_TO_UINT16X3(a) FFXM_MIN16_U3(FFXM_TO_UINT16((a).x), FFXM_TO_UINT16((a).y), FFXM_TO_UINT16((a).z))
#define FFXM_TO_UINT16X4(a) FFXM_MIN16_U4(FFXM_TO_UINT16((a).x), FFXM_TO_UINT16((a).y), FFXM_TO_UINT16((a).z), FFXM_TO_UINT16((a).w))
#endif 

#if (FFXM_HLSL_6_2) && !defined(FFXM_NO_16_BIT_CAST)
#define FFXM_TO_FLOAT16(x) asfloat16(x)
#define FFXM_TO_FLOAT16X2(x) asfloat16(x)
#define FFXM_TO_FLOAT16X3(x) asfloat16(x)
#define FFXM_TO_FLOAT16X4(x) asfloat16(x)
#else
#define FFXM_TO_FLOAT16(a) FFXM_MIN16_F(f16tof32(FfxUInt32(a)))
#define FFXM_TO_FLOAT16X2(a) FFXM_MIN16_F2(FFXM_TO_FLOAT16((a).x), FFXM_TO_FLOAT16((a).y))
#define FFXM_TO_FLOAT16X3(a) FFXM_MIN16_F3(FFXM_TO_FLOAT16((a).x), FFXM_TO_FLOAT16((a).y), FFXM_TO_FLOAT16((a).z))
#define FFXM_TO_FLOAT16X4(a) FFXM_MIN16_F4(FFXM_TO_FLOAT16((a).x), FFXM_TO_FLOAT16((a).y), FFXM_TO_FLOAT16((a).z), FFXM_TO_FLOAT16((a).w))
#endif 

#define FFXM_BROADCAST_FLOAT16(a)   FFXM_MIN16_F(a)
#define FFXM_BROADCAST_FLOAT16X2(a) FFXM_MIN16_F(a)
#define FFXM_BROADCAST_FLOAT16X3(a) FFXM_MIN16_F(a)
#define FFXM_BROADCAST_FLOAT16X4(a) FFXM_MIN16_F(a)

#define FFXM_BROADCAST_INT16(a)   FFXM_MIN16_I(a)
#define FFXM_BROADCAST_INT16X2(a) FFXM_MIN16_I(a)
#define FFXM_BROADCAST_INT16X3(a) FFXM_MIN16_I(a)
#define FFXM_BROADCAST_INT16X4(a) FFXM_MIN16_I(a)

#define FFXM_BROADCAST_UINT16(a)   FFXM_MIN16_U(a)
#define FFXM_BROADCAST_UINT16X2(a) FFXM_MIN16_U(a)
#define FFXM_BROADCAST_UINT16X3(a) FFXM_MIN16_U(a)
#define FFXM_BROADCAST_UINT16X4(a) FFXM_MIN16_U(a)

FFXM_MIN16_U ffxAbsHalf(FFXM_MIN16_U a)
{
	return FFXM_MIN16_U(abs(FFXM_MIN16_I(a)));
}
FFXM_MIN16_U2 ffxAbsHalf(FFXM_MIN16_U2 a)
{
	return FFXM_MIN16_U2(abs(FFXM_MIN16_I2(a)));
}
FFXM_MIN16_U3 ffxAbsHalf(FFXM_MIN16_U3 a)
{
	return FFXM_MIN16_U3(abs(FFXM_MIN16_I3(a)));
}
FFXM_MIN16_U4 ffxAbsHalf(FFXM_MIN16_U4 a)
{
	return FFXM_MIN16_U4(abs(FFXM_MIN16_I4(a)));
}

FFXM_MIN16_F ffxClampHalf(FFXM_MIN16_F x, FFXM_MIN16_F n, FFXM_MIN16_F m)
{
	return max(n, min(x, m));
}
FFXM_MIN16_F2 ffxClampHalf(FFXM_MIN16_F2 x, FFXM_MIN16_F2 n, FFXM_MIN16_F2 m)
{
	return max(n, min(x, m));
}
FFXM_MIN16_F3 ffxClampHalf(FFXM_MIN16_F3 x, FFXM_MIN16_F3 n, FFXM_MIN16_F3 m)
{
	return max(n, min(x, m));
}
FFXM_MIN16_F4 ffxClampHalf(FFXM_MIN16_F4 x, FFXM_MIN16_F4 n, FFXM_MIN16_F4 m)
{
	return max(n, min(x, m));
}

FFXM_MIN16_F ffxFract(FFXM_MIN16_F x)
{
	return x - floor(x);
}
FFXM_MIN16_F2 ffxFract(FFXM_MIN16_F2 x)
{
	return x - floor(x);
}
FFXM_MIN16_F3 ffxFract(FFXM_MIN16_F3 x)
{
	return x - floor(x);
}
FFXM_MIN16_F4 ffxFract(FFXM_MIN16_F4 x)
{
	return x - floor(x);
}

FFXM_MIN16_F ffxLerp(FFXM_MIN16_F x, FFXM_MIN16_F y, FFXM_MIN16_F a)
{
	return lerp(x, y, a);
}
FFXM_MIN16_F2 ffxLerp(FFXM_MIN16_F2 x, FFXM_MIN16_F2 y, FFXM_MIN16_F a)
{
	return lerp(x, y, a);
}
FFXM_MIN16_F2 ffxLerp(FFXM_MIN16_F2 x, FFXM_MIN16_F2 y, FFXM_MIN16_F2 a)
{
	return lerp(x, y, a);
}
FFXM_MIN16_F3 ffxLerp(FFXM_MIN16_F3 x, FFXM_MIN16_F3 y, FFXM_MIN16_F a)
{
	return lerp(x, y, a);
}
FFXM_MIN16_F3 ffxLerp(FFXM_MIN16_F3 x, FFXM_MIN16_F3 y, FFXM_MIN16_F3 a)
{
	return lerp(x, y, a);
}
FFXM_MIN16_F4 ffxLerp(FFXM_MIN16_F4 x, FFXM_MIN16_F4 y, FFXM_MIN16_F a)
{
	return lerp(x, y, a);
}
FFXM_MIN16_F4 ffxLerp(FFXM_MIN16_F4 x, FFXM_MIN16_F4 y, FFXM_MIN16_F4 a)
{
	return lerp(x, y, a);
}

FFXM_MIN16_F ffxMax3Half(FFXM_MIN16_F x, FFXM_MIN16_F y, FFXM_MIN16_F z)
{
	return max(x, max(y, z));
}
FFXM_MIN16_F2 ffxMax3Half(FFXM_MIN16_F2 x, FFXM_MIN16_F2 y, FFXM_MIN16_F2 z)
{
	return max(x, max(y, z));
}
FFXM_MIN16_F3 ffxMax3Half(FFXM_MIN16_F3 x, FFXM_MIN16_F3 y, FFXM_MIN16_F3 z)
{
	return max(x, max(y, z));
}
FFXM_MIN16_F4 ffxMax3Half(FFXM_MIN16_F4 x, FFXM_MIN16_F4 y, FFXM_MIN16_F4 z)
{
	return max(x, max(y, z));
}

FFXM_MIN16_F ffxMin3Half(FFXM_MIN16_F x, FFXM_MIN16_F y, FFXM_MIN16_F z)
{
	return min(x, min(y, z));
}
FFXM_MIN16_F2 ffxMin3Half(FFXM_MIN16_F2 x, FFXM_MIN16_F2 y, FFXM_MIN16_F2 z)
{
	return min(x, min(y, z));
}
FFXM_MIN16_F3 ffxMin3Half(FFXM_MIN16_F3 x, FFXM_MIN16_F3 y, FFXM_MIN16_F3 z)
{
	return min(x, min(y, z));
}
FFXM_MIN16_F4 ffxMin3Half(FFXM_MIN16_F4 x, FFXM_MIN16_F4 y, FFXM_MIN16_F4 z)
{
	return min(x, min(y, z));
}

FFXM_MIN16_F ffxMed3Half(FFXM_MIN16_F x, FFXM_MIN16_F y, FFXM_MIN16_F z)
{
    return max(min(x, y), min(max(x, y), z));
}
FFXM_MIN16_F2 ffxMed3Half(FFXM_MIN16_F2 x, FFXM_MIN16_F2 y, FFXM_MIN16_F2 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FFXM_MIN16_F3 ffxMed3Half(FFXM_MIN16_F3 x, FFXM_MIN16_F3 y, FFXM_MIN16_F3 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FFXM_MIN16_F4 ffxMed3Half(FFXM_MIN16_F4 x, FFXM_MIN16_F4 y, FFXM_MIN16_F4 z)
{
    return max(min(x, y), min(max(x, y), z));
}

FFXM_MIN16_I ffxMed3Half(FFXM_MIN16_I x, FFXM_MIN16_I y, FFXM_MIN16_I z)
{
    return max(min(x, y), min(max(x, y), z));
}
FFXM_MIN16_I2 ffxMed3Half(FFXM_MIN16_I2 x, FFXM_MIN16_I2 y, FFXM_MIN16_I2 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FFXM_MIN16_I3 ffxMed3Half(FFXM_MIN16_I3 x, FFXM_MIN16_I3 y, FFXM_MIN16_I3 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FFXM_MIN16_I4 ffxMed3Half(FFXM_MIN16_I4 x, FFXM_MIN16_I4 y, FFXM_MIN16_I4 z)
{
    return max(min(x, y), min(max(x, y), z));
}

FFXM_MIN16_F ffxReciprocalHalf(FFXM_MIN16_F x)
{
	return rcp(x);
}
FFXM_MIN16_F2 ffxReciprocalHalf(FFXM_MIN16_F2 x)
{
	return rcp(x);
}
FFXM_MIN16_F3 ffxReciprocalHalf(FFXM_MIN16_F3 x)
{
	return rcp(x);
}
FFXM_MIN16_F4 ffxReciprocalHalf(FFXM_MIN16_F4 x)
{
	return rcp(x);
}

FFXM_MIN16_F ffxReciprocalSquareRootHalf(FFXM_MIN16_F x)
{
	return rsqrt(x);
}
FFXM_MIN16_F2 ffxReciprocalSquareRootHalf(FFXM_MIN16_F2 x)
{
	return rsqrt(x);
}
FFXM_MIN16_F3 ffxReciprocalSquareRootHalf(FFXM_MIN16_F3 x)
{
	return rsqrt(x);
}
FFXM_MIN16_F4 ffxReciprocalSquareRootHalf(FFXM_MIN16_F4 x)
{
	return rsqrt(x);
}

FFXM_MIN16_F ffxSaturate(FFXM_MIN16_F x)
{
	return saturate(x);
}
FFXM_MIN16_F2 ffxSaturate(FFXM_MIN16_F2 x)
{
	return saturate(x);
}
FFXM_MIN16_F3 ffxSaturate(FFXM_MIN16_F3 x)
{
	return saturate(x);
}
FFXM_MIN16_F4 ffxSaturate(FFXM_MIN16_F4 x)
{
	return saturate(x);
}

FFXM_MIN16_U ffxBitShiftRightHalf(FFXM_MIN16_U a, FFXM_MIN16_U b)
{
	return FFXM_MIN16_U(FFXM_MIN16_I(a) >> FFXM_MIN16_I(b));
}
FFXM_MIN16_U2 ffxBitShiftRightHalf(FFXM_MIN16_U2 a, FFXM_MIN16_U2 b)
{
	return FFXM_MIN16_U2(FFXM_MIN16_I2(a) >> FFXM_MIN16_I2(b));
}
FFXM_MIN16_U3 ffxBitShiftRightHalf(FFXM_MIN16_U3 a, FFXM_MIN16_U3 b)
{
	return FFXM_MIN16_U3(FFXM_MIN16_I3(a) >> FFXM_MIN16_I3(b));
}
FFXM_MIN16_U4 ffxBitShiftRightHalf(FFXM_MIN16_U4 a, FFXM_MIN16_U4 b)
{
	return FFXM_MIN16_U4(FFXM_MIN16_I4(a) >> FFXM_MIN16_I4(b));
}
#endif 

#if defined(FFXM_WAVE)

FfxFloat32 AWaveXorF1(FfxFloat32 v, FfxUInt32 x)
{
    return WaveReadLaneAt(v, WaveGetLaneIndex() ^ x);
}
FfxFloat32x2 AWaveXorF2(FfxFloat32x2 v, FfxUInt32 x)
{
    return WaveReadLaneAt(v, WaveGetLaneIndex() ^ x);
}
FfxFloat32x3 AWaveXorF3(FfxFloat32x3 v, FfxUInt32 x)
{
    return WaveReadLaneAt(v, WaveGetLaneIndex() ^ x);
}
FfxFloat32x4 AWaveXorF4(FfxFloat32x4 v, FfxUInt32 x)
{
    return WaveReadLaneAt(v, WaveGetLaneIndex() ^ x);
}
FfxUInt32 AWaveXorU1(FfxUInt32 v, FfxUInt32 x)
{
    return WaveReadLaneAt(v, WaveGetLaneIndex() ^ x);
}
FfxUInt32x2 AWaveXorU1(FfxUInt32x2 v, FfxUInt32 x)
{
    return WaveReadLaneAt(v, WaveGetLaneIndex() ^ x);
}
FfxUInt32x3 AWaveXorU1(FfxUInt32x3 v, FfxUInt32 x)
{
    return WaveReadLaneAt(v, WaveGetLaneIndex() ^ x);
}
FfxUInt32x4 AWaveXorU1(FfxUInt32x4 v, FfxUInt32 x)
{
    return WaveReadLaneAt(v, WaveGetLaneIndex() ^ x);
}
FfxBoolean AWaveIsFirstLane()
{
    return WaveIsFirstLane();
}
FfxUInt32 AWaveLaneIndex()
{
    return WaveGetLaneIndex();
}
FfxBoolean AWaveReadAtLaneIndexB1(FfxBoolean v, FfxUInt32 x)
{
    return WaveReadLaneAt(v, x);
}
FfxUInt32 AWavePrefixCountBits(FfxBoolean v)
{
    return WavePrefixCountBits(v);
}
FfxUInt32 AWaveActiveCountBits(FfxBoolean v)
{
    return WaveActiveCountBits(v);
}
FfxUInt32 AWaveReadLaneFirstU1(FfxUInt32 v)
{
    return WaveReadLaneFirst(v);
}
FfxUInt32 WaveOr(FfxUInt32 a)
{
    return WaveActiveBitOr(a);
}
FfxFloat32 WaveMin(FfxFloat32 a)
{
    return WaveActiveMin(a);
}
FfxFloat32 WaveMax(FfxFloat32 a)
{
    return WaveActiveMax(a);
}
FfxUInt32 WaveLaneCount()
{
    return WaveGetLaneCount();
}
FfxBoolean WaveAllTrue(FfxBoolean v)
{
    return WaveActiveAllTrue(v);
}
FfxFloat32 QuadReadX(FfxFloat32 v)
{
    return QuadReadAcrossX(v);
}
FfxFloat32x2 QuadReadX(FfxFloat32x2 v)
{
    return QuadReadAcrossX(v);
}
FfxFloat32 QuadReadY(FfxFloat32 v)
{
    return QuadReadAcrossY(v);
}
FfxFloat32x2 QuadReadY(FfxFloat32x2 v)
{
    return QuadReadAcrossY(v);
}

#if FFXM_HALF
FfxFloat16x2 ffxWaveXorFloat16x2(FfxFloat16x2 v, FfxUInt32 x)
{
    return FFXM_UINT32_TO_FLOAT16X2(WaveReadLaneAt(FFXM_FLOAT16X2_TO_UINT32(v), WaveGetLaneIndex() ^ x));
}
FfxFloat16x4 ffxWaveXorFloat16x4(FfxFloat16x4 v, FfxUInt32 x)
{
    return FFXM_UINT32X2_TO_FLOAT16X4(WaveReadLaneAt(FFXM_FLOAT16X4_TO_UINT32X2(v), WaveGetLaneIndex() ^ x));
}
FfxUInt16x2 ffxWaveXorUint16x2(FfxUInt16x2 v, FfxUInt32 x)
{
    return FFXM_UINT32_TO_UINT16X2(WaveReadLaneAt(FFXM_UINT16X2_TO_UINT32(v), WaveGetLaneIndex() ^ x));
}
FfxUInt16x4 ffxWaveXorUint16x4(FfxUInt16x4 v, FfxUInt32 x)
{
    return FFXM_UINT32X2_TO_UINT16X4(WaveReadLaneAt(FFXM_UINT16X4_TO_UINT32X2(v), WaveGetLaneIndex() ^ x));
}
#endif 
#endif 
