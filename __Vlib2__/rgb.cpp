#pragma once
#define V_RGBTOOLS

#define C_TRANSPARENT 0x0F0f0f0f
#define C_ALPHA       0xff000000
#define C_BLACK       0xff000000
#define C_WHITE       0xffffffff
#define C_BLUE        0xff0000ff
#define C_GREEN       0xff00ff00
#define C_RED         0xffff0000
#define C_CYAN        0xff00ffff
#define C_MAGENTA     0xffff00ff
#define C_YELLOW      0xffffff00
#define C_XP          0x00ffffff //mask used for XP colors

#define RGB_DW(r,g,b) (DWORD)((b<<16)|(g<<8)|r) //COLORREF (for XP)
#define BGR_DW(r,g,b) (DWORD)((r<<16)|(g<<8)|b) //DX
#define RGBX_DW(r,g,b) (DWORD)(0xff000000|(b<<16)|(g<<8)|r) //GL_BGR
#define BGRX_DW(r,g,b) (DWORD)(0xff000000|(r<<16)|(g<<8)|b) //DX
#define RGBA_DW(r,g,b,a) (DWORD)(((a)<<24)|((b)<<16)|((g)<<8)|(r)) //COLORREF,GL_ABGR
#define BGRA_DW(r,g,b,a) (DWORD)(((a)<<24)|((r)<<16)|((g)<<8)|(b)) //DX
#define NBGRA_DW(r,g,b,a) (DWORD)((a<<24)|((255-r)<<16)|((255-g)<<8)|(255-b)) //compat
#define CMY_DW(c,m,y) (DWORD)(((y)<<16)|((m)<<8)|(c))
#define CMYK_DW(c,m,y,k) (DWORD)((k<<24)|((y)<<16)|((m)<<8)|(c))

#define notRGB(rgb) (DWORD)(rgb^0xffffff) //invert rgb bits
#define RGB_BGR(dw) (DWORD)((dw&0xff00ff00)|((dw&0xff0000)>>16)|((dw&0xff)<<16)) //swap R<->B
#define RGB_GBR(dw) (DWORD)((dw&0xff000000)|((dw&0xff0000)>>16)|((dw&0xffff)<<8)) //R<-G<-B<-R
#define RGB_BRG(dw) (DWORD)((dw&0xff000000)|((dw&0xffff00)>>8)|((dw&0xff)<<16)) //R->G->B->R

#define PF_bpix(pf) ((pf&0xf)+((pf&0xf0)>>4)+((pf&0xf00)>>8)+((pf&0xf000)>>12))
#define PF_bcch(pf,channel) (((pf&0xffff)>>(((pf>>(((channel)+4)<<2))&0xf)<<2))&0xf) //bits per color channel (0,1,2,3)
#define PF_make(a,r,g,b,c3,c2,c1,c0) (DWORD)((a<<28)|(r<<24)|(g<<20)|(b<<16)|(c3<<12)|(c2<<8)|(c1<<4)|c0)

//Color information in diffrent representation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
union COLOR
{
 COLORREF abgr; //WINDOWS COLOR FORMAT
 DWORD dw;
 float f;
 BYTE ch[4]; //channels
 struct {BYTE B,G,R,A;}; //d3d,dd,gdi
 struct {BYTE r,g,b,a;}; //COLORREF,gl
 struct {BYTE c,m,y,k;}; //CMYK subtractive colors
 struct {WORD h; BYTE s,l;}; //hue,sat,lum
 struct {BYTE c1,c2,c3,c4;}; //4 greys
 
 operator COLORREF() //COLORREF
  {
  return abgr;
  }
 COLOR& operator =(COLORREF cref) //=COLORREF
  {
  abgr=cref;
  return *this;
  }
 COLOR& operator()(BYTE ch0=0,BYTE ch1=0,BYTE ch2=0,BYTE ch3=0xff)//(,,,)
  {
  B=ch0; G=ch1; R=ch2; A=ch3;
  return *this;
  }
 COLOR& operator()(float ch0=0,float ch1=0,float ch2=0,float ch3=1)//(.f,.f,.f,.f)
  {
  B=ch0*255; G=ch1*255; R=ch2*255; A=ch3*255;
  return *this;
  }
};
//#define RGBA COLOR //obsolete alias

//*****************************************************************************************

//-------------------------------------------------------------------------
inline void RGBtoCMYK(BYTE r,BYTE g,BYTE b,BYTE&c,BYTE&m,BYTE&y,BYTE&k)
{
c=255-r;
m=255-g;
y=255-b;
k=MIN3(c,m,y);   //semnal de mod comun
c-=k;
m-=k;
y-=k;
}

//-------------------------------------------------------------------------
inline void CMYKtoRGBA(BYTE c,BYTE m,BYTE y,BYTE k,BYTE &r,BYTE &g,BYTE &b,BYTE &a)
{
a=255-k;
r=a-c;
g=a-m;
b=a-y;
a=255;
}

//matches a PF to a wide range supportted format -------------------------------------
inline DWORD std_PF(DWORD pixform=0)
{
fpuioi=PF_bpix(pixform);
if(fpuioi<=16) //0..16
 {
 if(pixform&0xf000) //with alpha
  return 0x32101555;
 else //without alpha
  return 0x32100565;
 }
else if(fpuioi<=24) //16..24
 return 0x32100888;
else //25..32
 return 0x32108888;
}

//returns the closest standard GDI supported PF matching bpp -------------------------------------
inline DWORD gdi_PF(int bpp=0)
{
if(bpp<=16) //0..16
 return 0x32101555;
else if(bpp>24) //25..32
 return 0x32108888;
else //16..24
 return 0x32100888;
}

//puts any remaining unused bits up to bpp in the alpha channel ------------------------
inline DWORD PF_align(DWORD pf,int bpp=32)
{
bpp-=(pf&0xf)+((pf&0xf0)>>4)+((pf&0xf00)>>8);
return (pf&0xffff0fff)|((bpp&0xf)<<12);
}

//returns the PF associated with RGB masks -----------------------------------------------
inline DWORD masks_PF(DWORD mask0,DWORD mask1,DWORD mask2,DWORD mask3=0,int ord=0x32100000)
{
return ord|(setbits(mask3)<<12)|(setbits(mask2)<<8)|(setbits(mask1)<<4)|setbits(mask0);
}

//returnes the PF associated with a number of b/color and b/alpha----------------------------
inline DWORD bpc_PF(int bpp=0,int bpa=0,int ord=0x32100000)
{
bpp=(bpp-bpa)/3;
return ord|(bpa<<12)|(bpp<<8)|(bpp<<4)|bpp;
}

//returnes the number of channels in a PF ---------------------------------------------------
inline int __cdecl PF_nrch(DWORD pixform)
{
__asm
 {
 mov edx,pixform
 and edx,0xffff
 xor ecx,ecx
 xor eax,eax
LNextChannel:
  test edx,0xf
  setnz cl
  add eax,ecx
  shr edx,4
  jnz LNextChannel //loop if edx!=0
 }
}

//returns a mask for the given channel-----------------------------------------------------
DWORD __cdecl PF_mask(DWORD pixform,NAT channel=3)
{
__asm
 {
 mov edx,pixform
 mov ecx,channel
 shl ecx,2 //channel*=4
 add ecx,16 //get HIWORD
 mov eax,edx
 shr eax,cl 
 and eax,0xf //edx=DW_NIB(pixform,channel+4)
 xor ebx,ebx //ebx=msb
LNextChannelMask:
  mov ecx,edx
  and ecx,0xf //ecx=DW_NIB(pixform,channel) (bcnt)
  add ebx,ecx //msb+=nibble
  shr edx,4 //next nibble
  dec eax
  jge LNextChannelMask //loop if eax>=0
 xor eax,eax //eax=mask
 jecxz LPF_maskRET //if(bcnt==0) return 0;
 not eax //eax=0xffffffff
 sub ecx,32 //bcnt-=32
 neg ecx //ecx=32-bcnt
 shl eax,cl //put ecx 0s in eax
 mov ecx,32
 sub ecx,ebx //ecx=32-(msb-1)
 ror eax,cl //put mask in position
LPF_maskRET:
 }
}

//returns the offset of the MSBit for a color channel ----------------------------------
inline NAT __cdecl PF_msb(DWORD pixform=0,int channel=0)
{
__asm
 {
 xor eax,eax //eax=msb
 mov edx,pixform
 mov ecx,channel
 shl ecx,2 //channel*=4
 add ecx,16 //get HIWORD
 mov ebx,edx
 shr ebx,cl 
 and ebx,0xf //edx=DW_NIB(pixform,channel+4)
LNextChannelMSB:
  mov ecx,edx
  and ecx,0xf //get nibble
  add eax,ecx //msb+=nibble
  shr edx,4 //next nibble
  dec ebx
  jge LNextChannelMSB //loop if edx>=0
 dec eax //msb--
 }
}

//returns the offset of the LSBit for a color channel (shift)----------------------------------
inline NAT __cdecl PF_lsb(DWORD pixform=0,int channel=0)
{
__asm
 {
 xor eax,eax //eax=lsb
 mov edx,pixform
 mov ecx,channel
 shl ecx,2 //channel*=4
 add ecx,16 //get HIWORD
 mov ebx,edx
 shr ebx,cl 
 and ebx,0xf //edx=DW_NIB(pixform,channel+4)
 mov ecx,ebx
 jecxz LPF_lsbRET 
LNextChannelLSB:
  mov ebx,edx
  and ebx,0xf //get nibble
  add eax,ebx //msb+=nibble
  shr edx,4 //next nibble
  loop LNextChannelLSB //loop if edx>0
LPF_lsbRET:
 }
}

//linearly interpolates betwen 2 RGBA colors by component ------------------------------
DWORD __cdecl RGBAlerp(DWORD src1=0,DWORD src2=0,float ratio=.5f)
{
__asm
 {
 mov edi,src1
 mov esi,src2   //save params
 fld ratio      //st()=ratio
 xor eax,eax
 mov ecx,4
LNextChannel:
  mov edx,edi
  mov ebx,esi
  and edx,0xff
  and ebx,0xff
  sub ebx,edx    //ebx=src2-src1
  mov src1,ebx
  fild src1
  fmul st,st(1)
  fistp src1
  mov ebx,src1   //ebx=(src2-src1)*ratio
  add edx,ebx    //edx=(src2-src1)*ratio+src1
  or eax,edx
  ror eax,8      //eax|=edx<<(4-cl)
  ror edi,8
  ror esi,8      //next channel
  loop LNextChannel
 ffree st
 }
}

//-----------------------------------------------------------------------
void RGBtoHSL(BYTE R,BYTE G,BYTE B,BYTE&h,BYTE&s,BYTE&l)
{
double var_R,var_G,var_B,var_Min,var_Max,del_Max,H,S,L,del_R,del_G,del_B;
var_R=(double)R/255.;
var_G=(double)G/255.;
var_B=(double)B/255.;
var_Min=MIN3(var_R,var_G,var_B);    //Min. value of RGB
var_Max=MAX3(var_R,var_G,var_B);    //Max. value of RGB
del_Max=var_Max-var_Min;            //Delta RGB value
L=(var_Max+var_Min)/2;
if(del_Max==0) //This is a gray, no chroma...
 H=S=0.;
else           //Chromatic data...
 {
 if(L<0.5)
  S=del_Max/(var_Max+var_Min);
 else
  S=del_Max/(2.-var_Max-var_Min);
 del_R=(((var_Max-var_R)/6.)+(del_Max/2.))/del_Max;
 del_G=(((var_Max-var_G)/6.)+(del_Max/2.))/del_Max;
 del_B=(((var_Max-var_B)/6.)+(del_Max/2.))/del_Max;
 if(var_R==var_Max)
  H=del_B-del_G;
 else if(var_G==var_Max)
  H=del_R-del_B+(1./3.);
 else// if(var_B==var_Max)
  H=del_G-del_R+(2./3.);
 if(H<0.) H+=1.;
 if(H>1.) H-=1.;
 }
h=H*240;
s=S*240;
l=L*240;
}

//--------------------------------------------------------------------------
inline float Hue_2_RGB(float v1,float v2,float vH )
{
   if ( vH < 0. ) vH += 1.;
   if ( vH > 1. ) vH -= 1.;
   if ( ( 6. * vH ) < 1. ) return ( v1 + ( v2 - v1 ) * 6. * vH );
   if ( ( 2. * vH ) < 1. ) return ( v2 );
   if ( ( 3. * vH ) < 2. ) return ( v1 + ( v2 - v1 ) * ( ( 2. / 3. ) - vH ) * 6. );
   return ( v1 );
}


//--------------------------------------------------------------------------
void HSLtoRGB(BYTE h,BYTE s,BYTE l,BYTE&R,BYTE&G,BYTE&B)
{
double H,S,L,var_1,var_2;
H=(double)h/240.;
S=(double)s/240.;
L=(double)l/240.;
if ( S == 0 )                       //HSL values = 0 ÷ 1
{
R=G=B=L*255.;                     //RGB results = 0 ÷ 255
}
else
{
   if ( L < 0.5 ) var_2 = L * ( 1. + S );
   else           var_2 = ( L + S ) - ( S * L );

   var_1 = 2. * L - var_2;

   R = 255. * Hue_2_RGB( var_1, var_2, H + ( 1. / 3. ) ) ;
   G = 255. * Hue_2_RGB( var_1, var_2, H );
   B = 255. * Hue_2_RGB( var_1, var_2, H - ( 1. / 3. ) );
} 
}

//creates a pallette for grey BMP --------------------------------------------
void GreyPal(COLOR*rgbs,NAT count=256,BYTE a=0)
{
float delta;
delta=(float)255./(count-1);
for(int i=0;i<count;i++)
 {
 rgbs[i].b=rgbs[i].g=rgbs[i].r=i*delta;
 rgbs[i].a=a;
 }
}

//creates count shades of a given RGB color --------------------------------------------
void RGBShades(COLOR*rgbs,NAT count=1,BYTE r=255,BYTE g=255,BYTE b=255)
{
int delta;
if(r>=g&&r>=b) delta=255-r;
else if(g>=r&&g>=b) delta=255-g;
else delta=255-b;
rgbs[0].r=r+delta;
rgbs[0].g=g+delta;
rgbs[0].b=b+delta;
rgbs[0].a=0;
if(r<=g&&r<=b) delta+=r;
else if(g<=r&&g<=b) delta+=g;
else delta+=b;
float step;
step=(float)delta/(count-1);
for(int l=1;l<count;l++)
 {
 rgbs[l].r=rgbs[0].r-l*step;
 rgbs[l].g=rgbs[0].g-l*step;
 rgbs[l].b=rgbs[0].b-l*step;
 rgbs[l].a=0;
 }
}

//.............................................................................
inline void RandomColor(COLOR*pc,BYTE min=128,BYTE max=255)
{
pc->r=RAND(min,max);
pc->g=RAND(min,max);
pc->b=RAND(min,max);
pc->a=0;
}

//standard windows dialog for RGB colors (keeps A) -------------------------------------
COLORREF GetColorRef(DWORD color,HWND parent=HWND_DESKTOP,int iscref=1)
{
if(!iscref) color=RGB_BGR(color);
COLORREF cust[16];
CHOOSECOLOR cc;
ZeroMemory(&cc,sizeof(cc));
cc.lStructSize=sizeof(cc);
cc.hwndOwner=parent;
cc.Flags=CC_RGBINIT|CC_FULLOPEN;
cc.rgbResult=color&0xffffff;
cc.lpCustColors=(COLORREF*)cust;
RGBShades((COLOR*)cust,16,GetRValue(color),GetGValue(color),GetBValue(color));
ChooseColor(&cc);
cc.rgbResult|=color&0xff000000;
return (iscref?cc.rgbResult:RGB_BGR(cc.rgbResult));
}

#include <compat/cconv.cpp>

//builds default palette for ARGB simulated indexed modes -----------------------------
BYTE* PF_palette(BYTE *pal=NULL,DWORD indpf=0x32100332,DWORD palpf=0x32108888)
{
NAT nrc;
nrc=1<<PF_bpix(indpf);
if(nrc>256) return NULL; //max 256 colors pal
CCONV lcc;
lcc.Tf(indpf,palpf);
if(!pal) pal=(BYTE*)malloc(lcc.DBpp*nrc);
for(NAT ci=0;ci<nrc;ci++)
 lcc.BGRA(pal+ci*lcc.DBpp,ci,ci,ci,ci);
return pal;
}

#include <bltrgb.cpp>