

#define FSR_RCAS_LIMIT (0.25-(1.0/16.0))

 FFXM_STATIC void FsrRcasCon(FfxUInt32x4 con,

                            FfxFloat32 sharpness)
 {

     sharpness = exp2(-sharpness);
     FfxFloat32x2 hSharp  = {sharpness, sharpness};
     con[0] = ffxAsUInt32(sharpness);
     con[1] = packHalf2x16(hSharp);
     con[2] = 0;
     con[3] = 0;
 }

#if defined(FFXM_GPU)&&defined(FSR_RCAS_F)

 FfxFloat32x4 FsrRcasLoadF(FfxInt32x2 p);
 void FsrRcasInputF(inout FfxFloat32 r,inout FfxFloat32 g,inout FfxFloat32 b);

 void FsrRcasF(out FfxFloat32 pixR,  
               out FfxFloat32 pixG,
               out FfxFloat32 pixB,
#ifdef FSR_RCAS_PASSTHROUGH_ALPHA
               out FfxFloat32 pixA,
#endif
               FfxUInt32x2 ip,  
               FfxUInt32x4 con)
 {  

     FfxInt32x2   sp = FfxInt32x2(ip);
     FfxFloat32x3 b  = FsrRcasLoadF(sp + FfxInt32x2(0, -1)).rgb;
     FfxFloat32x3 d  = FsrRcasLoadF(sp + FfxInt32x2(-1, 0)).rgb;
#ifdef FSR_RCAS_PASSTHROUGH_ALPHA
     FfxFloat32x4 ee = FsrRcasLoadF(sp);
     FfxFloat32x3 e  = ee.rgb;
     pixA            = ee.a;
#else
     FfxFloat32x3 e = FsrRcasLoadF(sp).rgb;
#endif
     FfxFloat32x3 f = FsrRcasLoadF(sp + FfxInt32x2(1, 0)).rgb;
     FfxFloat32x3 h = FsrRcasLoadF(sp + FfxInt32x2(0, 1)).rgb;

     FfxFloat32 bR = b.r;
     FfxFloat32 bG = b.g;
     FfxFloat32 bB = b.b;
     FfxFloat32 dR = d.r;
     FfxFloat32 dG = d.g;
     FfxFloat32 dB = d.b;
     FfxFloat32 eR = e.r;
     FfxFloat32 eG = e.g;
     FfxFloat32 eB = e.b;
     FfxFloat32 fR = f.r;
     FfxFloat32 fG = f.g;
     FfxFloat32 fB = f.b;
     FfxFloat32 hR = h.r;
     FfxFloat32 hG = h.g;
     FfxFloat32 hB = h.b;

     FsrRcasInputF(bR, bG, bB);
     FsrRcasInputF(dR, dG, dB);
     FsrRcasInputF(eR, eG, eB);
     FsrRcasInputF(fR, fG, fB);
     FsrRcasInputF(hR, hG, hB);

     FfxFloat32 bL = bB * FfxFloat32(0.5) + (bR * FfxFloat32(0.5) + bG);
     FfxFloat32 dL = dB * FfxFloat32(0.5) + (dR * FfxFloat32(0.5) + dG);
     FfxFloat32 eL = eB * FfxFloat32(0.5) + (eR * FfxFloat32(0.5) + eG);
     FfxFloat32 fL = fB * FfxFloat32(0.5) + (fR * FfxFloat32(0.5) + fG);
     FfxFloat32 hL = hB * FfxFloat32(0.5) + (hR * FfxFloat32(0.5) + hG);

     FfxFloat32 nz = FfxFloat32(0.25) * bL + FfxFloat32(0.25) * dL + FfxFloat32(0.25) * fL + FfxFloat32(0.25) * hL - eL;
     nz            = ffxSaturate(abs(nz) * ffxApproximateReciprocalMedium(ffxMax3(ffxMax3(bL, dL, eL), fL, hL) - ffxMin3(ffxMin3(bL, dL, eL), fL, hL)));
     nz            = FfxFloat32(-0.5) * nz + FfxFloat32(1.0);

     FfxFloat32 mn4R = ffxMin(ffxMin3(bR, dR, fR), hR);
     FfxFloat32 mn4G = ffxMin(ffxMin3(bG, dG, fG), hG);
     FfxFloat32 mn4B = ffxMin(ffxMin3(bB, dB, fB), hB);
     FfxFloat32 mx4R = max(ffxMax3(bR, dR, fR), hR);
     FfxFloat32 mx4G = max(ffxMax3(bG, dG, fG), hG);
     FfxFloat32 mx4B = max(ffxMax3(bB, dB, fB), hB);

     FfxFloat32x2 peakC = FfxFloat32x2(1.0, -1.0 * 4.0);

     FfxFloat32 hitMinR = mn4R * rcp(FfxFloat32(4.0) * mx4R);
     FfxFloat32 hitMinG = mn4G * rcp(FfxFloat32(4.0) * mx4G);
     FfxFloat32 hitMinB = mn4B * rcp(FfxFloat32(4.0) * mx4B);
     FfxFloat32 hitMaxR = (peakC.x - mx4R) * rcp(FfxFloat32(4.0) * mn4R + peakC.y);
     FfxFloat32 hitMaxG = (peakC.x - mx4G) * rcp(FfxFloat32(4.0) * mn4G + peakC.y);
     FfxFloat32 hitMaxB = (peakC.x - mx4B) * rcp(FfxFloat32(4.0) * mn4B + peakC.y);
     FfxFloat32 lobeR   = max(-hitMinR, hitMaxR);
     FfxFloat32 lobeG   = max(-hitMinG, hitMaxG);
     FfxFloat32 lobeB   = max(-hitMinB, hitMaxB);
     FfxFloat32 lobe    = max(FfxFloat32(-FSR_RCAS_LIMIT), ffxMin(ffxMax3(lobeR, lobeG, lobeB), FfxFloat32(0.0))) * ffxAsFloat
     (con.x);

#ifdef FSR_RCAS_DENOISE
     lobe *= nz;
#endif

     FfxFloat32 rcpL = ffxApproximateReciprocalMedium(FfxFloat32(4.0) * lobe + FfxFloat32(1.0));
     pixR            = (lobe * bR + lobe * dR + lobe * hR + lobe * fR + eR) * rcpL;
     pixG            = (lobe * bG + lobe * dG + lobe * hG + lobe * fG + eG) * rcpL;
     pixB            = (lobe * bB + lobe * dB + lobe * hB + lobe * fB + eB) * rcpL;
     return;
 }
#endif

#if defined(FFXM_GPU) && FFXM_HALF == 1 && defined(FSR_RCAS_H)

 FfxFloat16x4 FsrRcasLoadH(FfxInt16x2 p);
 void FsrRcasInputH(inout FfxFloat16 r,inout FfxFloat16 g,inout FfxFloat16 b);

 void FsrRcasH(
 out FfxFloat16 pixR, 
 out FfxFloat16 pixG,
 out FfxFloat16 pixB,
 #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
  out FfxFloat16 pixA,
 #endif
 FfxUInt32x2 ip, 
 FfxUInt32x4 con){ 

  FfxInt16x2 sp=FfxInt16x2(ip);
  FfxFloat16x3 b=FsrRcasLoadH(sp+FfxInt16x2( 0,-1)).rgb;
  FfxFloat16x3 d=FsrRcasLoadH(sp+FfxInt16x2(-1, 0)).rgb;
  #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
   FfxFloat16x4 ee=FsrRcasLoadH(sp);
   FfxFloat16x3 e=ee.rgb;pixA=ee.a;
  #else
   FfxFloat16x3 e=FsrRcasLoadH(sp).rgb;
  #endif
  FfxFloat16x3 f=FsrRcasLoadH(sp+FfxInt16x2( 1, 0)).rgb;
  FfxFloat16x3 h=FsrRcasLoadH(sp+FfxInt16x2( 0, 1)).rgb;

  FfxFloat16 bR=b.r;
  FfxFloat16 bG=b.g;
  FfxFloat16 bB=b.b;
  FfxFloat16 dR=d.r;
  FfxFloat16 dG=d.g;
  FfxFloat16 dB=d.b;
  FfxFloat16 eR=e.r;
  FfxFloat16 eG=e.g;
  FfxFloat16 eB=e.b;
  FfxFloat16 fR=f.r;
  FfxFloat16 fG=f.g;
  FfxFloat16 fB=f.b;
  FfxFloat16 hR=h.r;
  FfxFloat16 hG=h.g;
  FfxFloat16 hB=h.b;

  FsrRcasInputH(bR,bG,bB);
  FsrRcasInputH(dR,dG,dB);
  FsrRcasInputH(eR,eG,eB);
  FsrRcasInputH(fR,fG,fB);
  FsrRcasInputH(hR,hG,hB);

  FfxFloat16 bL=bB*FFXM_BROADCAST_FLOAT16(0.5)+(bR*FFXM_BROADCAST_FLOAT16(0.5)+bG);
  FfxFloat16 dL=dB*FFXM_BROADCAST_FLOAT16(0.5)+(dR*FFXM_BROADCAST_FLOAT16(0.5)+dG);
  FfxFloat16 eL=eB*FFXM_BROADCAST_FLOAT16(0.5)+(eR*FFXM_BROADCAST_FLOAT16(0.5)+eG);
  FfxFloat16 fL=fB*FFXM_BROADCAST_FLOAT16(0.5)+(fR*FFXM_BROADCAST_FLOAT16(0.5)+fG);
  FfxFloat16 hL=hB*FFXM_BROADCAST_FLOAT16(0.5)+(hR*FFXM_BROADCAST_FLOAT16(0.5)+hG);

  FfxFloat16 nz=FFXM_BROADCAST_FLOAT16(0.25)*bL+FFXM_BROADCAST_FLOAT16(0.25)*dL+FFXM_BROADCAST_FLOAT16(0.25)*fL+FFXM_BROADCAST_FLOAT16(0.25)*hL-eL;
  nz=ffxSaturate(abs(nz)*ffxApproximateReciprocalMediumHalf(ffxMax3Half(ffxMax3Half(bL,dL,eL),fL,hL)-ffxMin3Half(ffxMin3Half(bL,dL,eL),fL,hL)));
  nz=FFXM_BROADCAST_FLOAT16(-0.5)*nz+FFXM_BROADCAST_FLOAT16(1.0);

  FfxFloat16 mn4R=min(ffxMin3Half(bR,dR,fR),hR);
  FfxFloat16 mn4G=min(ffxMin3Half(bG,dG,fG),hG);
  FfxFloat16 mn4B=min(ffxMin3Half(bB,dB,fB),hB);
  FfxFloat16 mx4R=max(ffxMax3Half(bR,dR,fR),hR);
  FfxFloat16 mx4G=max(ffxMax3Half(bG,dG,fG),hG);
  FfxFloat16 mx4B=max(ffxMax3Half(bB,dB,fB),hB);

  FfxFloat16x2 peakC=FfxFloat16x2(1.0,-1.0*4.0);

  FfxFloat16 hitMinR=mn4R*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mx4R);
  FfxFloat16 hitMinG=mn4G*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mx4G);
  FfxFloat16 hitMinB=mn4B*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mx4B);
  FfxFloat16 hitMaxR=(peakC.x-mx4R)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mn4R+peakC.y);
  FfxFloat16 hitMaxG=(peakC.x-mx4G)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mn4G+peakC.y);
  FfxFloat16 hitMaxB=(peakC.x-mx4B)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mn4B+peakC.y);
  FfxFloat16 lobeR=max(-hitMinR,hitMaxR);
  FfxFloat16 lobeG=max(-hitMinG,hitMaxG);
  FfxFloat16 lobeB=max(-hitMinB,hitMaxB);
  FfxFloat16 lobe=max(FFXM_BROADCAST_FLOAT16(-FSR_RCAS_LIMIT),min(ffxMax3Half(lobeR,lobeG,lobeB),FFXM_BROADCAST_FLOAT16(0.0)))*FFXM_UINT32_TO_FLOAT16X2(con.y).x;

  #ifdef FSR_RCAS_DENOISE
   lobe*=nz;
  #endif

  FfxFloat16 rcpL=ffxApproximateReciprocalMediumHalf(FFXM_BROADCAST_FLOAT16(4.0)*lobe+FFXM_BROADCAST_FLOAT16(1.0));
  pixR=(lobe*bR+lobe*dR+lobe*hR+lobe*fR+eR)*rcpL;
  pixG=(lobe*bG+lobe*dG+lobe*hG+lobe*fG+eG)*rcpL;
  pixB=(lobe*bB+lobe*dB+lobe*hB+lobe*fB+eB)*rcpL;
}
#endif

#if defined(FFXM_GPU)&& FFXM_HALF == 1 && defined(FSR_RCAS_HX2)

 FfxFloat16x4 FsrRcasLoadHx2(FfxInt16x2 p);
 void FsrRcasInputHx2(inout FfxFloat16x2 r,inout FfxFloat16x2 g,inout FfxFloat16x2 b);

 void FsrRcasDepackHx2(out FfxFloat16x4 pix0,out FfxFloat16x4 pix1,FfxFloat16x2 pixR,FfxFloat16x2 pixG,FfxFloat16x2 pixB){
  #ifdef FFXM_HLSL

   pix0.a=pix1.a=0.0;
  #endif
  pix0.rgb=FfxFloat16x3(pixR.x,pixG.x,pixB.x);
  pix1.rgb=FfxFloat16x3(pixR.y,pixG.y,pixB.y);}

 void FsrRcasHx2(

 out FfxFloat16x2 pixR,
 out FfxFloat16x2 pixG,
 out FfxFloat16x2 pixB,
 #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
  out FfxFloat16x2 pixA,
 #endif
 FfxUInt32x2 ip, 
 FfxUInt32x4 con){ 

  FfxInt16x2 sp0=FfxInt16x2(ip);
  FfxFloat16x3 b0=FsrRcasLoadHx2(sp0+FfxInt16x2( 0,-1)).rgb;
  FfxFloat16x3 d0=FsrRcasLoadHx2(sp0+FfxInt16x2(-1, 0)).rgb;
  #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
   FfxFloat16x4 ee0=FsrRcasLoadHx2(sp0);
   FfxFloat16x3 e0=ee0.rgb;pixA.r=ee0.a;
  #else
   FfxFloat16x3 e0=FsrRcasLoadHx2(sp0).rgb;
  #endif
  FfxFloat16x3 f0=FsrRcasLoadHx2(sp0+FfxInt16x2( 1, 0)).rgb;
  FfxFloat16x3 h0=FsrRcasLoadHx2(sp0+FfxInt16x2( 0, 1)).rgb;
  FfxInt16x2 sp1=sp0+FfxInt16x2(8,0);
  FfxFloat16x3 b1=FsrRcasLoadHx2(sp1+FfxInt16x2( 0,-1)).rgb;
  FfxFloat16x3 d1=FsrRcasLoadHx2(sp1+FfxInt16x2(-1, 0)).rgb;
  #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
   FfxFloat16x4 ee1=FsrRcasLoadHx2(sp1);
   FfxFloat16x3 e1=ee1.rgb;pixA.g=ee1.a;
  #else
   FfxFloat16x3 e1=FsrRcasLoadHx2(sp1).rgb;
  #endif
  FfxFloat16x3 f1=FsrRcasLoadHx2(sp1+FfxInt16x2( 1, 0)).rgb;
  FfxFloat16x3 h1=FsrRcasLoadHx2(sp1+FfxInt16x2( 0, 1)).rgb;

  FfxFloat16x2 bR=FfxFloat16x2(b0.r,b1.r);
  FfxFloat16x2 bG=FfxFloat16x2(b0.g,b1.g);
  FfxFloat16x2 bB=FfxFloat16x2(b0.b,b1.b);
  FfxFloat16x2 dR=FfxFloat16x2(d0.r,d1.r);
  FfxFloat16x2 dG=FfxFloat16x2(d0.g,d1.g);
  FfxFloat16x2 dB=FfxFloat16x2(d0.b,d1.b);
  FfxFloat16x2 eR=FfxFloat16x2(e0.r,e1.r);
  FfxFloat16x2 eG=FfxFloat16x2(e0.g,e1.g);
  FfxFloat16x2 eB=FfxFloat16x2(e0.b,e1.b);
  FfxFloat16x2 fR=FfxFloat16x2(f0.r,f1.r);
  FfxFloat16x2 fG=FfxFloat16x2(f0.g,f1.g);
  FfxFloat16x2 fB=FfxFloat16x2(f0.b,f1.b);
  FfxFloat16x2 hR=FfxFloat16x2(h0.r,h1.r);
  FfxFloat16x2 hG=FfxFloat16x2(h0.g,h1.g);
  FfxFloat16x2 hB=FfxFloat16x2(h0.b,h1.b);

  FsrRcasInputHx2(bR,bG,bB);
  FsrRcasInputHx2(dR,dG,dB);
  FsrRcasInputHx2(eR,eG,eB);
  FsrRcasInputHx2(fR,fG,fB);
  FsrRcasInputHx2(hR,hG,hB);

  FfxFloat16x2 bL=bB*FFXM_BROADCAST_FLOAT16X2(0.5)+(bR*FFXM_BROADCAST_FLOAT16X2(0.5)+bG);
  FfxFloat16x2 dL=dB*FFXM_BROADCAST_FLOAT16X2(0.5)+(dR*FFXM_BROADCAST_FLOAT16X2(0.5)+dG);
  FfxFloat16x2 eL=eB*FFXM_BROADCAST_FLOAT16X2(0.5)+(eR*FFXM_BROADCAST_FLOAT16X2(0.5)+eG);
  FfxFloat16x2 fL=fB*FFXM_BROADCAST_FLOAT16X2(0.5)+(fR*FFXM_BROADCAST_FLOAT16X2(0.5)+fG);
  FfxFloat16x2 hL=hB*FFXM_BROADCAST_FLOAT16X2(0.5)+(hR*FFXM_BROADCAST_FLOAT16X2(0.5)+hG);

  FfxFloat16x2 nz=FFXM_BROADCAST_FLOAT16X2(0.25)*bL+FFXM_BROADCAST_FLOAT16X2(0.25)*dL+FFXM_BROADCAST_FLOAT16X2(0.25)*fL+FFXM_BROADCAST_FLOAT16X2(0.25)*hL-eL;
  nz=ffxSaturate(abs(nz)*ffxApproximateReciprocalMediumHalf(ffxMax3Half(ffxMax3Half(bL,dL,eL),fL,hL)-ffxMin3Half(ffxMin3Half(bL,dL,eL),fL,hL)));
  nz=FFXM_BROADCAST_FLOAT16X2(-0.5)*nz+FFXM_BROADCAST_FLOAT16X2(1.0);

  FfxFloat16x2 mn4R=min(ffxMin3Half(bR,dR,fR),hR);
  FfxFloat16x2 mn4G=min(ffxMin3Half(bG,dG,fG),hG);
  FfxFloat16x2 mn4B=min(ffxMin3Half(bB,dB,fB),hB);
  FfxFloat16x2 mx4R=max(ffxMax3Half(bR,dR,fR),hR);
  FfxFloat16x2 mx4G=max(ffxMax3Half(bG,dG,fG),hG);
  FfxFloat16x2 mx4B=max(ffxMax3Half(bB,dB,fB),hB);

  FfxFloat16x2 peakC=FfxFloat16x2(1.0,-1.0*4.0);

  FfxFloat16x2 hitMinR=mn4R*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mx4R);
  FfxFloat16x2 hitMinG=mn4G*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mx4G);
  FfxFloat16x2 hitMinB=mn4B*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mx4B);
  FfxFloat16x2 hitMaxR=(peakC.x-mx4R)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mn4R+peakC.y);
  FfxFloat16x2 hitMaxG=(peakC.x-mx4G)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mn4G+peakC.y);
  FfxFloat16x2 hitMaxB=(peakC.x-mx4B)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mn4B+peakC.y);
  FfxFloat16x2 lobeR=max(-hitMinR,hitMaxR);
  FfxFloat16x2 lobeG=max(-hitMinG,hitMaxG);
  FfxFloat16x2 lobeB=max(-hitMinB,hitMaxB);
  FfxFloat16x2 lobe=max(FFXM_BROADCAST_FLOAT16X2(-FSR_RCAS_LIMIT),min(ffxMax3Half(lobeR,lobeG,lobeB),FFXM_BROADCAST_FLOAT16X2(0.0)))*FFXM_BROADCAST_FLOAT16X2(FFXM_UINT32_TO_FLOAT16X2(con.y).x);

  #ifdef FSR_RCAS_DENOISE
   lobe*=nz;
  #endif

  FfxFloat16x2 rcpL=ffxApproximateReciprocalMediumHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*lobe+FFXM_BROADCAST_FLOAT16X2(1.0));
  pixR=(lobe*bR+lobe*dR+lobe*hR+lobe*fR+eR)*rcpL;
  pixG=(lobe*bG+lobe*dG+lobe*hG+lobe*fG+eG)*rcpL;
  pixB=(lobe*bB+lobe*dB+lobe*hB+lobe*fB+eB)*rcpL;}
#endif
