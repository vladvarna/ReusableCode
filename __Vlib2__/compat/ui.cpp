#ifndef V_UNIFACE
#define V_UNIFACE

#include <compat/ui_form.cpp>

void* ProcessUINotify(void*,DWORD);

class UIMODLG
{
public:
 UISKIN uiskn;
 UIFORM uifrm;
 HWND wnd,hwp;
 int done;

 UIMODLG();
 void Init(int,int,HWND);
 void Run();
 int Pick(LPSTR,LPSTR,int); //pick item from list
 int PickL(LPSTR,LPSTR,NAT,NAT,int); //pick item from list
 int PickS(LPSTR,char**,NAT,NAT,int); //pick item from list
 ~UIMODLG();
}uidlg;
//***************************************************************************************

// UIMODLG <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
UIMODLG::UIMODLG()
{
ZeroMemory(this,sizeof(UIMODLG));
uiskn.Make(ID_WIN);
uiskn.UseFont(0,"Times New Roman",20,0x5);
uiskn.UseFont(1,"Arial",16,0x4);
uiskn.UseFont(2,"Courier New",40,0x7);
hwp=hmwnd;
WindowClass("UIModalDialogWC",UIFormProc,0,GetSysColorBrush(COLOR_BTNFACE));
wnd=CreateWindowEx(WS_EX_CLIENTEDGE|WS_EX_TOOLWINDOW|WS_EX_TOPMOST,
                   "UIModalDialogWC","UI Modal Dialog",WS_CAPTION,
                   CW_USEDEFAULT,0,400,200,hwp,NULL,appinst,NULL);
uifrm.Init("UIModalDialogForm",&uiskn,wnd,NULL);
uifrm.gstat|=IS_NACT|IS_NVIS;
uifrm.Make(IF_NINF|IF_NPCM,0,IL_0);
uifrm.AddI(0,"Done",IT_P,IL_APOP,0x1,&done,0,NULL,NULL,NULL,0,uifrm.d-uiskn.chh*2,100,0,"Done.");
}

UIMODLG::~UIMODLG()
{
uifrm.Free();
uiskn.Free();
DestroyWindow(wnd);
}

void UIMODLG::Init(int w=0,int h=0,HWND hparent=NULL)
{
hwp=hparent;
RECT cr,wr;
GetClientRect(wnd,&cr);
GetWindowRect(hwp,&wr);
if(w>cr.right) cr.right=w;
if(h>cr.bottom) cr.bottom=h;
AdjustWindowRectEx(&cr,GetWindowLong(wnd,GWL_STYLE),0,GetWindowLong(wnd,GWL_EXSTYLE));
CenterRect(&cr,&wr);
MoveWindow(wnd,cr.left,cr.top,cr.right-cr.left,cr.bottom-cr.top,1);
GetClientRect(wnd,(RECT*)&uifrm.l);
}

void UIMODLG::Run()
{
done=0;
ShowWindow(wnd,SW_SHOW);
EnableWindow(hwp,0);
MSG message;
while(!done)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  if(message.message==WM_QUIT)
   {
   done=1;
   PostQuitMessage(message.wParam);
   }
  else
   { 
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 }
EnableWindow(hwp,1);
ShowWindow(wnd,SW_HIDE);
for(int i=1;i<uifrm.nri;i++)
 uifrm.uitems[i]->Free();
uifrm.nri=1;
}

int UIMODLG::Pick(LPSTR label,LPSTR list,int ind=0)
{
SetWindowText(wnd,"Pick");
uifrm.AddI(-1,"Defs",IT_M,IL_BLP,0x0,&ind,0,label,NULL,NULL,0,50,300,0,list);
uifrm.Align();
Run();
return ind;
}

int UIMODLG::PickL(LPSTR label,LPSTR list,NAT span=0,NAT icnt=1,int ind=0)
{
SetWindowText(wnd,"Pick");
uifrm.AddI(-1,"List",IT_M,IL_BLP,0x0,&ind,0,label,NULL,NULL,0,50,300,0,NULL);
uifrm.uitems[1]->List(list,span,icnt);
uifrm.Align();
Run();
return ind;
}

int UIMODLG::PickS(LPSTR label,char**list,NAT icnt=1,NAT span=1,int ind=0)
{
SetWindowText(wnd,"Pick");
uifrm.AddI(-1,"Slut",IT_M,IL_BLP,0x0,&ind,0,label,NULL,NULL,0,50,300,0,NULL);
uifrm.uitems[1]->Slut(list,icnt,span);
uifrm.Align();
Run();
return ind;
}
// UIMODLG >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif