#ifndef V_PCX
#define V_PCX

#include <rgb.cpp>

#pragma pack(push,default_pack)
#pragma pack(1) //byte packing
struct PCXHEADER
{
 BYTE id;        //10
 BYTE vers;      //5
 BYTE comp;      //1-RLE
 BYTE bpp;       //bits per plane (1,2,4,8)
 WORD l,u,r,d;   //image window
 WORD hdpi,vdpi; //dots per inch
 BYTE pal16[48]; //pallette for 16 colors pcx
 BYTE reserved;  //0
 BYTE plcnt;     //num planes of colour
 WORD Bppl;      //bytes per plane line (WORD aligned)
 WORD paltip;    //1|2
 WORD hres,vres; //screen sizes
 //BYTE unused[54];//0 (fill up to 128 bytes)
};
#pragma pack(pop,default_pack)

class PCXimage
{
public:
 FILE *Fpcx;
 int lng,lat; //(STD) image width and height
 int pmw,pmh; //(STD) resolution in pixels per meter
 NAT bpp,Bpl; //(STD) bits per pixel, B/line
 NAT BpP,ppl; //Bytes per plane,pixels per line
 BYTE *pal;
 int FPos,lind; //file pointer, line index that will be grabed next
 
 int Open(LPSTR filename=NULL);
 int Seek(NAT li=0); //(STD) seeks a line
 void Grab(BYTE *linbuf); //decompress RLE line
 void GetF(BYTE *colbuf,NAT lcnt=1,NAT pitch=0,DWORD pixform=0x32100888); //(STD)
 void ShowDC(HDC hdc,int l=0,int u=0,int r=0,int d=0);
 PCXimage()
  {
  Fpcx=NULL;
  pal=NULL;
  lng=lat=0;
  pmw=pmh=0;
  FPos=0;
  lind=0;
  }
 ~PCXimage()
  {
  if(Fpcx) fclose(Fpcx);
  if(pal) free(pal);
  }
};
///////////////////////////////////////////////////////////////////////////

int PCXimage::Open(LPSTR filename)
{
if(Fpcx) fclose(Fpcx);
if(pal) free(pal);
pal=NULL;
if(!filename)
 {
 Fpcx=NULL;
 return 0; //close Ok
 }
Fpcx=FOPEN(filename,"rb");
if(!Fpcx) return 1; //can't access file
PCXHEADER head;
fread(&head,sizeof(head),1,Fpcx);
if(head.id!=10||head.comp!=1) 
 {
 fclose(Fpcx);
 Fpcx=NULL;
 return 2;//Not a pcx file
 }
lng=head.r-head.l+1;
lat=head.d-head.u+1;
pmw=head.hdpi*M_INCH;
pmh=head.vdpi*M_INCH;
BpP=head.Bppl;            //B/plane
Bpl=head.plcnt*head.Bppl; //B/line=num_planes*Bytes_per_plane
bpp=head.plcnt*head.bpp; //bits/pixel=bpplane*num_planes
ppl=ALIGN(lng,3); //DWORD aligned line buffer size
if(bpp==24) //true color
 pal=NULL; 
else if(bpp==8) //256 colors
 {
 pal=(BYTE*)malloc(768);
 fseek(Fpcx,-769,SEEK_END);
 if(fgetc(Fpcx)==12) fread(pal,768,1,Fpcx); //pcx 256 palette
 else return 4; //coruppted palette
 }
else if(bpp==4) //16 colors
 {
 pal=(BYTE*)malloc(48);
 CopyMemory(pal,head.pal16,48);
 ppl=ALIGN(lng,15); //16-bytes aligned line buffer size
 }
else
 return 3; //unknown color format
Seek();
return 0; //Ok
}

int PCXimage::Seek(NAT li)
{
if(li==0||li>=lat)
 {
 FPos=128;
 lind=0;
 }
else if(li!=lind)
 {
 BYTE byte;
 NAT count=0;
 fseek(Fpcx,128,0);
 while(count<Bpl*li)
  {
  byte=fgetc(Fpcx);
  if((byte&0xc0)==0xc0) //upper bits set <=> run
   {
   count+=byte&0x3f;
   byte=fgetc(Fpcx);
   }
  else count++;
  }
 FPos=ftell(Fpcx);
 lind=li;
 }
return FPos;
}

void PCXimage::Grab(BYTE *linbuf)
{
BYTE byte;
NAT count,rep;
fseek(Fpcx,FPos,0);
count=0;
while(count<Bpl)
 {
 byte=fgetc(Fpcx);
 if((byte&0xc0)==0xc0) //b7|b6 are set
  {
  rep=byte&0x3f;
  byte=fgetc(Fpcx);
  while(rep>0)
   {
   linbuf[count++]=byte;
   rep--;
   }
  }
 else linbuf[count++]=byte;
 }
FPos=ftell(Fpcx);
lind++;
if(lind>=lat) Seek(); //wrap to first line
}

void PCXimage::GetF(BYTE*colbuf,NAT lcnt,NAT pitch,DWORD pixform)
{
if(!Fpcx) return;
BYTE *linbuf,*aux;
NAT el,ind,pos;
CCONV lcc;
if(!lcnt) lcnt=lat;
if(!pixform) pixform=0x32100888;
lcc.Tf(0x32100888,pixform,24);
if(!pitch) pitch=ppl*lcc.DBpp;
linbuf=(BYTE*)malloc(Bpl+4);
while(lcnt>0)
 { 
 Grab(linbuf);
 aux=colbuf;
 if(bpp==24)
  {
  ind=BpP<<1;
  for(el=0;el<lng;el++)
   {
   lcc.BGRA(colbuf,linbuf[el+ind],linbuf[el+BpP],linbuf[el],0xff);
   colbuf+=lcc.DBpp;
   }
  }
 else if(bpp==8)
  {
  for(el=0;el<lng;el++)
   {
   ind=linbuf[el]*3;
   lcc.BGRA(colbuf,pal[ind+2],pal[ind+1],pal[ind],0xff);
   colbuf+=lcc.DBpp;
   }
  }
 else if(bpp==4)
  {
  pos=7;
  el=0;
  while(el<BpP)
   {
   ind=((linbuf[el]>>pos)&1); //b0
   ind|=(((linbuf[el+BpP]>>pos)&1)<<1); //b1
   ind|=(((linbuf[el+(BpP<<1)]>>pos)&1)<<2); //b2
   ind|=(((linbuf[el+BpP*3]>>pos)&1)<<3); //b3
   ind*=3; //index in pal
   lcc.BGRA(colbuf,pal[ind+2],pal[ind+1],pal[ind],0xff);
   colbuf+=lcc.DBpp;
   if(pos>0) pos--;
   else
    {
    pos=7;
    el++;
    }
   }
  }
 colbuf=aux+pitch;
 lcnt--;
 }
free(linbuf);
}

void PCXimage::ShowDC(HDC hdc,int l,int u,int r,int d)
{
if(r<=l) r=lng;
else r-=l-1;
if(d<=u) d=lat;
else d-=u-1;
if(Fpcx) 
 {
 BITMAPINFO bmpinf;
 InitBMPINF(&bmpinf,ppl,-lat,24);
 BYTE *pcxbuf;
 pcxbuf=(BYTE*)malloc(ppl*3*lat+4);
 Seek();
 GetF(pcxbuf,lat,ppl*3,0x32100888);
 StretchDIBits(hdc,l,u,r,d,0,0,lng,lat,pcxbuf,&bmpinf,DIB_RGB_COLORS,SRCCOPY);
 free(pcxbuf);
 }
else BitBlt(hdc,l,u,r,d,hdc,0,0,BLACKNESS);
}

#endif