#ifndef V_UINTERFACE_SKIN
#define V_UINTERFACE_SKIN

#include <compat/ui_bas.cpp>

#define ID_WIN  0x0001 //windows like
#define ID_STD  0x0002 //standard
#define IC_VLAD 0x0003 //VLAD color scheme
#define ID_CLS  0x0004 //classic (form like)
#define IC_FULL 0x0005 //color ful
#define IC_GREY 0x0006 //all grey
#define IC_HASH 0x0007 //hasurat (hatch)
#define ID_GRD  0x0008 //gradient

#define UI_PENS   12 //max 32
#define UI_FILLS  14 //max 32
#define UI_FONTS  3  //max 32
#define UI_COLORS 20 //no max yet
#define UI_PMENUSKINS "Windows|Standard"
#define UI_PMENUCOLOR "Windows|_|Vlad"
#define UI_POPMENU "|Customize|Colors>|Skin>"
#define UI_POPMENUC "|Done|Save|Load"
#define UI_POPMENUID(dp0) (4095+dp0)

//#######################################################################################
class UISKIN
{
public:
 DWORD skin;//color palette
 HPEN pen[UI_PENS];
 HBRUSH fill[UI_FILLS];
 HFONT font[UI_FONTS];
 COLORREF color[UI_COLORS];
 HMENU hmuipop,hmuipopc;
 DWORD delpen,delfill,delfont; //delete flags
 int chw,cha,chh,chh1; //character max width,height and average width
 int gcw,gch; //grid cell sizes
 
 UISKIN();
 void Make(DWORD,int,int);
 void UseDef(DWORD);
 void Menus(LPSTR,LPSTR);
 void Fonts();
 void UseFont(NAT,LPSTR,int,int,DWORD,DWORD);
 void UseFill(NAT,COLORREF);
 void Free(int);
 void CalcGrid();
 ~UISKIN();
};
//***************************************************************************************************
//UISKIN <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
UISKIN::UISKIN()
{
ZeroMemory(this,sizeof(UISKIN));
}

UISKIN::~UISKIN()
{
Free(0xf);
}

void UISKIN::Menus(LPSTR popmenu=NULL,LPSTR popmenuc=NULL)
{
HMENU lhmpskn,lhmpclr;
lhmpskn=MakeMenu(UI_PMENUSKINS,UI_POPMENUID(100));
lhmpclr=MakeMenu(UI_PMENUCOLOR,UI_POPMENUID(200));
if(hmuipop) DestroyMenu(hmuipop);
if(!popmenu) popmenu=UI_POPMENU;
hmuipop=MakeMenu(popmenu,UI_POPMENUID(0),lhmpclr,lhmpskn);
if(hmuipopc) DestroyMenu(hmuipopc);
if(!popmenuc) popmenuc=UI_POPMENUC;
hmuipopc=MakeMenu(popmenuc,UI_POPMENUID(0));
}

void UISKIN::Make(DWORD lskin=ID_WIN,int hg=0,int vg=0)
{
skin=lskin;
UseDef(skin);
font[0]=(HFONT)GetStockObject(SYSTEM_FONT);
font[1]=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
font[2]=(HFONT)GetStockObject(SYSTEM_FIXED_FONT);
gcw=hg;
gch=vg;
CalcGrid();
Menus();
}

void UISKIN::Free(int mod=0xf)
{
int i;
if(mod&0x1) //delete fonts
 {
 for(i=0;i<UI_FONTS;i++)
  if(GETBIT(delfont,i))
   DeleteObject(font[i]);
 delfont=0;
 }
if(mod&0x2) //delete fills
 {
 for(i=1;i<UI_FILLS;i++)
  if(GETBIT(delfill,i))
   DeleteObject(fill[i]);
 delfill=0;
 }
if(mod&0x4) //delete pens
 {
 for(i=0;i<UI_PENS;i++)
  if(GETBIT(delpen,i))
   DeleteObject(pen[i]);
 delpen=0;
 }
if(mod&0x8) //destroy menus
 {
 if(hmuipop) DestroyMenu(hmuipop);
 if(hmuipopc) DestroyMenu(hmuipopc);
 hmuipop=hmuipopc=NULL;
 }
}

void UISKIN::CalcGrid()
{
HDC hldc;
TEXTMETRIC tm;
hldc=GetDC(hmwnd);
SaveDC(hldc);
SelectObject(hldc,font[0]);
GetTextMetrics(hldc,&tm);
chh=tm.tmHeight;
chw=tm.tmMaxCharWidth;
cha=tm.tmAveCharWidth;
if(gcw<chw) gcw=chw;
if(gch<chh) gch=chh+2;
SelectObject(hldc,font[1]);
GetTextMetrics(hldc,&tm);
chh1=tm.tmHeight;
RestoreDC(hldc,-1);
ReleaseDC(hmwnd,hldc);
}

void UISKIN::Fonts()
{
Free(0x1);
font[0]=FontDlg(NULL,&color[1],hmwnd);
font[1]=FontDlg(NULL,&color[8],hmwnd);
font[2]=FontDlg(NULL,&color[18],hmwnd);
CalcGrid();
delfont=0x7;
}

void UISKIN::UseFont(NAT ind=0,LPSTR name=NULL,int cellh=16,int atrib=0,DWORD chset=DEFAULT_CHARSET,DWORD pitch=DEFAULT_PITCH)
{
ind%=UI_FONTS;
if(GETBIT(delfont,ind))
 DeleteObject(font[ind]);
delfont=SETBIT(delfont,ind);
font[ind]=CreateFont(cellh,0,0,0,(atrib&0xf)*100,atrib&0x10,(atrib>>8)&1,atrib>>12,chset,
          OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FF_DONTCARE|pitch,name);
if(ind<=1) CalcGrid();
}

void UISKIN::UseFill(NAT ind=0,COLORREF c=0)
{
ind%=UI_FILLS;
if(GETBIT(delfill,ind))
 DeleteObject(fill[ind]);
delfill=SETBIT(delfill,ind);
fill[ind]=CreateSolidBrush(c);
}

void UISKIN::UseDef(DWORD lskin=ID_WIN)
{
Free(0x6);
pen[0]=CreatePen(PS_NULL,0,0);
fill[0]=(HBRUSH)GetStockObject(HOLLOW_BRUSH);
color[0]=0;
switch(lskin)
 {
 case ID_STD:
 case IC_VLAD:
  pen[1]=CreatePen(PS_SOLID,0,BGR_DW(0,255,0)); //normal
  pen[2]=CreatePen(PS_SOLID,0,BGR_DW(0,0,255)); //foc
  pen[3]=CreatePen(PS_SOLID,0,BGR_DW(0,0,0)); //ro
  pen[4]=CreatePen(PS_SOLID,0,BGR_DW(0,128,0)); //normal
  pen[8]=CreatePen(PS_SOLID,0,BGR_DW(0,0,255)); //exp marg for combo
  pen[9]=CreatePen(PS_SOLID,0,BGR_DW(255,0,0));//exp marg for menu
  pen[10]=CreatePen(PS_SOLID,0,BGR_DW(0,0,0));//exp no pick
  pen[11]=CreatePen(PS_SOLID,0,BGR_DW(255,255,0));//exp for draw state
  delpen=0xf1f;
  fill[1]=CreateSolidBrush(BGR_DW(192,192,192)); //edit
  fill[2]=CreateSolidBrush(BGR_DW(224,224,224)); //edit foc
  fill[3]=CreateSolidBrush(BGR_DW(128,128,128)); //menu
  fill[4]=CreateSolidBrush(BGR_DW(96,96,96)); //button
  fill[5]=CreateSolidBrush(BGR_DW(64,64,64)); //ro
  fill[6]=CreateSolidBrush(BGR_DW(0,255,0)); //hlit
  fill[7]=CreateSolidBrush(BGR_DW(0,0,255)); //error
  fill[8]=CreateSolidBrush(BGR_DW(224,224,224)); //exp back for combo
  fill[9]=CreateSolidBrush(BGR_DW(128,128,128)); //exp back for menu
  fill[10]=CreateSolidBrush(BGR_DW(64,64,64)); //exp back for 'no pick'
  fill[11]=CreateSolidBrush(BGR_DW(255,0,255)); //info bar
  fill[12]=CreateSolidBrush(BGR_DW(92,92,92)); //window background
  fill[13]=CreateSolidBrush(BGR_DW(0,0,255)); //selection in exp
  delfill=0x1fff;
  color[1]=BGR_DW(0,0,0);//edit
  color[2]=BGR_DW(0,0,0);//edit foc
  color[3]=BGR_DW(0,0,0);//menu
  color[4]=BGR_DW(255,255,255);//button
  color[5]=BGR_DW(255,255,255);//ro
  color[6]=BGR_DW(0,0,0);//hlit
  color[7]=BGR_DW(255,255,255);//error
  color[8]=BGR_DW(0,0,0);//exp text for combo
  color[9]=BGR_DW(255,255,255);//exp text for menu
  color[10]=BGR_DW(0,255,255);//exp text for no pick and draw state 0
  color[11]=BGR_DW(255,255,255);//info bar text
  color[12]=BGR_DW(0,0,0);//state 0
  color[13]=BGR_DW(255,255,255);//state 1
  color[14]=BGR_DW(0,0,0);//bef lab
  color[15]=BGR_DW(0,0,0);//aft lab
  color[16]=BGR_DW(0,255,0);  //msg1 
  color[17]=BGR_DW(255,255,0);  //msg2 
  color[18]=BGR_DW(0,0,0);  //msg text 
  break;
 default: //ID_WIN,ID_CLS
  pen[1]=CreatePen(PS_SOLID,0,GetSysColor(COLOR_INACTIVECAPTION)); //normal
  pen[2]=CreatePen(PS_SOLID,0,GetSysColor(COLOR_ACTIVECAPTION)); //foc
  pen[3]=CreatePen(PS_SOLID,0,GetSysColor(COLOR_APPWORKSPACE)); //ro
  pen[4]=CreatePen(PS_SOLID,0,GetSysColor(COLOR_GRADIENTINACTIVECAPTION)); //normal 2nd shade
  pen[8]=CreatePen(PS_SOLID,0,GetSysColor(COLOR_WINDOWTEXT)); //exp marg for combo
  pen[9]=CreatePen(PS_SOLID,0,GetSysColor(COLOR_MENUTEXT));//exp marg for menu
  pen[10]=CreatePen(PS_SOLID,0,GetSysColor(COLOR_INFOTEXT));//exp marg for no pick
  pen[11]=CreatePen(PS_SOLID,0,GetSysColor(COLOR_HIGHLIGHT));//exp marg for draw state
  delpen=0xf1f;
  fill[1]=GetSysColorBrush(COLOR_WINDOW); //edit
  fill[2]=GetSysColorBrush(COLOR_WINDOW); //edit foc
  fill[3]=GetSysColorBrush(COLOR_MENU); //menu
  fill[4]=GetSysColorBrush(COLOR_BTNFACE); //button
  fill[5]=GetSysColorBrush(COLOR_BTNFACE); //ro
  fill[6]=GetSysColorBrush(COLOR_HIGHLIGHT); //hlit
  fill[7]=GetSysColorBrush(COLOR_INFOBK); //error
  fill[8]=GetSysColorBrush(COLOR_WINDOW); //exp back for combo
  fill[9]=GetSysColorBrush(COLOR_MENU);//exp back for menu
  fill[10]=GetSysColorBrush(COLOR_INFOBK);//exp no pick
  fill[11]=GetSysColorBrush(COLOR_INFOBK); //info bar
  fill[12]=GetSysColorBrush(COLOR_APPWORKSPACE); //window background
  fill[13]=GetSysColorBrush(COLOR_INFOBK); //selection in exp
  delfill=0x000;
  color[1]=GetSysColor(COLOR_WINDOWTEXT);//edit
  color[2]=GetSysColor(COLOR_WINDOWTEXT);//edit foc
  color[3]=GetSysColor(COLOR_MENUTEXT);//menu
  color[4]=GetSysColor(COLOR_BTNTEXT);//button
  color[5]=GetSysColor(COLOR_BTNTEXT);//ro
  color[6]=GetSysColor(COLOR_HIGHLIGHTTEXT);//hlit
  color[7]=GetSysColor(COLOR_INFOTEXT);//error
  color[8]=GetSysColor(COLOR_WINDOWTEXT);//exp text for combo
  color[9]=GetSysColor(COLOR_MENUTEXT);//exp text for menu
  color[10]=GetSysColor(COLOR_INFOTEXT);//exp text for no pick and draw state 0
  color[11]=GetSysColor(COLOR_INFOTEXT);//info bar text
  color[12]=GetSysColor(COLOR_MENUTEXT);//state 0
  color[13]=GetSysColor(COLOR_HIGHLIGHTTEXT);//state 1
  color[14]=GetSysColor(COLOR_BTNTEXT);//bef lab
  color[15]=GetSysColor(COLOR_BTNTEXT);//aft lab
  color[16]=GetSysColor(COLOR_ACTIVECAPTION); //msg1 
  color[17]=GetSysColor(COLOR_GRADIENTACTIVECAPTION);  //msg2 
  color[18]=GetSysColor(COLOR_CAPTIONTEXT);  //msg text 
 } 
}
//UISKIN >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
