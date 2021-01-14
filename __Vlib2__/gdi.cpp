#ifndef V_IWINGDI
#define V_IWINGDI

#include <win.cpp>
#include <rgb.cpp>

#define RF_cbpp(rf) (rf&0xff)       //color b/pix
#define RF_abpp(rf) ((rf>>8)&0xff)  //alpha b/pix
#define RF_dbpp(rf) ((rf>>16)&0xff) //depth b/pix
#define RF_sbpp(rf) (rf>>24) //stencil b/pix
#define RF_make(s,d,a,c) (DWORD)((s<<24)|(d<<16)|(a<<8)|c) //render format

#define PointToPixelSz(pts) (NAT)((scrpmh*INCH_M)*pts/72) //transforms point size to pixel size

#define OpenIco(iconame,iconind) ExtractIcon(appinst,ParsePath(iconame),iconind) //must call DestroyIcon()

#define PBMI BITMAPINFO*

//BITMAPINFO with masks for color bitfields ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct BMINF
{
DWORD szof;
LONG  w,h;
WORD  Planes;
WORD  bpp;
DWORD comp;
DWORD szB;
LONG  pmw,pmh;
DWORD ClrUsed;
DWORD ClrImp;
DWORD maskR,maskG,maskB;
};

//BITMAPINFO with masks for color bitfields ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct GREYBMINF
{
DWORD szof;
LONG  w,h;
WORD  Planes;
WORD  bpp;
DWORD comp;
DWORD szB;
LONG  pmw,pmh;
DWORD ClrUsed;
DWORD ClrImp;
RGBQUAD pal[256];
};

//returns current graphics mode characteristics -------------------------------------------------------------
void ModeInfo(int &bpp,int &hres,int &vres)
{
DEVMODE dm;
EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dm);
bpp=dm.dmBitsPerPel;
hres=dm.dmPelsWidth;
vres=dm.dmPelsHeight;
}

//sets display resolution to hresxvresxbpp --------------------------------------------------------
void InitMode(int bpp=0,int hres=0,int vres=0)
{
DEVMODE dm;
ZeroMemory(&dm,sizeof(dm));
dm.dmSize=sizeof(DEVMODE);
if(bpp) 
 {
 dm.dmBitsPerPel=bpp;
 dm.dmFields|=DM_BITSPERPEL;
 }
if(hres)
 {
 dm.dmPelsWidth=hres;
 dm.dmPelsHeight=vres?vres:(hres*3)>>2;
 dm.dmFields|=DM_PELSWIDTH|DM_PELSHEIGHT;
 }
if(ChangeDisplaySettings(&dm,0)==DISP_CHANGE_SUCCESSFUL)
 {
 EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dm);
 scrbpp=dm.dmBitsPerPel;
 scrw=dm.dmPelsWidth;
 scrh=dm.dmPelsHeight;
 }
}

//builds a PF from a DC PFDESCRIPTOR --------------------------------------------------------------
DWORD DCPFDtoPF(PIXELFORMATDESCRIPTOR pfd)
{
BYTE a=0,r=0,g=0,b=0;
if(pfd.cAlphaShift)
 {
 if(pfd.cAlphaShift>pfd.cRedShift) a++;
 else r++;
 if(pfd.cAlphaShift>pfd.cGreenShift) a++;
 else g++;
 if(pfd.cAlphaShift>pfd.cBlueShift) a++;
 else b++;
 a<<=2;
 }
else 
 {
 a=12; //(12=3<<2) alpha position unknown, default to 3 (no alpha channel)
 pfd.cAlphaBits=pfd.cColorBits-pfd.cRedBits-pfd.cGreenBits-pfd.cBlueBits;
 }
if(pfd.cRedShift>pfd.cGreenShift) r++;
else g++;
if(pfd.cRedShift>pfd.cBlueShift) r++;
else b++;
if(pfd.cGreenShift>pfd.cBlueShift) g++;
else b++;
r<<=2; g<<=2; b<<=2;
return (a<<26)|(r<<22)|(g<<18)|(b<<14)|(pfd.cAlphaBits<<a)|(pfd.cRedBits<<r)|(pfd.cGreenBits<<g)|(pfd.cBlueBits<<b);
}

//inits a PIXELFORMATDESCRIPTOR struc ------------------------------------------------------------------
void InitDCPFD(PIXELFORMATDESCRIPTOR* ppfd,DWORD flags=PFD_SUPPORT_GDI,
               DWORD pixform=0x32108888,int dbpp=0,int sbpp=0,int abpp=0)
{
ZeroMemory(ppfd,sizeof(PIXELFORMATDESCRIPTOR));
ppfd->nSize=sizeof(PIXELFORMATDESCRIPTOR);
ppfd->nVersion=1;
ppfd->dwFlags=flags; //|PFD_SWAP_EXCHANGE
ppfd->iPixelType=PFD_TYPE_RGBA;
ppfd->cColorBits=PF_bpix(pixform);
ppfd->cRedBits=PF_bcch(pixform,2);
ppfd->cGreenBits=PF_bcch(pixform,1);
ppfd->cBlueBits=PF_bcch(pixform,0);
ppfd->cAlphaBits=PF_bcch(pixform,3);
ppfd->cRedShift=PF_lsb(pixform,2);
ppfd->cGreenShift=PF_lsb(pixform,1);
ppfd->cBlueShift=PF_lsb(pixform,0);
ppfd->cAlphaShift=PF_lsb(pixform,3);
ppfd->cAccumBits=abpp;
ppfd->cAccumRedBits=ppfd->cAccumGreenBits=ppfd->cAccumBlueBits=ppfd->cAccumAlphaBits=abpp>>2;
ppfd->cDepthBits=dbpp;
ppfd->cStencilBits=sbpp;
ppfd->cAuxBuffers=ppfd->iLayerType=ppfd->bReserved=0;
ppfd->dwVisibleMask=ppfd->dwDamageMask=0;
if(!dbpp) ppfd->dwFlags|=PFD_DEPTH_DONTCARE;
}

//inits a BITMAPINFO structure ------------------------------------------------------------------------
inline void InitBMPINF(BITMAPINFO*bmpinf,int width,int height,NAT bpp=32,DWORD pixform=0)
{
bmpinf->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
bmpinf->bmiHeader.biWidth=width;
bmpinf->bmiHeader.biHeight=height;
bmpinf->bmiHeader.biPlanes=1;
bmpinf->bmiHeader.biBitCount=bpp;
bmpinf->bmiHeader.biSizeImage=(((width*bpp+31)>>3)&~3)*(height<0?-height:height);
bmpinf->bmiHeader.biXPelsPerMeter=scrpmw;
bmpinf->bmiHeader.biYPelsPerMeter=scrpmh;
bmpinf->bmiHeader.biClrUsed=0;
bmpinf->bmiHeader.biClrImportant=0;
if((pixform&0xffff)==0x0565) //must have 3 trailing DWORDS
 {
 bmpinf->bmiHeader.biCompression=BI_BITFIELDS;
 ((DWORD*)bmpinf->bmiColors)[0]=PF_mask(pixform,2);
 ((DWORD*)bmpinf->bmiColors)[1]=PF_mask(pixform,1);
 ((DWORD*)bmpinf->bmiColors)[2]=PF_mask(pixform,0);
 }
else
 bmpinf->bmiHeader.biCompression=BI_RGB;
}

//inits a BITMAPINFO structure for MONO display ------------------------------------------------------------------------
inline void InitBMPINF_BW(BITMAPINFO*bmpinf,int width,int height)
{
bmpinf->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
bmpinf->bmiHeader.biWidth=width;
bmpinf->bmiHeader.biHeight=height;
bmpinf->bmiHeader.biPlanes=1;
bmpinf->bmiHeader.biBitCount=1;
//bmpinf->bmiHeader.biSizeImage=(((width*1+31)>>3)&~3)*(height<0?-height:height);
bmpinf->bmiHeader.biSizeImage=(((width+31)>>3)&~3)*(height<0?-height:height);
bmpinf->bmiHeader.biXPelsPerMeter=scrpmw;
bmpinf->bmiHeader.biYPelsPerMeter=scrpmh;
bmpinf->bmiHeader.biClrUsed=0;
bmpinf->bmiHeader.biClrImportant=0;
bmpinf->bmiHeader.biCompression=BI_RGB;
((DWORD*)bmpinf->bmiColors)[0]=0;
((DWORD*)bmpinf->bmiColors)[1]=0x00ffffff;
}

//Creates a DIB section optionally selected in a DC --------------------------------------
DWORD MakeMemDC(int w,int h,HDC*pmdc,HBITMAP*pdib,DWORD pixform=0,void**ppmem=NULL)
{
HBITMAP hdib;
void*pdibits;
//DWORD masks[2];
BITMAPINFO bi;
pixform=pixform?std_PF(pixform):gdi_PF(scrbpp);
InitBMPINF(&bi,w,-h,PF_bpix(pixform),pixform);
hdib=CreateDIBSection(NULL,&bi,DIB_RGB_COLORS,&pdibits,NULL,0);
if(pdib)
 {
 DeleteObject(*pdib);
 *pdib=hdib;
 }
if(ppmem) *ppmem=pdibits;
if(pmdc)
 {
 if(GetObjectType(*pmdc)!=OBJ_MEMDC) *pmdc=CreateCompatibleDC(NULL);
 hdib=(HBITMAP)SelectObject(*pmdc,hdib);
 if(!hdib) return 0;
 DeleteObject(hdib);
 }
return pixform;
}

//returns a dc's pixel format --------------------------------------------------------
BOOL GetDCInfo(HDC hdc,DWORD *pixform=NULL,int *bpp=NULL,int *dbpp=NULL,int *sbpp=NULL,int *abpp=NULL,int *hres=NULL,int *vres=NULL)
{         
PIXELFORMATDESCRIPTOR pfd;
if(hres) *hres=GetDeviceCaps(hdc,HORZRES);
if(vres) *vres=GetDeviceCaps(hdc,VERTRES);
if(bpp) *bpp=GetDeviceCaps(hdc,BITSPIXEL);
ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
if(!DescribePixelFormat(hdc,GetPixelFormat(hdc),sizeof(PIXELFORMATDESCRIPTOR),&pfd))
 return 0;
if(pfd.iPixelType!=PFD_TYPE_RGBA)
 return 0;
if(pixform) *pixform=DCPFDtoPF(pfd);
if(bpp) *bpp=pfd.cColorBits;
if(dbpp) *dbpp=pfd.cDepthBits;
if(sbpp) *sbpp=pfd.cStencilBits;
if(abpp) *abpp=pfd.cAccumBits;
return 1; //0-indexed, 1=RGBA
}

//loads a packed bitmap (BITMAPINFO+pal+bmpbits) (should free) ----------------------------------
BITMAPINFO*LoadDIB(LPSTR filename,NAT &szb)
{
FILE*Fdib;
BITMAPINFO*packdib;
Fdib=fopen(filename,"rb");
if(!Fdib) return NULL;
fseek(Fdib,0,SEEK_END);
szb=ftell(Fdib)-sizeof(BITMAPFILEHEADER);
packdib=(BITMAPINFO*)malloc(szb+4);
fseek(Fdib,sizeof(BITMAPFILEHEADER),SEEK_SET);
fread(packdib,szb,1,Fdib);
fclose(Fdib);
return packdib;
}

//draws a bitmap (leave pdata to NULL if its a packed DIB) --------------------------------------
inline void DrawDIB(HDC hldc,int l,int u,int r,int d,BITMAPINFO*pdib,char*pdata=NULL)
{
if(!pdib)
 {
 BitBlt(hldc,l,u,r,d,NULL,0,0,BLACKNESS);
 return;
 }
if(!pdata) //packed DIB
 {
 pdata=(char*)pdib;
 pdata+=sizeof(BITMAPINFOHEADER);
 if(pdib->bmiHeader.biBitCount<=8) pdata+=(1<<pdib->bmiHeader.biBitCount)<<2;
 }
StretchDIBits(hldc,l,u,r,d,0,0,pdib->bmiHeader.biWidth,pdib->bmiHeader.biHeight,pdata,pdib,DIB_RGB_COLORS,SRCCOPY);
}   

//creates a dc and loads a bitmap into it -------------------------------------------------
HDC OpenDC(LPSTR bmpname,BITMAP &bmp,DWORD style=0,HBITMAP*phbmp=NULL)//must use DeleteDC() 
{
HDC hdcbmp;
HBITMAP hbmp;
hbmp=(HBITMAP)LoadImage(NULL,bmpname,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|style);
if(hbmp==NULL) return NULL;
else if(phbmp) *phbmp=hbmp;
hdcbmp=CreateCompatibleDC(NULL);
if(!hdcbmp) return NULL;
GetObject(hbmp,sizeof(bmp),&bmp);
SelectObject(hdcbmp,hbmp);
return hdcbmp;
}

//draws a rect that has proc% of its area filled ---------------------------------------------------
void ProcentRect(HDC hdc,RECT*lr,float proc=100.0f,HBRUSH hbrs=NULL)
{
fpuioi=lr->right;
FrameRect(hdc,lr,hbrs);
lr->right=lr->left+(lr->right-lr->left)*proc/100;
FillRect(hdc,lr,hbrs);
lr->right=fpuioi;
}

//draws an ellipse that has proc% of its area filled ------------------------------------------------
void ProcentPie(HDC hdc,int x,int y,int hr,int vr,double proc=100.0f)
{
proc*=_2PI/100;
Pie(hdc,x-hr,y-vr,x+hr,y+vr+1,x+hr+1,y,x+hr*cos(proc),y-vr*sin(proc));
}

//draws a grid in a rectangle ------------------------------------------------------------------------
void DrawGrid(HDC hdc,int l,int u,int r,int d,NAT gridw=1,NAT gridh=1)
{
SetROP2(hdc,R2_XORPEN);
l=CutU(l,gridw);
u=CutU(u,gridh);
r=CutD(r,gridw);
d=CutD(d,gridh);
for(int y=u;y<d;y+=gridh)
 for(int x=l;x<r;x+=gridw)
  SetPixelV(hdc,x,y,0xffffff);
SetROP2(hdc,R2_COPYPEN);
}

//simulates cprintf --------------------------------------------------------------------------------
void TextAt(HWND hwnd,int &x,int &y,char *formstr,...)
{
va_list vparam;
va_start(vparam,formstr);
vsprintf(strbuf,formstr,vparam);
va_end(vparam);
SIZE tsz;
RECT rct;
NAT strl=strlen(strbuf);
HDC hdc=GetDC(hwnd);
GetTextExtentPoint32(hdc,strbuf,strl,&tsz);
GetClientRect(hwnd,&rct);
if(x+tsz.cx>rct.right)
 {
 x=0;
 y+=tsz.cy;
 if(y>rct.bottom) y=0;
 }
TextOut(hdc,x,y,strbuf,strl);
x+=tsz.cx;
ReleaseDC(hwnd,hdc);
}

//converts a WMF to EMF -------------------------------------------------------------------------------
HENHMETAFILE WMFtoEMF(HMETAFILE hwmf)
{
HENHMETAFILE hemf;
METAFILEPICT mfp;
DWORD bufsz;
BYTE*locbuf;
bufsz=GetMetaFileBitsEx(hwmf,0,NULL);
locbuf=(BYTE*)malloc(bufsz+4);
GetMetaFileBitsEx(hwmf,bufsz,locbuf);
mfp.mm=MM_ISOTROPIC;
mfp.xExt=0;
mfp.yExt=0;
mfp.hMF=NULL;
hemf=SetWinMetaFileBits(bufsz,locbuf,NULL,&mfp);
return hemf;
}

//interface for ChooseFont dialog (CF_SELECTSCRIPT)------------------------------------------------------
HFONT FontDlg(HFONT hofon=NULL,COLORREF*pfc=NULL,HWND hwp=NULL,DWORD flags=CF_EFFECTS)
{
LOGFONT lf;
CHOOSEFONT cf;
if(hofon)
 {
 GetObject(hofon,sizeof(LOGFONT),&lf);
 flags|=CF_INITTOLOGFONTSTRUCT;
 }
ZeroMemory(&cf,sizeof(CHOOSEFONT));
cf.lStructSize=sizeof(CHOOSEFONT);
cf.hwndOwner=hwp;
cf.lpLogFont=&lf;
cf.Flags=CF_SCREENFONTS|flags;
cf.rgbColors=pfc?*pfc:GetSysColor(COLOR_WINDOWTEXT);
if(!ChooseFont(&cf)) return hofon;
if(pfc) *pfc=cf.rgbColors;
DeleteObject(hofon);
return CreateFontIndirect(&lf);
}

//GDI DC interface ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class WINDC
{
public:
 HDC dc;
 HWND wnd;
 int ap,ab,af,nrp,nrb,nrf; //active pen and brush number of pens and brushes
 HPEN *pen;
 HBRUSH *fill;
 HFONT *font;
 HBITMAP bmp; //mem dc only
 TEXTMETRIC tm;
 COLORREF tc,bc; //text, back color
 DWORD ta,bm,mm,rop; //text align, back mode, map mode, raster op
 int w,h; //width and height in pixels of the whole device (origin is always 0,0)
 int l,u,r,d; //clip rect usualy the whole viewable/printable area in pixels
 int xres,yres; //device width and height always in device units
 int pmw,pmh; //native device resolution in pixels per meter
 int apmw,apmh; //simulated resolution with Res (mainly for printing) 
 int type; //all of the techs from GetDeviceCaps
 int links;
//WinDC <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
/*
  By calling Res() you change the resolution of the DC from device units to some
other unit. The parameters to Res() represent the new resolution that the DC
should appear to have. After this l,u,r,d,w,h are in the new resolution units,
not in pixels (the default). (xres,yres) are always in pixels.
  For displays:
> (0,0)-(w-1,h-1)   is the screen rect, (w x h is the screen size)
> (l,u)-(r,d)       is the window client rect (r/d - window width/height).
> (0,0)-(xres,yres) is the window client rect in device units.
  For printers:
> (0,0)-(w-1,h-1) is the physical paper rect 
> (l,u)-(r,d) is the printable area of the paper wich is smaller than the paper.
> (xres,yres) is the printable area in device units.
*/
//.....................................................................................................
WINDC()
 {
 type=-1;
 pen=(HPEN*)malloc(sizeof(HPEN));
 fill=(HBRUSH*)malloc(sizeof(HBRUSH));
 font=(HFONT*)malloc(sizeof(HFONT));
 nrp=nrb=nrf=1;
 pen[0]=CreatePen(PS_NULL,0,0);
 fill[0]=(HBRUSH)GetStockObject(HOLLOW_BRUSH);
 font[0]=(HFONT)GetStockObject(SYSTEM_FONT);
 ap=ab=af=links=0;
 bmp=NULL;
 tc=0xffffff;
 bc=0;
 ta=TA_LEFT|TA_TOP;
 bm=TRANSPARENT;
 rop=R2_COPYPEN;
 mm=MM_TEXT;
 apmw=pmw=scrpmw;
 apmh=pmh=scrpmh;
 l=u=r=d=w=h=0;
 }
//................................................................................................
void Sync()
 {
 SetStretchBltMode(dc,COLORONCOLOR);
 SelectObject(dc,pen[ap]);
 SelectObject(dc,fill[ab]);
 SelectObject(dc,font[af]);
 SetTextColor(dc,tc);
 SetTextAlign(dc,ta);
 SetBkColor(dc,bc);
 SetBkMode(dc,bm);
 SetROP2(dc,rop);
 GetTextMetrics(dc,&tm);
 SetMapMode(dc,mm);
 if(mm==MM_ANISOTROPIC)
  {
  //SetViewportExtEx(dc,GetDeviceCaps(dc,HORZRES),GetDeviceCaps(dc,VERTRES),NULL);
  SetViewportExtEx(dc,xres,yres,NULL);
  SetWindowOrgEx(dc,l,u,NULL);
  //SetWindowExtEx(dc,GetDeviceCaps(dc,HORZRES)*apmw/pmw,GetDeviceCaps(dc,VERTRES)*apmh/pmh,NULL);
  SetWindowExtEx(dc,r-l,d-u,NULL);
  }
 }
//................................................................................................
void Res(int spmw=0,int spmh=0)										  
 {
 xres=GetDeviceCaps(dc,HORZRES);
 yres=GetDeviceCaps(dc,VERTRES);
 pmw=xres*1000/GetDeviceCaps(dc,HORZSIZE);
 pmh=yres*1000/GetDeviceCaps(dc,VERTSIZE);
 apmw=spmw?spmw:pmw;
 apmh=spmh?spmh:pmh;
 if(type==DT_RASPRINTER) //printer
  {
  w=GetDeviceCaps(dc,PHYSICALWIDTH)*apmw/pmw;
  h=GetDeviceCaps(dc,PHYSICALHEIGHT)*apmh/pmh;
  l=GetDeviceCaps(dc,PHYSICALOFFSETX)*apmw/pmw;
  u=GetDeviceCaps(dc,PHYSICALOFFSETY)*apmh/pmh;
  r=l+xres*apmw/pmw; //-apmw/pmw
  d=u+yres*apmh/pmh; //-apmh/pmh
  }
 else if(type==DT_RASDISPLAY) //display
  {
  w=xres*apmw/pmw;
  h=yres*apmh/pmh;
  GetClientRect(wnd,(RECT*)&l);
  xres=r;
  yres=d;
  r=r*apmw/pmw;
  d=d*apmh/pmh;
  }
 mm=MM_ANISOTROPIC;
 Sync();
 }
//...................................................................................................
void Get(HWND hnwnd=NULL)
 {
 links++;
 if(links>1) return;
 if(hnwnd) wnd=hnwnd;
 dc=GetDC(wnd);
 SaveDC(dc);
 type=GetDeviceCaps(dc,TECHNOLOGY);
 //Res(); //default to native resolution
 Sync();
 }
//...................................................................................................
BOOL Prn(int ui=1)
 {
 int ok;
 links++;
 if(links>1) return 1;
 if(ui==1)
  ok=GetPageInfo(&dc);
 else if(ui==2)
  ok=(dc=GetPrintInfo(NULL,NULL))!=NULL;
 else
  {
  ok=(dc=GetPrintInfo(NULL,NULL,PD_RETURNDEFAULT))!=NULL;
  }
 if(ok)
  {
  type=GetDeviceCaps(dc,TECHNOLOGY);
  Res(scrpmw,scrpmh); //default to display resolution
  }
 else
  { 
  isERROR("Prn");
  dc=NULL;
  links=0;
  }
 return ok;
 } 
//..................................................................................................
void Pag()
 {
 StartPage(dc);
 Sync();
 }
//......................................................................................................
void End()
 {
 EndPage(dc);
 }
//..................................................................................................
void Rel()
 {
 links--;
 if(links) return;
 if(type==DT_RASDISPLAY) //1
  {
  RestoreDC(dc,-1);
  ReleaseDC(wnd,dc);
  }
 else if(type==DT_RASPRINTER) //2
  {
  EndDoc(dc);
  DeleteDC(dc);
  }
 dc=NULL;
 }
//....................................................................................................
void MakePen(COLORREF pc=0xffffff,int gros=0,int pi=0)
 {
 if(!pi||pi>=nrp)
  {
  pi=nrp;
  nrp++;
  pen=(HPEN*)realloc(pen,nrp*sizeof(HPEN));
  }
 else
  DeleteObject(pen[pi]);
 pen[pi]=CreatePen(PS_SOLID,gros,pc);
 ap=pi;
 if(links) SelectObject(dc,pen[ap]);
 }
//.......................................................................................................
void UsePen(int nap=0,DWORD nrop=0)
 {
 if(nap<nrp) ap=nap;
 if(nrop) rop=nrop;
 if(links)
  {
  SelectObject(dc,pen[ap]);
  SetROP2(dc,rop);
  }
 }
//.......................................................................................................
void MakeBrush(COLORREF sc=0,int bi=0)
 {
 if(!bi||bi>=nrb)
  {
  bi=nrb;
  nrb++;
  fill=(HBRUSH*)realloc(fill,nrb*sizeof(HBRUSH));
  }
 else
  DeleteObject(fill[bi]);
 fill[bi]=CreateSolidBrush(sc);
 ab=bi;
 if(links) SelectObject(dc,fill[ab]);
 }
//...................................................................................................
void MakeHatch(COLORREF fc=0,int styl=HS_DIAGCROSS,int bi=0)
 {
 if(!bi||bi>=nrb)
  {
  bi=nrb;
  nrb++;
  fill=(HBRUSH*)realloc(fill,nrb*sizeof(HBRUSH));
  }
 else
  DeleteObject(fill[bi]);
 fill[bi]=CreateHatchBrush(styl,fc);
 ab=bi;
 if(links) SelectObject(dc,fill[ab]);
 }
//.......................................................................................................
void UseFill(int nab=0,DWORD bkmod=0,COLORREF bkcol=0xff000000)
 {
 if(nab<nrb) ab=nab;
 if(bkmod) bm=bkmod;  //1-transp, 2-opaque
 if(!(bkcol&0xff000000)) bc=bkcol;
 if(links)
  {
  SetBkMode(dc,bm);
  SetBkColor(dc,bc);
  SelectObject(dc,fill[ab]);
  }
 }
//.....................................................................................................
void MakeFont(LPSTR name=NULL,int cellh=16,int atrib=0,DWORD chset=0,int fi=0)
 {
 if(!fi||fi>=nrf)
  {
  fi=nrf;
  nrf++;
  font=(HFONT*)realloc(font,nrf*sizeof(HFONT));
  }
 else
  DeleteObject(font[fi]);
 font[fi]=CreateFont(cellh,0,0,0,(atrib&0xf)*100,(atrib>>4)&1,(atrib>>8)&1,(atrib>>12)&1,
                     chset,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,
                     FF_DONTCARE|(atrib>>16?FIXED_PITCH:DEFAULT_PITCH),name);
 af=fi;
 if(links)
  {
  SelectObject(dc,font[fi]);
  GetTextMetrics(dc,&tm);
  }
 }
//.....................................................................................................
void UseFont(int naf=0,COLORREF fcol=0xff000000,DWORD nta=-1)
 {
 if(naf<nrf) af=naf;
 if(!(fcol&0xff000000)) tc=fcol;
 if(nta!=-1) ta=nta;
 if(links)
  {
  SetTextAlign(dc,ta);
  SetTextColor(dc,tc);
  SelectObject(dc,font[af]);
  }
 }
//..................................................................................................
~WINDC()
 {
 if(links) links=1; //force delete
 Rel();
 if(pen)
  {
  while(nrp>0)
   DeleteObject(pen[--nrp]);
  free(pen);
  pen=NULL;
  }
 if(fill)
  {
  while(nrb>1)
   DeleteObject(fill[--nrb]);
  free(fill);
  fill=NULL;
  }
 if(font)
  {
  while(nrf>1)
   DeleteObject(font[--nrf]);
  free(font);
  font=NULL;
  }
 if(bmp)
  DeleteObject(bmp);
 }
//....................................................................................................
};
// WinDC >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#endif