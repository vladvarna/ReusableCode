//VERSION 1.0.a
#pragma once

#define WCAF_LABELNC		0x1	//based on label
#define WCAF_FIT			0x2	//bigest item sets size
#define WCAF_CONST			0x4 //static size used at creation

struct WCARRAY_FIELD
{
 DWORD flags; //WCAF_*
 int headind; //header index
 char*defwc; //default window class
 DWORD datatype;
 void*pdata;
 NAT datasz; //in B
};

class WCARRAY
{
public:
 HWND hbasewnd;
 HWND hheader; //header control
 NAT nrf,nrr; //number of fields, rows
 int cw,ch; //base cell width/height
 WCARRAY_FIELD* field;
 HWND **hchild;

 WCARRAY() { ZeroMemory(this,sizeof(WCARRAY)); }
 void Create(int,int,int,int,HWND);
 void Free();
 int AddField(LPSTR,DWORD,int);
 //int AddDBField();
 void FreeFields();
 LRESULT CALLBACK Respond(HWND,UINT,WPARAM,LPARAM);
 ~WCARRAY() { Free(); }
};

//////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WCARRAY_WndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{                         
WCARRAY*pwcarray;
pwcarray=(WCARRAY*)GetWindowLong(hwnd,GWL_USERDATA);
if(pwcarray)
 return pwcarray->Respond(hwnd,message,wparam,lparam);
else 
 return DefWindowProc(hwnd,message,wparam,lparam);
}

//..................................................................................................................................
LRESULT CALLBACK WCARRAY::Respond(HWND lhwnd,UINT message,WPARAM wparam,LPARAM lparam)
{                         
//HDC hdc;
switch(message)
 {
 case WM_KEYDOWN:
  switch(WK_SCANCODE(lparam))
	{
	case 72: //up
	 //Scroll(-1);
	 break;
	case 80: //down
	 //Scroll(1);
	 break;
	case 75: //left
	 //Scroll(0,-1);
	 break;
	case 77: //right
	 //Scroll(0,1);
	 break;
	case 73: //pg up
	 //Scroll(-vislin+1);
	 break;
	case 81: //pg down
	 //Scroll(dlog.vislin-1);
	 break;
	case 71: //home
	 //Scroll(0,-dlog.indent);
	 break;
	case 79: //end
	 //Scroll(0,dlog.viscol-1);
	 break;
	}
  break;  
 //case WM_VSCROLL:
 // switch(LOWORD(wparam))
 //  {
 //  case SB_THUMBTRACK:
	//InvalidateRect(hwnd,NULL,1);
 //  case SB_THUMBPOSITION:
 //   Scroll(-first+HIWORD(wparam));
 //   break;
 //  case SB_LINEUP:
 //   Scroll(-1);
 //   break;
 //  case SB_LINEDOWN:
 //   Scroll(1);
 //   break;
 //  case SB_PAGEUP: //pg up
 //   Scroll(-vislin+1);
 //   break;
 //  case SB_PAGEDOWN: //pg down
 //   Scroll(dlog.vislin-1);
 //   break;
 //  case SB_BOTTOM:
 //   Scroll(-first+nre-1);
 //   break;
 //  case SB_TOP:
 //   Scroll(-first);
 //   break;
 //  }
 // break;
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
  break;
 case WM_CHAR:
  break;
 //case WM_ERASEBKGND:
 // return 0;
 case WM_PAINT:
  //Show();
  break;
 case WM_DESTROY:
  break;
 }
return DefWindowProc(lhwnd,message,wparam,lparam);
}

//.............................................................................................
void WCARRAY::Free()
{
FreeFields();
}

//.............................................................................................
void WCARRAY::Create(int x,int y,int w,int h,HWND hparent=NULL)
{
WindowClass("WCARRAY",WCARRAY_WndProc,0,NULL,NULL,NULL,NULL);
if(hparent)
 {
 hbasewnd=CreateWindowEx(WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT,"WCARRAY","",WS_CHILD,
			x,y,w,h,hparent,(HMENU)0x7fffffff,appinst,NULL);
 }
else
 {
 hbasewnd=CreateWindowEx(WS_EX_CONTROLPARENT,"WCARRAY","",WS_POPUPWINDOW|WS_CAPTION|WS_MINIMIZEBOX|WS_SIZEBOX,
			x,y,w,h,NULL,(HMENU)NULL,appinst,NULL);
 }
SetWindowLong(hbasewnd,GWL_USERDATA,(long)this); 
cw=ch=20;
hheader=CreateWindowEx(0,WC_HEADER,"",WS_CHILD|WS_VISIBLE|HDS_BUTTONS|HDS_DRAGDROP,0,0,w,16,
                       hbasewnd,(HMENU)0x7fffffff,appinst,NULL);
isNULL(hheader,"!hheader");
HDLAYOUT hdl;
RECT pr;
WINDOWPOS wp; 
GetClientRect(hbasewnd,&pr);
hdl.prc=&pr;
hdl.pwpos=&wp;
SendMessage(hheader,HDM_LAYOUT,0,(LPARAM)&hdl); 
SetWindowPos(hheader,wp.hwndInsertAfter,wp.x,wp.y,wp.cx,wp.cy,wp.flags|SWP_SHOWWINDOW); 

ShowWindow(hbasewnd,SW_SHOW);
}

//.............................................................................................
void WCARRAY::FreeFields()
{
for(int i=0;i<nrf;i++)
 SendMessage(hheader,HDM_DELETEITEM,i,0); 
FREE(field);
nrf=0;
}

//.............................................................................................
int WCARRAY::AddField(LPSTR fldname,DWORD fldf=0,int fldw=0)
{
field=(WCARRAY_FIELD*)REALLOC(field,(nrf+1)*sizeof(WCARRAY_FIELD));
field[nrf].flags=fldf;
HDITEM hdi; 
hdi.mask=HDI_TEXT|HDI_FORMAT|HDI_WIDTH; 
hdi.pszText=fldname; 
hdi.cxy=fldw?fldw:sl(fldname)*cw;
hdi.cchTextMax=sl(hdi.pszText); 
hdi.fmt=HDF_LEFT|HDF_STRING; 
field[nrf].headind=SendMessage(hheader,HDM_INSERTITEM,(WPARAM)0,(LPARAM)&hdi); 
nrf++;

InvalidateRect(hbasewnd,NULL,0);
return nrf-1;
}
