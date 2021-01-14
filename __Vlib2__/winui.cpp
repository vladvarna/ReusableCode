#pragma once
#define V_WIN_GENERIC_CTRLS

#include <winter.cpp>

//Dialog for reading a string or a number ---------------------------------------------------------
int InputBox(LPSTR txtbuf,LPSTR label,int maxdig=0,HWND parent=HWND_DESKTOP,int number=0)
{
HWND hdlg,hed,hok,hcancel,hspin;
SIZE txtsz;
HDC hdc;
int block;
if(number)
 {
 number=ES_NUMBER;
 if(!txtbuf)
  {
  txtbuf=REGs;
  if(maxdig>BUFFSZ)
   maxdig=BUFFSZ;
  }
 }
if(maxdig<=0) maxdig=BUFFSZ;
hdc=GetDC(parent);
GetTextExtentPoint32(hdc,label,strlen(label),&txtsz);
ReleaseDC(parent,hdc);
if(txtsz.cx>300) 
 {
 txtsz.cy+=txtsz.cx/300*txtsz.cy+1;
 txtsz.cx=300;
 }
else if(txtsz.cx<80)
 txtsz.cx=80;
hdlg=CreateWindowEx(0,WC_DEFAULT_PROC,number?"Number":"String",WS_POPUPWINDOW|WS_CAPTION,
					0,0,txtsz.cx+100,txtsz.cy+120,parent,NULL,appinst,NULL);
//isERROR("CreateWindow");
CreateWindowEx(0,"STATIC",label,WS_VISIBLE|WS_CHILD|SS_CENTER,
			   50,15,txtsz.cx,txtsz.cy,hdlg,(HMENU)STAT(0),appinst,NULL);
hed=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",txtbuf,WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL|WS_TABSTOP|WS_GROUP|number,
                   50,txtsz.cy+20,txtsz.cx,22,hdlg,(HMENU)EDIT(0),appinst,NULL);
if(number)
 hspin=CreateUpDownControl(WS_VISIBLE|WS_CHILD|WS_TABSTOP|WS_GROUP|UDS_ARROWKEYS|UDS_SETBUDDYINT|UDS_WRAP|UDS_ALIGNRIGHT,
       0,0,16,22,hdlg,SPIN(0),appinst,hed,0x7fff,0x8000,0);
hok=CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
                  ((txtsz.cx+100)>>2)-30,txtsz.cy+60,60,22,hdlg,(HMENU)BUTT(0),appinst,NULL);
hcancel=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                      ((txtsz.cx+100)>>2)*3-30,txtsz.cy+60,60,22,hdlg,(HMENU)BUTT(1),appinst,NULL);
SendMessage(hed,EM_SETLIMITTEXT,(WPARAM)maxdig,0);
if(parent)
 {
 CenterDlg(hdlg,parent);
 EnableWindow(parent,FALSE);
 }
else
 CenterOnScreen (hdlg);
ShowWindow(hdlg,SW_SHOW);
SetFocus(hed);
block=TRUE;
MSG message;
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  if(message.message==WM_KEYDOWN)
   {
   if(WK_SCANCODE(message.lParam)==28) //ENTER
    SendMessage(hok,BM_SETSTATE,BST_PUSHED,0);
   else if(WK_SCANCODE(message.lParam)==1) //ESC
    SendMessage(hcancel,BM_SETSTATE,BST_PUSHED,0);
   }
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 else
  WaitMessage();
 if(SendMessage(hok,BM_GETSTATE,0,0)&BST_PUSHED) //Ok
  {
  SendMessage(hed,WM_GETTEXT,(WPARAM)(maxdig+1),(LPARAM)txtbuf);
  block=FALSE;
  }
 if(SendMessage(hcancel,BM_GETSTATE,0,0)&BST_PUSHED) //Cancel
  {
  txtbuf[0]=0;
  block=FALSE;
  }
 if(!IsWindow(hdlg)) block=FALSE;
 }
EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
if(number) return atoi(txtbuf);
else return strlen(txtbuf);
}

//Dialog for reading a number ---------------------------------------------------------
double InputNumber(LPSTR label,HWND parent=HWND_DESKTOP,double def=0.,double min=0.,double max=0.,NAT lprec=0,NAT lradix=10)
{
double retv;
HWND hdlg,hed,hok,hcancel,hslide=NULL;
HDC hdc;
SIZE z;
int block,maxdig=255,lpos,npos;
char txtbuf[256];
hdc=GetDC(parent);
GetTextExtentPoint32(hdc,label,strlen(label),&z);
ReleaseDC(parent,hdc);
if(z.cx<200) z.cx=200;
z.cy+=4;
RtoS(def,txtbuf,80,lradix,lprec);
hdlg=CreateWindowEx(0,WC_DEFAULT_PROC,"Number",WS_POPUP|WS_CAPTION,
					0,0,10,10,parent,NULL,appinst,NULL);
AdjustClientRect(hdlg,z.cx+40,z.cy*7);
CreateWindowEx(0,"STATIC",label,WS_VISIBLE|WS_CHILD|SS_CENTER,
			   20,z.cy*0.5,z.cx,z.cy,hdlg,(HMENU)STAT(0),appinst,NULL);
hed=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",txtbuf,WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL|WS_TABSTOP|WS_GROUP,
                   20,z.cy*2,z.cx,z.cy,hdlg,(HMENU)EDIT(0),appinst,NULL);
hslide=CreateWindowEx(0,TRACKBAR_CLASS,txtbuf,WS_VISIBLE|WS_CHILD|WS_TABSTOP|WS_GROUP|TBS_AUTOTICKS,
                      20,z.cy*3,z.cx,z.cy*1.5,hdlg,(HMENU)SLID(0),appinst,NULL);
hok=CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
                  ((z.cx+40)>>2)-30,z.cy*5.2,60,z.cy,hdlg,(HMENU)BUTT(0),appinst,NULL);
hcancel=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                      ((z.cx+40)>>2)*3-30,z.cy*5.2,60,z.cy,hdlg,(HMENU)BUTT(1),appinst,NULL);
SendMessage(hed,EM_SETLIMITTEXT,(WPARAM)maxdig,0);
if((long)min<(long)max)
 {
 lpos=def;
 SendMessage(hslide,TBM_SETRANGEMIN,0,min);
 SendMessage(hslide,TBM_SETRANGEMAX,0,max);
 SendMessage(hslide,TBM_SETTICFREQ,(max-min)/20,0);
 SendMessage(hslide,TBM_SETPOS,1,lpos);
 }
else
 {
 lpos=0;
 EnableWindow(hslide,FALSE);
 }
if(parent)
 {
 CenterDlg(hdlg,parent);
 EnableWindow(parent,FALSE);
 }
else
 CenterOnScreen (hdlg);
ShowWindow(hdlg,SW_SHOW);
SetFocus(hed);
block=TRUE;
MSG message;
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  if(message.message==WM_KEYDOWN)
   {
   if(WK_SCANCODE(message.lParam)==28) //ENTER
    SendMessage(hok,BM_SETSTATE,BST_PUSHED,0);
   else if(WK_SCANCODE(message.lParam)==1) //ESC
    SendMessage(hcancel,BM_SETSTATE,BST_PUSHED,0);
   }
  if((npos=SendMessage(hslide,TBM_GETPOS,0,0))!=lpos&&(GetFocus()==hslide))
   {
   //retv=StoR(txtbuf,radix)+(npos-lpos);
   RtoS(npos,txtbuf,80,lradix,lprec);
   SetWindowText(hed,txtbuf);
   lpos=npos;
   } 
  if(SendMessage(hok,BM_GETSTATE,0,0)&BST_PUSHED) //Ok
   {
   GetWindowText(hed,txtbuf,maxdig);
   retv=StoR(txtbuf,lradix);
   block=FALSE;
   }
  if(SendMessage(hcancel,BM_GETSTATE,0,0)&BST_PUSHED) //Cancel
   {
   txtbuf[0]=0;
   retv=def;
   block=FALSE;
   }
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 else
  WaitMessage();
 if(!IsWindow(hdlg)) block=FALSE;
 }
EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
if(min<max)
 return CLAMP(retv,min,max);
else 
 return retv;
}

//--------------------------------------------------------------------------------------------------
BOOL InputStrings(NAT nr,HWND parent,LPSTR title,/*"label",buffer,buffer_size-1,...*/...)
{
HWND hdlg,hok,hcancel,*hed;
HDC hdc;
MSG message;
LPSTR*params=&title+1;
int i,block=TRUE,retv=0;
SIZE z;
int lw=0,tw=0;
hdc=GetDC(parent);
for(i=0;i<(nr*3);i+=3)
 {
 if(((NAT)params[i+2])>tw) tw=(NAT)params[i+2];
 if(params[i])
  {
  GetTextExtentPoint32(hdc,params[i],sl(params[i]),&z);
  if(z.cx>lw) lw=z.cx;
  }
 }
GetTextExtentPoint32(hdc,"X",1,&z);
ReleaseDC(parent,hdc);
z.cy+=4; //add border
tw*=z.cx;
if(tw>(mwarea.r-mwarea.l-lw-100)) tw=(mwarea.r-mwarea.l-lw-100);
hdlg=CreateWindowEx(0,WC_DEFAULT_PROC,title,WS_POPUPWINDOW|WS_CAPTION,
		    0,0,lw+tw+50,z.cy*(nr*2+4),parent,NULL,appinst,NULL);
hok=CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
                  ((lw+tw+50)>>2)-30,z.cy*(nr*2+1),60,z.cy,hdlg,(HMENU)BUTT(0),appinst,NULL);
hcancel=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                      ((lw+tw+50)>>2)*3-30,z.cy*(nr*2+1),60,z.cy,hdlg,(HMENU)BUTT(1),appinst,NULL);
hed=(HWND*)ALLOC(nr*sizeof(HWND));
for(i=0;i<nr;i++)
 {
 if(params[i*3])
  CreateWindowEx(0,"STATIC",params[i*3],WS_VISIBLE|WS_CHILD|SS_LEFT,
                 20,z.cy*(i*2+1),lw,z.cy,hdlg,(HMENU)STAT(i),appinst,NULL);
 hed[i]=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",params[i*3+1],WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL|WS_TABSTOP|WS_GROUP|(scmp(params[i*3],"PASSWORD")?ES_PASSWORD:0),
                 30+lw,z.cy*(i*2+1),tw,z.cy,hdlg,(HMENU)EDIT(i),appinst,NULL);
 SendMessage(hed[i],EM_SETLIMITTEXT,(WPARAM)params[i*3+2],0);
 }
if(parent)
 EnableWindow(parent,FALSE);
CenterOnScreen (hdlg);
ShowWindow(hdlg,SW_SHOW);
SetFocus(hed[0]);
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  if(message.message==WM_KEYDOWN)
   {
   if(WK_SCANCODE(message.lParam)==28) //ENTER
    SendMessage(hok,BM_SETSTATE,BST_PUSHED,0);
   else if(WK_SCANCODE(message.lParam)==1) //ESC
    SendMessage(hcancel,BM_SETSTATE,BST_PUSHED,0);
   }
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 else
  WaitMessage();
 if(SendMessage(hok,BM_GETSTATE,0,0)&BST_PUSHED) //Ok
  {
  block=FALSE;
  retv=1; //ok
  for(i=0;i<nr;i++)
   SendMessage(hed[i],WM_GETTEXT,(WPARAM)(((NAT)params[i*3+2])+1),(LPARAM)params[i*3+1]);
  }
 if(SendMessage(hcancel,BM_GETSTATE,0,0)&BST_PUSHED) //Cancel
  block=FALSE;
 if(!IsWindow(hdlg)) block=FALSE;
 }
EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
FREE(hed);
return retv;
}

//--------------------------------------------------------------------------------------------------
DWORD InputFlags(NAT nr,DWORD flags,HWND parent,LPSTR title,/*"label",mask,...*/...)
{
HWND hdlg,hok,hcancel,*hit;
HDC hdc;
MSG message;
LPSTR*label=&title+1;
DWORD*mask=(DWORD*)&title+2;
int i,block=TRUE;
SIZE z;
int lw=0;
hdc=GetDC(parent);
for(i=0;i<(nr*2);i+=2)
 {
 GetTextExtentPoint32(hdc,label[i],sl(label[i]),&z);
 if(z.cx>lw) lw=z.cx;
 }
ReleaseDC(parent,hdc);
z.cy+=4; //add border
if(lw<100) lw=100;
hdlg=CreateWindowEx(0,WC_DEFAULT_PROC,title,WS_POPUPWINDOW|WS_CAPTION,
		    0,0,lw+48,z.cy*(nr+2)+z.cy/2*(nr+3),parent,NULL,appinst,NULL);
hok=CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
                  ((lw+40)>>2)-20,z.cy*nr+z.cy/2*(nr+1),50,z.cy,hdlg,(HMENU)BUTT(nr),appinst,NULL);
hcancel=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                      ((lw+40)>>2)*3-30,z.cy*nr+z.cy/2*(nr+1),50,z.cy,hdlg,(HMENU)BUTT(nr+1),appinst,NULL);
hit=(HWND*)ALLOC(nr*sizeof(HWND));
for(i=0;i<nr;i++)
 {
 hit[i]=CreateWindowEx(0,"BUTTON",label[i*2],WS_VISIBLE|WS_CHILD|WS_TABSTOP|WS_GROUP|BS_AUTOCHECKBOX|BS_LEFTTEXT|BS_FLAT,
                       10,z.cy*i+z.cy/2*(i+1),lw+20,z.cy,hdlg,(HMENU)BUTT(i),appinst,NULL);
 SendMessage(hit[i],BM_SETCHECK,flags&mask[i*2],0);
 }
if(parent)
 {
 CenterDlg(hdlg,parent);
 EnableWindow(parent,FALSE);
 }
else
 CenterOnScreen(hdlg);
ShowWindow(hdlg,SW_SHOW);
//SetFocus(hit[0]);
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  if(message.message==WM_KEYDOWN)
   {
   if(WK_SCANCODE(message.lParam)==28) //ENTER
    SendMessage(hok,BM_SETSTATE,BST_PUSHED,0);
   else if(WK_SCANCODE(message.lParam)==1) //ESC
    SendMessage(hcancel,BM_SETSTATE,BST_PUSHED,0);
   }
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 else
  WaitMessage();
 if(SendMessage(hok,BM_GETSTATE,0,0)&BST_PUSHED) //Ok
  {
  block=FALSE;
  for(i=0;i<nr;i++)
   {
   if(SendMessage(hit[i],BM_GETCHECK,0,0)==BST_CHECKED)
    flags|=mask[i*2];
   else 
    flags&=~mask[i*2];
   }
  }
 if(SendMessage(hcancel,BM_GETSTATE,0,0)&BST_PUSHED) //Cancel
  block=FALSE;
 if(!IsWindow(hdlg)) block=FALSE;
 }
EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
FREE(hit);
return flags;
}

//-------------------------------------------------------------------------------------------------- 
int ChooseButton(LPSTR title,LPSTR label,HWND parent,NAT nb,...)
{
va_list vparam;
HWND hdlg;
SIZE z,bz;
char*lpstr;
HDC hdc;
int block,retv=-1,b; //buttons per row
if(nb<2) return 0;
va_start(vparam,nb);
hdc=GetDC(parent);
bz.cx=0;
for(b=0;b<nb;b++)
 {
 lpstr=va_arg(vparam,char*);
 GetTextExtentPoint32(hdc,lpstr,strlen(lpstr),&z);
 if(z.cx>bz.cx) bz.cx=z.cx;
 }
bz.cy=z.cy+10;
bz.cx+=20;
GetTextExtentPoint32(hdc,label,strlen(label),&z);
ReleaseDC(parent,hdc);
hdlg=CreateWindowEx(0,WC_DEFAULT_PROC,title?title:"Choose...",WS_POPUP|WS_CAPTION|WS_BORDER,
					0,0,10,10,parent,NULL,appinst,NULL);
AdjustClientRect(hdlg,(bz.cx+20)*nb+20,bz.cy+z.cy*4);
CreateWindowEx(0,"STATIC",label,WS_VISIBLE|WS_CHILD|SS_LEFT,
              (((bz.cx+20)*nb+20)-z.cx)/2,z.cy,z.cx,z.cy,hdlg,(HMENU)STAT(0),appinst,NULL);
va_start(vparam,nb);
for(b=0;b<nb;b++)
 CreateWindowEx(0,"BUTTON",va_arg(vparam,char*),WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_NOTIFY,
               (bz.cx+20)*b+20,z.cy*3,bz.cx,bz.cy,hdlg,(HMENU)BUTT(b),appinst,NULL);
CenterOnScreen(hdlg);
ShowWindow(hdlg,SW_SHOW);
//EnableWindow(parent,FALSE);
block=TRUE;
retv=0;
MSG message;
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  if(message.message==WM_LBUTTONUP)
   for(b=0;b<nb;b++)
    if(SendDlgItemMessage(hdlg,BUTT(b),BM_GETSTATE,0,0)&BST_PUSHED) //Ok
     {
     retv=BUTT(b)-BUTT(0);
     block=FALSE;
     }
  if(message.message==WM_KEYDOWN)
   if(WK_SCANCODE(message.lParam)==1) //ESC
    block=FALSE;
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 else
  WaitMessage();
 if(!IsWindow(hdlg)) block=FALSE;
 }
//EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
return retv;
}

//-------------------------------------------------------------------------------------------------- 
int InputChoiceL(LPSTR label,HWND parent,void*basep,NAT nrit,int itsz=4,int defsel=0)
{
HWND hdlg,hlist,hok,hcancel;
SIZE z;
HDC hdc;
int block,retv=-1;
char *lpstr,lsbuf[1024];
ifn(nrit) return -1;
#ifndef _DEBUG
 if(nrit==1) return 0;	//if just one return imediatly
#endif
hdc=GetDC(parent);
GetTextExtentPoint32(hdc,label,strlen(label),&z);
ReleaseDC(parent,hdc);
if(z.cy*nrit>((mwarea.bottom-mwarea.top)>>1))
 z.cy=(mwarea.bottom-mwarea.top)>>1;
else
 z.cy*=nrit;
if(z.cx>((mwarea.right-mwarea.left)>>1)) 
 z.cx=(mwarea.right-mwarea.left)>>1;
else if(z.cx<200) 
 z.cx=200;
hdlg=CreateWindowEx(0,WC_DEFAULT_PROC,label,WS_POPUP|WS_CAPTION|WS_BORDER,
					0,0,z.cx,z.cy,parent,NULL,appinst,NULL);
AdjustClientRect(hdlg,z.cx+16,z.cy+50);
hlist=CreateWindowEx(WS_EX_CLIENTEDGE,"ListBox","",WS_VISIBLE|WS_CHILD,
                     8,8,z.cx,z.cy+4,hdlg,(HMENU)LIST(0),appinst,NULL);
SendMessage(hlist,LB_RESETCONTENT,0,0);
hok=CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
                  ((z.cx+16)>>2)-30,z.cy+20,60,22,hdlg,(HMENU)BUTT(0),appinst,NULL);
hcancel=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                      ((z.cx+16)>>2)*3-30,z.cy+20,60,22,hdlg,(HMENU)BUTT(1),appinst,NULL);
CenterOnScreen(hdlg);
lpstr=(char*)basep;
for(int i=0;i<nrit;i++)
 {
 SendMessage(hlist,LB_ADDSTRING,0,(LPARAM)lpstr); //add
 lpstr+=itsz;
 }
SendMessage(hlist,LB_SETCURSEL,(WPARAM)defsel%nrit,0); //select
SetFocus(hlist);
ShowWindow(hdlg,SW_SHOW);
EnableWindow(parent,FALSE);
block=TRUE;
MSG message;
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  if(message.message==WM_KEYDOWN)
   {
   if(WK_SCANCODE(message.lParam)==28) //ENTER
    SendMessage(hok,BM_SETSTATE,BST_PUSHED,0);
   else if(WK_SCANCODE(message.lParam)==1) //ESC
    SendMessage(hcancel,BM_SETSTATE,BST_PUSHED,0);
   }
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 else
  WaitMessage();
 if(SendMessage(hok,BM_GETSTATE,0,0)&BST_PUSHED) //Ok
  {
  retv=SendMessage(hlist,LB_GETCURSEL,0,0); //select
  block=FALSE;
  }
 if(SendMessage(hcancel,BM_GETSTATE,0,0)&BST_PUSHED) //Cancel
  {
  retv=-1;
  block=FALSE;
  }
 if(!IsWindow(hdlg)) block=FALSE;
 }
EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
return retv;
}

//-------------------------------------------------------------------------------------------------- 
int ChooseFileInFolder(LPSTR filepath,LPSTR folderpath,LPSTR label,HWND parent)
{
HWND hdlg,hlist,hok,hcancel;
SIZE z;
HDC hdc;
int block,retv=-1,nrit=0;
char *lpstr,lsbuf[1024];
hdc=GetDC(parent);
GetTextExtentPoint32(hdc,label,strlen(label),&z);
ReleaseDC(parent,hdc);
if(z.cx<300) z.cx=300;
hdlg=CreateWindowEx(0,WC_DEFAULT_PROC,label,WS_POPUP|WS_CAPTION|WS_BORDER,
					0,0,z.cx+24,z.cy*14,parent,NULL,appinst,NULL);
hlist=CreateWindowEx(WS_EX_CLIENTEDGE,"ListBox","",WS_VISIBLE|WS_CHILD|WS_VSCROLL|LBS_DISABLENOSCROLL|LBS_STANDARD,
                     8,8,z.cx,z.cy*10,hdlg,(HMENU)LIST(0),appinst,NULL);
HANDLE hff;
WIN32_FIND_DATA wfd;
char lBufDir[PATHSZ];
strcpy(lBufDir,folderpath);
if(lBufDir[strlen(lBufDir)-1]!='\\')
 strcat(lBufDir,"\\*");
else
 strcat(lBufDir,"*");
//SendMessage(hlist,LB_RESETCONTENT,0,0);
hff=FindFirstFile(lBufDir,&wfd);
if(hff==INVALID_HANDLE_VALUE) return 0;
do{
 if((wfd.cFileName[0]!='.')&&!(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) //exclude parent and current because they can cause infinite loops
  {
  SendMessage(hlist,LB_ADDSTRING,0,(LPARAM)wfd.cFileName); //add
  nrit++;
  }
 }while(FindNextFile(hff,&wfd));
FindClose(hff);
hok=CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
                  ((z.cx+16)>>2)-30,z.cy*10+8,60,22,hdlg,(HMENU)BUTT(0),appinst,NULL);
hcancel=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                      ((z.cx+16)>>2)*3-30,z.cy*10+8,60,22,hdlg,(HMENU)BUTT(1),appinst,NULL);
CenterOnScreen(hdlg);
SendMessage(hlist,LB_SETCURSEL,(WPARAM)0,0); //select
SetFocus(hlist);
ShowWindow(hdlg,SW_SHOW);
EnableWindow(parent,FALSE);
block=TRUE;
MSG message;
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  if(message.message==WM_KEYDOWN)
   {
   if(WK_SCANCODE(message.lParam)==28) //ENTER
    SendMessage(hok,BM_SETSTATE,BST_PUSHED,0);
   else if(WK_SCANCODE(message.lParam)==1) //ESC
    SendMessage(hcancel,BM_SETSTATE,BST_PUSHED,0);
   }
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  if(SendMessage(hok,BM_GETSTATE,0,0)&BST_PUSHED) //Ok
   {
   retv=SendMessage(hlist,LB_GETCURSEL,0,0); //select
   SendMessage(hlist,LB_GETTEXT,retv,(LPARAM)filepath);  
   block=FALSE;
   }
  if(SendMessage(hcancel,BM_GETSTATE,0,0)&BST_PUSHED) //Cancel
   {
   retv=-1;
   block=FALSE;
   }
  }
 else
  WaitMessage();
 if(!IsWindow(hdlg)) block=FALSE;
 }
EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
return retv;
}