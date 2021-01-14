#ifndef V_UINTERFACE_FORM
#define V_UINTERFACE_FORM

#include <compat/ui_skin.cpp>

/*
    Be carefull when processing messages (esspecialy WM_KEY*), if you modify lparam
  or wparam before passing them to Respond shit can happen
    Don't end meesage cases with returns unless you must
    Place Respond after the switch(message) but before DefWindowProc
    You must process WM_ERASEBACK if you need transparency (added RECT in lparam)
*/

//Form styles
#define IF_0    0

#define IF_GRID 0x00100000 //use grid
#define IF_LSKN 0x00200000 //load skin
#define IF_LWIN 0x00400000 //restore window
#define IF_LFNT 0x00800000 //restore fonts
#define IF_NMOV 0x01000000 //no move (also no resize)
#define IF_NCST 0x02000000 //no customizing
#define IF_NPCM 0x04000000 //no right click context menu
#define IF_NRIB 0x08000000 //no resize inf bar 
#define IF_NINF 0x10000000 //hide info bar
#define IF_NLOD 0x20000000 //no load
#define IF_NSAV 0x40000000 //no save
#define IF_USEB 0x80000000 //use background image for form background
//Form status
#define IS_MSG  0x04000000 //display message
#define IS_WAIT 0x08000000 //is in wait mode
#define IS_FIMG 0x10000000 //should free foreground image
#define IS_BIMG 0x20000000 //should free background image
#define IS_ISKN 0x40000000 //should free skin
#define IS_FOWD 0x80000000 //free itself on window destruction

//#######################################################################################
class UIFORM
{
public:
 char name[32]; //name used to search persistent
 UISKIN *uiskin;
 UITEM **uitems,*uitfoc;
 UINotify Notify; //pointer to callback notification function
 DWORD gstat,cstyl; //common style for all items
 LPSTR info; //info text
 NAT infnc;
//Persistent struct (40B)
 int l,u,r,d; //client frame rect
 int bl,bu,br,bd; //info bar rect
 DWORD gstyl;
 NAT nri;// nri=nrl*nrr (nr of lines, columns)
//!Persistent struct
 int ml,mu,mr,md; //message bar rect (or bounds)
 void*bp; //global base pointer
 Image *fimg,*bimg; //global form image (background)
 HWND wnd;
 HDC dc;
 int dcrefs;
 
 UIFORM();
 ~UIFORM();
 void Init(LPSTR,UISKIN*,HWND,UINotify,void*);
 void Make(DWORD,int,DWORD);
 void Images(Image*,LPSTR,Image*,LPSTR);
 void AddI(int,LPSTR,DWORD,DWORD,int,void*,NAT,LPSTR,LPSTR,LPSTR,int,int,int,int,LPSTR,int,int,void*,void*);
 void NewI(UITEM*);
 void DelI(LPSTR);
 void Link(LPSTR,void*,NAT,void*);
 void Links(NAT,NAT,...);
 void Unlink();
 void AllocLabels();
 void Align(int,int);
 void Sync();
 void ChBP(void*);
 int GetInd(UITEM *); //index of item from pointer
 int GetProp(LPSTR); //index of item from name
 void SetState(DWORD);
 void RstState(DWORD);
 void Free();
 void CapDC(NAT);
 void RelDC();
 void Wait(NAT);
 void Info(LPSTR);
 void Msg(LPSTR,NAT);
 void MouseOver(int,int,int,int,int,int);
 void Click(LPSTR,int);
 void Select(LPSTR,LPSTR,int);
 void Type(LPSTR,LPSTR,NAT,int);
 void Bounds();
 LRESULT Respond(HWND,UINT,WPARAM,LPARAM); //call in the begining of WindowProc
 void Draw();
 void DrawC();
 void Save(IOSFile*);
 void Load(IOSFile*);
 void To(LPSTR);
 void From(LPSTR,LPSTR);
};
//***************************************************************************************************

#include <compat/ui_item.cpp>

//default window proc (calls the associated form) -------------------------------------
LRESULT CALLBACK UIFormProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
UIFORM*uiform;
uiform=(UIFORM*)GetWindowLong(hwnd,GWL_USERDATA);
if(uiform)
 {
 if(uiform->Respond(hwnd,message,wparam,lparam)) return 0;
 }
return DefWindowProc(hwnd,message,wparam,lparam);
}

//UIFORM <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
UIFORM::UIFORM()
{
ZeroMemory(this,sizeof(UIFORM));
}

UIFORM::~UIFORM()
{
Free();
}

void UIFORM::Init(LPSTR lname,UISKIN *luiskin,HWND hpwnd=NULL,UINotify lNotify=NULL,void*lbp=NULL)
{
sc(name,lname);
uiskin=luiskin;
wnd=hpwnd;
Notify=lNotify;
bp=lbp;
SetWindowLong(wnd,GWL_USERDATA,(long)this);
}

void UIFORM::Make(DWORD lgstyle=0,int frame=0,DWORD lcstyle=0)
{
gstyl|=lgstyle;
cstyl=lcstyle;
Free();
GetClientRect(wnd,(RECT*)&l);
BorderRect((RECT*)&l,-frame,-frame);
CopyMemory(&bl,&l,sizeof(RECT));
bu=d;
if(!(gstyl&IF_NINF))
 bu-=uiskin->chh1;
ml=mu=0; //start default positioning
}

void UIFORM::Images(Image *efimg=NULL,LPSTR efin=NULL,Image *ebimg=NULL,LPSTR ebin=NULL)
{
if(gstat&IS_FIMG&&fimg)
 {
 fimg->Free();
 free(fimg);
 }
if(efimg)
 fimg=efimg;
else if(sl(efin))
 {
 fimg=(Image*)calloc(1,sizeof(Image));
 if(gstyl&IF_CDIR) PathNameExt(efin,NULL,&efin,NULL);
 fimg->From(efin,NULL,0,0,IMG_IMB|IMG_GDI);
 fimg->DCTex();
 fimg->Free(IMG_IMB);
 gstat|=IS_FIMG;
 }
else
 fimg=NULL;
if(gstat&IS_BIMG&&bimg)
 {
 bimg->Free();
 free(bimg);
 }
if(ebimg)
 bimg=ebimg;
else if(sl(ebin))
 {
 bimg=(Image*)calloc(1,sizeof(Image));
 if(gstyl&IF_CDIR) PathNameExt(ebin,NULL,&ebin,NULL);
 bimg->From(ebin,NULL,0,0,IMG_IMB|IMG_GDI);
 bimg->DCTex();
 bimg->Free(IMG_IMB);
 gstat|=IS_BIMG;
 }
else
 bimg=NULL;
if(fimg&&bimg)
 fimg->Resize(bimg->lng,bimg->lat);
}

void UIFORM::AddI(int ind,LPSTR iname,DWORD ltype,DWORD lstyl,int r0,void*lnk,
                  NAT maxch,LPSTR bef,LPSTR aft,LPSTR tip,int x,int y,int w,int h,
                  LPSTR defs=NULL,int ix=0,int iy=0,void*min=NULL,void*max=NULL)
{
if(ind<0||ind>=nri)
 {
 ind=nri;
 nri++;
 uitems=(UITEM**)realloc(uitems,sizeof(UITEM*)*nri);
 uitems[ind]=(UITEM*)calloc(1,sizeof(UITEM));
 }
else
 uitems[ind]->Free();
uitems[ind]->Init(this,iname,ltype,lstyl|cstyl,r0);
uitems[ind]->Text(bef,aft,tip);
uitems[ind]->Make(maxch,0);
uitems[ind]->Link(lnk);
uitems[ind]->Opts(min,max);
uitems[ind]->Defs(defs);
if(ltype&IT_C) uitems[ind]->Color(r0);
if(!x) x=(r+l-w)/2;
else x+=l;
if(!y) y=1+ind*uiskin->gch;
else y+=u;
uitems[ind]->Pos(x,y,w,h,ix,iy);
InvalidateRect(wnd,(RECT*)&l,0);
}

void UIFORM::NewI(UITEM*orig=NULL)
{
int ind;
ind=nri;
nri++;
uitems=(UITEM**)realloc(uitems,sizeof(UITEM*)*nri);
uitems[ind]=(UITEM*)calloc(1,sizeof(UITEM));
LPSTR bef,aft,tip,name;
name=(LPSTR)malloc(32);
bef=(LPSTR)malloc(64);
aft=(LPSTR)malloc(64);
tip=(LPSTR)malloc(512);
sprintf(name,"Item%u",ind);
sc(bef,"Before");
sc(aft,"after");
sc(tip,"...");
if(orig)
 {
 uitems[ind]->Init(this,name,orig->type,orig->styl|cstyl,orig->reg0);
 uitems[ind]->Make(orig->bufsz,orig->stat);
 uitems[ind]->Pos(orig->l,orig->d,orig->r-orig->l,orig->d-orig->u,orig->px,orig->py);
 uitems[ind]->Defs("N/A,");
 }
else
 {
 uitems[ind]->Init(this,name,IT_T,IL_0|cstyl,0);
 uitems[ind]->Make(0,0);
 uitems[ind]->Pos(0,0,100,20,0,0);
 uitems[ind]->Defs("Ok,");
 }
uitems[ind]->Text(bef,aft,tip);
uitems[ind]->Link(NULL);
uitems[ind]->stat|=IS_ILAB|IS_ITIP|IS_IPRP;
InvalidateRect(wnd,(RECT*)&l,0);
}

void UIFORM::Align(int hm=0,int vm=0)
{
int i,sp=0,y;
for(i=0;i<nri;i++)
 sp+=uitems[i]->d-uitems[i]->u;
sp=(d-u-sp)/(nri+1);
y=d-sp;
for(i=0;i<nri;i++)
 {
 DeltaRect((RECT*)&uitems[i]->l,(r+l-uitems[i]->r-uitems[i]->l)>>1,y-uitems[i]->d);
 uitems[i]->Resize();
 y-=uitems[i]->d-uitems[i]->u+sp;
 }
}

void UIFORM::AllocLabels()
{
for(int i=0;i<nri;i++)
 {
 if(uitems[i]->prop)
  uitems[i]->prop=(LPSTR)REALLOC(uitems[i]->prop,32);
 else
  uitems[i]->prop=(LPSTR)ALLOC0(32);
 if(uitems[i]->bef)
  uitems[i]->bef=(LPSTR)REALLOC(uitems[i]->bef,64);
 else
  uitems[i]->bef=(LPSTR)ALLOC0(64);
 if(uitems[i]->aft)
  uitems[i]->aft=(LPSTR)REALLOC(uitems[i]->aft,64);
 else
  uitems[i]->aft=(LPSTR)ALLOC0(64);
 if(uitems[i]->tip)
  uitems[i]->tip=(LPSTR)REALLOC(uitems[i]->tip,512);
 else
  uitems[i]->tip=(LPSTR)ALLOC0(512);
 uitems[i]->styl|=IL_NTFY;
 }
}//TODO: make it work for static labels not just dynamic

void UIFORM::Link(LPSTR prop,void*link,NAT linksz=0,void*listbase=NULL)
{
int ind=GetProp(prop);
if(ind<0) return;
if(uitems[ind]->type&IT_L)
 uitems[ind]->reg0=(int)listbase;
uitems[ind]->Link(link,linksz);
}

void UIFORM::Links(NAT sti=0,NAT icnt=0,...)
{
va_list vparam;
va_start(vparam,icnt);
if(!icnt) icnt=nri;
icnt+=sti;
if(icnt>nri) icnt=nri;
for(;sti<icnt;sti++)
 {
 uitems[sti]->Link(va_arg(vparam,void*),0);
 if(uitems[sti]->type&IT_L)
  uitems[sti]->reg0=(int)va_arg(vparam,void*);
 }
va_end(vparam);
}

void UIFORM::Unlink()
{
for(int i=0;i<nri;i++)
 uitems[i]->Link();
if(gstat&IS_CSTM) DrawC();
else Draw();
}

void UIFORM::ChBP(void*nbp=NULL)
{
for(int i=0;i<nri;i++)
 if(uitems[i]->styl&IL_PABP)
  {
  //uitems[i]->lnkb-=bp;
  //uitems[i]->lnkb+=nbp;
  }
bp=nbp;
}

void UIFORM::Sync()
{
for(int i=0;i<nri;i++)
 uitems[i]->Sync();
if(gstat&IS_CSTM) DrawC();
else Draw();
}
 
void UIFORM::Free()
{
uitfoc=NULL;
info=NULL;
if(dcrefs) //release DC
 {
 RestoreDC(dc,-1);
 ReleaseDC(wnd,dc);
 dcrefs=0;
 }
for(int i=0;i<nri;i++)
 {
 uitems[i]->Free();
 free(uitems[i]);
 }
FREE(uitems);
nri=0;
if(gstat&IS_FIMG&&fimg)
 {
 fimg->Free();
 FREE(fimg);
 }
if(gstat&IS_BIMG&&bimg)
 {
 bimg->Free();
 FREE(bimg);
 }
if(gstat&IS_ISKN&&uiskin)
 FREE(uiskin);
gstat=0;
}

int UIFORM::GetInd(UITEM *puitem)
{
if(!puitem) return -1;
for(int i=0;i<nri;i++)
 if(uitems[i]==puitem)
  return i;
return -1;
}

int UIFORM::GetProp(LPSTR prop)
{
if(!prop) return 0;
for(int i=0;i<nri;i++)
 if(strcmp(uitems[i]->prop,prop)==0)
  return i;
return -1;
}

void UIFORM::DelI(LPSTR iname)
{
int ind=GetProp(iname);
if(ind<0) return;
if(uitfoc==uitems[ind]) uitfoc=NULL;
uitems[ind]->Free();
ShiftMemL(uitems+ind+1,nri-ind-1);
nri--;
if(gstat&IS_CSTM) DrawC();
else Draw();
}

void UIFORM::Wait(NAT milisec=0)
{
MSTimer ltm;
MSG message;
NAT lmark;
gstat|=IS_WAIT;
CapDC(0);
ltm.Abs();
do{
 lmark=ltm.Rel();
 if(lmark>=milisec)
  gstat&=~IS_WAIT;
 else if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  switch(message.message)
   {
   case WM_QUIT:
    PostQuitMessage(message.wParam);
   case WM_KEYDOWN:
    if(WK_SCANCODE(message.lParam)==14)
     {
     ltm.Abs();
     milisec=60000;
     FillRect(dc,(RECT*)&bl,(HBRUSH)GetClassLong(wnd,GCL_HBRBACKGROUND));
     break;
     }
   case WM_MOUSEWHEEL:
   case WM_MBUTTONDOWN:
   case WM_LBUTTONDOWN:
   case WM_RBUTTONDOWN:
    gstat&=~IS_WAIT;
    break;
   default:
    DispatchMessage(&message);
   }
  }
 if(!(gstyl&IF_NINF)&&!(lmark&0x1f)&&(milisec>=1000.0f))
  ProcentRect(dc,(RECT*)&bl,(float)lmark*100.0f/milisec,uiskin->fill[11]);
}while(gstat&IS_WAIT);
RelDC();
}

void UIFORM::Info(LPSTR inf)
{
if(inf)
 {
 info=inf;
 infnc=sl(info);
 }
}

void UIFORM::Msg(LPSTR str,NAT delay=0)
{
gstat|=IS_MSG;
Info(str);
CopyMemory(&ml,&l,sizeof(RECT));
BorderRect((RECT*)&ml,-10,-1);
CapDC(2);
DrawText(dc,info,infnc,(RECT*)&ml,DT_WORDBREAK|DT_CENTER|DT_CALCRECT);
CenterRect((RECT*)&ml,(RECT*)&l);
BorderRect((RECT*)&ml,10,1);
if(gstat&IS_CSTM) DrawC();
else Draw();
if(!delay) delay=countch(' ',str);
Wait(delay*500);
gstat&=~IS_MSG;
if(gstat&IS_CSTM) DrawC();
else Draw();
RelDC();
}

void UIFORM::MouseOver(int l,int u,int r,int d,int vv=5,int vt=5)
{
int vh=vv;
MSTimer ltm;
uitsz.cx=(l+r)>>1;
uitsz.cy=(u+d)>>1;
ClientToScreen(wnd,(POINT*)&uitsz);
ltm.Abs();
do{
 if(ltm.Rel()>vt)
  {
  ltm.Abs();
  GetCursorPos((POINT*)&mouse.x);
  mouse.dx=uitsz.cx-mouse.x;
  mouse.dy=uitsz.cy-mouse.y;
  while(!(mouse.dx>>vh)&&vh)
   {vh--;}
  while(!(mouse.dy>>vv)&&vv)
   {vv--;}
  mouse.x+=mouse.dx>>vh;
  mouse.y+=mouse.dy>>vv;
  SetCursorPos(mouse.x,mouse.y);
  }
}while(mouse.x!=uitsz.cx||mouse.y!=uitsz.cy);
}

void UIFORM::Click(LPSTR prop,int mbut=MB_L)
{
if(gstat&IS_CSTM) return;
int ind=GetProp(prop);
if(ind<0) return;
if(uitfoc) uitfoc->Focus(0); //make sure you don't click on some expansion
MouseOver(uitems[ind]->l,uitems[ind]->u,uitems[ind]->r,uitems[ind]->d,5,5);
uitems[ind]->RunM(mbut,uitems[ind]->l,uitems[ind]->u);
Draw();
}

void UIFORM::Select(LPSTR prop,LPSTR def,int mbut=MB_R)
{
if(gstat&IS_CSTM) return;
int ind=GetProp(prop),ito;
if(ind<0) return;
ito=uitems[ind]->InList(def);
if(ito<0) return;
if(!(uitems[ind]->stat&IS_EXP))
 Click(prop,uimb2);
if(ito<uitems[ind]->vof) //scrool up
 {
 MouseOver(uitems[ind]->el1,uitems[ind]->eu1,uitems[ind]->er1,uitems[ind]->ed1,5,5);
 while(ito<uitems[ind]->vof)
  {
  uitems[ind]->RunM(uimb1,uitems[ind]->el1,uitems[ind]->eu1);
  uitems[ind]->Redraw(2);
  Wait(200);
  }
 }
else if(ito>=uitems[ind]->vof+uitems[ind]->nrl) //scrool down
 {
 MouseOver(uitems[ind]->el2,uitems[ind]->eu2,uitems[ind]->er2,uitems[ind]->ed2,5,5);
 while(ito>=uitems[ind]->vof+uitems[ind]->nrl)
  {
  uitems[ind]->RunM(uimb1,uitems[ind]->el2,uitems[ind]->eu2);
  uitems[ind]->Redraw(2);
  Wait(200);
  }
 }
ito-=uitems[ind]->vof;
MouseOver(uitems[ind]->el,uitems[ind]->eu+ito*uiskin->chh1,uitems[ind]->er,uitems[ind]->eu+(ito+1)*uiskin->chh1,10,10);
uitems[ind]->RunM(mbut,uitems[ind]->el,uitems[ind]->eu+ito*uiskin->chh1);
Draw();
}

void UIFORM::Type(LPSTR prop,LPSTR inp,NAT acp=0,int mbut=MB_M)
{
if(gstat&IS_CSTM) return;
int ind=GetProp(prop);
if(ind<0) return;
if(uitems[ind]!=uitfoc)
 Click(prop,mbut);
if(acp>uitfoc->bufnc) acp=uitfoc->bufnc;
while(uitfoc->hof+uitfoc->cur!=acp) //scrool
 {
 if(uitfoc->hof+uitfoc->cur>acp)
  uitfoc->Cursor(uitfoc->hof,uitfoc->cur-1);
 else
  uitfoc->Cursor(uitfoc->hof,uitfoc->cur+1);
 uitfoc->Redraw(1);
 Wait(200);
 }
while(*inp)
 {
 uitfoc->RunK(*inp,0);
 uitfoc->Redraw(1);
 Wait(500);
 inp++;
 }
Draw();
}

void UIFORM::SetState(DWORD stat=0)
{
for(int i=0;i<nri;i++)
 uitems[i]->stat|=stat;
InvalidateRect(wnd,(RECT*)&l,1);
}

void UIFORM::RstState(DWORD stat=0)
{
stat=~stat;
for(int i=0;i<nri;i++)
 uitems[i]->stat&=stat;
InvalidateRect(wnd,(RECT*)&l,1);
}

void UIFORM::CapDC(NAT find=0)
{
if(!dcrefs)
 {
 dc=GetDC(wnd);
 SaveDC(dc);
 SetBkMode(dc,TRANSPARENT);
 }
SelectObject(dc,uiskin->font[find]);
dcrefs++;
}

void UIFORM::RelDC()
{
dcrefs--;
if(!dcrefs)
 {
 RestoreDC(dc,-1);
 ReleaseDC(wnd,dc);
 }
}

void UIFORM::Bounds()
{
GetClientRect(wnd,(RECT*)&ml);
if(gstyl&IF_GRID)
 {
 l=CutM(l,uiskin->gcw);
 r=CutM(r,uiskin->gcw);
 u=CutM(u,uiskin->gch);
 d=CutM(d,uiskin->gch);
 }
KeepInside((RECT*)&l,ml,mu,mr,md,2,2);
for(int i=0;i<nri;i++)
 {
 if(l>uitems[i]->l) l=uitems[i]->l;
 if(u>uitems[i]->u) u=uitems[i]->u;
 if(r<uitems[i]->r) r=uitems[i]->r;
 if(d<uitems[i]->d) d=uitems[i]->d;
 }
}

void UIFORM::Draw()
{
if(gstat&IS_NVIS) return;
CapDC(1);
if(gstyl&IF_USEB&&bimg)
 StretchBlt(dc,l,u,r-l,d-u,bimg->mdc,0,0,bimg->lng,bimg->lat,SRCCOPY);
else 
 SendMessage(wnd,WM_ERASEBKGND,(WPARAM)dc,(LPARAM)&l); //uses lparam to pass the rect
for(int i=0;i<nri;i++)
 uitems[i]->Draw();
if(gstat&IS_MSG) //msg bar
 {
 if(uitfoc) uitfoc->DrawEx();
 SelectObject(dc,uiskin->font[2]);
 SetTextAlign(dc,TA_LEFT|TA_TOP);
 SetTextColor(dc,uiskin->color[18]);
 GradRect(dc,ml,mu,mr,md,uiskin->color[16],uiskin->color[17],1);
 DrawText(dc,info,infnc,(RECT*)&ml,DT_WORDBREAK|DT_CENTER);
 FrameRect(dc,(RECT*)&ml,uiskin->fill[11]);
 }
else if(!(gstyl&IF_NINF)) //info bar
 {
 FillRect(dc,(RECT*)&bl,uiskin->fill[11]);
 if(info)
  {
  SelectObject(dc,uiskin->font[1]);
  SetTextAlign(dc,TA_LEFT|TA_TOP);
  SetTextColor(dc,uiskin->color[11]);
  DrawText(dc,info,infnc,(RECT*)&bl,DT_WORD_ELLIPSIS);
  }
 if(uitfoc) uitfoc->DrawEx();
 }
else if(uitfoc) uitfoc->DrawEx();
RelDC();
}

void UIFORM::DrawC()
{
if(gstat&IS_NVIS) return;
CapDC(1);
if(gstyl&IF_USEB&&bimg)
 StretchBlt(dc,l,u,r-l,d-u,bimg->mdc,0,0,bimg->lng,bimg->lat,SRCCOPY);
else 
 SendMessage(wnd,WM_ERASEBKGND,(WPARAM)dc,(LPARAM)&l); //uses lparam to pass the rect
if(gstyl&IF_GRID)
 DrawGrid(dc,l,u,r,d,uiskin->gcw,uiskin->gch);
DrawEdge(dc,(RECT*)&l,BDR_RAISEDINNER,BF_RECT);
BorderRect((RECT*)&l,-uidragbord+1,-uidragbord+1);
DrawEdge(dc,(RECT*)&l,BDR_SUNKENOUTER,BF_RECT);
BorderRect((RECT*)&l,uidragbord-1,uidragbord-1);
for(int i=0;i<nri;i++)
 uitems[i]->DrawC();
if(gstat&IS_MSG) //msg bar
 {
 SelectObject(dc,uiskin->font[2]);
 SetTextAlign(dc,TA_LEFT|TA_TOP);
 SetTextColor(dc,uiskin->color[18]);
 GradRect(dc,ml,mu,mr,md,uiskin->color[16],uiskin->color[17],1);
 DrawText(dc,info,infnc,(RECT*)&ml,DT_WORDBREAK|DT_CENTER);
 FrameRect(dc,(RECT*)&ml,uiskin->fill[11]);
 }
else if(!(gstyl&IF_NINF)) //info bar
 {
 FrameRect(dc,(RECT*)&bl,uiskin->fill[11]);
 BorderRect((RECT*)&bl,-uidragbord+1,-uidragbord+1);
 FillRect(dc,(RECT*)&bl,uiskin->fill[11]);
 BorderRect((RECT*)&bl,uidragbord-1,uidragbord-1);
 }
RelDC();
}

//returns true if message used (should be called after the message switch)---------------------------------------------------------
LRESULT UIFORM::Respond(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
if(!uiskin||!uitems) return 0;//protect against early calls
int i,j;
//Common message handling
switch(message)
 {
 case WM_SHOWWINDOW:
  if(wparam) gstat&=~IS_NVIS;
  else gstat|=IS_NVIS;
  break;
 case WM_SIZE:
  if(wparam==SIZE_MINIMIZED||wparam==SIZE_MAXHIDE) gstat|=IS_NVIS;
  else gstat&=~IS_NVIS;
  break;
 case WM_ENABLE:
  if(wparam) gstat&=~IS_NACT;
  else gstat|=IS_NACT;
  break;
 case WM_ACTIVATE:
  if(LOWORD(wparam)==WA_INACTIVE) gstat|=IS_NACT;
  else gstat&=~IS_NACT;
  break;
 case WM_CANCELMODE:
 case WM_KILLFOCUS:
  if(uitfoc) uitfoc->Focus(0);
  break;
 }
//Customize mode message handling
if(gstat&IS_CSTM)
 {
 switch(message)
  {
  case WM_LBUTTONDOWN:
   if((wparam&MK_SHIFT)&&(wparam&MK_LBUTTON)) wparam=MB_M;
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN:
   mouse.ev=wparam;
   mouse.ex=LOWORD(lparam);
   mouse.ey=HIWORD(lparam);
   return 1;
  case WM_MOUSEMOVE:
   mouse.x=LOWORD(lparam);
   mouse.y=HIWORD(lparam);
   break;
  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MBUTTONUP:
   mouse.x=LOWORD(lparam);
   mouse.y=HIWORD(lparam);
   mouse.dx=mouse.x-mouse.ex;
   mouse.dy=mouse.y-mouse.ey;
   wparam=mouse.ev;
   mouse.ev=0;
   if(!(gstyl&IF_NINF)) //has info bar
    if(i=PinRb(mouse.ex,mouse.ey,bl,bu,br,bd,uidragbord)) //drag info bar
     if(DeltaRect((RECT*)&bl,mouse.dx,mouse.dy,i,gstyl&IF_NRIB))
      {
      if(gstyl&IF_GRID)
       {
       bl=CutM(bl,uiskin->gcw>>1);
       br=CutM(br,uiskin->gcw>>1);
       bu=CutM(bu,uiskin->gch>>2);
       bd=CutM(bd,uiskin->gch>>2);
       }
      NormRect((RECT*)&bl);
      DrawC();
      return 1;
      }
   lparam=PinRb(mouse.ex,mouse.ey,l,u,r,d,uidragbord);
   if(!lparam) break;
   for(i=0;i<nri;i++)
    {
    if(uitems[i]->RunMC(wparam,mouse.ex,mouse.ey))
     {
     if(Notify) Notify(uitems[i],IN_NPOS);
     return 1;
     }
    }
   if(wparam&uimb1) //drag form
    {
    if(gstyl&IF_NMOV) return 1;
    if(DeltaRect((RECT*)&l,mouse.dx,mouse.dy,lparam,0))
     {
     Bounds();
     DrawC();
     return 1;
     }
    }
   else if(wparam&uimb2) //context menu
    {
    ClientToScreen(wnd,(POINT*)&mouse.x);
    TrackPopupMenuEx(uiskin->hmuipopc,TPM_LEFTBUTTON,mouse.x,mouse.y,wnd,NULL);
    }
   break;
  case WM_PAINT:
   DrawC();
   ValidateRect(wnd,(RECT*)&l);
   break;
  case WM_COMMAND:
   if(HIWORD(wparam)==0)//menu command
    {
    switch(LOWORD(wparam))
     {
     case UI_POPMENUID(0)://done
      gstat&=~IS_CSTM;
      for(i=0;i<nri;i++)
       uitems[i]->Custom(0);
      return 1;
     case UI_POPMENUID(1)://save
      if(!(gstyl&IF_NSAV))
       To(NULL);
      return 1;
     case UI_POPMENUID(2)://load
      if(!(gstyl&IF_NLOD))
       From(name,NULL);
      return 1;
     }
    }
   break;
  case WM_SIZING:
   GetWindowRect(wnd,(RECT*)&ml);
   SubRect((RECT*)lparam,(RECT*)&ml,&i,&j);
   GetClientRect(wnd,(RECT*)&ml);
   if(mr+i<r||md+j<d)
    GetWindowRect(wnd,(RECT*)lparam);
   return TRUE;
  case WM_CLOSE:
   gstat&=~IS_CSTM;
   for(i=0;i<nri;i++)
    uitems[i]->Custom(0);
   return 1;
  }
 return 0;
 }
//normal mode message handling
switch(message)
 {
 case WM_MOUSEMOVE:
  mouse.x=LOWORD(lparam);
  mouse.y=HIWORD(lparam);
  break;
 case WM_LBUTTONDOWN:
  if((wparam&MK_SHIFT)&&(wparam&MK_LBUTTON)) wparam=MB_M;
 case WM_RBUTTONDOWN:
 case WM_MBUTTONDOWN:
  if(gstat&IS_NACT) return 0;
  uitsz.cx=LOWORD(lparam);
  uitsz.cy=HIWORD(lparam);
  if(uitsz.cx<l||uitsz.cx>r||uitsz.cy<u||uitsz.cy>d) break;
  if(uitfoc)
   {
   if(uitfoc->RunM(wparam,uitsz.cx,uitsz.cy))
    return 1;
   }
  for(i=0;i<nri;i++)
   {
   if(uitems[i]->RunM(wparam,uitsz.cx,uitsz.cy))
    return 1;
   }
  if(wparam&uimb2&&!(gstyl&IF_NPCM)) //context menu
   {
   ClientToScreen(wnd,(POINT*)&uitsz);
   TrackPopupMenuEx(uiskin->hmuipop,TPM_LEFTBUTTON,uitsz.cx,uitsz.cy,wnd,NULL);
   }
  return 0;
 case WM_CHAR:
  if(gstat&IS_NACT) return 0;
  if(uitfoc)
   uitfoc->RunK(wparam);
  break;
 case WM_KEYDOWN: //form keys
  if(gstat&IS_NACT) return 0;
  lparam=WK_SCANCODE(lparam);
  switch(lparam)
   {
   case 15: //TAB
    uitems[(GetInd(uitfoc)+1)%nri]->Focus(1);
    return 1;
   }
  if(uitfoc)
   uitfoc->RunK(0,lparam);
  break;
 case WM_MOUSEWHEEL:
  uitsz.cx=LOWORD(lparam);
  uitsz.cy=HIWORD(lparam);
  if(uitsz.cx<l||uitsz.cx>r||uitsz.cy<u||uitsz.cy>d) break;
  if(HIWORD(wparam)&0x8000) //>0
   uitems[(GetInd(uitfoc)+1)%nri]->Focus(1);
  else
   uitems[(GetInd(uitfoc)-1)%nri]->Focus(1);
  return 1;
 case WM_PAINT:
  Draw();
  ValidateRect(wnd,(RECT*)&l);
  break;
 case WM_COMMAND:
  if(HIWORD(wparam)==0)//menu command (=>LOWORD(wparam)==wparam)
   {
   switch(wparam)
    {
    case UI_POPMENUID(0)://cust
     if(gstyl&IF_NCST) return 1;
     gstat|=IS_CSTM;
     for(i=0;i<nri;i++)
      uitems[i]->Custom(1);
     if(Notify) Notify(NULL,IN_CSTM|IN_FORM);
     break;
    case UI_POPMENUID(3)://
     break;
    default:
     if(wparam>=UI_POPMENUID(100)&&wparam<UI_POPMENUID(131)) //skin
      {
      uiskin->skin=1<<(wparam-UI_POPMENUID(100));
      if(Notify) Notify(NULL,IN_SKIN|IN_FORM);
      Draw();//InvalidateRect(wnd,NULL,1);
      }
     else if(wparam>=UI_POPMENUID(200)&&wparam<UI_POPMENUID(231)) //pall
      {
      uiskin->UseDef(wparam-UI_POPMENUID(200));
      if(Notify) Notify(NULL,IN_PALL|IN_FORM);
      Draw();//InvalidateRect(wnd,NULL,1);
      }
    }
   }
  break;
 case WM_SIZING:
  GetWindowRect(wnd,(RECT*)&ml);
  SubRect((RECT*)lparam,(RECT*)&ml,&i,&j);
  GetClientRect(wnd,(RECT*)&ml);
  if(mr+i<r||md+j<d)
   GetWindowRect(wnd,(RECT*)lparam);
  return TRUE;
 case WM_DESTROY:
  if(gstat&IS_FOWD)
   {
   Free();
   free(this);
   return 1;
   }
  break;
 }
return 0;
}

//*
void UIFORM::Save(IOSFile*iof)
{
int i;
if(uitfoc) uitfoc->Focus(0);
iof->write(&l,40);
iof->write(&uiskin->gcw,4);
iof->write(&uiskin->gch,4);
iof->write(&uiskin->skin,4);
LOGFONT lf;
for(i=0;i<UI_FONTS;i++)
 {
 GetObject(uiskin->font[i],sizeof(LOGFONT),&lf);
 iof->write(&lf,sizeof(LOGFONT));
 }
TextBuf ltb;
LPSTR lstr=NULL;
NAT ldw;
ltb.Init('\n',NULL,0);
ltb.Push(bimg?bimg->file:":"); //':' means no image file
ltb.Push(fimg?fimg->file:":");
for(i=0;i<nri;i++)
 {
 ltb.Push(uitems[i]->prop,uitems[i]->prpnc);
 ltb.Push(uitems[i]->bef,uitems[i]->befnc);
 ltb.Push(uitems[i]->aft,uitems[i]->aftnc);
 ltb.Push(uitems[i]->tip,uitems[i]->tipnc);
 uitems[i]->Items(&lstr,&ldw,'|');
 ltb.Push(lstr,ldw);
 }
if(lstr) free(lstr);
iof->write(&ltb.tsz,4);
iof->write(ltb.text,ltb.tsz);
ltb.Free();
for(i=0;i<nri;i++)
 iof->write(&uitems[i]->l,64);
}

void UIFORM::Load(IOSFile*iof)
{
NAT lnri=nri,i,lsz;
TextBuf ltb;
iof->read(&l,40);
iof->read(&uiskin->gcw,4);
iof->read(&uiskin->gch,4);
iof->read(&uiskin->skin,4);
uiskin->UseDef(uiskin->skin);
LOGFONT lf;
for(i=0;i<UI_FONTS;i++)
 {
 if(GETBIT(uiskin->delfont,i))
  DeleteObject(uiskin->font[i]);
 iof->read(&lf,sizeof(LOGFONT));
 uiskin->font[i]=CreateFontIndirect(&lf);
 }
uiskin->delfont=0x7;
uiskin->CalcGrid();
iof->read(&lsz,4);
ltb.Init('\n',NULL,lsz,0);
iof->read(ltb.text,ltb.tsz);
ltb.Sync();
LPSTR lstr,lstr1;
ltb.Seek(0);
lstr=ltb.Pull();
lstr1=ltb.Pull();
Images(NULL,*lstr1==':'?NULL:lstr1,NULL,*lstr==':'?NULL:lstr);
FREE(lstr1);
if(uitfoc) uitfoc->Focus(0);
for(i=0;i<lnri;i++) //free all
 {
 uitems[i]->Free();
 FREE(uitems[i]);
 }
FREE(uitems);
uitems=(UITEM**)ALLOC(sizeof(UITEM*)*nri);
for(i=0;i<nri;i++)
 {
 uitems[i]=(UITEM*)calloc(1,sizeof(UITEM));
 uitems[i]->Link(NULL);
 iof->read(&uitems[i]->l,64);
 uitems[i]->stat&=IM_PERM;
 if(uitems[i]->type&IT_L) uitems[i]->reg0=0; //break link
 uitems[i]->prpnc=ltb.lsz;
 uitems[i]->prop=ltb.Pull();
 uitems[i]->uiform=this;
 uitems[i]->INotify=Notify;
 uitems[i]->befnc=ltb.lsz;
 uitems[i]->bef=ltb.Pull();
 uitems[i]->aftnc=ltb.lsz;
 uitems[i]->aft=ltb.Pull();
 uitems[i]->tipnc=ltb.lsz;
 uitems[i]->tip=ltb.Pull();
 uitems[i]->Make(0,0);
 uitems[i]->Resize();
 lstr=ltb.Pull();
 uitems[i]->Defs(lstr,1);
 uitems[i]->Pick(uitems[i]->sel);
 uitems[i]->stat|=IS_CSTM|IS_ILAB|IS_ITIP|IS_IPRP;
 }
if(lstr) free(lstr);
ltb.Free();
bp=NULL;
if(Notify) Notify(NULL,IN_LINK|IN_FORM);
InvalidateRect(wnd,NULL,0);
}

void UIFORM::To(LPSTR path=NULL)
{
char lbuf[PATHSZ];
IOSFile liof;
smerge(lbuf,4,path?path:":>UI","\\",name,".ui1");
if(liof.open(lbuf,FU_WO)) return; //error
WndFileSave(&liof,wnd);
Save(&liof);
liof.close();
}

void UIFORM::From(LPSTR nname=NULL,LPSTR path=NULL)
{
char lbuf[PATHSZ];
IOSFile liof;
if(nname)
 smerge(lbuf,4,path?path:":>UI","\\",nname,".ui1");
else //"path" is a complete file path
 sc(lbuf,path);
if(liof.open(lbuf,FU_R)) return; //error
if(nname)
 sc(name,nname,32);
else
 {
 PathNameExt(path,NULL,&nname);
 sc(name,nname,32,'.');
 }
WndFileLoad(&liof,wnd);
Load(&liof);
liof.close();
}

//UIFORM >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif