//VERSION 1.0.a
#pragma once

#include <vstr.cpp>
#include <vtime.cpp>

//Universal Data Types
#define UDT_IPOINTER     			0x1	//internal managed pointer
#define UDT_EPOINTER				0x2	//external managed pointer
#define UDT_POINTER					0x3	//UDT_IPOINTER|UDT_EPOINTER

#define UDT_BINARY      	       0x10	//binary[]
#define UDT_STR					   0x20	//asciiz[]
#define UDT_PSTR				   0x21	//*asciiz
#define UDT_CPSTR				   0x22	//const*asciiz
//#define UDT_UNICODE			       0x40	//wchar[]
#define UDT_VSTR			       0x80	//VSTR

#define UDT_NAT				      0x100
#define UDT_INT	                  0x200
#define UDT_REAL				  0x400
//#define UDT_COMPLEX				  0x800
#define UDT_NUMBER 				  0xf00

#define UDT_DATE				 0x1000	//VTIME
#define UDT_TIME				 0x2000 //VTIME
#define UDT_DATETIME			 0x3000	//UDT_DATE|UDT_TIME


//UDT_NUMBER
#define UDT_RADIX			   0xf00000	//(radix-1)
#define UDTR_BINARY			   0x100000	//
#define UDTR_OCTAL  		   0x700000	//
#define UDTR_DECIMAL		   0x900000	//default
#define UDTR_HEXA   		   0xf00000	//
#define UDT_EXP				 0xff000000	//precision for UDT_NUMBER
#define UD_PREC(prec) (((signed)prec)<<24) //(+/-exp)
#define UD_RAD(type) (unsigned)(((type&UDT_RADIX)>>20)+1)	//(radix+1)
#define UD_EXP(type) (((signed)type)>>24) //(+/-exp)
#define UD_ADJUST(type) pow((int)UD_RAD(type),(int)UD_EXP(type)) //(radix+1)^exp

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct UDATA
{
 BITS32 type; //UDT_*
 NAT itsz; //item size
 NAT nrit; //number of items
 union { void*pv; void**pit; char**pstr;
  char*c; BYTE*pb; short*ps; WORD*pw; long*pl; DWORD*pdw; __int64*pli; unsigned __int64*pqw;
  int*pi; unsigned*pu;
  float*pf; double*pd;
  VSTR*pvstr; VTIME*ptm; };

 UDATA() { ZeroMemory(this,sizeof(UDATA)); }
 ~UDATA() { Free(); }
 NAT total(NAT);
 NAT scan(NAT,char*,NAT); //from string
 NAT text(NAT,char*,NAT); //to string
 int choose(LPSTR,HWND,int,int);
 
//............................................................................................ 
 void UDATA::Free()
 {
 if(type&UDT_IPOINTER)
  for(int i=0;i<nrit;i++)
   FREE(pit[i]);
 else if(type&UDT_VSTR)
  for(int i=0;i<nrit;i++)
   pvstr[i].Free();
 FREE(pv);
 nrit=0;
 }
//............................................................................................ 
 void DefItSz(NAT litsz=0)
 {
 if(litsz)
  itsz=litsz;
 else if(type&UDT_POINTER)
  itsz=sizeof(void*);
 else if(type&(UDT_NUMBER))
  itsz=8;
 else if(type&UDT_VSTR)
  itsz=sizeof(VSTR);	//12
 else if(type&UDT_DATETIME)
  itsz=sizeof(VTIME);	//12
 else
  error("UDATA item size is 0");
 if((type&UDT_STR)&&!(type&UDT_POINTER)) itsz++;
 }
//............................................................................................ 
 void UDATA::init(BITS32 ltype=0,NAT lnrit=0,NAT litsz=0)
 {
 Free();
 if(ltype) type=ltype;
 nrit=lnrit;
 DefItSz(litsz);
 if((type&UDT_NUMBER)&&!(type&UDT_RADIX))  type|=UDTR_DECIMAL; //default to decimal numbers
 pv=ALLOC0(itsz*nrit);
 }
//ins at end, del from end............................................................................................ 
 void UDATA::resize(NAT lnrit=0)
 {
 pv=REALLOC(pv,itsz*lnrit);
 if(lnrit>nrit) ZeroMemory(pb+nrit*itsz,(lnrit-nrit)*itsz);
 nrit=lnrit;
 }
//............................................................................................ 
 void UDATA::ins(NAT pos=0,NAT itcnt=1)
 {
 pv=REALLOC(pv,itsz*(nrit+itcnt));
 if(pos<nrit&&itcnt)
  {
  ShiftMemR(pb+itsz*pos,itcnt*itsz,(nrit-pos)*itsz);
  ZeroMemory(pb+itsz*pos,itcnt*itsz);
  }
 nrit+=itcnt;
 }
//............................................................................................ 
 void UDATA::del(NAT pos=0,NAT itcnt=1)
 {
 if(pos>=nrit) return;
 if(pos+itcnt>=nrit) itcnt=nrit-pos;
 if(itcnt)
  {
  ShiftMemL(pb+(pos+itcnt)*itsz,itcnt*itsz,(nrit-pos-itcnt)*itsz);
  }
 nrit-=itcnt;
 pv=REALLOC(pv,itsz*(nrit-itcnt));
 }
//UDT_IPOINTER............................................................................................ 
 void*UDATA::allocit(NAT itind,NAT szB)	//alloc item
 {
 if(type&UDT_IPOINTER)
 if(itind>=nrit)
  {
  itind=nrit;
  resize(nrit+1);
  }
 else
  FREE(pit[itind]);
 if(!szB)
  {
  if(type&UDT_NUMBER)
   szB=8;
  else if(type&UDT_VSTR)
   szB=sizeof(VSTR);	//12
  else if(type&UDT_DATETIME)
   szB=sizeof(VTIME);	//12
  else
   return NULL;
  }
 return pit[itind]=ALLOC(szB);
 }
//UDT_IPOINTER............................................................................................ 
 void UDATA::freeit(NAT itind)	//free item
 {
 if(type&UDT_IPOINTER)
  if(itind<nrit)
   FREE(pit[itind]);
 }
//UDT_STR............................................................................................ 
 char*UDATA::getit(NAT itind) //return pointer
 {
 exitif_(itind>=nrit,NULL,"UDATA::gitem(%u) index too big",itind);
 return c+itind*itsz;
 }
//get universal data ............................................................................................ 
 void UDATA::getu(NAT itind,void*pval)
 {
 exitif(itind>=nrit,"UDATA::get(%u) item index too big",itind);
 CopyMemory(pval,pb+itind*itsz,itsz);
 }
//set universal data ............................................................................................ 
 void UDATA::setu(NAT itind,void*pval)
 {
 exitif(itind>=nrit,"UDATA::set(%u) item index too big",itind);
 CopyMemory(pb+itind*itsz,pval,itsz);
 }
//UDT_STR............................................................................................ 
 NAT UDATA::set(NAT itind,char*lstr,NAT lstrnc=0)	//string copy
 {
 exitif_(!(type&(UDT_STR|UDT_VSTR)),-1,"UDATA type %x!=UDT_STR",type);
 if(itind>=nrit)
  {
  itind=nrit;
  resize(nrit+1);
  }
 if(type&UDT_EPOINTER)
  pstr[itind]=lstr;
 else if(type&UDT_IPOINTER)
  {
  if(!lstrnc) lstrnc=sl(lstr);
  FREE(pstr[itind]);
  pstr[itind]=SALLOC(lstrnc);
  sc(pstr[itind],lstr,lstrnc);
  }
 else if(type&UDT_VSTR)
  pvstr[itind].copy(lstr,lstrnc);
 else
  sc(c+itind*itsz,lstr,lstrnc?MIN(itsz-1,lstrnc):itsz-1);
 return itind;
 }
//UDT_NUMBER............................................................................................ 
 NAT UDATA::set(NAT itind,__int64 val=0) //set value
 {
 exitif_(!(type&UDT_NUMBER),-1,"UDATA type %x!=UDT_NUMBER",type);
 if(itind>=nrit)
  {
  itind=nrit;
  resize(nrit+1);
  }
 if(type&(UDT_INT|UDT_NAT))
  CopyNumber(pb+itind*itsz,&val,itsz,sizeof(val),type&UDT_NAT);
 else if(type&UDT_REAL)
  {
  double ld=val;
  CopyFloat(pb+itind*itsz,&ld,itsz,8);
  }
 else
  return -1;
 return itind;
 }
//UDT_NUMBER............................................................................................ 
 NAT UDATA::set(NAT itind,double val=0) //set real value
 {
 exitif_(!(type&UDT_NUMBER),-1,"UDATA type %x!=UDT_NUMBER",type);
 if(itind>=nrit)
  {
  itind=nrit;
  resize(nrit+1);
  }
 if(type&UDT_REAL)
  CopyFloat(pb+itind*itsz,&val,itsz,8);
 else if(type&(UDT_INT|UDT_NAT))
  {
  __int64 big=val;
  CopyNumber(pb+itind*itsz,&big,itsz,8,type&UDT_NAT);
  }
 else
  return -1;
 return itind;
 }
//UDT_STR............................................................................................ 
 char*UDATA::gstr(NAT itind)	//return string
 {
 ifn(type&(UDT_STR|UDT_VSTR)) return NULL;
 exitif_(itind>=nrit,NULL,"UDATA::gstr(%u) index too big",itind);
 if(type&UDT_POINTER)
  return pstr[itind];
 else if(type&UDT_VSTR)
  return pvstr[itind].pc;
 else
  return c+itind*itsz;
 }
//UDT_NUMBER............................................................................................ 
 __int64 UDATA::gint(NAT itind) //return int
 {
 ifn(type&UDT_NUMBER) return 0;
 exitif_(itind>=nrit,0i64,"UDATA::gint(%u) index too big",itind);
 __int64 big;
 if(type&(UDT_INT|UDT_NAT))
  {
  CopyNumber(&big,pb+itind*itsz,8,itsz,type&UDT_NAT);
  return big;
  }
 else if(type&UDT_REAL)
  {
  double ld;
  CopyFloat(&ld,pb+itind*itsz,8,itsz);
  return (__int64)ld;
  }
 else
  return 0;
 }
//UDT_NUMBER............................................................................................ 
 double UDATA::greal(NAT itind) //return real
 {
 ifn(type&UDT_NUMBER) return 0.;
 exitif_(itind>=nrit,0.,"UDATA::greal(%u) index too big",itind);
 if(type&UDT_REAL)
  {
  double ld;
  CopyFloat(&ld,pb+itind*itsz,8,itsz);
  return ld;
  }
 else if(type&(UDT_INT|UDT_NAT))
  {
  __int64 big;
  CopyNumber(&big,pb+itind*itsz,8,itsz,type&UDT_NAT);
  return big;
  }
 else
  return 0.;
 }
//UDT_DATETIME............................................................................................ 
 VTIME*UDATA::gpdt(NAT itind) //get pointer to VTIME
 {
 ifn(type&UDT_DATETIME) return NULL;
 exitif_(itind>=nrit,NULL,"UDATA::gpdt(%u) index too big",itind);
 return (VTIME*)(pb+itind*itsz);
 }
};

//UDATA <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
 //............................................................................................ 
 NAT UDATA::total(NAT itind) //calc total
 {
 int i,j;
 if(!nrit) return -1;
 if(itind>=nrit)
  {
  itind=nrit;
  resize(nrit+1);
  }
 if(type&(UDT_STR|UDT_VSTR)) //string
  {
  VSTR lvs;
  char*lpc;
  for(i=0;i<nrit;i++)
   {
   if(i==itind) continue;
   lpc=gstr(i);
   if(!lpc) continue;
   for(j=0;j<i;j++) //search for duplicate
    {
    if(scmp(lpc,gstr(j)))
     break;
    } 
   if(j==i) //not found
    {
    if(lvs.nc) lvs+(char*)" ";
    lvs+lpc;
    }
   }
  set(itind,lvs.pc,lvs.nc);
  }
 else if(type&(UDT_INT|UDT_NAT)) //int
  {
  __int64 big=0;
  for(i=0;i<nrit;i++)
   if(i!=itind)
    big+=gint(i);
  set(itind,big);
  }
 else if(type&UDT_REAL)	//real
  {
  double ld=0.;
  for(i=0;i<nrit;i++)
   if(i!=itind)
    ld+=greal(i);
  set(itind,ld);
  }
 else if(type&UDT_DATETIME)
  (gpdt(itind))->Now();
 else
  return -1;
 return itind;
 }
 //............................................................................................ 
 NAT UDATA::scan(NAT itind,char*label,NAT labelnc=0) //from string
 {
 exitif_(!label,-1,"UDATA::scan(%u,NULL)",itind);
 if(itind>=nrit)
  {
  itind=nrit;
  resize(nrit+1);
  }
 if(!labelnc)  labelnc=sl(label);
 if(type&(UDT_STR|UDT_VSTR))  //string
  return set(itind,label,labelnc);
 else if(type&(UDT_INT|UDT_NAT)) //int
  {
  __int64 big;
  big=StoF(label,UD_RAD(type),UD_EXP(type),type&UDT_INT,labelnc);
  CopyNumber(pb+itind*itsz,&big,itsz,8,type&UDT_NAT);
  }
 else if(type&UDT_REAL)	//real
  {
  double ld;
  ld=StoR(label,UD_RAD(type),labelnc);
  CopyFloat(pb+itind*itsz,&ld,itsz,8);
  }
 else if((type&UDT_DATETIME)==UDT_DATETIME)	//both date and time
  (gpdt(itind))->setStr(label,VTM_ASPECT_ISO_DATETIME,labelnc);
 else if(type&UDT_DATE)	//date
  (gpdt(itind))->setStr(label,VTM_ASPECT_ISO_DATE,labelnc);
 else if(type&UDT_TIME)	//time
  (gpdt(itind))->setStr(label,VTM_ASPECT_ISO_TIME,labelnc);
 else
  return -1;
 return itind;
 }
 //............................................................................................ 
 NAT UDATA::text(NAT itind,char*label,NAT labelnc) //to string
 {
 exitif_(!label,-1,"UDATA::text(%u,NULL)",itind);
 exitif_(itind>=nrit,-1,"UDATA::text(%u) index too big",itind);
 exitif_(!labelnc,-1,"UDATA::text(%u,,0) zero buffer size",itind);
 if(type&(UDT_STR|UDT_VSTR))  //string
  sc(label,gstr(itind),labelnc);
 else if(type&(UDT_INT|UDT_NAT)) //int
  {
  __int64 big;
  CopyNumber(&big,pb+itind*itsz,8,itsz,type&UDT_NAT);
  FtoS(big,label,labelnc,UD_RAD(type),UD_EXP(type),type&UDT_INT);
  }
 else if(type&UDT_REAL)	//real
  RtoS(greal(itind),label,labelnc,UD_RAD(type),UD_EXP(type));
 else if((type&UDT_DATETIME)==UDT_DATETIME)	//both date and time
  (gpdt(itind))->getStr(label,VTM_ASPECT_ISO_DATETIME,labelnc);
 else if(type&UDT_DATE)	//date
  (gpdt(itind))->getStr(label,VTM_ASPECT_ISO_DATE,labelnc);
 else if(type&UDT_TIME)	//time
  (gpdt(itind))->getStr(label,VTM_ASPECT_ISO_TIME,labelnc);
 else
  return -1;
 return itind;
 }
//............................................................................................ 
int UDATA::choose(LPSTR label,HWND parent=HWND_DESKTOP,int toff=0,int tsz=0)
{//toff= 0-native text, X-text offset in binary struct
HWND hdlg,hlist,hok,hcancel;
SIZE z;
HDC hdc;
int block,retv=-1;
char lsbuf[1024];
if(nrit<=0) return -1;
if(nrit==1) return 0;	//if just one return imediatly
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
EnableWindow(parent,FALSE);
SetFocus(hlist);
for(int i=0;i<nrit;i++)
 {
 if(!(type&UDT_POINTER)&&toff)
  sc(lsbuf,c+i*itsz+toff,1023);
 else
  text(i,lsbuf,1023);
 SendMessage(hlist,LB_ADDSTRING,0,(LPARAM)lsbuf); //add
 }
SendMessage(hlist,LB_SETCURSEL,(WPARAM)0,0); //select
ShowWindow(hdlg,SW_SHOW);
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
//UDATA >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>