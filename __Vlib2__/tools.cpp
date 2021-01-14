#ifndef V_COMMON_TOOLS
#define V_COMMON_TOOLS

//Rect manipulation functions :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#define RCT_OUT 0x00 //outside
#define RCT_L   0x01 //over left border
#define RCT_R   0x02 //over right border
#define RCT_U   0x04 //over top border
#define RCT_D   0x08 //over bottom border
#define RCT_B   0x0f //over any border
#define RCT_MH  0x10 //x over middle
#define RCT_MV  0x20 //y over middle
#define RCT_M   0x30 //over middle

//ReCTangle ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct RCT //uses windows convension (excludes bottom/right)
{
union
 {
 struct{ long l,u,r,d; };
 struct{ float L,U,R,D; }; //convention: capitals are real, smalls are integers
 struct{ long x,y,w,h; };
 struct{ long left,top,right,bottom; }; //for compatibility
 RECT rect;
 };

//.................................................................
operator RECT*() //RECT*,LPRECT
 {
 return (RECT*)this;
 }
//.................................................................
operator RECT() //RECT
 {
 return rect;
 }
//.................................................................
RCT& operator =(RCT rct) //=RCT
 {
 CopyMemory(this,&rct,sizeof(RCT));
 return *this;
 }
//.................................................................
RCT& operator =(RECT lrect) //=RECT
 {
 CopyMemory(this,&lrect,sizeof(RCT));
 return *this;
 }
//.................................................................
RCT& operator =(VECT4 v4) //=VECT4
 {
 CopyMemory(this,&v4,sizeof(RCT));
 return *this;
 }
//.................................................................
RCT& operator =(HWND hwnd) //=HWND
 {
 GetClientRect(hwnd,&rect);
 return *this;
 }
//.................................................................
RCT& operator>>(int delta)//move right/left
 {
 l+=delta; r+=delta;
 return *this;
 }
//.................................................................
RCT& operator<<(int delta)//move left/right
 {
 l-=delta; r-=delta;
 return *this;
 }
//.................................................................
RCT& operator^(int delta)//move up/down
 {
 u-=delta; d-=delta;
 return *this;
 }
//.................................................................
RCT& operator()(int x1=0,int y1=0,int x2=0,int y2=0)//(,,,)
 {
 l=x1; u=y1; r=x2; d=y2;
 return *this;
 }
//.................................................................
long W()
 {
 return ABS(r-l);
 }
//.................................................................
long H()
 {
 return ABS(d-u);
 }
//.................................................................
void LU(int x1=0,int y1=0)
 {
 l=x1; u=y1;
 }
//.................................................................
void RD(int x2=0,int y2=0)
 {
 r=x2; d=y2;
 }
//.................................................................
void add(int dx,int dy)
 {
 l+=dx; r+=dx;
 u+=dy; d+=dy;
 }
//.................................................................
void norm()
 {
 if(l>r)
  swap(l,r);
 if(u>d)
  swap(u,d);
 }
//check if point is inside rect.................................................................
BOOL inc(int x,int y) //rect includes point
 {
 if(x<l) return 0;
 if(x>=r) return 0;
 if(y<u) return 0;
 if(y>=d) return 0;
 return 1;
 }
//keep inside ...................................................................................
void inside(int x1,int y1,int x2,int y2)
 {
 norm();
 if(l<x1) l=x1;
 if(r>x2) r=x2;
 if(u<y1) u=y1;
 if(d>y2) d=y2;
 }
//map rect from his plane(mpl) to another(opl) ----------------------------------------------------------------------------
inline void map(RCT*mpl,RCT*opl)
 {
 float f;
 f=(float)(opl->r-opl->l)/(mpl->r-mpl->l);
 l=(l-mpl->l)*f+opl->l;
 r=(r-mpl->l)*f+opl->l;
 f=(float)(opl->d-opl->u)/(mpl->d-mpl->u);
 u=(u-mpl->u)*f+opl->u;
 d=(d-mpl->u)*f+opl->u;
 }
//map rect from one window to another  ----------------------------------------------------------------------------
inline void map(HWND child,HWND parent)
 {
 MapWindowPoints(parent,child,(POINT*)this,2);
 }
//drag closest corner to point .................................................................
void DragClosest(int mx,int my)
 {
 if(ABS(l-mx)<ABS(r-mx))
  l=mx;
 else
  r=mx;
 if(ABS(u-my)<ABS(d-my))
  u=my;
 else
  d=my;
 norm();
 }
//.................................................................
void DBG()
 {
 printbox("(%i %i)-(%i %i) W=%i H=%i",l,u,r,d,r-l,d-u);
 }
#ifdef V_MFC
//.................................................................
RCT& operator =(CWnd*pcwnd) //=CWnd*
 {
 pcwnd->GetClientRect(&rect);
 return *this;
 }
//.................................................................
operator CRect*() //(CRect*)
 {
 return (CRect*)this;
 }
//.................................................................
operator CRect() //(CRect)
 {
 return *((CRect*)this);
 }
#endif
}mwarea; //workarea: main monitor desktop minus taskbar

//map point from one rect to another ----------------------------------------------------------------------------
inline void MapPfromRStoRD(int&mx,int&my,RCT*RS,RCT*RD)
{
mx=(mx-RS->l)*(RD->r-RD->l)/(RS->r-RS->l)+RD->l;
my=(my-RS->u)*(RD->d-RD->u)/(RS->d-RS->u)+RD->u;
}

//inits a RECT structure ----------------------------------------------------------------------------
inline void __cdecl Rect(void *pr,int x,int y,int w,int h)
{
__asm
 {
 mov edx,pr
 mov eax,x
 mov [edx],eax
 add eax,w
 mov [edx+8],eax
 mov eax,y
 mov [edx+4],eax
 add eax,h
 mov [edx+12],eax
 }
}

//tests if point is inside rect ---------------------------------------------------------
BOOL __cdecl PinR(int x,int y,int l,int u,int r,int d)
{
__asm
 {
 xor eax,eax
 mov edx,x
 cmp edx,l //x<l
 jl LBreakOut
 cmp edx,r
 jge LBreakOut //x>=r
 mov edx,y
 cmp edx,u //y<u
 jl LBreakOut
 cmp edx,d
 jge LBreakOut //y>=d
 mov eax,1
LBreakOut:
 }
}

//returns the zone where the point is relative to a rectangle with a border --------------------------
DWORD PinRb(int x,int y,int l,int u,int r,int d,int b=1)
{
if(x<l||x>=r||y<u||y>=d) return RCT_OUT;
DWORD ret;
if(x<l+b) ret=RCT_L;
else if(x>=r-b) ret=RCT_R;
else ret=RCT_MH;
if(y<u+b) ret|=RCT_U;
else if(y>=d-b) ret|=RCT_D;
else ret|=RCT_MV;
return ret;
}

//adjusts rect based on flags ------------------------------------------------------------
int DeltaRect(RECT*dr,int hd=0,int vd=0,DWORD flags=RCT_M,int nrsz=0)
{
if(flags==RCT_M) //move
 {
 dr->left+=hd;
 dr->right+=hd;
 dr->top+=vd;
 dr->bottom+=vd;
 return 1;
 }
if(nrsz) return 0; //no resize
if(flags&RCT_L) dr->left+=hd;
else if(flags&RCT_R) dr->right+=hd;
if(flags&RCT_U) dr->top+=vd;
else if(flags&RCT_D) dr->bottom+=vd;
return 1;
}

//swaps points so that: d>u&&r>l -------------------------------------------------------------
inline void NormRect(RECT *rct)
{
if(rct->left>rct->right)
 {
 fpuioi=rct->left;
 rct->left=rct->right;
 rct->right=fpuioi;
 }
else if (rct->left==rct->right)
 rct->right++;
if(rct->top>rct->bottom)
 {
 fpuioi=rct->top;
 rct->top=rct->bottom;
 rct->bottom=fpuioi;
 }
else if (rct->top==rct->bottom)
 rct->bottom++;
}

//enlarges/shrinks a rectangle to accomodate a border (simmetricaly) ----------------------
inline void BorderRect(RECT *rct,int hbord=0,int vbord=0)
{
rct->left-=hbord;
rct->right+=hbord;
rct->top-=vbord;
rct->bottom+=vbord;
}

//centers drct in srct keeping width and height ------------------------------------------
inline void CenterRect(RECT *drct,RECT *srct)
{
drct->left=(srct->right-srct->left-drct->right+drct->left)>>1;
drct->right=srct->right-drct->left;
drct->left+=srct->left;
drct->top=(srct->bottom-srct->top-drct->bottom+drct->top)>>1;
drct->bottom=srct->bottom-drct->top;
drct->top+=srct->top;
}

//calculates the differences in width and height between rect a and b ------------------------
inline void SubRect(RECT *arct,RECT *brct,int*pw,int*ph)
{
*pw=arct->right-arct->left-brct->right+brct->left;
*ph=arct->bottom-arct->top-brct->bottom+brct->top;
}

//enlarges dr to enclose [l,u,r,d] -----------------------------------------------------
inline void KeepAround(RECT*dr,int l,int u,int r,int d)
{
if(dr->left>l) dr->left=l;
if(dr->right<r) dr->right=r;
if(dr->top>u) dr->top=u;
if(dr->bottom<d) dr->bottom=d;
}

//shrinks dr to be enclosed by [l,u,r,d] but not smaller than [minw,minh] -----------------------------------------------------
inline void KeepInside(RECT*dr,int l,int u,int r,int d,int minw=0,int minh=0)
{
if(dr->left<l) dr->left=l;
if(dr->left>r) dr->left=r-minw;
if(dr->right<l) dr->right=l+minw;
if(dr->right>r) dr->right=r;
if(dr->top<u) dr->top=u;
if(dr->top>d) dr->top=d-minh;
if(dr->bottom<u) dr->bottom=u+minh;
if(dr->bottom>d) dr->bottom=d;
}

//in place rect creation -------------------------------------------------------------
inline RECT MRECT(int x,int y,int w,int h)
{
RECT rct;
rct.right=(rct.left=x)+w;
rct.bottom=(rct.top=y)+h;
return rct;
}

#endif