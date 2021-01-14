#ifndef V_IOPENGL
#define V_IOPENGL

#include <gdi.cpp>
#include <ext/gl.h> //needs: opengl32.lib
#include <ext/glext.h>
//#include <wglext.h>
#pragma comment(lib,"opengl32.lib")
#include <3D/bas3d.cpp>
#include <3D/3d.cpp>

#define GLTEX GLuint
#define GLIST GLuint

#if defined(GL_VERSION_1_4)
#pragma message("OpenGL 1.4")
#elif defined(GL_VERSION_1_3)
#pragma message("OpenGL 1.3")
#elif defined(GL_VERSION_1_2)
#pragma message("OpenGL 1.2")
#elif defined(GL_VERSION_1_1)
#pragma message("OpenGL 1.1")
#endif
//multitexture
PFNGLACTIVETEXTUREPROC glActiveTexture=NULL;
PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f=NULL;
//transpose matrix
PFNGLLOADTRANSPOSEMATRIXFPROC glLoadTransposeMatrixf=NULL;
PFNGLMULTTRANSPOSEMATRIXFPROC glMultTransposeMatrixf=NULL;

HGLRC glrc=NULL; //OpenGL rendering context

class GLight
{
public:
 float pos[4]; //pos
 float a,b,c; //attenuation
 float spc[4],dif[4],amb[4]; //light's colors
 float dir[3]; //spot dir
 float cut,exp; //spot cutoff, exponent
 int ind;

 void Init(int li=0,float x=0,float y=0,float z=0,float w=1,float a0=1,float a1=0,float a2=0);
 void Cols(float sr=.3,float sg=.3,float sb=.3,float sa=1,float dr=.5,float dg=.5,float db=.5,float da=1,float ar=.2,float ag=.2,float ab=.2,float aa=1);
 void Spot(float x=0,float y=0,float z=-1,float fov=90,float dens=2);
};
//*****************************************************************************************

//creates a 4 floats array from a dword abgr ----------------------------------------------
inline void RGBdwtof(float *frgba,DWORD dwabgr=0)
{
COLOR c;
c.dw=dwabgr;
frgba[0]=(float)c.r/255;
frgba[1]=(float)c.g/255;
frgba[2]=(float)c.b/255;
frgba[3]=(float)c.a/255;
}

//checks the GL error flag ----------------------------------------------------------------
int isGLOk(LPSTR errmsg=NULL)
{
if(glGetError())
 {
 if(MessageBox(hmwnd,errmsg,"OpenGL ERROR",MB_RETRYCANCEL|MB_ICONWARNING)==IDCANCEL)
  exit(1);
 }
return 1;
}

//counts how many of the exts extensions are available -------------------------------------
int GLExtCaps(LPSTR exts)
{
int bof,dof,cnt=0,ord=1;
LPSTR glexts;
glexts=(LPSTR)glGetString(GL_EXTENSIONS);
bof=0;
dof=chinstr('|',exts);
while(bof<dof)
 {
 if(!blkinstr(exts+bof,dof-bof,glexts)) cnt--;
 bof=dof+1;
 dof=chinstr('|',exts,++ord);
 }
return cnt;
}

//macro for obtaining a GL extension functions -----------------------------------------------
PROC GLExt(LPSTR basfn)
{
PROC extfp;
extfp=wglGetProcAddress(basfn);
if(extfp) return extfp;
char extfn[64];
int sl;
sl=strlen(basfn);
CopyMemory(extfn,basfn,sl);
strcpy(extfn+sl,"ARB");
extfp=wglGetProcAddress(extfn);
if(extfp) return extfp;
strcpy(extfn+sl,"EXT");
extfp=wglGetProcAddress(extfn);
if(extfp) return extfp;
return NULL;
}

//returns GL internal texture format from PF ---------------------------------------------
int PFtoGLF1(DWORD pixform)
{
pixform&=0xffff;
if(pixform==0x8888)
	return GL_RGBA8; //32
else if(pixform==0x1555)
	return GL_RGB5_A1; //1+15
else if(pixform==0x4444)
	return GL_RGBA4; //4+12
else if(pixform==0x2222)
	return GL_RGBA2; //2+6
else if(pixform==0x0888)
	return GL_RGB8; //24
else if(pixform==0x0555)
	return GL_RGB5; //15
else if(pixform==0x0444)
	return GL_RGB4; //12
else if(pixform==0x0332)
	return GL_R3_G3_B2; //8
else if(pixform==0x8000)
	return GL_ALPHA8; //8+0
else
	return ALIGN(PF_bpix(pixform),7)>>3; //B/pixel
}

//returns GL internal texture format from PF ---------------------------------------------
int PFtoGLF2(DWORD pixform)
{
if(pixform==0x32108888)
	return GL_BGRA_EXT;
else if(pixform==0x32100888)
	return GL_BGR_EXT;
else if(pixform==0x30128888)
	return GL_RGBA;
else if(pixform==0x30120888)
	return GL_RGB;
pixform&=0xffff;
if(pixform==0x8000)
	return GL_ALPHA;
else
	return 0;
}

// handles texture transformations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct GLTEXTURETF
 {
 float mt[16];
 
 void On()
  {
  glMatrixMode(GL_TEXTURE);
  glLoadMatrixf(mt);
  glMatrixMode(GL_MODELVIEW);
  }
 void Flip(float x=1.0f,float y=-1.0f)
  {
  mt[1]=mt[2]=mt[3]=mt[4]=mt[6]=mt[7]=mt[8]=mt[9]=mt[11]=mt[12]=mt[13]=mt[14]=0;
  mt[0]=x; mt[5]=y;
  mt[10]=mt[15]=1;
  }
 }glttf;

//general viewport format ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct GLVIEWPORT
 {
 long x,y;    //ul corner  |
 DWORD w,h;   //dimensions }D3DVIEWPORT8
 float f,b;   //minz,maxz  |
 float mv[16];//tf matrix

 void On()
  {
  glViewport(x,y,w,h);
  glDepthRange(f,b);
  }
 void Set(int l=0,int u=0,int lng=0,int lat=0,float minz=0,float maxz=1)
  {
  x=l; y=u;
  w=lng; h=lat;
  f=minz; b=maxz;
  M_viewport(mv,x,y,w,h,f,b);
  }
 }glport;
 
// handles projection transformations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct GLPROJECTION
 {
 float mp[16];
 
 void On()
  {
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(mp);
  glMatrixMode(GL_MODELVIEW);
  }
 void Id()
  {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
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
 }glproj;

//general camera ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct GLCAMERA
 {
 float mc[16];
 float Ox,Oy,Oz; //camera origin in world space

 void Init() //set it to null
  {
  Ox=Oy=Oz=0;
  Mnull(mc);
  }
 void On() //set it as GL's view tf
  {
  glLoadMatrixf(mc);
  }
 void Mul() //combine it with GL's view tf
  {
  glMultMatrixf(mc);
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
 }glcam;

//inits the GL RC -------------------------------------------------------------------------
int InitGL(DWORD rendform=0,int width=0,int height=0,DWORD flags=PFD_DOUBLEBUFFER)
{
int pfi;
PIXELFORMATDESCRIPTOR pfd;
ZeroMemory(&pfd,sizeof(pfd));
pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
pfd.nVersion=1;
pfd.dwFlags=PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_SWAP_EXCHANGE|flags;
pfd.iPixelType=PFD_TYPE_RGBA;
pfd.cColorBits=RF_cbpp(rendform);
pfd.cAlphaBits=RF_abpp(rendform);
pfd.cAccumBits=0;
pfd.cDepthBits=RF_dbpp(rendform);
pfd.cStencilBits=RF_sbpp(rendform);
if(!pfd.cDepthBits) pfd.dwFlags|=PFD_DEPTH_DONTCARE;
ModeInfo(scrbpp,scrw,scrh);
if(pfd.cColorBits) //fullscreen
 {
 if(!width) width=scrw;
 if(!height) height=scrh;
 AdjustClientRect(hmwnd,width,height);
 InitMode(pfd.cColorBits,width,height);
 }
else if(width)
 {
 if(!height) height=(width*3)>>2;
 AdjustClientRect(hmwnd,width,height);
 }
hmdc=GetDC(hmwnd); //hmwnd must have CS_OWNDC
pfi=ChoosePixelFormat(hmdc,&pfd);
isZERO(pfi,"No matching DC Pixel Format found");
erret=DescribePixelFormat(hmdc,pfi,sizeof(PIXELFORMATDESCRIPTOR),&pfd);
isZERO(erret,"Chosen DC PF description unavailable");
erret=SetPixelFormat(hmdc,pfi,&pfd);
isZERO(erret,"Set DC Pixel Format");
scrpf=DCPFDtoPF(pfd);
rendform=RF_make(pfd.cStencilBits,pfd.cDepthBits,pfd.cAlphaBits,pfd.cColorBits);
glrc=wglCreateContext(hmdc);
isNULL(glrc,"Create GL Rendering Context");
erret=wglMakeCurrent(hmdc,glrc);
isZERO(erret,"Select GL Rendering Context");
glClearDepth(1);
glDepthFunc(GL_LEQUAL);
if(pfd.cDepthBits) glEnable(GL_DEPTH_TEST);
glAlphaFunc(GL_GREATER,0);
glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//extensions
glActiveTexture=(PFNGLACTIVETEXTUREPROC)GLExt("glActiveTexture");
glMultiTexCoord2f=(PFNGLMULTITEXCOORD2FPROC)GLExt("glMultiTexCoord2f");
glLoadTransposeMatrixf=(PFNGLLOADTRANSPOSEMATRIXFPROC)GLExt("glLoadTransposeMatrixf");
glMultTransposeMatrixf=(PFNGLMULTTRANSPOSEMATRIXFPROC)GLExt("glMultTransposeMatrixf");
return erret;
}

//deletes the GL RC ----------------------------------------------------------------------
void FreeGL()
{
erret=wglMakeCurrent(NULL,NULL);
isZERO(erret,"Deselect GL Rendering Context");
erret=wglDeleteContext(glrc);
glrc=NULL;
isZERO(erret,"Release GL Rendering Context");
ReleaseDC(hmwnd,hmdc);
hmdc=NULL;
InitMode();
}

//transforms a vertex using a view and projection matrix ----------------------------------
inline void GLtf4(float *dest,float *src=NULL,float *matv=NULL,float *matp=NULL)
{
if(src) CopyMemory(dest,src,16);
if(matv) VxM4(dest,matv);
if(matp) VxM4(dest,matp);
dest[0]/=dest[3];
dest[1]/=dest[3];
dest[2]/=dest[3];
}

//sets the current material ---------------------------------------------------------------
void GLmater(float shin=32,DWORD spc=0xffffffff,DWORD dif=0xffffffff,DWORD amb=0xffffffff,
             DWORD ems=0,int side=GL_FRONT_AND_BACK)
{
float fc[4];
RGBdwtof(fc,dif);
glMaterialfv(side,GL_DIFFUSE,fc);
RGBdwtof(fc,amb);
glMaterialfv(side,GL_AMBIENT,fc);
RGBdwtof(fc,ems);
glMaterialfv(side,GL_EMISSION,fc);
RGBdwtof(fc,spc);
glMaterialfv(side,GL_SPECULAR,fc);
glMaterialf(side,GL_SHININESS,shin);
}

//enables GL fog blending -----------------------------------------------------------------
void GLfog(float fbeg=1,float fdist=10,float r=1,float g=1,float b=1,float a=1)
{
glFogi(GL_FOG_MODE,GL_LINEAR);
glFogf(GL_FOG_START,fbeg);
glFogf(GL_FOG_END,fbeg+fdist);
glFogfv(GL_FOG_COLOR,&r);
glClearColor(r,g,b,a);
glEnable(GL_FOG);
}

// GLight <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void GLight::Init(int li,float x,float y,float z,float w,float a0,float a1,float a2)
{
ind=GL_LIGHT0+li;
pos[0]=x; pos[1]=y; pos[2]=z; pos[3]=w;
a=a2; b=a1; c=a0;
glLightfv(ind,GL_POSITION,pos);
/*
glLightf(ind,GL_CONSTANT_ATTENUATION,c);
glLightf(ind,GL_LINEAR_ATTENUATION,b);
glLightf(ind,GL_QUADRATIC_ATTENUATION,a);
*/
glEnable(ind);
}

void GLight::Spot(float x,float y,float z,float fov,float dens)
{
dir[0]=x; dir[1]=y; dir[2]=z;
cut=fov;
exp=dens;
glLightfv(ind,GL_SPOT_DIRECTION,dir);
glLightf(ind,GL_SPOT_CUTOFF,cut);
glLightf(ind,GL_SPOT_EXPONENT,exp);
}

void GLight::Cols(float sr,float sg,float sb,float sa,float dr,float dg,float db,float da,float ar,float ag,float ab,float aa)
{
CopyMemory(spc,&sr,48);
glLightfv(ind,GL_SPECULAR,spc);
glLightfv(ind,GL_DIFFUSE,dif);
glLightfv(ind,GL_AMBIENT,amb);
}
// GLight >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#endif