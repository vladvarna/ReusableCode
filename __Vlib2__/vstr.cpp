#pragma once

#include <str.cpp>
#include <io.cpp>

#define VSTR_CONST			0x8000	//or external (no free)
#define VSTR_SELECTED		0x0100

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#pragma pack(push,default_pack)
#pragma pack(1) //byte packing
class VSTR
{
public:
 union
  {
  char*pc;
  unsigned char*pb;
  void*pv;
  };
 DWORD nc;
 WORD stat; //state
 WORD f; //flags 
#pragma pack(pop,default_pack)
 VSTR() { ZeroMemory(this,sizeof(VSTR)); }   //zero
 ~VSTR() { Free(); } //copy size
 void Free() { if(!(f&VSTR_CONST)) FREE(pc); nc=0; f=0; }
 
 //....................................................................................................
 operator char*() //char*,LPSTR
  {
  return pc;
  }
 //....................................................................................................
 operator NAT() //NAT
  {
  return nc;
  }
 //....................................................................................................
 operator int() //int
  {
  return nc;
  }
 //....................................................................................................
 char* operator =(char*lstr) //copy
  {
  if(!lstr) return NULL;
  nc=sl(lstr);
  if(!(f&VSTR_CONST)) FREE(pc);
  pc=SALLOC(nc);
  f&=~VSTR_CONST;
  if(!pv) return NULL;
  CopyMem(pv,lstr,nc);
  pc[nc]=0;
  return pc;
  }
 //....................................................................................................
 NAT operator =(NAT lsize) //=NAT
  {
  nc=lsize;
  return nc;
  }
 //....................................................................................................
 char* operator +(char*lstr) //copy
  {
  NAT l;
  if((!lstr)||(f&VSTR_CONST)) return pc;
  l=sl(lstr);
  f&=~VSTR_CONST;
  pv=REALLOC(pv,nc+l+1);
  if(!pv) return NULL;
  CopyMem(pc+nc,lstr,l);
  nc+=l;
  pc[nc]=0;
  return pc;
  }
 //....................................................................................................
 char* operator =(HWND hwnd) //=HWND
  {
  if(!(f&VSTR_CONST)) FREE(pc);
  nc=GetWindowTextLength(hwnd);
  pc=SALLOC(nc);
  if(!pv) return NULL;
  f&=~VSTR_CONST;
  GetWindowText(hwnd,pc,nc+1);
  return pc;
  }
 //....................................................................................................
 char* operator +(HWND hwnd) //copy
  {
  NAT l;
  if(f&VSTR_CONST) return pc;
  l=GetWindowTextLength(hwnd);
  f&=~VSTR_CONST;
  pv=REALLOC(pv,nc+l+1);
  if(!pv) return NULL;
  GetWindowText(hwnd,pc+nc,l+1);
  nc+=l;
  return pc;
  }
 //....................................................................................................
 void operator ()(HWND hwnd) //=HWND
  {
  SetWindowText(hwnd,pc);
  }
 //....................................................................................................
 char* alloc(NAT lsize=0) //copy size
  {
  nc=lsize;
  if(!(f&VSTR_CONST)) FREE(pc);
  pc=SALLOC(nc);
  f&=~VSTR_CONST;
  if(!pv) return NULL;
  *pc=0;
  return pc;
  }
 //....................................................................................................
 char* copy(char*lstr,NAT lsize=0) //copy size
  {
  if(!lstr) return NULL;
  nc=lsize?lsize:sl(lstr);
  if(!(f&VSTR_CONST)) FREE(pc);
  pc=SALLOC(nc);
  f&=~VSTR_CONST;
  if(!pv) return NULL;
  CopyMem(pv,lstr,nc);
  pc[nc]=0;
  return pc;
  }
 //....................................................................................................
 char* bind(char*lstr,NAT lsize=0) //bind
  {
  if(!lstr) return NULL;
  if(!(f&VSTR_CONST)) FREE(pc);
  f|=VSTR_CONST;
  pc=lstr;
  nc=lsize?lsize:sl(pc);
  return pc;
  }
 //concatenate....................................................................................................
 char* ccat(char*lstr,NAT lsize=0) //copy
  {
  if((!lstr)||(!lsize)||(f&VSTR_CONST)) return pc;
  f&=~VSTR_CONST;
  pv=REALLOC(pv,nc+lsize+1);
  if(!pv) return NULL;
  CopyMem(pc+nc,lstr,lsize);
  nc+=lsize;
  pc[nc]=0;
  return pc;
  }
 //....................................................................................................
 void ins(int pos,char*lstr,NAT lsize=0) //insert lstr[lsize] at pos
  {
  if(f&VSTR_CONST) return;  //can't insert in const char
  if(pos>nc) pos=nc;
  if(!lsize) lsize=sl(lstr);
  pv=REALLOC(pv,nc+lsize+1);
  if(!pv) return;
  ShiftMemR(pc+pos,lsize,nc-pos);
  CopyMem(pc+pos,lstr,lsize);
  nc+=lsize;
  pc[nc]=0;
  }
 //sprintf....................................................................................................
 void spf(char *formstr,...)
  {
  char buffer[1024];
  NAT l;
  if(!formstr) return;
  va_list vparam;
  va_start(vparam,formstr);
  l=vsprintf(buffer,formstr,vparam);
  va_end(vparam);
  copy(buffer,l);
  }
 //append sprintf ....................................................................................................
 void apf(char *formstr,...)
  {
  char buffer[1024];
  NAT l;
  if(!formstr) return;
  va_list vparam;
  va_start(vparam,formstr);
  l=vsprintf(buffer,formstr,vparam);
  va_end(vparam);
  ccat(buffer,l);
  }
 //....................................................................................................
 void show(char*title=NULL)
  {
  MessageBox(hmwnd,pc,title?title:"VSTR",MB_OK);
  } 
};
