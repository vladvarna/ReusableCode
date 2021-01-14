#ifndef V_WINBMP
#define V_WINBMP

#include <rgb.cpp>
#include <twain.cpp>
#include <olectl.h>
#ifndef V_NOGDIPLUS
#include <atlstr.h>
#include <atlimage.h>
#endif

//Packed DIB
class BMPimage
{
public:
 BITMAPINFO *inf;//header + palette + data (always a packed DIB)
 DWORD pf;    //(STD) Open 0x0888-24,0x8888 for 8,4,1 |Save 0x0888-24,0x0332-8,0x1111-4,0x1000-1
 int lng,lat; //(STD) image width and height
 int pmw,pmh; //(STD) resolution in pixels per meter
 int bpp,Bpl; //(STD) bits/pixel,B/line
 int ppl;     //pixels/line
 int palsz;   //data offset in file, palette size in bytes
 int totsz;   //total size header + palette + data
 int lind;    //next line index
 char*line;   //curent line
 char*data;   //pointer to data (also lat-1 line)

 BMPimage();
 void Free();
 void Set(int,int,DWORD,int,int,BYTE*);
 void Pal(BYTE *pall);
 int FromMem(BITMAPINFO*,char*,int); //init from memory
#ifndef V_NOGDIPLUS
 void FromCImage(CImage*);
#endif
 FAIL FromGDI(HGDIOBJ,int,int,int,int); //inits from GDI objects
 int FromICO(HICON); //inits from icon or cursor objects
 int Open(LPSTR); //(STD) open/close
 int Load(LPSTR); //external load: jpg,gif,emf,wmf,ico
 int Acquire(int); //TWAIN
 int Paste(); //load from clipboard
 int Clipboard(int); //copy to clipboard 
 int Save(LPSTR); //write header and palette (inf)
 DWORD CanSave(DWORD,int); //(STD) closest format that can be saved or 0 if can't save
 char*Seek(NAT); //(STD) seeks a line
 void GetF(BYTE *colbuf,NAT lcnt=1,NAT pitch=0,DWORD pixform=0x32100888); //(STD)
 void PutF(BYTE *colbuf,NAT lcnt=1,NAT pitch=0,DWORD pixform=0x32108888); //for mono images alpha is recommended
 HBITMAP HBitmap(HDC); //must delete the handler returned
 void ShowDC(HDC hdc,int l=0,int u=0,int r=0,int d=0);
 ~BMPimage();
};
/*
 Supports opening as packed DIB
 Saving in 1,4,8 bpp with palette and 16(1:5:5:5),24,32 bpp as true color
*/

//--------------------------------------------------------------------------------------------------------------
BOOL BMPwritePAL(LPSTR path,NAT ind=0,COLORREF nc=0)
{
IOSFile iof;
if(erret=iof.open(path,FU_RW))
 {
 printbox("Couldn't access: %s",iof.filepath);
 return 0;
 }
iof.amask=0;
iof.setfp(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+(ind*sizeof(RGBQUAD)));
iof.write(&nc,sizeof(RGBQUAD));
//printbox("%x",nc);
return 1;
}

//*****************************************************************************************

BMPimage::BMPimage()
{
ZeroMemory(this,sizeof(BMPimage));
}

BMPimage::~BMPimage()
{
Free();
}

//.............................................................................................
void BMPimage::Free()
{
if(inf) free(inf);
inf=NULL;
}

//.............................................................................................
int BMPimage::FromMem(BITMAPINFO*pdib,char*pdata=NULL,int copy=0)
{
if(!pdib) return 2; //memory error
lng=pdib->bmiHeader.biWidth;
lat=ABS(pdib->bmiHeader.biHeight); //TODO: test with modulo
pmw=pdib->bmiHeader.biXPelsPerMeter;
pmh=pdib->bmiHeader.biYPelsPerMeter;
if(!pmw) pmw=scrpmw;
if(!pmh) pmh=scrpmh;
bpp=pdib->bmiHeader.biBitCount;
if(bpp==24)
 pf=0x32100888;
else if(bpp==16)
 pf=0x32101555;
else //32 and <=8
 pf=0x32108888;
ppl=ALIGN(lng,32/bpp-1);
Bpl=ALIGN((ppl*bpp)>>3,3);
if(bpp>8)
 palsz=pdib->bmiHeader.biCompression==BI_BITFIELDS?12:0;
else
 palsz=(1<<bpp)<<2; //2^bpp*4
totsz=sizeof(BITMAPINFOHEADER)+palsz+Bpl*lat; //recalc total size
if(inf) free(inf);
if(pdata) //explicit pdata involves copying
 {
 inf=(BITMAPINFO*)malloc(totsz+4);
 CopyMemory(inf,pdib,sizeof(BITMAPINFOHEADER)+palsz);
 CopyMemory((char*)inf+sizeof(BITMAPINFOHEADER)+palsz,pdata,Bpl*lat);
 }
else if(copy==1)
 {
 inf=(BITMAPINFO*)malloc(totsz+4);
 CopyMemory(inf,pdib,sizeof(BITMAPINFOHEADER));
 }
else if(copy==2)
 {
 inf=(BITMAPINFO*)malloc(totsz+4);
 CopyMemory(inf,pdib,sizeof(BITMAPINFOHEADER)+palsz);
 }
else if(copy==3)
 {
 inf=(BITMAPINFO*)malloc(totsz+4);
 CopyMemory(inf,pdib,totsz);
 }
else
 inf=pdib;
data=(char*)inf+sizeof(BITMAPINFOHEADER)+palsz;
Seek(0);
if(inf->bmiHeader.biCompression!=BI_RGB&&inf->bmiHeader.biCompression!=BI_BITFIELDS) return 4; //RLE compresion not suported
if(inf->bmiHeader.biSize!=sizeof(BITMAPINFOHEADER)) return 3; //old style BMP not suported
return 0;//Ok
}

//.............................................................................................
int BMPimage::Open(LPSTR filename)
{
FILE *Fdib;  //file handle
BITMAPFILEHEADER head;
BITMAPINFO*pbi;
Fdib=FOPEN(filename,"rb");
if(!Fdib) return 1; //can't open file
fread(&head,sizeof(BITMAPFILEHEADER),1,Fdib);
if(head.bfType!=0x4d42) 
 {
 fclose(Fdib);
 return 2; //not a BMP
 }
fseek(Fdib,0,SEEK_END);
head.bfSize=ftell(Fdib)-sizeof(BITMAPFILEHEADER);
pbi=(BITMAPINFO*)malloc(head.bfSize+4);
fseek(Fdib,sizeof(BITMAPFILEHEADER),0);
fread(pbi,head.bfSize,1,Fdib);
fclose(Fdib);
return FromMem(pbi,NULL,0);
}

//.............................................................................................
FAIL BMPimage::FromGDI(HGDIOBJ hobj,int l=0,int u=0,int r=0,int d=0)
{
HDC hlcdc;
HGDIOBJ holdobj;
HBITMAP hlbmp;
BITMAPINFO bi;
int del=0;
bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
bi.bmiHeader.biBitCount=0;
switch(GetObjectType(hobj))
 {
 //capture rect from DC
 case OBJ_DC:
  hlcdc=CreateCompatibleDC((HDC)hobj);
  hlbmp=CreateCompatibleBitmap((HDC)hobj,r-l,d-u);
  if(!(holdobj=SelectObject(hlcdc,hlbmp)))
   {
   DeleteObject(hlbmp);
   DeleteDC(hlcdc);
   return 2; //couldn't select
   }
  DeleteObject(holdobj);
  BitBlt(hlcdc,0,0,r-l,d-u,(HDC)hobj,l,u,SRCCOPY);
  GetDIBits(hlcdc,hlbmp,0,0,NULL,&bi,DIB_RGB_COLORS);
  FromMem(&bi,NULL,1);
  GetDIBits(hlcdc,hlbmp,0,bi.bmiHeader.biHeight,data,inf,DIB_RGB_COLORS);
  DeleteObject(hlbmp);
  DeleteDC(hlcdc);
  break;
 //hbitmap not selected in a DC
 case OBJ_BITMAP:
  hlbmp=(HBITMAP)hobj;
  hlcdc=CreateCompatibleDC(NULL);
  if(!(holdobj=SelectObject(hlcdc,hlbmp)))
   {
   DeleteDC(hlcdc);
   return 2; //couldn't select
   }
  DeleteObject(holdobj);
  GetDIBits(hlcdc,hlbmp,0,0,NULL,&bi,DIB_RGB_COLORS);
  FromMem(&bi,NULL,1);
  GetDIBits(hlcdc,hlbmp,0,bi.bmiHeader.biHeight,data,inf,DIB_RGB_COLORS);
  DeleteDC(hlcdc);
  break;
 //compatible DC with hbitmap selected into it
 case OBJ_MEMDC:
  hlcdc=(HDC)hobj;
  hlbmp=(HBITMAP)GetCurrentObject(hlcdc,OBJ_BITMAP);
  if(!hlbmp) return 2; //couldn't get hbitmap
  GetDIBits(hlcdc,hlbmp,0,0,NULL,&bi,DIB_RGB_COLORS);
  FromMem(&bi,NULL,1);
  GetDIBits(hlcdc,hlbmp,0,bi.bmiHeader.biHeight,data,inf,DIB_RGB_COLORS);
  break;
 //win16 meta file handler
 case OBJ_METAFILE:
  hobj=(HMETAFILE)WMFtoEMF((HMETAFILE)hobj);
  del=1;
 //enh meta file handler
 case OBJ_ENHMETAFILE:
  ENHMETAHEADER emh;
  GetEnhMetaFileHeader((HENHMETAFILE)hobj,sizeof(ENHMETAHEADER),&emh);
  hlcdc=GetDC(NULL);
  hlbmp=CreateCompatibleBitmap(hlcdc,emh.rclBounds.right-emh.rclBounds.left,emh.rclBounds.bottom-emh.rclBounds.top);
  ReleaseDC(NULL,hlcdc);
  hlcdc=CreateCompatibleDC(NULL);
  if(!(holdobj=SelectObject(hlcdc,hlbmp)))
   {
   DeleteObject(hlbmp);
   DeleteDC(hlcdc);
   return 2; //couldn't select
   }
  DeleteObject(holdobj);
  PlayEnhMetaFile(hlcdc,(HENHMETAFILE)hobj,(RECT*)&emh.rclBounds);
  GetDIBits(hlcdc,hlbmp,0,0,NULL,&bi,DIB_RGB_COLORS);
  bi.bmiHeader.biXPelsPerMeter=emh.szlDevice.cx*1000/emh.szlMillimeters.cx; //get X resolution
  bi.bmiHeader.biYPelsPerMeter=emh.szlDevice.cy*1000/emh.szlMillimeters.cy; //get Y resolution
  FromMem(&bi,NULL,1);
  GetDIBits(hlcdc,hlbmp,0,bi.bmiHeader.biHeight,data,inf,DIB_RGB_COLORS);
  DeleteObject(hlbmp);
  DeleteDC(hlcdc);
  if(del) DeleteEnhMetaFile((HENHMETAFILE)hobj);
  break;//Ok
 //case OBJ_ENHMETADC:
 //case OBJ_METADC:
 //TODO?: use CloseEMF() to obtain a handler then init from handler
 default:
  return 1; //unknown object
 }
if(inf->bmiHeader.biCompression==BI_BITFIELDS) //adjust pf to match masks
 {
 pf=masks_PF(*(DWORD*)inf->bmiColors,*((DWORD*)inf->bmiColors+1),*((DWORD*)inf->bmiColors+2));
 pf=PF_align(pf,bpp);
 }
return 0;//Ok
}

//.............................................................................................
int BMPimage::FromICO(HICON hico)
{
ICONINFO ii;
GetIconInfo(hico,&ii);
return FromGDI(ii.hbmColor);
}

#ifndef V_NOGDIPLUS
//.............................................................................................
void BMPimage::FromCImage(CImage*pci)
{
HDC hldc;
int w,h;
hldc=pci->GetDC();
w=pci->GetWidth();
h=pci->GetHeight();
erret=FromGDI(hldc,0,0,w,h);
pci->ReleaseDC();
}
#endif

//.............................................................................................
int BMPimage::Load(LPSTR filename)
{
IPicture*pic=NULL;
HANDLE hobj=NULL;
WCHAR fnw[PATHSZ];
short type;
int retv;
REGi=MetaPath(filename,strbuf,REGi); //doesn't look for more than 1
filename=strbuf;
if(countch(':',filename,PATHSZ)) //has absolute path
 A_W(fnw,filename);
else //has relative or no path
 {
 REGu=GetCurrentDirectory(200,REGs);
 REGu+=sc(REGs+REGu,"\\",1);
 A_W(fnw,REGs);
 A_W(fnw+REGu,filename);
 }
erret=OleLoadPicturePath(fnw,NULL,0,C_TRANSPARENT,IID_IPicture,(void**)&pic);
if(!pic||R_ERR) return 1; //file open error
pic->get_Handle((OLE_HANDLE*)&hobj);
if(!hobj)
 {
 DELCOM(pic);
 return 2; //handle retriving failed
 }
pic->get_Type(&type);
if(type==1) //bitmap
 {
 retv=FromGDI(hobj);
 DeleteObject(hobj);
 }
else if(type==3) //icon
 {
 retv=FromICO((HICON)hobj);
 DestroyIcon((HICON)hobj);
 }
else if(type==4) //enh metafile
 {
 retv=FromGDI(hobj);
 DeleteEnhMetaFile((HENHMETAFILE)hobj);
 }
else if(type==2) //metafile
 {
 retv=FromGDI(hobj);
 DeleteMetaFile((HMETAFILE)hobj);
 }
else //unknown type
 retv=3;
DELCOM(pic);
return retv;
}

//.............................................................................................
int BMPimage::Paste()
{
HANDLE hgmem;
BITMAPINFO*pbi;
int retv=0;
if(!OpenClipboard(hmwnd)) return 1;
hgmem=GetClipboardData(CF_DIB);
if(hgmem)
 {
 pbi=(BITMAPINFO*)GlobalLock(hgmem);
 retv=FromMem(pbi,NULL,3);
 GlobalUnlock(hgmem);
 }
else
 retv=2;
CloseClipboard();
return retv;
}

//.............................................................................................
int BMPimage::Clipboard(int mod=0)
{
HANDLE hdib;
HDC hldc;
BITMAPINFO*pbi=NULL;
if(!OpenClipboard(hmwnd)) return 1;
if(mod==1)
 {
 hldc=GetDC(NULL);
 hdib=HBitmap(hldc);
 ReleaseDC(NULL,hldc);
 if(hdib) SetClipboardData(CF_BITMAP,hdib);
 }
else
 {
 hdib=GlobalAlloc(GMEM_MOVEABLE,totsz);
 if(hdib)
  pbi=(BITMAPINFO*)GlobalLock(hdib);
 if(pbi)
  {
  CopyMemory(pbi,inf,totsz);
  GlobalUnlock(hdib);
  if(!SetClipboardData(CF_DIB,hdib))
   GlobalFree(hdib);
  }
 else
  GlobalFree(hdib);
 }
CloseClipboard();
return 0;
}

#ifdef V_TWAIN
//.............................................................................................
int BMPimage::Acquire(int nri=1) //must have called InitTWAIN() before
{
HGLOBAL hdib;
BITMAPINFO*pbi;
if(!TWAcquire(&hdib,1,nri)) return 1; //canceled or error in acquisition
pbi=(BITMAPINFO*)GlobalLock(hdib);
FromMem(pbi,NULL,3);
GlobalUnlock(hdib);
GlobalFree(hdib);
return 0; //Ok
}
#endif

//.............................................................................................
void BMPimage::Set(int width=0,int height=0,DWORD pixform=0x32100888,int xres=0,int yres=0,BYTE *pall=NULL)
{
if(width>0) lng=width;
if(height>0) lat=height;
if(pixform) pf=pixform;
if(xres) pmw=xres;
if(yres) pmh=yres;
if(!pmw) pmw=scrpmw;
if(!pmh) pmh=scrpmh;
bpp=PF_bpix(pf);
ppl=ALIGN(lng,32/bpp-1);
Bpl=ALIGN((ppl*bpp)>>3,3);
palsz=(bpp>8?0:((1<<bpp)<<2));
totsz=sizeof(BITMAPINFOHEADER)+palsz+Bpl*lat; //recalc total size
inf=(BITMAPINFO*)REALLOC(inf,totsz+4);
data=(char*)inf+sizeof(BITMAPINFOHEADER)+palsz;
inf->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
inf->bmiHeader.biWidth=lng;
inf->bmiHeader.biHeight=lat;
inf->bmiHeader.biPlanes=1;
inf->bmiHeader.biBitCount=bpp;
inf->bmiHeader.biCompression=BI_RGB;
inf->bmiHeader.biSizeImage=0;
inf->bmiHeader.biXPelsPerMeter=pmw;
inf->bmiHeader.biYPelsPerMeter=pmh;
inf->bmiHeader.biClrUsed=0;
inf->bmiHeader.biClrImportant=0;
Pal(pall);
Seek(0);
}

//.............................................................................................
void BMPimage::Pal(BYTE *pall=NULL)
{
if(!palsz) return;
if(pall) CopyMemory(inf->bmiColors,pall,palsz);
else PF_palette((BYTE*)inf->bmiColors,pf,0x32108888);
}

//.............................................................................................
int BMPimage::Save(LPSTR filename)
{
FILE *Fdib;  //file handle
BITMAPFILEHEADER head;
if(!inf) return 2; //no info to write
Fdib=FOPEN(filename,"wb");
if(!Fdib) return 1; //no file to write to
head.bfType=0x4d42; //"BM"
head.bfSize=totsz+sizeof(BITMAPFILEHEADER);
head.bfReserved1=0;
head.bfReserved2=0;
head.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+palsz;
fwrite(&head,sizeof(head),1,Fdib);
fwrite(inf,totsz,1,Fdib);
fclose(Fdib);
return 0;//Ok
}

//.............................................................................................
DWORD BMPimage::CanSave(DWORD pixform=0,int std=0)
{
if(std) return 0x32100888; //only fully supported mode
switch(PF_bpix(pixform))
 {
 case 32:
  pixform=0x8888;
  break;
 case 16:
  pixform=0x1555;
  break;
 case 8:
  pixform=0x0332;
  break;
 case 4:
  pixform=0x0121;
  break;
 case 1:
  pixform=0x0001;
  break;
 case 24:
 default:
  pixform=0x0888;
 }
return 0x32100000|pixform;
}

//.............................................................................................
inline char*BMPimage::Seek(NAT li=0)
{
lind=li%lat;
line=data+(lat-1-lind)*Bpl;
return line;
}

//.............................................................................................
void BMPimage::GetF(BYTE *colbuf,NAT lcnt,NAT pitch,DWORD pixform)
{
if(!inf) return;
CCONV lcc;
BYTE *aux;
NAT el,masc;
if(!lcnt) lcnt=lat;
if(!pixform) pixform=0x32100888;
lcc.Tf(pf,pixform,bpp);
if(!pitch) pitch=ppl*lcc.DBpp;
while(lcnt>0)
 {
 aux=colbuf;
 if(bpp>8) //true/high color
  lcc.Blt(colbuf,line,lng);
 else if(bpp==8) //256
  lcc.Ptl(colbuf,inf->bmiColors,line,lng);
 else if(bpp==4) //16
  {
  for(el=0;el<Bpl;el++)
   {
   masc=(line[el]&0xf0)>>4;
   lcc.BGRA(colbuf,inf->bmiColors[masc].rgbBlue,inf->bmiColors[masc].rgbGreen,inf->bmiColors[masc].rgbRed,0xff);
   colbuf+=lcc.DBpp;
   masc=line[el]&0xf;
   lcc.BGRA(colbuf,inf->bmiColors[masc].rgbBlue,inf->bmiColors[masc].rgbGreen,inf->bmiColors[masc].rgbRed,0xff);
   colbuf+=lcc.DBpp;
   }
  }
 else if(bpp==1) //mono
  {
  masc=0x80;
  el=0;
  while(el<Bpl)
   {
   if(line[el]&masc)
    lcc.BGRA(colbuf,inf->bmiColors[1].rgbBlue,inf->bmiColors[1].rgbGreen,inf->bmiColors[1].rgbRed,0xff);
   else
    lcc.BGRA(colbuf,inf->bmiColors[0].rgbBlue,inf->bmiColors[0].rgbGreen,inf->bmiColors[0].rgbRed,0xff);
   colbuf+=lcc.DBpp;
   if(masc==0x1) 
    {
    masc=0x80;
    el++;
    }
   else masc>>=1;
   }
  }
 lind++;
 if(lind>=lat)
  {
  line=(char*)inf+sizeof(BITMAPINFOHEADER)+palsz;
  lind=0;
  }
 else
  line-=Bpl; //next line
 colbuf=aux+pitch;
 lcnt--;
 }
}

//.............................................................................................
void BMPimage::PutF(BYTE *colbuf,NAT lcnt,NAT pitch,DWORD pixform)
{
if(!inf) return;
CCONV lcc;
BYTE *aux;
NAT el,masc;
if(!lcnt) lcnt=lat;
if(!pitch) pitch=Bpl;
if(!pixform) pixform=0x32100888;
lcc.Tf(pixform,pf);
while(lcnt>0)
 {
 aux=colbuf;
 if(bpp>=8) //true/high/256 color
  lcc.Blt(line,colbuf,lng);
 else if(bpp==4) //16
  {
  for(el=0;el<Bpl;el++)
   {
   lcc.BGRA(&masc,colbuf[0],colbuf[1],colbuf[2],colbuf[3]);
   colbuf+=lcc.SBpp;
   line[el]=masc<<4;
   lcc.BGRA(&masc,colbuf[0],colbuf[1],colbuf[2],colbuf[3]);
   colbuf+=lcc.SBpp;
   line[el]|=masc;
   }
  }
 else if(bpp==1) //mono
  {
  el=0;
  masc=0x80;
  line[el]=0;
  while(el<Bpl)
   {
   lcc.BGRA(&pixform,colbuf[0],colbuf[1],colbuf[2],colbuf[3]);
   if(pixform)
    line[el]|=masc;
   colbuf+=lcc.SBpp;
   if(masc==0x1)
    {
    masc=0x80;
    el++;
    line[el]=0;
    }
   else masc>>=1;
   }
  }
 lind++;
 if(lind>=lat)
  {
  line=(char*)inf+sizeof(BITMAPINFOHEADER)+palsz;
  lind=0;
  }
 else
  line-=Bpl; //next line
 colbuf=aux+pitch;
 lcnt--;
 }
}

//(hrefdc MUST be valid).............................................................................................
inline HBITMAP BMPimage::HBitmap(HDC hrefdc)
{
return CreateDIBitmap(hrefdc,&inf->bmiHeader,CBM_INIT,data,inf,DIB_RGB_COLORS);
}

//.............................................................................................
inline void BMPimage::ShowDC(HDC hdc,int l,int u,int r,int d)
{
if(r<=l) r=lng;
else r-=l-1;
if(d<=u) d=lat;
else d-=u-1;
//SetStretchBltMode(hdc,COLORONCOLOR);
if(inf)
 StretchDIBits(hdc,l,u,r,d,0,0,lng,lat,data,inf,DIB_RGB_COLORS,SRCCOPY);
else
 BitBlt(hdc,l,u,r,d,hdc,0,0,BLACKNESS);
}

#endif