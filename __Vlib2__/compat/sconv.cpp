#pragma once
#define V_SCONV

struct SCONV
 {
 DWORD SnrC,DnrC; //always mono output (SnrC must not be greater than 255)
 DWORD SBpS,DBpS; //src,dest B per sample (1|2)
 DWORD SFps,DFps; //src,dest Frames per sec (Hz)
 DWORD Sxor,Dxor; //masks for changing bit sign
 DWORD Sshl,Drol; //Sshl must be less than 32
 float scaleF,scaleB;   //scaleF=dest Frames/src Frames,scaleB=dest B/src B

 void Tf(DWORD Ssampform=0,DWORD Dsampform=0);
 DWORD Frame(void *pds,void *pss);
 void Blt(void *pds,void *pss,NAT count=1); //count=Source nr of Frames
 }sconv;
//**************************************************************************************

//Sound CONVertor <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void SCONV::Tf(DWORD Ssampform,DWORD Dsampform)
{
SnrC=Ssampform>>28;
DnrC=Dsampform>>28;
SBpS=(Ssampform&0xf800000)>>23;
DBpS=(Dsampform&0xf800000)>>23;
SFps=Ssampform&0xfffff;
DFps=Dsampform&0xfffff;
Sxor=SBpS==1?0x80:0;
Dxor=DBpS==1?0x80:0;
Sshl=32-(SBpS<<3);
Drol=(DBpS>=SBpS?DBpS-SBpS:4+DBpS-SBpS)<<3;
scaleF=(float)DFps/SFps;
scaleB=scaleF*DBpS/SBpS*DnrC/SnrC;
}
/*
inline void SCONV::From(WAVEFORMATEX swf)
{
SnrC=swf.nChannels;
SBpS=swf.wBitsPerSample>>3;
SFps=swf.nSamplesPerSec;
Sxor=SBpS==1?0x80:0;
Sshl=32-(SBpS<<3);
Drol=(DBpS>=SBpS?DBpS-SBpS:4+DBpS-SBpS)<<3;
scaleF=(float)DFps/SFps;
scaleB=scaleF*DBpS/SBpS*DnrC/SnrC;
}

inline void SCONV::To(DWORD Dsampform)
{
DnrC=Dsampform>>28;
DBpS=(Dsampform&0xf800000)>>23;
DFps=Dsampform&0xfffff;
Dxor=DBpS==1?0x80:0;
Drol=(DBpS>=SBpS?DBpS-SBpS:4+DBpS-SBpS)<<3;
scaleF=(float)DFps/SFps;
scaleB=scaleF*DBpS/SBpS*DnrC/SnrC;
}
*/
DWORD SCONV::Frame(void *pds,void *pss)
{
__asm
{
 mov ebx,ecx //ebx=this
 mov esi,pss
 xor eax,eax //eax will hold the sum of all source channels
 mov ecx,[ebx].Sshl
 mov edi,[ebx].SnrC //do for each sample in source channel
LNextSChannel:
  mov edx,[esi] //read sample
  add esi,[ebx].SBpS //next sample
  xor edx,[ebx].Sxor //to signed
  shl edx,cl
  sar edx,cl //sign extend
  add eax,edx
  dec edi
  jnz LNextSChannel
 cdq
 idiv [ebx].SnrC //average samples
 mov ecx,[ebx].Drol //cl=roll
 rol eax,cl //modify size
 xor eax,[ebx].Dxor //to unsigned
 mov edi,pds
 mov ecx,[ebx].DnrC //do for each sample in destination channel
LNextDChannel:
  mov [edi],eax //write sample
  add edi,[ebx].DBpS //next sample
  loop LNextDChannel 
 }
}

void SCONV::Blt(void *pds,void *pss,NAT count)
{
__asm
 {
 mov edi,ecx //edi=this
 mov ebx,[edi].SFps //ebx will be the decision register
LNextFrame:
  mov ecx,[edi].Sshl //cl=Sshl
  mov esi,pss //load source pointer
  xor eax,eax //eax will hold the sum of all source samples
LNextSFrame:
   add ecx,0x10000 //HIWORD(ecx)++
   mov ch,[edi] //do for each sample in source channel
LNextSChannel:
    mov edx,[esi] //read sample
    add esi,[edi].SBpS
    xor edx,[edi].Sxor //to signed
    shl edx,cl
    sar edx,cl //sign extend
    add eax,edx //store sample
    dec ch
    jnz LNextSChannel
   sub ebx,[edi].DFps
   jg LNextSFrame //loop if ebx>0
  shr ecx,16 //ecx=nr of samples in eax
  sub count,ecx
  imul ecx,[edi] //ecx=nr of frames in eax
  cdq
  idiv ecx //average samples
  mov ecx,[edi].Drol //cl=roll
  rol eax,cl //modify size
  xor eax,[edi].Dxor //to unsigned
  mov pss,esi //save source pointer
  mov esi,pds //load destination pointer
LNextDFrame:
   mov ecx,[edi].DnrC //do for each sample in destination channel
LNextDChannel:
    mov [esi],eax //write sample
    add esi,[edi].DBpS //next sample
    loop LNextDChannel
   add ebx,[edi].SFps
   jle LNextDFrame //loop if edi <=0
  mov pds,esi //save destination pointer
  cmp count,0
  jg LNextFrame //loop if count>0
 }
}
//Sound CONVertor >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
