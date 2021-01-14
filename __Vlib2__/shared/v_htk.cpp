#pragma once

#include <shell.cpp>
#include <shared/audio1.cpp>

char V_HTK_PATH[]="c:\\htk-3.3\\bin.win32";

#pragma pack(push,default_pack)
#pragma pack(1) //byte packing
struct V_HTK_HEADER
{
DWORD NrF;
DWORD T0;
WORD BpS;
WORD paramkind;
};
#pragma pack(pop,default_pack)

#define V_KIND_BASEMASK  077
LPSTR V_HTKparamkind[]={
      "WAVEFORM",            /* Raw speech waveform (handled by HWave) */
      "LPC","LPREFC","LPCEPSTRA","LPDELCEP",   /* LP-based Coefficients */
      "IREFC",                           /* Ref Coef in 16 bit form */
      "MFCC",                            /* Mel-Freq Cepstra */
      "FBANK",                           /* Log Filter Bank */
      "MELSPEC",                         /* Mel-Freq Spectrum (Linear) */
      "USER",                            /* Arbitrary user specified data */
      "DISCRETE",                        /* Discrete VQ symbols (shorts) */
      "PLP",                             /* Standard PLP coefficients */
      "ANON"};
#define HASENERGY   0100       /* _E log energy included */
#define HASNULLE    0200       /* _N absolute energy suppressed */
#define HASDELTA    0400       /* _D delta coef appended */
#define HASACCS    01000       /* _A acceleration coefs appended */
#define HASCOMPX   02000       /* _C is compressed */
#define HASZEROM   04000       /* _Z zero meaned */
#define HASCRCC   010000       /* _K has CRC check */
#define HASZEROC  020000       /* _0 0'th Cepstra included */
#define HASVQ     040000       /* _V has VQ index attached */
#define HASTHIRD 0100000       /* _T has Delta-Delta-Delta index attached */

UARRAY<double> vart; //varianta totala

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class VHTKparams
{
public:
 WORD kind;
 NAT T0; //in microseconds
 NAT vsz; //vector size
 NAT nrv; //total number of vectors
 UARRAY<float> vct; //vectors
 UARRAY<double> mean,var,F; //vectors
 NAT c1,c2,en; //c1,c2 position in vector
 double minc1,maxc1;
 double minc2,maxc2;
 double minen,maxen;
 
 VHTKparams() { ZEROCLASS(VHTKparams); }
 ~VHTKparams() { Free(); }
 void Free()
  {
  mean.Free();
  var.Free();
  vct.Free();
  kind=0;
  nrv=0;
  }
 void DBG()
  {
  //printbox("%s nrv=%u vsz=%u T0=%uus\n%f %f %f",V_HTKparamkind[kind&V_KIND_BASEMASK],nrv,vsz,T0,vct[0],vct[1],vct[2]);
  }
 BOOL Load(LPSTR);
 void Variance(LPSTR,float,float,NAT);
 void VarianceSp(LPSTR,char*,SpeakerSegs*,float,float,NAT);
 void MinMaxEn();
 void MinMaxC1C2();
 void DrawEn(Image*,SpeakerSegs*,SpeakerSet*);
 void DrawC1C2(Image*,SpeakerSegs*,SpeakerSet*);
};

//................................................................................................
BOOL VHTKparams::Load(LPSTR path)
{
IOSFile iof;
V_HTK_HEADER htkhead;
NAT st,n;
if(iof.open(path,FU_R|FF_WAIT))
 return 0;
ifn(ios.exist(path))
 return 0;
iof.read(&htkhead,sizeof(htkhead));
//swap byte order
htkhead.NrF=BswapDW(htkhead.NrF);
htkhead.T0=BswapDW(htkhead.T0); //convert 100ns -> ms
htkhead.BpS=BswapW(htkhead.BpS);
htkhead.paramkind=BswapW(htkhead.paramkind);
if(kind&&vct.nrit)
 {
 if(kind!=htkhead.paramkind)
  {
  error("Can't append because of different format");
  return 0;
  }
 }
else
 kind=htkhead.paramkind;
vsz=htkhead.BpS/4;
//nrv=(iof.size()-sizeof(htkhead))/htkhead.BpS;
T0=htkhead.T0/10;
n=htkhead.NrF;
//printbox("NrF=%u T0=%ums f=%.1fHz\nBpS=%u VSZ=%u\n%s",htkhead.NrF,T0/1000,1000000./T0,htkhead.BpS,vsz,V_HTKparamkind[htkhead.paramkind&V_KIND_BASEMASK]);
st=vct.nrit;
vct.dim(st+n*vsz);
nrv=vct.nrit/vsz;
iof.read(vct.item+st,n*htkhead.BpS);
BswapDW((DWORD*)(vct.item+st),n*vsz);
//TODO: set these based on kind
en=vsz/4-1;
c1=0;
c2=1;
//error("see vect");
return 1;
}

#define V_SEP_CHAR '\t'

//................................................................................................
void VHTKparams::Variance(LPSTR path,float lwndstep=100000.,float lwndsize=250000.,NAT nrceps=12)
{
ifn(nrv) return;
mean.dim(vsz);
mean.Zero();
var.dim(vsz);
var.Zero();
NAT v,i;
float*pv=vct.item;
for(v=0;v<nrv;v++)
 {
 for(i=0;i<vsz;i++)
  {
  mean.item[i]+=pv[i];
  var.item[i]+=pv[i]*pv[i];
  }
 pv+=vsz;
 }
for(NAT i=0;i<vsz;i++)
 {
 mean.item[i]/=nrv;
 var.item[i]=((var[i]/nrv)-(mean[i]*mean[i]));//D(v)=M(v^2)-M^2(v)
 }
vart.Clone(&var);
ifn(path) return;
IOSFile iof;
if(erret=iof.open(path,FU_WO))
 {
 printbox("Couldn't create: %s",iof.filepath);
 return;
 }
//iof.printf("\r\n\r\n");
//iof.printf("File %s\r\n",snd.l);
iof.printf("WndStep %f us\r\n",lwndstep);
iof.printf("WndSize %f us\r\n",lwndsize);
iof.printf("Nr ceps %u\r\n",nrceps);
iof.printf("\r\n");
iof.printf("AVG(C)\tVAR(C)\tAVG(dC)\tVAR(dC)\r\n");
for(NAT i=0;i<vsz/4;i++)
 {
 //iof.printf("%.2f\t%.2f\t%.2f\t%.2f\t",mean[i],mean[i+vsz/4],mean[i+vsz/2],mean[i+vsz*3/4]);
 //iof.printf("%.2f\t%.2f\t%.2f\t%.2f\r\n",var[i],var[i+vsz/4],var[i+vsz/2],var[i+vsz*3/4]);
 //iof.printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\r\n",mean[i],var[i],mean[i+vsz/4],var[i+vsz/4],mean[i+vsz/2],var[i+vsz/2]);
 iof.printf("%.2f\t%.2f\t%.2f\t%.2f\r\n",mean[i],var[i],mean[i+vsz/4],var[i+vsz/4]);
 }
}

//................................................................................................
void VHTKparams::VarianceSp(LPSTR path,char*llab,SpeakerSegs*pss,float lwndstep=100000.,float lwndsize=250000.,NAT nrceps=12)
{
ifn(nrv) return;
ifn(vart.nrit) 
 {
 error("To get F ratios, you should first calculate total variance");
 vart.dim(vsz);
 vart.Zero();
 }
mean.dim(vsz);
mean.Zero();
var.dim(vsz);
var.Zero();
F.dim(vsz);
F.Zero();
double avgF[4];
NAT v,i;
float*pv=vct.item;
int seg;
for(v=0;v<nrv;v++)
 {
 seg=pss->WhichSeg(v*T0,(v+1)*T0);
 if(scmp(llab,pss->s[seg].lab))
  {
  for(i=0;i<vsz;i++)
   {
   mean.item[i]+=pv[i];
   var.item[i]+=pv[i]*pv[i];
   }
  } 
 pv+=vsz;
 }
for(NAT i=0;i<vsz;i++)
 {
 mean.item[i]/=nrv;
 var.item[i]=((var[i]/nrv)-(mean[i]*mean[i]));//D(v)=M(v^2)-M^2(v)
 F.item[i]=vart.item[i]/var.item[i];
 //avgF+=F.item[i];
 }
ifn(path) return;
IOSFile iof;
if(erret=iof.open(path,FU_WO))
 {
 printbox("Couldn't create: %s",iof.filepath);
 return;
 }
//iof.printf("\r\n\r\n");
iof.printf("Speaker %s\r\n",llab);
iof.printf("WndStep %f us\r\n",lwndstep);
iof.printf("WndSize %f us\r\n",lwndsize);
iof.printf("Nr ceps %u\r\n",nrceps);
iof.printf("\r\n");
avgF[0]=avgF[1]=avgF[2]=avgF[3]=0.;
iof.printf("AVG(C)\tVAR(C)\tF(C)\tAVG(dC)\tVAR(dC)\tF(dC)\r\n");
for(NAT i=0;i<vsz/4;i++)
 {
 iof.printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\r\n",mean[i],var[i],F[i],mean[i+vsz/4],var[i+vsz/4],F[i+vsz/4]);
 //iof.printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t\r\n",mean[i],var[i],F[i],mean[i+vsz/4],var[i+vsz/4],F[i+vsz/4],mean[i+vsz/2],var[i+vsz/2],F[i+vsz/2]);
 //iof.printf("%.2f\t%.2f\t%.2f\t%.2f\t",mean[i],mean[i+vsz/4],mean[i+vsz/2],mean[i+vsz*3/4]);
 //iof.printf("%.2f\t%.2f\t%.2f\t%.2f\r\n",var[i],var[i+vsz/4],var[i+vsz/2],var[i+vsz*3/4]);
 avgF[0]+=F.item[i];
 avgF[1]+=F.item[i+vsz/4];
 avgF[2]+=F.item[i+vsz/2];
 avgF[3]+=F.item[i+vsz*3/4];
 }
iof.printf("\r\nAverage F =\t%.2f\t%.2f\t%.2f\t%.2f",avgF[0]*4/vsz,avgF[1]*4/vsz,avgF[2]*4/vsz,avgF[3]*4/vsz);
//sprintf(strbuf,"%.2f\t%.2f\t%.2f\t%.2f",avgF[0]*4/vsz,avgF[1]*4/vsz,avgF[2]*4/vsz,avgF[3]*4/vsz);
//StrToClipboard(strbuf,sl(strbuf));
}

//................................................................................................
void VHTKparams::MinMaxEn()
{
minen=10000.;
maxen=-10000.;
float*pv=vct.item;
for(NAT v=0;v<nrv;v++)
 {
 if(pv[en]<minen) minen=pv[en];
 else
 if(pv[en]>maxen) maxen=pv[en];
 pv+=vsz;
 }
}

//................................................................................................
void VHTKparams::MinMaxC1C2()
{
/*
minc1=minc2=10000.;
maxc1=maxc2=-10000.;
float*pv=vct.item;
for(NAT v=0;v<nrv;v++)
 {
 if(pv[c1]<minc1) minc1=pv[c1];
 else if(pv[c1]>maxc1) maxc1=pv[c1];
 if(pv[c2]<minc2) minc2=pv[c2];
 else if(pv[c2]>maxc2) maxc2=pv[c2];
 pv+=vsz;
 }
*/
minc1=minc2=-50.;
maxc1=maxc2=50.;
}

//................................................................................................
void VHTKparams::DrawEn(Image*img,SpeakerSegs*pss,SpeakerSet*pset)
{
ifn(nrv) return;
img->Clear(0);
int x,y,pas,lim;
float hf,vf;
hf=(float)img->lng/nrv;
vf=(float)(img->lat-1)/(maxen-minen);
if(nrv<img->lng)
 pas=127;
else
 pas=CLAMP(nrv/img->lng,96,127);
lim=255-pas;
float*pv=vct.item;
img->color=0xff0000;
int seg;
SpeakerInfo*lpsi;
for(NAT v=0;v<nrv;v++)
 {
 x=v*hf;
 y=(pv[en]-minen)*vf;
 img->at(x,img->lat-1-y);
 seg=pss->WhichSeg(v*T0,(v+1)*T0);
 lpsi=pset->ByLAB(pss->s.item[seg].lab);
 img->color=lpsi->color.dw;
 img->linv(y);
 /*
 if(img->cp->R<lim)
  img->cp->R+=pas;
 else if(img->cp->B<lim)
  img->cp->B+=pas;
 else if(img->cp->G<lim)
  img->cp->G+=pas;
 */ 
 pv+=vsz;
 }
}

//................................................................................................
void VHTKparams::DrawC1C2(Image*img,SpeakerSegs*pss,SpeakerSet*pset)
{
ifn(nrv) return;
img->Clear(0);
int x,y,pas,lim;
float hf,vf;
hf=(float)(img->lng-1)/(maxc1-minc1);
vf=(float)(img->lat-1)/(maxc2-minc2);
pas=127;//CLAMP(nrv/img->lng,50,100);
lim=255-pas;
float*pv=vct.item;
int seg;
SpeakerInfo*lpsi;
for(NAT v=0;v<nrv;v++)
 {
 x=CLAMP((pv[c1]-minc1)*hf,0,img->lng);
 y=CLAMP((pv[c2]-minc2)*vf,0,img->lat);
 seg=pss->WhichSeg(v*T0,(v+1)*T0);
 lpsi=pset->ByLAB(pss->s.item[seg].lab);
 //img->color=lpsi->color.dw;
 img->at(x,img->lat-1-y);
 img->cp->dw=lpsi->color.dw;
 pv+=vsz;
 }
}

//---------------------------------------------------------------------------------------------------
BOOL CreateHCopyCFG(LPSTR path,char*type="MFCC_E_D_A_T",float lwndstep=100000.,float lwndsize=250000.,NAT nrceps=12)
{
FILE*fis;
//sprintf(path,"%s\\HCopy %s %.fms %.fms %u.cfg",folder,type,lwndstep/10000.,lwndsize/10000.,nrceps);
//sprintf(path,"%s\\HCopy.cfg",folder,type,lwndstep/10000.,lwndsize/10000.,nrceps);
ios.del(path,0);
ifn(fis=fopen(path,"w+t"))
 {
 error("Can't creat Hcopy.cfg");
 return 0;
 }
fprintf(fis,"SOURCEFORMAT=WAV\r\n");
fprintf(fis,"TARGETKIND=%s\r\n",type);
fprintf(fis,"TARGETRATE=%.f\r\n",lwndstep*10.); //pasul ferestrei  [100ns=0.1us=0.0001ms]
fprintf(fis,"SAVECOMPRESSED=F\r\n");
fprintf(fis,"SAVEWITHCRC=F\r\n");
fprintf(fis,"WINDOWSIZE=%.f\r\n",lwndsize*10.); //dimensiunea ferestrei [100ns=0.1us=0.0001ms]
fprintf(fis,"\r\n");
fprintf(fis,"USEHAMMING=T\r\n");
fprintf(fis,"ZMEANSOURCE=%c\r\n",'T'); //remove DC
fprintf(fis,"PREEMCOEF=%f\r\n",0.97);
fprintf(fis,"\r\n");
fprintf(fis,"NUMCEPS=%u\r\n",nrceps); //numarul de parametrii cepstrali
fprintf(fis,"NUMCHANS=%u\r\n",26); //numarul de canale in bancul de filtre
fprintf(fis,"CEPLIFTER=%u\r\n",22); //lifter value for cepstral
fprintf(fis,"ENORMALISE=%c\r\n",'T');
//fprintf(fis,"USEPOWER=%c\r\n",'T'); //use FFT power instead of magnitude
fclose(fis);
SetFileAttributes(path,GetFileAttributes(path)|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_ARCHIVE);
return 1;
}

//---------------------------------------------------------------------------------------------------
BOOL CreateHCopyFileList(LPSTR path,LPSTR src,LPSTR dst)
{
FILE*fis;
ifn(fis=fopen(path,"wt"))
 return 0;
fprintf(fis,"%s %s",src,dst);
}

//-----------------------------------------------------------------------------------------------
void GetMFCC(VHTKparams*pmfcc,LPSTR wavfile,float lwndstep=100000.,float lwndsize=250000.,NAT nrceps=12,char*type="MFCC_E_D_A_T")
{
char mfccfile[PATHSZ],folder[PATHSZ],args[256];
SplitPath(wavfile,folder,mfccfile);
ios.cd(folder);
sc(mfccfile,wavfile);
//sc(wavfile,mfccfile);
changefileext(mfccfile,"mfcc");
ios.del(mfccfile,0);
ifn(ios.exist(mfccfile))
 {
 CreateHCopyCFG("HCopy.cfg",type,lwndstep,lwndsize,nrceps);
 //CreateHCopyFileList("HCopy.lst",wavfile,mfccfile);
 //printbox(args);
 //if((int)ShellExecute(NULL,NULL,"HCopy",args,folder,SW_SHOWMAXIMIZED)<=32)
 #ifdef _DEBUG
 sprintf(args,"/D /C \"HCopy -T 7 -C %s %s %s\"","HCopy.cfg",wavfile,mfccfile);
 if((int)ShellExecute(NULL,NULL,"cmd.exe",args,folder,SW_SHOWMAXIMIZED)<=32)
 #else
 sprintf(args,"/D /C \"HCopy -C %s %s %s\"","HCopy.cfg",wavfile,mfccfile);
 if((int)ShellExecute(NULL,NULL,"cmd.exe",args,folder,SW_HIDE)<=32)
 #endif
  {
  error("Couldn't execute HCopy");
  return;
  }
 } 
pmfcc->Free();
pmfcc->Load(mfccfile);
//SetFileAttributes(mfccfile,GetFileAttributes(mfccfile)|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_ARCHIVE);
}

//-----------------------------------------------------------------------------------------------
BOOL RunHVITE(LPSTR wavfile,LPSTR hmmfolder)
{
char mfccfile[PATHSZ],args[256];
ios.cd(hmmfolder);
sc(mfccfile,wavfile);
changefileext(mfccfile,"mfc");
ifn(ios.exist(mfccfile))
 {
 #ifdef _DEBUG
 sprintf(args,"/D /K \"HCopy -T 7 -C config %s %s\"",wavfile,mfccfile);
 if((int)ShellExecute(NULL,NULL,"cmd.exe",args,hmmfolder,SW_SHOWMAXIMIZED)<=32)
 #else
 sprintf(args,"/D /C \"HCopy -C config %s %s\"",wavfile,mfccfile);
 if((int)ShellExecute(NULL,NULL,"cmd.exe",args,hmmfolder,SW_HIDE)<=32)
 #endif
  {
  error("Couldn't parametrize");
  return 0;
  }
 } 
//hvite -T 1 -o S -d dest_HRest -l Rezultate_total -w spnet dict hmmlist data\sp1\sp1_1.mfc
#ifdef _DEBUG
sprintf(args,"/D /K \"hvite -T 7 -o S -d dest_HRest -w spnet dict hmmlist %s\"",mfccfile);
//printbox(args);
if((int)ShellExecute(NULL,NULL,"cmd.exe",args,hmmfolder,SW_SHOWMAXIMIZED)<=32)
#else
sprintf(args,"/D /C \"hvite -o S -d dest_HRest -w spnet dict hmmlist %s\"",mfccfile);
if((int)ShellExecute(NULL,NULL,"cmd.exe",args,hmmfolder,SW_HIDE)<=32)
#endif
 {
 error("Couldn't recognize");
 return 0;
 }
return 1;
}