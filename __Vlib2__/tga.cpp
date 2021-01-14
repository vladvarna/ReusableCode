#ifndef V_TRUEVISIONTARGA
#define V_TRUEVISIONTARGA

#include <rgb.cpp>

#pragma pack(push,default_pack)
#pragma pack(1) //byte packing
struct TGAHEADER
{
 BYTE idlen;  //lenght of imgID following header
 BYTE paltype;//0-no pal,1-pal
 BYTE imgtype;//0-no img,1-pal img,2-true img,3-B&W img,9...11-RLE 1,2,3
 WORD palste; //file pal corespondence to mem pal (start entry)
 WORD palnre; //number of entries in pal
 BYTE palbpe; //pal bits per entry: 15,16,24,32
 WORD imgL;   
 WORD imgD;
 WORD imgW;
 WORD imgH;
 BYTE imgbpp; //8,16,24,32
 BYTE imgdesc;//b3...0: alpha bits per pixel, b4: 0-left,1-right, b5: 0-bottom,1-top
};
#pragma pack(pop,default_pack)

class TGAimage
{
public:
 FILE *Ftga; //file handle
 DWORD pf;   //(STD) pixel format (r=g=b)
 int lng,lat;//(STD) image width and height
 NAT bpp,Bpl;//(STD) bits/pixel,B/line
 BYTE *pal;  //pal
 NAT type;
 NAT dataoff,paloff;//data,pal offset in file
 NAT dataszB,palszB;//data,pal size in bytes
 int lind;   //next line index
 
 void Set(int width=0,int height=0,DWORD pixform=0x32108888);
 int Open(LPSTR filename=NULL); //(STD) open/close
 int Save(LPSTR filename=NULL); //only true color and b/channel must be the same for r,g,b
 DWORD CanSave(DWORD,int); //(STD) matches desired format to supported save format, if std returns only fully supported formats
 int Seek(NAT li=0); //(STD) seeks a line
 void GetF(BYTE *colbuf,NAT lcnt=1,NAT pitch=0,DWORD pixform=0x32100888); //(STD)
 void PutF(BYTE *colbuf,NAT lcnt=1,NAT pitch=0,DWORD pixform=0x32108888); //bits/channel must be equal for R,G,B channels
 
 void ShowDC(HDC hdc,int l=0,int u=0,int r=0,int d=0);
 
 TGAimage()
  {
  Ftga=NULL;
  pal=NULL;
  lng=lat=0;
  lind=0;
  }
 ~TGAimage()
  {
  if(pal) free(pal);
  if(Ftga) fclose(Ftga);
  }
};
/*
 Opens any true or paletted uncompressed TGAs
 Fully saves only in formats with the same number of bits per R,G,B channels
and arbitrary A (ex: ?:8:8:8, ?:5:5:5) and total bpp must be >= 16
 Saves in formats with 4(?:1:1:1) and 8(?:2:2:2) bpp and formats like
?:3:3:3, ?:4:4:4, ?:6:6:6, ?:7:7:7 but other programs (Irfan,PaintShopPro)
don't recognize them.
 Saving as 1 bpp, greyscale, or any paletted format is NOT supported !
*/
//*****************************************************************************************

void TGAimage::Set(int width,int height,DWORD pixform)
{
if(Ftga) fclose(Ftga);
if(pal) free(pal);
Ftga=NULL;
pal=NULL;
if(width>0) lng=width;
if(height>0) lat=height;
if(pixform) pf=pixform;
bpp=PF_bpix(pixform);
Bpl=lng*(ALIGN(bpp,7)>>3);
paloff=sizeof(TGAHEADER);
palszB=0;
dataoff=paloff;
dataszB=Bpl*lat;
type=2;
lind=0;
}

int TGAimage::Open(LPSTR filename)
{
if(Ftga) fclose(Ftga);
if(pal) free(pal);
Ftga=NULL;
pal=NULL;
if(!filename) return 0; //close ok
Ftga=FOPEN(filename,"r+b");
if(!Ftga) return 1; //can't open file
TGAHEADER head;
fread(&head,sizeof(TGAHEADER),1,Ftga);
fread(strbuf,head.idlen,1,Ftga);
if(head.imgtype>2) //only color uncompressed TGAs supported
 {
 fclose(Ftga);
 Ftga=NULL;
 return 2; //not supported 
 }
lng=head.imgW;
lat=head.imgH;
bpp=head.imgbpp;
Bpl=lng*(ALIGN(bpp,7)>>3);
type=head.imgtype;
lind=ALIGN(head.palbpe,7)>>3; //lind=B/entry in pal
paloff=sizeof(TGAHEADER)+head.idlen;
palszB=(1<<bpp)*lind;
dataoff=paloff+head.palnre*lind;
dataszB=Bpl*lat;
if(head.paltype)
 {
 pal=(BYTE*)malloc(palszB+4);
 fseek(Ftga,paloff,SEEK_SET);
 fread(pal+head.palste*lind,head.palnre*lind,1,Ftga);
 lind=head.palbpe/3;
 if(lind>8) lind=8;
 pf=((head.palbpe-(lind*3))<<12)|(lind<<8)|(lind<<4)|lind|0x32100000;
 }
else
 pf=bpc_PF(head.imgbpp,head.imgdesc&0xf);
lind=0;
return 0;//Ok
}

int TGAimage::Save(LPSTR filename)
{
if(filename) //create new file
 {
 if(Ftga) fclose(Ftga);
 Ftga=FOPEN(filename,"w+b");
 }
if(!Ftga) return 1; //no file to write to
TGAHEADER head;
ZeroMemory(&head,sizeof(TGAHEADER));
head.imgtype=2; //true color
head.imgW=lng;
head.imgH=lat;
head.imgbpp=bpp;
head.imgdesc=((pf>>12)&0xf);
fseek(Ftga,0,SEEK_SET);
fwrite(&head,sizeof(TGAHEADER),1,Ftga);
return 0;//Ok
}

//matches color format ---------------------------------------------------------------
DWORD TGAimage::CanSave(DWORD pixform=0,int std=0)
{
if(!std) return bpc_PF(PF_bpix(pixform),PF_bcch(pixform,3));
//make sure you don't loose any alpha
std=PF_bcch(pixform,3);
if(std>1)
 return 0x32108888;
else if(std>0)
 return 0x32101555;
//make sure you don't loose any color
std=PF_bpix(pixform);
if(std>24) //25 to 32
 return 0x32108888;
else if(std>16) //17 to 24
 return 0x32100888;
else //1 to 16
 return 0x32101555;
return 0;
}

inline int TGAimage::Seek(NAT li)
{
lind=li%lat;
return dataoff+(lat-lind-1)*Bpl; //offset in file
}

void TGAimage::GetF(BYTE *colbuf,NAT lcnt,NAT pitch,DWORD pixform)
{
if(!Ftga) return;
CCONV lcc;
BYTE *locbuf;
if(!lcnt) lcnt=lat;
if(!pixform) pixform=pf;
lcc.Tf(pf,pixform,bpp);
if(!pitch) pitch=lng*lcc.DBpp;
locbuf=(BYTE*)malloc(Bpl+4);
while(lcnt>0)
 {
 fseek(Ftga,dataoff+(lat-lind-1)*Bpl,SEEK_SET);
 fread(locbuf,Bpl,1,Ftga);
 if(pal)
  lcc.Ptl(colbuf,pal,locbuf,lng);
 else
  lcc.Blt(colbuf,locbuf,lng);
 colbuf+=pitch;
 lind=(lind+1)%lat;
 lcnt--;
 }
free(locbuf);
}

void TGAimage::PutF(BYTE *colbuf,NAT lcnt,NAT pitch,DWORD pixform)
{
if(!Ftga||type!=2) return;
CCONV lcc;
BYTE *locbuf;
if(!lcnt) lcnt=lat;
if(!pixform) pixform=pf;
lcc.Tf(pixform,pf);
if(!pitch) pitch=Bpl;
locbuf=(BYTE*)malloc(lng*lcc.DBpp+4);
while(lcnt>0)
 {
 lcc.Blt(locbuf,colbuf,lng);
 fseek(Ftga,dataoff+(lat-lind-1)*Bpl,SEEK_SET);
 fwrite(locbuf,Bpl,1,Ftga);
 colbuf+=pitch;
 lind=(lind+1)%lat;
 lcnt--;
 }
free(locbuf);
}

void TGAimage::ShowDC(HDC hdc,int l,int u,int r,int d)
{
if(r>l) r-=l-1;
else r=lng;
if(d>u) d-=u-1;
else d=lat;
if(Ftga) 
 {
 BITMAPINFO bmpinf;
 InitBMPINF(&bmpinf,lng,-lat,32);
 BYTE *tgabuf;
 tgabuf=(BYTE*)malloc((lng<<2)*lat+4);
 Seek();
 GetF(tgabuf,lat,lng<<2,0x32108888);
 StretchDIBits(hdc,l,u,r,d,0,0,lng,lat,tgabuf,&bmpinf,DIB_RGB_COLORS,SRCCOPY);
 free(tgabuf);
 }
else BitBlt(hdc,l,u,r,d,hdc,0,0,BLACKNESS);
}

#endif