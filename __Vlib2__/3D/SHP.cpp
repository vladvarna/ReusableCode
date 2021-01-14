#ifndef V_SHAPEFILES
#define V_SHAPEFILES

class SHPfile
{
public:
 FILE *Fshp;
 unsigned VBoff,IBoff,NrV,NrI;
 unsigned V,N,T,C,Vsz,Nsz,Tsz,Csz; //in Bytes
 unsigned Bpi,Type,NrL; //Type:0-unk,1-list,2-strip,3-fan
 unsigned NrT;
 
 void Set(NAT nrv=1,NAT ofs=0x06070309,NAT szs=0x01020303,NAT tip=1,NAT nri=0,NAT isz=2,NAT nrl=3);
 int Save(LPSTR filename=NULL);
 int Open(LPSTR filename=NULL);
 
 int Get(void *pbuf,NAT vi=0,NAT vcnt=1);
 int Put(void *pbuf,NAT vi=0,NAT vcnt=1);
 int IGet(void *pi,NAT sti=0,NAT icnt=1);
 int IPut(void *pi,NAT sti=0,NAT icnt=1);
 void GetF(NAT ti,float *pt,NAT ov=0,NAT on=0,NAT ot=0,NAT oc=0,NAT tcnt=1);
 void PutF(NAT ti,float *pt,NAT ov=0,NAT on=0,NAT ot=0,NAT oc=0,NAT tcnt=1);
 
 SHPfile()
  {
  Fshp=NULL;
  VBoff=IBoff=0;
  }
 ~SHPfile()
  {
  if(Fshp) fclose(Fshp);
  }
#ifdef V_IDIRECTXG8
 int DXBuffers(IDirect3DVertexBuffer8 *d3dvb=NULL,IDirect3DIndexBuffer8 *d3dib=NULL);
#endif
};

void SHPfile::Set(NAT nrv,NAT ofs,NAT szs,NAT tip,NAT nri,NAT isz,NAT nrl)
{
NrV=nrv; NrI=nri;
V=(ofs&0xff)<<2;
N=(ofs&0xff00)>>6;
T=(ofs&0xff0000)>>14;
C=(ofs&0xff000000)>>22;
Vsz=(szs&0xff)<<2;
Nsz=(szs&0xff00)>>6;
Tsz=(szs&0xff0000)>>14;
Csz=(szs&0xff000000)>>22;
Bpi=isz;
NrL=nrl;
Type=tip;
NrT=NumTris(NrI?NrI:NrV,Type);
}

int SHPfile::Open(LPSTR filename)
{
if(Fshp) fclose(Fshp);
if(!filename) //close
 {
 Fshp=NULL;
 return -1;
 }
Fshp=FOPEN(filename,"rb+");
if(Fshp==NULL) return 1; //can't find
QUADB head;
fread(head.b,4,1,Fshp);
if(head.n!=0x43544e56) 
 {
 fclose(Fshp);
 Fshp=NULL;
 return 2; //id not recognized
 }
fread(&VBoff,8,1,Fshp);
fread(head.b,4,1,Fshp);
if(head.b[3]>0x11) return 3; //unknown version
NrL=head.b[2];
Type=head.b[1];
Bpi=head.b[0];
fread(head.b,4,1,Fshp);
V=(head.n&0xff)<<2;
N=(head.n&0xff00)>>6;
T=(head.n&0xff0000)>>14;
C=(head.n&0xff000000)>>22;
fread(head.b,4,1,Fshp);
Vsz=(head.n&0xff)<<2;
Nsz=(head.n&0xff00)>>6;
Tsz=(head.n&0xff0000)>>14;
Csz=(head.n&0xff000000)>>22;
if(VBoff)
 {
 fseek(Fshp,VBoff,SEEK_SET);
 fread(&NrV,4,1,Fshp);
 VBoff+=4;
 NrT=NumTris(NrV,Type);
 }
else NrV=0;
if(IBoff)
 {
 fseek(Fshp,IBoff,SEEK_SET);
 fread(&NrI,4,1,Fshp);
 IBoff+=4;
 NrT=NumTris(NrI,Type);
 }
else NrI=0;
return 0; //Ok
}

int SHPfile::Save(LPSTR filename)
{
if(filename) //create new file
 {
 if(Fshp) fclose(Fshp);
 Fshp=FOPEN(filename,"wb+");
 }
if(!Fshp) return 1; //can't create
QUADB head;
head.n=0x43544e56; //ID="VNTC"
fwrite(head.b,4,1,Fshp);
VBoff=(NrV==0?0:24);
IBoff=(NrI==0?0:VBoff+1+V*NrV);
fwrite(&VBoff,8,1,Fshp);
head.b[0]=Bpi;
head.b[1]=Type;
head.b[2]=NrL;
head.b[3]=0x11; //vers.rel=1.1
fwrite(head.b,4,1,Fshp);
head.n=((C&0x3fc)<<22)|((T&0x3fc)<<14)|((N&0x3fc)<<6)|((V>>2)&0xff);
fwrite(head.b,4,1,Fshp);
head.n=((Csz&0x3fc)<<22)|((Tsz&0x3fc)<<14)|((Nsz&0x3fc)<<6)|((Vsz>>2)&0xff);
fwrite(head.b,4,1,Fshp);
if(VBoff)
 {
 fseek(Fshp,VBoff,SEEK_SET);
 fwrite(&NrV,4,1,Fshp);
 VBoff+=4;
 }
if(IBoff)
 {
 fseek(Fshp,IBoff,SEEK_SET);
 fwrite(&NrI,4,1,Fshp);
 IBoff+=4;
 }
return 0; //Ok
}

int SHPfile::Get(void *pbuf,NAT vi,NAT vcnt)
{
if(Fshp==NULL) return 0;
if((!vcnt)||(vi+vcnt>NrV)) vcnt=NrV-vi;
fseek(Fshp,VBoff+vi*V,SEEK_SET);
return fread(pbuf,V,vcnt,Fshp);
}

int SHPfile::Put(void *pbuf,NAT vi,NAT vcnt)
{
if(Fshp==NULL) return 0;
if((!vcnt)||(vi+vcnt>NrV)) vcnt=NrV-vi;
fseek(Fshp,VBoff+vi*V,SEEK_SET);
return fwrite(pbuf,V,vcnt,Fshp);
}

int SHPfile::IGet(void *pi,NAT sti,NAT icnt)
{
if((Fshp==NULL)||!IBoff) return 0;
if((!icnt)||(sti+icnt>NrI)) icnt=NrI-sti;
fseek(Fshp,IBoff+sti*Bpi,SEEK_SET);
return fread(pi,Bpi,icnt,Fshp);
} 

int SHPfile::IPut(void *pi,NAT sti,NAT icnt)
{
if((Fshp==NULL)||!IBoff) return 0;
if((!icnt)||(sti+icnt>NrI)) icnt=NrI-sti;
fseek(Fshp,IBoff+sti*Bpi,SEEK_SET);
return fwrite(pi,Bpi,icnt,Fshp);
} 

void SHPfile::GetF(NAT ti,float *pt,NAT ov,NAT on,NAT ot,NAT oc,NAT tcnt)
{
if(!Fshp) return;
if(!tcnt) tcnt=NrT-ti;
tcnt*=3;
ti=VBoff+ti*V*3;
while(tcnt>0)
 {
 fseek(Fshp,ti,SEEK_SET);
 fread(pt,12,1,Fshp);
 if(N&&on)
  {
  fseek(Fshp,ti+N,SEEK_SET);
  fread(pt+on,12,1,Fshp);
  }
 if(T&&ot)
  {
  fseek(Fshp,ti+T,SEEK_SET);
  fread(pt+ot,8,1,Fshp);
  }
 if(C&&oc)
  {
  fseek(Fshp,ti+C,SEEK_SET);
  fread(pt+oc,4,1,Fshp);
  }
 pt+=ov;
 ti+=V;
 tcnt--;
 }
}

void SHPfile::PutF(NAT ti,float *pt,NAT ov,NAT on,NAT ot,NAT oc,NAT tcnt)
{
if(!Fshp) return;
if(!tcnt) tcnt=NrT-ti;
tcnt*=3;
ti=VBoff+ti*V*3;
while(tcnt>0)
 {
 fseek(Fshp,ti,SEEK_SET);
 fwrite(pt,12,1,Fshp);
 if(N&&on)
  {
  fseek(Fshp,ti+N,SEEK_SET);
  fwrite(pt+on,12,1,Fshp);
  }
 if(T&&ot)
  {
  fseek(Fshp,ti+T,SEEK_SET);
  fwrite(pt+ot,8,1,Fshp);
  }
 if(C&&oc)
  {
  fseek(Fshp,ti+C,SEEK_SET);
  fwrite(pt+oc,4,1,Fshp);
  }
 pt+=ov;
 ti+=V;
 tcnt--;
 }
}

#endif