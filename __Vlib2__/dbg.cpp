#ifdef _DEBUG //compile only for debug builds
#pragma once
#define V_DEBUG
#define V_SAFE

#include <assert.h>

#pragma message("Debug functions included")
#define DBGLOG_FILE_NAME "dump.log"
#define DEFLOG_FILE_NAME "def.log"

FILE*dbglog=NULL,*deflog=NULL; //debug log file
int NoBlock=0; //(SCROOL LOCK) inhibits all Wait... and Break... functions
HWND hdbgwnd=NULL;

LRESULT CALLBACK DbgWndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);

#define FLUSH_LOG fclose(dbglog),dbglog=NULL;

#define MEMORY_BASIC_INFORMATION_MAP "ppnnnnn"
#define WINDOWINFO_MAP "nllllllllnnnuuww"
#define WNDCLASSEX_MAP "unpllppppttp"
#define WINDOWPLACEMENT_MAP "nnnllllllll"
#define GLYPHMETRICS_MAP ",W:u,H:u,X0:l,Y0:l,dx:s,dy:s"
#define PANOSE_MAP "bbbbbbbbbb" //10B
#define TEXTMETRIC_MAP ",H:l,Asc:l,Desc:l,Int:l,Ext:l,AveW:l,MaxW:l,Bold:l,Overhang:l,Ax:l,Ay:lcccc,/:b,_:b,-:b,Fam:b,Set:b>4" //53+4 bytes
#define CHCheader_MAP ",ID:$4,H:u,A:u,MW:u,ATRIB:u<4,SUIB:w,PITCH FAM:b,CHSET:b"
#define OUTLINETEXTMETRIC_MAP "u'TextMetric'>58'Panose'>10uuiiiuiiuuulllliiuulllllllluiiipppp"
#define SYSTEM_INFO_MAP ",CPU:u,PagSz:n,MinAdr:p,MaxAdr:p,CPUmask:n,NrCPUs:n,CPUtype:u,AlocGran:n,CPUfam:w,CPUrev:w"
#define MEMORYSTATUS_MAP "u,%:u,TotPhys:u,AvailPhys:u,TotPageFile:u,AvailPageFile:u,TotVirt:u,AvailVirt:u"
#define BITMAPINFO_MAP ",sizeof:u,W:l,H:l,Planes:w,Bpp:w,Compres:n,SzB:u,Xres:l,Yres:l,ClrUsed:n,ClrImp:n'Palette'"
#define TW_VERSION_MAP ",Vers:w,Rel:w,Lang:w,Country:w$34"
#define TW_IDENTITY_MAP ",ID:u"TW_VERSION_MAP",ProtMaj:w,ProtMin:w,Suport:u$34$34$34"
#define TW_IMAGEINFO_MAP ",XRes:i,YRes:i,W:i,H:i,Sample/pix:w'Bits/sample'wwwwwwww,Bpp:w,Planar:b,Type:w,Compres:w"
#define ENHMETAHEADER_MAP ",Type:u,szof:u'Bounds'llll'Frame(.01mm)'llll,ID:u,Vers:u,szB:u,Records:u,Handles:u,Desc nc:u,Desc off:u,PalEnts:u,W:l,H:l,W(mm):l,H(mm):l" //",PFD:u,PFD off:u,GL:u"
#define WAVEOUTCAPS_MAP ",M/Pid:u,Vers:u$32,Formats:u,Channels:ww,Caps:u"
#define WAVEINCAPS_MAP ",M/Pid:u,Vers:u$32,Formats:u,Channels:w"
#define AUXCAPS_MAP ",M/Pid:u,Vers:u$32,Formats:u,Aux/CD:ww,Caps:u"
#define CCONV_MAP "'Left rolls'bbbb'Masks'uuuu,SBpp:u,DBpp:u,Bpi:u,Imask:u" 
#define DCB_MAP "'DCB',szof:u,baud:u,flags:u,rsvd:w,XonLim:w,XoffLim:w,Bsz:b,Parity:b,Stopb:b,Xon:c,Xoff:c,Eof:c,Evt:c,rsvd:w"
#define COMMCONFIG_MAP "'COMMCONFIG',szof:u,vers:w,rsvd:w\n"DCB_MAP"\n'Provider',subtype:u,offset:u,size:u"
#define COMMPROP_MAP "'COMMPROP',packetsz:w,packetvers:w,service:u,rsvd:u,MaxTxQ:u,MaxRxQ:u,MaxBaud:u,ProvType:u,Caps:u,SetParams:u,SetBauds:u,SetData:w,SetStop:w,CurTxQ:u,CurRxQ:u,ProvSpec1:u,ProvSpec2:u"
//*******************************************************************************************

//prints a structure using a memory map ----------------------------------------------------
char* __cdecl dump(LPSTR memmap,...)
{
va_list vparam;
va_start(vparam,memmap);
LPSTR locbuf,cbuf;
cbuf=locbuf=(LPSTR)malloc(4096);
//POINTER pmem;
char*pmem;
NAT l=0;
int retbuf=0; //1-return buffer; 2-output to log
pmem=va_arg(vparam,char*);
while(*memmap&&(cbuf<locbuf+4096))
 {
 switch(toupper(*memmap))
  {
  case 'I':
  case 'L': //long
   cbuf+=sprintf(cbuf,"\t0x%08X  %i\n",*(long*)pmem,*(long*)pmem);
   pmem+=4;
   memmap++;
   break;
  case 'U':
  case 'N': //NAT 
   cbuf+=sprintf(cbuf,"\t0x%08X  %u\n",*(NAT*)pmem,*(NAT*)pmem);
   pmem+=4;
   memmap++;
   break;
  case 'S': //short __int16
   cbuf+=sprintf(cbuf,"\t0x%04X  %i\n",*(short*)pmem,*(short*)pmem);
   pmem+=2;
   memmap++;
   break;
  case 'W': //word
   cbuf+=sprintf(cbuf,"\t0x%04X  %u\n",*(WORD*)pmem,*(WORD*)pmem);
   pmem+=2;
   memmap++;
   break;
  case 'O': //octet
  case 'B': //byte
  case 'C': //char __int8
   cbuf+=sprintf(cbuf,"\t0x%02X %u '%c'\n",*(BYTE*)pmem,*(BYTE*)pmem,*(char*)pmem?*(char*)pmem:C_NULL);
   pmem++;
   memmap++;
   break;
  case 'F': //float
   cbuf+=sprintf(cbuf,"\t%f\n",*(float*)pmem);
   pmem+=4;
   memmap++;
   break;
  case 'D': //double
   cbuf+=sprintf(cbuf,"\t%.15f\n",*(double*)pmem);
   pmem+=8;
   memmap++;
   break;
  case 'M': //wchar
   cbuf+=sprintf(cbuf,"\t%04X '%C'\n",*(WORD*)pmem,*(WORD*)pmem,*(WCHAR*)pmem?*(WCHAR*)pmem:WC_NULL);
   pmem+=2;
   memmap++;
   break;
  case 'E': //__int64 ~ 2^60=1 ExaByte
   cbuf+=sprintf(cbuf,"\t%I64i  %I64X\n",*(__int64*)pmem,*(__int64*)pmem);
   pmem+=8;
   memmap++;
   break;
  case 'Q': //quadword
   cbuf+=sprintf(cbuf,"\t%I64u  %I64X\n",*(__int64*)pmem,*(__int64*)pmem);
   pmem+=8;
   memmap++;
   break;
  case 'P': //pointer
   cbuf+=sprintf(cbuf,"\t%p\n",*(void**)pmem);
   pmem+=4;
   memmap++;
   break;
  case 'T': //text with null terminator
   if(!(*(char**)pmem))
    cbuf+=sc(cbuf,"!null",5,'\0','\n')+1;
   else 
    {
    cbuf+=sc(cbuf,"\"");
    cbuf+=sc(cbuf,*(char**)pmem,BUFFSZ,'\0','\"')+1;
    cbuf+=sc(cbuf,"\n");
    }
   pmem+=4;
   memmap++;
   break;
  case '~': //text with next char as terminator
   if(!(*(char**)pmem))
    cbuf+=sc(cbuf,"!null",5,'\0','\n')+1;
   else 
    {
    cbuf+=sc(cbuf,">");
    cbuf+=sc(cbuf,*(char**)pmem,BUFFSZ,*(memmap+1),'<')+1;
    cbuf+=sc(cbuf,"\n");
    }
   pmem+=4;
   memmap+=2;
   break;
  case '\'': //text as is
   l=sc(cbuf,memmap+1,BUFFSZ,'\'','\n');
   cbuf+=l+1;
   memmap+=l+2;
   break;
  case ',': //label
   l=sc(cbuf,memmap+1,BUFFSZ,':',':');
   cbuf+=l+1;
   *cbuf=' ';
   cbuf++;
   memmap+=l+2;
   break;
  case '$': //text buffer embeded in struct ex: char[32]
   l=StoI(memmap+1,10,&memmap);
   cbuf+=sc(cbuf,pmem,l,0,'\n')+1;
   pmem+=l;
   break;
  case '>': //move pointer forward
   pmem+=StoI(memmap+1,10,&memmap);
   break;
  case '<': //move pointer back
   pmem-=StoI(memmap+1,10,&memmap);
   break;
  case '-': //delete last outputed character
   cbuf--;
   memmap++;
   break;
  case '|': //return buffer
   retbuf|=1;
   memmap++;
   break;
  case '#': //log
   retbuf|=2;
   memmap++;
   break;
  case '^': //load next pointer
   pmem=va_arg(vparam,char*);
   memmap++;
   break;
  default: //copy any unrecognized chars directly to output
   *cbuf=*memmap;
   cbuf++;
   memmap++;
  }
 }
*cbuf=0;
va_end(vparam);
if(retbuf&2) //log
 {
 if(!dbglog) dbglog=fopen(DBGLOG_FILE_NAME,"wt");
 fputc('\n',dbglog);
 fputs(locbuf,dbglog);
 }
if(retbuf&1)
 return locbuf;
if(!retbuf)
 MessageBox(HWND_DESKTOP,locbuf,"Mem dump",MB_OK);
free(locbuf);
return NULL;
}

//Breakpoint --------------------------------------------------------------------------------
void BreakIf(int cond,char*formstr,...)
{
if(NoBlock) return;
if(!cond) return;
char buffer[BUFFSZ];
int retv;
va_list vparam;
va_start(vparam,formstr);
vsprintf(buffer,formstr,vparam);
va_end(vparam);
retv=MessageBox(HWND_DESKTOP,buffer,"Breakpoint",MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_TOPMOST);
if(retv==IDABORT) exit(cond);
if(retv==IDRETRY) DebugBreak();
}

//write to log --------------------------------------------------------------------------------
void Log(char *formstr,...)
{
va_list vparam;
char buffer[BUFFSZ];
va_start(vparam,formstr);
vsprintf(buffer,formstr,vparam);
if(!dbglog) dbglog=fopen(DEFLOG_FILE_NAME,"a+t");
fputs(buffer,dbglog);
fputc('\n',dbglog);
va_end(vparam);
}

//write to log --------------------------------------------------------------------------------
void log(char *formstr,...)
{
va_list vparam;
char buffer[BUFFSZ];
va_start(vparam,formstr);
vsprintf(buffer,formstr,vparam);
if(!dbglog) dbglog=fopen(DBGLOG_FILE_NAME,"wt");
fputs(buffer,dbglog);
fputc('\n',dbglog);
va_end(vparam);
}

//appends a message to title bar -----------------------------------------------------------------
void apstat(char *formstr,...)
{
va_list vparam;
va_start(vparam,formstr);
char buffer[BUFFSZ];
vsprintf(buffer+GetWindowText(hmwnd,buffer,BUFFSZ),formstr,vparam);
SetWindowText(hmwnd,buffer);
va_end(vparam);
}

//blocks until key pressed ----------------------------------------------------------
void WaitVKey(char vkey=VK_RETURN)
{
if(NoBlock) return;
if(GetAsyncKeyState(VK_ESCAPE)&0x8000) return;
FlashWindow(hmwnd,1);
Sleep(100);
FlashWindow(hmwnd,0);
GetWindowText(hmwnd,strbuf,256);
SetWindowText(hmwnd,"Press key ...");
while(!(GetAsyncKeyState(vkey)&0x8000))
 {
 if(GetAsyncKeyState(VK_ESCAPE)&0x8000) 
  {
  NoBlock=1;   //
  break;
  }
 Sleep(0);
 }
SetWindowText(hmwnd,strbuf);
}

//MessageBox ------------------------------------------------------------------------------------------
void printstr(LPSTR msg,NAT slen=0)
{
if(!slen) slen=strlen(msg);
LPSTR buffer;
buffer=(LPSTR)malloc(slen+1);
CopyMemory(buffer,msg,slen);
buffer[slen]=0;
MessageBox(HWND_DESKTOP,buffer,"String",MB_OK);
free(buffer);
}

//prints an array of floats ------------------------------------------------------------------------
void printfloats(float *pf,NAT nrf=0)
{
NAT el,l=0;
char buffer[BUFFSZ];
for(el=0;el<nrf;el++)
 l+=sprintf(buffer+l,"[%u] %.3f ",el,pf[el]);
MessageBox(hmwnd,buffer,"Array of floats",MB_OK);
}

//prints a PFD struc -------------------------------------------------------------------
void dumppfd(PIXELFORMATDESCRIPTOR pfd)
{
printbox("%u (%u:%u:%u:%u)(%u:%u:%u:%u) %u %u %u",pfd.cColorBits,pfd.cAlphaBits,pfd.cRedBits,pfd.cGreenBits,pfd.cBlueBits,pfd.cAlphaShift,pfd.cRedShift,pfd.cGreenShift,pfd.cBlueShift,pfd.cDepthBits,pfd.cStencilBits,pfd.cAccumBits);
}

//get device caps and dimmensions---------------------------------------------------------------------
void devcaps(HDC dc)
{
POINT vo,wo,co;
SIZE ve,we;
GetDCOrgEx(dc,&co);
GetViewportOrgEx(dc,&vo);
GetViewportExtEx(dc,&ve);
GetWindowOrgEx(dc,&wo);
GetWindowExtEx(dc,&we);
printbox("Viewport: %i %i %i %i\nWindow: %i %i %i %i\nDCOrg: %i %i\nW: %i (%i mm), H: %i (%i mm), bpp: %i, pl: %i\nRes: %i dpi, %i dpi, Aspect: %i/%i\\%i \nPaper: X %i, Y %i, W %i, H %i, SX %i, SY %i",
         vo.x,vo.y,ve.cx,ve.cy,wo.x,wo.y,we.cx,we.cy,co.x,co.y,
         GetDeviceCaps(dc,HORZRES),GetDeviceCaps(dc,HORZSIZE),
         GetDeviceCaps(dc,VERTRES),GetDeviceCaps(dc,VERTSIZE),
         GetDeviceCaps(dc,BITSPIXEL),GetDeviceCaps(dc,PLANES),
         GetDeviceCaps(dc,LOGPIXELSX),GetDeviceCaps(dc,LOGPIXELSY),
         GetDeviceCaps(dc,ASPECTX),GetDeviceCaps(dc,ASPECTY),GetDeviceCaps(dc,ASPECTXY),
         GetDeviceCaps(dc,PHYSICALOFFSETX),GetDeviceCaps(dc,PHYSICALOFFSETY),
         GetDeviceCaps(dc,PHYSICALWIDTH),GetDeviceCaps(dc,PHYSICALHEIGHT),
         GetDeviceCaps(dc,SCALINGFACTORX),GetDeviceCaps(dc,SCALINGFACTORY));
}

//prints a DEVMODE --------------------------------------------------------------------
void dumpdevmode(DEVMODE*dm)
{
printbox("%s\nVers: %hx %hx, szof: %hx+%hx, Fields:%x\nPaper: L/P %hi, Sz %hi , W %hi mm, H %hi mm, Zoom %hi%%, Count %hi, DefSrc %hx, Qual(Xres) %hi dpi(>0), Mono %hx, Duplex %hx, Yres %hi dpi, TT %hx, Collate %hx\n>%s< NUP %x",
         dm->dmDeviceName,dm->dmDriverVersion,dm->dmSpecVersion,dm->dmSize,dm->dmDriverExtra,dm->dmFields,
         dm->dmOrientation,dm->dmPaperSize,dm->dmPaperLength/10,dm->dmPaperWidth/10,
         dm->dmScale,dm->dmCopies,dm->dmDefaultSource,dm->dmPrintQuality,dm->dmColor,
         dm->dmDuplex,dm->dmYResolution,dm->dmTTOption,dm->dmCollate,
         dm->dmFormName,dm->dmNup);
}

//prints a DEVNAMES -----------------------------------------------------------------------
void dumpdevnames(DEVNAMES*dn)
{
printbox("%s\n%s\n%s\n%hx",(char*)dn+dn->wDriverOffset,(char*)dn+dn->wDeviceOffset,(char*)dn+dn->wOutputOffset,dn->wDefault);
} 

//memory status info ----------------------------------------------------------------------
void MemStat()
{
MEMORYSTATUS lms;
char lsbuf[128];
GlobalMemoryStatus(&lms);
sprintf(lsbuf,"Load %u%%\nAvailRAM: %u MB of %u MB\nAvailFile: %u MB of %u MB\nAvailVirt: %u MB of %u MB\n",
        lms.dwMemoryLoad,lms.dwAvailPhys>>20,lms.dwTotalPhys>>20,lms.dwAvailPageFile>>20,
        lms.dwTotalPageFile>>20,lms.dwAvailVirtual>>20,lms.dwTotalVirtual>>20);
MessageBox(NULL,lsbuf,"Memory Status",MB_OK);
}

//prints info about virtual memory pages --------------------------------------------------------------------------------
void dumpvmem(void*ppag,LPSTR msg=NULL)
{
MEMORY_BASIC_INFORMATION lmbi;
SYSTEM_INFO lsi;
char lsbuf[128];
GetSystemInfo(&lsi);
VirtualQuery(ppag,&lmbi,sizeof(MEMORY_BASIC_INFORMATION));
sprintf(lsbuf,"Page: %p\nBase: %p\nSize: %u KB = %u pages = %u blocks\n%s%s%s\n%s%s%s%s",
        lmbi.BaseAddress,lmbi.AllocationBase,lmbi.RegionSize>>10,lmbi.RegionSize/lsi.dwPageSize,lmbi.RegionSize/lsi.dwAllocationGranularity,
        lmbi.State&MEM_COMMIT?"Commited":"",lmbi.State&MEM_RESERVE?"Reserved":"",lmbi.State&MEM_FREE?"Free":"",
        lmbi.Protect&PAGE_NOACCESS?"NA":"",lmbi.Protect&PAGE_READONLY?"RO":"",lmbi.Protect&PAGE_READWRITE?"RW":"",lmbi.Protect&0xf0?"exec":"");
MessageBox(NULL,lsbuf,msg?msg:"Memory Basic Info",MB_OK);
}

//mod:b0-enum regions, b1-enum blocks -----------------------------------------------------------------------------
void VirtualMemWalk(int mod=0,char*pmem=NULL)
{
MEMORY_BASIC_INFORMATION lmbi;
SYSTEM_INFO lsi;
char lsbuf[128];
NAT nrpag,nrblk,regcnt=0,freecnt=0,rsvdcnt=0,commcnt=0;
GetSystemInfo(&lsi);
nrpag=((NAT)lsi.lpMaximumApplicationAddress-(NAT)lsi.lpMinimumApplicationAddress)/lsi.dwPageSize;
nrblk=((NAT)lsi.lpMaximumApplicationAddress-(NAT)lsi.lpMinimumApplicationAddress)/lsi.dwAllocationGranularity;
if(!pmem) pmem=(char*)lsi.lpMinimumApplicationAddress;
while(pmem<lsi.lpMaximumApplicationAddress)
 {
 regcnt++;
 VirtualQuery(pmem,&lmbi,sizeof(lmbi));
 if(lmbi.State&MEM_COMMIT) commcnt+=lmbi.RegionSize/lsi.dwPageSize;
 if(lmbi.State&MEM_RESERVE) rsvdcnt+=lmbi.RegionSize/lsi.dwPageSize;
 if(lmbi.State&MEM_FREE) freecnt+=lmbi.RegionSize/lsi.dwPageSize;
 if(mod&0x3)
  {
  sprintf(lsbuf,"Page %u of %u\nBlock %u of %u\nPage: %p\nBase: %p\nSize: %u KB  = %u pages = %u blocks\n%s%s%s\n%s%s%s%s\n",
          ((NAT)pmem-(NAT)lsi.lpMinimumApplicationAddress)/lsi.dwPageSize,nrpag,
          ((NAT)pmem-(NAT)lsi.lpMinimumApplicationAddress)/lsi.dwAllocationGranularity,nrblk,
          lmbi.BaseAddress,lmbi.AllocationBase,lmbi.RegionSize>>10,lmbi.RegionSize/lsi.dwPageSize,lmbi.RegionSize/lsi.dwAllocationGranularity,
          lmbi.State&MEM_COMMIT?"Commited":"",lmbi.State&MEM_RESERVE?"Reserved":"",lmbi.State&MEM_FREE?"Free":"",
          lmbi.Protect&PAGE_READONLY?"RO":"",lmbi.Protect&PAGE_READWRITE?"RW":"",lmbi.Protect&PAGE_NOACCESS?"NA":"",lmbi.Protect&0xf0?"exec":"");
  if(MessageBox(NULL,lsbuf,"Virtual Memory Walk",MB_OKCANCEL)==IDCANCEL) mod=0;
  }
 if(mod&0x2)
  {
  if(lmbi.State&MEM_FREE)
   pmem+=lmbi.RegionSize;
  else
   pmem+=lsi.dwAllocationGranularity;
  pmem=(char*)AlignPointer(pmem,lsi.dwAllocationGranularity);
  }
 else 
  pmem+=lmbi.RegionSize;
 }
sprintf(lsbuf,"Pages: %u\nBlocks: %u\nRegions: %u\nCommited: %u pages\nReserved: %u pages\nFree: %u pages",
        nrpag,nrblk,regcnt,commcnt,rsvdcnt,freecnt);
MessageBox(NULL,lsbuf,"Virtual Memory Statistic",MB_OK);
}

//-----------------------------------------------------------------------------------
template <typename vector> void ShowArray
(vector*v,int nrel,HWND hwnd,COLORREF c=0xffffff,int rop=R2_COPYPEN,int T=0,NAT*obj=NULL,NAT nrobj=1)
{
HDC hldc;
RECT lr;
HPEN hlp;
float l,p;
double s,o,m;
int x;
char lsbuf[256];
GetClientRect(hwnd,&lr);
l=0;
p=(float)nrel/lr.right;
o=v[findmin(v,nrel)];
//o=0;
m=v[findmax(v,nrel)];
sprintf(lsbuf,"[%.1f;%.1f]",o,m);
SetWindowText(hwnd,lsbuf);
s=(double)lr.bottom/(m-o);
o*=s;
hldc=GetDC(hwnd);
if(rop==R2_COPYPEN) BitBlt(hldc,lr.left,lr.top,lr.right,lr.bottom,NULL,0,0,BLACKNESS);
if(obj)
 {
 SelectObject(hldc,GetStockObject(GRAY_BRUSH));
 SelectObject(hldc,GetStockObject(WHITE_PEN));
 for(x=0;x<(nrobj<<1);x+=2)
  Rectangle(hldc,obj[x]/p,0,obj[x+1]/p+1,lr.bottom);
 }
SetROP2(hldc,rop);
hlp=(HPEN)SelectObject(hldc,CreatePen(PS_SOLID,0,c));
for(x=0;x<lr.right;x++)
 {
 MoveToEx(hldc,x,lr.bottom+o,NULL);
 LineTo(hldc,x,lr.bottom+o-s*v[(NAT)l]);
 l+=p;
 }
//SetROP2(hldc,R2_WHITE);
//MoveToEx(hldc,T/p,lr.bottom+o,NULL);
//LineTo(hldc,T/p,0);

DeleteObject(SelectObject(hldc,hlp));
ReleaseDC(hwnd,hldc);
}

#endif //_DEBUG