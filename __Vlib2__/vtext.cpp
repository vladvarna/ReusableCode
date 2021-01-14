#pragma once

#include <str.cpp>
#include <io.cpp>


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//CString like class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define VCHARS_CONST		0x1	//prevent memory deallocation
#define VCHARS_SELECTED     0x2 //select flag (used in an array of these)
class VCHARS
{
public:
 DWORD f; //flags
 int sz; //string length
 union
  {
  char*pc; //string
  void*pv;
  };

 VCHARS() { ZEROCLASS(VCHARS); }   //zero
 ~VCHARS() { Free(); } //copy size
 void Free() { sz=0; if(!(f&VCHARS_CONST)) FREE(pc); }
//copy string.......................................................................... 
 char* operator =(char*lstr)
  {
  if(!lstr) return NULL;
  sz=sl(lstr);
  if(!(f&VCHARS_CONST)) FREE(pc);
  pc=SALLOC(sz);
  f&=~VCHARS_CONST;
  if(!pv) return NULL;
  CopyMem(pv,lstr,sz);
  pc[sz]=0;
  return pc;
  }
//copy fixed(known) length string.......................................................................... 
 void copy(char*lstr,NAT lsize=0)
  {
  sz=lsize;
  pc=SALLOC(sz);
  f&=~VCHARS_CONST;
  if(!pv) return;
  CopyMem(pv,lstr,sz);
  pc[sz]=0;
  }
//insert sting:lstr of size:lsize at position:pos .......................................................................... 
 void ins(int pos,char*lstr,NAT lsize=0)
  {
  if(f&VCHARS_CONST) return;  //can't insert in const char
  if(pos>sz) pos=sz;
  if(!lsize) lsize=sl(lstr);
  pv=REALLOC(pv,sz+lsize+1);
  if(!pv) return;
  ShiftMemR(pc+pos,lsize,sz-pos);
  CopyMem(pc+pos,lstr,lsize);
  sz+=lsize;
  pc[sz]=0;
  }
//print string - mainly for debug ......................................................... 
 void show()
  {
  MessageBox(hmwnd,pc,"VCHARS",MB_OK);
  } 
};
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 
//text file parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class VText
{
public:
 DWORD stat; //b0=prevent memory deallocation
 VCHARS*line; //lines from file loaded in memory (lines are separated by 'sep')
 NAT nrlines; //number of lines loaded in memory
 char*sep; //separator between lines (must be a const)
 int sepnc; //length of separator
 int ctxbeg,ctxend; //range of active lines
 
 VText() { ZEROCLASS(VText); }
 ~VText() { Free(); }
 void Free();
 NAT Add(char*,NAT);
 BOOL Mem(char*,NAT,char*,DWORD);
 BOOL Load(char*,char*,DWORD,char*,NAT);
 BOOL Save(char*,char*,DWORD);
 void SelectAll();
 void Unselect();
 int SelectLines(NAT,NAT);
 int SelectC(char*,NAT);
 int SelectAX(int,char*,NAT);
 int Context(char*,NAT);
 int GetINT(char*,NAT);
 int GetINTs(int*,NAT,char*,NAT);
 int Get_INT(char*,NAT,char*,NAT);
 int Get_HEX(char*,NAT,char*,NAT,int);
 double Get_REAL(char*,NAT,char*,NAT);
 int GetSTR(char*,NAT,char*,NAT,char*,NAT,char*,NAT);
 int Get_STR(char*,char*,NAT,char*,NAT);
 int GetARRAYofINT(int,char*,NAT);
 int GetARRAYofINTs(int*,NAT,int,char*,NAT);
 int GetARRAYofSTR(char*,int,char*,NAT,char*,NAT);
 int GetDateTime(VTIME*,char*,NAT,char*,NAT,DWORD);
 void Show();
};

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//free memory......................................................................................................................
inline void VText::Free()
{
if(stat&0x1) return;
for(NAT l=0;l<nrlines;l++)
 {
 line[l].Free();
 }
FREE(line);
nrlines=ctxbeg=ctxend=0;
}

//add line at the end.......................................................................................................................
inline NAT VText::Add(char*lline,NAT llinesz=0)
{
if(ctxend==nrlines) ctxend++;
nrlines++;
line=(VCHARS*)REALLOC(line,nrlines*sizeof(VCHARS));
line[nrlines-1].copy(lline,llinesz);
return 1;
}

//initilize from a text buffer in memory..........................................................................................................................
BOOL VText::Mem(char*fbuf,NAT fsz,char*lsep,DWORD flags=0)
{ //flags: b0=remove empty lines (TODO)
NAT l,p=0;
if(lsep)
 {
 sep=lsep;
 sepnc=sl(sep);
 }
if(!fbuf) return 0;
if(!sep) return error("No separator in VText::Load()");
do{
 l=s_seq(fbuf+p,sep,sepnc,fsz-p);
 if(l==R_NULL) break;
 Add(fbuf+p,l);
 p+=l+sepnc;
 }while(p<fsz);
Add(fbuf+p,fsz-p);
SelectAll();
return nrlines;
}

//load a file, optionaly starting with "label" ..........................................................................................................................
BOOL VText::Load(char*path,char*lsep=NULL,DWORD flags=0,char*label=NULL,NAT labelnc=0)
{ //flags: b0=remove empty lines
IOSFile iof;
char*fbuf;
NAT fsz,p;
fbuf=(char*)iof.loadfile(path,&fsz);
if(!fbuf) return 0;
if(label) //
 {
 if(!labelnc) labelnc=sl(label);
 p=s_seqI(fbuf,label,labelnc,fsz);
 if(p!=R_NULL)
  Mem(fbuf+p,fsz-p,lsep,flags);
 else
  error("VText::Load() couldn't find trim string in file !!!");
 }
else
 Mem(fbuf,fsz,lsep,flags);
FREE(fbuf);
return nrlines;
}

//save in a text file...................................................................................................................
BOOL VText::Save(char*path,char*lterm=NULL,DWORD flags=0)
{
IOSFile iof;
NAT ltermnc;
if(iof.open(path,FU_W)) return 0;
iof.amask=0;
if(!lterm) lterm=sep;
ltermnc=sl(lterm);
for(NAT l=0;l<nrlines;l++)
 {
 iof.write(line[l].pc,line[l].sz);
 iof.write(lterm,ltermnc);
 }
//iof.write(sep,sepnc);	//ctrl-z
return nrlines;
}

//mark all lines as selected  ..................................................................................................................
inline void VText::SelectAll()
{
for(NAT l=0;l<nrlines;l++)
 line[l].f|=VCHARS_SELECTED;
}

//mark all lines as unselected...................................................................................................................
inline void VText::Unselect()
{
for(NAT l=0;l<nrlines;l++)
 line[l].f&=~VCHARS_SELECTED;
}

//mark lines in range as selected ...................................................................................................................
inline int VText::SelectLines(NAT l1=0,NAT l2=0x7fffffff)
{
ctxbeg=l1<=nrlines?l1:0;
ctxend=l2<=nrlines?l2:nrlines;
return l2-l1;
}

//select all lines who have "label" in them ..................................................................................................................
int VText::SelectC(char*label,NAT labelnc=0)
{
NAT p;
int match=0;
if(!labelnc) labelnc=sl(label);
for(NAT l=0;l<nrlines;l++)
 {
 line[l].f&=~VCHARS_SELECTED;
 p=s_seqI(line[l].pc,label,labelnc,line[l].sz);
 if(p!=R_NULL)
  {
  line[l].f|=VCHARS_SELECTED;
  match++;
  //line[l].show();
  }
 }
return match;
}

//select all lines who have "label [labelind]" in them (ex "BIN 01")...................................................................................................................
int VText::SelectAX(int labelind,char*label,NAT labelnc=0)
{
NAT p;
int match=0;
if(!labelnc) labelnc=sl(label);
for(NAT l=0;l<nrlines;l++)
 {
 line[l].f&=~VCHARS_SELECTED;
 p=s_seqI(line[l].pc,label,labelnc,line[l].sz);
 if(p!=R_NULL)
  {
  if(StoI(line[l].pc+p+labelnc,10,NULL,line[l].sz)==labelind)
   {
   line[l].f|=VCHARS_SELECTED;
   match++;
   //line[l].show();
   }
  }
 }
return match;
}

//finds a line who contains the string context ...................................................................................................................
int VText::Context(char*context,NAT contextnc=0)
{
NAT p;
if(!contextnc) contextnc=sl(context);
for(NAT l=0;l<nrlines;l++)
 {
 p=s_seqI(line[l].pc,context,contextnc,line[l].sz);
 if(p!=R_NULL)
  {
  return l;
  }
 } 
return -1;
}

//get number preceded by label (ex: "System ID #0345" returns 345)  ...................................................................................................................
int VText::GetINT(char*label,NAT labelnc=0)
{//uses selection
NAT p;
if(!labelnc) labelnc=sl(label);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 if(line[l].f&VCHARS_SELECTED)
  {
  p=s_seqI(line[l].pc,label,labelnc,line[l].sz);
  if(p!=R_NULL)
   {
   return StoI(line[l].pc+p+labelnc,10,NULL,line[l].sz);
   }
  }
 }
return 0;
}

//get numbers preceded by label (ex: "Total 1000 600 300 100" returns int[]={1000,600,300,100} )  ...................................................................................................................
int VText::GetINTs(int*res,NAT nri,char*label,NAT labelnc=0)
{//uses selection
NAT p;
char*mark;
if(!labelnc) labelnc=sl(label);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 if(line[l].f&VCHARS_SELECTED)
  {
  p=s_seqI(line[l].pc,label,labelnc,line[l].sz);
  if(p!=R_NULL)
   {
   mark=line[l].pc+p+labelnc;
   while(mark<line[l].pc+line[l].sz&&nri>0)
    {
    *res=StoF(mark,10,2,1,line[l].sz,&mark);
    res++;
    nri--;
    }
   }
  }
 }
return nri;
}

//get number, in any radix, preceded by label and start (ex: "System ID=#0345" returns 345)  ...................................................................................................................
int VText::Get_HEX(char*label,NAT labelnc=0,char*start=NULL,NAT startnc=0,int radix=16)
{ //uses selection
NAT p,d;
if(!labelnc) labelnc=sl(label);
if(!startnc) startnc=sl(start);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 if(line[l].f&VCHARS_SELECTED)
  {
  p=s_seqI(line[l].pc,label,labelnc,line[l].sz);
  if(p!=R_NULL)
   {
   p+=labelnc;
   d=s_seqI(line[l].pc+p,start,startnc,line[l].sz-p);
   if(d!=R_NULL)
    {
    d+=startnc+p;
    return StoI(line[l].pc+d,radix,NULL,line[l].sz-d);
    }
   }
  }
 }
return 0;
}

//get number preceded by label and start (ex: "System ID=#0345" returns 345)  ...................................................................................................................
int VText::Get_INT(char*label,NAT labelnc=0,char*start=NULL,NAT nrstch=0)
{//uses selection
NAT p,d;
if(!labelnc) labelnc=sl(label);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 if(line[l].f&VCHARS_SELECTED)
  {
  p=s_seqI(line[l].pc,label,labelnc,line[l].sz);
  if(p!=R_NULL)
   {
   p+=labelnc;
   d=s_anych(line[l].pc+p,start,nrstch,line[l].sz-p);
   if(d!=R_NULL)
    {
    return StoI(line[l].pc+p+d+1,10,NULL,line[l].sz-p-d);
    }
   }
  }
 }
return 0;
}

//get real number preceded by label and start (ex: "Procent= 5.5%" returns 5.5)  ...................................................................................................................
double VText::Get_REAL(char*label,NAT labelnc=0,char*start=NULL,NAT nrstch=0)
{//uses selection
NAT p,d;
if(!labelnc) labelnc=sl(label);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 if(line[l].f&VCHARS_SELECTED)
  {
  p=s_seqI(line[l].pc,label,labelnc,line[l].sz);
  if(p!=R_NULL)
   {
   p+=labelnc;
   d=s_anych(line[l].pc+p,start,nrstch,line[l].sz-p);
   if(d!=R_NULL)
    {
    return StoR(line[l].pc+p+d+1,10.,line[l].sz-p-d);
    }
   }
  }
 }
return 0;
}

//get string preceded by label and start (ex: "Operator = vlad" returns "vlad")  ...................................................................................................................
int VText::GetSTR(char*retbuf,NAT maxsz,char*label,NAT labelnc,char*start,NAT nrstch=0,char*end=NULL,NAT endnc=0)
{
NAT p=0,d,e;
if(!labelnc) labelnc=sl(label);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 d=s_seqI(line[l].pc,label,labelnc,line[l].sz);
 if(d!=R_NULL)
  {
  d+=labelnc;
  p=s_anych(line[l].pc+d,start,nrstch,line[l].sz-d);
  if(p!=R_NULL)
   {
   p+=d+1;
   if(end)
    {
    e=s_seqI(line[l].pc+p,end,endnc,line[l].sz-p);
    if(e==R_NULL) e=line[l].sz;
    }
   else 
    e=line[l].sz;
   d=sc(retbuf,line[l].pc+p,maxsz<=e-p?maxsz:e-p);
   return TrimStr(0,32,retbuf,d);
   }
  }
 }
return 0;
}

//get string preceded by label and start (ex: "Operator = vlad" returns "vlad")  ...................................................................................................................
int VText::Get_STR(char*retbuf,char*start,NAT nrstch=0,char*end=NULL,NAT endnc=0)
{//uses selection
NAT p,e,t=0;
int match=0;
if(!nrstch) nrstch=sl(start);
if(!endnc) endnc=sl(end);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 if(line[l].f&VCHARS_SELECTED)
  {
  p=s_anych(line[l].pc,start,nrstch,line[l].sz);
  if(p!=R_NULL)
   {
   match++;
   e=s_seqI(line[l].pc+p+1,end,endnc,line[l].sz-p-1);
   if(e==R_NULL) e=line[l].sz;
   CopyMemory(retbuf+t,line[l].pc+p+1,e-p-1);
   t+=e-p;
   retbuf[t-1]=',';
   }
  }
 }
if(t>0)
 retbuf[t-1]='\0';
else
 sc(retbuf,"N/A");
return match;
}

//this uses a label+index to search...................................................................................................................
int VText::GetARRAYofINT(int labelind,char*label,NAT labelnc)
{
NAT p;
char *atf;
if(!labelnc) labelnc=sl(label);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 p=s_seqI(line[l].pc,label,labelnc,line[l].sz);
 if(p!=R_NULL)
  {
  if(StoI(line[l].pc+p+labelnc,10,&atf,line[l].sz)==labelind)
   {
   return StoI(atf,10,NULL,line[l].sz-(atf-line[l].pc));
   }
  }
 }
return 0;
}

//this uses a label+index to search...................................................................................................................
int VText::GetARRAYofINTs(int*res,NAT nri,int labelind,char*label,NAT labelnc)
{
NAT p;
char *mark;
if(!labelnc) labelnc=sl(label);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 p=s_seqI(line[l].pc,label,labelnc,line[l].sz);
 if(p!=R_NULL)
  {
  if(StoI(line[l].pc+p+labelnc,10,&mark,line[l].sz)==labelind)
   {
   while(mark<line[l].pc+line[l].sz&&nri>0)
    {
    *res=StoF(mark,10,2,1,line[l].sz,&mark);
    res++;
    nri--;
    }
   }
  }
 }
return nri;
}

//this uses a label+index to search...................................................................................................................
int VText::GetARRAYofSTR(char*retbuf,int labelind,char*label,NAT labelnc,char*start,NAT nrstch=0)
{
NAT p,d,s,e,t=0;
int match=0;
if(!labelnc) labelnc=sl(label);
if(!nrstch) nrstch=sl(start);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 p=0;
 for(;;)
  {
  d=s_seqI(line[l].pc+p,label,labelnc,line[l].sz-p);
  if(d==R_NULL)
   break;
  else
   {
   p+=d+labelnc;
   if(StoI(line[l].pc+p,10,NULL,line[l].sz-p)==labelind)
    {
    s=s_anych(line[l].pc+p,start,nrstch,line[l].sz-p);
    if(s!=R_NULL)
     {
     match++;
     s+=p+1;
     e=s_seqI(line[l].pc+s,label,labelnc,line[l].sz-s);
     if(e==R_NULL) e=line[l].sz-s;
     CopyMemory(retbuf+t,line[l].pc+s,e);
     t+=e;
     retbuf[t++]=',';
     }
    }
   }
  }
 }
if(t>0)
 retbuf[t-1]='\0';
else
 sc(retbuf,"N/A");
return match;
}

//get DATE/TIME preceded by label and start...................................................................................................................
int VText::GetDateTime(VTIME*ptm,char*label,NAT labelnc,char*start,NAT nrstch=0,DWORD order=0x00123456)
{ //order fomat: Y=6 M=5 D=4 h=3 m=2 s=1 ms=0
NAT p=0,d,e;
NAT dt[7],M;
int i;
char*mark;
if(!labelnc) labelnc=sl(label);
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 d=s_seqI(line[l].pc,label,labelnc,line[l].sz);
 if(d!=R_NULL)
  {
  d+=labelnc;
  p=s_anych(line[l].pc+d,start,nrstch,line[l].sz-d);
  if(p!=R_NULL)
   {
   p+=d+1;
   ptm->getDate((int&)dt[6],dt[5],dt[4],i);
   ptm->getTime(dt[3],dt[2],dt[1],dt[0]);
   dt[5]++;
   if(order&0x80000000) //search literal month
    {
    for(M=0;M<12;M++)
     if(s_seqI(line[l].pc+p,MonthName[M],sl(MonthName[M]),line[l].sz-p)!=R_NULL)
      {
      dt[5]=M+1;
      goto LFound;
      }
    for(M=0;M<12;M++) //search only for first 3 letters in month name
     if(s_seqI(line[l].pc+p,MonthName[M],3,line[l].sz-p)!=R_NULL)
      {
      dt[5]=M+1;
      goto LFound;
      }
    }
LFound:
   mark=line[l].pc+p;
   i=0;
   while((mark<line[l].pc+line[l].sz)&&(i<7))
    {
    if(((order>>(i<<2))&0xf)<7)
     dt[((order>>(i<<2))&0xf)]=StoI(mark,10,&mark,line[l].sz-(mark-line[l].pc));
    i++;
    }
   ptm->setDate(dt[6],dt[5]-1,dt[4],0);
   ptm->setTime(dt[3],dt[2],dt[1],dt[0]);
   return 1;
   }
  }
 }
return 0;
}

#ifdef _DEBUG
//print all lines - for debug only...................................................................................................................
void VText::Show()
{
char lsbuf[6000];
NAT p=0;
for(NAT l=ctxbeg;l<ctxend;l++)
 {
 if(!line[l].sz) continue;
 CopyMemory(REGs,line[l].pc,line[l].sz);
 REGs[line[l].sz]=0;
 //MessageBox(hmwnd,REGs,"VText",MB_OK);
 p+=sprintf(lsbuf+p,">%s< \n",REGs);
 if(p>4000)
  {
  MessageBox(hmwnd,lsbuf,"VText",MB_OK);
  p=0;
  }
 }
if(p) MessageBox(hmwnd,lsbuf,"VText",MB_OK);
}
#endif 
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 
