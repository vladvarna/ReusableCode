#ifndef V_IDIRECT3DX8
#define V_IDIRECT3DX8

#include <DX/dg8.cpp>
//#include <d3dx8.h>
//#pragma comment(lib,"d3dx8.lib")
#include <mmed.cpp>
#include <img.cpp>
#include <3D/mesh.cpp>


//holds important device states ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct DevState
{
DWORD shader;
IDirect3DVertexBuffer8 *vb;
UINT vs; //vertex stride
IDirect3DIndexBuffer8 *ib;
UINT bi; //base index
DWORD depthbuf,dbwrite;
DWORD light,specular;
DWORD atest,ablend;
DWORD fog;
DWORD colvert;
DWORD colop;
void Get()
 {
 dxdev->GetVertexShader(&shader);
 dxdev->GetStreamSource(0,&vb,&vs);
 if(vb) vb->Release();
 dxdev->GetIndices(&ib,&bi);
 if(ib) ib->Release();
 dxdev->GetRenderState(D3DRS_ZENABLE,&depthbuf);
 dxdev->GetRenderState(D3DRS_ZWRITEENABLE,&dbwrite);
 dxdev->GetRenderState(D3DRS_LIGHTING,&light);
 dxdev->GetRenderState(D3DRS_SPECULARENABLE,&specular);
 dxdev->GetRenderState(D3DRS_ALPHATESTENABLE,&atest);
 dxdev->GetRenderState(D3DRS_ALPHABLENDENABLE,&ablend);
 dxdev->GetRenderState(D3DRS_FOGENABLE,&fog);
 dxdev->GetRenderState(D3DRS_COLORVERTEX,&colvert);
 dxdev->GetTextureStageState(0,D3DTSS_COLOROP,&colop);
 }
void Set()
 {
 dxdev->SetVertexShader(shader);
 dxdev->SetStreamSource(0,vb,vs);
 dxdev->SetIndices(ib,bi);
 dxdev->SetRenderState(D3DRS_ZENABLE,depthbuf);
 dxdev->SetRenderState(D3DRS_ZWRITEENABLE,dbwrite);
 dxdev->SetRenderState(D3DRS_LIGHTING,light);
 dxdev->SetRenderState(D3DRS_SPECULARENABLE,specular);
 dxdev->SetRenderState(D3DRS_ALPHATESTENABLE,atest);
 dxdev->SetRenderState(D3DRS_ALPHABLENDENABLE,ablend);
 dxdev->SetRenderState(D3DRS_FOGENABLE,fog);
 dxdev->SetRenderState(D3DRS_COLORVERTEX,colvert);
 dxdev->SetTextureStageState(0,D3DTSS_COLOROP,colop);
 }
}dstat;
//***************************************************************************************

//gets and prints surface description ---------------------------------------------------
void printsurf(IDirect3DSurface8 *d3dsurf)
{
D3DSURFACE_DESC dsd;
d3dsurf->GetDesc(&dsd);
printbox("%ux%u %x %uB",dsd.Width,dsd.Height,D3DFtoPF(dsd.Format),dsd.Size);
}

//creates a solid color D3D texture from a ARGB ---------------------------------------
IDirect3DTexture8* DXTexSolid(DWORD dwabgr,int w=1,int h=1)
{
IDirect3DTexture8* tmptex;
erret=dxdev->CreateTexture(w,h,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&tmptex);
if(erret!=D3D_OK) return NULL; //unable to create texture object
D3DLOCKED_RECT lck;
erret=tmptex->LockRect(0,&lck,NULL,0);
if(erret!=D3D_OK) return NULL; //unable to lock texture
DWORD *tex=(DWORD*)lck.pBits;
for(unsigned el=0;el<w*h;el++)
 tex[el]=dwabgr;
erret=tmptex->UnlockRect(0);
return tmptex;
}

//draws a bar in a DX surface ---------------------------------------------------------
void DXFillRect(IDirect3DSurface8 *dxsurf,int l=0,int u=0,int r=0,int d=0,DWORD dwabgr=0)
{
D3DLOCKED_RECT lck;
erret=dxsurf->LockRect(&lck,NULL,0);
if(erret!=D3D_OK) return; //unable to lock surface
lck.Pitch>>=2;
DWORD *surf=(DWORD*)lck.pBits+u*lck.Pitch;
int el;
while(u<=d)
 {
 surf+=lck.Pitch;
 for(el=l;el<=r;el++)
  *(surf+el)=dwabgr;
 u++;
 }
erret=dxsurf->UnlockRect();
}

//draws a 3d line ----------------------------------------------------------------------------------
void DLine3(float x1=0,float y1=0,float z1=0,DWORD c1=0,float x2=0,float y2=0,float z2=0,DWORD c2=0)
{
dstat.Get();
dxdev->SetVertexShader(FVF_VC);
dxdev->SetRenderState(D3DRS_LIGHTING,0);
dxdev->SetTextureStageState(0,D3DTSS_COLOROP,1);
dxdev->DrawPrimitiveUP(D3DPT_LINELIST,1,&x1,16);
dstat.Set();
}

//draws a 3d triangle ------------------------------------------------------------------------------
void DTri3(float x1=0,float y1=0,float z1=0,DWORD c1=0,float x2=0,float y2=0,float z2=0,DWORD c2=0,float x3=0,float y3=0,float z3=0,DWORD c3=0)
{
dstat.Get();
dxdev->SetVertexShader(FVF_VC);
dxdev->SetRenderState(D3DRS_LIGHTING,0);
dxdev->SetTextureStageState(0,D3DTSS_COLOROP,1);
dxdev->DrawPrimitiveUP(D3DPT_TRIANGLELIST,1,&x1,16);
dstat.Set();
}

//draws a 2d triangle ------------------------------------------------------------------------------
void DTri2(float x1=0,float y1=0,float z1=0,float w1=0,DWORD c1=0,float x2=0,float y2=0,float z2=0,float w2=0,DWORD c2=0,float x3=0,float y3=0,float z3=0,float w3=0,DWORD c3=0)
{
dstat.Get();
dxdev->SetVertexShader(D3DFVF_XYZRHW|D3DFVF_DIFFUSE);
dxdev->SetTextureStageState(0,D3DTSS_COLOROP,1);
dxdev->SetRenderState(D3DRS_LIGHTING,0);
dxdev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
dxdev->DrawPrimitiveUP(D3DPT_TRIANGLELIST,1,&x1,16);
dstat.Set();
}

//draws normals as lines with origin at vertex -----------------------------------------
void drawnorms(float *vb,unsigned vcnt=1,unsigned stride=9)
{
while(vcnt>0)
 {
 DLine3(vb[0],vb[1],vb[2],0,vb[0]+vb[3],vb[1]+vb[4],vb[2]+vb[5],0xffffff);
 vb+=stride;
 vcnt--;
 }
}

//visual apperance of a matrix ---------------------------------------------------------
void drawmat(float *m,float *o=NULL)
{
if(!o) o=m+12;
dxdev->SetTransform(D3DTS_WORLD,(D3DMATRIX*)M_null);
DTri3(o[0],o[1],o[2],0xffffff,
      o[0]+m[2],o[1]+m[6],o[2]+m[10],0xff0000,
      o[0]+m[0],o[1]+m[4],o[2]+m[8],0xff);
DTri3(o[0],o[1],o[2],0xffffff,
      o[0]+m[0],o[1]+m[4],o[2]+m[8],0xff,
      o[0]+m[1],o[1]+m[5],o[2]+m[9],0xff00);
DTri3(o[0],o[1],o[2],0xffffff,
      o[0]+m[1],o[1]+m[5],o[2]+m[9],0xff00,
      o[0]+m[2],o[1]+m[6],o[2]+m[10],0xff0000);
}

//predefined key actions ---------------------------------------------------------------
void DXDefKeyAction(unsigned scancode)
{
DWORD s;
switch(scancode)
 {
 case 82: //ins
  dcam.MoveFB(.5);
  break;
 case 83: //del
  dcam.MoveLR(-.5);
  break;
 case 71: //home
  dcam.MoveFB(-.5);
  break;
 case 79: //end
  dcam.MoveLR(.5);
  break;
 case 73: //pgup
  dcam.MoveUD(.5);
  break;
 case 81: //pgdown
  dcam.MoveUD(-.5);
  break;
 case 72: //up
  dcam.SpinUD(RAD(1));
  break;
 case 75: //left
  dcam.SpinLR(-RAD(1));
  break;
 case 77: //right
  dcam.SpinLR(RAD(1));
  break;
 case 80: //down
  dcam.SpinUD(-RAD(1));
  break;
 case 76: //5 (pad)
  dcam.Init();
  break;
//states 
 case 30: //a
  dxdev->GetRenderState(D3DRS_ALPHATESTENABLE,&s);
  dxdev->SetRenderState(D3DRS_ALPHATESTENABLE,!s);
  break;
 case 48: //b
  dxdev->GetRenderState(D3DRS_ALPHABLENDENABLE,&s);
  dxdev->SetRenderState(D3DRS_ALPHABLENDENABLE,!s);
  break;
 case 33: //f
  dxdev->GetRenderState(D3DRS_FOGENABLE,&s);
  dxdev->SetRenderState(D3DRS_FOGENABLE,!s);
  break;
 case 50: //m
  dxdev->GetTextureStageState(0,D3DTSS_MIPFILTER,&s);
  dxdev->SetTextureStageState(0,D3DTSS_MIPFILTER,(s+1)%3);
  break;
 case 20: //t
  dxdev->GetTextureStageState(0,D3DTSS_COLOROP,&s);
  dxdev->SetTextureStageState(0,D3DTSS_COLOROP,1+s%11);
  break;
 case 21: //y
  dxdev->GetTextureStageState(1,D3DTSS_COLOROP,&s);
  dxdev->SetTextureStageState(1,D3DTSS_COLOROP,1+s%11);
  break;
 case 46: //c
  dxdev->GetRenderState(D3DRS_CULLMODE,&s);
  dxdev->SetRenderState(D3DRS_CULLMODE,1+s%3);
  break;
 case 17: //w
  dxdev->GetRenderState(D3DRS_FILLMODE,&s);
  dxdev->SetRenderState(D3DRS_FILLMODE,1+s%3);
  break;
 case 34: //g
  dxdev->GetRenderState(D3DRS_SHADEMODE,&s);
  dxdev->SetRenderState(D3DRS_SHADEMODE,1+s%2);
  break;
 case 35: //h
  dxdev->GetRenderState(D3DRS_SPECULARENABLE,&s);
  dxdev->SetRenderState(D3DRS_SPECULARENABLE,!s);
  break;
 case 37: //k
  dxdev->GetRenderState(D3DRS_COLORVERTEX,&s);
  dxdev->SetRenderState(D3DRS_COLORVERTEX,!s);
  break;
 case 38: //l
  dxdev->GetRenderState(D3DRS_LIGHTING,&s);
  dxdev->SetRenderState(D3DRS_LIGHTING,!s);
  break;
 case 32: //d
  dxdev->GetRenderState(D3DRS_ZENABLE,&s);
  dxdev->SetRenderState(D3DRS_ZENABLE,(s+1)%3);
  break;
 case 18: //e
  dxdev->GetRenderState(D3DRS_ZWRITEENABLE,&s);
  dxdev->SetRenderState(D3DRS_ZWRITEENABLE,!s);
  break;
//dialogs 
 case 19: //r
  printbox("%u %u %ux%u %u=%x",mww,mwh,scrw,scrh,scrbpp,scrpf);
  break;
 }
if(scancode>=71&&scancode<=83) dcam.On();
}

#endif