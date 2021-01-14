#ifndef V_WINSHELL
#define V_WINSHELL

#include <COMPAT/ole.cpp>
#include <utree.cpp>
#include <shlobj.h>
#include <intshcut.h>
//#pragma comment(lib,"url.lib")

IShellFolder*shfdesk=NULL; //main desktop folder object

//dialog to choose path ---------------------------------------------------------------
BOOL BrowsePath(LPSTR pathbuf,LPSTR label=NULL,FLAGS flags=0,LPSTR rootpath=NULL,HWND howner=NULL)
{//BIF_BROWSEINCLUDEFILES|BIF_EDITBOX|BIF_NONEWFOLDERBUTTON|BIF_RETURNFSANCESTORS|BIF_RETURNONLYFSDIRS|BIF_UAHINT|
LPITEMIDLIST shfolder=NULL,shroot=NULL;
BROWSEINFO bi;
if(rootpath)
 {
 IShellFolder*psfdesk;
 SHGetDesktopFolder(&psfdesk);
 if(psfdesk)
  {
  A_W(REGsw,rootpath);
  psfdesk->ParseDisplayName(hmwnd,NULL,REGsw,NULL,&shroot,NULL);
  psfdesk->Release();
  }
 }
bi.hwndOwner=howner;
bi.pidlRoot=shroot;
bi.pszDisplayName=pathbuf;
bi.lpszTitle=label;
bi.ulFlags=flags|BIF_RETURNONLYFSDIRS|BIF_EDITBOX;
bi.lpfn=NULL;
bi.lParam=0;
bi.iImage=0;
shfolder=SHBrowseForFolder(&bi);
COFREE(shroot);
if(!shfolder) return 0;
flags=SHGetPathFromIDList(shfolder,pathbuf);
COFREE(shfolder);
return flags;
}

//--------------------------------------------------------------------------------------------------
BOOL InputPaths(NAT nr,DWORD flags,HWND parent,LPSTR title,/*"label",path_buffer,...*/...)
{
HWND hdlg,hok,hcancel,*hit;
HDC hdc;
MSG message;
LPSTR*param=&title+1;
int i,block=TRUE,retv=0;
SIZE z;
int lw=0,tw=0;
char lBufDir[PATHSZ];
hdc=GetDC(parent);
for(i=0;i<nr;i++)
 {
 if(param[i*2])
  {
  GetTextExtentPoint32(hdc,param[i*2],sl(param[i*2]),&z);
  if(z.cx>lw) lw=z.cx;
  }
 }
GetTextExtentPoint32(hdc,"W",1,&z);
ReleaseDC(parent,hdc);
z.cy+=4; //add border
tw=z.cx*260;
if(tw>(mwarea.r-mwarea.l-lw-100)) tw=(mwarea.r-mwarea.l-lw-100);
hdlg=CreateWindowEx(WS_EX_CONTROLPARENT,WC_DEFAULT_PROC,title,WS_POPUPWINDOW|WS_CAPTION,
		    0,0,lw+tw+50,z.cy*(nr*2+4),parent,NULL,appinst,NULL);
hok=CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
                  ((lw+tw+50)>>2)-30,z.cy*(nr*2+1),60,z.cy,hdlg,(HMENU)BUTT(nr),appinst,NULL);
hcancel=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                      ((lw+tw+50)>>2)*3-30,z.cy*(nr*2+1),60,z.cy,hdlg,(HMENU)BUTT(nr+1),appinst,NULL);
hit=(HWND*)ALLOC(nr*sizeof(HWND));
for(i=0;i<nr;i++)
 {
 if(param[i*2])
  CreateWindowEx(0,"STATIC",param[i*2],WS_VISIBLE|WS_CHILD|SS_LEFT,
                 20,z.cy*(i*2+1),lw,z.cy,hdlg,(HMENU)STAT(i),appinst,NULL);
 hit[i]=CreateWindowEx(0,"BUTTON",param[i*2+1],WS_VISIBLE|WS_CHILD|WS_TABSTOP|WS_GROUP|BS_FLAT|BS_VCENTER,
                       30+lw,z.cy*(i*2+1),tw,z.cy,hdlg,(HMENU)BUTT(i),appinst,NULL);
 }
if(parent)
 EnableWindow(parent,FALSE);
CenterOnScreen(hdlg);
ShowWindow(hdlg,SW_SHOW);
//SetFocus(hit[0]);
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  //!!! you cannot receive WM_COMMAND this way because WM_COMMAND (and other notifications) are sent directly to the WindowProc, they are not posted to the message queue
  if(message.message==WM_KEYUP)
   {
   if(WK_SCANCODE(message.lParam)==28) //ENTER
    SendMessage(hok,BM_SETSTATE,BST_PUSHED,0);
   else if(WK_SCANCODE(message.lParam)==1) //ESC
    SendMessage(hcancel,BM_SETSTATE,BST_PUSHED,0);
   }
  if(message.message==WM_LBUTTONUP||message.message==WM_KEYUP)
   {
   for(i=0;i<nr;i++)
    {
    if(SendMessage(hit[i],BM_GETSTATE,0,0)&BST_PUSHED)
     {
     SendMessage(hit[i],WM_GETTEXT,PATHSZ-1,(LPARAM)lBufDir);
     if(BrowsePath(lBufDir,param[i*2],flags,NULL,hdlg))
      SendMessage(hit[i],WM_SETTEXT,0,(LPARAM)lBufDir);
     break;
     }
    }
   if(SendMessage(hok,BM_GETSTATE,0,0)&BST_PUSHED) //Ok
    {
    block=FALSE;
    retv=1;
    for(i=0;i<nr;i++)
     SendMessage(hit[i],WM_GETTEXT,PATHSZ-1,(LPARAM)param[i*2+1]);
    }
   if(SendMessage(hcancel,BM_GETSTATE,0,0)&BST_PUSHED) //Cancel
    block=FALSE;
   }
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 if(!IsWindow(hdlg)) block=FALSE;
 }
EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
FREE(hit);
return retv;
}

//--------------------------------------------------------------------------------------------------
BOOL InputPathsUT(UTREE*pathlist,DWORD flags,LPSTR title=NULL,HWND parent=NULL,LPSTR root=NULL)
{
HWND hdlg,hok,hcancel,hadd,hrem,hlst;
MSG message;
int i,block=TRUE,retv=0;
SIZE z;
char lBufDir[PATHSZ];
z.cy=24; //add border
z.cx=400;
hdlg=CreateWindowEx(WS_EX_CONTROLPARENT,WC_DEFAULT_PROC,title,WS_POPUPWINDOW|WS_CAPTION,
		    0,0,z.cx+8,z.cy*20,parent,NULL,appinst,NULL);
hok=CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
                   (z.cx>>2)-30,z.cy*17+z.cy/2,60,z.cy,hdlg,(HMENU)BUTT(0),appinst,NULL);
hcancel=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                   (z.cx>>2)*3-30,z.cy*17+z.cy/2,60,z.cy,hdlg,(HMENU)BUTT(1),appinst,NULL);
hadd=CreateWindowEx(0,"BUTTON","Add",WS_VISIBLE|WS_CHILD|WS_TABSTOP,
                   (z.cx>>2)-40,z.cy/2,80,z.cy,hdlg,(HMENU)BUTT(2),appinst,NULL);
hrem=CreateWindowEx(0,"BUTTON","Remove",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                   (z.cx>>2)*3-40,z.cy/2,80,z.cy,hdlg,(HMENU)BUTT(3),appinst,NULL);
hlst=CreateWindowEx(0,"ListBox","",WS_VISIBLE|WS_CHILD|LBS_EXTENDEDSEL|LBS_STANDARD|LBS_NOINTEGRALHEIGHT,
                    20,z.cy*2,z.cx-40,z.cy*15,hdlg,(HMENU)LIST(0),appinst,NULL);
pathlist->HList(hlst,0x3);
if(parent) EnableWindow(parent,FALSE);
CenterOnScreen(hdlg);
ShowWindow(hdlg,SW_SHOW);
//SetFocus(hit[0]);
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  //!!! you cannot receive WM_COMMAND this way because WM_COMMAND (and other notifications) are sent directly to the WindowProc, they are not posted to the message queue
  if(message.message==WM_KEYUP)
   {
   if(WK_SCANCODE(message.lParam)==28) //ENTER
    SendMessage(hok,BM_SETSTATE,BST_PUSHED,0);
   else if(WK_SCANCODE(message.lParam)==1) //ESC
    SendMessage(hcancel,BM_SETSTATE,BST_PUSHED,0);
   }
  if(message.message==WM_LBUTTONUP||message.message==WM_KEYUP)
   {
   if(SendMessage(hadd,BM_GETSTATE,0,0)&BST_PUSHED) //add
    {
    *lBufDir=0;
    if(BrowsePath(lBufDir,"Add path",flags,root,hdlg))
      SendMessage(hlst,LB_ADDSTRING,0,(LPARAM)lBufDir);
    }
   else if(SendMessage(hrem,BM_GETSTATE,0,0)&BST_PUSHED) //remove
    {
    VListBox_RemoveSelected(hlst);
    }
   else if(SendMessage(hok,BM_GETSTATE,0,0)&BST_PUSHED) //Ok
    {
    block=FALSE;
    pathlist->Free();
    for(i=0;i<SendMessage(hlst,LB_GETCOUNT,0,0);i++)
     {
     SendMessage(hlst,LB_GETTEXT,i,(LPARAM)lBufDir);
     pathlist->Add(lBufDir);
     }
    retv=1;
    }
   if(SendMessage(hcancel,BM_GETSTATE,0,0)&BST_PUSHED) //Cancel
    block=FALSE;
   }
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 if(!IsWindow(hdlg)) block=FALSE;
 }
EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
return retv;
}

#endif
