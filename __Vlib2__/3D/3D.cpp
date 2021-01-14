#ifndef V_W3DBASIC
#define V_W3DBASIC

#include <mat.cpp>

#define VNTC_DW(v,n,t,c) (DWORD)((c<<24)|(t<<16)|(n<<8)|v) //VNTC tag

struct VECT {float x,y,z;};
float M_null[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
//*****************************************************************************************

//prints a matrix -------------------------------------------------------------------------
void printmat(float *mtx)
{
sprintf(strbuf,"%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f",
        mtx[0],mtx[1],mtx[2],mtx[3],mtx[4],mtx[5],mtx[6],mtx[7],mtx[8],
        mtx[9],mtx[10],mtx[11],mtx[12],mtx[13],mtx[14],mtx[15]);
MessageBox(hmwnd,strbuf,"Matrix",MB_OK);
}

//initializes mtx to identity -------------------------------------------------------------
inline void Mnull(float *mtx)
{
mtx[1]=mtx[2]=mtx[3]=mtx[4]=mtx[6]=mtx[7]=mtx[8]=mtx[9]=mtx[11]=mtx[12]=mtx[13]=mtx[14]=0;
mtx[0]=mtx[5]=mtx[10]=mtx[15]=1;
}

//rotates mtx around X by ux (keeps origin) -----------------------------------------------
inline void MspinX(float *mtx,float ux=0)
{
float cx,sx;
cx=cos(ux); sx=sin(ux);
ux=mtx[1];
mtx[1]=mtx[1]*cx-mtx[2]*sx;
mtx[2]=ux*sx+mtx[2]*cx;
ux=mtx[5];
mtx[5]=mtx[5]*cx-mtx[6]*sx;
mtx[6]=ux*sx+mtx[6]*cx;
ux=mtx[9];
mtx[9]=mtx[9]*cx-mtx[10]*sx;
mtx[10]=ux*sx+mtx[10]*cx;
}

//rotates mtx around Y by uy (keeps origin) -----------------------------------------------
inline void MspinY(float *mtx,float uy=0)
{
float cy,sy;
cy=cos(uy); sy=sin(uy);
uy=mtx[0];
mtx[0]=mtx[0]*cy+mtx[2]*sy;
mtx[2]=mtx[2]*cy-uy*sy;
uy=mtx[4];
mtx[4]=mtx[4]*cy+mtx[6]*sy;
mtx[6]=mtx[6]*cy-uy*sy;
uy=mtx[8];
mtx[8]=mtx[8]*cy+mtx[10]*sy;
mtx[10]=mtx[10]*cy-uy*sy;
}

//rotates mtx around Z by uz (keeps origin) -----------------------------------------------
inline void MspinZ(float *mtx,float uz=0)
{
float cz,sz;
cz=cos(uz); sz=sin(uz);
uz=mtx[0];
mtx[0]=mtx[0]*cz-mtx[1]*sz;
mtx[1]=uz*sz+mtx[1]*cz;
uz=mtx[4];
mtx[4]=mtx[4]*cz-mtx[5]*sz;
mtx[5]=uz*sz+mtx[5]*cz;
uz=mtx[8];
mtx[8]=mtx[8]*cz-mtx[9]*sz;
mtx[9]=uz*sz+mtx[9]*cz;
}
                        
//translates mtx by tx, ty, tz ------------------------------------------------------------
inline void Mtrans(float *mtx,float tx=0,float ty=0,float tz=0)
{
mtx[0]+=mtx[3]*tx;   mtx[1]+=mtx[3]*ty;   mtx[2]+=mtx[3]*tz;
mtx[4]+=mtx[7]*tx;   mtx[5]+=mtx[7]*ty;   mtx[6]+=mtx[7]*tz;
mtx[8]+=mtx[11]*tx;  mtx[9]+=mtx[11]*ty;  mtx[10]+=mtx[11]*tz;
mtx[12]+=mtx[15]*tx; mtx[13]+=mtx[15]*ty; mtx[14]+=mtx[15]*tz;
}

//translates origin by dx, dy, dz ---------------------------------------------------------
inline void Mmove(float *mtx,float dx=0,float dy=0,float dz=0)
{
mtx[12]+=dx; mtx[13]+=dy; mtx[14]+=dz;
}

//scales mtx by sx, sy, sz ----------------------------------------------------------------
inline void Mscale(float *mtx,float sx=1,float sy=1,float sz=1)
{
mtx[0]*=sx;  mtx[1]*=sy;  mtx[2]*=sz;
mtx[4]*=sx;  mtx[5]*=sy;  mtx[6]*=sz;
mtx[8]*=sx;  mtx[9]*=sy;  mtx[10]*=sz;
mtx[12]*=sx; mtx[13]*=sy; mtx[14]*=sz;
}

//scales mtx by sf (keeps origin) ---------------------------------------------------------
inline void Mzoom(float *mtx,float sf=1)
{
mtx[0]*=sf; mtx[1]*=sf; mtx[2]*=sf;
mtx[4]*=sf; mtx[5]*=sf; mtx[6]*=sf;
mtx[8]*=sf; mtx[9]*=sf; mtx[10]*=sf;
}

//makes a vector a unit vector ------------------------------------------------------------
inline void Vunit(float *vct)
{
float l;
l=1.0f/sqrt(vct[0]*vct[0]+vct[1]*vct[1]+vct[2]*vct[2]);
vct[0]*=l;
vct[1]*=l;
vct[2]*=l;
}

//translates vct by dx,dy,dz --------------------------------------------------------------
inline void Vmove(float *vct,float dx=0,float dy=0,float dz=0)
{
vct[0]+=dx; vct[1]+=dy; vct[2]+=dz;
}

//rotates vct(2) u rads in 2D plane  -------------------------------------------------------
inline void Vspin(float *vct,float u=0)
{
float c,s;
c=cos(u); s=sin(u);
u=vct[0];
vct[0]*=c; vct[0]-=vct[1]*s;
vct[1]*=c; vct[1]+=u*s;
}

//rotates vct ux rads around X ------------------------------------------------------------
inline void VspinX(float *vct,float ux=0)
{
float cx,sx;
cx=cos(ux); sx=sin(ux);
ux=vct[1];
vct[1]*=cx; vct[1]-=vct[2]*sx;
vct[2]*=cx; vct[2]+=ux*sx;
}

//rotates vct uy rads around Y ------------------------------------------------------------
inline void VspinY(float *vct,float uy=0)
{
float cy,sy;
cy=cos(uy); sy=sin(uy);
uy=vct[2];
vct[2]*=cy; vct[2]-=vct[0]*sy;
vct[0]*=cy; vct[0]+=uy*sy;
}

//rotates vct uz rads around Z ------------------------------------------------------------
inline void VspinZ(float *vct,float uz=0)
{
float cz,sz;
cz=cos(uz); sz=sin(uz);
uz=vct[0];
vct[0]*=cz; vct[0]-=vct[1]*sz;
vct[1]*=cz; vct[1]+=uz*sz;
}

//scales vct by sf ------------------------------------------------------------------------
inline void Vzoom(float *vct,float sf=1)
{
vct[0]*=sf; vct[1]*=sf; vct[2]*=sf;
}

//reverses the vector's heading -------------------------------------------------------
inline void Vopus(float *vct)
{
vct[0]=-vct[0]; vct[1]=-vct[1]; vct[2]=-vct[2];
}

//calculates vector dot product (Vdot = A dot B) ---------------------------------------
inline float Vdot(float *A,float *B)
{
return (A[0]*B[0]+A[1]*B[1]+A[2]*B[2]);
}

//calculates vector cross product (vct=AxB) -----------------------------------------------
inline void Vcross(float *vct,float Ax,float Ay,float Az,float Bx,float By,float Bz)
{
vct[0]=Ay*Bz-Az*By;
vct[1]=Az*Bx-Ax*Bz;
vct[2]=Ax*By-Ay*Bx;
}

//calculates vector cross product (dvct=dvctxsvct) ---------------------------------------------
inline void VxV(float *dvct,float *svct)
{
float x,y;
x=dvct[0]; y=dvct[1];
dvct[0]=y*svct[2]-dvct[2]*svct[1];
dvct[1]=dvct[2]*svct[0]-x*svct[2];
dvct[2]=x*svct[1]-y*svct[0];
}

//vct(3)*=mat(4x4) (normal transformation) -----------------------------------------------------
inline void VxM3(float *vct,float *mat)
{
float x,y;
x=vct[0]; y=vct[1];
vct[0]=x*mat[0]+y*mat[4]+vct[2]*mat[8];
vct[1]=x*mat[1]+y*mat[5]+vct[2]*mat[9];
vct[2]=x*mat[2]+y*mat[6]+vct[2]*mat[10];
}

//vct(3+1)*=mat(4x4) (world transformation) -----------------------------------------------------
inline void PxM3(float *vct,float *mat)
{
float x,y;
x=vct[0]; y=vct[1];
vct[0]=x*mat[0]+y*mat[4]+vct[2]*mat[8]+mat[12];
vct[1]=x*mat[1]+y*mat[5]+vct[2]*mat[9]+mat[13];
vct[2]=x*mat[2]+y*mat[6]+vct[2]*mat[10]+mat[14];
}

//vct=vct*mat (DX,GL transformations) -----------------------------------------------------
inline void VxM4(float *vct,float *mat)
{
float x,y,z;
x=vct[0]; y=vct[1]; z=vct[2];
vct[0]=x*mat[0]+y*mat[4]+z*mat[8]+vct[3]*mat[12];
vct[1]=x*mat[1]+y*mat[5]+z*mat[9]+vct[3]*mat[13];
vct[2]=x*mat[2]+y*mat[6]+z*mat[10]+vct[3]*mat[14];
vct[3]=x*mat[3]+y*mat[7]+z*mat[11]+vct[3]*mat[15];
}

//vct=mat*vct (invers transformations) ----------------------------------------------------
inline void MxV4(float *mat,float *vct)
{
float x,y,z;
x=vct[0]; y=vct[1]; z=vct[2];
vct[0]=mat[0]*x+mat[1]*y+mat[2]*z+mat[3]*vct[3];
vct[1]=mat[4]*x+mat[5]*y+mat[6]*z+mat[7]*vct[3];
vct[2]=mat[8]*x+mat[9]*y+mat[10]*z+mat[11]*vct[3];
vct[3]=mat[12]*x+mat[13]*y+mat[14]*z+mat[15]*vct[3];
}

//d*=s matrix multiplication -----------------------------------------------------------------------
void MxM4(float *d,float *s)
{
float x,y,z;
x=d[0]; y=d[1]; z=d[2];
d[0]=x*s[0]+y*s[4]+z*s[8]+d[3]*s[12];
d[1]=x*s[1]+y*s[5]+z*s[9]+d[3]*s[13];
d[2]=x*s[2]+y*s[6]+z*s[10]+d[3]*s[14];
d[3]=x*s[3]+y*s[7]+z*s[11]+d[3]*s[15];
x=d[4]; y=d[5]; z=d[6];
d[4]=x*s[0]+y*s[4]+z*s[8]+d[7]*s[12];
d[5]=x*s[1]+y*s[5]+z*s[9]+d[7]*s[13];
d[6]=x*s[2]+y*s[6]+z*s[10]+d[7]*s[14];
d[7]=x*s[3]+y*s[7]+z*s[11]+d[7]*s[15];
x=d[8]; y=d[9]; z=d[10];
d[8]=x*s[0]+y*s[4]+z*s[8]+d[11]*s[12];
d[9]=x*s[1]+y*s[5]+z*s[9]+d[11]*s[13];
d[10]=x*s[2]+y*s[6]+z*s[10]+d[11]*s[14];
d[11]=x*s[3]+y*s[7]+z*s[11]+d[11]*s[15];
x=d[12]; y=d[13]; z=d[14];
d[12]=x*s[0]+y*s[4]+z*s[8]+d[15]*s[12];
d[13]=x*s[1]+y*s[5]+z*s[9]+d[15]*s[13];
d[14]=x*s[2]+y*s[6]+z*s[10]+d[15]*s[14];
d[15]=x*s[3]+y*s[7]+z*s[11]+d[15]*s[15];
}

//multiplies 2 matrices md=m1xm2 ----------------------------------------------
void Mmul(float *md,float *m1,float *m2)
{
CopyMemory(md,m1,64);
MxM4(md,m2);
}

//calculates face normal from 3 verts -----------------------------------------------------
inline void Tnorm(float *N,float *A,float *B,float *C)
{
Vcross(N,A[0]-C[0],A[1]-C[1],A[2]-C[2],B[0]-C[0],B[1]-C[1],B[2]-C[2]);
Vunit(N);
}

//creates a transformation matrix from the world, camera, projection and vieport matrices ----------
void M_tf(float *mtx,float *matw=NULL,float *matc=NULL,float *matp=NULL,float *matv=NULL)
{
Mnull(mtx);
if(matw) MxM4(mtx,matw);
if(matc) MxM4(mtx,matc);
if(matp) MxM4(mtx,matp);
if(matv) MxM4(mtx,matv);
}

//transforms a point using a transformation matrix -------------------------------------------------
void Ptf4(float *dpct,float *spct=NULL,float *mtx=NULL)
{
if(spct) CopyMemory(dpct,spct,16);
if(mtx)	VxM4(dpct,mtx);
dpct[3]=1.0f/dpct[3];
dpct[0]*=dpct[3];
dpct[1]*=dpct[3];
dpct[2]*=dpct[3];
}

//creates a translation matrix ------------------------------------------------------------
inline void M_trans(float *mtx,float tx=0,float ty=0,float tz=0)
{
mtx[1]=mtx[2]=mtx[3]=mtx[4]=mtx[6]=mtx[7]=mtx[8]=mtx[9]=mtx[11]=0;
mtx[0]=mtx[5]=mtx[10]=mtx[15]=1;
mtx[12]=tx; mtx[13]=ty; mtx[14]=tz;
}

//creates a concatenation of the 3 rotation matrices (M=RZxRYxRX) -------------------------
void M_rotZYX(float *mtx,float sx=0,float sy=0,float sz=0)
{
float cx,cy,cz;
cx=cos(sx); sx=sin(sx);
cy=cos(sy); sy=sin(sy);
cz=cos(sz); sz=sin(sz);
mtx[0]=cy*cz;
mtx[1]=cx*sz+sx*sy*cz;
mtx[2]=sx*sz-cx*sy*cz;
mtx[4]=-cy*sz;
mtx[5]=cx*cz-sx*sy*sz;
mtx[6]=sx*cz+cx*sy*sz;
mtx[8]=sy;
mtx[9]=-sx*cy;
mtx[10]=cx*cy;
mtx[3]=mtx[7]=mtx[11]=mtx[12]=mtx[13]=mtx[14]=0;
mtx[15]=1;
}

//creates a rotation matrix around a vector (ax,ay,az) ------------------------------------
void M_rot(float *mtx,float ru=0,float ax=1,float ay=1,float az=1)
{
double c,s,ic;
c=cos(ru); s=sin(ru); ic=1-c;
Vunit(&ax);
mtx[0]=ax*ax*ic+c;
mtx[1]=ax*ay*ic-az*s;
mtx[2]=ax*az*ic+ay*s;
mtx[4]=ay*ax*ic+az*s;
mtx[5]=ay*ay*ic+c;
mtx[6]=ay*az*ic-ax*s;
mtx[8]=az*ax*ic-ay*s;
mtx[9]=az*ay*ic+ax*s;
mtx[10]=az*az*ic+c;
mtx[3]=mtx[7]=mtx[11]=mtx[12]=mtx[13]=mtx[14]=0;
mtx[15]=1;
}

//rotates vct around arbitrary axis -------------------------------------------------------
void Vrot(float *vct,float ru=0,float ax=1,float ay=1,float az=1)
{
float mr[16];
M_rot(mr,ru,ax,ay,az);
VxM3(vct,mr);
}

//rotates mtx around arbitrary axis -------------------------------------------------------
void Mrot(float *mtx,float ru=0,float ax=1,float ay=1,float az=1)
{
float mr[16];
M_rot(mr,ru,ax,ay,az);
MxM4(mtx,mr);
}

//rotates mtx around ZYX axes -------------------------------------------------------------
void MrotZYX(float *mtx,float rx=0,float ry=0,float rz=0)
{
float mr[16];
M_rotZYX(mr,rx,ry,rz);
MxM4(mtx,mr);
}

//creates a camera 'look at' matrix (CZ=target-eye) -----------------------------------
inline void M_cam(float *mat,float *CO,float *CZ)
{
float CX[3],CY[3];
Vunit(CZ); //Camera Z axis
Vcross(CX,0,1,0,CZ[0],CZ[1],CZ[2]);
Vunit(CX); //Camera X axis
Vcross(CY,CZ[0],CZ[1],CZ[2],CX[0],CX[1],CX[2]);
Vunit(CY); //Camera Y axis
mat[0]=CX[0]; mat[1]=CY[0]; mat[2]=CZ[0];
mat[4]=CX[1]; mat[5]=CY[1]; mat[6]=CZ[1];
mat[8]=CX[2]; mat[9]=CY[2]; mat[10]=CZ[2];
mat[12]=-Vdot(CO,CX);
mat[13]=-Vdot(CO,CY);
mat[14]=-Vdot(CO,CZ);
mat[3]=mat[7]=mat[11]=0;
mat[15]=1;
}

//calculeaza distanta de la un punct P la un plan ABC ---------------------------------------
inline float dPP(float *P,float *A,float *B,float *C)
{
float N[3];
Tnorm(N,A,B,C);
return Vdot(P,N)-Vdot(A,N);
}

//determines if the 3d seg OP intersects tri ABC --------------------------------------
int VinT(float *O,float *P,float *A,float *B,float *C)
{
if(dPP(P,O,A,B)>=0)
 {
 if(dPP(P,O,B,C)>=0)
  {
  if(dPP(P,O,C,A)>=0)
   {
   if(dPP(P,A,B,C)>=0)
    {
    if(dPP(O,A,B,C)<0) return 1;
    }
   else
    {
    if(dPP(O,A,B,C)>0) return 1;
    }
   }
  }
 } 
else
 {
 if(dPP(P,O,B,C)<0)
  {
  if(dPP(P,O,C,A)<0)
   {
   if(dPP(P,A,B,C)>=0)
    {
    if(dPP(O,A,B,C)<0) return 1;
    }
   else
    {
    if(dPP(O,A,B,C)>0) return 1;
    }
   }
  }
 } 
return 0;
}

//creates a perspective correction matrix ------------------------------------
inline void M_perspect(float *mp,float logw=2,float logh=2,float minz=1,float maxz=100)
{
mp[1]=mp[2]=mp[3]=mp[4]=mp[6]=mp[7]=mp[8]=mp[9]=mp[12]=mp[13]=mp[15]=0;
mp[0]=minz*2/logw;
mp[5]=minz*2/logh;
mp[10]=maxz/(maxz-minz);
mp[11]=1;
mp[14]=-minz*mp[10]; //translation is in (0;1) not (minz;maxz)
}

//creates a perspective correction matrix based on fov -----------------------
inline void M_fov(float *mp,float hfov=PI2,float vfov=PI2,float minz=1,float maxz=100)
{
mp[1]=mp[2]=mp[3]=mp[4]=mp[6]=mp[7]=mp[8]=mp[9]=mp[12]=mp[13]=mp[15]=0;
mp[0]=1.0f/tan(hfov*.5);
mp[5]=1.0f/tan(vfov*.5);
mp[10]=maxz/(maxz-minz);
mp[11]=1;
mp[14]=-minz*mp[10]; //translation is in (0;1) not (minz;maxz)
}

//creates an ortho perspective correction matrix ------------------------------
inline void M_ortho(float *mp,float logw=2,float logh=2,float minz=1,float maxz=100)
{
mp[1]=mp[2]=mp[3]=mp[4]=mp[6]=mp[7]=mp[8]=mp[9]=mp[11]=mp[12]=mp[13]=0;
mp[0]=2.0f/logw;
mp[5]=2.0f/logh;
mp[10]=1.0f/(maxz-minz);
mp[14]=-minz*mp[10]; //translation is in (0;1) not (minz;maxz)
mp[15]=1;
}

//creates a viewport rescale matrix -----------------------------------------
inline void M_viewport(float *mv,float l=0,float u=0,float w=0,float h=0,float minz=0,float maxz=1)
{
mv[1]=mv[2]=mv[3]=mv[4]=mv[6]=mv[7]=mv[8]=mv[9]=mv[11]=0;
mv[0]=w/2;
mv[5]=-h/2;
mv[10]=maxz-minz;
mv[12]=l+mv[0];
mv[13]=u-mv[5];
mv[14]=minz;
mv[15]=1;
}

//number of triangles in a list, strip or fan -----------------------------
inline unsigned NumTris(NAT nrv,int tip=1)
{
return (tip==1?nrv/3:nrv-2);
}
#endif