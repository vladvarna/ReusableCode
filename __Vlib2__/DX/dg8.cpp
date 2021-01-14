#ifndef V_IDIRECTXG8
#define V_IDIRECTXG8

#include <DX/dx8.cpp>
#include <d3d8.h> //d3d8.lib
#pragma comment(lib,"d3d8.lib")
#include <3D/3d.cpp>

IDirect3D8 *dxobj=NULL; //D3D main object
IDirect3DDevice8 *dxdev=NULL; //D3D main device
IDirect3DSurface8 *dxbbuf=NULL; //back buffer surface

#define FVF_VC (D3DFVF_XYZ|D3DFVF_DIFFUSE)
#define FVF_VT (D3DFVF_XYZ|D3DFVF_TEX1)
#define FVF_VCT (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define FVF_VNT (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
#define FVF_VNCT (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define FVF_VNCT2 (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2)
#define FVF_VNCT3 (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX3)

class Light
{
public:
 int li;
 D3DLIGHT8 dl;
 
 void Make(int lind=0,float x=0,float y=0,float z=1,float w=1);
 void Spot(int lind=0,float px=0,float py=0,float pz=0,float qx=1,float qy=1,float qz=1,
           float fovo=PI,float fovi=PI2,float fexp=1);
 void Range(float r=1,float c=1,float b=0,float a=0);
 void Color(float sr=0,float sg=0,float sb=0,float sa=0,float dr=0,float dg=0,float db=0,float da=0,float ar=0,float ag=0,float ab=0,float aa=0);
 void Mono(float spc=.3,float dif=.5,float amb=.2);
};
//*****************************************************************************************

//returns D3DFORMAT associated with PF --------------------------------------------------------
D3DFORMAT PFtoD3DF(DWORD pixform=0x32108888,int x=0)
{
if(pixform==0x32100565) return D3DFMT_R5G6B5; //16
else if(pixform==0x32108888) return x?D3DFMT_X8R8G8B8:D3DFMT_A8R8G8B8; //32
else if(pixform==0x32101555) return x?D3DFMT_X1R5G5B5:D3DFMT_A1R5G5B5; //1+15
else if(pixform==0x32104444) return x?D3DFMT_X4R4G4B4:D3DFMT_A4R4G4B4; //4+12
else if(pixform==0x32100888) return D3DFMT_R8G8B8; //24
else if(pixform==0x32100332) return D3DFMT_R3G3B2; //8
else if(pixform==0x32108332) return D3DFMT_A8R3G3B2; //8+8
else if(pixform==0x32108000) return D3DFMT_A8; //8+0
else return D3DFMT_UNKNOWN;
}

//returns PF associated with D3DFORMAT -----------------------------------------------------
DWORD D3DFtoPF(D3DFORMAT d3dform=D3DFMT_UNKNOWN)
{
if(d3dform==D3DFMT_R5G6B5) return 0x32100565; //16
else if(d3dform==D3DFMT_X8R8G8B8||d3dform==D3DFMT_A8R8G8B8) return 0x32108888; //32
else if(d3dform==D3DFMT_X1R5G5B5||d3dform==D3DFMT_A1R5G5B5) return 0x32101555; //1+15
else if(d3dform==D3DFMT_A4R4G4B4||d3dform==D3DFMT_X4R4G4B4) return 0x32104444; //4+12
else if(d3dform==D3DFMT_R8G8B8) return 0x32100888; //24
else if(d3dform==D3DFMT_R3G3B2) return 0x32100332; //8
else if(d3dform==D3DFMT_A8R3G3B2) return 0x32108332; //8+8
else if(d3dform==D3DFMT_A8) return 0x32108000; //8+0
else return 0;
}

//returns back buffer D3DFORMAT associated with RF --------------------------------------------------------
D3DFORMAT RFtoDGF1(DWORD rendform=0)
{
if(RF_cbpp(rendform)==32) return RF_abpp(rendform)?D3DFMT_A8R8G8B8:D3DFMT_X8R8G8B8; //32
else if(RF_cbpp(rendform)==16) return RF_abpp(rendform)?D3DFMT_A1R5G5B5:D3DFMT_R5G6B5; //16
else if(RF_cbpp(rendform)==24) return D3DFMT_R8G8B8; //24
else return D3DFMT_UNKNOWN;
}

//returns stencil buffer D3DFORMAT associated with RF --------------------------------------------------------
D3DFORMAT RFtoDGF2(DWORD rendform=0)
{
if(RF_dbpp(rendform)==32) return D3DFMT_D32;
else if(RF_dbpp(rendform)==24)
 {
 if(RF_sbpp(rendform)==8) return D3DFMT_D24S8; //24+8
 else if(RF_sbpp(rendform)==4) return D3DFMT_D24X4S4; //24+4
 else return D3DFMT_D24X8; //24
 }
else if(RF_dbpp(rendform)==16) return RF_sbpp(rendform)?D3DFMT_D16_LOCKABLE:D3DFMT_D16; //16
else if(RF_dbpp(rendform)==15) return D3DFMT_D15S1; //15+1
else return D3DFMT_UNKNOWN;
}

// handles texture transformations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct DGTEXTURETF
 {
 float mt[16];
 
 void On(int stage=0)
  {
  dxdev->SetTransform((_D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage),(D3DMATRIX*)mt);
  }
 void Flip(float x=1.0f,float y=-1.0f)
  {
  mt[1]=mt[2]=mt[3]=mt[4]=mt[6]=mt[7]=mt[8]=mt[9]=mt[11]=mt[12]=mt[13]=mt[14]=0;
  mt[0]=x; mt[5]=y;
  mt[10]=mt[15]=1;
  }
 }dttf;

//general viewport format ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct DGVIEWPORT
 {
 long x,y;    //ul corner  |
 DWORD w,h;   //dimensions }D3DVIEWPORT8
 float f,b;   //minz,maxz  |
 float mv[16];//tf matrix

 void Set(int l=0,int u=0,int lng=0,int lat=0,float minz=0,float maxz=1)
  {
  x=l; y=u;
  w=lng; h=lat;
  f=minz; b=maxz;
  M_viewport(mv,x,y,w,h,f,b);
  }
 void On()
  {
  dxdev->SetViewport((D3DVIEWPORT8*)this);
  }
 }dport;

// handles projection transformations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct DGPROJECTION
 {
 float mp[16];
 
 void On()
  {
  dxdev->SetTransform(D3DTS_PROJECTION,(D3DMATRIX*)mp);
  }
 void Ortho(float logw=2,float logh=2,float minz=1,float maxz=100)
  {
  mp[1]=mp[2]=mp[3]=mp[4]=mp[6]=mp[7]=mp[8]=mp[9]=mp[11]=mp[12]=mp[13]=0;
  mp[0]=2/logw;
  mp[5]=2/logh;
  mp[10]=1/(maxz-minz);
  mp[14]=-minz*mp[10]; //translation is in (0;1) not (minz;maxz)
  mp[15]=1;
  }
 void Perspect(float logw=2,float logh=2,float minz=1,float maxz=100)
  {
  mp[1]=mp[2]=mp[3]=mp[4]=mp[6]=mp[7]=mp[8]=mp[9]=mp[12]=mp[13]=mp[15]=0;
  mp[0]=minz*2/logw;
  mp[5]=minz*2/logh;
  mp[10]=maxz/(maxz-minz);
  mp[11]=1;
  mp[14]=-minz*mp[10]; //translation is in (0;1) not (minz;maxz)
  }
 void Fov(float hfov=PI2,float vfov=PI2,float minz=1,float maxz=100)
  {
  mp[1]=mp[2]=mp[3]=mp[4]=mp[6]=mp[7]=mp[8]=mp[9]=mp[12]=mp[13]=mp[15]=0;
  mp[0]=1/tan(hfov*.5);
  mp[5]=1/tan(vfov*.5);
  mp[10]=maxz/(maxz-minz);
  mp[11]=1;
  mp[14]=-minz*mp[10]; //translation is in (0;1) not (minz;maxz)
  }
 }dproj;

//general camera ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct DGCAMERA
 {
 float mc[16];
 float Ox,Oy,Oz; //camera origin in world space

 void Init() //set it to null
  {
  Ox=Oy=Oz=0;
  Mnull(mc);
  }
 void On() //set it as D3D's view tf
  {
  dxdev->SetTransform(D3DTS_VIEW,(D3DMATRIX*)mc);
  }
 void Set(float ox=0,float oy=0,float oz=-1,float hx=0,float hy=0,float hz=1)
  {
  Ox=ox; Oy=oy; Oz=oz;
  M_cam(mc,&ox,&hx);
  }
 void MoveTo(float x=0,float y=0,float z=0)
  {
  Ox=x; Oy=y; Oz=z; //x=-x;
  mc[12]=-x*mc[0]-y*mc[4]-z*mc[8];
  mc[13]=-x*mc[1]-y*mc[5]-z*mc[9];
  mc[14]=-x*mc[2]-y*mc[6]-z*mc[10];
  }
 void LookAt(float x=0,float y=0,float z=0)
  {
  x-=Ox; y-=Oy; z-=Oz;
  M_cam(mc,&Ox,&x);
  }
 void SpinLR(float u=0)
  {
  MspinY(mc,-u);
  MoveTo(Ox,Oy,Oz);
  }
 void SpinUD(float u=0)
  {
  MspinX(mc,u);
  MoveTo(Ox,Oy,Oz);
  }
 void MoveLR(float d=0) 
  {
  MoveTo(Ox+d*mc[0],Oy+d*mc[4],Oz+d*mc[8]);
  }
 void MoveUD(float d=0)
  {
  MoveTo(Ox+d*mc[1],Oy+d*mc[5],Oz+d*mc[9]);
  }
 void MoveFB(float d=0)
  {
  MoveTo(Ox+d*mc[2],Oy+d*mc[6],Oz+d*mc[10]);
  }
 }dcam;

//initializes D3D8 ------------------------------------------------------------------------
int InitDG(DWORD rendform=0,int hres=0,int vres=0,DWORD flag=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER,D3DDEVTYPE dtype=D3DDEVTYPE_HAL)
{
D3DDISPLAYMODE dxdm;
D3DSURFACE_DESC dxsd;
D3DPRESENT_PARAMETERS dxpp;
dxobj=Direct3DCreate8(D3D_SDK_VERSION);
isNULL(dxobj,"DirectX Graphics object creation failed");
dxobj->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&dxdm);
ZeroMemory(&dxpp,sizeof(dxpp));
dxpp.BackBufferCount=1;
dxpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
dxpp.hDeviceWindow=hmwnd;
dxpp.Flags=flag;
if(rendform&0xffff) //fullscreen
 {
 dxpp.BackBufferFormat=RFtoDGF1(rendform);
 dxpp.BackBufferWidth=hres?hres:dxdm.Width;
 dxpp.BackBufferHeight=vres?vres:dxdm.Height;
 dxpp.FullScreen_RefreshRateInHz=D3DPRESENT_RATE_DEFAULT;
 dxpp.FullScreen_PresentationInterval=D3DPRESENT_INTERVAL_ONE;
 }
else //windowed
 {
 dxpp.Windowed=TRUE;
 dxpp.BackBufferFormat=dxdm.Format;
 dxpp.BackBufferWidth=hres;
 dxpp.BackBufferHeight=vres;
 AdjustClientRect(hmwnd,hres,vres);
 }
if(rendform>>16) //enable z-buffer
 {
 dxpp.EnableAutoDepthStencil=TRUE;
 dxpp.AutoDepthStencilFormat=RFtoDGF2(rendform);
 }
erret=dxobj->CreateDevice(D3DADAPTER_DEFAULT,dtype,hmwnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&dxpp,&dxdev);
if(erret!=D3D_OK)
 {
 if(error("Device creation failed\n\nTry the reference rasterizer ?"))
  {
  erret=dxobj->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_REF,hmwnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&dxpp,&dxdev);
  }
 if(erret!=D3D_OK) 
  {
  DELCOM(dxobj);
  return erret;
  }
 }
erret=dxdev->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&dxbbuf);
isDXOk("Back buffer access failed");
erret=dxobj->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&dxdm);
isDXOk("New display mode identification failed");
erret=dxbbuf->GetDesc(&dxsd);
scrw=dxdm.Width;  
scrh=dxdm.Height;
scrpf=D3DFtoPF(dxdm.Format);
scrbpp=PF_bpix(scrpf);
dxdev->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATER);
dport.Set(0,0,dxsd.Width,dxsd.Height,0,1);
dcam.Init();
return erret;
}

//Releases main D3D object and device ---------------------------------------------------------
void FreeDG()
{
RelCOM(dxbbuf,"D3D Back buffer");
dxbbuf=NULL;
RelCOM(dxdev,"D3D Device");
dxdev=NULL;
RelCOM(dxobj,"D3D Object");
dxobj=NULL;
InitMode();
}

//transforms a vertex (position and normal) using the D3D matrices --------------------------------------------------------
void DXtf(float *dpct,float *spct=NULL,float *matw=NULL,float *matc=NULL,float *matp=NULL,float *matv=NULL,unsigned ond=0,unsigned ons=0)
{
if(spct) CopyMemory(dpct,spct,12);
dpct[3]=1.0f;
if(matw) VxM4(dpct,matw);
if(matc) VxM4(dpct,matc);
if(matp) VxM4(dpct,matp);
if(matv) VxM4(dpct,matv);
dpct[3]=1.0f/dpct[3];
dpct[0]*=dpct[3];
dpct[1]*=dpct[3];
dpct[2]*=dpct[3];
if(!ond||!matw) return;
if(spct)
 {
 if(!ons) ons=ond;
 CopyMemory(dpct+ond,spct+ons,12);
 }
VxM3(dpct+ond,matw);
}

//D3D material ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct DGMATERIAL
 {
 D3DMATERIAL8 fab;
 void On()
  {
  dxdev->SetMaterial(&fab);
  }
 void Set(float sr=1,float sg=1,float sb=1,float sa=1,float dr=1,float dg=1,float db=1,float da=1,float ar=1,float ag=1,float ab=1,float aa=1,float er=0,float eg=0,float eb=0,float ea=0,float sp=16)
  {
  CopyMemory(&fab,&sr,sizeof(D3DMATERIAL8));
  }
 void Mono(float spc=1,float dif=1,float amb=1,float ems=0,float sp=16)
  {
  fab.Diffuse.r=fab.Diffuse.g=fab.Diffuse.b=fab.Diffuse.a=dif;
  fab.Ambient.r=fab.Ambient.g=fab.Ambient.b=fab.Ambient.a=amb;
  fab.Specular.r=fab.Specular.g=fab.Specular.b=fab.Specular.a=spc;
  fab.Emissive.r=fab.Emissive.g=fab.Emissive.b=fab.Emissive.a=ems;
  fab.Power=sp;
  }
 void Use(int ssrc=0,int dsrc=0,int asrc=0,int esrc=0)
  {
  dxdev->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,dsrc);
  dxdev->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,asrc);
  dxdev->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,ssrc);
  dxdev->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,esrc);
  }
 }dmater; //default material

// Light <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void Light::Make(int lind,float x,float y,float z,float w)
{
li=lind;
dxdev->LightEnable(li,TRUE);
if(w==0)
 {
 dl.Type=D3DLIGHT_DIRECTIONAL;
 dl.Direction.x=x; dl.Direction.y=y; dl.Direction.z=z;
 }
else if(w==1)
 {
 dl.Type=D3DLIGHT_POINT;
 dl.Position.x=x; dl.Position.y=y; dl.Position.z=z;
 }
else ZeroMemory(&dl,sizeof(dl));
dxdev->SetLight(li,&dl);
}

void Light::Spot(int lind,float px,float py,float pz,float qx,float qy,float qz,float fovo,float fovi,float fexp)
{
li=lind;
dl.Type=D3DLIGHT_SPOT;
dl.Position.x=px; dl.Position.y=py; dl.Position.z=pz;
dl.Direction.x=qx; dl.Direction.y=qy; dl.Direction.z=qz;
dl.Phi=fovo; dl.Theta=fovi;
dl.Falloff=fexp;
dxdev->SetLight(li,&dl);
dxdev->LightEnable(li,TRUE);
}

void Light::Range(float r,float c,float b,float a)
{
dl.Range=r;
dl.Attenuation0=c;
dl.Attenuation1=b;
dl.Attenuation2=a;
dxdev->SetLight(li,&dl);
}

void Light::Color(float sr,float sg,float sb,float sa,float dr,float dg,float db,float da,float ar,float ag,float ab,float aa)
{
dl.Ambient.r=ar; dl.Ambient.g=ag; dl.Ambient.b=ab; dl.Ambient.a=aa;
dl.Diffuse.r=dr; dl.Diffuse.g=dg; dl.Diffuse.b=db; dl.Diffuse.a=da;
dl.Specular.r=sr; dl.Specular.g=sg; dl.Specular.b=sb; dl.Specular.a=sa;
dxdev->SetLight(li,&dl);
}

void Light::Mono(float spc,float dif,float amb)
{
dl.Ambient.r=dl.Ambient.g=dl.Ambient.b=dl.Ambient.a=amb;
dl.Diffuse.r=dl.Diffuse.g=dl.Diffuse.b=dl.Diffuse.a=dif;
dl.Specular.r=dl.Specular.g=dl.Specular.b=dl.Specular.a=spc;
dxdev->SetLight(li,&dl);
}
// Light >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//D3D Fog ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct DGFOG
 {
 DWORD color;
 int type,mode; //0-pixel(z|w) 1-vertex(z) 2-vertex(w), 0-no 1-exp 2-exp2 3-lin
 QUADB beg,end;

 void Set(int tip=0x31,DWORD cdw=0,float z1=1,float z2=100)
  {
  type=tip&0x3; mode=(tip&0x30)>>4;
  beg.f=z1; end.f=z2;
  color=cdw;
  if(type)
   {
   dxdev->SetRenderState(D3DRS_FOGVERTEXMODE,mode);
   if(type==2) dxdev->SetRenderState(D3DRS_RANGEFOGENABLE,1);
   }
  else dxdev->SetRenderState(D3DRS_FOGTABLEMODE,mode);
  if(mode==3)
   {
   dxdev->SetRenderState(D3DRS_FOGSTART,beg.n);
   dxdev->SetRenderState(D3DRS_FOGEND,end.n);
   }
  else dxdev->SetRenderState(D3DRS_FOGDENSITY,beg.n);
  dxdev->SetRenderState(D3DRS_FOGCOLOR,color);
  dxdev->SetRenderState(D3DRS_FOGENABLE,1);
  }
 };

#endif