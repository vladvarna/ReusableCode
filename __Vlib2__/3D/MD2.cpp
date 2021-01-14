#ifndef V_QUAKE2MODEL
#define V_QUAKE2MODEL

#include <img.cpp>
#include <str.cpp>

struct MD2HEADER
{
 int id,vers; //0x32504449, 8
 int skinW,skinH;
 int FrmSz;
 int nrSkins,nrVertices,nrTexCoords,nrTriangles,nrComms,nrFrames;
 int offSkins,offTexCoords,offTriangles,offFrames,offComms,offEnd;
};

class Q2Model
{
public:
 FILE *Fmd2;
 MD2HEADER head;
 WORD *Tris,*Coords; //triangles list
 float msk[6]; //scale, translation masks (float=byte*s+t)
 char frmname[16];
 struct MD2FRAME { BYTE x,y,z,ni; } *Verts;
 
 int Open(LPSTR filename=NULL);
 int Skin(int skind,LPSTR skiname); //skiname=char[64]
 int Frame(NAT fi=0,float *vntbuf=NULL);
 void GetF(NAT ti,float *pt,NAT ov=0,NAT on=0,NAT ot=0,NAT tcnt=1);
 Q2Model()
  {
  Fmd2=NULL;
  Tris=Coords=NULL;
  Verts=NULL;
  }
 ~Q2Model()
  {
  if(Fmd2) fclose(Fmd2);
  if(Tris) free(Tris);
  if(Coords) free(Coords);
  if(Verts) free(Verts);
  }
#ifdef V_IDIRECTXG8
 IDirect3DTexture8* DXSkin(int skind=0,LPSTR defdir=NULL);
#endif
};

//162 * 3 floats
float MD2norms[]={
-0.525731, 0.000000, 0.850651, 
-0.442863, 0.238856, 0.864188, 
-0.295242, 0.000000, 0.955423, 
-0.309017, 0.500000, 0.809017, 
-0.162460, 0.262866, 0.951056, 
0.000000, 0.000000, 1.000000, 
0.000000, 0.850651, 0.525731, 
-0.147621, 0.716567, 0.681718, 
0.147621, 0.716567, 0.681718, 
0.000000, 0.525731, 0.850651, 
0.309017, 0.500000, 0.809017, 
0.525731, 0.000000, 0.850651, 
0.295242, 0.000000, 0.955423, 
0.442863, 0.238856, 0.864188, 
0.162460, 0.262866, 0.951056, 
-0.681718, 0.147621, 0.716567, 
-0.809017, 0.309017, 0.500000, 
-0.587785, 0.425325, 0.688191, 
-0.850651, 0.525731, 0.000000, 
-0.864188, 0.442863, 0.238856, 
-0.716567, 0.681718, 0.147621, 
-0.688191, 0.587785, 0.425325, 
-0.500000, 0.809017, 0.309017, 
-0.238856, 0.864188, 0.442863, 
-0.425325, 0.688191, 0.587785, 
-0.716567, 0.681718, -0.147621, 
-0.500000, 0.809017, -0.309017, 
-0.525731, 0.850651, 0.000000, 
0.000000, 0.850651, -0.525731, 
-0.238856, 0.864188, -0.442863, 
0.000000, 0.955423, -0.295242, 
-0.262866, 0.951056, -0.162460, 
0.000000, 1.000000, 0.000000, 
0.000000, 0.955423, 0.295242, 
-0.262866, 0.951056, 0.162460, 
0.238856, 0.864188, 0.442863, 
0.262866, 0.951056, 0.162460, 
0.500000, 0.809017, 0.309017, 
0.238856, 0.864188, -0.442863, 
0.262866, 0.951056, -0.162460, 
0.500000, 0.809017, -0.309017, 
0.850651, 0.525731, 0.000000, 
0.716567, 0.681718, 0.147621, 
0.716567, 0.681718, -0.147621, 
0.525731, 0.850651, 0.000000, 
0.425325, 0.688191, 0.587785, 
0.864188, 0.442863, 0.238856, 
0.688191, 0.587785, 0.425325, 
0.809017, 0.309017, 0.500000, 
0.681718, 0.147621, 0.716567, 
0.587785, 0.425325, 0.688191, 
0.955423, 0.295242, 0.000000, 
1.000000, 0.000000, 0.000000, 
0.951056, 0.162460, 0.262866, 
0.850651, -0.525731, 0.000000, 
0.955423, -0.295242, 0.000000, 
0.864188, -0.442863, 0.238856, 
0.951056, -0.162460, 0.262866, 
0.809017, -0.309017, 0.500000, 
0.681718, -0.147621, 0.716567, 
0.850651, 0.000000, 0.525731, 
0.864188, 0.442863, -0.238856, 
0.809017, 0.309017, -0.500000, 
0.951056, 0.162460, -0.262866, 
0.525731, 0.000000, -0.850651, 
0.681718, 0.147621, -0.716567, 
0.681718, -0.147621, -0.716567, 
0.850651, 0.000000, -0.525731, 
0.809017, -0.309017, -0.500000, 
0.864188, -0.442863, -0.238856, 
0.951056, -0.162460, -0.262866, 
0.147621, 0.716567, -0.681718, 
0.309017, 0.500000, -0.809017, 
0.425325, 0.688191, -0.587785, 
0.442863, 0.238856, -0.864188, 
0.587785, 0.425325, -0.688191, 
0.688191, 0.587785, -0.425325, 
-0.147621, 0.716567, -0.681718, 
-0.309017, 0.500000, -0.809017, 
0.000000, 0.525731, -0.850651, 
-0.525731, 0.000000, -0.850651, 
-0.442863, 0.238856, -0.864188, 
-0.295242, 0.000000, -0.955423, 
-0.162460, 0.262866, -0.951056, 
0.000000, 0.000000, -1.000000, 
0.295242, 0.000000, -0.955423, 
0.162460, 0.262866, -0.951056, 
-0.442863, -0.238856, -0.864188, 
-0.309017, -0.500000, -0.809017, 
-0.162460, -0.262866, -0.951056, 
0.000000, -0.850651, -0.525731, 
-0.147621, -0.716567, -0.681718, 
0.147621, -0.716567, -0.681718, 
0.000000, -0.525731, -0.850651, 
0.309017, -0.500000, -0.809017, 
0.442863, -0.238856, -0.864188, 
0.162460, -0.262866, -0.951056, 
0.238856, -0.864188, -0.442863, 
0.500000, -0.809017, -0.309017, 
0.425325, -0.688191, -0.587785, 
0.716567, -0.681718, -0.147621, 
0.688191, -0.587785, -0.425325, 
0.587785, -0.425325, -0.688191, 
0.000000, -0.955423, -0.295242, 
0.000000, -1.000000, 0.000000, 
0.262866, -0.951056, -0.162460, 
0.000000, -0.850651, 0.525731, 
0.000000, -0.955423, 0.295242, 
0.238856, -0.864188, 0.442863, 
0.262866, -0.951056, 0.162460, 
0.500000, -0.809017, 0.309017, 
0.716567, -0.681718, 0.147621, 
0.525731, -0.850651, 0.000000, 
-0.238856, -0.864188, -0.442863, 
-0.500000, -0.809017, -0.309017, 
-0.262866, -0.951056, -0.162460, 
-0.850651, -0.525731, 0.000000, 
-0.716567, -0.681718, -0.147621, 
-0.716567, -0.681718, 0.147621, 
-0.525731, -0.850651, 0.000000, 
-0.500000, -0.809017, 0.309017, 
-0.238856, -0.864188, 0.442863, 
-0.262866, -0.951056, 0.162460, 
-0.864188, -0.442863, 0.238856, 
-0.809017, -0.309017, 0.500000, 
-0.688191, -0.587785, 0.425325, 
-0.681718, -0.147621, 0.716567, 
-0.442863, -0.238856, 0.864188, 
-0.587785, -0.425325, 0.688191, 
-0.309017, -0.500000, 0.809017, 
-0.147621, -0.716567, 0.681718, 
-0.425325, -0.688191, 0.587785, 
-0.162460, -0.262866, 0.951056, 
0.442863, -0.238856, 0.864188, 
0.162460, -0.262866, 0.951056, 
0.309017, -0.500000, 0.809017, 
0.147621, -0.716567, 0.681718, 
0.000000, -0.525731, 0.850651, 
0.425325, -0.688191, 0.587785, 
0.587785, -0.425325, 0.688191, 
0.688191, -0.587785, 0.425325, 
-0.955423, 0.295242, 0.000000, 
-0.951056, 0.162460, 0.262866, 
-1.000000, 0.000000, 0.000000, 
-0.850651, 0.000000, 0.525731, 
-0.955423, -0.295242, 0.000000, 
-0.951056, -0.162460, 0.262866, 
-0.864188, 0.442863, -0.238856, 
-0.951056, 0.162460, -0.262866, 
-0.809017, 0.309017, -0.500000, 
-0.864188, -0.442863, -0.238856, 
-0.951056, -0.162460, -0.262866, 
-0.809017, -0.309017, -0.500000, 
-0.681718, 0.147621, -0.716567, 
-0.681718, -0.147621, -0.716567, 
-0.850651, 0.000000, -0.525731, 
-0.688191, 0.587785, -0.425325, 
-0.587785, 0.425325, -0.688191, 
-0.425325, 0.688191, -0.587785, 
-0.425325, -0.688191, -0.587785, 
-0.587785, -0.425325, -0.688191, 
-0.688191, -0.587785, -0.425325};
//****************************************************************************

int Q2Model::Open(LPSTR filename)
{
if(Fmd2) fclose(Fmd2);
Fmd2=NULL;
if(Tris) free(Tris);
if(Coords) free(Coords);
Tris=Coords=NULL;
if(Verts) free(Verts);
Verts=NULL;
if(!filename) return 0; //close Ok

Fmd2=FOPEN(filename,"rb");
if(Fmd2==NULL) return 1; //can't open file
fread(&head,sizeof(head),1,Fmd2);
if(head.id!=0x32504449||head.vers!=0x8) 
 {
 fclose(Fmd2);
 Fmd2=NULL;
 return 2;
 }
Tris=(WORD*)malloc(head.nrTriangles*12);
fseek(Fmd2,head.offTriangles,SEEK_SET);
fread(Tris,12,head.nrTriangles,Fmd2);

Coords=(WORD*)malloc(head.nrTexCoords*4);
fseek(Fmd2,head.offTexCoords,SEEK_SET);
fread(Coords,4,head.nrTexCoords,Fmd2);

Verts=(MD2FRAME*)malloc(head.FrmSz-40);
Frame();
return 0; //Ok
}

int Q2Model::Frame(NAT fi,float *vntbuf)
{
fi%=head.nrFrames;
fseek(Fmd2,head.offFrames+fi*head.FrmSz,SEEK_SET);
fread(msk,24,1,Fmd2);
fread(frmname,16,1,Fmd2);
fread(Verts,head.FrmSz-40,1,Fmd2);
if(vntbuf) GetF(0,vntbuf,8,3,6,head.nrTriangles);
return fi;
}

int Q2Model::Skin(int skind,LPSTR skiname)
{
if(head.nrSkins==0) return -1;
skind=skind%head.nrSkins;
fseek(Fmd2,head.offSkins+skind*64,SEEK_SET);
fread(skiname,64,1,Fmd2);
strcpy(skiname,skiname+lastch('/',skiname)+1);
return skind;
}

void Q2Model::GetF(NAT ti,float *pt,NAT ov,NAT on,NAT ot,NAT tcnt)
{
if(!tcnt) tcnt=head.nrTriangles;
ti*=6; //(3 verts + 3 coords)*sizeof(WORD)
while(tcnt>0)
 {
 for(NAT v=0;v<3;v++)
  {
  pt[0]=Verts[Tris[ti+v]].x*msk[0]+msk[3];
  pt[1]=Verts[Tris[ti+v]].y*msk[1]+msk[4];
  pt[2]=Verts[Tris[ti+v]].z*msk[2]+msk[5];
  CopyMemory(pt+on,MD2norms+Verts[Tris[ti+v]].ni*3,12);
  pt[ot]=(float)Coords[Tris[ti+v+3]<<1]/head.skinW;
  pt[ot+1]=(float)Coords[(Tris[ti+v+3]<<1)+1]/head.skinH;
  pt+=ov;
  }
 ti+=6;
 tcnt--;
 }
}

#ifdef V_IDIRECTXG8

IDirect3DTexture8* Q2Model::DXSkin(int skind,LPSTR defdir)
{
Image MD2skin;
char skinpath[260],sn=0;
if(defdir)
 {
 sn=strlen(defdir);
 CopyMemory(skinpath,defdir,sn+1);
 if(skind>=0) skinpath[sn++]='\\';
 }
if(skind>=0) 
 if(Skin(skind,skinpath+sn)<0) return NULL;
MD2skin.From(skinpath);
return MD2skin.DXTex();
}

#endif

#endif