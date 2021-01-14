#ifndef V_UINTERFACE_ITEM
#define V_UINTERFACE_ITEM

//Style
#define IL_0    0
#define IL_NW   0x00000001 //write protect
#define IL_NE   0x00000002 //don't expand
#define IL_NP   0x00000004 //don't allow picking
#define IL_RO   0x00000008 //static (no run), auto set NW and NP
#define IL_DIIS 0x00000010 //draw menu item independent state
#define IL_APOP 0x00000020 //auto popup
#define IL_NTFY 0x00000040 //call notifycation function
#define IL_UBRS 0x00000080 //use internal brush
#define IL_NTXT 0x00000100 //no text
#define IL_NHAR 0x00000200 //no horizontal Scroll arrows
#define IL_FIMG 0x00000400 //show fimg
#define IL_BIMG 0x00000800 //show bimg
#define IL_NMOV 0x00001000 //no move (also no resize)
#define IL_NRSZ 0x00002000 //no resize
#define IL_NTHA 0x00004000 //no toggle harrows
#define IL_NMLP 0x00008000 //no move labels pos
#define IL_NMIP 0x00010000 //no move image position
#define IL_GRID 0x00020000 //multiply coordinates by grid values
#define IL_PABP 0x00040000 //"pre-added" base pointer (link=uiform->bp+offset)

#define IL_BLP  0x01000000 //H/V
#define IL_ALP  0x02000000 //H/V
#define IL_BLA  0x04000000 //H:-, V:L/C
#define IL_ALA  0x08000000 //H:-, V:R/C
#define IL_NBL  0x10000000 //hide L border
#define IL_NBU  0x20000000 //hide U border
#define IL_NBR  0x40000000 //hide R border
#define IL_NBD  0x80000000 //hide D border

#define IL_CLABS 0x0f000000 //centered labels
#define IL_NBORD 0xf0000000 //no borders
//Status
#define IS_FOC  0x00000001 //has focus
#define IS_EXP  0x00000002 //is expanded
#define IS_EXPB 0x00000004 //expanded below
#define IS_OVER 0x00000008 //overwrite mode
#define IS_CSTM 0x00000010 //custumizing
#define IS_MAKE 0x00000020 //creating
#define IS_MIN  0x00000040 //check against min
#define IS_MAX  0x00000080 //check against max
#define IS_HAR  0x00000100 //horizontal arrows
#define IS_VAR  0x00000200 //vertical arrows
#define IS_DRWL 0x00000400 //in drawing label mode
#define IS_DRWE 0x00000800 //in drawing ecpansion mode
#define IS_NACT 0x00001000 //is not active
#define IS_NVIS 0x00002000 //is not visible
#define IS_NIR  0x00004000 //not in range
#define IS_HLIT 0x00008000 //is highlighted
#define IS_ILNK 0x00010000 //internal link (should free)
#define IS_ILAB 0x00020000 //internal labels (should free)
#define IS_ITIP 0x00040000 //internal tip (should free)
#define IS_IPRP 0x00080000 //internal prop (should free)

#define IM_PERM (IS_MIN|IS_MAX|IS_HLIT|IS_ILNK|IS_FOC|IS_EXP|IS_OVER|IS_NVIS) //state bits that are preserved

//Text formating
#define IFT_SEP  0x10000000 //use separator as terminator
#define IFT_LTRM 0x20000000 //L triming
#define IFT_RTRM 0x40000000 //R triming
#define IFT_UCOD 0x80000000 //unicode characters
//#######################################################################################
class UITEM
{
public:
 LPSTR prop,bef,aft,tip; //prop should be first (to id by name)
 NAT prpnc,befnc,aftnc,tipnc;
 UIFORM *uiform;
 union {
  LPSTR *lnks;
  BYTE *lnkb;
  WORD *lnkw;
  NAT *lnku;
  char *lnkc;
  int *lnki;
  float *lnkf;
  double *lnkd;
  void *link; };
 LPSTR buffer;
 int labnc,bufnc,cur,hof,vof,nrl,nri;
 UINotify INotify; //pointer to callback notification function for this item
//Persistent struct (64B)
 int l,u,r,d; //main
 int px,py; //image position
 DWORD type,styl,stat;
 NAT bufsz,lnksz;
 int lstw,sel; //list width
 NAT reg0; //IT_N-radix, IT_R-scientific notation, IT_M-offset, IT_L-base pointer , IT_P-bit mask
 QUADB min,max;
//!Persistent struct 
 int l1,u1,r1,d1, l2,u2,r2,d2; //harrows rects
 int el,eu,er,ed, el1,eu1,er1,ed1, el2,eu2,er2,ed2; //list & varrows rects
 DWORD balgn,aalgn; //bef&aft align flags
 int bx,by,ax,ay; //bef&aft reference points
 struct UITIND{
  LPSTR it;
  BYTE nc;  //doesn't include separator or null
  BYTE flg; //b0- free
  BYTE st;  //state: [0...mst]
  BYTE mst; //defaults to 1
  }*its;
 char sep;
 HBRUSH hib; //internal brush

 void Init(UIFORM*,LPSTR,DWORD,DWORD,int);
 void Make(NAT,DWORD);
 void Link(void *lnk,NAT,UINotify);
 void Text(LPSTR,LPSTR,LPSTR);
 void Opts(void*,void*); //optionals
 void Defs(LPSTR,int); //defs must be terminated by the separator
 void Def(LPSTR); //def is null terminated
 void List(LPSTR,NAT,int); //replace defs with a list of evenly spaced items
 void Slut(char**,int,NAT); //string look up table
 void Items(char**,NAT*,char);
 void States(BYTE);
 void Free();
 void Lnk_Lab();
 void Lab_Lnk();
 void Sync();
 void Clear();
 void Resize();
 void Pos(int,int,int,int,int,int);
 void Expand();
 void Hidexp();
 void Focus(int);
 void Pick(int,int);
 void Cursor(int,int); //move cursor
 int RunM(int,int,int,int); //butt=0-none, 1-left, 2-right, 3-mid
 int RunK(BYTE,BYTE); //butt=0-none, 1-left, 2-right, 3-mid
 int InList(LPSTR);
 char ValidKey(BYTE);
 void Color(COLORREF);
 void Redraw(int);
 void Draw();
 void DrawEx();
 void Draw0();
 void DrawEx0();
 void Draw1();
 void DrawEx1();
 void Draw2();
 void DrawEx2();
 void DrawC();
 int RunMC(int,int,int,int); //butt=0-none, 1-left, 2-right, 3-mid
 int RunKC(char,char); //butt=0-none, 1-left, 2-right, 3-mid
 void Custom(int);
};
//***************************************************************************************************

//integer to string (maxnc doesn't include the terminator) ------------------------
void ItoS_(int val,LPSTR str,int rad=10,int maxnc=1,char term='\0')
{
if(val<0)
 {
 val=-val;
 if(maxnc)
  {
  *str='-';
  str++;
  maxnc--;
  }
 }
int nd=LogN(rad,val);
if(nd>maxnc) nd=maxnc;
str+=nd;
if(term!=C_NULL) *str=term;
while(nd)
 {
 str--;
 *str=val%rad;
 if(*str<10) *str+='0';
 else *str+=55;
 val/=rad;
 nd--;
 }
}

//UITEM <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void UITEM::Init(UIFORM*pform,LPSTR lprop=0,DWORD ltype=IT_T,DWORD lstyl=IL_0,int r0=0)
{
if(pform) uiform=pform;
INotify=uiform->Notify;
if(lprop) prop=lprop;
prpnc=sl(prop);
if(ltype) type=ltype;
styl=lstyl;
reg0=r0;
if(!reg0)
 {
 if(type&IT_N) reg0=10; //protect against divide by 0
 if(type&IT_P) reg0=0xffffffff; // affect all bits
 }
if(type==IT_T)
 {
 bufsz=PATHSZ;
 lnksz=bufsz+1;
 }
else if(type==IT_F)
 {
 lnksz=4;
 bufsz=8;
 }
else if(type==IT_D)
 {
 lnksz=8;
 bufsz=16; //15+1
 }
else if(type==IT_B)
 {
 lnksz=1;
 bufsz=LogN(reg0,0xff)+1;
 }
else if(type==IT_W)
 {
 lnksz=2;
 bufsz=LogN(reg0,0xffff)+1;
 }
else if(type==IT_I)
 {
 lnksz=4;
 bufsz=LogN(reg0,0xffffffff)+1;
 }
else if(type==IT_Q)
 {
 lnksz=8;
 //bufsz=LogN(reg0,0xffffffffffffffffi64)+1;
 error("N/A");
 }
else if(type==IT_M) //menu
 {
 lnksz=4;
 bufsz=PATHSZ;
 styl|=IL_NW;
 }
else if(type==IT_L) //list
 {
 lnksz=4;
 bufsz=PATHSZ;
 styl|=IL_NW;
 }
else if(type==IT_P) //push button
 {
 lnksz=4;
 bufsz=PATHSZ;
 styl|=IL_NW|IL_NE|IL_APOP;
 }
else if(type==IT_C) //color button
 {
 lnksz=4;
 bufsz=PATHSZ;
 styl|=IL_NW|IL_NE|IL_UBRS;
 }
else if(type==IT_O) //options
 {
 lnksz=1;
 bufsz=32;
 styl|=IL_NW|IL_NP|IL_DIIS;
 }
else if(type==IT_X) //flags
 {
 lnksz=4;
 bufsz=32;
 styl|=IL_NW|IL_NP|IL_DIIS;
 }
if(styl&IL_RO) styl|=IL_NW|IL_NP;
}

void UITEM::Make(NAT maxch=0,DWORD lstat=0)
{
stat|=lstat;
if(maxch) bufsz=maxch;
buffer=(LPSTR)realloc(buffer,bufsz+1);
if(type&IT_O) scf(buffer,prop,bufsz);
else ZeroMemory(buffer,bufsz+1);
if(stat&IS_EXP) Expand();
if(stat&IS_ILNK) Link(NULL,0,NULL);
if(stat&IS_CSTM) Custom(1);
Cursor(0,0);
}

void UITEM::Text(LPSTR lbef=NULL,LPSTR laft=NULL,LPSTR ltip=NULL)
{
if(stat&IS_ILAB)
 {
 if(bef) free(bef);
 if(aft) free(aft);
 stat&=~IS_ILAB;
 }
if(stat&IS_ITIP&&tip)
 {
 free(tip);
 stat&=~IS_ITIP;
 }
bef=lbef;
befnc=sl(bef);
aft=laft;
aftnc=sl(aft);
tip=ltip;
tipnc=sl(tip);
}

void UITEM::Opts(void*pmin=NULL,void*pmax=NULL)
{
if(pmin)
 {
 stat|=IS_MIN;
 CopyMemory(min.b,pmin,4);
 }
if(pmax)
 {
 stat|=IS_MAX;
 CopyMemory(max.b,pmax,4);
 }
}

void UITEM::Color(COLORREF nc=0xffffff)
{
if(hib) DeleteObject(hib);
hib=CreateSolidBrush(nc);
if(type&IT_C) *lnku=nc;
if(styl&IL_NTFY&&INotify) INotify(this,IN_CLR);
Redraw(1);
}

void UITEM::Free()
{
if(its)
 {
 while(nri>0)
  {
  nri--;
  if((its[nri].flg&0x1)&&its[nri].it)
   {
   free(its[nri].it);
   its[nri].flg&=0xfe; //reset b0
   }
  its[nri].nc=0;
  }
 FREE(its);
 }
stat&=~(IS_EXP|IS_FOC);
if(hib) DeleteObject(hib);
hib=NULL;
if(stat&IS_ILNK)
 {
 FREE(link);
 stat&=~IS_ILNK;
 }
if(stat&IS_ILAB)
 {
 FREE(bef);
 FREE(aft);
 stat&=~IS_ILAB;
 }
if(stat&IS_ITIP)
 {
 FREE(tip);
 stat&=~IS_ITIP;
 }
if(stat&IS_IPRP)
 {
 FREE(prop);
 stat&=~IS_IPRP;
 }
FREE(buffer);
}

void UITEM::Defs(LPSTR defs,int icpy=0)
{
if(its)
 {
 while(nri>0)
  {
  nri--;
  if((its[nri].flg&0x1)&&its[nri].it)
   {
   free(its[nri].it);
   its[nri].flg&=0xfe; //reset b0
   }
  its[nri].nc=0;
  }
 FREE(its);
 }
else
 nri=0;
if(!defs) return; //free if null
int i,l;
sep=defs[strlen(defs)-1]; //set separator
nri=countch(sep,defs);
its=(UITIND*)malloc(nri*sizeof(UITIND));
uiform->CapDC(1);
for(i=0;i<nri;i++)
 {
 l=sl(defs,sep);
 its[i].nc=l<255?l:255;
 its[i].st=0;
 its[i].mst=1;
 GetTextExtentPoint32(uiform->dc,defs,its[i].nc,&uitsz);
 if(uitsz.cx>lstw) lstw=uitsz.cx;
 if(icpy) 
  {
  its[i].it=(LPSTR)malloc(its[i].nc+1);
  sc(its[i].it,defs,its[i].nc,sep,sep);
  its[i].flg|=0x1;
  }
 else
  {
  its[i].it=defs;
  its[i].flg&=0xfe;
  }
 defs+=l+1;
 }
uiform->RelDC();
if(type&IT_M) Lnk_Lab();
else if(styl&IL_NW) Pick(sel,0);
if(stat&IS_EXP) Expand();
}

void UITEM::List(LPSTR beg,NAT span,int nnri)
{
if(its)
 {
 while(nri>0)
  {
  nri--;
  if((its[nri].flg&0x1)&&its[nri].it)
   {
   free(its[nri].it);
   its[nri].flg&=0xfe; //reset b0
   }
  its[nri].nc=0;
  }
 FREE(its);
 }
if(!beg) return;
int i,l;
sep='\0'; //set separator
nri=nnri;
its=(UITIND*)malloc(nri*sizeof(UITIND));
uiform->CapDC(1);
for(i=0;i<nri;i++)
 {
 its[i].it=beg;
 its[i].flg=0; //reset b0
 its[i].st=0;
 its[i].mst=1;
 beg+=span;
 l=sl(its[i].it,sep);
 its[i].nc=l<255?l:255;
 GetTextExtentPoint32(uiform->dc,its[i].it,its[i].nc,&uitsz);
 if(uitsz.cx>lstw) lstw=uitsz.cx;
 }
uiform->RelDC();
if(type&IT_M) Lnk_Lab();
else if(styl&IL_NW) Pick(sel,0);
if(stat&IS_EXP) Expand();
}

void UITEM::Slut(char**lut,int nnri,NAT size=1)
{
if(its)
 {
 while(nri>0)
  {
  nri--;
  if((its[nri].flg&0x1)&&its[nri].it)
   {
   free(its[nri].it);
   its[nri].flg&=0xfe; //reset b0
   }
  its[nri].nc=0;
  }
 FREE(its);
 }
if(!lut) return;
int i,l;
sep='\0'; //set separator
nri=nnri;
its=(UITIND*)malloc(nri*sizeof(UITIND));
uiform->CapDC(1);
for(i=0;i<nri;i++)
 {
 its[i].it=*lut;
 its[i].flg=0; //reset b0
 its[i].st=0;
 its[i].mst=1;
 lut+=size;
 l=sl(its[i].it,sep);
 its[i].nc=l<255?l:255;
 GetTextExtentPoint32(uiform->dc,its[i].it,its[i].nc,&uitsz);
 if(uitsz.cx>lstw) lstw=uitsz.cx;
 }
uiform->RelDC();
if(type&IT_M) Lnk_Lab();
else if(styl&IL_NW) Pick(sel,0);
if(stat&IS_EXP) Expand();
}

void UITEM::Def(LPSTR def)
{
if(!def) return;
if(!(*def)) return;
int i;
for(i=0;i<nri;i++)
 if(CmpMem(def,its[i].it,its[i].nc)) return;
if(!nri) sep=0; //set separator
its=(UITIND*)realloc(its,(nri+1)*sizeof(UITIND));
i=sl(def);
if(i>bufsz) i=bufsz;
its[nri].nc=i<255?i:255;
its[nri].it=(LPSTR)malloc(its[nri].nc+2);
CopyMemory(its[nri].it,def,its[nri].nc);
its[nri].it[its[nri].nc]=sep;
its[nri].it[its[nri].nc+1]=0;
uiform->CapDC(1);
GetTextExtentPoint32(uiform->dc,its[i].it,its[i].nc,&uitsz);
if(uitsz.cx>lstw) lstw=uitsz.cx;
uiform->RelDC();
its[nri].flg|=0x1;
its[nri].st=0;
its[nri].mst=1;
nri++;
if(stat&IS_EXP) Expand();
}

int UITEM::InList(LPSTR def)
{
if(!def) return -1;
for(int i=0;i<nri;i++)
 if(CmpMem(def,its[i].it,its[i].nc)) return i;
return -1;
}

void UITEM::Items(char**defs,NAT*defsz,char isep='_')
{
NAT i;
*defsz=0;
if(!nri||!its)
 {
 if(*defs) free(*defs);
 *defs=NULL;
 return;
 }
for(i=0;i<nri;i++)
 *defsz+=its[i].nc+1;
LPSTR lstr;
lstr=*defs=(LPSTR)realloc(*defs,*defsz+1);
for(i=0;i<nri;i++)
 {
 lstr+=sc(lstr,its[i].it,its[i].nc,sep,isep)+1;
 }
*lstr='\0';
}

void UITEM::Link(void *lnk=NULL,NAT nlnksz=0,UINotify lNotify=NULL)
{
if(stat&IS_ILNK&&link) free(link);
if(nlnksz) lnksz=nlnksz;
if(lNotify) INotify=lNotify;
if(lnk)
 {
 link=lnk;
 stat&=~IS_ILNK;
 }
else if(type!=IT_O)
 {
 link=calloc(1,lnksz);
 stat|=IS_ILNK;
 }
else
 link=NULL;
if(type&IT_T)
 {
 bufsz=lnksz-1;
 buffer=(LPSTR)realloc(buffer,bufsz+1);
 }
else if(type&IT_L&&reg0&&link) //sync list with selection
 {
 Pick(InLUT((void*)reg0,nri,link,lnksz),0);
 }
Lnk_Lab();
}

void UITEM::States(BYTE nst=0)
{
for(int i=0;i<nri;i++)
 its[i].st=nst;
Lab_Lnk();
Redraw(0x3);
}
//internal

void UITEM::Lnk_Lab()
{
if(!buffer||!link)
 return;
if(type&IT_T)
 sc(buffer,lnkc,bufsz);
else if(type&IT_F)
 gcvt(*lnkf,bufsz-1,REGs);
else if(type&IT_D)
 gcvt(*lnkd,bufsz-1,REGs);
else if(type&IT_B)
 ItoS_(*lnkb,buffer,reg0,bufsz);
else if(type&IT_W)
 ItoS_(*lnkw,buffer,reg0,bufsz);
else if(type&IT_I)
 ItoS_(*lnki,buffer,reg0,bufsz);
else if(type&IT_P)
 Pick(*lnki&reg0?1:0,0x0);
else if(type&IT_M)
 Pick(*lnki-reg0,0x0);
//else if(type&IT_L)
// Pick(sel);
else if(type&IT_C)
 Color(*lnku);
else if(type&IT_O)
 {
 for(int i=0;i<nri;i++)
  CopyMemory(&its[i].st,lnkb+i*(lnksz+reg0),lnksz);
 }
else if(type&IT_X)
 {
 for(int i=0;i<nri;i++)
  its[i].st=GetBit(link,i+reg0);
 UtoS(*lnki,buffer,2,bufsz,'\0','0');
 }
if(type&IT_R) sc(buffer,REGs,bufsz);
Cursor(hof,cur);
}

void UITEM::Lab_Lnk()
{
if(!buffer||!link) return;
stat&=~IS_NIR;
if(type&IT_T)
 sc(lnkc,buffer,lnksz,0);
else if(type&IT_D)
 *lnkd=atof(buffer);
else if(type&IT_F)
 {
 *lnkf=(float)atof(buffer);
 if((stat&IS_MIN)&&(*lnkf<min.f))
  {
  stat|=IS_NIR;
  *lnkf=min.f;
  }
 else if((stat&IS_MAX)&&(*lnkf>max.f))
  {
  stat|=IS_NIR;
  *lnkf=max.f;
  }
 }
else if(type&IT_B) //byte
 {
 *lnkb=(BYTE)StoI(buffer,reg0);
 if((stat&IS_MIN)&&(*lnkb<min.b[0]))
  {
  stat|=IS_NIR;
  *lnkb=min.b[0];
  }
 else if((stat&IS_MAX)&&(*lnkb>max.b[0]))
  {
  stat|=IS_NIR;
  *lnkb=max.b[0];
  }
 }
else if(type&IT_W) //word
 {
 *lnkw=(WORD)StoI(buffer,reg0);
 if((stat&IS_MIN)&&(*lnkw<min.w[0]))
  {
  stat|=IS_NIR;
  *lnkw=min.w[0];
  }
 else if((stat&IS_MAX)&&(*lnkw>max.w[0]))
  {
  stat|=IS_NIR;
  *lnkw=max.w[0];
  }
 }
else if(type&IT_I) //long
 {
 *lnki=StoI(buffer,reg0);
 if((stat&IS_MIN)&&(*lnki<min.l))
  {
  stat|=IS_NIR;
  *lnki=min.l;
  }
 else if((stat&IS_MAX)&&(*lnki>max.l))
  {
  stat|=IS_NIR;
  *lnki=max.l;
  }
 }
else if(type&IT_M)
 *lnki=sel+reg0;
else if(type&IT_P)
 {
 if(sel) *lnki|=reg0;
 else *lnki&=~reg0;
 }
else if(type&IT_L)
 {
 if(reg0)
  CopyMemory(link,((BYTE*)reg0)+sel*lnksz,lnksz);
 }
else if(type&IT_O)
 {
 for(int i=0;i<nri;i++)
  CopyMemory(lnkb+i*(lnksz+reg0),&its[i].st,lnksz);
 }
else if(type&IT_X)
 {
 for(int i=0;i<nri;i++)
  {
  if(its[i].st) SetBit(link,i+reg0);
  else RstBit(link,i+reg0);
  }
 UtoS(*lnki,buffer,2,bufsz,'\0','0');
 }
//else if(type&IT_C)
if(stat&IS_NIR)
 {
 Lnk_Lab();
 if(styl&IL_NTFY&&INotify) INotify(this,IN_ERROR);
 }
}

void UITEM::Sync()
{
if(stat&IS_FOC)
 Lab_Lnk();
else
 Lnk_Lab();
}

void UITEM::Focus(int b=1)
{
if(b&&!(stat&IS_FOC))
 {
 if(uiform->uitfoc) uiform->uitfoc->Focus(0);
 uiform->uitfoc=this;
 uiform->Info(tip);
 stat|=IS_FOC;
 Redraw(0);
 if(styl&IL_NTFY&&INotify) INotify(this,IN_FOC);
 }
else if(!b&&stat&IS_FOC)
 {
 if(uiform->uitfoc==this) uiform->uitfoc=NULL;
 Hidexp();
 stat&=~IS_FOC;
 stat&=~IS_HLIT;
 Redraw(0);
 }  
}

void UITEM::Cursor(int lhof,int lcur)
{
hof=lhof;
bufnc=strlen(buffer);
if(hof>=bufnc) hof=bufnc-1;
if(hof<0) hof=0;
uiform->CapDC(0);
while(hof)
 {
 GetTextExtentPoint32(uiform->dc,buffer+hof-1,bufnc-hof+1,&uitsz);
 if(uitsz.cx>r-l)
  break;
 hof--;
 }
GetTextExtentExPoint(uiform->dc,buffer+hof,bufnc-hof,r-l,(int*)&labnc,NULL,&uitsz);
uiform->RelDC();
if((bufnc>labnc)&&!(stat&IS_HAR)&&!(styl&IL_NHAR)) stat|=IS_HAR;
else if((bufnc<=labnc)&&(stat&IS_HAR))
 {
 hof=0;
 stat&=~IS_HAR;
 Redraw(0);
 }
if(lcur>labnc)
 Cursor(hof+lcur-labnc,labnc);
else if(lcur<0)
 Cursor(hof+lcur,0);
else
 {
 cur=lcur;
 Redraw(1);
 }
}

//mod: b0-toggle, b1-notify ...........................................................
void UITEM::Pick(int lsel=0,int mod=0)
{
if(!nri) return;
sel=lsel;
if(nri!=1)
 {
 sel%=nri;
 if(sel<0) sel+=nri;
 lsel=sel;
 }
else
 {
 sel&=0x1;
 lsel=0;
 }
if(mod&1)
 {
 its[lsel].st=(its[lsel].st+1)%(its[lsel].mst+1);
 if(styl&IL_NP&&styl&IL_DIIS)
  {
  Lab_Lnk();
  Redraw(0x3);
  }
 }
if(!(styl&IL_NP))
 {
 Redraw(0x3);
 scf(buffer,its[lsel].it,bufsz,sep,0,0);
 Lab_Lnk();
 Cursor(0,0);
 }
if(mod&0x2&&styl&IL_NTFY&&INotify) INotify(this,IN_SEL);
}

void UITEM::Clear()
{
if(styl&IL_NW)
 {
 Pick(0);
 States(0);
 }
else
 { 
 FillMemory(buffer,bufsz,0);
 Lab_Lnk();
 Cursor(0,0);
 //if(styl&IL_NTFY&&INotify) INotify(this,IN_DEL);
 }
}

//input

int UITEM::RunM(int but=0,int mx=0,int my=0,int dz=0)
{
if(stat&IS_NVIS) return 0;
if(my>=u&&my<d) //in the main row
 {
 if(mx>=l&&mx<r) //in the main zone
  {
  Focus(1);
  if(but&uimb2&&!(styl&IL_NE))
   Expand(); //expand on MB2
  else if(styl&IL_NW) //menu or button
   {
   if(but&uimb1) //next on MB1
    Pick(sel+1,0x2);
   else if(but&uimb2&&styl&IL_UBRS) //choose color (if can't expand and is using hib)
    Color(GetColorRef(type&IT_C?*lnku:reg0,uiform->wnd));
   else if(but&uimb3||but&uimb2)  //prev (if can't expand)
    Pick(sel-1,0x2);
   }
  else //combo edit
   {
   if(but&uimb1) //place cursor on MB1
    {
    uiform->CapDC(0);
    GetTextExtentExPoint(uiform->dc,buffer+hof,bufsz-hof,mx-l,(int*)&cur,NULL,&uitsz);
    if(cur>labnc) cur=labnc;
    Cursor(hof,cur);
    uiform->RelDC();
    Hidexp();
    }
   else if(but&uimb3) //clear on MB3
    Clear();
   }
  return 1;
  }
 else if(stat&IS_HAR&&but&uimb1)
  {
  if(mx>=l1&&mx<r1) //clicked left H arrow
   {
   Cursor(hof-1,cur);
   return 1;
   }
  else if(mx>=l2&&mx<r2) //clicked right H arrow
   {
   Cursor(hof+1,cur);
   return 1;
   }
  }
 }
else if(stat&IS_EXP)
 {
 if(mx>=el&&mx<er) //clicked the expansion colon
  {
  if(my>=eu&&my<ed) //clicked the list
   {
   if(but&uimb1) //select and close on MB1
    {
    Pick(vof+(my-eu)*nrl/(ed-eu),0x3); //select and...
    Hidexp();
    }
   else if(but&uimb2) //select on MB2
    Pick(vof+(my-eu)*nrl/(ed-eu),0x3); //select
   else if(but&uimb3) //add to list and select
    {
    if(!(styl&IL_NW)) Def(buffer); //add label to defs and ...
    Pick(vof+(my-eu)*nrl/(ed-eu),0x3); //select
    }
   return 1;
   }
  else if(stat&IS_VAR)
   {
   if(my>=eu1&&my<ed1) //clicked upper V arrow
    {
    if(vof>0) vof--;
    Redraw(2);
    return 1;
    }
   else if(my>=eu2&&my<ed2) //clicked lower V arrow
    {
    if(vof<nri-nrl) vof++;
    Redraw(2);
    return 1;
    }
   }
  }
 }
Focus(0);
return 0;
}

int UITEM::RunK(BYTE key=0,BYTE scd=0)
{
if(stat&IS_NVIS) return 0;
switch(scd)
 {
 case 59: //F1 - help
  //MessageBox(uiform->wnd,tip,prop,MB_OK|MB_ICONINFORMATION);
  printbox("%i %i %i %i",l,u,r-l,d-u);
  break;
 case 63: //F5 - cut
  StrToClipboard(buffer,bufsz);
 case 76: //5 (pad) -clear
  Clear();
  break;
 case 67: //F9 - toggle
  for(uitsz.cx=0;uitsz.cx<nri;uitsz.cx++)
   its[uitsz.cx].st^=0x1;
  Lab_Lnk();
  Redraw(0x3);
  break;
 case 87: //F11 - set
  States(1);
  break;
 case 88: //F12 - reset
  States(0);
  break;
 case 64: //F6 - copy
  StrToClipboard(buffer,bufsz);
  break;
 case 65: //F7 - paste at cursor
  if(styl&IL_NW) break;
  StrFromClipboard(buffer+hof+cur,bufsz-hof-cur);
  FilterStr(0,' ',' ',buffer,bufsz,'\0');
  Lab_Lnk();
  Cursor(hof+cur,0);
  if(styl&IL_NTFY&&INotify) INotify(this,IN_WRI);
  break;
 case 66: //F8 - paste replacing and saving
  if(styl&IL_NW) break;
  Def(buffer); //add label to defs
  StrFromClipboard(buffer,bufsz);
  FilterStr(0,' ',' ',buffer,bufsz,'\0');
  Lab_Lnk();
  Cursor(0,0);
  if(styl&IL_NTFY&&INotify) INotify(this,IN_WRI);
  break;
 case 71: //home
  Cursor(0,0);
  break;
 case 79: //end
  Cursor(bufnc,0);
  Cursor(hof,labnc);
  break;
 case 73: //pgup
  Pick(sel-1,0x2);
  break;
 case 81: //pgdown
  Pick(sel+1,0x2);
  break;
 case 93: //menu
 case 72: //up
  Expand();
  break;
 case 80: //down
  Hidexp();
  break;
 case 28: //enter
  if(styl&IL_NTFY&&INotify) INotify(this,IN_OK);
  break;
 case 1: //esc
  if(styl&IL_NTFY&&INotify) INotify(this,IN_CANCEL);
  break;
 }
if(styl&IL_NW)
 {
 switch(scd)
  {
  case 75: //left
   Cursor(hof-1,0);
   break;
  case 77: //right
   Cursor(hof+1,0);
   break;
  }
 return 0;
 }
switch(scd)
 {
 case 75: //left
  Cursor(hof,cur-1);
  break;
 case 77: //right
  Cursor(hof,cur+1);
  break;
 case 82: //ins
  stat^=IS_OVER;
  Redraw(1);
  break;
 case 83: //del
  if(stat&IS_OVER)
   buffer[hof+cur]=(hof+cur>=bufnc)?'\0':' ';
  else
   {
   MoveMemory(buffer+hof+cur,buffer+hof+cur+1,bufsz-hof-cur-1);
   buffer[bufsz-1]='\0';
   }
  Lab_Lnk();
  Cursor(hof,cur);
  //if(styl&IL_NTFY&&INotify) INotify(this,IN_DEL);
  break;
 }
if(!key) 
 return 0;
else if(key==8) //BKSP
 {
 if(hof+cur==0) return 0;
 if(stat&IS_OVER)
  buffer[hof+cur-1]=(hof+cur>=bufnc)?'\0':' ';
 else
  {
  MoveMemory(buffer+hof+cur-1,buffer+hof+cur,bufsz-hof-cur);
  buffer[bufsz-1]='\0';
  }
 Lab_Lnk();
 Cursor(hof,cur-1);
 //if(styl&IL_NTFY&&INotify) INotify(this,IN_DEL);
 }
else if(hof+cur<bufsz) //character key
 {
 key=ValidKey(key);
 if(key<' ') return 0;
 if(!(stat&IS_OVER))
  MoveMemory(buffer+hof+cur+1,buffer+hof+cur,bufsz-hof-cur-1);
 buffer[hof+cur]=key;
 Lab_Lnk();
 Cursor(hof,cur+1);
 if(styl&IL_NTFY&&INotify) INotify(this,IN_WRI);
 }
return 1;
}

char UITEM::ValidKey(BYTE key)
{
if(type&IT_T)
 return key;
else if(type&IT_R)
 {
 if((key>='0'&&key<='9')||key=='.')
  return key;
 else if(key=='-'&&!(hof+cur))
  return key;
 }
else if(type&IT_N)
 {
 char max1=47+reg0,max2='A';
 if(max1>'9')
  {
  max1='9';
  max2='A'+reg0-10;
  }
 if(key>='0'&&key<=max1)
  return key;
 else if(key>='A'&&key<max2)
  return key;
 else if(key>='a'&&key<max2+32)
  return key-32;
 else if(key=='-'&&!(hof+cur))
  return key;
 }
return 0;
} 

//apperance

void UITEM::Resize()
{
KeepInside((RECT*)&l,uiform->l,uiform->u,uiform->r,uiform->d,uiform->uiskin->chw+1,uiform->uiskin->chh+1);
if(uiform->bimg&&styl&IL_BIMG)
 {
 if(r-l>uiform->bimg->lng) r=l+uiform->bimg->lng;
 if(d-u>uiform->bimg->lat) d=u+uiform->bimg->lat;
 }
if(styl&IL_GRID)
 {
 l=CutM(l,uiform->uiskin->gcw);
 r=CutM(r,uiform->uiskin->gcw);
 u=CutM(u,uiform->uiskin->gch);
 d=CutM(d,uiform->uiskin->gch);
 }
if(r-l<uiform->uiskin->cha) r=l+uiform->uiskin->cha;
if(d-u<uiform->uiskin->chh) d=u+uiform->uiskin->chh;
l1=l-uiform->uiskin->cha-1;
r1=l;
l2=r;
r2=r+uiform->uiskin->cha+1;
u1=u2=u;
d1=d2=d;
//labels
by=u;
ay=d;
if(!(styl&IL_BLP)) //H
 {
 balgn=TA_TOP|TA_RIGHT;
 bx=l1;
 by++;
 }
else if(styl&IL_BLA) //V:C
 {
 balgn=TA_BOTTOM|TA_CENTER;
 bx=(l+r)/2;
 }
else //V:L
 {
 balgn=TA_BOTTOM|TA_LEFT;
 bx=l;
 }
if(!(styl&IL_ALP)) //H
 {
 aalgn=TA_BOTTOM|TA_LEFT;
 ax=r2;
 ay--;
 }
else if(styl&IL_ALA) //V:C
 {
 aalgn=TA_TOP|TA_CENTER;
 ax=(l+r)/2;
 }
else //V:R
 {
 aalgn=TA_TOP|TA_RIGHT;
 ax=r;
 }
if(lstw<r-l) lstw=r-l;
Redraw(0);
}

void UITEM::Pos(int x=0,int y=0,int w=0,int h=0,int ix=0,int iy=0)
{
if(!w) w=uiform->uiskin->chw*bufsz;
if(!h) h=uiform->uiskin->chh+2;
l=x;
r=l+w;
u=y;
d=u+h;
//image
px=ix;
py=iy;
Resize();
}

void UITEM::Expand()
{
if(!its||(styl&IL_NE)) return;
stat|=IS_EXP|IS_EXPB;
vof=u-uiform->u; //space above label
nrl=uiform->d-d; //space below label
if(vof>nrl)
 {
 nrl=vof;
 stat&=~IS_EXPB;
 }
nrl/=uiform->uiskin->chh1; //numarul maxim de linii
el=el1=el2=(l+r-lstw)/2;
er=er1=er2=el+lstw;
if(nrl<nri) //need V arrows
 {
 stat|=IS_VAR;
 eu1=stat&IS_EXPB?d:u-nrl*uiform->uiskin->chh1;
 nrl--; 
 eu=ed1=eu1+(uiform->uiskin->chh1>>1);
 ed=eu2=eu+nrl*uiform->uiskin->chh1;
 ed2=ed+(uiform->uiskin->chh1>>1);
 }
else
 {
 stat&=~IS_VAR;
 nrl=nri;
 eu=eu1=eu2=stat&IS_EXPB?d:u-nrl*uiform->uiskin->chh1;
 ed=ed1=ed2=eu+nrl*uiform->uiskin->chh1;
 }
vof=sel;
if(vof>nri-nrl) vof=nri-nrl;
Redraw(0);
}

inline void UITEM::Hidexp()
{
if(stat&IS_EXP)
 {
 stat&=~IS_EXP;
 Redraw(0);
 }
}

void UITEM::Draw0()
{
if(!(stat&IS_FOC)) Lnk_Lab();
DWORD flags;
//draw labels
SelectObject(uiform->dc,uiform->uiskin->font[0]);
SetTextColor(uiform->dc,GetSysColor(COLOR_WINDOWTEXT));
if(!(stat&IS_DRWL))
 {
 if(bef)
  {
  SetTextAlign(uiform->dc,balgn);
  TextOut(uiform->dc,bx,by,bef,befnc);
  }
 if(aft)
  {
  SetTextAlign(uiform->dc,aalgn);
  TextOut(uiform->dc,ax,ay,aft,aftnc);
  }
 SetTextAlign(uiform->dc,TA_TOP|TA_LEFT);
 }
//main
flags=BF_RECT;
if(sel&&uiform->fimg&&styl&IL_FIMG)
 BitBlt(uiform->dc,l,u,r-l,d-u,uiform->fimg->mdc,px,py,SRCCOPY);
else if(uiform->bimg&&styl&IL_BIMG)
 BitBlt(uiform->dc,l,u,r-l,d-u,uiform->bimg->mdc,px,py,SRCCOPY);
else if(styl&IL_UBRS&&hib)
 FillRect(uiform->dc,(RECT*)&l,hib);
else if(stat&IS_NIR)
 FillRect(uiform->dc,(RECT*)&l,GetSysColorBrush(COLOR_INFOBK));
else if(stat&IS_HLIT)
 FillRect(uiform->dc,(RECT*)&l,GetSysColorBrush(COLOR_HIGHLIGHT));
else if(styl&IL_RO)
 FillRect(uiform->dc,(RECT*)&l,GetSysColorBrush(COLOR_BTNFACE));
else if(styl&IL_NE)
 FillRect(uiform->dc,(RECT*)&l,GetSysColorBrush(COLOR_BTNFACE));
else if(styl&IL_NW)
 FillRect(uiform->dc,(RECT*)&l,GetSysColorBrush(COLOR_MENU));
else
 FillRect(uiform->dc,(RECT*)&l,GetSysColorBrush(COLOR_WINDOW));
//borders
if(styl&IL_NBL) flags&=~BF_LEFT;
if(styl&IL_NBR) flags&=~BF_RIGHT;
if(styl&IL_NBU) flags&=~BF_TOP;
if(styl&IL_NBD) flags&=~BF_BOTTOM;
if(styl&IL_RO)
 DrawEdge(uiform->dc,(RECT*)&l,EDGE_ETCHED,flags);
else if(stat&IS_FOC)
 DrawEdge(uiform->dc,(RECT*)&l,EDGE_SUNKEN,flags);
else 
 DrawEdge(uiform->dc,(RECT*)&l,EDGE_RAISED,flags);
//h arrows
if(stat&IS_HAR) //show Harrows
 {
 DrawFrameControl(uiform->dc,(RECT*)&l1,DFC_SCROLL,DFCS_SCROLLLEFT);
 DrawFrameControl(uiform->dc,(RECT*)&l2,DFC_SCROLL,DFCS_SCROLLRIGHT);
 }
//text
if(stat&IS_NIR||styl&IL_RO) //err or static
 SetTextColor(uiform->dc,GetSysColor(COLOR_INFOTEXT));
else if(stat&IS_HLIT) //highlight
 SetTextColor(uiform->dc,GetSysColor(COLOR_HIGHLIGHTTEXT));
else if(styl&IL_NE||styl&IL_RO) //button
 SetTextColor(uiform->dc,GetSysColor(COLOR_BTNTEXT));
else if(styl&IL_NW) //menu
 SetTextColor(uiform->dc,GetSysColor(COLOR_MENUTEXT));
if(styl&IL_NW||!(stat&IS_FOC)) //edit
 flags=DT_CENTER;
else
 flags=0;
if(!(styl&IL_NTXT))
 DrawText(uiform->dc,buffer+hof,labnc,(RECT*)&l,flags|DT_VCENTER|DT_SINGLELINE);
//cursor
if(!(styl&IL_NW)&&stat&IS_FOC)
 {
 SetROP2(uiform->dc,R2_XORPEN);
 SelectObject(uiform->dc,GetStockObject(WHITE_PEN));
 GetTextExtentPoint32(uiform->dc,buffer+hof,cur,&uitsz);
 flags=l+uitsz.cx;
 if(stat&IS_OVER)
  {
  SelectObject(uiform->dc,GetStockObject(WHITE_BRUSH));
  GetTextExtentPoint32(uiform->dc,buffer+hof+cur,1,&uitsz);
  if(flags+uitsz.cx>r) uitsz.cx=r-flags;
  Rectangle(uiform->dc,flags,u+2,flags+uitsz.cx-1,d-2);
  }
 else
  {
  MoveToEx(uiform->dc,flags,u+2,NULL);
  LineTo(uiform->dc,flags,d-2);
  if(flags<r)
   {
   MoveToEx(uiform->dc,flags+1,u+2,NULL);
   LineTo(uiform->dc,flags+1,d-2);
   MoveToEx(uiform->dc,flags+2,u+2,NULL);
   LineTo(uiform->dc,flags+2,d-2);
   }
  }
 SetROP2(uiform->dc,R2_COPYPEN);
 }
}

//draw expansion
void UITEM::DrawEx0()
{
if(!(stat&IS_EXP)) return;
SelectObject(uiform->dc,uiform->uiskin->font[1]);
SetTextAlign(uiform->dc,TA_TOP|TA_CENTER);
SetTextColor(uiform->dc,GetSysColor(COLOR_MENUTEXT));
uitsz.cx=(el+er)>>1;
uitsz.cy=eu;
if(stat&IS_VAR) //show Varrows
 {
 DrawFrameControl(uiform->dc,(RECT*)&el1,DFC_SCROLL,DFCS_SCROLLUP);
 DrawFrameControl(uiform->dc,(RECT*)&el2,DFC_SCROLL,DFCS_SCROLLDOWN);
 }
DrawEdge(uiform->dc,(RECT*)&el,EDGE_RAISED,BF_RECT|BF_MIDDLE);
if(!(styl&IL_NP)&&sel-vof>=0&&sel-vof<nrl) //shadow
 {
 SelectObject(uiform->dc,uiform->uiskin->pen[0]);
 SelectObject(uiform->dc,GetSysColorBrush(COLOR_HIGHLIGHT));
 Rectangle(uiform->dc,el+2,eu+(sel-vof)*uiform->uiskin->chh1+2,er-1,eu+(sel-vof+1)*uiform->uiskin->chh1-1);
 }
for(int i=0;i<nrl;i++)
 {
 if(styl&IL_DIIS)
  SetTextColor(uiform->dc,its[vof+i].st?GetSysColor(COLOR_HIGHLIGHTTEXT):GetSysColor(COLOR_MENUTEXT));
 TextOut(uiform->dc,uitsz.cx,uitsz.cy,its[vof+i].it,its[vof+i].nc);
 uitsz.cy+=uiform->uiskin->chh1;
 }
}

void UITEM::Draw1()
{
if(!(stat&IS_FOC)) Lnk_Lab();
DWORD lind;
//draw labels
SelectObject(uiform->dc,uiform->uiskin->font[0]);
if(!(stat&IS_DRWL))
 {
 if(bef)
  {
  SetTextColor(uiform->dc,uiform->uiskin->color[14]);
  SetTextAlign(uiform->dc,balgn);
  TextOut(uiform->dc,bx,by,bef,befnc);
  }
 if(aft)
  {
  SetTextColor(uiform->dc,uiform->uiskin->color[15]);
  SetTextAlign(uiform->dc,aalgn);
  TextOut(uiform->dc,ax,ay,aft,aftnc);
  }
 SetTextAlign(uiform->dc,TA_TOP|TA_LEFT);
 }
//main
SetTextColor(uiform->dc,uiform->uiskin->color[0]);
if(sel&&uiform->fimg&&styl&IL_FIMG)
 BitBlt(uiform->dc,l,u,r-l+1,d-u+1,uiform->fimg->mdc,px,py,SRCCOPY);
else if(uiform->bimg&&styl&IL_BIMG)
 BitBlt(uiform->dc,l,u,r-l+1,d-u+1,uiform->bimg->mdc,px,py,SRCCOPY);
else if(styl&IL_UBRS&&hib)
 FillRect(uiform->dc,(RECT*)&l,hib);
else if(stat&IS_NIR)
 {
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[7]);
 SetTextColor(uiform->dc,uiform->uiskin->color[7]);
 }
else if(stat&IS_HLIT)
 {
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[6]);
 SetTextColor(uiform->dc,uiform->uiskin->color[6]);
 }
else if(styl&IL_RO)
 {
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[5]);
 SetTextColor(uiform->dc,uiform->uiskin->color[5]);
 }
else if(styl&IL_NE)
 {
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[4]);
 SetTextColor(uiform->dc,uiform->uiskin->color[4]);
 }
else if(styl&IL_NW)
 {
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[3]);
 SetTextColor(uiform->dc,uiform->uiskin->color[3]);
 }
else if(stat&IS_FOC)
 {
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[2]);
 SetTextColor(uiform->dc,uiform->uiskin->color[2]);
 }
else
 {
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[1]);
 SetTextColor(uiform->dc,uiform->uiskin->color[1]);
 }
//text
lind=(styl&IL_NW||!(stat&IS_FOC))?DT_CENTER:0;
if(!(styl&IL_NTXT))
 DrawText(uiform->dc,buffer+hof,labnc,(RECT*)&l,lind|DT_VCENTER|DT_SINGLELINE);
//cursor
if(!(styl&IL_NW)&&stat&IS_FOC)
 {
 SetROP2(uiform->dc,R2_XORPEN);
 SelectObject(uiform->dc,GetStockObject(WHITE_PEN));
 GetTextExtentPoint32(uiform->dc,buffer+hof,cur,&uitsz);
 lind=l+uitsz.cx;
 if(stat&IS_OVER)
  {
  SelectObject(uiform->dc,GetStockObject(WHITE_BRUSH));
  GetTextExtentPoint32(uiform->dc,buffer+hof+cur,1,&uitsz);
  if(lind+uitsz.cx>r) uitsz.cx=r-lind;
  Rectangle(uiform->dc,lind,u,lind+uitsz.cx-1,d);
  }
 else
  {
  MoveToEx(uiform->dc,lind,u,NULL);
  LineTo(uiform->dc,lind,d);
  if(lind<r)
   {
   MoveToEx(uiform->dc,lind+1,u,NULL);
   LineTo(uiform->dc,lind+1,d);
   }
  }
 SetROP2(uiform->dc,R2_COPYPEN);
 }
//borders and har
if(styl&IL_RO)
 SelectObject(uiform->dc,uiform->uiskin->pen[3]);
else if(stat&IS_FOC)
 SelectObject(uiform->dc,uiform->uiskin->pen[2]);
else
 SelectObject(uiform->dc,uiform->uiskin->pen[1]);
if(!(styl&IL_NBL))
 {
 MoveToEx(uiform->dc,l-1,d,NULL);
 LineTo(uiform->dc,l-1,u-1);
 }
if(!(styl&IL_NBR))
 {
 MoveToEx(uiform->dc,r,u-1,NULL);
 LineTo(uiform->dc,r,d);
 }
if(!(styl&IL_NBU))
 {
 MoveToEx(uiform->dc,l-1,u-1,NULL);
 LineTo(uiform->dc,r,u-1);
 }
if(!(styl&IL_NBD))
 {
 MoveToEx(uiform->dc,r,d,NULL);
 LineTo(uiform->dc,l-1,d);
 }
if(stat&IS_HAR)
 {
 Arc(uiform->dc,l2+l2-r2,u2,r2,d2,l2,d2,l2,u2);
 Arc(uiform->dc,l1,u1,r1+r1-l1,d1,r1,u1,r1,d1);
 }
}

void UITEM::DrawEx1()
{
if(!(stat&IS_EXP)) return;
SetTextAlign(uiform->dc,TA_TOP|TA_CENTER);
SelectObject(uiform->dc,uiform->uiskin->font[1]);
if(styl&IL_DIIS)
 {
 SelectObject(uiform->dc,uiform->uiskin->pen[11]);
 SelectObject(uiform->dc,uiform->uiskin->fill[9]);
 }
else if(styl&IL_NP)
 {
 SelectObject(uiform->dc,uiform->uiskin->pen[10]);
 SelectObject(uiform->dc,uiform->uiskin->fill[10]);
 SetTextColor(uiform->dc,uiform->uiskin->color[10]);
 }
else if(styl&IL_NW)
 {
 SelectObject(uiform->dc,uiform->uiskin->pen[9]);
 SelectObject(uiform->dc,uiform->uiskin->fill[9]);
 SetTextColor(uiform->dc,uiform->uiskin->color[9]);
 }
else
 {
 SelectObject(uiform->dc,uiform->uiskin->pen[8]);
 SelectObject(uiform->dc,uiform->uiskin->fill[8]);
 SetTextColor(uiform->dc,uiform->uiskin->color[8]);
 }
if(stat&IS_VAR) //show Varrows
 {
 Chord(uiform->dc,el1,eu1,er1,ed1+ed1-eu1,er1,ed1,el1,ed1);
 Chord(uiform->dc,el2,eu2+eu2-ed2,er2,ed2,el2,eu2,er1,eu2);
 Rectangle(uiform->dc,el,eu,er,ed+1);
 }
else
 RoundRect(uiform->dc,el,eu,er,ed,9,9);
uitsz.cx=(el+er)>>1;
uitsz.cy=eu;
if(!(styl&IL_NP)&&sel-vof>=0&&sel-vof<nrl) //shadow
 {
 SelectObject(uiform->dc,uiform->uiskin->pen[0]);
 SelectObject(uiform->dc,uiform->uiskin->fill[13]);
 Ellipse(uiform->dc,el+1,eu+(sel-vof)*uiform->uiskin->chh1+1,er,eu+(sel-vof+1)*uiform->uiskin->chh1);
 }
for(int i=0;i<nrl;i++)
 {
 if(styl&IL_DIIS) SetTextColor(uiform->dc,uiform->uiskin->color[its[vof+i].st?13:12]);
 TextOut(uiform->dc,uitsz.cx,uitsz.cy,its[vof+i].it,its[vof+i].nc);
 uitsz.cy+=uiform->uiskin->chh1;
 }
}

//0-whole window, 1-label, 2-label, 3-label+window
void UITEM::Redraw(int mod)
{
if(!mod)
 {
 InvalidateRect(uiform->wnd,NULL,0);
 return;
 }
uiform->CapDC(0);
if((mod&1)&&!(stat&IS_DRWL))
 {
 stat|=IS_DRWL;
 Draw();
 if(uiform->uitfoc)
  uiform->uitfoc->DrawEx();
 stat&=~IS_DRWL;
 }
if((mod&2)&&!(stat&IS_DRWE))
 {
 stat|=IS_DRWE;
 DrawEx();
 stat&=~IS_DRWE;
 }
uiform->RelDC();
}

void UITEM::Draw()
{
if(stat&IS_NVIS) return;
switch(uiform->uiskin->skin)
 {
 case ID_CLS:
  //Draw2();
  break;
 case ID_STD:
  Draw1();
  break;
 default:
  Draw0();
 }
if(styl&IL_APOP&&stat&IS_FOC)
 {
 if(uiform->uitfoc==this) uiform->uitfoc=NULL;
 stat&=~(IS_FOC|IS_HLIT|IS_EXP);
 Sleep(100);
 Redraw(1);
 }
}

void UITEM::DrawEx()
{
switch(uiform->uiskin->skin)
 {
 case ID_CLS:
  //DrawEx2();
  break;
 case ID_STD:
  DrawEx1();
  break;
 default:
  DrawEx0();
 }
}

//customize mode
void UITEM::Custom(int onf=1)
{
if(onf)
 {
 Hidexp();
 stat|=IS_CSTM;
 }
else
 {
 stat&=~(IS_CSTM|IS_FOC);
 }
Redraw(0);
}

void UITEM::DrawC()
{
DWORD flags;
//draw labels
SelectObject(uiform->dc,uiform->uiskin->font[0]);
SetTextColor(uiform->dc,GetSysColor(COLOR_WINDOWTEXT));
if(bef)
 {
 SetTextAlign(uiform->dc,balgn);
 TextOut(uiform->dc,bx,by,bef,befnc);
 }
if(aft)
 {
 SetTextAlign(uiform->dc,aalgn);
 TextOut(uiform->dc,ax,ay,aft,aftnc);
 }
SetTextAlign(uiform->dc,TA_TOP|TA_LEFT);
//main
if(sel&&uiform->fimg&&styl&IL_FIMG)
 BitBlt(uiform->dc,l,u,r-l,d-u,uiform->fimg->mdc,px,py,SRCCOPY);
else if(uiform->bimg&&styl&IL_BIMG)
 BitBlt(uiform->dc,l,u,r-l,d-u,uiform->bimg->mdc,px,py,SRCCOPY);
else if(styl&IL_RO)
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[5]);
else if(styl&IL_NE)
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[4]);
else if(styl&IL_NW)
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[3]);
else
 FillRect(uiform->dc,(RECT*)&l,uiform->uiskin->fill[1]);
//borders
r--; d--;
el=l+uidragbord; eu=u+uidragbord;
er=r-uidragbord; ed=d-uidragbord;
SelectObject(uiform->dc,uiform->uiskin->pen[stat&IS_FOC?3:4]);
MoveToEx(uiform->dc,el,d-1,NULL);
LineTo(uiform->dc,el,ed);
LineTo(uiform->dc,l,ed);
LineTo(uiform->dc,l,d);
LineTo(uiform->dc,er,d);
MoveToEx(uiform->dc,er,u+1,NULL);
LineTo(uiform->dc,er,eu);
LineTo(uiform->dc,r,eu);
LineTo(uiform->dc,r,u);
LineTo(uiform->dc,el,u);
SelectObject(uiform->dc,uiform->uiskin->pen[stat&IS_FOC?2:1]);
LineTo(uiform->dc,el,ed);
LineTo(uiform->dc,r,ed);
LineTo(uiform->dc,r,d);
LineTo(uiform->dc,er,d);
LineTo(uiform->dc,er,eu);
LineTo(uiform->dc,l,eu);
LineTo(uiform->dc,l,u);
LineTo(uiform->dc,el,u);
r++; d++;
//h arrows
flags=(styl&IL_NHAR)?DFCS_CAPTIONCLOSE|DFCS_PUSHED:DFCS_CAPTIONCLOSE;
DrawFrameControl(uiform->dc,(RECT*)&l1,DFC_CAPTION,flags);
DrawFrameControl(uiform->dc,(RECT*)&l2,DFC_CAPTION,flags);
//text
if(styl&IL_RO) //static
 SetTextColor(uiform->dc,GetSysColor(COLOR_INFOTEXT));
else if(styl&IL_NE||styl&IL_RO) //button
 SetTextColor(uiform->dc,GetSysColor(COLOR_BTNTEXT));
else if(styl&IL_NW) //menu
 SetTextColor(uiform->dc,GetSysColor(COLOR_MENUTEXT));
SelectObject(uiform->dc,uiform->uiskin->font[1]);
DrawText(uiform->dc,prop,prpnc,(RECT*)&l,DT_VCENTER|DT_SINGLELINE|DT_CENTER);
if(stat&IS_NVIS) InvertRect(uiform->dc,(RECT*)&l);
}

int UITEM::RunMC(int but=0,int mx=0,int my=0,int dz=0)
{
dz=PinRb(mx,my,l,u,r,d,uidragbord);
if(!dz) //RCT_OUT
 {
 if(PinR(mx,my,l1,u1,r1,d1)||PinR(mx,my,l2,u2,r2,d2))
  {
  if(styl&IL_NTHA) return 1;
  styl^=IL_NHAR;
  stat&=~IS_HAR;
  Redraw(0);
  return 1;
  }
 return 0;
 }
Focus(1);
if(but&uimb1)
 {    
 if(styl&IL_NMOV) return 1;
 if(dz==RCT_M) //move
  {
  l+=mouse.dx;
  r+=mouse.dx;
  u+=mouse.dy;
  d+=mouse.dy;
  }
 else //resize
  {
  if(styl&IL_NRSZ) return 1;
  if(dz&RCT_L) l+=mouse.dx;
  else if(dz&RCT_R) r+=mouse.dx;
  if(dz&RCT_U) u+=mouse.dy;
  else if(dz&RCT_D) d+=mouse.dy;
  }
 }
else if(but&uimb2) //slide image
 {
 if(styl&IL_NMIP) return 1;
 if(!uiform->bimg) return 0;
 if(dz==RCT_M)
  {
  px+=mouse.dx;
  py+=mouse.dx;
  }
 else
  {
  if(dz&RCT_L) px--;
  else if(dz&RCT_R) px++;
  if(dz&RCT_U) py--;
  else if(dz&RCT_D) py++;
  }
 if(px<0) px=0;
 if(py<0) py=0;
 if(px>(uiform->bimg->lng-r+l)) px=uiform->bimg->lng-r+l;
 if(py>(uiform->bimg->lat-d+u)) py=uiform->bimg->lat-d+u;
 uiform->CapDC(0);
 DrawC();
 uiform->RelDC();
 return 1;
 }
else if(but&uimb3)
 {    
 if(dz==RCT_M) //hide/show
  {
  stat^=IS_NVIS;
  uiform->CapDC(0);
  InvertRect(uiform->dc,(RECT*)&l);
  uiform->RelDC();
  return 1;
  }
 if(styl&IL_NMLP) return 1;
 if(dz&RCT_L)  //bef 
  {
  if(dz&RCT_U) //V:L
   {
   styl|=IL_BLP;
   styl&=~IL_BLA;
   }
  else //H
   styl&=~IL_BLP;
  }
 else if(dz&RCT_U) //V:C
  styl|=IL_BLP|IL_BLA;
 if(dz&RCT_R)  //aft
  {
  if(dz&RCT_D) //V:R
   {
   styl|=IL_ALP;
   styl&=~IL_ALA;
   }
  else //H
   styl&=~IL_ALP;
  }
 else if(dz&RCT_D) //V:C
  styl|=IL_ALP|IL_ALA;
 }
Resize();
return 1;
}
//UITEM>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
