#pragma once
#define V_BLT_RGB

//RGB BLiTter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class BLTRGB
{
public:
 NAT Dlng,Dlat,Slng,Slat; //in pixels
 int DBpl,SBpl; //bytes per line (span)
 int DeBpl,SeBpl; //extra bytes per line
 NAT SBpp,DBpp; //B/pixel (should be signed)
 NAT Bpi; //B/index
 DWORD rolls; //B,G,R,A rolls
 DWORD mask[4]; //B,G,R,A masks
 DWORD Imask; //index mask
 void*pDst,*pSrc,*pLUT;
 void (*DoIt)(BLTRGB*); //pointer to function who performs blit
 DWORD Dpixform,Spixform;

 void PFs(DWORD);
 void Copy(void*,int,DWORD,void*,int,DWORD,NAT,NAT,int);
 void Zoom(void*,int,DWORD,NAT,NAT,void*,int,DWORD,NAT,NAT,int);
 void Lut(void*,int,DWORD,void*,int,DWORD,NAT,NAT,void*,NAT);
#ifdef _DEBUG
 void IDF();
#endif
};

//do nothing ----------------------------------------------------------------------------------
void BLTRGBnull(BLTRGB*pthis=NULL)
{
WARN(1,"Do nothing function called");
}

//copy rect with no format change ----------------------------------------------------------------
void BLTRGBsamepf(BLTRGB*pthis=NULL)
{
__asm
 {
 mov ebx,pthis
 push ebp
 mov edi,[ebx]pthis.pDst
 mov esi,[ebx]pthis.pSrc
 mov eax,[ebx]pthis.Dlng
 mov edx,[ebx]pthis.DBpp
 imul edx //eax=Dlng*DBpp
 mov ebp,[ebx]pthis.Dlat //use ebp as outer counter
 mov edx,[ebx]pthis.DeBpl
 mov ebx,[ebx]pthis.SeBpl //no more need for pthis
LNextLine:
  mov ecx,eax
  rep movsb
  add edi,edx       //next d line
  add esi,ebx       //next s line
  dec ebp //lat--
  jnz LNextLine //loop if lat>0
 pop ebp
 }
}

//stretch rect same pf PFs ----------------------------------------------------------------
void BLTRGBstretch(BLTRGB*pthis=NULL)
{
__asm
 {
 mov ebx,pthis
 push ebp
 mov edi,[ebx]pthis.pDst
 mov esi,[ebx]pthis.pSrc    //load pointers
 mov gpr0,0                 //use as vertical decision (vrap)
 mov eax,[ebx]pthis.Dlat
 mov gpr1,eax               //use as outer loop counter
LNextDLine:
  push esi                  //save pointer to begining of line
  xor edx,edx               //use EDX as hrap
  mov ebp,[ebx]pthis.Dlng   //use EBP as counter for inner loop
LNextDPixel:
   mov ecx,[ebx]pthis.DBpp
   rep movsb
   mov eax,edx
   add eax,[ebx]pthis.Slng
   cdq
   idiv [ebx]pthis.Dlng
   dec eax                  //esi already incresead by Bpp
   imul eax,[ebx]pthis.DBpp //eax=(edx+Slng)/Dlng*Bpp
   add esi,eax              //next src X
   dec ebp
   jnz LNextDPixel          //loop if ebp>0
  add edi,[ebx]pthis.DeBpl  //edi+=extra B
  pop esi                   //restore src pointer to begining of line
  mov eax,gpr0
  add eax,[ebx]pthis.Slat
  cdq
  idiv [ebx]pthis.Dlat
  mov gpr0,edx              //vrap%=Dlng
  imul [ebx]pthis.SBpl      //eax=(vrap+Slat)/Dlat*SBpl
  add esi,eax               //next src Y
  dec gpr1
  jnz LNextDLine            //loop if --Dlat>0
 pop ebp
 }
}

//copy rect with diffrent PFs ----------------------------------------------------------------
void BLTRGBcopy(BLTRGB*pthis=NULL)
{
__asm
 {
 mov ebx,pthis                  //ebx=this
 push ebp
 mov edi,[ebx]pthis.pDst
 mov esi,[ebx]pthis.pSrc        //load pointers
 mov ecx,[ebx]pthis.rolls       //ecx=rolls
 mov ebp,[ebx]pthis.Slng        //use ebp as inner loop counter
LNextLinePix:                   //label for both inner and outer loop
   mov eax,[esi]                //read pixel 1
   mov edx,eax                  //read pixel 2
   and eax,[ebx]pthis.mask[0]   //eax&=mask[0]
   rol eax,cl                   //eax=B
   ror ecx,8                    //cl=roll[1]
   and edx,[ebx]pthis.mask[4]   //edx&=mask[1]
   rol edx,cl
   or eax,edx                   //eax|=G
   ror ecx,8                    //cl=roll[2]
   mov edx,[esi]                //read pixel 3
   and edx,[ebx]pthis.mask[8]   //edx&=mask[2]
   rol edx,cl
   or eax,edx                   //eax|=R
   ror ecx,8                    //cl=roll[3]
   mov edx,[esi]                //read pixel 4
   and edx,[ebx]pthis.mask[12]  //edx&=mask[3]
   rol edx,cl
   or eax,edx                   //eax|=A
   ror ecx,8                    //cl=roll[0]
   mov [edi],eax                //write pixel
   add edi,[ebx]pthis.DBpp      //pdc+=DBpp
   add esi,[ebx]pthis.SBpp      //next src X
   dec ebp
   jnz LNextLinePix             //loop if lng>0
  add edi,[ebx]pthis.DeBpl      //edi+=extra B
  add esi,[ebx]pthis.SeBpl      //esi+=extra B
  mov ebp,[ebx]pthis.Slng
  dec [ebx]pthis.Slat
  jnz LNextLinePix              //loop if Slat>0
 mov eax,[ebx]pthis.Dlat
 mov [ebx]pthis.Slat,eax        //restore Slat=Dlat
 pop ebp
 }
} //takes advantage of the fact that Dlat=Slat

//stretch rect any PFs ---------------------------------------------------------------------------
void BLTRGBzoom(BLTRGB*pthis=NULL)
{
__asm
 {
 mov ebx,pthis                  //ebx=this
 push ebp
 mov ecx,[ebx]pthis.rolls       //ecx=rolls
 mov edi,[ebx]pthis.pDst
 mov esi,[ebx]pthis.pSrc        //load pointers
 mov eax,[ebx]pthis.Dlat
 mov gpr0,eax                   //use as outer loop counter
 mov gpr2,0                     //vrap
LNextDLine:
  xor ebp,ebp                   //hrap
  push esi                      //save pointer to begining of line
  mov eax,[ebx]pthis.Dlng
  mov gpr1,eax                  //use as counter for inner loop
LNextDPixel:
   mov eax,[esi]                //read pixel 1
   mov edx,eax                  //read pixel 2
   and eax,[ebx]pthis.mask[0]   //eax&=mask[0]
   rol eax,cl                   //eax=B
   ror ecx,8                    //cl=roll[1]
   and edx,[ebx]pthis.mask[4]   //edx&=mask[1]
   rol edx,cl
   or eax,edx                   //eax|=G
   ror ecx,8                    //cl=roll[2]
   mov edx,[esi]                //read pixel 3
   and edx,[ebx]pthis.mask[8]   //edx&=mask[2]
   rol edx,cl
   or eax,edx                   //eax|=R
   ror ecx,8                    //cl=roll[3]
   mov edx,[esi]                //read pixel 4
   and edx,[ebx]pthis.mask[12]  //edx&=mask[3]
   rol edx,cl
   or eax,edx                   //eax|=A
   ror ecx,8                    //cl=roll[0]
   mov [edi],eax                //write pixel
   add edi,[ebx]pthis.DBpp      //pdc+=DBpp
   mov eax,ebp
   add eax,[ebx]pthis.Slng
   cdq
   idiv [ebx]pthis.Dlng
   mov ebp,edx                  //hrap%=dlng
   imul [ebx]pthis.SBpp         //eax=(hrap+slng)/dlng*SBpp
   add esi,eax                  //next src X
   dec gpr1
   jnz LNextDPixel              //loop if gpr1>0
  add edi,[ebx]pthis.DeBpl      //edi+=extra B
  pop esi                       //restore src pointer to begining of line
  mov eax,gpr2
  add eax,[ebx]pthis.Slat
  cdq
  idiv [ebx]pthis.Dlat
  mov gpr2,edx                  //vrap%=dlng
  imul [ebx]pthis.SBpl          //eax=(vrap+slat)/dlat*sspan
  add esi,eax                   //next src Y
  dec gpr0
  jnz LNextDLine                //loop if gpr0>0
 pop ebp
 }
}

// BLTRGB <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//sets up masks&rolls based on Pixel Formats .................................................
inline void BLTRGB::PFs(DWORD channels=0x3210) //channels is used for "grey" or "alpha" copy
{
NAT Sbpc,Dbpc,Smsb,Dmsb; //bits per channel
rolls=0;
if(!(Spixform&0xffff0000)) Spixform|=0x32100000;
if(!(Dpixform&0xffff0000)) Dpixform|=0x32100000;
for(int ch=0;ch<=3;ch++)
 {
 Sbpc=PF_bcch(Spixform,channels&0xf);
 Dbpc=PF_bcch(Dpixform,ch);
 if(Sbpc&&Dbpc&&!(channels&0xc)) //channels low nibble <= 3
  {
  Smsb=PF_msb(Spixform,channels&0x3);
  Dmsb=PF_msb(Dpixform,ch);
  mask[ch]=BitMask(Smsb,Sbpc<Dbpc?Sbpc:Dbpc);
  rolls|=((Smsb<=Dmsb?Dmsb-Smsb:32+Dmsb-Smsb)&0x1f)<<(ch<<3); //BYTE ch in rolls
  }
 else //channels nibble > 3
  mask[ch]=0;
 channels>>=4; //next nibble
 }
}

//simple rect image copy .......................................................................
inline void BLTRGB::Copy(void*pD,int Dspan,DWORD Dpf,void*pS,int Sspan,DWORD Spf,NAT w,NAT h,int mod=0x3210) //channels contains the source channels
{
if(!pD||!pS)
 {
 DoIt=BLTRGBnull;
 return;
 }
pDst=pD;        pSrc=pS;
Dlng=Slng=w;    Dlat=Slat=h;
Dpixform=Dpf;   Spixform=Spf;
DBpp=ALIGN(PF_bpix(Dpixform),7)>>3;
SBpp=ALIGN(PF_bpix(Spixform),7)>>3;
DBpl=Dspan?Dspan:DBpp*Dlng;
SBpl=Sspan?Sspan:SBpp*Slng;
DeBpl=DBpl-DBpp*Dlng;
SeBpl=SBpl-SBpp*Slng;
if(Dpixform==Spixform)
 DoIt=BLTRGBsamepf;
else
 {
 PFs(mod&0xffff);
 DoIt=BLTRGBcopy;
 }
}

//simple rect image copy .......................................................................
inline void BLTRGB::Zoom(void*pD,int Dspan,DWORD Dpf,NAT Dw,NAT Dh,void*pS,int Sspan,DWORD Spf,NAT Sw,NAT Sh,int mod=0x3210) //channels contains the source channels
{
if(!pD||!pS)
 {
 DoIt=BLTRGBnull;
 return;
 }
if(Dw==Sw&&Dh==Sh)
 {
 Copy(pD,Dspan,Dpf,pS,Sspan,Spf,Dw,Dh);
 return;
 }
pDst=pD;        pSrc=pS;
Dlng=Dw;        Slng=Sw;
Dlat=Dh;        Slat=Sh;
Dpixform=Dpf;   Spixform=Spf;
DBpp=ALIGN(PF_bpix(Dpixform),7)>>3;
SBpp=ALIGN(PF_bpix(Spixform),7)>>3;
DBpl=Dspan?Dspan:DBpp*Dlng;
SBpl=Sspan?Sspan:SBpp*Slng;
DeBpl=DBpl-DBpp*Dlng;
SeBpl=SBpl-SBpp*Slng;
if(Dpixform==Spixform)
 DoIt=BLTRGBstretch;
else
 {
 PFs(mod&0xffff);
 DoIt=BLTRGBzoom;
 }
}

//copy indexed (palette) image to ARGB image (no resize) .........................................
inline void BLTRGB::Lut(void*pD,int Dspan,DWORD Dpf,void*pSi,int Sspan,DWORD Spf,NAT w,NAT h,void*plut,NAT bpi) //channels contains the source channels
{
Imask=BitMask(bpi-1,bpi);
Bpi=ALIGN(bpi,7)>>3;
}

#ifdef _DEBUG
//ids the function used ........................................................................
void BLTRGB::IDF()
{
if(DoIt==BLTRGBsamepf)
 sc(strbuf,"SamePF");
else if(DoIt==BLTRGBcopy)
 sc(strbuf,"Copy");
else if(DoIt==BLTRGBstretch)
 sc(strbuf,"Stretch");
else if(DoIt==BLTRGBzoom)
 sc(strbuf,"Zoom");
else if(DoIt==BLTRGBnull)
 sc(strbuf,"Null");
else
 sc(strbuf,"?");
status("BLTRGB::%s()",strbuf);
}
#endif

// BLTRGB >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
