#ifndef V_SHAPES
#define V_SHAPES

#include <3D/3d.cpp>
#include <rgb.cpp>
#include <rnd.cpp>

#define NRV_SFER(h,v) (h*(v*2-1)*6)
#define NRV_CAL_C(h,v) (h*v*6)
#define NRV_CAL(h,v) (h*(v*2-1)*3)
#define NRV_TR(n) (n*6)
#define NRV_TR_C(n) (n*12)
#define NRV_PIR(n) (n*3)
#define NRV_PIR_C(n) (n*6)
#define NRV_BOX 36

#define S_LIST     0x1
#define S_STRIP    0x2
#define S_FAN      0x4
#define S_LOOP     0x8
#define S_N_NORM   0x10 //auto normalization of normals
#define S_N_ORIG   0x20 //pt sfere
#define S_N_FLAT   0x40
#define S_N_OPUS   0x80
#define S_T_RL     0x100
#define S_T_DU     0x200
#define S_T_LIN    0x400 //texture is mapped so if seen on z looks like in bmp
#define S_T_CURB   0x800 //for sphres corect mapping and 'sliced' mapping for tr and pir
#define S_T_YX     0x1000 //rotates texture 90 deg
#define S_T_CENTER 0x2000 //centers tiled textures
#define S_C_GREY   0x4000 //greys all colors
#define S_C_RND    0x8000 //generates random colors
#define S_FLIP     0x10000 //flips caps horiz and vert
#define S_MIROR    0x20000 //flips caps only horiz
#define S_CLOSE    0x40000 //will generate a closed shape (+caps)
#define S_SIM      0x80000
#define S_N_DEF    0x100000
#define S_C_DEF    0x200000
#define S_CORECT   0x400000
#define S_APROX    0x800000
#define S_RHAND    0x1000000 //toggles the CW/CCW order
#define S_C_BGR    0x2000000 //swaps R and B
#define S_C_NOT    0x4000000 //inverts colors
#define S_T_GEN    0x8000000 //generates texture coordinates from positional X,y
#define S_N_AUTO   0x10000000 //automaticaly calls Normals, Coords and Colors from Vertex
#define S_T_AUTO   0x20000000
#define S_C_AUTO   0x40000000
#define S_N_REV    0x80000000 //reversed flat

class Shape
{
public:
 float *sb;
 WORD *ib;
 unsigned V,N,T,C,I;
 unsigned mod;
 float htile,vtile;
 float tx,ty,tz;
 float Dnorm[3];
 COLOR Dcol;

 void Init(NAT model=0,NAT ov=0,NAT on=0,NAT ot=0,NAT oc=0,float *sbuf=NULL,WORD *indb=NULL,NAT isz=2);
 void Vertex(NAT vi=0,float x=0,float y=0,float z=0);
 void Normal(NAT vi=0,float x=0,float y=0,float z=0,float w=1);
 void Coords(NAT vi=0,float x=0,float y=0);
 void Colors(NAT vi=0,DWORD cdw=0);
 //void Tri(NAT,NAT,NAT);
 void Copy(NAT vdest=0,NAT vsrc=0,NAT cnt=0);
 void CopyTo(float *sbuf,NAT ov=9,NAT on=3,NAT ot=7,NAT oc=6,NAT dt=0,NAT vi=0,NAT vcnt=3);

 void NFlat(NAT vi=0,NAT tcnt=1);
 void NRot(NAT vi=0,NAT vcnt=3,float rx=0,float ry=0,float rz=0);
 void Tslide(NAT vi=0,NAT tcnt=1,float du=0,float dv=0);
 void Ttile(NAT vi=0,NAT tcnt=1,float ht=1,float vt=1);
 void Cmono(NAT vi=0,NAT vcnt=3,DWORD cul=0xf0f0f0);
 void Crand(NAT vi=0,NAT vcnt=3,DWORD min=0,DWORD max=0xffffff,int mono=0);
 void tf(NAT vi=0,NAT vcnt=1,float *mtx=NULL);
 
 unsigned Rect(NAT vi=0,float lx=0,float ly=0,float lz=0);
 unsigned Box(float x1=0,float x2=0,float y1=0,float y2=0,float z1=0,float z2=0);
 unsigned Box1(float x1=0,float x2=0,float y1=0,float y2=0,float z1=0,float z2=0);

 unsigned Calota(float r=1,int n=3,int m=1,float z=1);
 unsigned Sfera(float r=1,int n=6,int m=3);
 unsigned Cerc(float r=1,int n=6);
 
 unsigned Trunc(float h=1,float r1=1,float r2=1,int n=3);
 unsigned Cub(float l=1);
 
 unsigned Piram(float h=1,float r=1,int n=3);
 unsigned Polig(float r=1,int n=3);
};
//*****************************************************************************************

// Shape <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//should be called before any other function
void Shape::Init(NAT model,NAT ov,NAT on,NAT ot,NAT oc,float *sbuf,WORD *indb,NAT isz)
{
mod=model;
V=ov;
N=on;
T=ot;
C=oc;
if(sbuf) sb=sbuf;
htile=vtile=1;
tx=ty=tz=0;
ib=indb;
I=isz;
}

void Shape::CopyTo(float *sbuf,NAT ov,NAT on,NAT ot,NAT oc,NAT dt,NAT vi,NAT vcnt)
{
vi*=V;
for(int vc=0;vc<vcnt;vc++)
 {
 CopyMemory(sbuf,sb+vi,12); //vertex
 if(N&&on) CopyMemory(sbuf+on,sb+vi+N,12); //norms
 if(T&&ot) CopyMemory(sbuf+ot,sb+vi+T,8); //tex coords
 if(C&&oc) CopyMemory(sbuf+oc,sb+vi+C,4); //color
 vi+=V;
 sbuf+=ov;
 if(vc%3==2) 
  sbuf+=dt;//extra space between tris (from the end of v3 to begining of v1)
 }
}

void Shape::Vertex(NAT vi,float x,float y,float z)
{
sb[vi*V]=x+tx;
sb[vi*V+1]=y+ty;
sb[vi*V+2]=z+tz;
if(mod&S_N_AUTO) Normal(vi,x,y,z,1);
if(mod&S_T_AUTO) Coords(vi,x,y);
if(mod&S_C_AUTO) Colors(vi,0xf0f0f0f0);
}
 
void Shape::Normal(NAT vi,float x,float y,float z,float w)
{
if(!N) return;
if(mod&S_N_FLAT) //use 3 adj verts to compute normal
 {
 vi-=vi%3;
 Tnorm(&sb[vi*V+N],&sb[vi*V],&sb[(vi+1)*V],&sb[(vi+2)*V]);
 Tnorm(&sb[(vi+1)*V+N],&sb[vi*V],&sb[(vi+1)*V],&sb[(vi+2)*V]);
 Tnorm(&sb[(vi+2)*V+N],&sb[vi*V],&sb[(vi+1)*V],&sb[(vi+2)*V]);
 if(mod&S_N_REV)
  {
  Vopus(&sb[vi*V+N]);
  Vopus(&sb[(vi+1)*V+N]);
  Vopus(&sb[(vi+2)*V+N]);
  }
 return;
 }
vi*=V;
vi+=N;
if(mod&S_N_ORIG)
 {
 if(w==0) return; //division by zero safe
 if(w<0) w=-w; //magnitude is modulus
 sb[vi]=sb[vi-N]/w;
 sb[vi+1]=sb[vi-N+1]/w;
 sb[vi+2]=sb[vi-N+2]/w;
 }
else if(mod&S_N_DEF) //use defaults
 {
 sb[vi]=Dnorm[0];
 sb[vi+1]=Dnorm[1];
 sb[vi+2]=Dnorm[2];
 }
else 
 {
 sb[vi]=x;
 sb[vi+1]=y;
 sb[vi+2]=z;
 if(mod&S_N_NORM) Vunit(&sb[vi]);
 }
if(mod&S_N_OPUS)
 {
 sb[vi]=-sb[vi];
 sb[vi+1]=-sb[vi+1];
 sb[vi+2]=-sb[vi+2];
 }
}

void Shape::Coords(NAT vi,float x,float y)
{
if(!T) return;
vi*=V;
if(mod&S_T_GEN)
 {
 x=sb[vi];
 y=sb[vi+1];
 }
vi+=T;
if(mod&S_T_YX)
 {
 sb[vi]=y;
 sb[vi+1]=x;
 }
else
 {
 sb[vi]=x;
 sb[vi+1]=y;
 }
if(mod&S_T_RL) sb[vi]=1-sb[vi];
if(mod&S_T_DU) sb[vi+1]=1-sb[vi+1];
sb[vi]*=htile;
sb[vi+1]*=vtile;
if(mod&S_T_CENTER)
 {
 sb[vi]+=.5-htile*.5;
 sb[vi+1]+=.5-vtile*.5;
 }
}

void Shape::Colors(NAT vi,DWORD cdw)
{
if(!C) return;
COLOR c;
vi*=V;
vi+=C;
if(mod&S_C_RND)
 {
 c.a=0xff;
 c.r=RND(255);
 c.g=RND(255);
 c.b=RND(255);
 }
else if(mod&S_C_DEF) c.dw=Dcol.dw;
else c.dw=cdw;
if(mod&S_C_NOT) c.dw=notRGB(c.dw);
if(mod&S_C_BGR) c.dw=RGB_BGR(c.dw);
if(mod&S_C_GREY) c.r=c.g=c.b=(c.r+c.g+c.b)/3;
*(sb+vi)=c.f;
}

void Shape::Copy(NAT vdest,NAT vsrc,NAT cnt)
{
if(cnt==0) cnt=V;
cnt<<=2;
vdest*=V;
vsrc*=V;
CopyMemory(sb+vdest,sb+vsrc,cnt);
}

void Shape::NFlat(NAT vi,NAT tcnt)
{
if(!N) return;
vi*=V;
mod|=S_N_NORM;
for(unsigned t=0;t<tcnt;t++)
 {
 Tnorm(&sb[vi+N],&sb[vi],&sb[vi+V],&sb[vi+V*2]);
 Normal(vi/V+1,sb[vi+N],sb[vi+N+1],sb[vi+N+2]);
 Normal(vi/V+2,sb[vi+N],sb[vi+N+1],sb[vi+N+2]);
 vi+=V*3;
 }
}

void Shape::NRot(NAT vi,NAT vcnt,float rx,float ry,float rz)
{
if(!N) return;
vi*=V;
mod|=S_N_NORM;
for(NAT v=0;v<vcnt;v++)
 {
 if(rx) VspinX(&sb[vi+N],rx);
 if(ry) VspinY(&sb[vi+N],ry);
 if(rz) VspinZ(&sb[vi+N],rz);
 vi+=V;
 }
}

void Shape::Cmono(NAT vi,NAT vcnt,DWORD cul)
{
if(!C) return;
COLOR c;
c.dw=cul;
vi*=V;
vi+=C;
while(vcnt>0)
 {
 *(sb+vi)=c.f;
 vi+=V;
 vcnt--;
 }
}

void Shape::Crand(NAT vi,NAT vcnt,DWORD min,DWORD max,int mono)
{
if(!C) return;
COLOR minc,maxc,c;
minc.dw=min;
maxc.dw=max;
c.a=0xff;
vi*=V;
vi+=C;
for(NAT v=0;v<vcnt;v++)
 {
 if(mono==0)
  {
  c.r=RAND(minc.r,maxc.r);
  c.g=RAND(minc.g,maxc.g);
  c.b=RAND(minc.b,maxc.b);
  }
 else
  c.r=c.g=c.b=RAND(minc.r,maxc.r);
 *(sb+vi)=c.f;
 vi+=V;
 }
}
 
//3d rectangle (0,0,0)-(lx,ly,lz) ---------------------------------------------------------
unsigned Shape::Rect(NAT vi,float lx,float ly,float lz)
{
mod|=S_LIST;
mod|=S_N_FLAT;
Vertex(vi,0,0,0); Normal(vi); Coords(vi,0,0);
(mod&S_RHAND)?vi++:vi+=2;//CW,CCW
if(lx==0)
 {
 Vertex(vi,0,0,lz); Normal(vi); Coords(vi,1,0);
 (mod&S_RHAND)?vi++:vi--;//CW,CCW
 Vertex(vi,0,ly,0); Normal(vi); Coords(vi,0,1);
 (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
 Vertex(vi,0,ly,0); Normal(vi); Coords(vi,0,1);
 (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
 Vertex(vi,0,0,lz); Normal(vi); Coords(vi,1,0);
 (mod&S_RHAND)?vi++:vi--;//CW,CCW
 }
if(ly==0)
 {
 Vertex(vi,lx,0,0); Normal(vi); Coords(vi,1,0);
 (mod&S_RHAND)?vi++:vi--;//CW,CCW
 Vertex(vi,0,0,lz); Normal(vi); Coords(vi,0,1);
 (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
 Vertex(vi,0,0,lz); Normal(vi); Coords(vi,0,1);
 (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
 Vertex(vi,lx,0,0); Normal(vi); Coords(vi,1,0);
 (mod&S_RHAND)?vi++:vi--;//CW,CCW
 }
if(lz==0)
 {
 Vertex(vi,0,ly,0); Normal(vi); Coords(vi,1,0);
 (mod&S_RHAND)?vi++:vi--;//CW,CCW
 Vertex(vi,lx,0,0); Normal(vi); Coords(vi,0,1);
 (mod&S_RHAND)?vi++:vi+=2;//CW,CCW          
 Vertex(vi,lx,0,0); Normal(vi); Coords(vi,0,1);
 (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
 Vertex(vi,0,ly,0); Normal(vi); Coords(vi,1,0);
 (mod&S_RHAND)?vi++:vi--;//CW,CCW
 }
Vertex(vi,lx,ly,lz); Normal(vi); Coords(vi,1,1);
(mod&S_RHAND)?vi++:vi+=2;//CW,CCW
return 6;
}

unsigned Shape::Box(float x1,float x2,float y1,float y2,float z1,float z2)
{
NAT vi=0;
tx=x1; ty=y1; tz=z1;
x2-=x1; y2-=y1; z2-=z1;
vi+=Rect(vi,0,y2,z2);
vi+=Rect(vi,x2,0,z2);
if(mod&S_CLOSE) mod^=S_T_YX|S_T_RL;
vi+=Rect(vi,x2,y2,0);
mod^=S_RHAND;
if(mod&S_CLOSE) mod^=S_T_YX|S_T_RL;
if(mod&S_MIROR) mod^=S_T_RL;
if(mod&S_FLIP) mod^=S_T_RL|S_T_DU;
tx+=x2;
vi+=Rect(vi,0,y2,z2);
tx=x1; ty+=y2;
vi+=Rect(vi,x2,0,z2);
ty=y1; tz+=z2;
if(mod&S_CLOSE) mod^=S_T_YX|S_T_RL;
vi+=Rect(vi,x2,y2,0);
return vi;
}


unsigned Shape::Box1(float x1,float x2,float y1,float y2,float z1,float z2)
{
NAT vi=0;
//front
Vertex(vi,x1,y1,z1); Normal(vi); Coords(vi,0,0); Colors(vi++,C_WHITE);
Vertex(vi,x1,y2,z1); Normal(vi); Coords(vi,0,1); Colors(vi++,C_BLUE);
Vertex(vi,x2,y1,z1); Normal(vi); Coords(vi,1,0); Colors(vi++,C_RED);
Vertex(vi,x2,y2,z1); Normal(vi); Coords(vi,1,1); Colors(vi++,C_MAGENTA);
Vertex(vi,x2,y1,z1); Normal(vi); Coords(vi,1,0); Colors(vi++,C_RED);
Vertex(vi,x1,y2,z1); Normal(vi); Coords(vi,0,1); Colors(vi++,C_BLUE);
if(mod&S_MIROR) mod^=S_T_RL;
//back
Vertex(vi,x1,y1,z2); Normal(vi); Coords(vi,0,0); Colors(vi++,C_GREEN);
Vertex(vi,x2,y1,z2); Normal(vi); Coords(vi,1,0); Colors(vi++,C_YELLOW);
Vertex(vi,x1,y2,z2); Normal(vi); Coords(vi,0,1); Colors(vi++,C_CYAN);
Vertex(vi,x2,y2,z2); Normal(vi); Coords(vi,1,1); Colors(vi++,C_BLACK);
Vertex(vi,x1,y2,z2); Normal(vi); Coords(vi,0,1); Colors(vi++,C_CYAN);
Vertex(vi,x2,y1,z2); Normal(vi); Coords(vi,1,0); Colors(vi++,C_YELLOW);
//bottom
Vertex(vi,x1,y1,z1); Normal(vi); Coords(vi,0,0); Colors(vi++,C_WHITE);
Vertex(vi,x2,y1,z1); Normal(vi); Coords(vi,0,1); Colors(vi++,C_RED);
Vertex(vi,x1,y1,z2); Normal(vi); Coords(vi,1,0); Colors(vi++,C_GREEN);
Vertex(vi,x2,y1,z2); Normal(vi); Coords(vi,1,1); Colors(vi++,C_YELLOW);
Vertex(vi,x1,y1,z2); Normal(vi); Coords(vi,1,0); Colors(vi++,C_GREEN);
Vertex(vi,x2,y1,z1); Normal(vi); Coords(vi,0,1); Colors(vi++,C_RED);
if(mod&S_MIROR) mod^=S_T_RL;
//top
Vertex(vi,x1,y2,z1); Normal(vi); Coords(vi,0,0); Colors(vi++,C_BLUE);
Vertex(vi,x1,y2,z2); Normal(vi); Coords(vi,1,0); Colors(vi++,C_CYAN);
Vertex(vi,x2,y2,z1); Normal(vi); Coords(vi,0,1); Colors(vi++,C_MAGENTA);
Vertex(vi,x2,y2,z2); Normal(vi); Coords(vi,1,1); Colors(vi++,C_BLACK);
Vertex(vi,x2,y2,z1); Normal(vi); Coords(vi,0,1); Colors(vi++,C_MAGENTA);
Vertex(vi,x1,y2,z2); Normal(vi); Coords(vi,1,0); Colors(vi++,C_CYAN);
//left
Vertex(vi,x1,y1,z1); Normal(vi); Coords(vi,0,0); Colors(vi++,C_WHITE);
Vertex(vi,x1,y1,z2); Normal(vi); Coords(vi,0,1); Colors(vi++,C_GREEN);
Vertex(vi,x1,y2,z1); Normal(vi); Coords(vi,1,0); Colors(vi++,C_BLUE);
Vertex(vi,x1,y2,z2); Normal(vi); Coords(vi,1,1); Colors(vi++,C_CYAN);
Vertex(vi,x1,y2,z1); Normal(vi); Coords(vi,1,0); Colors(vi++,C_BLUE);
Vertex(vi,x1,y1,z2); Normal(vi); Coords(vi,0,1); Colors(vi++,C_GREEN);
if(mod&S_MIROR) mod^=S_T_RL;
//rigth
Vertex(vi,x2,y1,z1); Normal(vi); Coords(vi,0,0); Colors(vi++,C_RED);
Vertex(vi,x2,y2,z1); Normal(vi); Coords(vi,1,0); Colors(vi++,C_MAGENTA);
Vertex(vi,x2,y1,z2); Normal(vi); Coords(vi,0,1); Colors(vi++,C_YELLOW);
Vertex(vi,x2,y2,z2); Normal(vi); Coords(vi,1,1); Colors(vi++,C_BLACK);
Vertex(vi,x2,y1,z2); Normal(vi); Coords(vi,0,1); Colors(vi++,C_YELLOW);
Vertex(vi,x2,y2,z1); Normal(vi); Coords(vi,1,0); Colors(vi++,C_MAGENTA);
return vi;
}

//generates a 3D semi sphere (n side horiz, m*2 sided vert) -------------------------------
unsigned Shape::Calota(float r,int n,int m,float z)
{
if(n<3) n=3;
if(m<1) m=1;
mod|=S_LIST;
mod|=S_N_ORIG;
if(mod&S_RHAND) mod^=S_N_OPUS;
NAT vi=0;
float r1,r2,p1,p2;
float s1=0,c1=1,s2,c2;
for(int ncnt=1;ncnt<=n;ncnt++)
 {
 c2=cos((float)ncnt*_2PI/n);
 s2=sin((float)ncnt*_2PI/n);
 r1=r*sin((float)PI2/m);
 if(mod&S_T_LIN) p1=sin((float)PI2/m);
 else p1=(float)1.0f/m;
 Vertex(vi,0,0,z*r); Normal(vi,0,0,z,r); Coords(vi,.5,.5);
 (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
 Vertex(vi,c2*r1,s2*r1,sqrt(r*r-r1*r1)*z); Normal(vi,0,0,z,r); Coords(vi,.5*(1-c2*p1),.5*(1-s2*p1)); 
 (mod&S_RHAND)?vi++:vi--;//CW,CCW
 Vertex(vi,c1*r1,s1*r1,sqrt(r*r-r1*r1)*z); Normal(vi,0,0,z,r); Coords(vi,.5*(1-c1*p1),.5*(1-s1*p1)); 
 (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
 for(int mcnt=2;mcnt<=m;mcnt++)
  {
  r2=r*sin((float)mcnt*PI2/m);
  if(mod&S_T_LIN) p2=sin((float)mcnt*PI2/m);
  else p2=(float)mcnt/m;
  Vertex(vi,c1*r1,s1*r1,sqrt(r*r-r1*r1)*z); Normal(vi,0,0,z,r); Coords(vi,.5*(1-c1*p1),.5*(1-s1*p1)); 
  (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
  Vertex(vi,c2*r1,s2*r1,sqrt(r*r-r1*r1)*z); Normal(vi,0,0,z,r); Coords(vi,.5*(1-c2*p1),.5*(1-s2*p1)); 
  (mod&S_RHAND)?vi++:vi--;//CW,CCW
  Vertex(vi,c1*r2,s1*r2,sqrt(r*r-r2*r2)*z); Normal(vi,0,0,z,r); Coords(vi,.5*(1-c1*p2),.5*(1-s1*p2)); 
  (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
  Vertex(vi,c2*r2,s2*r2,sqrt(r*r-r2*r2)*z); Normal(vi,0,0,z,r); Coords(vi,.5*(1-c2*p2),.5*(1-s2*p2)); 
  (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
  Vertex(vi,c1*r2,s1*r2,sqrt(r*r-r2*r2)*z); Normal(vi,0,0,z,r); Coords(vi,.5*(1-c1*p2),.5*(1-s1*p2)); 
  (mod&S_RHAND)?vi++:vi--;//CW,CCW
  Vertex(vi,c2*r1,s2*r1,sqrt(r*r-r1*r1)*z); Normal(vi,0,0,z,r); Coords(vi,.5*(1-c2*p1),.5*(1-s2*p1)); 
  (mod&S_RHAND)?vi++:vi+=2;//CW,CCW
  r1=r2;            
  p1=p2;
  }
 s1=s2;
 c1=c2;
 }
if(mod&S_CLOSE) 
 {
 mod&=~S_CLOSE;
 mod|=S_T_LIN;
 mod|=S_N_FLAT;
 mod^=S_RHAND;
 if(mod&S_N_OPUS) mod|=S_N_REV;
 htile=vtile=1;
 if(mod&S_MIROR) mod^=S_T_RL;
 if(mod&S_FLIP) mod^=S_T_RL|S_T_DU;
 sb+=V*vi;
 vi+=Calota(r,n,1,0);
 }
return vi;
}

//macro for creating a sphere from 2 calots -----------------------------------------------
unsigned Shape::Sfera(float r,int n,int m)
{
NAT vi;
mod&=~S_CLOSE;
vi=Calota(r,n,m,1);
mod^=S_RHAND;
if(mod&S_RHAND) mod^=S_N_OPUS;
if(mod&S_MIROR) mod^=S_T_RL;
if(mod&S_FLIP) mod^=S_T_RL|S_T_DU;
sb+=V*vi;
vi+=Calota(r,n,m,-1);
return vi;
}

//macro for creating a circle from a flatten calot ----------------------------------------
unsigned Shape::Cerc(float r,int n)
{
mod|=S_N_FLAT;
return Calota(r,n,1,0);
}

unsigned Shape::Trunc(float h,float r1,float r2,int n)
{
if(n<3) n=3;
mod|=S_LIST;
mod|=S_N_NORM;
NAT vi=0,ncnt;
float s1=0,c1=1,s2,c2;
float p1=0,p2,q1,q2;
float z=1;
if(h) z=(r1-r2)/h;
if(mod&S_T_CURB)
 {
 if(r2<r1) 
  {
  q1=0;
  q2=(r1-r2)/r1*.5/n;
  }
 else 
  {
  q1=(r2-r1)/r2*.5/n;
  q2=0;
  }
 }
else
 {
 if(r2<r1) 
  {
  q1=1;
  q2=r2/r1;
  }
 else 
  {
  q1=r1/r2;
  q2=1;
  }
 }
for(ncnt=1;ncnt<=n;ncnt++)
 {
 c2=cos((float)ncnt*_2PI/n);
 s2=sin((float)ncnt*_2PI/n);
 p2=(float)ncnt/n;
 Vertex(vi,c1*r1,s1*r1,0); Normal(vi,c1,s1,z); 
 (mod&S_T_CURB)?Coords(vi,p1+q1,0):Coords(vi,(1-c1*q1)*.5,(1-s1*q1)*.5);
 (mod&S_RHAND)?vi+=2:vi++;//CCW,CW
 Vertex(vi,c2*r1,s2*r1,0); Normal(vi,c2,s2,z);
 (mod&S_T_CURB)?Coords(vi,p2-q1,0):Coords(vi,(1-c2*q1)*.5,(1-s2*q1)*.5);
 (mod&S_RHAND)?vi--:vi++;//CCW,CW
 Vertex(vi,c1*r2,s1*r2,h); Normal(vi,c1,s1,z);
 (mod&S_T_CURB)?Coords(vi,p1+q2,1):Coords(vi,(1-c1*q2)*.5,(1-s1*q2)*.5);
 (mod&S_RHAND)?vi+=2:vi++;//CCW,CW
 Vertex(vi,c2*r1,s2*r1,0); Normal(vi,c2,s2,z);
 (mod&S_T_CURB)?Coords(vi,p2-q1,0):Coords(vi,(1-c2*q1)*.5,(1-s2*q1)*.5);
 (mod&S_RHAND)?vi+=2:vi++;//CCW,CW
 Vertex(vi,c2*r2,s2*r2,h); Normal(vi,c2,s2,z);
 (mod&S_T_CURB)?Coords(vi,p2-q2,1):Coords(vi,(1-c2*q2)*.5,(1-s2*q2)*.5);
 (mod&S_RHAND)?vi--:vi++;//CCW,CW
 Vertex(vi,c1*r2,s1*r2,h); Normal(vi,c1,s1,z);
 (mod&S_T_CURB)?Coords(vi,p1+q2,1):Coords(vi,(1-c1*q2)*.5,(1-s1*q2)*.5);
 (mod&S_RHAND)?vi+=2:vi++;//CCW,CW
 c1=c2;
 s1=s2;
 p1=p2;
 }
if(mod&S_CLOSE) 
 {
 mod&=~S_CLOSE;
 htile=vtile=1;
 mod|=S_N_FLAT;
 mod|=S_T_CENTER;
 mod^=S_RHAND;
 if(!(mod&S_T_CURB)) htile=vtile=q1;
 if(mod&S_MIROR) mod^=S_T_RL;
 if(mod&S_FLIP) mod^=S_T_RL|S_T_DU;
 sb+=V*vi;
 ncnt=Piram(0,r1,n);
 vi+=ncnt;
 mod^=S_RHAND;
 tz+=h;
 if(!(mod&S_T_CURB)) htile=vtile=q2;
 if(mod&S_MIROR) mod^=S_T_RL;
 if(mod&S_FLIP) mod^=S_T_RL|S_T_DU;
 sb+=ncnt*V;
 vi+=Piram(0,r2,n);
 }
return vi;
}

unsigned Shape::Cub(float l)
{
mod|=S_N_FLAT|S_T_CURB;
htile=4;
return Trunc(l,l*.7071,l*.7071,4);
}

unsigned Shape::Piram(float h,float r,int n)
{
if(n<3) n=3;
mod|=S_LIST;
mod|=S_N_NORM;
NAT vi=0,ncnt;
float s1=0,c1=1,s2,c2;
float p1=0,p2;
float z=1;
if(h) z=r/h;
for(ncnt=1;ncnt<=n;ncnt++)
 {
 c2=cos((float)ncnt*_2PI/n);
 s2=sin((float)ncnt*_2PI/n);
 p2=(float)ncnt/n;
 Vertex(vi,0,0,h); Normal(vi,(c1+c2)/2,(s1+s2)/2,z); 
 (mod&S_T_CURB)?Coords(vi,(p2+p1)/2,0):Coords(vi,.5,.5);
 (mod&S_RHAND)?vi+=2:vi++;//CCW,CW
 Vertex(vi,c1*r,s1*r,0); Normal(vi,c1,s1,z);
 (mod&S_T_CURB)?Coords(vi,p1,1):Coords(vi,(1-c1)*.5,(1-s1)*.5);
 (mod&S_RHAND)?vi--:vi++;//CCW,CW
 Vertex(vi,c2*r,s2*r,0); Normal(vi,c2,s2,z);
 (mod&S_T_CURB)?Coords(vi,p2,1):Coords(vi,(1-c2)*.5,(1-s2)*.5);
 (mod&S_RHAND)?vi+=2:vi++;//CCW,CW
 c1=c2;
 s1=s2;
 p1=p2;
 }
if(mod&S_CLOSE) 
 {
 mod&=~S_CLOSE;
 mod|=S_N_FLAT;
 mod^=S_RHAND;
 htile=vtile=1;
 if(mod&S_MIROR) mod^=S_T_RL;
 if(mod&S_FLIP) mod^=S_T_RL|S_T_DU;
 sb+=V*vi;
 vi+=Piram(0,r,n);
 }
return vi;
}

unsigned Shape::Polig(float r,int n)
{
mod|=S_N_FLAT;
mod&=~S_T_CURB;
return Piram(0,r,n);
}
// Shape >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif