#pragma once
#define V_WINTERFACE

#include <gdi.cpp>
#include <winuser.h>

#define MENU(x) (100+x)
#define BUTT(x) (300+x)
#define CBOX(x) (500+x)
#define EDIT(x) (700+x)
#define LIST(x) (800+x)
#define STAT(x) (900+x)
#define HOTK(x) (1000+x)
#define SPIN(x) (1100+x)
#define SLID(x) (1300+x)
#define DISP(x) (1500+x)
#define SBAR(x) (2000+x)

#define W_ID(c0,c1) (WORD)(c0|(c1<<8)) //word id

//string to word id (mainly for menus) --------------------------------------------------------------------
inline WORD StrToW_ID(LPSTR label)
{
NAT l;
l=firstch('&',label);
if(label[l]=='&')
 return *((WORD*)(label+l+1));
l=sl(label); 
if(l>=1)
 return label[0]|(label[l-1]<<8);
else
 return '_'|('_'<<8);
}

//string to dword id (mainly for treeview) --------------------------------------------------------------------
inline DWORD StrToDW_ID(LPSTR label)
{
NAT l;
l=firstch('&',label);
if(label[l]=='&')
 return *((DWORD*)(label+l+1));
l=sl(label); 
if(l>=2)
 return label[0]|(label[1]<<8)|(label[l-2]<<16)|(label[l-1]<<24);
else if(l>=1)
 return label[0]|(label[0]<<8)|(label[l-1]<<16)|(label[l-1]<<24);
else
 return '_'|('_'<<8)|('_'<<16)|('_'<<24); 
}

//registers a window class ----------------------------------------------------------------
WNDCLASSEX WindowClass(LPSTR wcn,WNDPROC wp=NULL,UINT style=0,HBRUSH hbrs=NULL,HCURSOR hcur=NULL,HICON hibig=NULL,HICON hism=NULL)
{
WNDCLASSEX wc; //main window class
wc.cbSize=sizeof(WNDCLASSEX);
wc.style=style;
wc.lpfnWndProc=wp?wp:DefWindowProc;
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=appinst;
wc.hIcon=hibig?hibig:LoadIcon(NULL,IDI_WINLOGO);
wc.hCursor=hcur?hcur:LoadCursor(NULL,IDC_ARROW);
wc.hbrBackground=hbrs?hbrs:(HBRUSH)(COLOR_BTNFACE+1);
wc.lpszMenuName=NULL;
wc.lpszClassName=wcn;
wc.hIconSm=hism?hism:wc.hIcon;
RegisterClassEx(&wc);
return wc;
}

//use in long loops so the program doesn't freeze -------------------------------------------------
inline void AntiBlock()
{
MSG message;
while(PeekMessage(&message,NULL,0,0,PM_REMOVE))
 {
 if(message.message!=WM_QUIT&&message.message!=WM_DESTROY)
  {
  TranslateMessage(&message);
  DispatchMessage(&message);
  }
 }
}

//sets window caption -----------------------------------------------------------------------------
void wndcap(HWND hwnd,char *formstr,...)
{
char buffer[BUFFSZ];
va_list vparam;
va_start(vparam,formstr);
vsprintf(buffer,formstr,vparam);
SetWindowText(hwnd,buffer);
va_end(vparam);
}

//appends a message to window text -----------------------------------------------------------------
void wndtext(HWND hwnd,char *formstr,...)
{
va_list vparam;
va_start(vparam,formstr);
char buffer[BUFFSZ];
vsprintf(buffer+GetWindowText(hwnd,buffer,BUFFSZ),formstr,vparam);
SetWindowText(hwnd,buffer);
va_end(vparam);
}

//builds a rectangle with screen relative client coordinates -----------------------------------
inline void GetAbsClientRect(RECT *rct,HWND hwnd=NULL)
{
ifn(GetClientRect(hwnd,rct))
 return;
ClientToScreen(hwnd,(POINT*)rct);
rct->right+=rct->left;
rct->bottom+=rct->top;
}

//set the client area size and (ex)style of a window ----------------------------------------------------
inline void AdjustClientRect(HWND hwnd,int width=0,int height=0,int x=0,int y=0)
{
RECT cr;
ifn(GetClientRect(hwnd,&cr))
 return;
if(width) cr.right=width;
if(height) cr.bottom=height;
AdjustWindowRectEx(&cr,GetWindowLong(hwnd,GWL_STYLE),(BOOL)GetMenu(hwnd),GetWindowLong(hwnd,GWL_EXSTYLE));
MoveWindow(hwnd,x,y,cr.right-cr.left,cr.bottom-cr.top,1);
}

//get extra space occupied by window ----------------------------------------------------
inline void GetWindowBorders(HWND hwnd,int&ew,int&eh)
{
RECT lr;
GetWindowRect(hwnd,&lr);
ew=lr.right-lr.left;
eh=lr.bottom-lr.top;
GetClientRect(hwnd,&lr);
ew-=lr.right;
eh-=lr.bottom;
}

//adjust window so (lng,lat) is shrinked only if it doesn't fit on screen also keeping aspect ratio ------------
void FitWindowToRect(HWND hwnd,int lng,int lat,int hb=0,int vb=0)
{
int ew,eh;
double f;
if(!lng||!lat) return;
SystemParametersInfo(SPI_GETWORKAREA,0,&mwarea,0);
GetWindowBorders(hwnd,ew,eh);
ew+=hb;
eh+=vb;
f=MIN3(1.,(double)(mwarea.right-mwarea.left-ew)/lng,(double)(mwarea.bottom-mwarea.top-eh)/lat);
AdjustClientRect(hwnd,lng*f+hb,lat*f+vb,(mwarea.right+mwarea.left-ew-(lng*f))/2,(mwarea.bottom+mwarea.top-eh-(lat*f))/2);
}

//centers client in window keeping width and height ------------------------------------------
inline void CenterDlg(HWND hdlg,HWND hwnd=NULL)
{
RECT wrct,drct;
GetWindowRect(hwnd,&wrct);
GetWindowRect(hdlg,&drct);
drct.left=(wrct.right-wrct.left-drct.right+drct.left)>>1;
drct.right=wrct.right-drct.left;
drct.left+=wrct.left;
drct.top=(wrct.bottom-wrct.top-drct.bottom+drct.top)>>1;
drct.bottom=wrct.bottom-drct.top;
drct.top+=wrct.top;
MoveWindow(hdlg,drct.left,drct.top,drct.right-drct.left,drct.bottom-drct.top,1);
}

//centers window in work area keeping width and height ------------------------------------------
inline void CenterOnScreen(HWND hwnd=NULL)
{
RECT wrct;
SystemParametersInfo(SPI_GETWORKAREA,0,&mwarea,0);
GetWindowRect(hwnd,&wrct);
wrct.left=(mwarea.right-mwarea.left-wrct.right+wrct.left)>>1;
wrct.right=mwarea.right-wrct.left;
wrct.left+=mwarea.left;
wrct.top=(mwarea.bottom-mwarea.top-wrct.bottom+wrct.top)>>1;
wrct.bottom=mwarea.bottom-wrct.top;
wrct.top+=mwarea.top;
//MoveWindow(hwnd,wrct.left,wrct.top,wrct.right-wrct.left,wrct.bottom-wrct.top,1);
SetWindowPos(hwnd,HWND_TOP,wrct.left,wrct.top,wrct.right-wrct.left,wrct.bottom-wrct.top,SWP_NOSIZE|SWP_NOACTIVATE);
}

//interface for InsertMenuItem ------------------------------------------------------------
//! Atentie functie recursiva
HMENU MakeMenuX(LPSTR menuf)
//| -separates menu items, > -item is a popup submenu, _ -line separator, / -vertical line separator, ^ -new column separator
{
HMENU hlmenu;
MENUITEMINFO mii;
char item[NAMESZ],*submenuf,lsep=' ';
int l,go=1,pos=0;
mii.cbSize=sizeof(MENUITEMINFO);
mii.fMask=MIIM_TYPE|MIIM_ID|MIIM_SUBMENU;
mii.fState=MFS_UNHILITE;
mii.hbmpChecked=NULL;
mii.hbmpUnchecked=NULL;
mii.dwItemData=NULL;
mii.cch=0;
mii.dwTypeData=item;
mii.fType=MFT_STRING;
if(*menuf=='>') 
 {
 hlmenu=CreatePopupMenu();
 menuf++;
 }
else
 hlmenu=CreateMenu();
while(go)
 {
 if(*menuf=='\0')
  break;
 if(lsep=='_')
  {
  mii.fType=MFT_SEPARATOR;
  InsertMenuItem(hlmenu,pos++,TRUE,&mii); 
  mii.fType=MFT_STRING;
  }
 else if(lsep=='^')
  mii.fType=MFT_STRING|MFT_MENUBREAK; 
 else if(lsep=='/')
  mii.fType=MFT_STRING|MFT_MENUBARBREAK; 
 else 
  mii.fType=MFT_STRING;
 mii.hSubMenu=NULL;
 menuf+=sc_any(item,menuf,"|_/^>\0<",7);
 lsep=*menuf;
 if(*menuf=='\0')
  go=0;
 else if(*menuf=='>')  //sub menu
  {
  submenuf=SALLOC(BUFFSZ);
  l=sc_paren(submenuf,menuf,'>','<');
  submenuf[l-1]='\0'; //delete last '<' paren 
  mii.hSubMenu=MakeMenuX(submenuf+1);
  FREE(submenuf);
  menuf+=l;
  }
 else
  menuf++;
 mii.wID=StrToW_ID(item);
 InsertMenuItem(hlmenu,pos++,TRUE,&mii); 
 }
return hlmenu;
}

//interface for InsertMenuItem ------------------------------------------------------------
HMENU MakeMenu(LPSTR menuf,UINT id0,...)
//| -separates menu items(marks popup), > -item is a popup submenu, _ -line separator, / -vertical line separator, ^ -new column separator
{
HMENU hlmenu;
MENUITEMINFO mii;
BOOL go=TRUE;
NAT el;
char item[NAMESZ];
va_list vparam;
va_start(vparam,id0);
mii.cbSize=sizeof(MENUITEMINFO);  
mii.fMask=MIIM_TYPE|MIIM_ID|MIIM_SUBMENU; 
mii.fState=MFS_UNHILITE; 
mii.hbmpChecked=NULL; 
mii.hbmpUnchecked=NULL; 
mii.dwItemData=NULL; 
mii.cch=0; 
mii.dwTypeData=item; 
if(*menuf=='|') 
 {
 hlmenu=CreatePopupMenu();
 menuf++;
 }
else
 hlmenu=CreateMenu();
while(go)
 {
 mii.hSubMenu=NULL; 
 mii.fType=MFT_STRING; 
 el=0;
 while(*menuf!='|'&&go)
  {
  if(*menuf=='\0') go=FALSE;
  else if(*menuf=='_') mii.fType=MFT_SEPARATOR;
  else if(*menuf=='^') mii.fType=MFT_MENUBREAK; 
  else if(*menuf=='/') mii.fType=MFT_MENUBARBREAK; 
  else if(*menuf=='>') mii.hSubMenu=va_arg(vparam,HMENU); 
  else item[el++]=*menuf; //copy
  menuf++;
  }
 menuf++;
 item[el]=NULL;
 mii.wID=id0; 
 InsertMenuItem(hlmenu,id0,FALSE,&mii); 
 id0++;
 }
va_end(vparam);
return hlmenu;
}

//Print common dialog box -----------------------------------------------------------------
HDC GetPrintInfo(int*pag1=NULL,int*pag2=NULL,DWORD flags=0)
{
PRINTDLG pd;
DOCINFO di;
int jobid;
ZeroMemory(&pd,sizeof(PRINTDLG));
pd.lStructSize=sizeof(PRINTDLG);
pd.hwndOwner=hmwnd;
pd.Flags=PD_RETURNDC|flags;
pd.nFromPage=1;
pd.nToPage=1;
pd.nMinPage=pag1?*pag1:1;
pd.nMaxPage=pag2?*pag2:1;
pd.nCopies=1;
if(!pag1&&!pag2) pd.Flags|=PD_NOPAGENUMS;
else pd.Flags|=PD_PAGENUMS;
erret=PrintDlg(&pd);
if(pd.hDevMode) GlobalFree(pd.hDevMode);
if(pd.hDevNames) GlobalFree(pd.hDevNames);
if(!erret) return NULL;
if(pd.Flags&PD_PAGENUMS) //if explicit page range return in p1 and p2
 {
 if(pag1) *pag1=pd.nFromPage;
 if(pag2) *pag2=pd.nToPage;
 }
else if(pd.Flags&PD_SELECTION) //if selection set p1=p2=0
 {
 if(pag1) *pag1=0;
 if(pag2) *pag2=0;
 }
di.cbSize=sizeof(DOCINFO);
di.lpszDocName=AppName;
di.lpszOutput=NULL;
di.lpszDatatype=NULL;
di.fwType=0;
jobid=StartDoc(pd.hDC,&di);
if(jobid<=0)
 {
 DeleteDC(pd.hDC);
 return NULL;
 }
return pd.hDC;  //must call EndDoc() and DeleteDC()
}

//get page orientation, size and border in mm ---------------------------------------------------------------
int GetPageInfo(HDC*phdc=NULL,int*pagw=NULL,int*pagh=NULL,RECT*pagfrm=NULL)
{
PAGESETUPDLG psd;
DOCINFO di;
DEVMODE*dm;
DEVNAMES*dn;
int jobid;
ZeroMemory(&psd,sizeof(PAGESETUPDLG));
psd.lStructSize=sizeof(PAGESETUPDLG);
psd.hwndOwner=hmwnd;
psd.Flags=PSD_INHUNDREDTHSOFMILLIMETERS;
if(pagfrm)
 {
 psd.rtMargin.left=pagfrm->left*100;
 psd.rtMargin.right=pagfrm->right*100;
 psd.rtMargin.top=pagfrm->top*100;
 psd.rtMargin.bottom=pagfrm->bottom*100;
 psd.Flags|=PSD_MARGINS;
 }
else
 psd.Flags|=PSD_DISABLEMARGINS;
if(!phdc)
 psd.Flags|=PSD_DISABLEPRINTER;
erret=PageSetupDlg(&psd);
if(pagw) *pagw=psd.ptPaperSize.x/100;
if(pagh) *pagh=psd.ptPaperSize.y/100;
if(pagfrm)
 {
 pagfrm->left=psd.rtMargin.left/100;
 pagfrm->right=psd.rtMargin.right/100;
 pagfrm->top=psd.rtMargin.top/100;
 pagfrm->bottom=psd.rtMargin.bottom/100;
 }
if(!erret||!phdc)
 {
 GlobalFree(psd.hDevMode);
 GlobalFree(psd.hDevNames);
 return erret;
 }
dm=(DEVMODE*)GlobalLock(psd.hDevMode);
dn=(DEVNAMES*)GlobalLock(psd.hDevNames);
*phdc=CreateDC((char*)dn+dn->wDriverOffset,(char*)dn+dn->wDeviceOffset,NULL,dm);
GlobalUnlock(psd.hDevMode);
GlobalUnlock(psd.hDevNames);
GlobalFree(psd.hDevMode);
GlobalFree(psd.hDevNames);
if(*phdc==NULL) return 0; //DC creation failed
di.cbSize=sizeof(DOCINFO);
di.lpszDocName=AppName;
di.lpszOutput=NULL;
di.lpszDatatype=NULL;
di.fwType=0;
jobid=StartDoc(*phdc,&di);
if(jobid<=0) //job id
 {
 DeleteDC(*phdc);
 return 0;
 }
return 1; //must call EndDoc() and DeleteDC()
}

//creates and sets a font for a control in a dialog ------------------------------------------------
HFONT ControlFont(HWND hwnd,LPSTR name=NULL,int cellh=16,int atrib=0,DWORD chset=0)
 {
 HFONT hfnt;
 hfnt=CreateFont(cellh,0,0,0,(atrib&0xf)*100,(atrib>>4)&1,(atrib>>8)&1,(atrib>>12)&1,
                 chset,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,
                 FF_DONTCARE|(atrib>>16?FIXED_PITCH:DEFAULT_PITCH),name);
 SendMessage(hwnd,WM_SETFONT,(WPARAM)hfnt,1);
 return hfnt;
 }

//Checks a menu item ---------------------------------------------------------------------------
inline void CheckMark(HMENU hmenu,UINT pos=0,int check=0)
{
MENUITEMINFO mii;
mii.cbSize=sizeof(MENUITEMINFO);
mii.fMask=MIIM_STATE;
mii.fState=check?MFS_CHECKED:MFS_UNCHECKED;
SetMenuItemInfo(hmenu,pos,TRUE,&mii);
}

//interface for STATIC control ----------------------------------------------------------------------
SIZE MakeLabel(LPSTR label,int x,int y,HWND parent,DWORD style=0)
{
SIZE txtsz;
HDC hdc;
hdc=GetDC(parent);
GetTextExtentPoint32(hdc,label,strlen(label),&txtsz);
ReleaseDC(parent,hdc);
CreateWindowEx(0,"STATIC",label,WS_VISIBLE|WS_CHILD|SS_LEFTNOWORDWRAP|style,x,y,
               txtsz.cx,txtsz.cy,parent,NULL,appinst,NULL);
return txtsz;
}

//interface for DLG -----------------------------------------------------------------------------------
HWND MakeDlg(DWORD style,DWORD exstyle,DLGPROC lDlgProc,HWND parent,RECT rct,LPSTR title,BYTE id)
{
#pragma pack(push,default_pack)
#pragma pack(1) //byte packing
struct LOCAL_DLGTEMPLATE
 {
 DWORD style,dwExtendedStyle;
 WORD cdit;
 short x,y,cx,cy;
 WORD nomenu,defclass,notitle; //needed at end of DLGTEMPLATE
 }ldt;
#pragma pack(pop,default_pack)
HWND hdlg;
ldt.style=WS_POPUP|WS_BORDER|WS_SYSMENU|DS_MODALFRAME|WS_CAPTION|style;
ldt.dwExtendedStyle=exstyle;
ldt.cdit=0;
REGi=GetDialogBaseUnits();
ldt.x=MulDiv(rct.left,4,LOWORD(REGi));
ldt.y=MulDiv(rct.top,8,HIWORD(REGi));
ldt.cx=MulDiv(ldt.x+rct.right,4,LOWORD(REGi));
ldt.cy=MulDiv(ldt.y+rct.bottom,8,HIWORD(REGi));
ldt.nomenu=ldt.defclass=ldt.notitle=0;
hdlg=CreateDialogIndirectParam(appinst,(DLGTEMPLATE*)&ldt,parent,lDlgProc,0);
SetWindowText(hdlg,title);
return hdlg;
}

#define DEFAULT_NOTIFY_ICON_ID      1 
#define WM_NOTIFY_ICON              WM_USER+1 
int WM_TASKBAR_CREATED=0;

//creates icon in notification area -----------------------------------------------------------
void AddTaskbarIcon(HWND howner,HICON hico,char*ttip,UINT wm_id=WM_NOTIFY_ICON,UINT id=DEFAULT_NOTIFY_ICON_ID,int mod=0)
{
NOTIFYICONDATA nid;
ZeroMemory(&nid,sizeof(nid));
nid.cbSize=NOTIFYICONDATA_V1_SIZE;
nid.hWnd=howner;
nid.hIcon=hico;
sc(nid.szTip,ttip,63);
nid.uCallbackMessage=wm_id;
nid.uID=id;
nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
erret=Shell_NotifyIcon(mod?NIM_MODIFY:NIM_ADD,&nid);
}

//creates icon in notification area -----------------------------------------------------------
void DelTaskbarIcon(HWND howner,UINT id=DEFAULT_NOTIFY_ICON_ID)
{
NOTIFYICONDATA nid;
ZeroMemory(&nid,sizeof(nid));
nid.cbSize=NOTIFYICONDATA_V1_SIZE; //sizeof(nid);
nid.hWnd=howner;
nid.uID=id;
erret=Shell_NotifyIcon(NIM_DELETE,&nid);
}

//STATUS BAR interface ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class WStatusBar
{
public:
 HWND hwnd,hown; //status bar, owner
 int parts; //max 256
 RECT rect; //client
 
 WStatusBar() { hwnd=NULL; parts=0; }
 ~WStatusBar() { DestroyWindow(hwnd); }
 //,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, 
 void Init(int lparts=1,HWND parent=NULL,DWORD style=SBARS_SIZEGRIP)
  {
  hwnd=CreateWindowEx(0,STATUSCLASSNAME,NULL,WS_CHILD|WS_VISIBLE|style,
                      0,0,0,0,hown=parent?parent:hmwnd,(HMENU)SBAR(0),appinst,NULL);
  parts=lparts>0&&lparts<=256?lparts:1;
  Resize(0);
  }
 //wparam and lparam are owner's new sizes ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
 void Resize(LPARAM lparam)
  {
  int *pwidth;
  if(!hwnd) return;
  SendMessage(hwnd,WM_SIZE,SIZE_RESTORED,lparam);
  GetWindowRect(hwnd,&rect);
  pwidth=(int*)ALLOC(parts*sizeof(int));
  for(int i=0; i<parts; i++)
   pwidth[i]=(i+1)*(rect.right-rect.left)/parts; 
  SendMessage(hwnd,SB_SETPARTS,(WPARAM)parts,(LPARAM)pwidth); 
  FREE(pwidth);
  }
 //(max 127 chars per part) --------------------------------------------------------------------
 void stat(int part,char *formstr,...)
  {
  char buffer[BUFFSZ];
  va_list vparam;
  if(!hwnd) return;
  va_start(vparam,formstr);
  vsprintf(buffer,formstr,vparam);
  SendMessage(hwnd,SB_SETTEXT,(WPARAM)part,(LPARAM)buffer);
  va_end(vparam);
  }
 //,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
}; //main 
#ifndef V_MFC
WStatusBar wsb;
#endif

//removes current selection from a list box --------------------------------------------------
BOOL VListBox_RemoveSelected(HWND hlbox)
{
NAT nrs,sel;
for(;;)
 {
 nrs=SendMessage(hlbox,LB_GETSELITEMS,1,(LPARAM)&sel);
 if(!nrs) //no more
  return 1;
 else if(nrs==LB_ERR) //single selection
  {
  SendMessage(hlbox,LB_DELETESTRING,(WPARAM)SendMessage(hlbox,LB_GETCURSEL,0,0),0);
  return 1;
  }
 SendMessage(hlbox,LB_DELETESTRING,(WPARAM)sel,0);
 }
}

//Unsorted list box --------------------------------------------------------------------------
int ListBoxU(HWND hlbox,char*items,int append=0)
 { // |-separates list items, /-default item ( <- applies for prev text)
 char lbuf[NAMESZ];
 int i=0,count=0;
 if(!append) SendMessage(hlbox,LB_RESETCONTENT,0,0); //remove all
 while(*items)
  {
  if(*items=='|'||*items=='/'||*items=='^')
   {
   lbuf[i++]='\0';
   count++;
   i=SendMessage(hlbox,LB_ADDSTRING,0,(LPARAM)lbuf); //add
   if(*items=='/')
    SendMessage(hlbox,LB_SETCURSEL,(WPARAM)i,0); //select
   i=0;
   }
  else
   lbuf[i++]=*items;
  items++;
  }
 if(i) //add last item
  {
  lbuf[i++]='\0';
  count++;
  i=SendMessage(hlbox,LB_ADDSTRING,0,(LPARAM)lbuf);
  }
 return count;
 }

//Unsorted combo list ----------------------------------------------------------------------
int ComboListU(HWND hcbox,char*items,int append=0)
 { // |,-separates list items, /-default item, ^-just in edit ( <- applies for prev text)
 char lbuf[NAMESZ];
 int i=0,count=0;
 if(!append) SendMessage(hcbox,CB_RESETCONTENT,0,0); //remove all
 while(*items)
  {
  if(*items=='|'||*items==','||*items=='/'||*items=='^')
   {
   lbuf[i++]='\0';
   count++;
   if(*items=='^')
    SetWindowText(hcbox,lbuf); //edit
   else
    {
    i=SendMessage(hcbox,CB_ADDSTRING,0,(LPARAM)lbuf); //add
    if(*items=='/')
     SendMessage(hcbox,CB_SETCURSEL,(WPARAM)i,0); //select
    } 
   i=0;
   }
  else
   lbuf[i++]=*items;
  items++;
  }
 if(i) //add last item
  {
  lbuf[i++]='\0';
  count++;
  i=SendMessage(hcbox,CB_ADDSTRING,0,(LPARAM)lbuf);
  }
 return count;
 }

//interface for TreeView population ------------------------------------------------------------
int TreeViewU(HWND htree,char*menuf,int mod=0,HTREEITEM order=TVI_SORT)
//| -separates tree items, > -go down one level, <-go up one level
//mod: b0 - append, b1 - id format
{
char item[NAMESZ];
HTREEITEM htvitem[PATHDEPTH]; //stack
int level=0;
TVINSERTSTRUCT tvi;
tvi.hInsertAfter=order;
tvi.itemex.mask=TVIF_PARAM|TVIF_TEXT;
tvi.itemex.stateMask=0;
tvi.itemex.pszText=item;
if(!(mod&1)) TreeView_DeleteAllItems(htree); //don't append
for(;;)
 {
 menuf+=sc_any(item,menuf,"|><\0",4);
 if(level>=1)
  tvi.hParent=htvitem[level-1];
 else
  tvi.hParent=TVI_ROOT;
 if(mod&2) //first 2 last 2 chars
  tvi.itemex.lParam=StrToDW_ID(item);
 else  //level, unique number
  tvi.itemex.lParam=(SendMessage(htree,TVM_GETCOUNT,0,0)&0xffffff)|(level<<24);
 //tvi.itemex.cchTextMax=sl(item);
 htvitem[level]=(HTREEITEM)SendMessage(htree,TVM_INSERTITEM,0,(LPARAM)&tvi);
 if(*menuf=='\0')
  break;
 else if(*menuf=='>')  //sub menu
  {
  level++;
  if(level>PATHDEPTH-1) error("Invalid argument in TreeViewU(): too many '>' (stack overflow)");
  }
 else if(*menuf=='<')  //sub menu
  {
  if(level<1) error("Invalid argument in TreeViewU(): too many '<'");
  level--;
  }
 menuf++;
 }
return level;
}

#if(WINVER>0x0400)

//saves window to file (uses fixed number of bytes)-----------------------------------------------------------------
void WndFileSave(IOSFile*iof,HWND hwnd)
{
WNDCLASSEX wcls;
WINDOWINFO winf;
WINDOWPLACEMENT wpos;
char clsname[32];
wcls.cbSize=sizeof(WNDCLASSEX);
winf.cbSize=sizeof(WINDOWINFO);
wpos.length=sizeof(WINDOWPLACEMENT);
GetWindowInfo(hwnd,&winf);
GetWindowPlacement(hwnd,&wpos);
GetClassName(hwnd,clsname,32);
GetClassInfoEx(appinst,clsname,&wcls);
iof->write(&winf,sizeof(winf));
iof->write(&wpos,sizeof(wpos));
iof->write(&wcls,sizeof(wcls));
iof->write((void*)wcls.lpszClassName,32);
//Bytes written=sizeof(WNDCLASSEX)+sizeof(WINDOWINFO)+sizeof(WINDOWPLACEMENT)+32
}

//updates or creates window from data in file ---------------------------------------------------
HWND WndFileLoad(IOSFile*iof,HWND hwnd=NULL,WNDPROC wproc=NULL,HWND hpwnd=NULL,HBRUSH hbrs=NULL,HICON hico=NULL,HCURSOR hcur=NULL)
{
WNDCLASSEX wcls;
WINDOWINFO winf;
WINDOWPLACEMENT wpos;
char clsname[32];
iof->read(&winf,sizeof(winf));
iof->read(&wpos,sizeof(wpos));
iof->read(&wcls,sizeof(wcls));
iof->read(clsname,32);
if(hwnd) //just update
 {
 SetClassLong(hwnd,GCL_STYLE,wcls.style);
 SetWindowLong(hwnd,GWL_EXSTYLE,winf.dwExStyle);
 SetWindowLong(hwnd,GWL_STYLE,winf.dwStyle);
 MoveWindow(hwnd,winf.rcWindow.left,winf.rcWindow.top,winf.rcWindow.right-winf.rcWindow.left,winf.rcWindow.bottom-winf.rcWindow.top,1);
 }
else //create
 {
 wcls=WindowClass(clsname,wproc,wcls.style,hbrs,hcur,hico,hico);
 hwnd=CreateWindowEx(winf.dwExStyle,wcls.lpszClassName,REGs,winf.dwStyle,
      winf.rcWindow.left,winf.rcWindow.top,winf.rcWindow.right-winf.rcWindow.left,winf.rcWindow.bottom-winf.rcWindow.top,
      hpwnd,NULL,appinst,NULL);
 }
SetWindowPlacement(hwnd,&wpos);
//ShowWindow(hwnd,wpos.showCmd);
return hwnd;
}

#endif //WINVER>0x0400