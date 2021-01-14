#ifndef V_IDIRECTX8
#define V_IDIRECTX8

#include <gdi.cpp>
#include <mmed.cpp>
#include <dxerr.h>
#pragma comment(lib,"dxerr8.lib")
#pragma comment(lib,"dxguid.lib") //needed by DS

//tests erret -----------------------------------------------------------------------------
int isDXOk(LPSTR errmsg=NULL)
{
if(!HRESULT_SEVERITY(erret)) return 1;
sprintf(strbuf,"%x=%s\n%s\nContinue ?",erret,DXGetErrorString(erret),errmsg);
if(MessageBox(hmwnd,strbuf,"DirectX8 ERROR",MB_ICONERROR|MB_YESNO|MB_DEFBUTTON2)==IDNO)
 exit(1);
return 0;
}

#endif V_IDIRECTX8