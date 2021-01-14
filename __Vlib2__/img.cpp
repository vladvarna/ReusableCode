#pragma once

#ifndef V_IMAGE
#define V_IMAGE
#endif

#include <gdi.cpp>
#include <bmp.cpp>
#include <tga.cpp>
#include <pcx.cpp>

#define IMG_ALIGN   0xf //align lng (and implicitly Bpl) to (stat&0xf)+1
#define IMG_EMB     0x10 //img is an externally managed memory buffer
#define IMG_IMB     0x20 //img is an internally managed memory buffer
#define IMG_IDC     0x40 //img is a DIB section selected in mdc
#define IMG_IDD     0x80 //img is a locked rect of dds
#define IMG_DC      0x100 //has a cached memory DC
#define IMG_DD      0x200 //has a cached DD surface
//#define IMG_DG      0x400 //has a cached D3D surface
//#define IMG_GL      0x800 //has a cached GL texture
#define IMG_EDIT    0x1000 //auto keep edit mode (32bpp)
#define IMG_GDI     0x2000 //auto keep GDI compatible
#define IMG_GREY    0x4000 //one channel
#define IMG_REAL	0x8000 //float stencil

#define FILTER_ZOOM      1
#define FILTER_TEXTURE   2
#define FILTER_REDUCE    3
#define FILTER_MIP       4

class Image
{
public:
 union
  {
  void *img;
  signed char*imgS;
  BYTE *imgB;
  DWORD*imgDW;
  COLOR*imgC;
  float*imgF;
  };
 int Bpl,Bpp; //B/line,B/pixel
 int lng,lat; //(STD) width (reccomended alignment to Q) and height
 DWORD pf; //(STD) pixel format tag (0x32108888)
 int bpp; // bits/pixel
 int nrp,szB; //nr of pixels, image sz in B (<= allocated amount)
 int pmw,pmh; //(STD) resolution in pixels per meter
 DWORD stat; //(STD) status flags
 char file[PATHSZ];

 COLOR*cp; //current position
 int cx,cy; //current position in (x,y)
 DWORD maskA,maskO,maskX; //AND,OR,XOR masks (currently using just the AND mask)
 DWORD color; //current drawing color

 HDC mdc; //memory device context
 HBITMAP hbm; //handler to bitmap hbmp
 HFONT hfont;
 TEXTMETRIC tm;
 
 //general management
 Image() { ZEROCLASS(Image); }
 ~Image() { Free(stat); }
 void Init(int,int,DWORD,DWORD,void*,int,int,char*);
 void Free(int);
 int From(LPSTR,LPSTR,DWORD,int,DWORD);
 int To(LPSTR,LPSTR,DWORD,int);
#ifndef V_NOGDIPLUS
 int ToCImage(CImage*);
#endif
 void Conv(DWORD pixform=0x32108888,int neww=0,int newh=0);
 void Clone(Image*,int,int,int,int);
 void Align(int);
 void Clear(DWORD);
 void HFlip();
 void VFlip();
 //editing tools (work only for 32bpp)
 DWORD Color(int,int); //returns a certain color from image
 void Fill(int way=0,DWORD c1=0,DWORD c2=0xffffffff);
 void Alpha(int way=0,BYTE a0=0xff,DWORD c1=0,DWORD c2=0xffffffff);
 void Mod(int,double,int);   //applies filters or convertors
 void GreyTf(BYTE*,int); //apply greyscale transform
 void Convolute1D(double*,int);
 void Convolute2D(double*,int,int);
 NAT ChHist(NAT*,int); //channel histogram
 NAT ColorHist(NAT*); //true color histogram
 NAT CountColors();
 void EqualHist(int); //equalize histogram
 NAT Threshold(int,int); //divide histogram
 void HistSeg(int,NAT*,NAT,int);
 float HFore(int,BYTE,int);
 float VFore(int,BYTE,int);
 NAT HCut(VECT2*,NAT,VECT2,float,int);
 NAT VCut(VECT2*,NAT,float,int);
 void CropFore(VECT2,float,int);
 void OCRFilter(VECT2,VECT2);
 void Resize(int neww,int newh,int filter=0);
 void Crop(int l,int u,int r,int d,DWORD bc=0);
 void bop(DWORD,DWORD,DWORD); //binary operation
 //cp relative functions
 void at(int,int);
 //void put(BYTE,BYTE,BYTE,BYTE);
 //void get(BYTE*,BYTE*,BYTE*,BYTE*);
 void lin(int,int);
 void linh(int);
 void linv(int);
 //void lind(DWORD,int);//diagonal line
 void rect(int,int);
 void rectf(int,int);
 //self suficient versions
 void pix(int,int);
 void line(int,int,int,int);
 void spline2(int,int,int,int,int,int);
 void spline3(int,int,int,int,int,int,int,int);
 void dtrace(short*,int,int); //draw trace
 void dwtrace(short*,int,int,float,float); //draw widened trace
 void dstrace(short*,int,int,float,float,float); //draw scaled trace
 void chcolor(DWORD,DWORD);
 void bar(int,int,int,int);
 void chcolinbar(int,int,int,int,DWORD,DWORD);
 //One channel functions
 void CloneCh(Image*,int,int,int,int,int);
 void DerivH(Image*,int);
 void DerivV(Image*,int);
 void Sobel(int**,float**);
 void Deriv(int);
 //void Canny(int**,float**,int tH=96,int tL=32);
 //Win GUI specific functions
 void Font(LPSTR,int,int,DWORD);
 HDC DCTex();
 void ShowBW(HDC hdc,int,int,int,int,RCT*);
 void DrawDC(HDC hdc,int,int,int,int,int,int,int,HWND);
 void ShowDC(HDC hdc,int,int,int,int,RCT*);
 void HFont(HFONT,char*); //init from font and char
 void FitWindow(HWND,int,int);
 //extensions
#ifdef V_IDIRECTDRAW
 IDirectDrawSurface7*dds;

 IDirectDrawSurface7* DDTex();
 void DrawDD(IDirectDrawSurface7*,int,int,int,int,int,int);
 void ShowDD(IDirectDrawSurface7*,int,int,int,int,int);
#endif
#ifdef V_IDIRECTXG8
 IDirect3DTexture8* DXTex(IDirect3DTexture8*,DWORD,int);
#endif
#ifdef V_IOPENGL
 GLTEX GLTex(GLTEX,DWORD,int);
#endif
};
//*************************************************************************************

//releases image resources; internally use Free(stat).............................................................
void Image::Free(int nstat=0)
{
if(!nstat) nstat=stat;
if(nstat&IMG_EMB)
 {
 img=NULL;
 stat&=~IMG_EMB;
 }
if(nstat&IMG_IMB)
 {
 if(img) free(img);
 img=NULL;
 stat&=~IMG_IMB;
 }
if(nstat&IMG_DC)
 {
 if(stat&IMG_IDC) img=NULL;
 if(hfont) DeleteObject(hfont);
 hfont=NULL;
 DeleteDC(mdc);
 mdc=NULL;
 DeleteObject(hbm);
 hbm=NULL;
 stat&=~(IMG_DC|IMG_IDC);
 }
#ifdef V_IDIRECTDRAW
if(nstat&IMG_DD)
 {
 DELCOM(dds);
 if(nstat&IMG_IDD) FREE(img);
 stat&=~(IMG_DD|IMG_IDD);
 }
#endif
}

//allocates resources for image and updates internal format data ..........................
void Image::Init(int width,int height,DWORD pixform=0x32108888,DWORD nstat=0,
                 void*extmem=NULL,int xres=0,int yres=0,char*origfile=NULL)
{
if(width) lng=width;
if(height) lat=height;
if(pixform) pf=pixform;
if(xres) pmw=xres;
if(yres) pmh=yres;
if(pmw<500||pmw>100000) pmw=scrpmw; //pixeli nu pot fi mai mari de 5mm
if(pmh<500||pmh>100000) pmh=scrpmh; //sau mai mici de 0.01mm
if(origfile) sc(file,origfile);
if(nstat&IMG_GREY)
 {
 if(PF_bpix(pf)!=8) pf=0x32100008;
 nstat|=0x3; //align to DWORD
 stat|=IMG_GREY;
 }
else if(nstat&IMG_REAL)
 {
 if(PF_bpix(pf)!=32) pf=0x32108888;
 stat|=IMG_REAL;
 }
else if(nstat&IMG_EDIT)
 {
 pf=(pf&0xffff0000)|0x8888;
 stat|=IMG_EDIT;
 }
else if(nstat&IMG_GDI||nstat&IMG_IDC)
 {
 pf=std_PF(pf);
 nstat|=0x3; //align to DWORD
 stat|=IMG_GDI;
 }
else if(nstat&IMG_IDD)
 {
 nstat|=0x3; //align to DWORD
 }
bpp=PF_bpix(pf);
Bpp=ALIGN(bpp,7)>>3;
Bpl=ALIGN(lng*Bpp,nstat&0xf);
nrp=lng*lat;
szB=Bpl*lat;
Free(stat);
if(nstat&IMG_EMB&&extmem)
 {
 img=extmem;
 stat|=IMG_EMB;
 }
else if(nstat&IMG_IDC) //needs IMG_GDI
 {
 if(stat&IMG_IDC) img=NULL;
 pf=MakeMemDC(lng,lat+1,&mdc,&hbm,pf,&img);
 if(extmem) CopyMemory(img,extmem,szB);
 stat|=IMG_DC|IMG_IDC;
 }
#ifdef V_IDIRECTDRAW
else if(nstat&IMG_IDD) //needs to be DWORD aligned
 {
 img=realloc(extmem,szB+4);
 dds=MakeDDS7(lng,lat+1,pf,Bpl,0,imgB);
 stat&=~(IMG_DD|IMG_IDD);
 }
#endif
else// if(nstat&IMG_IMB)
 {
 img=realloc(extmem,szB+4);
 stat|=IMG_IMB;
 }
stat|=nstat&0xf; //keeps bigest mask
}

//........................................................................
inline void Image::Clone(Image*pimg,int l,int u,int r,int d)
{
Init(pimg->lng,pimg->lat,pimg->pf,pimg->stat,NULL,pimg->pmw,pimg->pmh,pimg->file);
CopyMemory(img,pimg->img,szB);
if(l>=0&&u>=0&&r>0&&d>0)
 Crop(l,u,r,d);
}

//loads an image from disk ................................................................
int Image::From(LPSTR filename=NULL,LPSTR filetype=NULL,DWORD pixform=0,int frmnum=0,DWORD nstat=0)
{
int tip;
if(!filename) //file open dialog
 {
 GetFileName(strbuf,OFN_PATHMUSTEXIST,hmwnd,"Images\0*.bmp;*.tga;*.pcx;*.gif;*.png;*.jpg;*.tif;*.ico;*.emf;*.wmf\0All\0*.*\0","Load image from file");
 if(!*strbuf) return 6; //not a valid operation
 filename=strbuf;
 }
ifn(filetype) filetype=filename+lastch('.',filename);
if(LOcase(filetype,PATHSZ)<3) return 5; //unknown file format
BMPimage bmpimg;
if(scmp(filetype,"bmp")) //Bitmap
 {
 if(tip=bmpimg.Open(filename)) return tip;//file open error
 }
else if(scmp(filetype,"tga")) //TGA
 {
 TGAimage tgaimg;
 if(tip=tgaimg.Open(filename)) return tip;//file open error
 if(!pixform) pixform=tgaimg.pf; //load in file format
 Init(tgaimg.lng,tgaimg.lat,pixform,nstat,NULL,scrpmw,scrpmh,filename);
 tgaimg.GetF(imgB,lat,Bpl,pf);
 if(PF_bcch(pf,3)&&!PF_bcch(tgaimg.pf,3))
  ORmaskU(img,PF_mask(pf,3),Bpp,nrp); //set to solid alpha if no alpha in file
 return 0; //Ok
 }
else if(scmp(filetype,"pcx")) //PCX
 {
 PCXimage pcximg;
 if(tip=pcximg.Open(filename)) return tip;//file open error
 if(!pixform) pixform=0x32100888; //load in PCX standard format
 Init(pcximg.ppl,pcximg.lat,pixform,nstat,NULL,pcximg.pmw,pcximg.pmh,filename);
 pcximg.GetF(imgB,lat,Bpl,pf);
 if(PF_bcch(pf,3))
  ORmaskU(img,PF_mask(pf,3),Bpp,nrp); //set to solid alpha
 return 0; //Ok
 }
else if(scmp(filetype,"paste")||scmp(filetype,"clipboard")) //paste clipboard
 {
 filename=NULL;
 if(tip=bmpimg.Paste()) return tip; //no data
 }
else if(scmp(filetype,"twain")) //TWAIN
 {
 filename=NULL;
 InitTWAIN();
 TWSource();
 tip=bmpimg.Acquire();
 FreeTWAIN();
 if(tip) return tip;
 }
else if(tip=s4inlist(filetype,"ico\0cur\0ani\0dll\0exe\0",5))
 {
 HICON hlico;
 hlico=ExtractIcon(appinst,filename,frmnum);
 if(!hlico) return 1;
 tip=bmpimg.FromICO(hlico);
 DestroyIcon(hlico);
 if(tip) return tip;
 }
#ifndef V_NOGDIPLUS
else if(tip=s4inlist(filetype,"jpegjpg\0png\0gif\0tif\0emf\0wmf\0bmp\0ico\0",9)) //ATL load
 {
 CImage ci;
 erret=ci.Load(filename);
 if(HRESULT_SEVERITY(erret)) return 2; //couldn't load file
 bmpimg.FromCImage(&ci);
 ci.Destroy();
 if(erret) return 3;
 }
#else
else if(tip=s4inlist(filetype,"jpegjpg\0gif\0bmp\0ico\0emf\0wmf\0",7))   //OLE load
 {
 if(tip=bmpimg.Load(filename)) return tip;
 }
#endif
else
 return 4; //unknown file format
if(!pixform) pixform=bmpimg.pf; //use closest to BMP original format
Init(bmpimg.ppl,bmpimg.lat,pixform,nstat,NULL,bmpimg.pmw,bmpimg.pmh,filename);
bmpimg.GetF(imgB,lat,Bpl,pf);
if(PF_bcch(pf,3))
 ORmaskU(img,PF_mask(pf,3),Bpp,nrp); //set to solid alpha
return 0;//Ok
}

#ifndef V_NOGDIPLUS
//..............................................................................................
int Image::ToCImage(CImage*pci)
{
DWORD BitFields,BitMasks[3];
HDC hldc;
if(!img) return 1; //no data
if(stat&IMG_GREY)
 {
 GREYBMINF gi;
 InitBMPINF((PBMI)&gi,lng,-lat,8,pf);
 GreyPal((COLOR*)gi.pal,256,0xff);
 pci->Create(lng,-lat,8,0);
 pci->SetColorTable(0,256,(RGBQUAD*)gi.pal);
 hldc=pci->GetDC();
 erret=SetDIBitsToDevice(hldc,0,0,lng,lat,0,0,0,lat,img,(PBMI)&gi,DIB_RGB_COLORS);
 pci->ReleaseDC();
 }
else
 {
 BMINF bi;
 switch(pf&0xffff) //must have 3 trailing DWORDS
  {
  case 0x8888:  //32
  case 0x0888:  //24
  case 0x1555:  //16
   BitFields=BI_RGB;
   break;
  default:
   BitFields=BI_BITFIELDS;
   BitMasks[0]=PF_mask(pf,2);
   BitMasks[1]=PF_mask(pf,1);
   BitMasks[2]=PF_mask(pf,0);
  } 
 InitBMPINF((PBMI)&bi,lng,-lat,ALIGN(bpp,7),pf);
 pci->CreateEx(lng,lat,Bpp<<3,BitFields,BitMasks,0);
 hldc=pci->GetDC();
 SetDIBitsToDevice(hldc,0,0,lng,lat,0,0,0,lat,img,(PBMI)&bi,DIB_RGB_COLORS);
 pci->ReleaseDC();
 }
return 0; //ok
}
#endif 

//saves an image to disk ...................................................................
int Image::To(LPSTR filename=NULL,LPSTR filetype=NULL,DWORD pixform=0,int frmnum=0)
{
if(!img) return 3; //no data to write
if(!filename) //file open dialog
 {
#ifndef V_NOGDIPLUS
 GetFileName(strbuf,OFN_OVERWRITEPROMPT,hmwnd,"BMP\0*.bmp\0TGA\0*.tga\0JPEG\0*.jpg\0PNG\0*.png\0GIF\0*.gif\0TIFF\0*.tif\0",
             "Save image to file","auto",NULL,NULL,NULL,'S',NULL);
#else
 GetFileName(strbuf,OFN_OVERWRITEPROMPT,hmwnd,"BMP\0*.bmp\0TGA\0*.tga\0",
             "Save image to file","auto",NULL,NULL,NULL,'S',NULL);
#endif
 if(!*strbuf) return 6; //not a valid operation
 filename=strbuf;
 }
if(!filetype) filetype=filename+lastch('.',filename);
if(LOcase(filetype,PATHSZ)<3) return 5; //unknown file format
if(!pixform) pixform=pf; //save in internal format
if(scmp(filetype,"bmp")) //Bitmap
 {
 BMPimage bmpimg;
 pixform=bmpimg.CanSave(pixform,0);
 bmpimg.Set(lng,lat,pixform);
 bmpimg.PutF(imgB,lat,Bpl,pf);
 if(bmpimg.Save(filename)) return 1;//file access error
 }
else if(scmp(filetype,"tga")) //Targa
 {
 TGAimage tgaimg;
 pixform=tgaimg.CanSave(pixform,0);
 tgaimg.Set(lng,lat,pixform);
 if(tgaimg.Save(filename)) return 1;//file access error
 tgaimg.PutF(imgB,lat,Bpl,pf);
 }
else if(scmp(filetype,"copy")||scmp(filetype,"clipboard")) //copy
 {
 filename=NULL;
 BMPimage bmpimg;
 pixform=bmpimg.CanSave(pixform,0);
 bmpimg.Set(lng,lat,pixform);
 bmpimg.PutF(imgB,lat,Bpl,pf);
 if(bmpimg.Clipboard()) return 1; //error
 }
else if(scmp(filetype,"print")||scmp(filetype,"paper")) //print
 {
 WINDC prn;
 prn.Prn();
 prn.Pag();
 prn.MakePen(0xff);
 Rectangle(prn.dc,prn.l,prn.u,prn.r,prn.d);
 prn.MakeFont("Times New Roman",-28,0x105);
 prn.UseFont(1,0x0);
 PathNameExt(file,NULL,&filename,NULL);
 TextOut(prn.dc,prn.l,prn.u,filename,firstch('.',filename,PATHSZ,'\0'));
 ShowDC(prn.dc,(prn.r+prn.l-lng)>>1,(prn.d+prn.u-lat)>>1,(prn.r+prn.l+lng)>>1,(prn.d+prn.u+lat)>>1,NULL);
 prn.End();
 prn.Rel();
 filename=NULL;
 }
#ifndef V_NOGDIPLUS
else if(s4inlist(filetype,"jpg\0png\0gif\0tif\0bmp\0",5)) //ATL save
 {
 CImage ci;
 ToCImage(&ci);
 erret=ci.Save(filename);
 ci.Destroy();
 if(R_ERR) return 7; //CImage failed
 }
#endif
else
 return 5; //don't know file format
if(filename) sc(file,filename);
return 0;//Ok
}

//changes an image format ..............................................................
void Image::Conv(DWORD pixform,int neww,int newh)
{
if(!img) return;
Image tmpimg;
if(!pixform) pixform=pf;
if(!neww) neww=lng;
if(!newh) newh=lat;
tmpimg.Init(neww,newh,pf,stat,NULL,pmw,pmh,file); //create the new one
CopyImg(tmpimg.img,tmpimg.Bpl,tmpimg.lng,tmpimg.lat,tmpimg.pf,img,Bpl,lng,lat,pf);
Free(stat);
CopyMemory(this,&tmpimg,sizeof(Image)); //this=tmpimg
tmpimg.stat=0; //prevent freeing in destructor
}

//changes an image format ..............................................................
void Image::Align(int mask=3)
{
if(!img) return;
stat|=mask;
if(!(Bpl&(stat&0xf))) return; //already aligned
Image tmpimg;
tmpimg.Init(ALIGN(lng,stat&0xf),lat,pf,stat,NULL,pmw,pmh,file); //create the new one
BltU(tmpimg.img,tmpimg.Bpl,img,Bpl,lng,lat,Bpp);
Free(stat);
CopyMemory(this,&tmpimg,sizeof(Image)); //this=tmpimg
tmpimg.stat=0; //prevent freeing in destructor
}

//clears memory to c  ...........................................................................
inline void Image::Clear(DWORD c=0)
{
ifn(img) return;
FillMem(img,&c,Bpp,nrp);
}

//>>>>>>>>>>>>>>>>>>>>>>>>>Editing functions (only for 32 b/pixel) <<<<<<<<<<<<<<<<<<<<<
//...............................................................................................
void Image::HFlip()
{
WARN(pf&0xffff!=0x8888,"You're using a function in Image class that is not compatible with curent pixel format");
NAT l;
DWORD aux;
for(int y=0;y<lat;y++)
 {
 l=y*lng;
 for(int x=0;x<(lng>>1);x++)
  {
  aux=imgDW[l+x];
  imgDW[l+x]=imgDW[l+(lng-x-1)];
  imgDW[l+(lng-x-1)]=aux;
  }
 }
}

//...............................................................................................
void Image::VFlip()
{
WARN(pf&0xffff!=0x8888,"You're using a function in Image class that is not compatible with curent pixel format");
NAT l;
DWORD aux;
for(int x=0;x<lng;x++)
 {
 for(int y=0;y<(lat>>1);y++)
  {
  aux=imgDW[y*lng+x];
  imgDW[y*lng+x]=imgDW[(lat-y-1)*lng+x];
  imgDW[(lat-y-1)*lng+x]=aux;
  }
 }
}

//changes dimensions of the image with filter ..........................................
void Image::Resize(int neww,int newh,int filter)
{
WARN(pf&0xffff!=0x8888,"You're using a function in Image class that is not compatible with curent pixel format");
if(!img) return;
if(neww<=0) neww=1;
if(newh<=0) newh=1;
CCONV lcc;
BYTE *altB;
altB=(BYTE*)ALLOC(neww*newh*Bpp+4);
NAT od,c;
int xs,ys,xd,yd,osl,osr,osu,osd;
float x,y,dx,dy,fl,fr,fu,fd;
if(!filter) //choose best filter
 {
 if(neww<=lng&&newh<=lat) filter=3;
 else if((pf&0xffff)==0x8888) filter=1;
 else filter=2;
 }
if(filter==1&&(pf&0xffff)==0x8888) //resample bilinear
 {
 od=0;
 dx=(float)lng/neww;
 dy=(float)lat/newh;
 y=0;
 for(yd=0;yd<newh;yd++)
  {
  ys=RoundF(y,0);
  fu=ys-y+.5f;
  fd=y-ys+.5f;
  osu=ys-1;
  if(osu>0) osu*=Bpl;
  else osu=0;
  osd=(ys<lat-1?ys*Bpl:szB-Bpl);
  x=0;
  for(xd=0;xd<neww;xd++)
   {
   xs=RoundF(x,0);
   fl=xs-x+.5f;
   fr=x-xs+.5f;
   osl=xs-1;
   if(osl>0) osl<<=2;
   else osl=0;
   osr=(xs<lng-1?xs<<2:Bpl-4);
   for(c=0;c<4;c++)
    {
    altB[od]=fu*imgB[osu+osl]*fl;
    altB[od]+=fu*imgB[osu+osr]*fr;
    altB[od]+=fd*imgB[osd+osl]*fl;
    altB[od]+=fd*imgB[osd+osr]*fr;
    osl++;
    osr++;
    od++;
    }
   x+=dx;
   }
  y+=dy;
  }
 }
else if(filter==2) //resize
 ZoomU(altB,neww*Bpp,neww,newh,img,Bpl,lng,lat,Bpp);
else if(filter==3&&neww<=lng&&newh<=lat) //shrink box averaging
 {
 lcc.Tf(pf,pf);
 lcc.Rpr(altB,neww*Bpp,neww,newh,img,Bpl,lng,lat);
 }
else return; //no valid filter
Init(neww,newh,pf,IMG_IMB,altB,pmw*lng/neww,pmw*lat/newh);
}

//changes image dimensions keeping resolution ...........................................
void Image::Crop(int l,int u,int r,int d,DWORD bc)
{
if(!imgDW) return;
if(l>r||u>d) return;
int w,h;
w=r-l+1; h=d-u+1;
DWORD *altDW;
altDW=(DWORD*)malloc(((w*h)<<2)+4);
FillDW(altDW,bc,w*h);
if(l>=0)
 {
 if(l+w>lng) r=lng-l;
 else r=w;
 }
else
 {
 if(r+1>lng) r=lng;
 else r++;
 }
if(u>=0)
 {
 if(u+h>lat) d=lat-u;
 else d=h;
 }
else
 {
 if(d+1>lat) d=lat;
 else d++;
 }
BltQ(altDW,w<<2,l<0?-l:0,u<0?-u:0,imgDW,Bpl,l<0?0:l,u<0?0:u,r,d);
Init(w,h,pf,IMG_IMB,altDW);
}

//find color .....................................................................
DWORD Image::Color(int way=0,int ch=0)
{
int min,max,p,x,y;
if(ch>=Bpp) ch=0;
if(way==1) //min per channel
 {
 min=255;
 for(y=0;y<lat;y++)
  {
  p=y*Bpl+ch;
  for(x=0;x<lng;x++)
   {
   if((int)imgB[p]<min) min=imgB[p];
   p+=Bpp;
   }
  } 
 return min;
 }
else if(way==2) //max per channel
 {
 max=0;
 for(y=0;y<lat;y++)
  {
  p=y*Bpl+ch;
  for(x=0;x<lng;x++)
   {
   if((int)imgB[p]>max) max=imgB[p];
   p+=Bpp;
   }
  } 
 return max; 
 }
return 0;
}

//predefined images .....................................................................
void Image::Fill(int way,DWORD c1,DWORD c2)
{
WARN(pf&0xffff!=0x8888,"You're using a function in Image class that is not compatible with curent pixel format");
int x,y;
NAT el=0;
int r,g,b;   //relative positions
float f,ff;
BYTE m;
if(!imgDW) Init(512,512,0x32108888,IMG_IMB,NULL);
b=(pf>>16)&0xf;
g=(pf>>20)&0xf;
r=(pf>>24)&0xf;
if(way==0) //color fill
 {
 FillMem(img,&c1,Bpp,nrp);
 }
else if(way==1) //horiz degrade c1<->c2
 {
 for(y=0;y<lat;y++)
  FillDW(imgDW+y*lng,RGBAlerp(c1,c2,(float)y/lat),lng);
 }
else if(way==2) //vert degrade c1<->c2
 {
 for(y=0;y<lat;y++)
  for(x=0;x<lng;x++)
   imgDW[el++]=RGBAlerp(c1,c2,(float)x/lng);
 }
else if(way==3) //oblic / degrade c1<->c2
 {
 for(y=0;y<lat;y++)
  for(x=0;x<lng;x++)
   imgDW[el++]=RGBAlerp(c1,c2,((float)x/lng+(float)y/lat)/2);
 }
else if(way==4) //oblic \ degrade c1<->c2
 {
 for(y=0;y<lat;y++)
  for(x=lng;x>0;x--)
   imgDW[el++]=RGBAlerp(c1,c2,((float)x/lng+(float)y/lat)/2);
 }
else if(way==5) //R=x G=y B=1-x A=1-y
 {
 for(y=0;y<lat;y++)
  for(x=0;x<lng;x++)
   imgDW[el++]=BGRA_DW((x<<8)/lng,(y<<8)/lat,((lng-x-1)<<8)/lng,((lat-y-1)<<8)/lat);
 }
else if(way==6) //C=x M=y Y=1-x K=1-y
 {
 for(y=0;y<lat;y++)
  for(x=0;x<lng;x++)
   imgDW[el++]=NBGRA_DW((x<<8)/lng,(y<<8)/lat,((lng-x-1)<<8)/lng,((lat-y-1)<<8)/lat);
 }
else if(way==7) //grid
 {
 for(y=0;y<lat;y++)
  for(x=0;x<lng;x++)
   imgDW[el++]=BGRA_DW(x&1?0xff:0,0,y&1?0xff:0,(x&1)&&(y&1)?0xff:0);
 }
else if(way==8) //GREY: checkerboard
 {
 for(y=0;y<lat;y++)
  for(x=0;x<lng;x++)
   imgB[el++]=y&1?c1:c2;
 }
else if(way==9) //GREY: BYTE diagonal
 {
 FillMem(img,&c1,Bpp,nrp); //clear
 ff=(1.0f+1.0f/lat);
 for(f=0;f<szB;f+=(float)lng*ff)
  imgB[(NAT)f]=c2;
 }
else if(way==10) //GREY: BIT diagonal
 {
 FillMem(img,&c1,Bpp,nrp); //clear
 ff=(1.0f+1.0f/lat);
 m=0x80;
 for(f=0;f<szB;f+=(float)lng*ff)
  {
  imgB[(NAT)f]^=m;
  ROLR(m,1);
  }
 }
}

#define IMG_A_PREMOD		    0 //RGB*=A
#define IMG_A_CONST				1 //A=a0
#define IMG_A_NEG			    2 //A=255-A
#define IMG_A_BINARY		    3 //A=binary using a0 as threshold
#define IMG_A_R					7 //A=R
#define IMG_A_G				    8 //A=G
#define IMG_A_B				    9 //A=B
#define IMG_A_GREY0 		   10 //A=(R+G+B)/3
#define IMG_A_GREY1 		   11 //A=
#define IMG_A_GREY2			   12 //A=
#define IMG_A_GREY3		       13 //A=
#define IMG_A_BOUND			   20 //if(c1<c<c2) A=a0
#define IMG_A_EDGE_T		   21 //A=Edge(a0=threshhold)

//works on the transparency channel ......................................................
void Image::Alpha(int way,BYTE a0,DWORD c1,DWORD c2)
{
WARN(pf&0xffff!=0x8888,"You're using a function in Image class that is not compatible with current pixel format");
if(!imgDW) return;
NAT el=0;
float fc;
int r,g,b,x,y;   //relative positions
int A,B,G,R;
//int C,M,Y,K;
COLOR cc1,cc2;
cc1.dw=c1;
cc2.dw=c2;
b=(pf>>16)&0xf;
g=(pf>>20)&0xf;
r=(pf>>24)&0xf;
if(way==IMG_A_CONST) //A=a0
 {
 for(el=3;el<szB;el+=4)
  imgB[el]=a0;
 }
else if(way==IMG_A_BOUND) //if(c1<c<c2) A=a0
 {
 for(el=0;el<szB;el+=4)
  {
  if(imgB[el]<cc1.R||imgB[el]>cc2.R) continue;
  if(imgB[el+1]<cc1.G||imgB[el+1]>cc2.G) continue;
  if(imgB[el+2]<cc1.B||imgB[el+2]>cc2.B) continue;
  imgB[el+3]=a0;
  }
 }
else if(way==IMG_A_NEG) //A=not A
 {
 for(el=3;el<szB;el+=4)
  imgB[el]=255-imgB[el];
 }
else if(way==IMG_A_BINARY)
 {
 for(el=3;el<szB;el+=4)
  imgB[el]=imgB[el]>a0?255:0;
 }
else if(way==IMG_A_PREMOD) //RGB*=A/255 (premodulate)
 {
 while(el<szB)
  {
  fc=(float)imgB[el+3]/255.0f;
  imgB[el++]*=fc;
  imgB[el++]*=fc;
  imgB[el]*=fc;
  el+=2;
  }
 }
else if(way==IMG_A_GREY0) //A=(R+G+B)/3
 {
 for(el=0;el<szB;el+=4)
  imgB[el+3]=(imgB[el]+imgB[el+1]+imgB[el+2])/3;
 }
else if(way==IMG_A_GREY1) //A=R*222+G*707+B*71 (this is ITU standard) (recomended)
 {
 for(el=0;el<szB;el+=4)
  imgB[el+3]=(imgB[el+b]*71+imgB[el+g]*707+imgB[el+r]*222)/1000;
 }
else if(way==IMG_A_GREY2) //A=R*213+G*715+B*72 (REC701)
 {
 for(el=0;el<szB;el+=4)
  imgB[el+3]=(imgB[el+b]*72+imgB[el+g]*715+imgB[el+r]*213)/1000;
 }
else if(way==IMG_A_GREY3) //A=R*299+G*587+B*114  (REC601)
 {
 for(el=0;el<szB;el+=4)
  imgB[el+3]=(imgB[el+b]*114+imgB[el+g]*587+imgB[el+r]*299)/1000;
 }
else if(way==IMG_A_R) //A=R
 {
 for(el=0;el<szB;el+=4)
  imgB[el+3]=imgB[el+r];
 }
else if(way==IMG_A_G) //A=G
 {
 for(el=0;el<szB;el+=4)
  imgB[el+3]=imgB[el+g];
 }
else if(way==IMG_A_B) //A=B
 {
 for(el=0;el<szB;el+=4)
  imgB[el+3]=imgB[el+b];
 }
else if(way==IMG_A_EDGE_T) //edge detection
 {
 A=a0*a0;  //threshold
 for(el=3;el<szB;el+=4)
  imgB[el]=0;  //zero alpha channel
 el=lng;
 for(y=1;y<lat;y++)
  for(x=1;x<lng;x++)
   {
   el++;
   R=imgC[el-1].R-imgC[el].R;
   G=imgC[el-1].G-imgC[el].G;
   B=imgC[el-1].B-imgC[el].B;
   if(R*R+G*G+B*B>=A)  //compare with E
    imgC[el].A=255;
   else
    {
    R=imgC[el-lng].R-imgC[el].R;
    G=imgC[el-lng].G-imgC[el].G;
    B=imgC[el-lng].B-imgC[el].B;
    if(R*R+G*G+B*B>=A)   //compare with N
     imgC[el].A=255;
    //else
    // {
    // R=imgC[el-lng-1].R-imgC[el].R;
    // G=imgC[el-lng-1].G-imgC[el].G;
    // B=imgC[el-lng-1].B-imgC[el].B;
    // if(R*R+G*G+B*B>=A)  //compare with NE
    //  imgC[el].A=255;
    // }
    }
   }
 }
}

#define IMG_MOD_CMYK				1
#define IMG_MOD_HSL 				2
#define IMG_MOD_GAUSSIAN			3  //sigma=power, rz=gaussian mask size/2
#define IMG_MOD_BRIGHTNESS			4  //rz=shift in brightness
#define IMG_MOD_GAMMA				5  //sigma=gamma
#define IMG_MOD_SHARPNESS 			6  //rz=mask size/2
#define IMG_MOD_MEAN 				7  //rz=mask size
#define IMG_MOD_EDGE_LAPLACIAN		8  //rz=mask size
#define IMG_MOD_EMBOSS				9  //|rz|=depth (rz>0 emboss, rz<0 engrave)
#define IMG_MOD_AUTO_CONTRAST		10 //rz=channel
#define IMG_MOD_AUTO_CONTRAST_RGB	11
#define IMG_MOD_LN					12
#define IMG_MOD_LG					13
#define IMG_MOD_EXPONENTIAL			14  //sigma=base
#define IMG_MOD_NEG					15
#define IMG_MOD_MASK_LSBITS    		20  //reduce number of bits per color rz=LSBits to cut per ch
#define IMG_MOD_MASK_MSBITS    		21  //reduce number of bits per color rz=MSBits to cut per ch

//converts/filters the image ......................................................
void Image::Mod(int way=0,double sigma=1.,int rz=3)
{
WARN(pf&0xffff!=0x8888,"You're using a function in Image class that is not compatible with current pixel format");
if(!imgDW) return;
double*convmask,mag,sum;
NAT el;
int r,g,b,i,dm;   //relative positions
BYTE C,M,Y,K,H,S,L;
b=(pf>>16)&0xf;
g=(pf>>20)&0xf;
r=(pf>>24)&0xf;
if(way==IMG_MOD_NEG) //negate
 {
 for(el=0;el<szB;el++)
  imgB[el]=255-imgB[el];
 }
else if(way==IMG_MOD_CMYK)   //convert to CMYK
 {
 for(el=0;el<szB;el+=4)
  {
  RGBtoCMYK(imgB[el+r],imgB[el+g],imgB[el+b],C,M,Y,K);
  imgB[el]=C;
  imgB[el+1]=M;
  imgB[el+2]=Y;
  imgB[el+3]=K;
  }
 }
else if(way==IMG_MOD_HSL)   //convert to HSL
 {
 for(el=0;el<szB;el+=4)
  {
  RGBtoHSL(imgB[el+r],imgB[el+g],imgB[el+b],H,S,L);
  imgB[el]=L;  //B
  imgB[el+1]=S;//G
  imgB[el+2]=H;//R
  imgB[el+3]=0xff;
  }
 }
else if(way==IMG_MOD_BRIGHTNESS) //brightness
 {
 for(el=0;el<szB;el++)
  {
  i=rz+imgB[el];  //B
  if(i>255) imgB[el]=255;
  else if(i<0) imgB[el]=0;
  else imgB[el]=i;   //clamped
  }
 }
else if(way==IMG_MOD_AUTO_CONTRAST_RGB) //maximize contrast
 {
 Y=MIN3(Color(1,0),Color(1,1),Color(1,2));   //min(minR,minG,minB)
 M=MAX3(Color(2,0),Color(2,1),Color(2,2));   //max(maxR,maxG,maxB)
 mag=255./(M-Y);
 if(mag>1.)
  for(el=0;el<szB;el+=4)
   {
   imgB[el]=(imgB[el]-Y)*mag;
   imgB[el+1]=(imgB[el+1]-Y)*mag;
   imgB[el+2]=(imgB[el+2]-Y)*mag;
   }
 }
else if(way==IMG_MOD_AUTO_CONTRAST) //rz is channel
 {
 Y=Color(1,rz);   //minA
 M=Color(2,rz);   //maxA
 mag=255./(M-Y);
 if(mag>1.)
  for(el=rz;el<szB;el+=Bpp)
   imgB[el]=(imgB[el]-Y)*mag;
 }
else if(way==IMG_MOD_GAMMA) //gamma
 {
 for(el=0;el<szB;el++)
  imgB[el]=pow((double)imgB[el]/255.,sigma)*255;
 }
else if(way==IMG_MOD_EXPONENTIAL) //exponential
 {
 mag=255./(pow(sigma,255.)-1);
 for(el=0;el<szB;el++)
  imgB[el]=(pow(sigma,(double)imgB[el])-1.)*mag;
 }
else if(way==IMG_MOD_LN)   //natural logarithmic
 {
 mag=255./log(256.);
 for(el=0;el<szB;el++)
  imgB[el]=mag*log(1.+imgB[el]);
 }
else if(way==IMG_MOD_LG)   //base 10 logarithmic
 {
 mag=255./log10(256.);
 for(el=0;el<szB;el++)
  imgB[el]=mag*log10(1.+imgB[el]);
 }
else if(way==IMG_MOD_GAUSSIAN)   //gaussian filter
 {
 convmask=(double*)ALLOC((rz+1)*sizeof(double));
 mag=1./(SQRT(2.*PI)*sigma);
 sigma*=2.*sigma; //2*sqr(sigma)
 sum=0.;
 for(i=0;i<=rz;i++)
  {
  convmask[i]=mag*exp(-i*i/sigma);
  sum+=convmask[i];
  }
 sum=sum*2-convmask[0];  for(i=0;i<=rz;i++)
  convmask[i]/=sum;
 Convolute1D(convmask,rz);
 FREE(convmask);
 }
else if(way==IMG_MOD_MEAN)   //mean filter
 {
 convmask=(double*)ALLOC((rz+1)*sizeof(double));
 dm=((rz<<1)+1);
 for(i=0;i<=rz;i++)
  *(convmask+i)=1./(double)dm;
 Convolute1D(convmask,rz);
 FREE(convmask);
 }
else if(way==IMG_MOD_SHARPNESS)   //sharpness filter
 {
 rz=1;
 dm=(rz<<1)+1;
 convmask=(double*)ALLOC(dm*dm*sizeof(double));
 convmask[0]=convmask[2]=convmask[6]=convmask[8]=0;
 convmask[1]=convmask[3]=convmask[5]=convmask[7]=-1;
 convmask[4]=5;
 //NormVect(convmask,dm*dm);
 Convolute2D(convmask,rz,0);
 FREE(convmask);
 }
else if(way==IMG_MOD_EMBOSS)   //emboss/engrave
 {
 convmask=(double*)ALLOC(9*sizeof(double));
 for(i=0;i<9;i++)
  *(convmask+i)=0.;
 convmask[8]=2*rz;
 convmask[0]=convmask[4]=-rz;
 Convolute2D(convmask,1,128);
 FREE(convmask);
 }
else if(way==IMG_MOD_EDGE_LAPLACIAN)   //edge detection filter
 {
 convmask=(double*)ALLOC(25*sizeof(double));
 for(i=0;i<25;i++)   *(convmask+i)=-1.;
 convmask[2*5+2]=24.;
 Convolute2D(convmask,2,0);
 FREE(convmask);
 }
else if(way==IMG_MOD_MASK_LSBITS)
 {
 C=(0xff>>(rz&0xf))<<(rz&0xf);
 M=(0xff>>((rz>>4)&0xf))<<((rz>>4)&0xf);
 Y=(0xff>>((rz>>8)&0xf))<<((rz>>8)&0xf);
 K=(0xff>>((rz>>12)&0xf))<<((rz>>12)&0xf);
 for(el=0;el<szB;el+=4)
  {
  imgB[el]&=C;
  imgB[el+1]&=M;
  imgB[el+2]&=Y;
  imgB[el+3]&=K;
  }
 }
else if(way==IMG_MOD_MASK_MSBITS)
 {
 C=(0xff>>(rz&0xf));
 M=(0xff>>((rz>>4)&0xf));
 Y=(0xff>>((rz>>8)&0xf));
 K=(0xff>>((rz>>12)&0xf));
 for(el=0;el<szB;el+=4)
  {
  imgB[el]&=C;
  imgB[el+1]&=M;
  imgB[el+2]&=Y;
  imgB[el+3]&=K;
  }
 }
else
 error("Image::Mod() invalid method");
}

//...........................................................................................................................
inline void Image::GreyTf(BYTE*tf,int ch=3)
{
for(NAT p=ch;p<szB;p+=4)
 imgB[p]=tf[imgB[p]];
}

//apply 1D circular convolution (simetric)..............................................................
void Image::Convolute1D(double*convmask,int rz=3)
{
WARN(pf&0xffff!=0x8888,"You're using a function in Image class that is not compatible with curent pixel format");
int x,y,i,j;
COLOR*altC,*aux;
NAT el;
 altC=(COLOR*)ALLOC(szB);
 for(y=0;y<lat;y++)
  {
  for(x=0;x<lng;x++)
   {
   el=y*lng+x;
   altC[el].R=imgC[el].R*convmask[0];
   altC[el].G=imgC[el].G*convmask[0];
   altC[el].B=imgC[el].B*convmask[0];
   altC[el].A=imgC[el].A*convmask[0];
   if(x<rz||x>=lng-rz) //margins
	for(i=1;i<=rz;i++)
     {
     if(x-i<0)
      {
      altC[el].R+=(imgC[el+i].R<<1)*convmask[i];
      altC[el].G+=(imgC[el+i].G<<1)*convmask[i];
      altC[el].B+=(imgC[el+i].B<<1)*convmask[i];
      altC[el].A+=(imgC[el+i].A<<1)*convmask[i];
	  }
     else if(x+i>=lng)
      {
      altC[el].R+=(imgC[el-i].R<<1)*convmask[i];
      altC[el].G+=(imgC[el-i].G<<1)*convmask[i];
      altC[el].B+=(imgC[el-i].B<<1)*convmask[i];
      altC[el].A+=(imgC[el-i].A<<1)*convmask[i];
	  }
     else
      {
      altC[el].R+=(imgC[el+i].R+imgC[el-i].R)*convmask[i];
      altC[el].G+=(imgC[el+i].G+imgC[el-i].G)*convmask[i];
      altC[el].B+=(imgC[el+i].B+imgC[el-i].B)*convmask[i];
      altC[el].A+=(imgC[el+i].A+imgC[el-i].A)*convmask[i];
      }
     }
   else
	for(i=1;i<=rz;i++)   //middle
     {
     altC[el].R+=(imgC[el+i].R+imgC[el-i].R)*convmask[i];
     altC[el].G+=(imgC[el+i].G+imgC[el-i].G)*convmask[i];
     altC[el].B+=(imgC[el+i].B+imgC[el-i].B)*convmask[i];
     altC[el].A+=(imgC[el+i].A+imgC[el-i].A)*convmask[i];
     }
   }
  }
 aux=imgC;   //use new image as source and original as destination
 imgC=altC;
 altC=aux;
 for(x=0;x<lng;x++)
  {
  for(y=0;y<lat;y++)
   {
   el=y*lng+x;
   altC[el].R=imgC[el].R*convmask[0];
   altC[el].G=imgC[el].G*convmask[0];
   altC[el].B=imgC[el].B*convmask[0];
   altC[el].A=imgC[el].A*convmask[0];
   if(y<rz||y>=lat-rz) //margins
	for(i=1;i<=rz;i++)
     {
     j=i*lng;
     if(y-i<0)
      {
      altC[el].R+=(imgC[el+j].R<<1)*convmask[i];
      altC[el].G+=(imgC[el+j].G<<1)*convmask[i];
      altC[el].B+=(imgC[el+j].B<<1)*convmask[i];
      altC[el].A+=(imgC[el+j].A<<1)*convmask[i];
	  }
     else if(y+i>=lat)
      {
      altC[el].R+=(imgC[el-j].R<<1)*convmask[i];
      altC[el].G+=(imgC[el-j].G<<1)*convmask[i];
      altC[el].B+=(imgC[el-j].B<<1)*convmask[i];
      altC[el].A+=(imgC[el-j].A<<1)*convmask[i];
	  }
     else
      {
      altC[el].R+=(imgC[el+j].R+imgC[el-j].R)*convmask[i];
      altC[el].G+=(imgC[el+j].G+imgC[el-j].G)*convmask[i];
      altC[el].B+=(imgC[el+j].B+imgC[el-j].B)*convmask[i];
      altC[el].A+=(imgC[el+j].A+imgC[el-j].A)*convmask[i];
      }
     }
   else
	for(i=1;i<=rz;i++)   //middle
     {
     j=i*lng;
     altC[el].R+=(imgC[el+j].R+imgC[el-j].R)*convmask[i];
     altC[el].G+=(imgC[el+j].G+imgC[el-j].G)*convmask[i];
     altC[el].B+=(imgC[el+j].B+imgC[el-j].B)*convmask[i];
     altC[el].A+=(imgC[el+j].A+imgC[el-j].A)*convmask[i];
     }
   }
  }
 aux=imgC;
 imgC=altC;
 altC=aux;
 FREE(altC); //free alt1
}

//apply 2D convolution (convmask should be normalized)..............................................................
void Image::Convolute2D(double*convmask,int l=3,int corection=0)
{
WARN(pf&0xffff!=0x8888,"You're using a function in Image class that is not compatible with curent pixel format");
int x,y,i,j,m,n,o,p=0,ll;
int R,G,B,A;
double f;
COLOR*altC;
ll=(l<<1)+1;
altC=(COLOR*)ALLOC(szB);
for(y=0;y<lat;y++)
 for(x=0;x<lng;x++)
  {
  R=G=B=A=corection;
  if(x>=l&&x<lng-l&&y>=l&&y<lat-l) //middle
   for(j=-l;j<=l;j++)
    {
    m=(j+l)*ll;	//temp convmask line
    n=p+j*lng;  //temp image pixel line
    for(i=-l;i<=l;i++)
     {
     f=convmask[(i+l)+m];
     o=i+n;
     R+=f*imgC[o].R;
     G+=f*imgC[o].G;
     B+=f*imgC[o].B;
     A+=f*imgC[o].A;
     }
    }
  else   //margins
   for(j=-l;j<=l;j++)
    {
    m=(j+l)*ll; 
    if(y<l)
     n=p+ABS(j)*lng;
    else if(y>=lat-l)
     n=p-ABS(j)*lng;
    else
     n=p+j*lng;
    for(i=-l;i<=l;i++)
     {
     f=convmask[(i+l)+m];
     if(x<l)
      o=n+ABS(i);
     else if(x>=lng-l)
      o=n-ABS(i);
     else
      o=n+i;
     R+=f*imgC[o].R;
     G+=f*imgC[o].G;
     B+=f*imgC[o].B;
     A+=f*imgC[o].A;
     }
    }
  altC[p].R=CLAMP(R,0,255);
  altC[p].G=CLAMP(G,0,255);
  altC[p].B=CLAMP(B,0,255);
  altC[p].A=CLAMP(A,0,255);
  p++; //pixel position
  }
 FREE(imgC); //discard original
 imgC=altC;
}

//channel histogram (hist should have 256*sizeof(NAT))...............................................................
inline NAT Image::ChHist(NAT*hist,int ch=3)
{
NAT p,uniq=0;   //relative positions
ZeroMemory(hist,256*sizeof(NAT));
if(stat&IMG_GREY) ch=0;
for(p=ch;p<Bpl*lat;p+=Bpp)
 {
 if(!hist[imgB[p]]) uniq++;
 hist[imgB[p]]++;
 }
return uniq;
}

//true color (RGB channels) histogram (hist should have 16MB*sizeof(NAT))...............................................................
inline NAT Image::ColorHist(NAT*hist)
{
WARN(Bpp!=4,"You're using a function in Image class that is not compatible with curent pixel format");
NAT p,uniq=0;   //relative positions
ZeroMemory(hist,(1<<24)*sizeof(NAT));
for(p=0;p<(nrp);p++)
 hist[imgDW[p]&0xffffff]++;
for(p=0;p<(1<<24);p++)
 if(hist[p]) uniq++;
return uniq;
}

//..............................................................................................................................
inline NAT Image::CountColors()
{
NAT*hist,uniq;
hist=(NAT*)ALLOC((1<<24)*sizeof(NAT));
isNULL(hist,"Not enough memory to count colors");
uniq=ColorHist(hist);
FREE(hist);
return uniq;
}

//..............................................................................................................................
inline void Image::EqualHist(int ch=3)
{
NAT hist[256],p;
ChHist(hist,ch);
//integrate
hist[0]=0;   //this is so you also have a max contrast
for(p=2;p<256;p++)
 hist[p]+=hist[p-1];
//ShowArray(hist,256,hdbgwnd);
//grey tf (all channels)
for(p=0;p<szB;p+=1)
 imgB[p]=hist[imgB[p]]*255/hist[255];
}

#define IMG_THRESHOLD_ISODATA		1
#define IMG_THRESHOLD_OTSU  		2  //gives identical results to ISODATA
#define IMG_THRESHOLD_EQPROB		3  //equal probability between fore/back pixels
#define IMG_THRESHOLD_TRIANGLE		10  //searches for triangle peaks
#define IMG_THRESHOLD_GAUSSIAN		11  //searches for gaussian peaks

//([0;T]=background [T+1;255]=foreground)..............................................................................................................................
inline NAT Image::Threshold(int way=0,int ch=3)
{
NAT*hist,T,N,i;
double MF,MB,m,MT,sB;
hist=(NAT*)ALLOC(256*sizeof(NAT));
ChHist(hist,ch);
N=lng*lat;	//=sum(hist,256)
if(way==IMG_THRESHOLD_ISODATA)
 {
 T=127; //initial threshold
 do{
  MB=MF=m=0.;
  for(i=0;i<=T;i++)
   {
   MB+=i*hist[i];
   m+=hist[i];
   }
  MB/=m;
  m=0.;
  for(i=T+1;i<=255;i++)
   {
   MF+=i*hist[i];
   m+=hist[i];
   }
  MF/=m;
  N=T;
  T=(MF+MB)/2;
 }while(T!=N);
 }
else if(way==IMG_THRESHOLD_OTSU)
 {
 MB=0.;
 T=0; //T optim
 MF=m=(double)hist[0]/N;
 MT=Moment1(hist,256)/N;
 for(i=1;i<=255;i++)
  {
  sB=(m&&m!=1.)?(MT*m-MF)*(MT*m-MF)/(m*(1.-m)):0.;
  if(sB>MB)
   {
   MB=sB;
   T=i;
   }
  MF+=(double)hist[i]*(i+1)/N;
  m+=(double)hist[i]/N;
  }
 }
else if(way==IMG_THRESHOLD_EQPROB)
 {
 T=i=0;
 do{
  i+=hist[T++];
 }while(i<(N>>1));
 }
else
 error("Image::Threshold() invalid method");
FREE(hist);
return T;
}

//Find objects in histogram ()..............................................................................................................................
inline void Image::HistSeg(int way,NAT*obj,NAT nrobjs=1,int ch=3)
{
int hist[256];
ChHist((NAT*)hist,ch);
if(way==IMG_THRESHOLD_TRIANGLE)
 {
 //DataSegTriangle(hist,256,obj,nrobjs);
 }
else if(way==IMG_THRESHOLD_GAUSSIAN)
 {
 //DataSegGaussian(hist,256,obj,nrobjs);
 }
else
 error("Image::HistSeg() invalid method");
}

//..........................................................................
float Image::HFore(int x,BYTE fore,int ch=3)
{
float p=0.f;
for(int y=x*Bpp+ch;y<szB;y+=Bpl)
 if(imgB[y]>=fore)
  p++;
return p/lat;
}

//..........................................................................
float Image::VFore(int y,BYTE fore,int ch=3)
{
float p=0.f;
NAT el=y*Bpl+ch;
for(int x=0;x<lng;x++)
 {
 if(imgB[el]>=fore)
  p++;
 el+=Bpp;
 }
return p/lng;
}

//..........................................................................
void Image::CropFore(VECT2 fore,float eps=0.f,int ch=3)
{
int l,u,r,d;
for(l=0;l<lng;l++)
 if(HFore(l,fore.x,ch)>eps)
  break;
for(r=lng-1;r>0;r--)
 if(HFore(r,fore.x,ch)>eps)
  break;
for(u=0;u<lat;u++)
 if(VFore(u,fore.x,ch)>eps)
  break;
for(d=lat-1;d>0;d--)
 if(VFore(d,fore.x,ch)>eps)
  break;
Crop(l,u,r,d);
}


//find objects .................................................................
NAT Image::HCut(VECT2*obj,NAT nrobj,VECT2 fore,float eps=0.1f,int ch=3)
{
float*hmean,*hvar,*pfore,meanlim,maxvar;
VECT2 *cut;
float eps1=0.001f;
int x,y,o,peak,nrcut,u;
NAT el;
hmean=(float*)ALLOC(lng*sizeof(float));
hvar=(float*)ALLOC(lng*sizeof(float));
pfore=(float*)ALLOC(lng*sizeof(float));
cut=(VECT2*)ALLOC(lng*sizeof(VECT2));
for(x=0;x<lng;x++)
 {
 hmean[x]=0;
 hvar[x]=0;
 el=x*Bpp+ch;
 for(y=0;y<lat;y++)
  {
  hmean[x]+=imgB[el];
  hvar[x]+=imgB[el]*imgB[el];
  el+=Bpl;
  }
 hmean[x]/=lat;
 hvar[x]/=lat;
 hvar[x]-=hmean[x]*hmean[x];
 hvar[x]=SQRT(hvar[x]);
 pfore[x]=HFore(x,fore.x,3);
 }
meanlim=(hmean[findmax(hmean,lng)]+hmean[findmin(hmean,lng)])/2;
maxvar=hvar[findmax(hvar,lng)];

//AdjustClientRect(hdbgwnd,lng+1,512,0,mwh+50);
//ShowArray(hmean,lng,hdbgwnd,0xff);
//WaitVKey();
//ShowArray(hvar,lng,hdbgwnd,0xff00,R2_XORPEN);
//WaitVKey();
//ShowArray(pfore,lng,hdbgwnd,0xff0000,R2_XORPEN);
nrcut=0;
u=0;
for(x=1;x<lng;x++)
 {
 if(u)   //search for up->down
  {
  if((pfore[x]<eps1)&&(pfore[x-1]>eps1))
   {
   cut[nrcut].y=x;
   u=0;
   nrcut++;
   }
  } 
 else	 //search for down->up
  {
  if((pfore[x]>eps1)&&(pfore[x-1]<eps1))
   {
   cut[nrcut].x=x;
   u=1;
   }
  } 
 }
for(x=0;x<nrcut;x++)
 {
 color=0xff;
 line(cut[x].x,0,cut[x].x,lat-1);
 color=0xff00;
 line(cut[x].y,0,cut[x].y,lat-1);
 }
CopyMemory(obj,cut,nrobj*sizeof(VECT2));
FREE(hmean);
FREE(hvar);
FREE(pfore);
FREE(cut);
return nrobj;
}

//find objects .................................................................
NAT Image::VCut(VECT2*obj,NAT nrobj,float eps=0.1f,int ch=3)
{
float*vmean,*vvar,maxvar=0.;
int x,y;
NAT el;
vmean=(float*)ALLOC(lat*sizeof(float));
vvar=(float*)ALLOC(lat*sizeof(float));
for(y=0;y<lat;y++)
 {
 vmean[y]=0;
 vvar[y]=0;
 el=y*Bpl+ch;
 for(x=0;x<lng;x++)
  {
  vmean[y]+=imgB[el];
  vvar[y]+=imgB[el]*imgB[el];
  el+=Bpp;
  }
 vmean[y]/=lng;
 vvar[y]/=lng;
 vvar[y]-=vmean[y]*vmean[y];
 vvar[y]=SQRT(vvar[y]);
 }
//ShowArray(vvar,lat,hdbgwnd,0xff00);
//el=DataFindPeaks(vvar,lat,obj,nrobj,eps);
FREE(vmean);
FREE(vvar);
return el;
}

//replace color.................................................................
void Image::OCRFilter(VECT2 fore,VECT2 back)
{
NAT el;
for(el=0;el<nrp;el++)
 {
 if(imgC[el].A<back.x) 
  {
  imgC[el].A=back.x;
  imgC[el].R=0;
  imgC[el].G=0;
  imgC[el].B=0xff;
  }
 else if(imgC[el].A>back.y&&imgC[el].A<fore.x) 
  {
  imgC[el].A=back.y;
  imgC[el].R=0;
  imgC[el].G=0xff;
  imgC[el].B=0;
  }
 else if(imgC[el].A>fore.y) 
  {
  imgC[el].A=fore.y;
  imgC[el].R=0xff;
  imgC[el].G=0;
  imgC[el].B=0;
  }
 }
}

//pixel combination masks ..............................................................
inline void Image::bop(DWORD mand=0,DWORD mor=0,DWORD mxor=0)
{
maskA=mand;
maskO=mor;
maskX=mxor;
}

//sets current position ............................................................
void Image::at(int x=0,int y=0)
{
__asm
 {
 mov eax,y
 mov [ecx]this.cy,eax //cy=y
 imul [ecx]this.Bpl //eax=y*Bpl
 add eax,[ecx]this.imgDW //eax+=imgDW
 mov edx,x
 mov [ecx]this.cx,edx //cx=x
 shl edx,2 //eax=x*4
 add eax,edx //eax=imgDW+y*Bpl+x*4
 mov [ecx]this.cp,eax //cp=eax
 }
}

//put pixel (doesn't use cp, uses maskA) ............................................................
void Image::pix(int x=0,int y=0)
{
__asm
 {
 mov eax,y
 imul [ecx]this.Bpl //eax=y*Bpl
 add eax,[ecx]this.imgDW //eax+=imgDW
 mov edx,x
 shl edx,2 //eax=x*4
 add eax,edx //eax=imgDW+y*Bpl+x*4
 mov edx,[eax]
 and edx,[ecx]this.maskA
 xor edx,[ecx]this.color
 mov [eax],edx
 }
}

//horizontal line from cp to cp+(w,0) ..............................................................................
void Image::linh(int w=0)
{
__asm
 {
 mov edx,ecx //save this
 mov edi,[ecx]this.cp //edi=cp
 mov eax,[ecx]this.color
 mov ecx,w
 jecxz LRet
 add [edx]this.cx,ecx //cx+=w
 test ecx,ecx
 jns LPositive
 std
 neg ecx
LPositive:
 rep stosd
 cld //should use pushfd/popfd to properly preserve dir flag
 mov [edx]this.cp,edi //update cp
LRet:
 }
}

//vertical line from cp to cp+(0,h) (uses maskA)..............................................................................
void Image::linv(int h=0)
{
__asm
 {
 mov edi,[ecx]this.cp //edi=cp
 mov esi,[ecx]this.Bpl //edi=Bpl
 mov ebx,[ecx]this.maskA
 mov eax,[ecx]this.color
 mov edx,h
 add [ecx]this.cy,edx //cy+=h
 test edx,edx
 jz LRet
 jns Lloop
 neg esi
 neg edx
Lloop:
  and [edi],ebx //and mask
  xor [edi],eax //write
  add edi,esi //next line
  dec edx
  jnz Lloop //loop while(edx>0)
 mov [ecx]this.cp,edi //update cp
LRet:
 }
}

//line from cp to (x-1,y-1) ..............................................................................
void Image::lin(int x=0,int y=0)
{
__asm
 {
 push ecx //save this
 push ebp
 mov edi,[ecx]this.cp //edi=cp
 mov esi,[ecx]this.Bpl //esi=Bpl (vertical increment)
 mov eax,y
 mov edx,eax
 sub edx,[ecx]this.cy //edx=y-cy
 mov [ecx]this.cy,eax //cy=y
 test edx,edx
 jns LPostiveY
  neg esi
  neg edx
LPostiveY: //H=|h|
 mov eax,x
 mov ebx,eax
 sub ebx,[ecx]this.cx //ebx=x-cx
 mov [ecx]this.cx,eax //cx=x
 mov ebp,[ecx]this.color
 mov ecx,ebx
 mov ebx,4 //(horizontal increment)
 test ecx,ecx
 jns LPostiveX
  neg ebx
  neg ecx
LPostiveX: //W=|w|
 cmp edx,ecx
 jbe LNoSwap //don't swap if (H<=W)
  xchg esi,ebx
  xchg ecx,edx
LNoSwap:
 jecxz LRet //w==h==0
 shl edx,16
 mov dx,cx //edx=H|W / W|H
 xor eax,eax //ax is decision variable
Lloop:
  mov [edi],ebp //write pixel
  add edi,ebx //increment primary
  rol edx,16 //edx=W|H / H|W
  add ax,dx //increment decision by H / W
  rol edx,16 //edx=H|W / W|H
  cmp ax,dx
  jl LSkip
   add edi,esi //increment secondary
   sub ax,dx
LSkip:
  loop Lloop
LRet:
 pop ebp
 pop ecx //this
 mov [ecx]this.cp,edi //update cp
 }
}

//rect from cp to cp+(w,h) ..............................................................................
void Image::rect(int w=0,int h=0)
{
w--;
h--;
linh(w);
linv(h);
linh(-w);
linv(-h);
}

//filled rect from cp to cp+(w,h) (uses cp but doesn't update cp)..............................................................................
void Image::rectf(int w=0,int h=0)
{
__asm
 {
 mov edi,[ecx]this.cp //edi=cp
 mov esi,[ecx]this.Bpl
 mov edx,h
 test edx,edx
 jz LRet
 jns LPositiveH
 neg esi
 neg edx
LPositiveH:
 mov ebx,w
 sal ebx,2
 jz LRet
 sub esi,ebx //esi=+/-Bpl-ebx*4
 sar ebx,2
 jns LPositiveW
 std
 neg ebx
LPositiveW:
 mov eax,[ecx]this.color
Lloop:
  mov ecx,ebx
  rep stosd
  add edi,esi //next line
  dec edx
  jnz Lloop //loop while(edx>0)
 cld //should use pushfd/popfd to properly preserve dir flag
LRet:
 }
}

//(doesn't uses cp, uses maskA)..................................................................................
void Image::line(int l,int u,int r,int d)
{
int w,h,dc,hs=1,vs=1;
w=r-l;
h=d-u;
if(w<0) {w=-w; hs=-hs;}
if(h<0) {h=-h; vs=-vs;}
w++;
h++;
if(w>h)
 {
 dc=h>>1;
 while(l!=r)
  {
  pix(l,u);
  dc+=h;
  if(dc>=w) {dc-=w; u+=vs;}
  l+=hs;
  }
 }
else
 {
 dc=w>>1;
 while(u!=d)
  {
  pix(l,u);
  dc+=w;
  if(dc>=h) {dc-=h; l+=hs;}
  u+=vs;
  }
 }
pix(r,d);
}

//replace color.................................................................
void Image::chcolor(DWORD c1,DWORD c2)
{
WARN(Bpp!=4,"Image::chcolor() must be used with 32bpp")
if(!imgDW) return;
for(NAT el=0;el<(szB>>2);el++)
 if(imgDW[el]==c1) imgDW[el]=c2;
}

//(n't uses cp, uses maskA)..................................................................................
void Image::bar(int l,int u,int r,int d)
{
WARN(Bpp!=4,"Image::bar() must be used with 32bpp")
if(!imgDW) return;
l=CLAMP(l,0,lng-1);
r=CLAMP(r,0,lng-1);
u=CLAMP(u,0,lat-1);
d=CLAMP(d,0,lat-1);
if(l>r) swap(l,r);
if(u>d) swap(l,r);
at(l,u);
rectf(r-l,d-u);
}

//(n't uses cp, uses maskA)..................................................................................
void Image::chcolinbar(int l,int u,int r,int d,DWORD cold=0,DWORD cnew=0xffffff)
{
WARN(Bpp!=4,"Image::chcolinbar() must be used with 32bpp")
if(!imgDW) return;
l=CLAMP(l,0,lng-1);
r=CLAMP(r,0,lng-1);
u=CLAMP(u,0,lat-1);
d=CLAMP(d,0,lat-1);
for(;u<=d;u++)
 {
 at(l,u);
 for(int x=l;x<=r;x++)
  {
  if(cp->dw==cold)
   cp->dw=cnew;
  cp++; 
  }
 }
}

//quadratic bezier spline (doesn't uses cp).....................................................................
void Image::spline2(int x1,int y1,int x2,int y2,int x3,int y3)
{
int np;
float x,y,dx,dy,d2x,d2y;
np=(abs(x3-x2)+abs(x2-x1)+abs(y3-y2)+abs(y2-y1))>>6;
fpuiof=(float)1.0/(np*np);
d2x=(float)(2*x1-4*x2+2*x3)*fpuiof;
d2y=(float)(2*y1-4*y2+2*y3)*fpuiof;
dx=(float)(-2*x1+2*x2)/np+d2x/2;
dy=(float)(-2*y1+2*y2)/np+d2y/2;
x=x1;
y=y1;
at(x1,y1);
for(int n=1;n<np;n++)
 {
 x+=dx;
 y+=dy;
 dx+=d2x;
 dy+=d2y;
 lin(x,y);
 }
lin(x3,y3);
}

//cubic bezier spline (doesn't uses cp) ..................................................................
void Image::spline3(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4)
{
int np;
float x,y,dx,dy,d2x,d2y,d3x,d3y;
np=(abs(x4-x3)+abs(x3-x2)+abs(x2-x1)+abs(y4-y3)+abs(y3-y2)+abs(y2-y1))>>6;
fpuiof=(float)1.0/(np*np*np);
d3x=(float)(-6*x1+18*x2-18*x3+6*x4)*fpuiof;
d3y=(float)(-6*y1+18*y2-18*y3+6*y4)*fpuiof;
fpuiof*=np;
d2x=(float)(6*x1-12*x2+6*x3)*fpuiof+d3x;
d2y=(float)(6*y1-12*y2+6*y3)*fpuiof+d3y;
dx=(float)(-3*x1+3*x2)/np+d2x/2;
dy=(float)(-3*y1+3*y2)/np+d2y/2;
x=x1;
y=y1;
at(x1,y1);
for(int n=1;n<np;n++)
 {
 x+=dx;
 y+=dy;
 dx+=d2x;
 dy+=d2y;
 d2x+=d3x;
 d2y+=d3y;
 lin(x,y);
 }
lin(x4,y4);
}

//draw a contour b/w trace  ....................................................................
inline void Image::dtrace(short*trace,int x=0,int y=0)
{
__asm
 {
 mov esi,trace
 test esi,esi
 jz LReturn
//cliping
 mov ebx,x
 test ebx,ebx
 js LReturn //if(x<0) return;
 movzx edx,word ptr[esi+4]
 add edx,ebx
 cmp edx,[ecx]this.lng
 jg LReturn //if(x+trace[2]*wf>=lng) return;
 movzx eax,word ptr [esi+8] //eax=trace[4]
 add eax,y //eax=y+trace[4]
 js LReturn //if(y<0) return;
 movzx edx,word ptr [esi+6] //edx=trace[3]
 mov y,edx //y=trace[3]
 add edx,eax
 cmp edx,[ecx]this.lat
 jg LReturn //if(y+trace[4]+trace[3]>=lat) return;
//build start dest pointer
 mov edi,[ecx]this.img
 shl ebx,2
 add edi,ebx
 mov ebx,[ecx]this.Bpl
 mov x,ebx
 mul ebx
 add edi,eax //edi=imgDW+(y+trace[4])*Bpl+x<<2
 add esi,10 //esi=&trace[5]
 mov eax,[ecx]this.color
LNextLine:
  movzx ebx,word ptr[esi] //edx=nr of packets
  add esi,2
  shr ebx,1
  jz LSkipLine //skip if line empty
  mov edx,edi //edx=line pointer
LNextPacket:
   movsx ecx,word ptr[esi] //ecx=false packet
   add esi,2
   shl ecx,2
   add edi,ecx
   movzx ecx,word ptr[esi] //ecx=true packet
   add esi,2
   rep stosd //draw packet
   dec ebx
   jnz LNextPacket
  mov edi,edx
LSkipLine:
  add edi,x //edx+=this.Bpl
  dec y
  jnz LNextLine
LReturn:
 }
}

//draw a contour b/w widened trace  ....................................................................
inline void Image::dwtrace(short*trace,int x=0,int y=0,float wf=1.f,float tf=0.f)
{
__asm
 {
 mov esi,trace
 fld tf
 fld wf
 fdecstp //alloc space for remainder bucket
 fild word ptr [esi+8] //trace[4]
 fld st(0)
 fmul st(0),st(4) //trace[4]*tf
 fst st(2)
 fld st(0)
 frndint
 fsub st(3),st(0) //st(2)=st(0)-round(st(0))
 fistp fpuioi
 mov edi,fpuioi
 mov eax,edi
 shl edi,2
 neg edi  //edi=-trace[4]*tf*4
 add edi,[ecx]this.img
 DISCARD_ST0
 fiadd word ptr [esi+6] //trace[3]+trace[4]
 fmul st(0),st(3) //(trace[3]+trace[4])*tf
 fistp fpuioi
//cliping
 mov ebx,x
 mov edx,ebx
 sub edx,fpuioi //
 js LReturn //if(x-(trace[3]+trace[4])*tf<0) return;
 fild word ptr [esi+4] //ST(1)=trace[2]
 fmul st(0),st(2)
 fistp x
 mov edx,x //edx=trace[2]*wf
 add edx,ebx
 sub edx,eax
 cmp edx,[ecx]this.lng
 jg LReturn //if(x+trace[2]*wf-trace[4]*tf>=lng) return;
 shl ebx,2
 add edi,ebx //edi+=x*4
 movzx eax,word ptr [esi+8] //eax=trace[4]
 add eax,y //eax=y+trace[4]
 js LReturn //if(y<0) return;
 movzx edx,word ptr [esi+6] //edx=trace[3]
 mov y,edx //y=trace[3]
 add edx,eax
 cmp edx,[ecx]this.lat
 jg LReturn //if(y+trace[4]+trace[3]>=lat) return;
//build start dest pointer
 mov ebx,[ecx]this.Bpl
 mov x,ebx
 mul ebx
 add edi,eax //edi=imgDW+(y+trace[4])*Bpl+x<<2
 add esi,10 //esi=&trace[5]
 mov eax,[ecx]this.color
 mov edx,edi //edx=line pointer
LNextLine:
  movzx ebx,word ptr[esi] //edx=nr of packets
  add esi,2
  shr ebx,1
  jz LSkipLine //skip if line empty
  fldz //float cx=0 ST(1)
LNextPacket:
   fild word ptr[esi] //ST(0)=false packet
   add esi,2
   fmul st(0),st(3) //*=wf
   fadd st(1),st(0) //cx+=false packet
   DISCARD_ST0
   fist fpuioi //fpuioi=round(cx)
   mov edi,fpuioi
   shl edi,2
   add edi,edx
   fild word ptr[esi] //ST(0)=true packet
   add esi,2
   fmul st(0),st(3) //*=wf
   fadd st(1),st(0) //cx+=true packet
   fistp fpuioi //packet width
   mov ecx,fpuioi
   rep stosd //draw packet
   dec ebx
   jnz LNextPacket
  DISCARD_ST0
LSkipLine:
  fadd st(0),st(2) //rem+=tf
  fld st(0)
  frndint
  fsub st(1),st(0)
  fistp fpuioi
  mov ecx,fpuioi
  shl ecx,2
  sub edx,ecx
  add edx,x //edx+=this.Bpl
  dec y
  jnz LNextLine
LReturn:
 DISCARD_ST0
 DISCARD_ST0
 DISCARD_ST0
 }
}

//draw a contour b/w scaled trace  ....................................................................
inline void Image::dstrace(short*trace,int x=0,int y=0,float wf=1.f,float hf=1.f,float tf=0.f)
{
__asm
 {
 mov esi,trace
 fld wf //widith factor
 fld tf //tilt factor
 fld hf //height factor
 fild word ptr [esi+8] //t4 (used later as {tilt step})
 fmul st(0),st(1) //t4*hf
 fist fpuioi //fpuioi=t4*hf
 mov ebx,x
 mov eax,y
 add eax,fpuioi //eax=y+t4*hf
  js LReturn2 //if(y+t4*hf<0) ret
 mov edi,[ecx]this.Bpl
 mov x,edi //x=Bpl
 imul edi,eax //edi=(y+t4*hf)*Bpl
 add edi,[ecx]this.img //edi=img+(y+t4*hf)*Bpl
 fmul st(0),st(2) //t4*hf*tf
 fist fpuioi
 sub ebx,fpuioi //ebx=x-t4*hf*tf
 fild word ptr [esi+6] //t3
 fmul st(0),st(2) //t3*hf
 fist fpuioi
 add eax,fpuioi //eax=y+t4*hf+t3*hf
 cmp eax,[ecx]this.lat
  jge LReturn1 //if(y+t4*hf+t3*hf>lat) ret
 fmul st(0),st(3) //t3*hf*tf
 fistp fpuioi
 mov edx,ebx
 sub edx,fpuioi //edx=x-t4*hf*tf-t3*hf*tf
  js LReturn2 //if(x-t4*hf*tf-t3*hf*tf<0) ret
 fild word ptr [esi+4] //t2
 fmul st(0),st(4) //t2*wf
 fistp fpuioi
 mov eax,fpuioi
 add eax,ebx
 cmp eax,[ecx]this.lng
  jg LReturn2 //if(x-t4*hf*tf+t2*wf>lng) ret
 shl ebx,2
 add edi,ebx
 fld st(0) //copy t4*hf*tf
 frndint
 fsub st(1),st(0)
 DISCARD_ST0
 fld1
 fdiv st(0),st(2) //1/hf
 fstp st(2) //replace hf with 1/hf
 fldz //alloc reg for {line step}
 movzx eax,word ptr[esi+6] //t3
 dec eax
 mov y,eax //y=t3-1 (line counter)
 add esi,10 //esi=&trace[5]
 mov eax,[ecx]this.color
 mov edx,edi //edx=line pointer
LNextLine:
  mov trace,esi //save esi
  movzx ebx,word ptr[esi] //edx=nr of packets
  add esi,2
  shr ebx,1
  jz LSkipLine //skip if line empty
  fldz //float cx=0
LNextPacket:
   fild word ptr[esi] //ST(0)=false packet
   add esi,2
   fmul st(0),st(6) //*=wf
   fadd st(1),st(0) //cx+=false packet
   DISCARD_ST0
   fist fpuioi //fpuioi=round(cx)
   mov edi,fpuioi
   shl edi,2
   add edi,edx
   fild word ptr[esi] //ST(0)=true packet
   add esi,2
   fmul st(0),st(6) //*=wf
   fadd st(1),st(0) //cx+=true packet
   fistp fpuioi //packet width
   mov ecx,fpuioi
   rep stosd //draw packet
   dec ebx
   jnz LNextPacket
  DISCARD_ST0 //cx
LSkipLine:
  //advance edi
  fld st(1) //copy tilt step
  fadd st(0),st(4) //tilt step+=tf
  fst st(2) //
  frndint
  fsub st(2),st(0) //{tilt step}=tilt step+tf-[tilt step+tf]
  fistp fpuioi
  mov ecx,fpuioi
  shl ecx,2
  sub edx,ecx
  add edx,x //edx+=this.Bpl
  //advance esi
  mov esi,trace //restore esi
  fadd st(0),st(2) // {line step}+=1/hf
  fld st(0) //copy {line step}
  frndint
  fsub st(1),st(0)
  fistp fpuioi
  mov ecx,fpuioi
  jecxz LStupidShit //because jecxz can jump only short (-/+128)
  sub y,ecx
  js LReturn1
LNextESI:
   movzx ebx,word ptr[esi]
   inc ebx
   shl ebx,1 //ebx=(nr packets+1)*2
   add esi,ebx
   loop LNextESI
LStupidShit:
  jmp LNextLine
LReturn1:
 DISCARD_ST0
LReturn2:
 DISCARD_ST0
 DISCARD_ST0
 DISCARD_ST0
 DISCARD_ST0
 }
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>> Management functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//blits to a DC .......................................................................
HDC Image::DCTex()
{
if(!img) return mdc;
HDC hrefdc;
BMINF bi;
hrefdc=GetDC(NULL);
if(!mdc) mdc=CreateCompatibleDC(hrefdc);
if(hbm) DeleteObject(hbm);
hbm=CreateCompatibleBitmap(hrefdc,lng,lat);
ReleaseDC(NULL,hrefdc);
if(!SelectObject(mdc,hbm))
 {
 WARN(__LINE__,"SelectObject() failed in Image::DCTex at line:");
 return NULL;
 }
if((pf&0xfff)!=0x888&&(pf&0xfff)!=0x555&&(pf&0xfff)!=0x565) Conv(0x32108888);
if(Bpl&3) Conv(0x32108888); Align(3);
InitBMPINF((PBMI)&bi,lng,-lat,ALIGN(bpp,7),pf);
SetDIBits(mdc,hbm,0,lat,img,(PBMI)&bi,DIB_RGB_COLORS);
stat|=IMG_DC;
stat&=~IMG_IDC;
return mdc;
}

//.......................................................................
void Image::Font(LPSTR name=NULL,int cellh=16,int atrib=0,DWORD chset=0)
 {
 if(hfont) DeleteObject(hfont);
 hfont=CreateFont(cellh,0,0,0,(atrib&0xf)*100,(atrib>>4)&1,(atrib>>8)&1,(atrib>>12)&1,
                  chset,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,
                  FF_DONTCARE|(atrib>>16?FIXED_PITCH:DEFAULT_PITCH),name);
 if(mdc)
  {
  SelectObject(mdc,hfont);
  GetTextMetrics(mdc,&tm);
  SetBkColor(mdc,0);
  SetBkMode(mdc,TRANSPARENT);
  }
 }
//shows individual bits....................................................................................
void Image::ShowBW(HDC hdc,int l=0,int u=0,int r=0,int d=0,RCT*cb=NULL)
{
RCT lr;
if(r<=l) r=lng*8;
else r-=l;
if(d<=u) d=lat;
else d-=u;
if(!cb)
 {
 lr(0,0,lng*8,lat);
 cb=&lr;
 }
if(imgB&&(stat&IMG_GREY))
 {
 GREYBMINF gi;
 InitBMPINF_BW((PBMI)&gi,lng*8,lat);
 //GreyPal((COLOR*)gi.pal,256,0xff);
 StretchDIBits(hdc,l,u,r,d,cb->l,cb->d+1,cb->r-cb->l,cb->u-cb->d,img,(PBMI)&gi,DIB_RGB_COLORS,SRCCOPY);
 //SetDIBitsToDevice(hdc,l,u,r,d,0,0,0,lat,img,(PBMI)&gi,DIB_RGB_COLORS);
 }
else
 BitBlt(hdc,l,u,r,d,hdc,0,0,BLACKNESS);
}

//blits to a DC .......................................................................
void Image::DrawDC(HDC hdc,int x=0,int y=0,int e=0,int f=0,int w=0,int h=0,int mir=-1,HWND hwnd=NULL)
{
if(w<=0||w>lng-e) w=lng-e;
if(h<=0||h>lat-f) h=lat-f;
if(hwnd) hdc=GetDC(hwnd);
if(img)
 {
 if(stat&IMG_GREY)
  {
  GREYBMINF gi;
  InitBMPINF((PBMI)&gi,lng,mir*lat,8,pf);
  GreyPal((COLOR*)gi.pal,256,0xff);
  SetDIBitsToDevice(hdc,x,y,w,h,e,f,0,lat,img,(PBMI)&gi,DIB_RGB_COLORS);
  }
 else
  {
  BMINF bi;
  if((pf&0xfff)!=0x888&&(pf&0xfff)!=0x555&&(pf&0xfff)!=0x565) Conv(0x32108888);
  if(Bpl&3) Conv(0x32108888); Align(3);
  InitBMPINF((PBMI)&bi,lng,mir*lat,ALIGN(bpp,7),pf);
  SetDIBitsToDevice(hdc,x,y,w,h,e,f,0,lat,img,(PBMI)&bi,DIB_RGB_COLORS);
  }
 } 
else
 BitBlt(hdc,x,y,w,h,NULL,0,0,DSTINVERT);
if(hwnd) ReleaseDC(hwnd,hdc);
}

//....................................................................................
void Image::ShowDC(HDC hdc,int l=0,int u=0,int r=0,int d=0,RCT*cb=NULL)
{
RCT lr;
if(r<=l) r=lng;
else r-=l;
if(d<=u) d=lat;
else d-=u;
if(!cb)
 {
 lr(0,0,lng,lat);
 cb=&lr;
 }
if(imgB)
 {
 if(stat&IMG_GREY)
  {
  GREYBMINF gi;
  InitBMPINF((PBMI)&gi,lng,lat,8,pf);
  GreyPal((COLOR*)gi.pal,256,0xff);
  StretchDIBits(hdc,l,u,r,d,cb->l,cb->d+1,cb->r-cb->l,cb->u-cb->d,img,(PBMI)&gi,DIB_RGB_COLORS,SRCCOPY);
  }
 else
  { 
  BMINF bi;
  if((pf&0xfff)!=0x888&&(pf&0xfff)!=0x555&&(pf&0xfff)!=0x565) Conv(0x32108888);
  if(Bpl&3) Conv(0x32108888); Align(3);
  InitBMPINF((PBMI)&bi,lng,lat,ALIGN(bpp,7),pf);
  StretchDIBits(hdc,l,u,r,d,cb->l,cb->d+1,cb->r-cb->l,cb->u-cb->d,img,(PBMI)&bi,DIB_RGB_COLORS,SRCCOPY);
  }
 }
else
 BitBlt(hdc,l,u,r,d,hdc,0,0,BLACKNESS);
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Extensions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#ifdef V_IDIRECTDRAW
#pragma message("Image DirectDraw7 extensions")
 IDirectDrawSurface7* Image::DDTex()
 {
 if(!img) return dds; //no data, no change
 DELCOM(dds);
 if(Bpl&3) Conv(0,TRUNC(lng,3),lat);
 dds=MakeDDS7(lng,lat,pf,Bpl,0,imgB);
 stat|=IMG_IDD|IMG_DD;
 return dds;
 }

 void Image::DrawDD(IDirectDrawSurface7*ddsurf,int x=0,int y=0,int e=0,int f=0,int w=0,int h=0)
 {
 if(!imgB) return;
 DDSURFACEDESC2 sd;
 sd.dwSize=sizeof(DDSURFACEDESC2);
 if(w<=0||w>lng-e) w=lng-e;
 if(h<=0||h>lat-f) h=lat-f;
 erret=ddsurf->Lock(NULL,&sd,DDLOCK_NOSYSLOCK|DDLOCK_WRITEONLY,NULL);
 if(erret!=DD_OK) return;
 if((signed)(y*sd.lPitch+x*(sd.ddpfPixelFormat.dwRGBBitCount>>3))<0) //TODO: should also check against max address
  return; //can't blit outside locked area
 BLTRGB lblt;
 lblt.Copy((BYTE*)sd.lpSurface+y*sd.lPitch+x*(sd.ddpfPixelFormat.dwRGBBitCount>>3),sd.lPitch,DDPFtoPF(sd.ddpfPixelFormat),imgB+f*Bpl+e*Bpp,Bpl,pf,w,h);
 lblt.DoIt(&lblt);
 ddsbbuf->Unlock(NULL);
 }

 void Image::ShowDD(IDirectDrawSurface7*ddsurf,int l=0,int u=0,int r=0,int d=0,int grey=0)
 {
 if(!imgB) return;
 DDSURFACEDESC2 sd;
 sd.dwSize=sizeof(DDSURFACEDESC2);
 r-=l;
 d-=u;
 if(r<=0) r=lng;
 if(d<=0) d=lat;
 erret=ddsurf->Lock(NULL,&sd,DDLOCK_NOSYSLOCK|DDLOCK_WRITEONLY,NULL);
 if(erret!=DD_OK) return;
 if((signed)(u*sd.lPitch+l*(sd.ddpfPixelFormat.dwRGBBitCount>>3))<0) //TODO: should also check against max address
  return; //can't blit outside locked area
 BLTRGB lblt;
 lblt.Zoom((BYTE*)sd.lpSurface+u*sd.lPitch+l*(sd.ddpfPixelFormat.dwRGBBitCount>>3),sd.lPitch,DDPFtoPF(sd.ddpfPixelFormat),r,d,img,Bpl,pf,lng,lat);
 lblt.DoIt(&lblt);
 ddsbbuf->Unlock(NULL);
 }
#endif

#ifdef V_IDIRECTXG8
#pragma message("Image DXGraphics8 extensions")
 IDirect3DTexture8* Image::DXTex(IDirect3DTexture8*d3dtex=NULL,DWORD pixform=0x32108888,int mips=1)
 {
 if(!img) return d3dtex; //no data, no change
 D3DSURFACE_DESC dsd;
 D3DLOCKED_RECT lck,llck;
 CCONV lcc;
 if(!pixform) pixform=pf;
 if(!mips) mips=MSBit(lng<lat?lng:lat)-1; //generate down to 4x4
 DELCOM(d3dtex);
 if(dxdev->CreateTexture(CutDPow2(lng),CutDPow2(lat),mips,0,PFtoD3DF(pixform),D3DPOOL_MANAGED,&d3dtex)!=D3D_OK)
  return NULL; //unable to create texture object
 d3dtex->GetLevelDesc(0,&dsd);
 if(d3dtex->LockRect(0,&lck,NULL,0)!=D3D_OK) return d3dtex;
 lcc.Tf(pf,pixform);
 lcc.Rpr(lck.pBits,lck.Pitch,dsd.Width,dsd.Height,img,Bpl,lng,lat);
 lcc.Ed(pixform);
 for(mips=1;mips<d3dtex->GetLevelCount();mips++)
  {
  llck=lck;
  if(d3dtex->LockRect(mips,&lck,NULL,0)!=D3D_OK) return d3dtex;
  lcc.Mip(lck.pBits,llck.pBits,llck.Pitch,dsd.Width,dsd.Height);
  d3dtex->UnlockRect(mips-1);
  d3dtex->GetLevelDesc(mips,&dsd);
  }
 d3dtex->UnlockRect(mips-1);
 return d3dtex;
 }
#endif

#ifdef V_IOPENGL
#pragma message("Image OpenGL extensions")
 GLTEX Image::GLTex(GLTEX ogltex=0,DWORD pixform=0x32108888,int mips=1)
 {
 if(!img) return ogltex; //no data, no change
 CCONV lcc;
 BYTE *locbuf;
 int w,h,texpf,mip;
 w=CutDPow2(lng);
 h=CutDPow2(lat);
 texpf=PFtoGLF1(pixform);
 if(PF_nrch(pf)==4) pixform=0x30128888;
 else if(PF_nrch(pf)==3) pixform=0x30120888;
 else pixform=0x30128000;
 lcc.Tf(pf,pixform);
 locbuf=(BYTE*)malloc(w*h*lcc.DBpp+4);
 lcc.Rpr(locbuf,w*lcc.DBpp,w,h,img,Bpl,lng,lat);
 lcc.Ed(pixform);
 pixform=PFtoGLF2(pixform);
 glDeleteTextures(1,&ogltex);
 glGenTextures(1,&ogltex);
 glBindTexture(GL_TEXTURE_2D,ogltex);
 glTexImage2D(GL_TEXTURE_2D,0,texpf,w,h,0,pixform,GL_UNSIGNED_BYTE,locbuf);
 mip=MSBit(w>h?w:h)+1;
 if(!mips||mips>mip) mips=mip; //generate down to 1x1
 for(mip=1;mip<mips;mip++)
  {
  if(w>1&&h>1) lcc.Mip(locbuf,locbuf,w*lcc.Bpi,w,h);
  if(w>1) w>>=1;
  if(h>1) h>>=1;
  glTexImage2D(GL_TEXTURE_2D,mip,texpf,w,h,0,pixform,GL_UNSIGNED_BYTE,locbuf);
  }
 glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,mips==1?GL_NEAREST:GL_LINEAR_MIPMAP_NEAREST);
 glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 free(locbuf);
 return ogltex;
 }
#endif

//adjust window so image is shrinked only if it doesn't fit on screen also keeping aspect ratio ------------
void Image::FitWindow(HWND hwnd,int hb=0,int vb=0)
{
int ew,eh;
double f;
if(!img||!lng||!lat) return;
SystemParametersInfo(SPI_GETWORKAREA,0,&mwarea,0);
GetWindowBorders(hwnd,ew,eh);
ew+=hb;
eh+=vb;
f=MIN3(1.,(double)(mwarea.right-mwarea.left-ew)/lng,(double)(mwarea.bottom-mwarea.top-eh)/lat);
AdjustClientRect(hwnd,lng*f+hb,lat*f+vb,(mwarea.right+mwarea.left-ew-(lng*f))/2,(mwarea.bottom+mwarea.top-eh-(lat*f))/2);
}

//..............................................................................................................................
void Image::HFont(HFONT hfont,char*str)
{
SIZE csz;
HDC hldc=GetDC(NULL);
SelectObject(hldc,hfont);
GetTextExtentPoint32(hldc,str,sl(str),&csz);
ReleaseDC(NULL,hldc);
Init(csz.cx,csz.cy,0x32108888,IMG_IDC);
Clear();
SelectObject(mdc,hfont);
SetTextColor(mdc,0xffffff);
SetBkColor(mdc,0);
TextOut(mdc,0,0,str,sl(str));
}

//(magn si angle sunt mai mici cu 1 in toate cele 4 directii)..............................................................................................................................
void Image::Sobel(int**magn,float**angle)
{
float*a;
int*m;
int x,y,dx,dy;
NAT p,yy=3,el=0,l=lng<<2;
m=(int*)ALLOC((lng-2)*(lat-2)*sizeof(float));
a=(float*)ALLOC((lng-2)*(lat-2)*sizeof(float));
for(y=1;y<lat-1;y++)
 {
 yy+=l;
 p=yy;
 for(x=1;x<lng-1;x++)
  {
  p+=4;
  dx=((int)imgB[p+4]<<1)+(int)imgB[p+4-l]+(int)imgB[p+4+l]-((int)imgB[p-4]<<1)-(int)imgB[p-4-l]-(int)imgB[p-4+l];
  dy=((int)imgB[p+l]<<1)+(int)imgB[p+l-4]+(int)imgB[p+l+4]-((int)imgB[p-l]<<1)-(int)imgB[p-l-4]-(int)imgB[p-l+4];
  m[el]=SQRT((double)dx*dx+dy*dy);
  //edge direction is actually perpendicular to tan so you need to add 90  (this also transforms [-90;90] to [0;180])
  //also the circle is vertically flipped to resemble color wheel (the '-' for dy/dx)
  a[el]=((dx!=0)?GRD(atan((double)(-dy)/dx)):-90)+90;
  el++;
  }
 } 
yy=0;
el=0;
for(y=1;y<lat-1;y++)
 {
 yy+=lng;
 p=yy;
 for(x=1;x<lng-1;x++)
  {
  p++;
  HSLtoRGB(a[el]*240/360,m[el]*240/1024,120,imgC[p].R,imgC[p].G,imgC[p].B);
  imgC[p].A=CLAMP(m[el]/4,0,255);
  el++;
  }
 } 
if(magn)
 {
 FREE(*magn);
 *magn=m;
 }
else
 FREE(m);
if(angle)
 {
 FREE(*angle);
 *angle=a;
 }
else
 FREE(a);
}

//..............................................................................................................................
void Image::Deriv(int d)
{
int x,y;
NAT p;
NAT hist[512];
float prag=16;
ZeroMemory(hist,512*sizeof(NAT));
if(!d)
 {
 for(y=0;y<lat;y++)
  {
  p=y*lng;
  imgC[p].R=imgC[p].G=imgC[p].B=0;
  for(x=1;x<lng;x++)
   {
   p=y*lng+x;
   d=(int)imgC[p].A-(int)imgC[p-1].A;
   hist[(d+255)&0x1ff]++;
   imgC[p].R=imgC[p].G=imgC[p].B=0;
   if(ABS(d)<prag)
    imgC[p].B=ABS(d);
   else if(d>0)
    imgC[p].G=d;
   else
    imgC[p].R=-d;
   }
  } 
 }
else
 {
 for(y=1;y<lat;y++)
  {
  for(x=0;x<lng;x++)
   {
   p=y*lng+x;
   d=(int)imgC[p].A-(int)imgC[p-lng].A;
   hist[(d+255)&0x1ff]++;
   imgC[p].R=imgC[p].G=imgC[p].B=0;
   if(ABS(d)<prag)
    imgC[p].B=ABS(d);
   else if(d>0)
    imgC[p].G=d;
   else
    imgC[p].R=-d;
   }
  } 
 for(x=0;x<lng;x++)
  {
  imgC[x].R=imgC[x].G=imgC[x].B=0;
  }
 }
//color=0;
//at(0,0);
//rect(lng,lat);
//AdjustClientRect(hdbgwnd,512,512,mww+8);
//ShowArray(hist,512,hdbgwnd,0xffff00);
}

//..............................................................................................................................
void Image::CloneCh(Image*psrc,int l=0,int u=0,int r=0,int d=0,int ch=3)
{
if(l>psrc->lng) l=0;
if(r>psrc->lng) r=psrc->lng;
if(u>psrc->lat) u=0;
if(d>psrc->lat) d=psrc->lat;
if(r<=0) r=psrc->lng;
if(d<=0) d=psrc->lat;
if(ch>=psrc->Bpp) ch=0;
if(!psrc->img) return;
Init(r-l,d-u,0x32100008,IMG_GREY,NULL,0,0,"channel");
int x,y,s,a;
for(y=0;y<lat;y++)
 {
 s=ch+(u+y)*psrc->Bpl+l*psrc->Bpp;
 a=y*Bpl;
 for(x=0;x<lng;x++)
  {
  //imgB[a]=x*255/lng;
  imgB[a]=psrc->imgB[s];
  s+=psrc->Bpp;
  a+=Bpp;
  }
 }
}

//..............................................................................................................................
void Image::DerivH(Image*psrc,int ch=3)
{
Init(psrc->lng,psrc->lat,0x32100008,IMG_GREY,NULL,0,0,"derivH");
int x,y,s,d;
for(y=0;y<lat;y++)
 {
 s=ch+y*psrc->Bpl;
 d=y*Bpl;
 imgB[d]=128;
 for(x=1;x<lng;x++)
  {
  d+=Bpp;
  imgB[d]=128-(psrc->imgB[s]>>1);
  s+=psrc->Bpp;
  imgB[d]+=psrc->imgB[s]>>1;
  }
 }
}

//..............................................................................................................................
void Image::DerivV(Image*psrc,int ch=3)
{
Init(psrc->lng,psrc->lat,0x32100008,IMG_GREY,NULL,0,0,"derivV");
int x,y,s,d;
FillMemory(imgB,Bpl,128);
for(y=1;y<lat;y++)
 {
 s=ch+y*psrc->Bpl;
 d=y*Bpl;
 for(x=0;x<lng;x++)
  {
  imgB[d]=(((int)psrc->imgB[s]-psrc->imgB[s-psrc->Bpl])>>1)+128;
  d+=Bpp;
  s+=psrc->Bpp;
  }
 }
}

