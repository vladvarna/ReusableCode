#ifndef V_OPENGLTOOLS
#define V_OPENGLTOOLS

#include <3D/ogl.cpp>
#include <ext/glu.h> //needs: glu32.lib
#pragma comment(lib,"glu32.lib")
#include <mmed.cpp>
#include <img.cpp>
#include <3D/mesh.cpp>
#include <rnd.cpp>

class GLFont
{
public:
 GLuint fdl; //font display list
 char stf,nrf; //first,last letter
 
 void Use3D(float tz=0,int bold=0,int ital=0,LPSTR fntn=NULL,float qual=0,char a=32,char z=127);
 void Use2D(int tx=0,int ty=0,int bold=0,int ital=0,LPSTR fntn=NULL,char a=32,char z=127);
 void Print(LPSTR str=NULL);
 void Free();
};

class GLBeam
{
public:
 float *ps;
 unsigned nrp,P;
 float R,G,B,Ai,Ao;
 float raz,fade;

 void Init(unsigned pcnt=2,unsigned psz=3);
 void Prop(float lat=.1,float r=1,float g=1,float b=1,float ai=1,float ao=0,float amod=0);
 void Line(unsigned p=0,float x1=0,float y1=0,float z1=0,float x2=0,float y2=0,float z2=0);
 void RndLin(unsigned p=0,float rep=1,float dx=0,float dy=0,float dz=0,
             float x1=0,float y1=0,float z1=0,float x2=0,float y2=0,float z2=0);
 void Draw(unsigned p=0,unsigned pcnt=0);
 void Free();
};
//*****************************************************************************************

//meesage box with a PFD struc ---------------------------------------------------------
void GLDCinfo()
{
int pfi;
PIXELFORMATDESCRIPTOR pfd;
pfi=GetPixelFormat(hmdc);
isZERO(pfi,"Get DC PF");
erret=DescribePixelFormat(hmdc,pfi,sizeof(PIXELFORMATDESCRIPTOR),&pfd);
isZERO(erret,"Descrinbe DC PF");
printbox("PFind %u: %u(%u:%u:%u:%u)(%u:%u:%u:%u), Depth %u, Stencil %u, Accum %u",pfi,pfd.cColorBits,pfd.cAlphaBits,pfd.cRedBits,pfd.cGreenBits,pfd.cBlueBits,pfd.cAlphaShift,pfd.cRedShift,pfd.cGreenShift,pfd.cBlueShift,pfd.cDepthBits,pfd.cStencilBits,pfd.cAccumBits);
}

// GLBeam <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void GLBeam::Init(unsigned pcnt,unsigned psz)
{
nrp=pcnt;
P=psz;
ps=(float*)malloc(nrp*P*4);
}

void GLBeam::Prop(float lat,float r,float g,float b,float ai,float ao,float amod)
{
R=r; G=g; B=b;
Ai=ai; Ao=ao;
fade=amod;
raz=lat;
}

void GLBeam::Line(unsigned p,float x1,float y1,float z1,float x2,float y2,float z2)
{
p*=P;
ps[p]=x1; ps[p+1]=y1; ps[p+2]=z1;
ps[p+3]=x2; ps[p+4]=y2; ps[p+5]=z2;
}

void GLBeam::RndLin(unsigned p,float rep,float dx,float dy,float dz,float x1,float y1,float z1,float x2,float y2,float z2)
{
p*=P;
ps[p]=x1; ps[p+1]=y1; ps[p+2]=z1;
p+=P*rep;
ps[p]=x2; ps[p+1]=y2; ps[p+2]=z2;
x2=(x2-x1)/rep;
y2=(y2-y1)/rep;
z2=(z2-z1)/rep;
rep--;
while(rep>0)
 {
 p-=P;
 ps[p]=x1+rep*x2+RANDf(-dx,dx);
 ps[p+1]=y1+rep*y2+RANDf(-dy,dy); 
 ps[p+2]=z1+rep*z2+RANDf(-dz,dz);
 rep--;
 }
}

void GLBeam::Draw(unsigned p,unsigned pcnt)
{
if(p>=nrp) return;
if(pcnt==0) pcnt=nrp-p;
p*=P;
glPushAttrib(GL_ALL_ATTRIB_BITS);
glDisable(GL_CULL_FACE);
glDisable(GL_TEXTURE_2D);
glDisable(GL_LIGHTING);
glEnable(GL_BLEND);
glBegin(GL_QUADS);
 for(unsigned pi=1;pi<pcnt;pi++)
  {
  glColor4f(R,G,B,Ao); glVertex3f(ps[p]-raz,ps[p+1],ps[p+2]);
  glColor4f(R,G,B,Ao); glVertex3f(ps[p+3]-raz,ps[p+4],ps[p+5]);
  glColor4f(R,G,B,Ai); glVertex3f(ps[p+3],ps[p+4],ps[p+5]);
  glColor4f(R,G,B,Ai); glVertex3f(ps[p],ps[p+1],ps[p+2]);
  glColor4f(R,G,B,Ai); glVertex3f(ps[p],ps[p+1],ps[p+2]);
  glColor4f(R,G,B,Ai); glVertex3f(ps[p+3],ps[p+4],ps[p+5]);
  glColor4f(R,G,B,Ao); glVertex3f(ps[p+3]+raz,ps[p+4],ps[p+5]);
  glColor4f(R,G,B,Ao); glVertex3f(ps[p]+raz,ps[p+1],ps[p+2]);
  p+=P;
  }
glEnd();
glPopAttrib();
Ai-=fade;
Ao-=fade;
}

void GLBeam::Free()
{
if(ps) free(ps);
ps=NULL;
nrp=0;
}
// GLBeam >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// GLFont <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void GLFont::Use3D(float tz,int bold,int ital,LPSTR fntn,float qual,char a,char z)
{
HDC hdc;
HFONT hfnt;
hdc=wglGetCurrentDC();
hfnt=CreateFont(0,0,0,0,bold,ital,FALSE,FALSE,ANSI_CHARSET,OUT_TT_PRECIS,
                CLIP_DEFAULT_PRECIS,PROOF_QUALITY,DEFAULT_PITCH|FF_ROMAN,fntn);
hfnt=(HFONT)SelectObject(hdc,hfnt);
if(fdl) glDeleteLists(fdl,nrf);
stf=a;
nrf=z-a+1;
fdl=glGenLists(nrf);
wglUseFontOutlines(hdc,stf,nrf,fdl,qual,tz,WGL_FONT_POLYGONS,NULL);
DeleteObject(SelectObject(hdc,hfnt));
}

void GLFont::Use2D(int tx,int ty,int bold,int ital,LPSTR fntn,char a,char z)
{
HDC hdc;
HFONT hfnt;
hdc=wglGetCurrentDC();
hfnt=CreateFont(ty,tx,0,0,bold,ital,FALSE,FALSE,ANSI_CHARSET,OUT_TT_PRECIS,
                CLIP_DEFAULT_PRECIS,PROOF_QUALITY,DEFAULT_PITCH|FF_ROMAN,fntn);
hfnt=(HFONT)SelectObject(hdc,hfnt);
if(fdl) glDeleteLists(fdl,nrf);
stf=a;
nrf=z-a+1;
fdl=glGenLists(nrf);
wglUseFontBitmaps(hdc,stf,nrf,fdl);
DeleteObject(SelectObject(hdc,hfnt));
}

void GLFont::Print(LPSTR str)
{
if(!str) return;
glPushAttrib(GL_ALL_ATTRIB_BITS);
glListBase(fdl-stf);
glDisable(GL_TEXTURE_2D);
glEnable(GL_COLOR_MATERIAL);
glCallLists(strlen(str),GL_UNSIGNED_BYTE,str);
glPopAttrib();
}

void GLFont::Free()
{
glDeleteLists(fdl,nrf);
}
// GLFont >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//predefined key actions ---------------------------------------------------------------
void GLDefKeyAction(unsigned scancode)
{
static int side;
if(GetKeyState(VK_SHIFT)&0x8000)
 side=GL_FRONT; 
else if(GetKeyState(VK_CONTROL)&0x8000)
 side=GL_BACK;
else side=GL_FRONT_AND_BACK;
GLint s,ss[2];
switch(scancode)
 {
 case 82: //ins
  glcam.MoveFB(.5);
  break;
 case 83: //del
  glcam.MoveLR(-.5);
  break;
 case 71: //home
  glcam.MoveFB(-.5);
  break;
 case 79: //end
  glcam.MoveLR(.5);
  break;
 case 73: //pgup
  glcam.MoveUD(.5);
  break;
 case 81: //pgdown
  glcam.MoveUD(-.5);
  break;
 case 72: //up
  glcam.SpinUD(RAD(1));
  break;
 case 75: //left
  glcam.SpinLR(-RAD(1));
  break;
 case 77: //right
  glcam.SpinLR(RAD(1));
  break;
 case 80: //down
  glcam.SpinUD(-RAD(1));
  break;
 case 76: //5 (pad)
  glcam.Init();
  break;
//states 
 case 30: //a
  if(glIsEnabled(GL_ALPHA_TEST)) glDisable(GL_ALPHA_TEST);
  else glEnable(GL_ALPHA_TEST);
  break;
 case 48: //b
  if(glIsEnabled(GL_BLEND)) glDisable(GL_BLEND);
  else glEnable(GL_BLEND);
  break;
 case 33: //f
  //fog
  break;
 case 50: //m
  //mip
  break;
 case 20: //t
  if(glIsEnabled(GL_TEXTURE_2D)) glDisable(GL_TEXTURE_2D);
  else glEnable(GL_TEXTURE_2D);
  break;
 case 46: //c
  glCullFace(side);
  if(glIsEnabled(GL_CULL_FACE)) glDisable(GL_CULL_FACE);
  else glEnable(GL_CULL_FACE);
  break;
 case 17: //w
  glGetIntegerv(GL_POLYGON_MODE,ss);
  glPolygonMode(side,ss[side==GL_BACK?1:0]==GL_FILL?GL_LINE:GL_FILL);
  break;
 case 34: //g
  glGetIntegerv(GL_SHADE_MODEL,&s);
  glShadeModel(s==GL_FLAT?GL_SMOOTH:GL_FLAT);
  break;
 case 36: //j
  glGetIntegerv(GL_LIGHT_MODEL_TWO_SIDE,&s);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,!s);
  break;
 case 35: //h
  glGetIntegerv(GL_LIGHT_MODEL_LOCAL_VIEWER,&s);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,!s);
  break;
 case 37: //k
  if(glIsEnabled(GL_COLOR_MATERIAL)) glDisable(GL_COLOR_MATERIAL);
  else glEnable(GL_COLOR_MATERIAL);
  break;
 case 38: //l
  if(glIsEnabled(GL_LIGHTING)) glDisable(GL_LIGHTING);
  else glEnable(GL_LIGHTING);
  break;
 case 32: //d
  if(glIsEnabled(GL_DEPTH_TEST)) glDisable(GL_DEPTH_TEST);
  else glEnable(GL_DEPTH_TEST);
  break;
 case 18: //e
  glGetIntegerv(GL_DEPTH_WRITEMASK,&s);
  glDepthMask(!s);
  break;
//dialogs
 case 19: //r
  printbox("%u %u %ux%u %u=%x",mww,mwh,scrw,scrh,scrbpp,scrpf);
  break;
 case 23: //i
  GLDCinfo();
  break;
 }
if(scancode>=71&&scancode<=83) glcam.On();
}

#endif