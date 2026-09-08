

FfxFloat32x3 opAAddOneF3(FfxFloat32x3 d, FfxFloat32x3 a, FfxFloat32 b)
{
    d = a + ffxBroadcast3(b);
    return d;
}

FfxFloat32x3 opACpyF3(FfxFloat32x3 d, FfxFloat32x3 a)
{
    d = a;
    return d;
}

FfxFloat32x3 opAMulF3(FfxFloat32x3 d, FfxFloat32x3 a, FfxFloat32x3 b)
{
    d = a * b;
    return d;
}

FfxFloat32x3 opAMulOneF3(FfxFloat32x3 d, FfxFloat32x3 a, FfxFloat32 b)
{
    d = a * ffxBroadcast3(b);
    return d;
}

FfxFloat32x3 opARcpF3(FfxFloat32x3 d, FfxFloat32x3 a)
{
    d = rcp(a);
    return d;
}
