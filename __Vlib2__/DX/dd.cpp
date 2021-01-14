#pragma once
#define V_IDIRECTDRAW

#include <gdi.cpp>
#include <ddraw.h>
#pragma comment(lib,"ddraw.lib")
#pragma comment(lib,"dxguid.lib")

IDirectDraw7 *ddobj=NULL; //global DD object
IDirectDrawSurface7 *ddsprim=NULL; //main window primary surface
IDirectDrawSurface7 *ddsbbuf=NULL; //back buffer surface (for 2 surfaces flip chains)
IDirectDrawClipper *ddmwclip=NULL; //clipper assigned to main window

//tests erret ---------------------------------------------------------------------------
int isDDOk(LPSTR errmsg=NULL)
{
if(erret==DD_OK) return 1;
sprintf(strbuf,"%x %u\n%s\nContinue ?",HRESULT_FACILITY(erret),HRESULT_CODE(erret),errmsg);
if(MessageBox(hmwnd,strbuf,"DirectDraw ERROR",MB_ICONERROR|MB_YESNO|MB_DEFBUTTON2)==IDNO)
 exit(1);
return 0;
}

//returns PF from a DD PF --------------------------------------------------------------
inline DWORD DDPFtoPF(DDPIXELFORMAT ddpf)
{
ddpf.dwRBitMask=setbits(ddpf.dwRBitMask);
ddpf.dwGBitMask=setbits(ddpf.dwGBitMask);
ddpf.dwBBitMask=setbits(ddpf.dwBBitMask);
ddpf.dwRGBAlphaBitMask=ddpf.dwRGBBitCount-ddpf.dwRBitMask-ddpf.dwGBitMask-ddpf.dwBBitMask;
return (ddpf.dwRGBAlphaBitMask<<12)|(ddpf.dwRBitMask<<8)|(ddpf.dwGBitMask<<4)|ddpf.dwBBitMask|0x32100000;
}

//inits a DD PF struc from a PF -----------------------------------------------------------
DDPIXELFORMAT PFtoDDPF(DWORD pixform=0)
{
DDPIXELFORMAT ddpf;
ddpf.dwSize=sizeof(DDPIXELFORMAT);
ddpf.dwFlags=DDPF_RGB;
ddpf.dwFourCC=pixform;
ddpf.dwRGBBitCount=ALIGN(PF_bpix(pixform),7);
ddpf.dwRBitMask=PF_mask(pixform,2);
ddpf.dwGBitMask=PF_mask(pixform,1);
ddpf.dwBBitMask=PF_mask(pixform,0);
ddpf.dwRGBAlphaBitMask=PF_mask(pixform,3);
if(ddpf.dwRGBAlphaBitMask) ddpf.dwFlags|=DDPF_ALPHAPIXELS;
return ddpf;
}

//init DD (full screen set mode) ------------------------------------------------------
void InitDD(int bpp=0,int hres=0,int vres=0,int bbcnt=0,int useclip=1)
{
erret=DirectDrawCreateEx(NULL,(void**)&ddobj,IID_IDirectDraw7,NULL); 
isDDOk("Object creation failed");
erret=ddobj->SetCooperativeLevel(hmwnd,bpp||bbcnt?DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE:DDSCL_NORMAL);
isDDOk("CooperativeLevel refused");
ModeInfo(scrbpp,scrw,scrh);
if(bpp)
 {
 if(!hres) hres=scrw;
 if(!vres) vres=scrh;
 erret=ddobj->SetDisplayMode(hres,vres,bpp,0,0);
 isDDOk("Mode change failed 1");
 }
else if(hres)
 {
 if(!vres) vres=(hres*3)>>2;
 erret=ddobj->SetDisplayMode(hres,vres,scrbpp,0,0);
 isDDOk("Mode change failed 2");
 }
DDSURFACEDESC2 ddsd;
ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
ddsd.dwSize=sizeof(DDSURFACEDESC2);
ddsd.dwFlags=DDSD_CAPS;
ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
if(bbcnt)
 {
 ddsd.dwFlags|=DDSD_BACKBUFFERCOUNT; 
 ddsd.dwBackBufferCount=bbcnt;
 ddsd.ddsCaps.dwCaps|=DDSCAPS_COMPLEX|DDSCAPS_FLIP;
 }
erret=ddobj->CreateSurface(&ddsd,&ddsprim,NULL);
isDDOk("Primary surface creation failed");
if(bbcnt)
 {
 ddsd.ddsCaps.dwCaps=DDSCAPS_BACKBUFFER;
 erret=ddsprim->GetAttachedSurface(&ddsd.ddsCaps,&ddsbbuf);
 isDDOk("Back buffer retrieval failed");
 }
else ddsbbuf=ddsprim;//if no flip chain backbuffer=frontbuffer=primary surface
if(useclip)
 {
 erret=ddobj->CreateClipper(0,&ddmwclip,NULL);
 isDDOk("Clipper creation failed");
 erret=ddmwclip->SetHWnd(0,hmwnd);
 isDDOk("Clipper assignation to main window failed");
 erret=ddsprim->SetClipper(ddmwclip);
 isDDOk("Clipper assignation to primary surface failed");
 }
erret=ddsprim->GetSurfaceDesc(&ddsd);
isDDOk("Get primary surface description");
scrw=ddsd.dwWidth;
scrh=ddsd.dwHeight;
scrbpp=ddsd.ddpfPixelFormat.dwRGBBitCount;
scrpf=DDPFtoPF(ddsd.ddpfPixelFormat);
}

//release DD --------------------------------------------------------------------------
void FreeDD()
{
if(ddsbbuf!=ddsprim) 
 RelCOM(ddsbbuf,"DD main back buffer");
ddsbbuf=NULL;
RelCOM(ddsprim,"DD primary surface");
ddsprim=NULL;
RelCOM(ddmwclip,"DD main clipper"); //making sure (should be released with ddsprim)
ddmwclip=NULL;
RelCOM(ddobj,"DD object");
ddobj=NULL;
InitMode();
}

//create DD surface interface ---------------------------------------------------------
IDirectDrawSurface7* MakeDDS7(DWORD width=0,DWORD height=0,DWORD pixform=0,LONG pitch=0,
                              DWORD ddscaps=0,void *extmem=NULL)
{
IDirectDrawSurface7* ddsurface7;
DDSURFACEDESC2 ddsd;
ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
ddsd.dwSize=sizeof(DDSURFACEDESC2);
ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
ddsd.dwWidth=width?width:scrw;
ddsd.dwHeight=height?height:scrh;
if(pitch)
 {
 ddsd.dwFlags|=DDSD_PITCH;
 ddsd.lPitch=pitch;//ALIGN(pitch,3);
 }
if(pixform)
 {
 ddsd.dwFlags|=DDSD_PIXELFORMAT;
 ddsd.ddpfPixelFormat=PFtoDDPF(pixform);
 ddscaps|=DDSCAPS_SYSTEMMEMORY;
 }
if(extmem) //client memory
 {
 ddsd.dwFlags|=DDSD_LPSURFACE;
 ddsd.lpSurface=extmem;
 ddscaps|=DDSCAPS_SYSTEMMEMORY;
 }
ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|ddscaps;
if(ddobj->CreateSurface(&ddsd,&ddsurface7,NULL)==DD_OK)
 return ddsurface7;
else
 return NULL;
}

//ret DD surface characteristics -----------------------------------------------------------
DDSURFACEDESC2 GetDDSDesc(IDirectDrawSurface7* ddsurface7,DWORD *pixform,int *pitch=NULL,
                          int *width=NULL,int *height=NULL,int *bpp=NULL)
{
DDSURFACEDESC2 ddsd;
ddsd.dwSize=sizeof(DDSURFACEDESC2);
ddsurface7->GetSurfaceDesc(&ddsd);
if(pixform) *pixform=DDPFtoPF(ddsd.ddpfPixelFormat);
if(pitch) *pitch=ddsd.lPitch;
if(width) *width=ddsd.dwWidth;
if(height) *height=ddsd.dwHeight;
if(bpp) *bpp=ddsd.ddpfPixelFormat.dwRGBBitCount;
return ddsd;
}

//blits from a DC to a DD surface ----------------------------------------------------------------
BOOL StretchDCtoDDS(IDirectDrawSurface7* ddsurface7,int dx,int dy,int dw,int dh,
                    HDC hdc,int sx=0,int sy=0,int sw=0,int sh=0,DWORD rop=SRCCOPY)
{
if(!ddsurface7||!dw||!dh)
 return 1;
HDC hsdc;
ddsurface7->Restore();
if(ddsurface7->GetDC(&hsdc)!=DD_OK) 
 return 0;
if(!sw) sw=dw;
if(!sh) sh=dh;
StretchBlt(hsdc,dx,dy,dw,dh,hdc,sx,sy,sw,sh,rop);
ddsurface7->ReleaseDC(hsdc);
return 1;
} 

//DD bliting interface ---------------------------------------------------------------------
void DDBlt(IDirectDrawSurface7*ddsdst,int dx=0,int dy=0,int dw=0,int dh=0,
           IDirectDrawSurface7*ddssrc=NULL,int sx=0,int sy=0,int sw=0,int sh=0,
           DWORD rop=SRCCOPY,DWORD style=DDBLT_WAIT,DWORD sckey=0,DWORD ddfx=0)
{
if(!ddsdst) return;
DDBLTFX bfx;
ZeroMemory(&bfx,sizeof(DDBLTFX));
bfx.dwSize=sizeof(DDBLTFX);
if(rop)
 {
 bfx.dwROP=rop;
 style|=DDBLT_ROP;
 }
if(ddfx)
 {
 bfx.dwDDFX=ddfx;
 style|=DDBLT_DDFX;
 }
if(ddssrc)
 {
 if(sckey)
  {
  bfx.ddckSrcColorkey.dwColorSpaceLowValue=sckey;
  bfx.ddckSrcColorkey.dwColorSpaceHighValue=sckey;
  style=style|DDBLT_KEYSRCOVERRIDE;
  }
 ddssrc->Restore();
 }
else
 {
 bfx.dwFillColor=sckey;
 style=style|DDBLT_COLORFILL;
 }
ddsdst->Restore();
erret=ddsdst->Blt((RECT*)&dx,ddssrc,(RECT*)&sx,style,&bfx);
} 

//sets the color key from a COLORREF to a DD surface ------------------------------------
inline void SetCKey(IDirectDrawSurface7* ddsurf,DWORD cref=0)
{
DDCOLORKEY ck;
ck.dwColorSpaceLowValue=cref;
ck.dwColorSpaceHighValue=ck.dwColorSpaceLowValue;
erret=ddsurf->SetColorKey(DDCKEY_SRCBLT,&ck);
}