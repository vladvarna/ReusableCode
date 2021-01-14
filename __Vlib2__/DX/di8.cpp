#ifndef V_IDIRECTINPUT8
#define V_IDIRECTINPUT8

#include <dinput.h> //needs: dinput8.lib
#pragma comment(lib,"dinput8.lib")

IDirectInput8 *diobj=NULL; //main DI object
IDirectInputDevice8 *dmouse=NULL; //DI mouse device
IDirectInputDevice8 *dtast=NULL; //DI keyboard device
HRESULT diret=DI_OK; //default return value for DI funcs
//*****************************************************************************************

//tests diret for errors ------------------------------------------------------------------
int isDIOk(LPSTR errmsg=NULL)
{
if(diret==DI_OK) return 1;
wsprintf(strbuf,"%s\n(%x)\nContinue ?",errmsg,diret);
if(MessageBox(hmwnd,strbuf,"DirectInput ERROR",MB_ICONERROR|MB_YESNO|MB_DEFBUTTON2)==IDNO)
 PostQuitMessage(0);
return 0;
}

//inits the diobj, dmouse, dtast objects --------------------------------------------------
void InitDI(DWORD mcoop=DISCL_NONEXCLUSIVE|DISCL_BACKGROUND,DWORD kcoop=DISCL_EXCLUSIVE|DISCL_FOREGROUND)
{
diret=DirectInput8Create(appinst,DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&diobj,NULL);
isDIOk("DI Object creation failed");
if(mcoop)
 {
 diret=diobj->CreateDevice(GUID_SysMouse,&dmouse,NULL);
 isDIOk("Mouse device creation failed");
 diret=dmouse->SetCooperativeLevel(hmwnd,mcoop);
 isDIOk("Failed to set mouse cooperative level");
 diret=dmouse->SetDataFormat(&c_dfDIMouse);
 isDIOk("Failed to set mouse data format");
 dmouse->Acquire();
 GetCursorPos((POINT*)&mouse.x);
 GetClipCursor((RECT*)&mouse.l);
 mouse.s=1.0f;
 }
if(kcoop)
 {
 diret=diobj->CreateDevice(GUID_SysKeyboard,&dtast,NULL);
 isDIOk("Keyboard device creation failed");
 diret=dtast->SetCooperativeLevel(hmwnd,kcoop);
 isDIOk("Failed to set keyboard cooperative level");
 diret=dtast->SetDataFormat(&c_dfDIKeyboard);
 isDIOk("Failed to set keyboard data format");
 dtast->Acquire();
 }
}

//releases diobj, dmouse, dtast -----------------------------------------------------------
void FreeDI()
{
dmouse->Unacquire();
dtast->Unacquire();
if(dmouse)
 {
 dmouse->Release();
 dmouse=NULL;
 }
if(dtast)
 {
 dtast->Release();
 dtast=NULL;
 }
if(diobj)
 {
 diobj->Release();
 diobj=NULL;
 }
}

//retrieves immediate data from dmouse ----------------------------------------------------
inline void DIUpdateMouse(int flags=0)
{
if(dmouse->GetDeviceState(16,&mouse)!=DI_OK)
 {
 if(mwactive) dmouse->Acquire();
 return;
 }
if(flags)
 {
 mouse.x+=mouse.dx*mouse.s;
 mouse.y+=mouse.dy*mouse.s;
 if(flags&0x2)
  {
  if(mouse.x>mouse.r) mouse.x=mouse.r;
  else if(mouse.x<mouse.l) mouse.x=mouse.l;
  if(mouse.y>mouse.d) mouse.y=mouse.d;
  else if(mouse.y<mouse.u) mouse.y=mouse.u;
  }
 }
}

//retrieves immediate data for dtast ------------------------------------------------------
inline void DIUpdateKeys(int flags=0)
{
if(dtast->GetDeviceState(256,keyboard)!=DI_OK)
 {
 if(mwactive) dtast->Acquire();
 return;
 }
keyscan=flags&0xff;
while(keyscan>0)
 {
 if(keyboard[keyscan]) 
  {
  keyvirt=MapVirtualKey(keyscan,1);
  break;
  }
 keyscan--;
 }
}

#endif