#ifndef V_BASIC3D
#define V_BASIC3D

const double C1_255d=0.003921568627451; //1.0f/255.0f

#include <mat.cpp>
#include <rgb.cpp>

//Color information in the 4 floats representation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
union COLORF
 {
 float ch[4];
 struct {float r,g,b,a;};
 
 void Set(float Rr=.0f,float Gg=.0f,float Bb=.0f,float Aa=1.0f)
  {
  r=Rr; g=Gg; b=Bb; a=Aa;
  }
 void SetB(BYTE Rr=0,BYTE Gg=0,BYTE Bb=0,BYTE Aa=0xff)
  {
  r=Rr*C1_255d; g=Gg*C1_255d; b=Bb*C1_255d; a=Aa*C1_255d;
  }
 void RGBA(DWORD rgba=0xff000000)
  {
  __asm
   {
   fld C1_255d
   mov edx,rgba
   mov eax,edx
   and eax,0xff
   mov rgba,eax
   fild rgba
   fmul st,st(1)
   fstp [ecx]
   shr edx,8
   mov eax,edx
   and eax,0xff
   mov rgba,eax
   fild rgba
   fmul st,st(1)
   fstp [ecx+4]
   shr edx,8
   mov eax,edx
   and eax,0xff
   mov rgba,eax
   fild rgba
   fmul st,st(1)
   fstp [ecx+8]
   shr edx,8
   mov rgba,edx
   fild rgba
   fmul st,st(1)
   fstp [ecx+12]
   ffree st
   }
  }
 DWORD rgba()
  {
  return ((BYTE)(a*255)<<24)|((BYTE)(b*255)<<16)|((BYTE)(g*255)<<8)|(BYTE)(r*255);
  }
 };

#endif