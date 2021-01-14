//VERSION 1.0.a
#pragma once
#define V_WINDOWS

//*BEGIN: Windows SDK version updates
#if(_WIN32_WINNT<0x0500)
 //winuser.h
 #define MK_XBUTTON1                     0x0020
 #define MK_XBUTTON2                     0x0040
 #define WM_XBUTTONDOWN                  0x020B
 #define WM_XBUTTONUP                    0x020C
 #define WM_XBUTTONDBLCLK                0x020D
 #define EWX_FORCEIFHUNG                 0x00000010
 #define TPM_NOANIMATION                 0x4000L
#endif
//*/END: Windows SDK version updates

#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")

#ifdef WIN32_LEAN_AND_MEAN
 #include <stdlib.h>
 #include <unknwn.h>
 #include <Commdlg.h>
 #include <shellapi.h>
#endif 

#if defined(_DEBUG)||defined(V_WARNINGS_OFF)  //compile only for debug builds
 #pragma warning(1:4701) //potentially uninitialized local variable
 #pragma warning(disable:4706) //assignament in if
 #pragma warning(disable:4100) //unreferenced parameter
 #pragma warning(disable:4101) //unreferenced variable
 #pragma warning(disable:4018) //signed/unsigned mismatch
 #pragma warning(disable:4389) //signed/unsigned mismatch
 #pragma warning(disable:4245) //return signed/unsigned mismatch
 #pragma warning(disable:4244) //conversion from '__int64' to 'int' possible loss of data
 #pragma warning(disable:4267) //conversion from 'size_t' to 'int', possible loss of data
 #pragma warning(disable:4311) //'type cast' : pointer truncation from '' to ''
 #pragma warning(disable:4312) //'type cast' : pointer truncation from '' to ''
#else
 #pragma warning(1:4100) //unreferenced parameter
 #pragma warning(1:4101) //unreferenced variable
#endif
#pragma warning(error:4700) //variable used without been initialized
#pragma warning(disable:4201) //nameless struct/union
#pragma warning(disable:4554) //clarify operator precedence
#pragma warning(disable:4996) //declared deprecated
#pragma warning(disable:4731) //frame pointer register 'ebp' modified by inline assembly code

//////////////////////////////////////////////////////////////////////////////////////////////////

//Max sizes
#include <v_generic.h>
#include <vtype.h>

// Global registers ////////////////////////////////////////////////////////////////////////
long REGi;
unsigned long REGu;
float REGf;
char REGs[256];
unsigned long gpr0,gpr1,gpr2,gpr3; //general purpose registers for use in asm blocks
long fpuioi; //used in asm blocks to load integer values into FPU registers
float fpuiof; //used in asm blocks to load float values into FPU registers
unsigned __int64 fpuiol; //used in asm blocks to load 64bits values into FPU registers
double fpuiod; //used in asm blocks to load double values into FPU registers

// Global variables ///////////////////////////////////////////////////////////////////
HWND hmwnd=HWND_DESKTOP; //main window handler
HINSTANCE appinst=NULL; //must be initialized for some functions that create windows
HICON hmicon=NULL; //main icon (big and small) (default to icon 0 in exe)
HDC hmdc; //DC associated with main window (for CS_OWNDC mainly); also used as global REG
BOOL mwactive=TRUE,mwvisible=TRUE; //application state
LPSTR AppName=NULL; //used also as main window class name
SYSTEMTIME RealTime; //real time and date
DWORD scrpf=0; //screen pixel format tag (mainly for GL,DD,D3D)
int scrw=0,scrh=0,scrbpp=0; //screen resolution and pixel depth
int scrpmw=0,scrpmh=0; //screen resolution in pixels per meter
MouseStatus mouse;
DWORD PageSz,AllocGran; //mem page size in B, allocation granularity in B
DWORD REGdump; //used for functions who ask for pointers to return unneeded values
char strbuf[PATHSZ]; //global buffer for strings

#ifndef V_MFC
int mww=0,mwh=0; //main window client area width and height
HMENU hmenu=NULL; //main window menu
unsigned char keyboard[256]; //keyboard stats
unsigned keyscan=0,keychar=0,keyvirt=0; //keyboard buffer
#endif

#define RELCOM(cobj)        if(cobj) cobj->Release(),cobj=NULL
#define DELCOM(cobj)        if(cobj) {do{}while(cobj->Release()>0); cobj=NULL;}

#define WC_DEFAULT_PROC "WindowClass_DefWindowProc" //window class with default message processing
#define WK_SCANCODE(keydata) ((keydata&0x007f0000)>>16)
#define WK_PRESSED(keydata) ((unsigned)keydata>>31)

#define AppIcon(iconind) (HICON)LoadImage(appinst,MAKEINTRESOURCE(iconind),IMAGE_ICON,0,0,LR_SHARED|LR_DEFAULTSIZE) //don't destroy it

LRESULT CALLBACK MainWndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);//main wnd callback prototype

#ifndef V_COMPANY
 #define V_COMPANY "Soft Art"
#endif
#ifndef V_DEF_RES
 #define V_DEF_RES "W:\\__DEV__\\TEST\\" //must end with '\'
#endif

#ifdef V_FRAME_APP
 #ifdef _DEBUG
  #define ABOUT(hparentwnd) ::MessageBox(hparentwnd,V_THIS_APP" "V_THIS_VERSION"\nBuilt: "__DATE__"\nby "V_COMPANY" (C)"V_THIS_START_DATE,"About (DEBUG)",MB_OK|MB_ICONASTERISK)
 #else 
  #define ABOUT(hparentwnd) ::MessageBox(hparentwnd,V_THIS_APP" "V_THIS_VERSION"\nBuilt: "__DATE__"\nby "V_COMPANY" (C)"V_THIS_START_DATE,"About",MB_OK|MB_ICONASTERISK)
 #endif 
#else
 #define ABOUT(hparentwnd) ::MessageBox(hparentwnd,"Built: "__DATE__"\nby "V_COMPANY,"About",MB_OK|MB_ICONASTERISK)
#endif
//**************************************************************************************************

//releases a COM -----------------------------------------------------------------------------------
int RelCOM(IUnknown*obj,LPSTR errmsg=NULL)
{
if(!obj)
 {
 MessageBox(hmwnd,errmsg,"RelCOM: NULL pointer",MB_ICONWARNING|MB_OK|MB_TASKMODAL|MB_TOPMOST);
 return 0;
 }
int refcnt=obj->Release();
if(refcnt!=0) 
 {
 sprintf(strbuf,"%s\nReference count: %u",errmsg,refcnt);
 MessageBox(hmwnd,strbuf,"COM object still in memory",MB_ICONWARNING|MB_OK|MB_TASKMODAL|MB_TOPMOST);
 }
return refcnt;
}

//Performance timer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Chronos
{
public:
 double res;
 __int64 freq,mark,last;
 Chronos()
  {
  if(QueryPerformanceFrequency((LARGE_INTEGER*)&freq))
   res=(double)1.0/freq;
  else
   res=0.;
  mark=last=0;
  }
 void Abs()
  {
  QueryPerformanceCounter((LARGE_INTEGER*)&mark);
  }
 NAT Rel() //ticks count
  {
  QueryPerformanceCounter((LARGE_INTEGER*)&last);
  return last-mark;
  }
 NAT Sec(double multiplu=1.0) //ds=10,cs=100,ms=1000,ns=1000000
  {
  QueryPerformanceCounter((LARGE_INTEGER*)&last);
  return (last-mark)*(res*multiplu);
  }
};

#include <bas.cpp>
#include <mat.cpp>
#include <str.cpp>
#include <errors.cpp>
#include <tools.cpp>
#include <dbg.cpp>

//load a range of resource strings (MUST free) ----------------------------------------------
int LoadStrings(LPSTR strings[],NAT stval=0,NAT count=1,NAT maxlen=BUFFSZ)
 {
 LPSTR lbuf;
 NAT length,ind=0;
 lbuf=SALLOC(maxlen);
 while(count--)
  {
  if(length=LoadString(appinst,stval++,lbuf,maxlen))
   {
   strings[ind]=SALLOC(length);
   CopyMemory(strings[ind],lbuf,length+1);
   ind++;
   }
  }
 FREE(lbuf);
 return ind;
 }

//free a string array -------------------------------------------------------------------------
inline void FreeStrings(LPSTR strings[],NAT count=1)
{
while(count)
 {
 count--;
 FREE(strings[count]);
 }
}

//publish string to clipboard -----------------------------------------------------------------
int StrToClipboard(LPSTR sstr,NAT slen=0)
{
HANDLE hgmem;
LPSTR lstr;
if(!OpenClipboard(hmwnd))
	return 0;
if(!slen) slen=sl(sstr);
hgmem=GlobalAlloc(GMEM_MOVEABLE,slen+1);
if(!hgmem)
	{
	CloseClipboard();
	return 0;
	}
lstr=(LPSTR)GlobalLock(hgmem);
if(!lstr)
	{
	GlobalFree(hgmem);
	CloseClipboard();
	return 0;
	}
sc(lstr,sstr,slen,'\0');
GlobalUnlock(hgmem);
if(!SetClipboardData(CF_TEXT,hgmem))
	{
	GlobalFree(hgmem);
	CloseClipboard();
	return 0;
	}
CloseClipboard();
return 1;
}

//download string from clipboard -----------------------------------------------------------
LPSTR StrFromClipboard(LPSTR dstr,NAT slen=0)
{
if(!IsClipboardFormatAvailable(CF_TEXT)) 
	return FALSE; 
HANDLE hgmem;
LPSTR lstr;
if(!OpenClipboard(hmwnd))
	return NULL;
hgmem=GetClipboardData(CF_TEXT);
if(!hgmem)
	{
	CloseClipboard();
	return NULL;
	}
lstr=(LPSTR)GlobalLock(hgmem);
if(!lstr)
	{
	CloseClipboard();
	return NULL;
	}
if(!dstr)
 {
 slen=sl(lstr);
 dstr=(LPSTR)malloc(slen+1);
 }
sc(dstr,lstr,slen);
GlobalUnlock(hgmem);
CloseClipboard();
return dstr;
}

//interface for Open/Save As dialog ---------------------------------------------------------------------------
BOOL GetFileName(LPSTR fn,DWORD style=0,HWND parent=NULL,LPSTR filter=NULL,LPSTR title=NULL,LPSTR defext=NULL,
                 int*nameoff=NULL,int*extoff=NULL,LPSTR name=NULL,char mod='O',LPSTR initdir=NULL)
//OFN_:FILEMUSTEXIST,OVERWRITEPROMPT,CREATEPROMPT,PATHMUSTEXIST,ALLOWMULTISELECT,NOTESTFILECREATE
{
OPENFILENAME of;
BOOL nameok;
ZeroMemory(&of,sizeof(OPENFILENAME));
of.lStructSize=sizeof(OPENFILENAME);
of.hwndOwner=parent;
of.hInstance=appinst;
of.lpstrFilter=filter;
//of.lpstrCustomFilter=NULL;
//of.nMaxCustFilter=0;
of.nFilterIndex=1;
of.lpstrFile=fn;
of.nMaxFile=PATHSZ;
of.lpstrFileTitle=name;
of.nMaxFileTitle=PATHSZ;
of.lpstrInitialDir=initdir;
of.lpstrTitle=title;
of.Flags=OFN_HIDEREADONLY|OFN_NOCHANGEDIR|style;
//of.nFileOffset=0;
//of.nFileExtension=0;
of.lpstrDefExt=defext;
//*fn=0;
if(mod=='S')
 nameok=GetSaveFileName(&of);
else 
 nameok=GetOpenFileName(&of);
if(!nameok) *fn=0;
if(nameoff) *nameoff=of.nFileOffset;
if(extoff) *extoff=of.nFileExtension;
if(defext)
 {
 if(filter&&scmp(defext,"auto"))
  {
  defext=nextch('\0',filter,of.nFilterIndex*2-1)+2;
  changefileext(fn,defext);
  }
 //else
 // changefileext(fn,defext);
 }
return nameok;
}

#include <reg.cpp>
#include <io.cpp>
#include <winter.cpp>
#include <winui.cpp>
#include <vtime.cpp>
#include <vstr.cpp>
#include <udata.cpp>

//globals app initialization ----------------------------------------------------------
void InitApp(LPSTR appname,HINSTANCE hinst=NULL)
{
SYSTEM_INFO si;
AppName=appname;
appinst=hinst?hinst:GetModuleHandle(NULL);
hmdc=GetDC(NULL);
scrw=GetDeviceCaps(hmdc,HORZRES);
scrh=GetDeviceCaps(hmdc,VERTRES);
scrbpp=GetDeviceCaps(hmdc,BITSPIXEL);
scrpmw=GetDeviceCaps(hmdc,LOGPIXELSX)*M_INCH;
scrpmh=GetDeviceCaps(hmdc,LOGPIXELSY)*M_INCH;
ReleaseDC(NULL,hmdc);
SystemParametersInfo(SPI_GETWORKAREA,0,&mwarea,0);
GetCursorPos((POINT*)&mouse.x);
GdiSetBatchLimit(1); //disable batching
GetSystemInfo(&si);
PageSz=si.dwPageSize;
AllocGran=si.dwAllocationGranularity;
GetLocalTime(&RealTime); //time app was started
//hmicon=ExtractIcon(appinst,ios.AppDir,0); //the first icon is the default app icon
#ifndef V_MFC
GetKeyboardState(keyboard);
#endif
WindowClass(WC_DEFAULT_PROC,DefWindowProc,0,NULL,NULL,LoadIcon(NULL,IDI_QUESTION),NULL);
SetLastError(0); //discard any startup errors
}

