#ifndef V_UINTERFACE_BASIC
#define V_UINTERFACE_BASIC

#include <winter.cpp>
#include <img.cpp>
#include <mmed.cpp>
//#include <vfile.cpp>

SIZE uitsz; //internal buffer
//RECT uirct; //internal buffer
int uimb1=MB_L,uimb2=MB_R,uimb3=MB_M;
int uidragbord=5; //default border size

//Types
#define IT_N 0x00f0 //number
#define IT_R 0x0006 //real

//aBCDeFghIjkLMNOPQRsTUvWXyz
#define IT_T 0x0001 //string (reg0==text formating flags)
#define IT_F 0x0002 //float
#define IT_D 0x0004 //double
#define IT_U 0x0008 //unsigned
#define IT_B 0x0010 //byte (reg0==radix)
#define IT_W 0x0020 //word (reg0==radix)
#define IT_I 0x0040 //int (reg0==radix)
#define IT_Q 0x0080 //quad (reg0==radix)
#define IT_M 0x0100 //menu (check box, button) returns state inf
#define IT_L 0x0200 //pointer list (reg0==base, sel==offset, granularity==4)
#define IT_P 0x0400 //push button (also a menu)
#define IT_C 0x0800 //return reg0 ?
#define IT_O 0x1000 //options (link==list base, linksz==item size, reg0==span between items)
#define IT_X 0x2000 //flags (link==bit base, reg0==base bit offset)

//Notifications
#define IN_SEL    0x00000001 //selection changed
#define IN_WRI    0x00000002 //user wrote something
#define IN_DEL    0x00000004 //user deleted something
#define IN_FOC    0x00000008 //got focus
#define IN_OK     0x00000010 //enter
#define IN_CANCEL 0x00000020 //esc
#define IN_ERROR  0x00000040 //error occured
#define IN_CLR    0x00000080 //new color
#define IN_SKIN   0x00000100 //new skin
#define IN_PALL   0x00000200 //new palette
#define IN_CSTM   0x00000400 //entered customizing mode
#define IN_LINK   0x00000800 //should relink (including reg0 for IT_L)
#define IN_NPOS   0x00001000 //changed size or position

#define IN_FORM   0x80000000 //sender is a form

#define IN_NEW    0x00000017 //changed
//Global flags
#define IF_SLCL  0x00000001 //persist localy
#define IF_LPOS  0x00000002 //load pos only
#define IF_CDIR  0x00000004 //use files only from current directory

typedef void*(*UINotify)(void*,DWORD);

class UITEM;


//Text Buf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class TextBuf
{
public:
 LPSTR text,line;
 char sep; //separator
 int tsz,lsz,nrl; //text byte size,strlen(line), nr of lines(items)
 int row,cur; //curent positon
 DWORD stat;
 LPSTR*item;

 TextBuf();
 void Init(char,char*,NAT,int);
 void Sync();
 void FromFile(LPSTR,char);
 void ToFile(LPSTR);
 void FromStg();
 void ToStg();
 void Free();
 void Sep(char);
 void Seek(int,int);
 void Cursor(int);
 void Push(LPSTR,NAT); //append to the end
 LPSTR Pull(LPSTR); //read current and alloc mem if none
 void Pop(LPSTR); //read last and del
 void Ins(LPSTR,NAT); //inserts
 void Del(NAT); //deletes
 void Cut(LPSTR,NAT); //cut
 void Copy(LPSTR,NAT); //copy
 void Paste(LPSTR,NAT); //overwrites
 void Get(int,LPSTR,char);
 void Put(int,LPSTR,char);
 ~TextBuf()
  {
  Free();
  }
};

#define TB_SEEK_REL 0x1

#define TB_IMEM  0x00000001 //manages memory

//TextBuf <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
TextBuf::TextBuf()
{
line=text=NULL;
tsz=0;
stat=0;
}

void TextBuf::Init(char lsep='\n',char*link=NULL,NAT inisz=0,int mod=0)
{
sep=lsep;
row=cur=0;
if(!inisz) //alloc an empty buffer
 {
 line=text=(LPSTR)malloc(4);
 tsz=lsz=nrl=0;
 stat|=TB_IMEM;
 return;
 }
tsz=inisz;
if(link)
 {
 if(mod) //copy
  {
  if(stat&TB_IMEM&&text)
   text=(LPSTR)realloc(text,tsz);
  else
   {
   text=(LPSTR)malloc(tsz);
   stat|=TB_IMEM;
   }
  CopyMemory(text,link,tsz);
  }
 else
  {
  if(stat&TB_IMEM&&text) free(text);
  text=link;
  stat&=~TB_IMEM;
  }
 }
else
 {
 text=(LPSTR)realloc(text,tsz);
 stat|=TB_IMEM;
 FillMemory(text,tsz,mod);
 }
line=text;
nrl=1+countch(sep,text,tsz);
lsz=firstch(sep,line,tsz,sep);
}

void TextBuf::Sync()
{
row=cur=0;
line=text;
nrl=1+countch(sep,text,tsz);
lsz=firstch(sep,line,tsz,sep);
}

void TextBuf::FromFile(LPSTR fn,char lsep='\n')
{
FILE*lfis;
lfis=FOPEN(fn,"rb");
if(!lfis) return;
sep=lsep;
fseek(lfis,0,SEEK_END);
tsz=ftell(lfis); //file size
if(stat&TB_IMEM&&text)
 text=(LPSTR)realloc(text,tsz);
else
 {
 text=(LPSTR)malloc(tsz);
 stat|=TB_IMEM;
 }
fseek(lfis,0,SEEK_SET);
fread(text,1,tsz,lfis);
fclose(lfis);
line=text;
row=cur=0;
nrl=1+countch(sep,text,tsz);
lsz=firstch(sep,line,tsz,sep);
}

void TextBuf::ToFile(LPSTR fn)
{
FILE*lfis;
lfis=FOPEN(fn,"wb");
if(!lfis) return;
fwrite(text,1,tsz,lfis);
//fputc(26,lfis); //EOF
fclose(lfis);
}

void TextBuf::Free()
{
if(stat&TB_IMEM&&text) free(text);
line=text=NULL;
nrl=tsz=lsz=0;
}

void TextBuf::Sep(char lsep)
{
swapch(sep,lsep,text,tsz,C_NULL);
sep=lsep;
}

void TextBuf::Seek(int lrow=0,int mod=0)
{
if(mod&TB_SEEK_REL)
 lrow+=row;
if(lrow==row)
 {
 lsz=firstch(sep,line,text+tsz-line,sep);
 return;
 }
if(lrow<0)
 {
 line=text;
 row=cur=0;
 }
else if(lrow>=nrl)
 {
 line=prevch(sep,text+tsz,1,tsz);
 row=nrl-1;
 cur=text+tsz-line;
 }
else if(lrow>row)
 {
 line=nextch(sep,line,lrow-row,text+tsz-line);
 row=lrow;
 cur=0;
 }
else if(lrow<row)
 {
 line=prevch(sep,line,row-lrow+1,line-text);
 row=lrow;
 cur=0;
 }
lsz=firstch(sep,line,text+tsz-line,sep);
}

inline void TextBuf::Cursor(int lcur)
{
cur=lcur;
if(cur>lsz) cur=lsz;
else if(cur<0) cur=0;
}

void TextBuf::Push(LPSTR sstr,NAT slen=0)
{
lsz=slen?slen:sl(sstr);
text=(LPSTR)realloc(text,tsz+lsz+1);
line=text+tsz;
if(line>text)
 {
 *line=sep;
 line++;
 tsz++;
 }
tsz+=lsz;
row=nrl;
cur=0;
nrl++;
if(sstr)
 nrl+=CopyMemCnt(line,sstr,lsz,sep);
}

void TextBuf::Pop(LPSTR dstr)
{
if(!text) return;
line=prevch(sep,text+tsz,1,tsz);
lsz=text+tsz-line;
sc(dstr,line,lsz,sep,0);
tsz-=lsz+1;
if(tsz>0)
 {
 text=(LPSTR)realloc(text,tsz);
 line=prevch(sep,text+tsz,1,tsz);
 nrl--;
 row=nrl-1;
 cur=0;
 }
else
 Free();
}

LPSTR TextBuf::Pull(LPSTR dstr=NULL)
{
if(!text) return NULL;
if(lsz)
 {
 if(!dstr) dstr=(LPSTR)malloc(lsz+1);
 sc(dstr,line,lsz,sep,0);
 }
else
 dstr=NULL;
line+=lsz+1;
if(line<=text+tsz)
 row++;
else
 {
 line=text;
 row=0;
 }
cur=0;
lsz=firstch(sep,line,text+tsz-line,sep);
return dstr;
}

void TextBuf::Ins(LPSTR sstr,NAT slen=1)
{
tsz+=slen;
text=(LPSTR)realloc(text,tsz);
ShiftMemR(line+cur,slen,text+tsz-line-cur-slen);
nrl+=CopyMemCnt(line+cur,sstr,slen,sep);
lsz=cur+firstch(sep,line+cur,text+tsz-line-cur,sep);
}

void TextBuf::Del(NAT slen=1)
{
lsz=text+tsz-line-cur;
if(slen>lsz) slen=lsz;
nrl-=MemCnt(sep,line+cur,slen);
ShiftMemL(line+cur+slen,slen,lsz-slen);
tsz-=slen;
text=(LPSTR)realloc(text,tsz);
lsz=text+tsz-line-cur,sep;
lsz=cur+firstch(sep,line+cur,text+tsz-line-cur,sep);
}

void TextBuf::Copy(LPSTR dstr,NAT slen=1)
{
if(!slen) slen=lsz-cur;
CopyMemory(dstr,line+cur,slen);
}

void TextBuf::Cut(LPSTR dstr,NAT slen=1)
{
Copy(dstr,slen);
Del(slen);
}

void TextBuf::Paste(LPSTR sstr,NAT slen=1)
{
if(slen>text+tsz-line-cur) slen=text+tsz-line-cur;
if(!slen) return;
nrl-=MemCnt(sep,line+cur,slen);
nrl+=CopyMemCnt(line+cur,sstr,slen,sep);
lsz=cur+firstch(sep,line+cur,text+tsz-line-cur,sep);
}

//TextBuf >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif