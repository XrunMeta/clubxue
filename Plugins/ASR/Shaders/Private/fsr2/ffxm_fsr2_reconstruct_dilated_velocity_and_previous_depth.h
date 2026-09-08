

#ifndef FFXM_FSR2_RECONSTRUCT_DILATED_VELOCITY_AND_PREVIOUS_DEPTH_H
#define FFXM_FSR2_RECONSTRUCT_DILATED_VELOCITY_AND_PREVIOUS_DEPTH_H

struct ReconstructPrevDepthOutputs
{
    FfxFloat32 fDepth;
    FfxFloat32x2 fMotionVector;
    FfxFloat32 fLuma;
};

void ReconstructPrevDepth(FfxInt32x2 iPxPos, FfxFloat32 fDepth, FfxFloat32x2 fMotionVector, FfxInt32x2 iPxDepthSize)
{
    fMotionVector *= FfxFloat32(length(fMotionVector * DisplaySize()) > 0.1f);

    FfxFloat32x2 fUv = (iPxPos + FfxFloat32(0.5)) / iPxDepthSize;
    FfxFloat32x2 fReprojectedUv = fUv + fMotionVector;

    BilinearSamplingData bilinearInfo = GetBilinearSamplingData(fReprojectedUv, RenderSize());

    for (FfxInt32 iSampleIndex = 0; iSampleIndex < 4; iSampleIndex++) {

        const FfxInt32x2 iOffset = bilinearInfo.iOffsets[iSampleIndex];
        FfxFloat32 fWeight = bilinearInfo.fWeights[iSampleIndex];

        if (fWeight > fReconstructedDepthBilinearWeightThreshold) {

            FfxInt32x2 iStorePos = bilinearInfo.iBasePos + iOffset;
            if (IsOnScreen(iStorePos, iPxDepthSize)) {
                StoreReconstructedDepth(iStorePos, fDepth);
            }
        }
    }
}

void FindNearestDepth(FFXM_PARAMETER_IN FfxInt32x2 iPxPos, FFXM_PARAMETER_IN FfxInt32x2 iPxSize, FFXM_PARAMETER_OUT FfxFloat32 fNearestDepth, FFXM_PARAMETER_OUT FfxInt32x2 fNearestDepthCoord)
{
    const FfxInt32 iSampleCount = 9;
    const FfxInt32x2 iSampleOffsets[iSampleCount] = {
        FfxInt32x2(+0, +0),
        FfxInt32x2(+1, +0),
        FfxInt32x2(+0, +1),
        FfxInt32x2(+0, -1),
        FfxInt32x2(-1, +0),
        FfxInt32x2(-1, +1),
        FfxInt32x2(+1, +1),
        FfxInt32x2(-1, -1),
        FfxInt32x2(+1, -1),
    };

    FfxFloat32 depth[9];
    FfxInt32 iSampleIndex = 0;
    FFXM_UNROLL
    for (iSampleIndex = 0; iSampleIndex < iSampleCount; ++iSampleIndex) {

        FfxInt32x2 iPos = iPxPos + iSampleOffsets[iSampleIndex];
        depth[iSampleIndex] = LoadInputDepth(iPos);
    }

    fNearestDepthCoord = iPxPos;
    fNearestDepth = depth[0];
    FFXM_UNROLL
    for (iSampleIndex = 1; iSampleIndex < iSampleCount; ++iSampleIndex) {

        FfxInt32x2 iPos = iPxPos + iSampleOffsets[iSampleIndex];
        if (IsOnScreen(iPos, iPxSize)) {

            FfxFloat32 fNdDepth = depth[iSampleIndex];
#if FFXM_FSR2_OPTION_INVERTED_DEPTH
            if (fNdDepth > fNearestDepth) {
#else
            if (fNdDepth < fNearestDepth) {
#endif
                fNearestDepthCoord = iPos;
                fNearestDepth = fNdDepth;
            }
        }
    }
}

FfxFloat32 ComputeLockInputLuma(FfxInt32x2 iPxLrPos)
{

    FfxFloat32x3 fRgb = ffxMax(FfxFloat32x3(0, 0, 0), LoadInputColor(iPxLrPos));

    fRgb /= PreExposure();
    fRgb *= Exposure();

#if FFXM_FSR2_OPTION_HDR_COLOR_INPUT
    fRgb = Tonemap(fRgb);
#endif

    const FfxFloat32 fLockInputLuma = ffxPow(RGBToPerceivedLuma(fRgb), FfxFloat32(1.0 / 6.0));

    return fLockInputLuma;
}

ReconstructPrevDepthOutputs ReconstructAndDilate(FfxInt32x2 iPxLrPos)
{
    FfxFloat32 fDilatedDepth;
    FfxInt32x2 iNearestDepthCoord;

    FindNearestDepth(iPxLrPos, RenderSize(), fDilatedDepth, iNearestDepthCoord);

#if FFXM_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS
    FfxInt32x2 iSamplePos = iPxLrPos;
    FfxInt32x2 iMotionVectorPos = iNearestDepthCoord;
#else
    FfxInt32x2 iSamplePos = ComputeHrPosFromLrPos(iPxLrPos);
    FfxInt32x2 iMotionVectorPos = ComputeHrPosFromLrPos(iNearestDepthCoord);
#endif

    FfxFloat32x2 fDilatedMotionVector = LoadInputMotionVector(iMotionVectorPos);

    ReconstructPrevDepthOutputs results;

    results.fDepth = fDilatedDepth;
    results.fMotionVector = fDilatedMotionVector;
    ReconstructPrevDepth(iPxLrPos, fDilatedDepth, fDilatedMotionVector, RenderSize());
    FfxFloat32 fLockInputLuma = ComputeLockInputLuma(iPxLrPos);
    results.fLuma = fLockInputLuma;

    return results;
}

#endif 
