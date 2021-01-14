#ifndef V_MESH3D
#define V_MESH3D

#include <3D/3d.cpp>
#include <3D/shp.cpp>
#include <3D/md2.cpp>

class VNTCMESH //VNTC triangles buffer
{
public:
 float *vb;
 BYTE *ib;
 unsigned NrV,NrI,NrT;
 unsigned V,N,T,C;
 unsigned Bpi,Type; //Type: 0-unknown,1-list,2-strip,3-fan
 float mw[16];

 void Init(NAT nrv=1,NAT ov=9,NAT on=3,NAT ot=7,NAT oc=6,NAT tip=1,NAT nri=0,NAT isz=2);
 void Free();
 int From(LPSTR filename=NULL,LPSTR filetype=NULL,NAT frmnum=0);
 int To(LPSTR filename=NULL,LPSTR filetype=NULL,NAT frmnum=0);
 void CopyUV(int count=1);
 void ForGL(NAT ov=5,NAT on=2,NAT ot=0,NAT oc=0,NAT stride=9);
 VNTCMESH()
  {
  vb=NULL;
  ib=NULL;
  }
 ~VNTCMESH()
  {
  if(vb) free(vb);
  if(ib) free(ib);
  }
#ifdef V_IDIRECTXG8
 void DXDraw(NAT ti=0,NAT tcnt=0);
#endif
#ifdef V_IOPENGL
 void GLDraw(NAT ti=0,NAT tcnt=0,NAT form=GL_T2F_N3F_V3F);
#endif
};

void VNTCMESH::Init(NAT nrv,NAT ov,NAT on,NAT ot,NAT oc,NAT tip,NAT nri,NAT isz)
{
if(vb) free(vb);
if(ib) free(ib);
NrV=nrv; NrI=nri;
V=ov; N=on; T=ot; C=oc;
Bpi=isz; Type=tip; NrT=0;
if(NrV)
 {
 vb=(float*)malloc(NrV*V*4);
 NrT=NumTris(NrV,Type);
 }
else vb=NULL;
if(NrI)
 {
 ib=(BYTE*)malloc(NrI*Bpi);
 NrT=NumTris(NrI,Type);
 }
else ib=NULL;
Mnull(mw);
}

void VNTCMESH::Free()
{
if(vb) free(vb);
if(ib) free(ib);
vb=NULL;
ib=NULL;
}

int VNTCMESH::From(LPSTR filename,LPSTR filetype,NAT frmnum)
{
if(!filename) return 1; //no file
if(!filetype)
 {
 filetype=filename;
 while(*filetype!='.'&&*filetype!=0) filetype++;
 if(!*filetype) return 2; //unknown file format
 filetype++; //filetype should point to the extension
 }
Free();
if(_stricmp(filetype,"shp")==0) //SHP file
 {
 SHPfile shp;
 if(shp.Open(filename)) return 1; //can't access file
 Init(shp.NrT*3,shp.V>>2,shp.N>>2,shp.T>>2,shp.C>>2);
 shp.GetF(0,vb,V,N,T,C,0);
 }
else if(_stricmp(filetype,"md2")==0) //Quake 2 model file
 {
 Q2Model md2;
 if(md2.Open(filename)) return 1; //can't access file
 Init(md2.head.nrTriangles*3,V,N,T,C);
 md2.GetF(0,vb,V,N,T,0);
 M_trans(mw,0,-20,250);
 MspinX(mw,-PI2);
 //MspinY(mw,PI2);
 }
return 0;
}

int VNTCMESH::To(LPSTR filename,LPSTR filetype,NAT frmnum)
{
if(!filename) return 1; //no file
if(!filetype)
 {
 filetype=filename;
 while(*filetype!='.'&&*filetype!=0) filetype++;
 if(!*filetype) return 2; //unknown file format
 filetype++; //filetype should point to the extension
 }
if(_stricmp(filetype,"shp")==0) //SHP file
 {
 SHPfile shp;
 shp.Set(NrV,VNTC_DW(V,N,T,C),0x01020303,Type,NrI,Bpi);
 if(shp.Save(filename)) return 1; //can't access file
 shp.Put(vb,0,0);
 shp.IPut(ib,0,0);
 }
return 0;
}

void VNTCMESH::CopyUV(int count)
{
if(!T||!vb) return;
float *newvb;
int newV=V+count*2;
newvb=(float*)malloc(NrV*newV*4);
for(int vi=0;vi<NrV;vi++)
 {
 CopyMemory(newvb+vi*newV,vb+vi*V,V<<2);
 for(int c=1;c<=count;c++)
  CopyMemory(newvb+vi*newV+T+c*2,vb+vi*V+T,8);
 }
free(vb);
vb=newvb;
V=newV;
}

void VNTCMESH::ForGL(NAT ov,NAT on,NAT ot,NAT oc,NAT stride)
{
if(!T||!vb) return;
float *newvb;
newvb=(float*)malloc(NrV*stride*4);
for(int vi=0;vi<NrV;vi++)
 {
 CopyMemory(newvb+vi*stride+ov,vb+vi*V,12);
 if(N&&on) CopyMemory(newvb+vi*stride+on,vb+vi*V+N,12);
 if(T) CopyMemory(newvb+vi*stride+ot,vb+vi*V+T,8);
 //if(C&&oc) CopyMemory(newvb+vi*stride+oc,vb+vi*V+C,4);
 }
free(vb);
vb=newvb;
V=stride;
N=on;
T=ot;
C=oc;
}
 
#ifdef V_IDIRECTXG8
 void VNTCMESH::DXDraw(NAT ti,NAT tcnt)
 {
 if(!tcnt) tcnt=NrT;
 dxdev->SetTransform(D3DTS_WORLD,(D3DMATRIX*)mw);
 if(ib) //indexed mesh
  {
  dxdev->DrawIndexedPrimitiveUP((D3DPRIMITIVETYPE)(Type+3),0,NrV,tcnt,ib+ti*3*Bpi,(Bpi==4?D3DFMT_INDEX32:D3DFMT_INDEX16),vb,V<<2);
  }
 else //buffer mesh
  {
  dxdev->DrawPrimitiveUP((D3DPRIMITIVETYPE)(Type+3),tcnt,vb+ti*V*3,V<<2);
  }
 }
#endif

#ifdef V_IOPENGL
 void VNTCMESH::GLDraw(NAT ti,NAT tcnt,NAT form)
 {
 if(!tcnt) tcnt=NrT;
 glMultMatrixf(mw);
 //glInterleavedArrays(form,V*4,vb+ti*V*3);
 glVertexPointer(3,GL_FLOAT,V*4,vb+ti*V*3); //vertex
 glEnableClientState(GL_VERTEX_ARRAY);
 if(N)
  {
  glNormalPointer(GL_FLOAT,V*4,vb+ti*V*3+N); //normal
  glEnableClientState(GL_NORMAL_ARRAY);
  }
 if(T)
  {
  glTexCoordPointer(2,GL_FLOAT,V*4,vb+ti*V*3+T); //tex coord
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }
 if(C) 
  {
  glColorPointer(4,GL_UNSIGNED_BYTE,V*4,vb+ti*V*3+C); //color
  glEnableClientState(GL_COLOR_ARRAY);
  }
 glDrawArrays(Type+3,ti*3,tcnt*3);
 if(form) //draw normals
  {
  glColor3ub(0xff,0xff,0xff);
  glBegin(GL_LINES);
  for(int i=ti*3*V;i<tcnt*3*V;i+=V)
   {
   glVertex3f(*(vb+i),*(vb+i+1),*(vb+i+2));
   glVertex3f(*(vb+i)+*(vb+i+N),*(vb+i+1)+*(vb+i+1+N),*(vb+i+2)+*(vb+i+2+N));
   }
  glEnd();
  }
 }
#endif

#endif