#ifndef V_REPORT
#define V_REPORT

#include <compat/form.cpp>

HWND hRepWnd=NULL;
FILE *hRepLog=NULL;
int RepNrI=0;
FItem *RepItem=NULL;

//callback for Report ---------------------------------------------------------------------
LRESULT CALLBACK ReportProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
int i;
switch(message)
 {
 case WM_LBUTTONDOWN:
  for(i=0;i<RepNrI;i++)
   RepItem[i].RunM(LOWORD(lparam),HIWORD(lparam),1);
  return 0;
 case WM_RBUTTONDOWN:
  for(i=0;i<RepNrI;i++)
   RepItem[i].RunM(LOWORD(lparam),HIWORD(lparam),2);
  return 0;
 case WM_KEYUP:
  if(wparam==VK_F2) //F2
   {
   if(hRepLog) fprintf(hRepLog,"\n");
   for(i=1;i<RepNrI;i++)
    RepItem[i].Out(hRepLog);
   }
  break;
 case WM_CHAR:
  RepItem[0].RunK((char)wparam);
  return 0;
 case WM_PAINT:
  if(RepNrI>0)
   {
   for(i=0;i<RepNrI;i++)
    RepItem[i].Draw(GetSysColor(COLOR_BTNFACE));
   if(RepItem[0].state) DestroyWindow(hwnd);
   }
  ValidateRect(hwnd,NULL);
  return 0;
 case WM_DESTROY:
  for(i=0;i<RepNrI;i++)
   RepItem[i].Free();
  RepNrI=0;
  if(RepItem) free(RepItem);
  RepItem=NULL;
  hRepWnd=NULL;
 }
return DefWindowProc(hwnd,message,wparam,lparam);
}

//displays some values modal --------------------------------------------------------------
void Report(LPSTR formstr,int width=100,int height=100,LPSTR title=NULL,int align=0x1,FILE *fout=NULL)
{
if(hRepWnd) DestroyWindow(hRepWnd);
RECT cr;
GetWindowRect(hmwnd,&cr);
WindowClass("ReportWClass",ReportProc);
hRepWnd=CreateWindowEx(WS_EX_CLIENTEDGE|WS_EX_TOOLWINDOW,"ReportWClass",title,WS_CAPTION|WS_VISIBLE,
                       cr.right-mww,cr.bottom-mwh,width,height,hmwnd,NULL,appinst,NULL);
if(hRepWnd==NULL) return;
GetClientRect(hRepWnd,&cr);
width=cr.right;
height=cr.bottom;
align&=0xffff;
BorderRect(&cr,-20);
//creting items
LPSTR buffer;
unsigned sto,endo;
int j=30,i,block;
if(RepItem) free(RepItem);
RepNrI=countch('|',formstr)+2;
RepItem=(FItem*)malloc(RepNrI*sizeof(FItem));
RepItem[0].Init("@   Close   /",0,height-50,hRepWnd);
RepItem[0].Move((width-RepItem[0].fr.right-RepItem[0].fr.left)/2);
block=(height-100)/(RepNrI-1);
sto=0;
buffer=(LPSTR)malloc(BUFFSZ);
for(i=1;i<RepNrI;i++)
 {
 endo=findch('|',formstr,i,0);
 if(endo>strlen(formstr)) endo=strlen(formstr);
 CopyMemory(buffer,formstr+sto,endo-sto);
 buffer[endo-sto]='\0';
 RepItem[i].Init(buffer,0,j,hRepWnd);
 RepItem[i].Align(align,&cr);
 j+=block;
 sto=endo+1;
 }
free(buffer);
hRepLog=fout;
}

//updates fields in Report ----------------------------------------------------------------
void printrep(int nri,...)
{
void *param;
va_list vparam;
va_start(vparam,nri);
if(nri>=RepNrI) nri=RepNrI-1;
for(int i=1;i<=nri;i++)
 {
 param=va_arg(vparam,void*);
 RepItem[i].Set(param);
 }
va_end(vparam);
}

//updates a field in a Report -------------------------------------------------------------
void repitem(int ind,void *val,int mod=0)
{
if(ind>0&&ind<RepNrI) 
 {
 RepItem[ind].Set(val,mod);
 RepItem[ind].Draw(GetSysColor(COLOR_BTNFACE));
 }
}

//updates a field in a Report (only for int values) -------------------------------------------------------------
void repitemv(int ind,int val,int mod=0)
{
if(ind>0&&ind<RepNrI) 
 {
 RepItem[ind].Set(&val,mod);
 RepItem[ind].Draw(GetSysColor(COLOR_BTNFACE));
 }
}

#endif