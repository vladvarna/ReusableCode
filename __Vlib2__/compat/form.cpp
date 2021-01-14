#ifndef V_AUTODIALOGS
#define V_AUTODIALOGS

#include <gdi.cpp>

///////////////////////////////////////////////////////////////////////////////////////////
class FItem
{
public:
 LPSTR label,bef,aft,items;
 unsigned labsz,befsz,aftsz;
 char tip;//#-double %-int $-string ~ind menu *-str menu ^-int menu @-button
 int state,focus,ro,x,y;
 RECT fr;
 HWND hpwnd;
 void *link;
 NAT linksz;

 void Init(LPSTR inistr,int i=0,int j=0,HWND parent=HWND_DESKTOP,void *lnk=NULL,NAT lnksz=0);
 void Make(char type='@',unsigned maxch=0,int i=0,int j=0,HWND parent=HWND_DESKTOP,int ronly=0,void *lnk=NULL,NAT lnksz=0);
 void Move(int dx=0,int dy=0);
 void Align(int align=0,RECT *prct=NULL);
 void Draw(COLORREF cbrush=0xffffffff);
 int RunM(int mx,int my,int but=0); //butt=0-none, 1-left, 2-right, 3-mid
 int RunK(char key);
 void Get(void *val,int mod=0);
 void Set(void *val,int mod=0);
 void Out(FILE *fo,int mod=0);
 void Free();
};
///////////////////////////////////////////////////////////////////////////////////////////

#define I32MAX 0xffffffff

//pozitia a ord-uluia ch in str -----------------------------------------------------------
unsigned findch(char ch,LPSTR str,int ord=1,unsigned off=0)
{
if(ord==0) return 0xffffffff;
while(str[off]!=0)
 {
 if(str[off]==ch) 
  {
  ord--;
  if(!ord) return off;
  }
 off++;
 }
return 0xffffffff;
}

//pozitia pe care apare oricare caracter din chs in str -----------------------------------
unsigned findany(LPSTR chs,LPSTR str,int ord=1,unsigned off=0)
{
unsigned ch;
while(str[off]!='\0')
 {
 for(ch=0;chs[ch]!='\0';ch++)
  {
  if(str[off]==chs[ch])
   {
   ord--;
   if(!ord) return off;
   }
  }
 off++;
 }
return 0xffffffff;
}

//reads variable number of values from FItem controls -------------------------------------
int Formular(LPSTR title,int width,int height,LPSTR formstr,...)
{
//suport window
HWND hwnd;
RECT cr;
if(height<110) height=110;
if(width<100) width=100;
GetWindowRect(hmwnd,&cr);
WindowClass("FormWC");
hwnd=CreateWindowEx(WS_EX_CLIENTEDGE|WS_EX_TOOLWINDOW,"FormWC",title,WS_CAPTION|WS_VISIBLE,
                   cr.right-mww,cr.bottom-mwh,width,height,hmwnd,NULL,appinst,NULL);
GetClientRect(hwnd,&cr);
width=cr.right;
height=cr.bottom;
//creating items
void *param;
va_list vparam;
LPSTR buffer;
FItem *items;
unsigned sto,endo,nri,j=30,i,block;
nri=countch('|',formstr)+countch('&',formstr)+2;
items=(FItem*)malloc(nri*sizeof(FItem));
items[0].Init("@ Ready /",0,height-50,hwnd);
items[0].Align(0x1,&cr);
block=(height-100)/(nri-1);
buffer=(LPSTR)malloc(BUFFSZ);
sto=0;
va_start(vparam,formstr);
for(i=1;i<nri;i++)
 {
 endo=findany("|&",formstr,i,0);
 if(endo>strlen(formstr)) endo=strlen(formstr);
 CopyMemory(buffer,formstr+sto,endo-sto);
 buffer[endo-sto]='\0';
 items[i].Init(buffer,0,j,hwnd);
 items[i].Align(0x1,&cr);
 param=va_arg(vparam,void*);
 if(items[i].ro||formstr[endo]=='&') items[i].Set(param);
 j+=block;
 sto=endo+1;
 }
free(buffer);
va_end(vparam);
//run modal
EnableWindow(hmwnd,FALSE);
block=TRUE;
MSG message;
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  TranslateMessage(&message);
  DispatchMessage(&message);
  }
 if(message.hwnd==hwnd)
  {
  switch(message.message)
   {
   case WM_LBUTTONDOWN:
    for(i=0;i<nri;i++)
     items[i].RunM(LOWORD(message.lParam),HIWORD(message.lParam),1);
    break;
   case WM_RBUTTONDOWN:
    for(i=0;i<nri;i++)
     items[i].RunM(LOWORD(message.lParam),HIWORD(message.lParam),2);
    break;
   case WM_CHAR:
    for(i=0;i<nri;i++)
     items[i].RunK((char)message.wParam);
    break;
   case WM_PAINT:
    for(i=0;i<nri;i++)
     items[i].Draw(GetSysColor(COLOR_BTNFACE));
    ValidateRect(hwnd,NULL);
   }
  }
 if(items[0].state||!IsWindow(hwnd))
  block=FALSE;
 }
EnableWindow(hmwnd,TRUE);
//retrieve values for params
va_start(vparam,formstr);
for(i=1;i<nri;i++)
 {
 param=va_arg(vparam,void*);
 items[i].Get(param);
 }
va_end(vparam);
DestroyWindow(hwnd);
for(i=0;i<nri;i++)
 items[i].Free();
free(items);
return nri-1;
}

//FItem<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void FItem::Make(char type,unsigned maxch,int i,int j,HWND parent,int ronly,void *lnk,NAT lnksz)
{
tip=type;
ro=ronly;
hpwnd=parent;
bef=aft=label=items=NULL;
state=focus=befsz=aftsz=0;
labsz=maxch;
if(labsz==0) labsz=BUFFSZ;
HDC hdc;
SIZE sz;
fr.left=x=i;
fr.top=y=j;
hdc=GetDC(hpwnd);
if(tip=='%'||tip=='#'||tip=='^') GetTextExtentPoint32(hdc,"0",1,&sz);
else GetTextExtentPoint32(hdc,"m",1,&sz);
ReleaseDC(hpwnd,hdc);
fr.right=fr.left+sz.cx*labsz;
fr.bottom=fr.top+sz.cy+1;
link=lnk;
linksz=lnksz;
}
 
void FItem::Init(LPSTR inistr,int i,int j,HWND parent,void *lnk,NAT lnksz)
{
int sto,endo;
int strl=strlen(inistr);
hpwnd=parent;
items=NULL;
state=0;
focus=0;
if(inistr==NULL) return;
sto=findany("%#$~^*@",inistr);
if(sto==I32MAX) 
 {
 sto=-1;
 tip=' ';
 }
else tip=inistr[sto];
endo=findany("!/|",inistr);
if(endo==I32MAX) endo=strl;
if(inistr[endo]=='!') ro=1;
else ro=0;
//before
if(sto>0)
 {
 befsz=sto;
 bef=(LPSTR)malloc(befsz+1);
 CopyMemory(bef,inistr,befsz);
 bef[befsz]='\0';
 }
else 
 {
 bef=NULL;
 befsz=0;
 }
//after
if(endo<strl-1)
 {
 aftsz=strl-endo-1;
 aft=(LPSTR)malloc(aftsz+1);
 strcpy(aft,inistr+endo+1);
 }
else 
 {
 aft=NULL;
 aftsz=0;
 }
//label
link=lnk;
linksz=lnksz;
if(tip=='@')
 {
 labsz=endo-sto-1;
 label=(LPSTR)malloc(labsz+1);
 CopyMemory(label,inistr+sto+1,labsz);
 label[labsz]='\0';
 }
else if(tip=='~'||tip=='^'||tip=='*')
 {
 strl=endo-sto-1;
 items=(LPSTR)malloc(strl+1);
 CopyMemory(items,inistr+sto+1,strl);
 items[strl]='\0';
 labsz=findch('_',items,1);
 if(labsz==I32MAX) labsz=atoi(items);
 label=(LPSTR)malloc(labsz+1);
 CopyMemory(label,items,labsz);
 label[labsz]='\0';
 }
else if(tip=='#'||tip=='%'||tip=='$')
 {
 strl=findch('_',inistr,1,sto);
 if(strl<endo)
  {
  CopyMemory(strbuf,inistr+strl+1,endo-strl-1);
  strbuf[endo-strl-1]='\0';
  labsz=atoi(strbuf);
  if(labsz<=0) labsz=PATHSZ;
  }
 else 
  {
  labsz=endo-sto-1;
  strl=endo;
  }
 label=(LPSTR)malloc(labsz+1);
 CopyMemory(label,inistr+sto+1,strl-sto-1);
 label[strl-sto-1]='\0';
 state=strlen(label);
 }
else
 {
 label=NULL;
 labsz=0;
 }
//rects
HDC hdc;
SIZE sz;
x=i;
y=j;
hdc=GetDC(hpwnd);
GetTextExtentPoint32(hdc,bef,befsz,&sz);
fr.left=x+sz.cx;
fr.top=y;
if(tip=='%'||tip=='#'||tip=='^') GetTextExtentPoint32(hdc,"0",1,&sz);
else GetTextExtentPoint32(hdc,"m",1,&sz);
fr.right=fr.left+sz.cx*labsz;
fr.bottom=fr.top+sz.cy+1;
ReleaseDC(hpwnd,hdc);
}

void FItem::Move(int dx,int dy)
{
x+=dx;
y+=dy;
fr.left+=dx;
fr.top+=dy;
fr.right+=dx;
fr.bottom+=dy;
InvalidateRect(hpwnd,NULL,TRUE);
}

//align |31...|B|T|M|15...|C bef|L bef|R|L|CR|CL|C(lab)
void FItem::Align(int align,RECT *prct)
{//delta=dest-src
RECT cr;
if(prct) CopyMemory(&cr,prct,sizeof(RECT));
else GetClientRect(hpwnd,&cr);
cr.right-=cr.left;//width
cr.bottom-=cr.top;//height
//horizontal alignament
if(align&0x1)       //|___labl___|
 Move(((cr.right-fr.left-fr.right)>>1)+cr.left);
else if(align&0x2)  //|_____lab__|
 Move((cr.right>>1)-fr.left+cr.left);
else if(align&0x4)  //|__lab_____|
 Move((cr.right>>1)-fr.right+cr.left);
else if(align&0x8)  //|lab_______|
 Move(cr.left-fr.left);
else if(align&0x10) //|_______lab|
 Move(cr.right-fr.right+cr.left);
else if(align&0x20) //|bef_______|
 Move(cr.left-x);
else if(align&0x40) //|_____bef__|
 Move((cr.right>>1)-x+cr.left);
else if(align&0x80) //|__beflab__|
 Move(((cr.right-x-fr.right)>>1)+cr.left);
else if(align&0x100) //bef<-lab
 Move(x-fr.left);
//vertical alignament
if(align&0x10000)       //middle
 Move(0,((cr.bottom-fr.bottom-fr.top)>>1)+cr.top);
else if(align&0x20000)  //top
 Move(0,cr.top-y);
else if(align&0x40000)  //bottom
 Move(0,cr.bottom-fr.bottom+cr.top);
}
 
void FItem::Free()
{
if(bef) free(bef);
if(aft) free(aft);
if(label) free(label);
if(items) free(items);
bef=aft=label=items=NULL;
}

int FItem::RunK(char key)
{
if(ro||(!focus)) return 0;
unsigned sto,endo;
if(tip=='@') //Enter/Space toggles
 {
 if(key==32||key==13) 
  state=1-state;
 else  return 0;
 }
else if(tip=='~'||tip=='^'||tip=='*')
 {
 if(key==13) //<ENTER>=next
  {
  state++;
  sto=findch('_',items,state,0);
  if(sto>strlen(items)) sto=state=0;
  else sto++;
  endo=findch('_',items,state+1,0);
  if(endo>strlen(items)) endo=strlen(items);
  endo-=sto;
  if(endo>labsz) endo=labsz;
  CopyMemory(label,items+sto,endo);
  label[endo]='\0';
  }
 else if(key==8) //<BackSpace>=prev
  {
  state--;
  if(state<0) state=countch('_',items);
  sto=findch('_',items,state,0);
  if(sto==I32MAX) sto=0;
  if(state>0) sto++;
  endo=findch('_',items,state+1,0);
  if(endo>strlen(items)) endo=strlen(items);
  endo-=sto;
  if(endo>labsz) endo=labsz;
  CopyMemory(label,items+sto,endo);
  label[endo]='\0';
  }
 else return 0;
 }
else if(tip=='#'||tip=='%'||tip=='$')
 {
 if(key==27)//escape clears
  {
  state=0;
  label[0]='\0';
  }
 else if(key==8)
  {
  if(state==0) return 0;
  state--;
  label[state]='\0';
  }
 else if(state<(int)labsz)
  {
  if(key<'0'||key>'9')
   {
   if(key=='-')
    {
    if(state>0) return 0;
    }
   else
    { 
    if(tip=='%') return 0;
    if(tip=='#'&&key!='.') return 0;
    }
   }
  label[state]=key;
  state++;
  label[state]='\0';
  }
 else return 0;
 }
if(link)
 {
 if(focus) Get(link,linksz);
 else Set(link,linksz);
 }
InvalidateRect(hpwnd,&fr,FALSE);
return 1;
}

int FItem::RunM(int mx,int my,int but)
{
unsigned sto,endo;
if(ro) return 0;
if(mx<fr.left||mx>fr.right||my<fr.top||my>fr.bottom)
 {
 if(focus) InvalidateRect(hpwnd,&fr,FALSE);
 focus=0;
 return 0;
 }
else focus=1;
if(tip=='@') //press
 state=1-state;
else if(tip=='~'||tip=='^'||tip=='*')
 {
 if(but==1) //next 
  {
  state++;
  sto=findch('_',items,state,0);
  if(sto>strlen(items)) sto=state=0;
  else sto++;
  endo=findch('_',items,state+1,0);
  if(endo>strlen(items)) endo=strlen(items);
  endo-=sto;
  if(endo>labsz) endo=labsz;
  CopyMemory(label,items+sto,endo);
  label[endo]='\0';
  }
 else if(but==2) //prev
  {
  state--;
  if(state<0) state=countch('_',items);
  sto=findch('_',items,state,0);
  if(sto==I32MAX) sto=0;
  if(state>0) sto++;
  endo=findch('_',items,state+1,0);
  if(endo>strlen(items)) endo=strlen(items);
  endo-=sto;
  if(endo>labsz) endo=labsz;
  CopyMemory(label,items+sto,endo);
  label[endo]='\0';
  }
 }
else if(tip=='$'||tip=='%'||tip=='#')
 {
 if(but==2)
  {
  state=0;
  label[0]='\0';
  }
 }
if(link)
 {
 if(focus) Get(link,linksz);
 else Set(link,linksz);
 }
InvalidateRect(hpwnd,&fr,FALSE);
return 1;
}

void FItem::Get(void *val,int mod) //out
{
if(tip=='@'||tip=='~')
 *(int*)val=state;
else if(tip=='$'||tip=='*')
 strcpy((LPSTR)val,label);
else if(tip=='%'||tip=='^')
 {
 if(mod==1) *(unsigned char*)val=(unsigned char)atoi(label);
 else if(mod==2) *(short*)val=(short)atoi(label);
 else *(int*)val=(int)atoi(label);
 }
else if(tip=='#')
 {
 if(mod) *(float*)val=(float)atof(label);
 else *(double*)val=(double)atof(label);
 }
else ZeroMemory(val,1);
}

void FItem::Set(void *val,int mod) //in
{
if(tip=='@')
 state=*(int*)val;
else if(tip=='$')
 {
 if(strlen((LPSTR)val)<=labsz)
  strcpy(label,(LPSTR)val);
 else
  CopyMemory(label,val,labsz);
 state=strlen(label);
 }
else if(tip=='%')
 {
 state=(mod&0xff00)>>8;
 if(!state) state=10;
 mod&=0xff;
 if(mod==1) mod=*(unsigned char*)val;
 else if(mod&0xff==2) mod=*(short*)val;
 else mod=*(int*)val;
 itoa(mod,strbuf,state);
 CopyMemory(label,strbuf,labsz);
 state=strlen(label);
 }
else if(tip=='#')
 {
 if(mod) sprintf(strbuf,"%.3f",*(float*)val);
 else sprintf(strbuf,"%f",*(double*)val);
 CopyMemory(label,strbuf,labsz);
 state=strlen(label);
 }
else if(tip=='*'||tip=='^'||tip=='~')
 {
 unsigned sto,endo;
 if(mod)
  {
  if(items) free(items);
  items=(LPSTR)val;
  state=0;
  }
 else 
  state=*(int*)val;
 sto=findch('_',items,state,0);
 if(sto>strlen(items)) sto=state=0;
 else sto++;
 endo=findch('_',items,state+1,0);
 if(endo>strlen(items)) endo=strlen(items);
 endo-=sto;
 if(endo>labsz) endo=labsz;
 CopyMemory(label,items+sto,endo);
 label[endo]='\0';
 }
//else ZeroMemory(label,labsz);
label[labsz]='\0';
}

void FItem::Draw(COLORREF cbrush)
{
HDC hdc;
HPEN hpen;
HBRUSH hbrush;
if(link)
 {
 if(focus) Get(link,linksz);
 else Set(link,linksz);
 }
hdc=GetDC(hpwnd);
SetBkMode(hdc,TRANSPARENT);
SetTextColor(hdc,GetSysColor(COLOR_WINDOWTEXT));
TextOut(hdc,x,y,bef,befsz);
TextOut(hdc,fr.right,y,aft,aftsz);
//pen
if(ro)
 hpen=CreatePen(PS_SOLID,0,GetSysColor(COLOR_GRAYTEXT));
else if(focus)
 hpen=CreatePen(PS_SOLID,0,GetSysColor(COLOR_ACTIVECAPTION));
else
 hpen=CreatePen(PS_SOLID,0,GetSysColor(COLOR_INACTIVECAPTION));
hpen=(HPEN)SelectObject(hdc,hpen);
//brush
if(tip=='@')
 {
 if(state) 
  {
  SetTextColor(hdc,GetSysColor(COLOR_CAPTIONTEXT));
  cbrush=GetSysColor(COLOR_ACTIVECAPTION);
  }
 else 
  SetTextColor(hdc,GetSysColor(COLOR_BTNTEXT));
 }
else if(tip=='~'||tip=='^'||tip=='*')
 {
 if(focus) 
  {
  SetTextColor(hdc,GetSysColor(COLOR_HIGHLIGHTTEXT));
  cbrush=GetSysColor(COLOR_HIGHLIGHT);
  }
 else
  SetTextColor(hdc,GetSysColor(COLOR_MENUTEXT));
 }
else if(tip=='#'||tip=='%'||tip=='$')
 {
 SetTextColor(hdc,GetSysColor(COLOR_WINDOWTEXT));
 if(focus) cbrush=GetSysColor(COLOR_WINDOW);
 }
if(cbrush==0xffffffff) hbrush=(HBRUSH)GetStockObject(HOLLOW_BRUSH);
else hbrush=CreateSolidBrush(cbrush);
hbrush=(HBRUSH)SelectObject(hdc,hbrush);
//drawing
if(tip=='@')
 Ellipse(hdc,fr.left,fr.top,fr.right,fr.bottom);
else if(tip=='~'||tip=='^'||tip=='*')
 RoundRect(hdc,fr.left,fr.top,fr.right,fr.bottom,9,9);
else if(tip=='#'||tip=='%'||tip=='$')
 Rectangle(hdc,fr.left,fr.top,fr.right,fr.bottom);
DrawText(hdc,label,-1,&fr,DT_CENTER|DT_VCENTER);
hbrush=(HBRUSH)SelectObject(hdc,hbrush);
DeleteObject(hbrush);
hpen=(HPEN)SelectObject(hdc,hpen);
DeleteObject(hpen);
ReleaseDC(hpwnd,hdc);
}

void FItem::Out(FILE *fo,int mod)
{
if(!fo) return;
fprintf(fo,"\n");
if(bef) fprintf(fo,"%s",bef);
if(tip=='~'||tip=='^'||tip=='*') //meniuri
 {
 int l,x=0;
 if(state==0) fputs("\n * ",fo);
 else fputs("\n - ",fo);
 for(l=0;l<strlen(items);l++)
  {
  if(items[l]=='_')
   {
   x++;
   if(state==x) fputs("\n * ",fo);
   else fputs("\n - ",fo);
   }
  else fputc(items[l],fo);
  }
 }
else if(tip=='$'||tip=='%'||tip=='#')
 fprintf(fo," [%s] ",label);
else if(tip=='@')
 fprintf(fo," %c%s%c ",state?'>':'(',label,state?'<':')');
if(aft) fprintf(fo,"%s",aft);
}
//FItem>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif