//VERSION 1.0.a
#pragma once

#include <udata.cpp>

class UTABLE
{
public:
 USQLDB*store;
 NAT nrfields;
 UDATA fieldname;
 UDATA*field;
 int*fieldw;
 NAT nrrecs; //max number of records
 //int keyfield;
 
 HWND htwnd;
 int f0,fd,r0; //for scroll
 
 UTABLE() { ZeroMemory(this,sizeof(UTABLE)); fieldname.init(UDT_PSTR,1); }
 ~UTABLE() { Free(); }
 void Free();
 void Bind(USQLDB*); //bind to external storage (caller is responsible for initializing it)

 int AddField(char*,BITS32,NAT,NAT,int); //(propagates to storage)
 int AddRows(NAT,NAT);
 void Totals(NAT);
 void Draw();
 void Edit(int,int);
 void Scroll(int,int);
 void Window(HWND,DWORD,DWORD);
 LRESULT CALLBACK Respond(HWND,UINT,WPARAM,LPARAM);
 //int FieldInd(LPSTR); //get fields index from name
 //int RenField(char*,char*); //(propagates to storage)
 //int Select(int,...); //load fields from storage
 //int GetRows(NAT,NAT); //get a number of rows from offset
 //int GetRows(LPSTR); //get rows by keyfield=expresion
 //int Update(); //inserts UTABLE in storage
 //int Insert(); //inserts UTABLE in storage
 //int Replace(); //replace storage with UTABLE
};

//.............................................................................................
void UTABLE::Free()
{
for(int f=0;f<nrfields;f++)
 field[f].Free();
FREE(field);
nrfields=0;
nrrecs=0;
fieldname.Free();
}

//.............................................................................................
inline void UTABLE::Bind(USQLDB*lstore=NULL)
{
store=lstore;
}

//.............................................................................................
inline int UTABLE::AddField(char*fldname,BITS32 fldtype,NAT nritems=1,NAT fldszB=0,int fldw=0)
{
if(nrrecs<nritems) nrrecs=nritems;
fieldname.set(nrfields,fldname);
field=(UDATA*)REALLOC(field,sizeof(UDATA)*(nrfields+1));
ZeroMemory(&field[nrfields],sizeof(UDATA));
field[nrfields].init(fldtype,nritems,fldszB);
fieldw=(int*)REALLOC(fieldw,sizeof(int)*(nrfields+1));
fieldw[nrfields]=fldw?fldw:100;
nrfields++;
SetScrollRange(htwnd,SB_VERT,0,nrrecs-1,0);
SetScrollRange(htwnd,SB_HORZ,0,nrfields-1,0);
return 1;
}

//insert rows .............................................................................................
inline int UTABLE::AddRows(NAT pos=0,NAT itcnt=1)
{
for(int f=0;f<nrfields;f++)
 field[f].ins(pos,itcnt);
return nrrecs++;
}

//insert rows .............................................................................................
inline void UTABLE::Totals(NAT pos=0)
{
for(int f=0;f<nrfields;f++)
 field[f].total(pos);
}

//.............................................................................................
inline void UTABLE::Draw()
{
HDC hdc;
hdc=GetDC(htwnd);
SIZE tsz;
RCT rc,wr;
int f,r;
char lsbuf[1024];
GetTextExtentPoint32(hdc,"Q",1,&tsz);
tsz.cy+=4;
wr=htwnd;
SetBkMode(hdc,TRANSPARENT);
FillRect(hdc,wr,(HBRUSH)GetStockObject(DKGRAY_BRUSH));
rc.right=-fd;
SelectObject(hdc,(HBRUSH)GetStockObject(LTGRAY_BRUSH));
for(f=f0;f<nrfields;f++)
 {
 rc.left=rc.right;
 rc.right+=fieldw[f];
 rc.top=0;
 rc.bottom=tsz.cy;
 DrawEdge(hdc,rc,EDGE_RAISED,BF_RECT|BF_MIDDLE);
 DrawText(hdc,fieldname.gstr(f),sl(fieldname.gstr(f)),rc,DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_EXPANDTABS);
 for(r=r0;r<nrrecs;r++)
  {
  rc.top=rc.bottom;
  rc.bottom+=tsz.cy;
  if(field[f].text(r,lsbuf,1023)>=0)
   {
   if(field[f].type&UDT_NUMBER)
    DrawText(hdc,lsbuf,MIN(sl(lsbuf),1023),rc,DT_RIGHT|DT_VCENTER|DT_EDITCONTROL|DT_SINGLELINE|DT_EXPANDTABS);
   else if(field[f].type&UDT_DATETIME)
    DrawText(hdc,lsbuf,MIN(sl(lsbuf),1023),rc,DT_CENTER|DT_VCENTER|DT_EDITCONTROL|DT_SINGLELINE|DT_EXPANDTABS);
   else
    DrawText(hdc,lsbuf,MIN(sl(lsbuf),1023),rc,DT_VCENTER|DT_EDITCONTROL|DT_SINGLELINE|DT_EXPANDTABS);
   DrawEdge(hdc,rc,EDGE_SUNKEN,BF_RECT);
   }
  if(rc.bottom>wr.bottom) break;
  }
 if(rc.right>wr.right) break;
 }
ReleaseDC(htwnd,hdc);
}

//.............................................................................................
inline void UTABLE::Edit(int f=0,int r=0)
{
}

#define UT_SCROLL_ABS_ROW      0x1
#define UT_SCROLL_ABS_FIELD    0x2
#define UT_SCROLL_ROW          0x4
#define UT_SCROLL_FIELD        0x8
#define UT_SCROLL_HORIZ        0x10

//.............................................................................................
inline void UTABLE::Scroll(int d=0,int mod=0)
{
if(mod&UT_SCROLL_HORIZ)
 {
 fd+=d;
 goto Lhoriz;
 }
else if(mod&UT_SCROLL_FIELD)
 {
 f0+=d;
 goto Lhoriz;
 }
else if(mod&UT_SCROLL_ABS_FIELD)
 {
 f0=CLAMP(d,0,(int)nrfields);
 fd=0;
 goto Lhoriz;
 }
else if(mod&UT_SCROLL_ROW)
 {
 r0=CLAMP(r0+d,0,(int)nrrecs-1);
 goto Lvert;
 }
else if(mod&UT_SCROLL_ABS_ROW)
 {
 r0=CLAMP(d,0,(int)nrrecs-1);
 goto Lvert;
 }
return;
Lhoriz:
 while(fd>=fieldw[f0]&&(f0<(int)nrfields-1))
  {
  fd-=fieldw[f0];
  f0++;
  }
 while(fd<0&&f0>0)
  {
  f0--;
  fd+=fieldw[f0];
  }
 if(fd<0||fd>fieldw[f0]) fd=0;
 SetScrollPos(htwnd,SB_HORZ,f0,1);
 InvalidateRect(htwnd,NULL,1);
 return;
Lvert:
 SetScrollPos(htwnd,SB_VERT,r0,1);
 InvalidateRect(htwnd,NULL,1);
}

//-------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK UTABLEWndProc(HWND lhwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
UTABLE*putable;
putable=(UTABLE*)GetWindowLong(lhwnd,GWL_USERDATA);
if(putable)
 return putable->Respond(lhwnd,message,wparam,lparam);
else
 return DefWindowProc(lhwnd,message,wparam,lparam);
}

//....................................................................................................................
inline void UTABLE::Window(HWND hparent=NULL,DWORD flags=WS_TILEDWINDOW,DWORD eflags=WS_EX_APPWINDOW)
{
RCT lr;
if(htwnd) return; //already created
if(flags&WS_CHILD)
 lr=hparent;
else
 lr(mwarea.left,mwarea.top,mwarea.right,mwarea.bottom);
WindowClass("WC_UTABLE",UTABLEWndProc,CS_HREDRAW|CS_VREDRAW,0,LoadCursor(NULL,IDC_IBEAM),LoadIcon(NULL,IDI_INFORMATION));
htwnd=CreateWindowEx(eflags,"WC_UTABLE","UTABLE",flags,
                     lr.l,lr.u,lr.W(),lr.H(),hparent,NULL,appinst,NULL);
SetWindowLong(htwnd,GWL_USERDATA,(long)this);
SetScrollRange(htwnd,SB_VERT,0,nrrecs-1,0);
SetScrollRange(htwnd,SB_HORZ,0,nrfields-1,0);
SetForegroundWindow(htwnd);
SetFocus(htwnd);
ShowWindow(htwnd,SW_SHOWNORMAL);
}

//..................................................................................................................................
LRESULT CALLBACK UTABLE::Respond(HWND lhwnd,UINT message,WPARAM wparam,LPARAM lparam)
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
	 Scroll(-1,UT_SCROLL_ROW);
	 break;
	case 80: //down
	 Scroll(1,UT_SCROLL_ROW);
	 break;
	case 75: //left
	 Scroll(-8,UT_SCROLL_HORIZ);
	 break;
	case 77: //right
	 Scroll(8,UT_SCROLL_HORIZ);
	 break;
	case 73: //pg up
	 Scroll(0,UT_SCROLL_ABS_ROW);
	 break;
	case 81: //pg down
	 Scroll(nrrecs-1,UT_SCROLL_ABS_ROW);
	 break;
	case 71: //home
	 Scroll(0,UT_SCROLL_ABS_FIELD);
	 break;
	case 79: //end
	 Scroll(nrfields-1,UT_SCROLL_ABS_FIELD);
	 break;
	}
  break;  
 case WM_HSCROLL:
  switch(LOWORD(wparam))
   {
   case SB_THUMBTRACK:
	InvalidateRect(lhwnd,NULL,1);
   case SB_THUMBPOSITION:
    Scroll(HIWORD(wparam),UT_SCROLL_ABS_FIELD);
    break;
   case SB_LINELEFT: //left arrow
    Scroll(-4,UT_SCROLL_HORIZ);
    break;
   case SB_LINERIGHT: //right arrow
    Scroll(4,UT_SCROLL_HORIZ);
    break;
   case SB_PAGELEFT: //
   	Scroll(-1,UT_SCROLL_FIELD);
    break;
   case SB_PAGERIGHT://
    Scroll(1,UT_SCROLL_FIELD);
    break;
   case SB_LEFT: //home
    Scroll(0,UT_SCROLL_ABS_FIELD);
    break;
   case SB_RIGHT: //end
    Scroll(nrfields-1,UT_SCROLL_ABS_FIELD);
	break;
   }
  break;
 case WM_VSCROLL:
  switch(LOWORD(wparam))
   {
   case SB_THUMBTRACK:
	InvalidateRect(lhwnd,NULL,1);
   case SB_THUMBPOSITION:
    Scroll(HIWORD(wparam),UT_SCROLL_ABS_ROW);
    break;
   case SB_LINEUP: //up arrow
    Scroll(-1,UT_SCROLL_ROW);
	break;
   case SB_LINEDOWN: //down arrow
    Scroll(1,UT_SCROLL_ROW);
	break;
   case SB_PAGEUP: //pg up
    Scroll(-2,UT_SCROLL_ROW);
	break;
   case SB_PAGEDOWN: //pg down
    Scroll(2,UT_SCROLL_ROW);
	break;
   case SB_BOTTOM:
    Scroll(nrrecs-1,UT_SCROLL_ABS_ROW);
	break;
   case SB_TOP:
    Scroll(0,UT_SCROLL_ABS_ROW);
	break;
   }
  break;
 case WM_LBUTTONDOWN:
 case WM_RBUTTONDOWN:
  mouse.ex=LOWORD(lparam);
  mouse.ey=HIWORD(lparam);
  break;
 case WM_MOUSEMOVE:
  mouse.dx=LOWORD(lparam)-mouse.x;
  mouse.x+=mouse.dx;
  mouse.dy=HIWORD(lparam)-mouse.y;
  mouse.y+=mouse.dy;
  //SetFocus(GetWindow(hwnd,GW_OWNER));
  //Hover(mouse.x,mouse.y);
  break;
 case WM_CHAR:
  break;
 case WM_ERASEBKGND:
  return 0;
 case WM_PAINT:
  Draw();
  ValidateRect(htwnd,NULL);
  break;
 case WM_DESTROY:
  htwnd=NULL;
  break;
 }
return DefWindowProc(lhwnd,message,wparam,lparam);
}

