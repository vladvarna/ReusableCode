#pragma once
#define V_ERROR_HANDLING

HRESULT erret=0; //standard return value

#define COM_OK(herr) (herr>=0)
#define COM_ERR(herr) (herr&0x80000000)

#define R_OK (erret>=0)
#define R_ERR (erret&0x80000000)

//uses title bar as a status window ----------------------------------------------------------------
inline void status(char *formstr,...)
{
char buffer[BUFFSZ];
va_list vparam;
va_start(vparam,formstr);
vsprintf(buffer,formstr,vparam);
SetWindowText(hmwnd,buffer);
va_end(vparam);
}

//vararg MessageBox --------------------------------------------------------------------------------
void printbox(char *formstr,...)
{
char buffer[BUFFSZ];
va_list vparam;
va_start(vparam,formstr);
vsprintf(buffer,formstr,vparam);
MessageBox(HWND_DESKTOP,buffer,"PRINT BOX",MB_OK|MB_TASKMODAL);
va_end(vparam);
}

//vararg MessageBoxW --------------------------------------------------------------------------------
void printboxw(LPWSTR formstr,...)
{
WCHAR buffer[BUFFSZ];
va_list vparam;
va_start(vparam,formstr);
vswprintf(buffer,formstr,vparam);
MessageBoxW(HWND_DESKTOP,buffer,L"UNICODE PRINT BOX",MB_OK|MB_TASKMODAL);
va_end(vparam);
}

//prints an error message --------------------------------------------------------------------------
int error(LPSTR errmsg=NULL,int errcod=0,LPSTR errgrp=NULL,int critical=0)
{
char lsbuf[BUFFSZ];
int retv;
if(critical)
 {
 sprintf(lsbuf,"%s\n%i = %Xh\nProgram will close ...",errmsg,errcod,errcod);
 MessageBox(HWND_DESKTOP,lsbuf,errgrp?errgrp:"CRITICAL ERROR",MB_ICONERROR|MB_OK|MB_TASKMODAL|MB_TOPMOST);
 exit(errcod);
 }
else
 {
 sprintf(lsbuf,"%s\n%i = %Xh",errmsg,errcod,errcod);
 retv=MessageBox(HWND_DESKTOP,lsbuf,errgrp?errgrp:"ERROR",MB_ICONWARNING|MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_TOPMOST);
 if(retv==IDABORT) exit(errcod);
 if(retv==IDRETRY) DebugBreak();
 }
return errcod;
}

//tests if a pointer is null -----------------------------------------------------------------------
#define isNULL(pointer,errmsg) if(!pointer) error(errmsg,0,"NULL pointer");
//tests if a boolean is FALSE --------------------------------------------------------------
#define isZERO(boolval,errmsg) if(!boolval) error(errmsg,0,"FALSE result");
//signals an error that is treated in Release builds but should be signald in Debug buids-----------
#ifdef _DEBUG //debug builds
 #pragma message("Debug build")
 #define WARN(cond,msg) if(cond) error(msg,cond,"DEBUG WARNING");
#else
 #define WARN(cond,msg)
#endif

//message associated with last error --------------------------------------------------------------------------
inline int LastErrorMsg(LPSTR errbuf=NULL,NAT errbufsz=BUFFSZ)
{
int l,retv=GetLastError();
l=sprintf(errbuf,"GetLastError:%u=",retv);
FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,retv,
              MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),errbuf+l,errbufsz-l,NULL);
return retv;
}

//tests for a last error --------------------------------------------------------------------------
inline int isERROR(LPSTR errmsg=NULL)
{
int retv=GetLastError();
if(retv==ERROR_SUCCESS||retv==ERROR_IO_PENDING) return 0;
LPVOID errbuf;
FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
              NULL,retv,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR)&errbuf,0,NULL);
retv=MessageBox(HWND_DESKTOP,(LPTSTR)errbuf,errmsg,MB_ICONERROR|MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_TOPMOST);
LocalFree(errbuf);
if(retv==IDABORT) exit(1);
if(retv==IDRETRY) DebugBreak();//return 1;
return 0;
}

//tests erret for an error --------------------------------------------------------------------------
inline int isCOERR(LPSTR errmsg=NULL)
{
if(!HRESULT_SEVERITY(erret)) return 0;
int retv;
LPVOID errbuf;
FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
              NULL,HRESULT_CODE(erret),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR)&errbuf,0,NULL);
retv=MessageBox(HWND_DESKTOP,(LPTSTR)errbuf,errmsg,MB_ICONERROR|MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_TOPMOST);
LocalFree(errbuf);
if(retv==IDABORT) exit(1);
if(retv==IDRETRY) DebugBreak();//return 1;
return 0;
}

//error in function -------------------------------------------------------------------------
int exitproc(char *formstr,...)
{
char lsbuf[BUFFSZ];
int retv;
va_list vparam;
va_start(vparam,formstr);
retv=vsprintf(lsbuf,formstr,vparam);
va_end(vparam);
retv+=sprintf(lsbuf+retv,"\nStep out ?");
retv=MessageBox(HWND_DESKTOP,lsbuf,"ERROR in procedure",MB_ICONWARNING|MB_YESNOCANCEL|MB_TASKMODAL|MB_TOPMOST);
if(retv==IDCANCEL) DebugBreak();
return retv;
}

#define exitif(condition,mesaj,arg0) if(condition){if(exitproc(mesaj,arg0)==IDYES) return;}
#define exitif_(condition,retcode,mesaj,arg0) if(condition){if(exitproc(mesaj,arg0)==IDYES) return retcode;}

