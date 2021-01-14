#ifndef V_FONT
#define V_FONT

#include <contur.cpp>

#define ASCII_VECT 0x1 //contour
#define ASCII_IMAG 0x2 //trace
#define ASCII_BOTH 0x3

class AsciiSet
{
public:
 DWORD stat;
 LPSTR name; //text name
 DWORD atrib; //for the contour
 int H,A,D,MW; //height, ascent, descent, max width for the traces
 short cw[256],co[256]; //cell width, char box origin
 short *ich[256]; //[256] valid if status & ASCII_IMAG (ich[x]=trace for x char)
 Contur*vch; //[256] valid if status & ASCII_VECT
 float sw,sh; //width/height scale factors from outline and contour to traces
 
 DWORD color; //text output color
//for print1
 float Bf,If; //bold factor, italic factor 
//for print2
 float zw,zh; //width/height scale factors from traces to actual drawing
 int h,a; //text output size
 
 void Free();
 BOOL FromCHC(LPSTR,int,float,FLAGS);
 //int FromCHI(LPSTR);
 //int ToCHI(LPSTR);
 //void ReTrace(int,float);
 int Extent(ASCIZ,char);
 void Draw(Image*,char,int,int);
 void Look(DWORD,float,float,NAT);
 void print(Image*,ASCIZ,int,int,char); //draw traces with no modification
 void print1(Image*,ASCIZ,int,int); //ignore zw,zh,h,a; uses Bf and If
 void print2(Image*,ASCIZ,int,int); //use all params
 AsciiSet() { ZeroMemory(this,sizeof(AsciiSet)); color=C_WHITE; }
 AsciiSet(LPSTR,int,float,FLAGS);
 ~AsciiSet() { Free(); }
};

struct CHCheader
{
 DWORD id; //"CHC1" | "CHI1"
 int height,ascent; //descent=height-ascent
 int maxwidth;
 union
  { //suib: 0xSUIB: B=weight/100, I-italic, U-underlined, S-struckout
  DWORD atrib; //pitfam: 0xFP: P-fixed pitch if set, F-family
  struct {WORD suib; BYTE pitfam,chset;}; //chset: 0xCC: char set (1B)
  };
 DWORD off[256]; //file offset table (if entry is NULL then char is not in file)
};

#define CHC_LINE    1
#define CHC_QSPLINE 2
#define CHC_CSPLINE 3
#define CHC_MOVE    4

//************************************************************************************
#define FIXVAL(fixed) (float)((long)fixed>>16)

//creates a character contour file from TT fonts -----------------------------------
BOOL TTFtoCHC(LPSTR chcname,HFONT hfnt,HFONT*fmap=NULL)
{
HDC hldc;
TEXTMETRIC tm;
GLYPHMETRICS gm;
TTPOLYGONHEADER*ph;
TTPOLYCURVE*pc;
DWORD tfm[4],*data,tag; //transformation matrix
FILE*fchc;
CHCheader chh;
NAT nszb,foff;
int chszb,bleft,tx;
tfm[0]=tfm[3]=0x10000;
tfm[1]=tfm[2]=0;
hldc=GetDC(NULL);
if(!SelectObject(hldc,hfnt)) return 0;
GetTextMetrics(hldc,&tm);
chh.id=FOURCC("CHC1");
chh.height=tm.tmHeight;
chh.ascent=tm.tmAscent;
chh.maxwidth=tm.tmMaxCharWidth;
chh.suib=(tm.tmWeight/100)|((tm.tmItalic&0xf)<<4)|((tm.tmUnderlined&0xf)<<8)|((tm.tmStruckOut&0xf)<<12);
chh.pitfam=((tm.tmPitchAndFamily&TMPF_FIXED_PITCH)^TMPF_FIXED_PITCH)|(tm.tmPitchAndFamily&0xf0);
chh.chset=tm.tmCharSet;
REGu=GetTextFace(hldc,256,REGs);
REGu=ALIGN(REGu+1,3); //align name length to DWORD
if(!(tm.tmPitchAndFamily&TMPF_VECTOR)) error("Is not a vector font",0,REGs);
fchc=FOPEN(chcname,"wb");
if(!fchc) return 0; //file error
fseek(fchc,sizeof(chh),0);
fwrite(&REGu,4,1,fchc); //write name size in B
fwrite(REGs,REGu,1,fchc); //write text face name
foff=sizeof(chh)+4+REGu;
//dump(TEXTMETRIC_MAP"^"CHCheader_MAP"^$256",&tm,&chh,REGs);
for(NAT ch=0;ch<=255;ch++)
 {
 if(fmap) //use more than one font
  {
  if(!SelectObject(hldc,fmap[ch]?fmap[ch]:hfnt))
   error("Can't Select",__LINE__,__FILE__);
  }
 chszb=GetGlyphOutline(hldc,ch,GGO_NATIVE,&gm,chszb,NULL,(MAT2*)&tfm);
 if(chszb<0)
  {
  chh.off[ch]=0;
  continue;
  }
 data=(DWORD*)malloc(chszb);
 if(GetGlyphOutline(hldc,ch,GGO_NATIVE,&gm,chszb,data,(MAT2*)&tfm)==GDI_ERROR)
  {
  chh.off[ch]=0;
  continue;
  }
 chh.off[ch]=foff;
 nszb=0;
 fseek(fchc,foff+8,0);
 tx=-gm.gmptGlyphOrigin.x;
 ph=(TTPOLYGONHEADER*)data;
 while(chszb>0) //draw a glyph
  {
  if(ph->dwType!=TT_POLYGON_TYPE) error("TT invalid poly header");
  tag=(CHC_MOVE<<28)|1;
  ph->pfxStart.x.value+=tx;
  fwrite(&tag,4,1,fchc);
  fwrite(&ph->pfxStart,8,1,fchc);
  nszb+=12;
  bleft=ph->cb-sizeof(TTPOLYGONHEADER);
  REGu=sizeof(TTPOLYGONHEADER);
  PADR(pc,ph,REGu);
  while(bleft>0) //draw a glyph connex part
   {
   if(pc->wType==TT_PRIM_LINE)
    tag=(CHC_LINE<<28)|pc->cpfx;
   else if(pc->wType==TT_PRIM_QSPLINE)
    tag=(CHC_QSPLINE<<28)|pc->cpfx;
   else if(pc->wType==TT_PRIM_CSPLINE)
    tag=(CHC_CSPLINE<<28)|pc->cpfx;
   else
    error("TT unknown primitive");
   for(int p=0;p<pc->cpfx;p++)
    pc->apfx[p].x.value+=tx;
   REGi=4+(pc->cpfx<<3);
   fwrite(&tag,4,1,fchc);
   fwrite(pc->apfx,REGi-4,1,fchc);
   nszb+=REGi;
   bleft-=REGi;
   POFF(pc,REGi);
   }
  /*/begin: explicitly closing contur
  if(tag>>28==CHC_LINE) //add one more point to previous LINE entry
   {
   fwrite(&ph->pfxStart,8,1,fchc);
   tag++;
   fseek(fchc,-(REGi+8),SEEK_CUR);
   fwrite(&tag,4,1,fchc);
   fseek(fchc,REGi+4,SEEK_CUR);
   nszb+=8;
   }
  else //create a new LINE entry
   {
   tag=(CHC_LINE<<28)|1;
   fwrite(&tag,4,1,fchc);
   fwrite(&ph->pfxStart,8,1,fchc);
   nszb+=12;
   }
  //end: explicitly closing contour*/
  REGu=ph->cb;
  chszb-=REGu;
  POFF(ph,REGu);
  }
 fseek(fchc,foff,0);
 fwrite(&nszb,4,1,fchc);
 fwrite(&gm.gmCellIncX,2,1,fchc);
 fwrite(&gm.gmptGlyphOrigin.x,2,1,fchc);
 FREE(data);
 foff+=8+nszb;
 }
ReleaseDC(NULL,hldc);
fseek(fchc,0,0);
fwrite(&chh,sizeof(chh),1,fchc);
fclose(fchc);
return 1; //Ok
}

// AsciiSet <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//...........................................................................................
void AsciiSet::Free()
{
FREE(name);
for(int ch=0;ch<=255;ch++)
 {
 if(vch) vch[ch].Free();
 FREE(ich[ch]);
 }
FREE(vch);
stat=0;
}

//...........................................................................................
BOOL AsciiSet::FromCHC(LPSTR chcname,int nh=0,float wf=1.0f,FLAGS keep=ASCII_BOTH)
{
CHCheader chh;
FILE*fchc;
long*cdata,*mdata;
int csz,nrp,tag;
float Ax,Ay,Bx,By,Cx,Cy,Y;
fchc=FOPEN(chcname,"rb");
if(!fchc) return 0; //file error
fread(&chh,sizeof(chh),1,fchc);
if(chh.id!=FOURCC("CHC1"))
 {
 fclose(fchc);
 return 0; //not a CHC file
 }
fread(&REGu,4,1,fchc);
if(name) free(name);
name=(LPSTR)malloc(REGu);
fread(name,REGu,1,fchc);
if(!nh) nh=chh.height;
sh=(float)nh/chh.height;
sw=wf?wf*sh:sh;
H=nh;
A=chh.ascent*sh;
D=H-A;
MW=chh.maxwidth*sw;
atrib=chh.atrib;
Y=chh.ascent; //used to flip chars verticaly
stat=keep;
if(!vch) vch=(Contur*)calloc(256,sizeof(Contur));
for(NAT ch=0;ch<=255;ch++)
 {
 vch[ch].Free();
 FREE(ich[ch]);
 if(!chh.off[ch]) continue;
 //read contour
 fseek(fchc,chh.off[ch],0);
 fread(&csz,4,1,fchc);
 fread(&cw[ch],2,1,fchc);
 fread(&co[ch],2,1,fchc);
 cw[ch]*=sw; //scale char width by width factor
 co[ch]*=sw; //scale char origin by width factor
 mdata=cdata=(long*)malloc(csz+100);
 fread(cdata,csz,1,fchc);
 //draw
 while(csz>11)
  {
  nrp=*cdata&0xfffffff;
  tag=*cdata>>28;
  csz-=4+(nrp<<3);
  if(tag==CHC_MOVE)
   {
   Ax=FIXVAL(*(cdata+1));
   Ay=FIXVAL(*(cdata+2));
   cdata+=3;
   vch[ch].start(Ax,Y-Ay);
   }
  else if(tag==CHC_LINE)
   {
   cdata++;
   while(nrp>0)
    {
    Bx=FIXVAL(*cdata);
    cdata++;
    By=FIXVAL(*cdata);
    cdata++;
    vch[ch].lineto(Bx,Y-By);
    nrp--;
    }
   }
  else if(tag==CHC_QSPLINE)
   {
   cdata++;
   while(nrp>1)
    {
    Bx=FIXVAL(*cdata);
    cdata++;
    By=FIXVAL(*cdata);
    cdata++;
    Cx=FIXVAL(*cdata);
    Cy=FIXVAL(*(cdata+1));
    if(nrp>2)
     {
     Cx=(Bx+Cx)/2;
     Cy=(By+Cy)/2;
     }
    else
     cdata+=2;
    vch[ch].spline2(Bx,Y-By,Cx,Y-Cy);
    nrp--;
    }
   }
  else
   error("CHC unknown tag",tag);
  }
 free(mdata);
 vch[ch].close();
 if(stat&ASCII_IMAG) ich[ch]=vch[ch].Trace(H,sw,sh); //trace
 if(!(stat&ASCII_VECT)) vch[ch].Free(); //if vectorial representation is not wanted don't keep it
 }
fclose(fchc);
if(!(stat&ASCII_VECT)) FREE(vch);
Look(color,1.f,0.f,H);
return H;
}

//...........................................................................................
inline AsciiSet::AsciiSet(LPSTR chcname,int nh=0,float wf=1.0f,FLAGS keep=ASCII_BOTH)
{
FromCHC(chcname,nh,wf,keep);
}

//...........................................................................................
int AsciiSet::Extent(ASCIZ lstr,char term=0)
{
int retv=0;
while(*lstr!=term)
 {
 retv+=cw[*lstr];
 lstr++;
 }
return retv;
}

//...........................................................................................
void AsciiSet::Draw(Image*pimg,char ch,int x=0,int y=0)
{
if(!pimg) return;
vch[ch].Init(pimg);
//*
pimg->Clear(0);
pimg->color=C_MAGENTA;
pimg->at(x,0);
pimg->linv(H);
pimg->at(x,A);
pimg->linh(cw[ch]);
//vch[ch].Draw(C_RED,sw,sh);
vch[ch].Fill(C_GREEN,sw,sh,H,x+co[ch],y);
pimg->DrawDC(NULL,0,0,0,0,pimg->lng,pimg->lat,-1,hmwnd);
//*/
}

//...........................................................................................
void AsciiSet::Look(DWORD col=C_WHITE,float bold=1.f,float itang=0.f,NAT height=0)
{
color=col;
Bf=bold?bold:1.f;
If=tan(itang);
h=height?height:H;
zh=(float)h/H;
zw=zh*Bf;
a=A*zh;
if(If<0.f) If=0.f;
if(Bf<0.f) Bf=1.f;
}

//...........................................................................................
void AsciiSet::print(Image*limg,ASCIZ ltext,int x=0,int y=0,char term=0)
{
if(!limg||!(stat&ASCII_IMAG)||!ltext) return;
unsigned char*text=(unsigned char*)ltext;
limg->color=color;
while(*text!=term)
 {
 if(ich[*text]) limg->dtrace(ich[*text],x+co[*text],y);
 x+=cw[*text];
 text++;
 }
}

//...........................................................................................
void AsciiSet::print1(Image*limg,ASCIZ ltext,int x,int y)
{
if(!limg||!(stat&ASCII_IMAG)||!ltext) return;
unsigned char*text=(unsigned char*)ltext;
limg->color=color;
while(*text)
 {
 if(ich[*text]) limg->dwtrace(ich[*text],x+co[*text]*Bf,y,Bf,If);
 x+=cw[*text]*Bf;
 text++;
 }
}

//...........................................................................................
void AsciiSet::print2(Image*limg,ASCIZ ltext,int x,int y)
{
if(!limg||!(stat&ASCII_IMAG)||!ltext) return;
unsigned char*text=(unsigned char*)ltext;
limg->color=color;
while(*text)
 {
 if(ich[*text]) limg->dstrace(ich[*text],x+co[*text]*zw,y,zw,zh,If);
 x+=cw[*text]*zw;
 text++;
 }
}
// AsciiSet >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif