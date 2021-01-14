#ifndef V_DATA_LOG
#define V_DATA_LOG

#include <mmed.cpp>

#define ID_DATALOGCHILD        1801

#define CACHE_CHARS			0x1	//add buffer as chars
#define CACHE_TIMES			0x2	//add time stamps
#define CACHE_HEX 			0x4	//add as hex values
#define CACHE_DEC			0x8	//add as decimal values
#define CACHE_TRIM		   0x10	//add as chars (0-32,127-255 trimed)

#define DATALOG_ADD			0x1

#define DLOG_NO_APPEND		0x1	//always create new entry
#define DLOG_APPEND_BY_ID   0x2	//create new entry only if diffrent id
#define DLOG_APPEND_BY_TM   0x4	//create new entry only if diffrent time
#define DLOG_APPEND_BY_SEP  0x8	//create new entry only if separators sound

int DATA_ENTRY_GROW=16;
int MonitorSenzitivity=10; //min time in ms between succesive reads
int MinTimeDraw=66; //in ms
struct DATA_ENTRY;

#ifndef V_REPLACE_CHAR
#define V_REPLACE_CHAR '*'
#endif
//Data logger~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class DataLog
{
public:
 NAT nre; //number of entries
 DATA_ENTRY*de; //entries
 DATA_ENTRY*lde; //last data entry
 
 HWND hown,hwnd; //window used to display log, and create
 HFONT hfnt; //font used
 HBRUSH hbrs; //brush used for background
 COLORREF background; //text and non text background colors
 WStatusBar*sbar; //status bar used in the log window
 RECT box; //window rect
 SIZE chsz; //char cell size
 DWORD stat; //0x1 -Log/NOlog, 0x2-window/combo 
 DWORD dirty; //log needs redrawing
 int first,indent; //first line visible, first character in line visible
 int vislin,viscol; //number of visible lines, number of visible columns
 MSTimer dlt; //timer used to limit FPS (draws/second)
 int defcachemod; //
 NAT maxentries; //after this number it will loose first entries
 
 DataLog() { ZEROCLASS(DataLog); maxentries=0xffffffff; }
 ~DataLog() { Free(); }
 void Delete(NAT,NAT);
 void Clear();
 void Free();
 int Add(DWORD,VTIME*,void*,NAT,int,int);
 int Msg(DWORD,VTIME*,char*,...);
 int Str(DWORD,char*,NAT,int);
 int Recache(DWORD,int);
 void Init(HWND,WStatusBar*,COLORREF,int);
 void Font(HFONT);

 void Resize(int,int,int,int);
 void Scroll(int,int);
 void Hover(int,int);
 BOOL LastEntry(LPSTR,NAT,DWORD);
 void Combo(HWND,DWORD);
 void Show();
 void Step();  //one iteration
 void Redraw();
 LRESULT CALLBACK Respond(HWND,UINT,WPARAM,LPARAM);
 BOOL BitUsage(DWORD,DWORD,int);
 BOOL SearchChar(DWORD,DWORD,char);
 BOOL SearchFileASCII(LPSTR,DWORD,DWORD,VTIME*);
 BOOL SaveCache(LPSTR,int);
 BOOL LoadTXT(DWORD,LPSTR,int);
 BOOL SaveData(LPSTR,FLAGS);
 BOOL LoadData(LPSTR,int);
}dlog;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct DATA_ENTRY
{
 DWORD id; //also COLORREF
 VTIME tm; //time 
 NAT szB; //in B
 union { void*pv; char*pc; BYTE*pb; short*ps; WORD*pw; DWORD*pdw; int*pi; unsigned*pu; __int64*pqw; float*pf; double*pd; void**pp; };
 char*cache;
 NAT nc; //cache size

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 DATA_ENTRY()
  {
  Zero();
  }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 ~DATA_ENTRY()
  {
  Free();
  }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 void Zero()
  {
  ZeroMemory(this,sizeof(DATA_ENTRY));
  }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 void Free()
  {
  if(cache!=pc) FREE(cache);
  FREE(pv);
  }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 void Alloc(NAT bytes=0)
  {
  FREE(pv);
  pv=ALLOC(bytes);
  szB=bytes;
  }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 void Realloc(NAT bytes=0)
  {
  if(MEMSZ(pv)<bytes) pv=REALLOC(pv,bytes+DATA_ENTRY_GROW);
  szB=bytes;
  }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 void Compact()
  {
  pv=REALLOC(pv,szB);
  if(cache!=pc) cache=(char*)REALLOC(cache,nc+1);
  }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 void Cache(int mod)
  {
  if(!mod) //no cache
   {
   if(cache!=pc) FREE(cache);
   cache=pc;
   nc=szB;
   return;
   }
  if(cache==pc) cache=NULL;
  cache=(char*)REALLOC(cache,szB*8+BUFFSZ);
  nc=0;
  if(mod&CACHE_TIMES) //time stamps
   {
   NAT h=0,m=0,s=0,ms=0;
   tm.getTime(h,m,s,ms);
   nc+=sprintf(cache+nc,"[%2i:%2i:%2i:%3i] ",h,m,s,ms);
   }
  if(mod&CACHE_CHARS) //filtered chars
   {
   nc+=sprintf(cache+nc,">");
   CopyMemory(cache+nc,pv,szB);
   FilterStr(0,31,V_REPLACE_CHAR,cache+nc,szB,'\0');
   nc+=szB;
   nc+=sprintf(cache+nc,"<");
   }
  else if(mod&CACHE_TRIM) //trimed chars
   {
   CopyMemory(cache+nc,pv,szB);
   nc+=TrimStr(0,31,cache+nc,TrimCh('\x7f',cache+nc,szB,'\0'),'\0');
   }
  if(mod&CACHE_HEX) //hex
   {
   nc+=sprintf(cache+nc," {");
   for(int b=0;b<szB;b++)
	{
    nc+=sprintf(cache+nc,"%X,",pb[b]);
    }
   nc+=sprintf(cache+nc-1,"}")-1;
   cache[nc]=0;
   }
  if(mod&CACHE_DEC) //dec
   {
   nc+=sprintf(cache+nc," (");
   for(int b=0;b<szB;b++)
	{
    nc+=sprintf(cache+nc,"%u,",pb[b]);
    }
   nc+=sprintf(cache+nc-1,")")-1;
   cache[nc]=0;
   }
  }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 void Append(void*pmem,NAT bytes=4)
  {
  if(MEMSZ(pv)<szB+bytes) 
   {
   if(cache==pc)
    {
    pv=REALLOC(pv,bytes+szB+DATA_ENTRY_GROW);
    cache=pc;
    }
   else
    pv=REALLOC(pv,bytes+szB+DATA_ENTRY_GROW);
   }
  CopyMemory(pb+szB,pmem,bytes);
  szB+=bytes;
  }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 void Create(DWORD lid,VTIME*ptm,void*pmem,NAT bytes=4)
  {
  id=lid;
  if(ptm)
   CopyMemory(&tm,ptm,sizeof(VTIME));
  else
   {
   tm.init(VTMF_VLAD);
   tm.Now();
   }
  pv=REALLOC(pv,bytes+DATA_ENTRY_GROW);
  CopyMemory(pv,pmem,bytes);
  szB=bytes;
  }
};

////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK DataLogProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{                         
DataLog*pdatlog;
pdatlog=(DataLog*)GetWindowLong(hwnd,GWL_USERDATA);
if(pdatlog)
 return pdatlog->Respond(hwnd,message,wparam,lparam);
else 
 return DefWindowProc(hwnd,message,wparam,lparam);
}

//..................................................................................................................................
int DataLog::Add(DWORD id,VTIME*ptm,void*buffer,NAT bufsz,int append=DLOG_NO_APPEND,int cachemod=0)
 {
 int canapnd=1;
 if(!buffer||!bufsz) return -1;
 if(!(stat&DATALOG_ADD)) return 0;
 if(!append) error("!append");
 if((append&DLOG_NO_APPEND)||!lde) //determine if it can be appended
  canapnd=0;
 else
  {
  if(append&DLOG_APPEND_BY_ID)
   {
   if(lde->id!=id)  //not same id
	canapnd=0;
   }
  if(append&DLOG_APPEND_BY_TM)
   {
   if(ptm->D_ms(&lde->tm) > MonitorSenzitivity) //timeout
	canapnd=0;
   }
  }
 //canapnd=0;//turn off auto appending
 if(canapnd)
  {
  lde->Append(buffer,bufsz);
  }
 else
  {
  if(nre>=maxentries)
   Delete(0,maxentries>>1);
  de=(DATA_ENTRY*)REALLOC(de,(nre+1)*sizeof(DATA_ENTRY));
  de[nre].Zero();
  de[nre].Create(id,ptm,buffer,bufsz);
  lde=&de[nre];
  nre++;
  }
 lde->Cache(cachemod?cachemod:defcachemod);
 SetScrollRange(hwnd,SB_VERT,0,nre-1,0);
 if(vislin&&first+vislin<nre) first=nre-vislin;
 SetScrollPos(hwnd,SB_VERT,first,1);
 dirty=1;
 return append;
 }

//--------------------------------------------------------------------------------------------------------
int DataLog::Msg(DWORD id,VTIME*ptm,char *formstr,...)
 {
 if(!(stat&DATALOG_ADD)) return 0;
 char buffer[BUFFSZ];
 va_list vparam;
 va_start(vparam,formstr);
 REGu=vsprintf(buffer,formstr,vparam);
 Add(id,ptm,buffer,REGu,DLOG_NO_APPEND,0);
 va_end(vparam);
 return REGu;
 }

//--------------------------------------------------------------------------------------------------------
int DataLog::Str(DWORD id,char *lstr,NAT lstrnc=0,int cachemod=0)
 {
 if(!(stat&DATALOG_ADD)) return 0;
 return Add(id,NULL,lstr,lstrnc?lstrnc:sl(lstr),DLOG_NO_APPEND,cachemod);
 }

//--------------------------------------------------------------------------------------------------------
int DataLog::Recache(DWORD id,int ncachemod=0)
{
int n=0;
defcachemod=ncachemod;
for(int i=0;i<nre;i++)
 {
 if(!id||de[i].id==id)
  {
  de[i].Cache(ncachemod);
  de[i].Compact();
  n++;
  }
 }
dirty=1;
return n;
}

//..................................................................................................................................
void DataLog::Delete(NAT pos=0,NAT cnt=1)
{
if(pos>=nre) return;
if(pos+cnt>=nre) cnt=nre-pos;
for(int i=0;i<cnt;i++)
 de[pos+i].Free();
ShiftMemL(de+(pos+cnt),cnt*sizeof(DATA_ENTRY),(nre-pos-cnt)*sizeof(DATA_ENTRY));
nre-=cnt;
if(nre)
 {
 lde=&de[nre];
 //de=(DATA_ENTRY*)REALLOC(de,nre*sizeof(DATA_ENTRY));
 SetScrollRange(hwnd,SB_VERT,0,nre-1,0);
 first=CLAMP(first-cnt,0,nre);
 SetScrollPos(hwnd,SB_VERT,first,1);
 }
else
 {
 lde=NULL;
 first=0;
 }
indent=0;
dirty=1;
//InvalidateRect(hwnd,NULL,1);
}

//..................................................................................................................................
void DataLog::Clear()
 {
 lde=NULL;
 for(int i=0;i<nre;i++)
  de[i].Free();
 FREE(de);
 nre=0;
 first=0;
 indent=0;
 dirty=1;
 //InvalidateRect(hwnd,NULL,1);
 }

//..................................................................................................................................
void DataLog::Free()
 {
 stat=0;
 Clear();
 if(hbrs) DeleteObject(hbrs);
 hbrs=NULL;
 if(hfnt) DeleteObject(hfnt);
 hfnt=NULL;
 if(hwnd) DestroyWindow(hwnd);
 hwnd=NULL;
 }

//..................................................................................................................................
void DataLog::Init(HWND hparent,WStatusBar*lsbar=NULL,COLORREF lbackground=0xa0a0a0,int ldefcm=0)
{
hown=hparent?hparent:hmwnd;
sbar=lsbar?lsbar:&wsb;
defcachemod=ldefcm;
WindowClass("WC_DATA_LOGGER",DataLogProc,CS_HREDRAW|CS_VREDRAW,NULL,LoadCursor(NULL,IDC_CROSS),NULL,NULL);
hwnd=CreateWindowEx(WS_EX_CLIENTEDGE,"WC_DATA_LOGGER","DATA LOGGER",
                    WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,
                    0,0,100,100,hown,(HMENU)ID_DATALOGCHILD,appinst,NULL);
SetWindowLong(hwnd,GWL_USERDATA,(long)this);
Font(CreateFont(-16,0,0,0,400,0,0,0,
                ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,
                FF_DONTCARE|FIXED_PITCH,"Courier New"));
hbrs=CreateSolidBrush(background=lbackground);
stat|=DATALOG_ADD;
Show();
}

//..................................................................................................................................
void DataLog::Font(HFONT hnfont=NULL)
{
RECT lr;
if(hnfont)
 {
 if(hfnt) DeleteObject(hfnt);
 hfnt=hnfont;
 }
else
 hfnt=FontDlg(hfnt,NULL,hwnd,CF_FIXEDPITCHONLY); 
HDC hdc;
hdc=GetDC(hwnd);
SelectObject(hdc,hfnt);
GetTextExtentPoint32(hdc,"M",1,&chsz);
ReleaseDC(hwnd,hdc);
GetWindowRect(hwnd,&lr);
ScreenToClient(hown,(POINT*)&lr.left);
ScreenToClient(hown,(POINT*)&lr.right);
Resize(lr.left,lr.top,lr.right,lr.bottom);
}

//..................................................................................................................................
void DataLog::Resize(int x1,int y1,int x2,int y2)
 {
 vislin=(y2-y1)/chsz.cy;
 viscol=(x2-x1)/chsz.cx;
 MoveWindow(hwnd,x1,y1,x2-x1,vislin*chsz.cy+4,1); //align to char cell (with client edge)
 GetClientRect(hwnd,&box);
 dirty=1;
 //InvalidateRect(hown,NULL,1);
 }

//..................................................................................................................................
void DataLog::Scroll(int vd=0,int hd=0)
{
if(!nre) return;
if(vd)
 {
 if(first+vd>=0&&first+vd<(int)nre)
  first+=vd;
 else if(first+vd>=(int)nre)
  first=nre-1;
 else// if(first+vd<0)
  first=0;
 SetScrollPos(hwnd,SB_VERT,first,1);
 dirty=1;
 }
if(hd)
 {
 if(indent+hd>=0)
  {
  indent+=hd;
  dirty=1;
  //InvalidateRect(hwnd,NULL,1);
  }
 } 
}

//..................................................................................................................................
void DataLog::Hover(int x=0,int y=0)
{
int i,j;
i=x/chsz.cx+indent;
j=y/chsz.cy+first;
if(j>=nre)
 {
 sbar->stat(0,"%u entries",nre);
 return;
 }
if(i<de[j].nc)
 sbar->stat(0,"(%i,%i) %Xh=%u='%c'",j,i,(BYTE)de[j].cache[i],(BYTE)de[j].cache[i],de[j].cache[i]);
else
 {
 NAT h=0,m=0,s=0,ms=0;
 de[j].tm.getTime(h,m,s,ms);
 sbar->stat(0,"(%i,%u) [%i:%i:%i:%i]",j,de[j].szB,h,m,s,ms);
 }
}

//..................................................................................................................................
BOOL DataLog::LastEntry(LPSTR entrycache,NAT maxnc,DWORD id=0)
{
for(int line=nre-1;line>=0;line--)
 {
 if(de[line].id==id)
  {
  sc(entrycache,de[line].cache,de[line].nc<maxnc?de[line].nc:maxnc);
  return 1;
  }
 }
return 0;
}

//..................................................................................................................................
void DataLog::Combo(HWND hcb,DWORD id=0)
{
char lsbuf[PATHSZ];
SendMessage(hcb,CB_RESETCONTENT,0,0); //remove all
for(int line=nre-1;line>=0;line--)
 {
 if(de[line].id==id)
  {
  sc(lsbuf,de[line].cache,de[line].nc<PATHSZ?de[line].nc:PATHSZ);
  SendMessage(hcb,CB_ADDSTRING,0,(LPARAM)lsbuf); //add
  }
 }
SendMessage(hcb,CB_SETCURSEL,(WPARAM)0,0); //select last
}

//..................................................................................................................................
void DataLog::Show()
{
RECT lr;
int line;
HDC hdc;
hdc=GetDC(hwnd);
//DefWindowProc(hwnd,WM_ERASEBKGND,(WPARAM)hdc,0);
SelectObject(hdc,hfnt);
//SetBkColor(hdc,background);
SetBkMode(hdc,TRANSPARENT);
lr.left=box.left;
lr.right=box.right;
lr.top=box.top;
lr.bottom=lr.top+chsz.cy;
line=first;
while(lr.bottom<=box.bottom)
 {
 FillRect(hdc,&lr,hbrs);  //clear line
 if((line<nre)&&((int)de[line].nc-indent>0))
  {
  SetTextColor(hdc,de[line].id);
  TextOut(hdc,lr.left,lr.top,de[line].cache+indent,de[line].nc-indent);
  //DrawText(hdc,de[line].cache+indent,de[line].nc-indent,&lr,DT_EDITCONTROL|DT_NOPREFIX);
  }
 lr.top+=chsz.cy;
 lr.bottom+=chsz.cy;
 line++;
 }
ReleaseDC(hwnd,hdc);
ValidateRect(hwnd,NULL);
dlt.Abs();
dirty=0;
}

//..................................................................................................................................
LRESULT CALLBACK DataLog::Respond(HWND lhwnd,UINT message,WPARAM wparam,LPARAM lparam)
{                         
//HDC hdc;
switch(message)
 {
 case WM_KEYDOWN:
  //wsb.stat(0,"Key %u %x %x",WK_SCANCODE(lparam),(lparam&0x1000000),GetKeyState(VK_NUMLOCK)&1);
  if(!(lparam&0x1000000)&&((GetKeyState(VK_NUMLOCK)&0x1))) return 0;
  switch(WK_SCANCODE(lparam))
   {
	 case 72: //up
	  Scroll(-1);
	  break;
	 case 80: //down
	  Scroll(1);
	  break;
	 case 75: //left
	  Scroll(0,-1);
	  break;
	 case 77: //right
	  Scroll(0,1);
	  break;
	 case 73: //pg up
	  Scroll(-vislin+1);
	  break;
	 case 81: //pg down
	  Scroll(dlog.vislin-1);
	  break;
	 case 71: //home
	  Scroll(0,-dlog.indent);
	  break;
	 case 79: //end
	  Scroll(0,dlog.viscol/4);
	  break;
   }
  break;  
 case WM_VSCROLL:
  switch(LOWORD(wparam))
   {
   case SB_THUMBTRACK:
	InvalidateRect(hwnd,NULL,1);
   case SB_THUMBPOSITION:
    Scroll(-first+HIWORD(wparam));
    break;
   case SB_LINEUP:
    Scroll(-1);
    break;
   case SB_LINEDOWN:
    Scroll(1);
    break;
   case SB_PAGEUP: //pg up
    Scroll(-vislin+1);
    break;
   case SB_PAGEDOWN: //pg down
    Scroll(dlog.vislin-1);
    break;
   case SB_BOTTOM:
    Scroll(-first+nre-1);
    break;
   case SB_TOP:
    Scroll(-first);
    break;
   }
  break;
 case WM_LBUTTONDOWN:
 case WM_RBUTTONDOWN:
  mouse.ex=LOWORD(lparam);
  mouse.ey=HIWORD(lparam);
  SetFocus(hwnd);
  break;
 case WM_MOUSEMOVE:
  mouse.dx=LOWORD(lparam)-mouse.x;
  mouse.x+=mouse.dx;
  mouse.dy=HIWORD(lparam)-mouse.y;
  mouse.y+=mouse.dy;
  //SetFocus(GetWindow(hwnd,GW_OWNER));
  Hover(mouse.x,mouse.y);
  break;
 case WM_CHAR:
  break;
 case WM_ERASEBKGND:
  return 0;
 case WM_PAINT:
  Show();
  return 0;
 case WM_DESTROY:
  break;
 }
return DefWindowProc(lhwnd,message,wparam,lparam);
}

//..................................................................................................................................
void DataLog::Step()
{
if(dlt.Rel()>MinTimeDraw)
 {
 if(dirty)
  Show();
 }
}

//..................................................................................................................................
void DataLog::Redraw()
{
if(dirty)
 InvalidateRect(hwnd,NULL,1);
}
 
//..................................................................................................................................
BOOL DataLog::SearchChar(DWORD id,DWORD sid,char ch)
{
int lnre=nre;
NAT loff=0,coff=0,found=0;
for(int e=0;e<lnre;e++)
 {
 if(de[e].id!=sid) continue;
 for(int c=0;c<de[e].szB;c++)
  {
  if(de[e].pc[c]==ch)
   {
   Msg(id,NULL,"%u(%u,%u) >%i<",coff,e,c,coff-loff);
   loff=coff;
   found++;
   }
  coff++;
  } 
 }
Msg(id,NULL,"Found %Xh=%u='%c' %u times",ch,ch,ch,found);
return 0;
}

//..................................................................................................................................
BOOL DataLog::BitUsage(DWORD id,DWORD sid,int Bpi=1)
{
int lnre=nre;
NAT bu[32],bt=0,bpi;
bpi=Bpi<<3; //bpi=Bpi*8;
ZeroMemory(bu,sizeof(bu));
for(int e=0;e<lnre;e++)
 {
 if(de[e].id!=sid) continue;
 bt+=de[e].szB;
 for(int c=0;c<de[e].szB;c+=Bpi)
  {
  for(NAT pos=0;pos<bpi;pos++)
   bu[pos]+=(int)GetBit(de[e].pc+c,pos);
  } 
 }
if(bt)
 {
 REGi=0;
 for(lnre=bpi-1;lnre>=0;lnre--)
  REGi+=sprintf(REGs+REGi,"b%u[%u]%.1f%% ",lnre,bu[lnre],(double)bu[lnre]/bt*100);
 Msg(id,NULL,"Bit usage: %s",REGs);
 }
return 0;
}

//..................................................................................................................................
BOOL DataLog::SaveCache(LPSTR path,int overwrite=0)
{
FILE*lfile;
lfile=fopen(path,overwrite?"wb":"a+b");
if(!lfile)
 return 0;
if(!overwrite)
 fwrite("\xD\xA####################################\xD\xA",40,1,lfile);
for(int i=0;i<nre;i++)
 {
 fwrite(de[i].cache,de[i].nc,1,lfile);
 fwrite("\r\n",2,1,lfile);
 }
fclose(lfile);
return 1;
}

//..................................................................................................................................
BOOL DataLog::LoadTXT(DWORD id,LPSTR path,int cachemod=0)
{
IOSFile iof;
NAT l;
int fbsz;
char*fisbuf;
if(erret=iof.open(path,FU_R))
 {
 Msg(0xff,NULL,"Couldn't open %s",path);
 return 0;
 }
fbsz=iof.size();
fisbuf=SALLOC(fbsz);
for(;;)
 {
 //l=iof.scantext(fisbuf,fbsz);
 l=iof.scanline(fisbuf,fbsz,'\xD');
 if(!l) break;
 Add(id,NULL,fisbuf,l,DLOG_NO_APPEND,cachemod);
 }
FREE(fisbuf);
return 1;
}

//..................................................................................................................................
BOOL DataLog::SaveData(LPSTR path,FLAGS fmod=FU_W)
{
IOSFile iof;
if(erret=iof.open(path,fmod))
 {
 Msg(0xff,NULL,"Couldn't create %s",path);
 return 0;
 }
iof.amask=0;
iof.wdw(nre);
for(int i=0;i<nre;i++)
 {
 iof.write(de+i,4+sizeof(VTIME));   //id,tm
 iof.put(de[i].pv,de[i].szB);
 }
return 1;
}

//..................................................................................................................................
BOOL DataLog::LoadData(LPSTR path,int cachemod=0)
{
IOSFile iof;
DATA_ENTRY tde;
NAT tnre;
if(erret=iof.open(path,FU_R))
 {
 Msg(0xff,NULL,"Couldn't open %s",path);
 return 0;
 }
iof.amask=0;
tnre=iof.rdw();
for(int i=0;i<tnre;i++)
 {
 iof.read(&tde,4+sizeof(VTIME));
 tde.szB=iof.get(&tde.pv);
 Add(tde.id,&tde.tm,tde.pv,tde.szB,DLOG_NO_APPEND,cachemod);
 FREE(tde.pv);
 }
return 1;
}

#endif