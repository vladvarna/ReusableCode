#pragma once
#define V_SOUND

#include <mat.cpp>
#include <rnd.cpp>
#include <sndrec.cpp>
#include <wav.cpp>
#include <compat/sconv.cpp>

#define V_SND_PLAY                 0x10000 //is playing
#define V_SND_PAUSE                0x20000 //is paused
#define V_SND_AUTO_PARAMS         0x100000 //parameters are calculated automatically

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Sound
{
public:
 union
  {
  void *snd;
  BYTE *sndB;
  short*sndW;
  DWORD*sndDW;
  QWORD*sndQW;
  };
 DWORD sf; //sample format tag
 NAT BpS,BpF,Bps; //Bytes per: Sample, Frame, sec
 NAT NrC,NrF,Fps; //Nr channels, Frames, Frames per sec
 NAT szB; //Bytes in each buffer
 double secs; //number of seconds for each buffer
 FLAGS stat; //mode=PLAY
 NAT ch,sel1,sel2; //active channel, selection
 NAT cursor,mark1,mark2; //active position in frames
 float zoom; //how many frames to show in 1 pixel
 WAVEHDR wh; //play buffer
 double abssum,dc; //abs sum
 int min,max; //min/max sample value
 NAT ntz;
 char lastfile[PATHSZ];
 
 Sound() { ZEROCLASS(Sound); }
 ~Sound() { Free(); }
 //general purpose functions
 void Init(DWORD,NAT,void*);
 BOOL Free();
 int From(LPSTR);
 int To(LPSTR);
 void ResetParams();
 void UpdateParams(NAT,NAT);
 void Convert(DWORD); //should adjust the freq
 void SetSelect(NAT,NAT);
 void SetMarkers(NAT,float,int);
 FAIL StartPlay(NAT,NAT);
 void PausePlay();
 void StopPlay();
 NAT GetPlayPos();
  
 //editing tools (16mono recommended)
 void Channel(NAT newch=0);
 void Fill(int way=0,int v1=0,int v2=0,float v3=0);
 void ShowDC(HDC hdc,int x=0,int y=0,int w=0,int h=0,int frm=0);
#ifdef V_IMAGE 
 void Redraw1(Image*,Image*);
 void DrawAll(Image*,int);
 int MarkedStart(NAT);
 void DrawMarked1(Image*,int);
 void DrawHistogram(Image*,NAT,NAT);
 void Draw(Image*,int,int);
#endif
#ifdef V_IDXSOUND8
 #pragma message("Sound DSound8 extensions")
 IDirectSoundBuffer8* DSBuf(IDirectSoundBuffer8* dsbuffer8=NULL,DWORD smpform=0,DWORD flags=0);
 IDirectSoundBuffer8* DSBufX(IDirectSoundBuffer8* dsbuffer8=NULL,DWORD smpform=0,DWORD flags=0);
 BOOL DSCap(IDirectSoundCaptureBuffer8* dscapture8=NULL);
#endif
 
};

// Sound <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//..............................................................................................
inline void Sound::Init(DWORD sampform=0x1100ac44,NAT nrf=0,void*extmem=NULL)
{
if(sampform) sf=sampform;
BpS=SF_BpS(sf);
BpF=SF_BpF(sf);
Bps=SF_Bps(sf);
NrC=SF_nrC(sf);
Fps=SF_Fps(sf);
if(nrf) NrF=nrf;
szB=NrF*BpF;
secs=(double)szB/Bps;
FREE(snd);
if(extmem)
 snd=extmem; //external memory is surrendered ! (used by internal functions)
else
 snd=ALLOC(szB+BpF+4);
ch=0;
mark1=0;
mark2=NrF;
zoom=1.f;
sel1=sel2=0;
ResetParams();
}

//..............................................................................................
inline BOOL Sound::Free()
{
if(wfenv.hwavO)
 {
 if(wh.dwFlags&WHDR_INQUEUE)
  StopPlay();
 if(wh.dwFlags&WHDR_PREPARED)
  waveOutUnprepareHeader(wfenv.hwavO,&wh,sizeof(wh));
 } 
FREE(snd);
szB=NrF=0;
return 1;
}

//..............................................................................................
int Sound::From(LPSTR filename=NULL)
{
WAVsound wavsnd;
char lsbuf[PATHSZ];
FREE(snd);
ifn(filename)
 {
 *lsbuf=0;
 ifn(GetFileName(lsbuf,0,hmwnd,"WAV\0*.wav\0All\0*.*\0","Load wave","wav",NULL,NULL,NULL,'O',NULL))
  return 0;
 filename=lsbuf;
 }
if(wavsnd.Open(filename)) return 1;//file open error
sc(lastfile,filename);
Init(wavsnd.sf,wavsnd.NrF);
wavsnd.GetF(snd,NrF);
//wavsnd.Free();
if(stat&V_SND_AUTO_PARAMS)
 UpdateParams(0,NrF);
return 0;//Ok
}

//..............................................................................................
int Sound::To(LPSTR filename=NULL)
{
if(!snd) return 3; //no data to write
WAVsound wavsnd;
wavsnd.Set(sf,NrF);
if(wavsnd.Save(filename)) return 1;//file access error
wavsnd.PutF(snd,NrF);
return 0;//Ok
}

//..............................................................................................
inline void Sound::Convert(DWORD sampform=SF_KHZ(44.1))
{
ifn(snd) return;
if(sf==sampform||!sampform) return;
SCONV sndconv;
void*locsnd;
sndconv.Tf(sf,sampform);
locsnd=ALLOC((szB+BpF)*sndconv.scaleB+4);
sndconv.Blt(locsnd,snd,NrF);
Init(sampform,NrF*sndconv.scaleF,locsnd);
}

//..............................................................................................
inline void Sound::ResetParams()
{
abssum=0.;
dc=0.;
ntz=0;
if(BpS==1)
 {
 min=255;
 max=0;
 }
else
 {
 max=-0x8000;
 min=0x7fff;
 }
}

//..............................................................................................
inline void Sound::UpdateParams(NAT beg=0,NAT end=-1)
{
if(beg>end) return;
if(end>NrF) end=NrF;
int lbs=0,fv;
if(BpS==2)
 {
 for(int f=beg;f<end;f++)
  {
  fv=sndW[f];
  ntz+=((fv>>15)&1)^lbs;
  lbs=(fv>>15)&1;
  dc+=fv;
  abssum+=ABS(fv);
  if(sndW[f]<min) min=sndW[f];
  if(sndW[f]>max) max=sndW[f];
  }
 }
else 
 {
 for(int f=beg;f<end;f++)
  {
  fv=sndB[f]^0x80;
  ntz+=((fv>>15)&1)^lbs;
  lbs=(fv>>15)&1;
  dc+=fv;
  abssum+=ABS(fv);
  if(sndB[f]<min) min=sndB[f];
  if(sndB[f]>max) max=sndB[f];
  }
 }
dc/=(end-beg);
}

//..............................................................................................
inline void Sound::Channel(NAT newch)
{
ch=newch%NrC;
}
 
//..............................................................................................
void Sound::SetSelect(NAT lsel1,NAT lsel2)
{
sel1=CLAMP(lsel1,0,NrF);
sel2=CLAMP(lsel2,0,NrF);
if(sel1>sel2) swap(sel1,sel2);
}

//..............................................................................................
void Sound::SetMarkers(NAT lcur,float lzoom,int span=0)
{
zoom=CLAMP(lzoom,1.f,MAX(NrF/span,1.f));
cursor=CLAMP(lcur,0,NrF);
span>>=1;
mark1=CLAMP((int)(cursor-zoom*span),0,NrF);
mark2=CLAMP((int)(cursor+zoom*span),0,NrF);
}

//..............................................................................................
inline FAIL Sound::StartPlay(NAT lmark1=0,NAT lmark2=0)
{
if(lmark1>=NrF)
 return 1; //frame is passed the end
if(lmark2<=lmark1||lmark2>=NrF) lmark2=NrF; //play to end
if(wh.dwFlags&WHDR_INQUEUE)
 StopPlay();
if(wh.dwFlags&WHDR_PREPARED)
 waveOutUnprepareHeader(wfenv.hwavO,&wh,sizeof(wh));
//InitWAVEHDR(&wh,sndB+mark1*BpF,(mark2-mark1+1)*BpF,0,0);
wh.lpData=(char*)sndB+lmark1*BpF;
wh.dwBufferLength=(lmark2-lmark1+1)*BpF;
wh.dwBytesRecorded=lmark2; //use this to hold end play position
wh.dwUser=lmark1;
wh.dwFlags=0; //WHDR_BEGINLOOP|WHDR_ENDLOOP;
wh.dwLoops=0; //has meaning only for playback 
ifn(wfenv.OpenOUT(sf))
 return 2; //can't open device
ifn(wh.dwFlags&WHDR_PREPARED)
 waveOutPrepareHeader(wfenv.hwavO,&wh,sizeof(wh));
waveOutWrite(wfenv.hwavO,&wh,sizeof(wh));
stat|=V_SND_PLAY;
return 0; //ok
}

//..............................................................................................
inline void Sound::StopPlay()
{
if(stat&V_SND_PLAY&&wfenv.hwavO)
 {
 waveOutReset(wfenv.hwavO);
 stat&=~(V_SND_PLAY|V_SND_PAUSE);
 }
}

//..............................................................................................
inline void Sound::PausePlay()
{
ifn(wfenv.hwavO) return; //device is already gone
ifn(stat&V_SND_PLAY) return; //already stoped
if(stat&V_SND_PAUSE)
 {
 waveOutRestart(wfenv.hwavO);
 stat&=~V_SND_PAUSE;
 }
else
 {
 waveOutPause(wfenv.hwavO);
 stat|=V_SND_PAUSE;
 }
}

//..............................................................................................
inline NAT Sound::GetPlayPos()
{
MMTIME mmt;
ifn(wfenv.hwavO) return 0;
ifn(wh.dwFlags&WHDR_INQUEUE) return 0;
mmt.wType=TIME_SAMPLES;
waveOutGetPosition(wfenv.hwavO,&mmt,sizeof(mmt));
return mmt.u.sample+wh.dwUser;
}

//..............................................................................................
void Sound::Fill(int way,int v1,int v2,float v3)
{
if(!snd) Init(SAMPFORM(1,16,44100),441000); //default 16mono 44.1KHz 10secs
NAT frm,pak;
if(way==0) //fill sample with v1
 FillMem(snd,&v1,BpS,szB/BpS);
else if(way==1) //fill frame with v1|v2...
 FillMem(snd,&v1,BpF,NrF);
else if(way==2) //fill with noise between [v1,v2] !works per byte!
 fillrnd(sndB,szB,v1,v2);
else if(way==3) //sine: v1=A,v2=T0(in frames),v3=fi0(rad)
 {
 for(frm=0;frm<NrF;frm++)
  {
  if(BpS==1)
   sndB[frm*BpF+ch]=(BYTE)(sin(v3)*v1)^0x80;
  else if(BpS==2)
   sndW[frm*(BpF>>1)+ch]=sin(v3)*v1;
  v3+=_2PI/v2;
  }
 }
else if(way==4) //square: v1=A, v2=T0/2 (in frames)
 {
 for(pak=0;pak<NrF/v2;pak++)
  {
  for(frm=0;frm<v2;frm++)
   {
   if(BpS==1)
    sndB[(pak*v2+frm)*BpF+ch]=(BYTE)(v1)^0x80;
   else if(BpS==2)
    sndW[(pak*v2+frm)*(BpF>>1)+ch]=v1;
   }
  v1=-v1;
  }
 }
}

//..............................................................................................
void Sound::ShowDC(HDC hdc,int x,int y,int w,int h,int frm)
{
int i,j,c;
NAT colors[]={0xff,0xff00,0xff00ff,0xffff00,0xff0000,0xffffff,0xffff};
BitBlt(hdc,x,y,w,h,NULL,0,0,BLACKNESS);
REGi=SetROP2(hdc,R2_MERGEPEN);
for(i=0;i<w;i++,frm++)
 {
 if(frm<0) continue;
 if(frm>=NrF) break;
 for(c=NrC-1;c>=0;c--)
  {
  if(BpS==2) //16 bpS
   j=h-1-(sndW[frm*(BpF>>1)+c]+32768)*(h-1)/0xffff;
  else if(BpS==1) //8 bpS
   j=h-1-sndB[frm*BpF+c]*(h-1)/0xff;
  else
   j=h/2;
  SetPixelV(hdc,x+i,y+j,colors[c]);
  }
 }
SetROP2(hdc,REGi);
}

#ifdef V_IMAGE

#define V_SND_COLOR_VIEWALLBGRND   0x3f00
#define V_SND_COLOR_VIEWMARKBGRND  0x0f
#define V_SND_COLOR_SELECT         0xff0000

//..............................................................................................
void Sound::Redraw1(Image*piall,Image*pibuc)
{
ifn(snd)
 {
 piall->Clear(V_SND_COLOR_VIEWALLBGRND);
 return;
 }
piall->chcolinbar(sel1*piall->lng/NrF,piall->lat*.2,sel2*piall->lng/NrF,piall->lat*.8,V_SND_COLOR_VIEWALLBGRND,V_SND_COLOR_SELECT);
piall->chcolinbar(mark1*piall->lng/NrF,0,mark2*piall->lng/NrF,piall->lat-1,V_SND_COLOR_VIEWALLBGRND,V_SND_COLOR_VIEWMARKBGRND);
}

//..............................................................................................
void Sound::DrawAll(Image*img,int way=0)
{
ifn(snd)
 {
 img->Clear(V_SND_COLOR_VIEWALLBGRND);
 return;
 }
WARN((sf>>20)!=0x110,"Sound - invalid format for function");
int sil,d,f,x,y,pas,lim;
float hf,vf;
sil=img->lat>>1;
hf=(float)img->lng/NrF;
vf=(float)img->lat/0x10000;
pas=CLAMP(NrF/img->lng,50,100);
lim=255-pas;
img->Clear(V_SND_COLOR_VIEWALLBGRND);
img->at(0,sil);
img->color=V_SND_COLOR_VIEWALLBGRND&0x1f1f1f;
img->linh(img->lng);

img->at(0,sil-min*vf);
img->linh(img->lng);

img->at(0,sil-max*vf);
img->linh(img->lng);

img->at(0,sil-vf*CLAMP(dc,min,max));
img->color=0;
img->linh(img->lng);
if(way==1) //bars
 { 
 pas=16;
 for(f=0;f<NrF;f++)
  {
  x=f*hf;
  d=sndW[f]*vf;
  if(d<0)
   {
   for(y=0;y>=d;y--)
    {
    img->at(x,sil-y);
    if(img->cp->G<lim)
     img->cp->G+=pas;
    }
   }
  else
   {
   for(y=0;y<=d;y++)
    {
    img->at(x,sil-y);
    if(img->cp->G<lim)
     img->cp->G+=pas;
    }
   }
  }
 }
else if(way==2) //lines
 {
 for(f=0;f<NrF;f++)
  {
  x=f*hf;
  y=sndW[f]*vf;
  img->at(x,sil-y);
  if(img->cp->B<lim)
   img->cp->B+=pas;
  else if(img->cp->G<lim)
   img->cp->G+=pas;
  else if(img->cp->R<lim)
   img->cp->R+=pas;
  }
 }
}

//helper function to get..............................................................................................
inline int Sound::MarkedStart(NAT w)
{
if(mark1==0)
 return w-mark2/zoom;
else if(mark2==NrF)
 return 0;
else
 return (w-(mark2-mark1)/zoom)/2;
}

//no scaling ..............................................................................................
void Sound::DrawMarked1(Image*img,int way=0)
{
ifn(snd)
 {
 img->Clear(V_SND_COLOR_VIEWMARKBGRND);
 return;
 }
WARN((sf>>20)!=0x110,"Sound - invalid format for function");
int f,y,pas,lim,sil; //channel, silence level(j)
double x;
img->Clear(V_SND_COLOR_VIEWMARKBGRND);
sil=img->lat>>1;
pas=CLAMP(200-(zoom-1)*10,100,200);
lim=255-pas;
if(way==1) //point
 {
 x=MarkedStart(img->lng);
 for(f=mark1;f<mark2;f++)
  {
  y=sndW[f]*sil/0x8000;
  img->at(x,sil-y);
  if(f<sel1||f>sel2)
   {
   if(img->cp->G<lim)
    img->cp->G+=pas;
   else if(img->cp->R<lim)
    img->cp->R+=pas;
   else if(img->cp->B<lim)
    img->cp->B+=pas;
   }
  else
   {
   if(img->cp->R<lim)
    img->cp->R+=pas;
   else if(img->cp->B<lim)
    img->cp->B+=pas;
   else if(img->cp->G<lim)
    img->cp->G+=pas;
   } 
  x+=1./zoom;
  }
 }
img->color=V_SND_COLOR_VIEWALLBGRND;
img->maskA=0xffffff;
img->at(img->lng>>1,0);
img->linv(img->lat); //cursor
}

//no scaling ..............................................................................................
void Sound::DrawHistogram(Image*img,NAT beg,NAT end)
{
ifn(snd)
 {
 img->Clear(0);
 return;
 }
WARN((sf>>20)!=0x110,"Sound - invalid format for function");
double vf;
NAT f,y;
NAT*lhisto;
if(beg>end) return;
if(end>NrF) end=NrF;
lhisto=(NAT*)ALLOC(img->lat<<2);
ZeroMemory(lhisto,img->lat<<2);
vf=(double)img->lat/0x10000;
img->Clear(0);
for(f=beg;f<end;f++)
 lhisto[(NAT)((sndW[f]+32768)*vf)]++;
f=0;
for(y=0;y<img->lat;y++) //find max value for histogram
 if(lhisto[y]>f)
  f=lhisto[y];
vf=(double)(img->lng-1)/f;
img->color=0xff;
for(y=0;y<img->lat;y++)
 {
 img->at(0,y);
 img->linh(lhisto[y]*vf);
 }
}

//no scaling ..............................................................................................
void Sound::Draw(Image*img,int way=0,int frm=0)
{
int lj,i1,i2,i,j,ch,sil; //channel, silence level(j)
POINTER cf,bf; //current and begining frame
TFI1 ltf;
DWORD colors[]={0xff0000,0xff00,0xff00ff,0xffff,0xff,0xffffff,0xffff00};
if(frm<0)
 {
 i1=-frm;
 frm=0;
 }
else
 i1=0;
if(img->lng-i1>NrF-frm)
 i2=i1+NrF-frm;
else
 i2=img->lng;
bf.b=sndB+frm*BpF;
if(BpS==2) //16 bpS
 ltf.Tf(-32768,32767,img->lat-1,0);
else if(BpS==1) //8 bpS
 ltf.Tf(0,255,img->lat-1,0);
else
 ltf.Tf();
img->Clear(V_SND_COLOR_VIEWMARKBGRND);
img->maskA=0xffffff;
j=sil=img->lat>>1;
if(way==1) //points
 {
 for(ch=0;ch<NrC;ch++)
  {
  cf.b=bf.b+ch*BpS;
  for(i=i1;i<i2;i++)
   {
   if(BpS==2) //16 bpS
    j=ltf.f(*cf.s);
   else if(BpS==1) //8 bpS
    j=ltf.f(*cf.b);
   img->color=colors[ch];
   img->pix(i,j);
   cf.b+=BpF; //next frame
   }
  }
 }
else if(way==2) //line
 {
 for(ch=0;ch<NrC;ch++)
  {
  cf.b=bf.b+ch*BpS;
  img->at(i1,sil);
  i1++;
  for(i=i1;i<i2;i++)
   {
   if(BpS==2) //16 bpS
    j=ltf.f(*cf.s);
   else if(BpS==1) //8 bpS
    j=ltf.f(*cf.b);
   img->color=colors[ch];
   img->lin(i,j);
   cf.b+=BpF; //next frame
   }
  }
 }
else if(way==3) //solid
 {
 for(ch=0;ch<NrC;ch++)
  {
  cf.b=bf.b+ch*BpS;
  for(i=i1;i<i2;i++)
   {
   if(BpS==2) //16 bpS
    j=ltf.f(*cf.s);
   else if(BpS==1) //8 bpS
    j=ltf.f(*cf.b);
   img->at(i,sil);
   img->color=colors[ch];
   img->linv(j-sil);
   cf.b+=BpF; //next frame
   }
  }
 }
else if(way==4) //delta between frames
 {
 for(ch=0;ch<NrC;ch++)
  {
  cf.b=bf.b+ch*BpS;
  lj=sil;
  for(i=i1;i<i2;i++)
   {
   if(BpS==2) //16 bpS
    j=ltf.f(*cf.s);
   else if(BpS==1) //8 bpS
    j=ltf.f(*cf.b);
   img->at(i,sil);
   img->color=colors[ch];
   img->linv((j-lj)/2);
   lj=j;
   cf.b+=BpF; //next frame
   }
  }
 }
else if(way==5) //delta between channel and channel media (mix)
 {
 cf.b=bf.b;
 for(i=i1;i<i2;i++)
  {
  lj=0;
  for(ch=0;ch<NrC;ch++)
   {
   if(BpS==2) //16 bpS
    lj+=ltf.f(cf.s[ch]);
   else if(BpS==1) //8 bpS
    lj+=ltf.f(cf.b[ch]);
   }
  lj/=NrC;
  for(ch=0;ch<NrC;ch++)
   {
   if(BpS==2) //16 bpS
    j=ltf.f(*cf.s);
   else if(BpS==1) //8 bpS
    j=ltf.f(*cf.b);
   img->at(i,sil);
   img->color=colors[ch];
   img->linv(j-lj);
   cf.b+=BpS; //next channel
   }
  }
 }
}
#endif

#ifdef V_IDXSOUND8
//..............................................................................................
 IDirectSoundBuffer8* Sound::DSBuf(IDirectSoundBuffer8* dsbuffer8,DWORD smpform,DWORD flags)
 {
 if(!snd) return dsbuffer8; //no data no change
 SCONV sctods;
 if(!smpform) smpform=sf;
 sctods.Tf(sf,smpform);
 DELCOM(dsbuffer8);
 dsbuffer8=MakeDSB8(SFtoWF(smpform),flags,szB*sctods.scaleB);
 if(!dsbuffer8) return NULL;
 void* plck;
 DWORD lckb;
 if(dsbuffer8->Lock(0,0,&plck,&lckb,NULL,NULL,DSBLOCK_ENTIREBUFFER)!=DS_OK)
  {
  dsbuffer8->Release();
  return NULL;
  }
 if(smpform==sf) //don't alter if src==dest
  CopyMemory(plck,snd,lckb);
 else
  sctods.Blt(plck,snd,NrF);
 dsbuffer8->Unlock(plck,lckb,NULL,0);
 return dsbuffer8;
 }

//..............................................................................................
 BOOL Sound::DSCap(IDirectSoundCaptureBuffer8* dscapture8)
 {
 if(!dscapture8) return 0; //no data
 WAVEFORMATEX cwf;
 dscapture8->GetFormat(&cwf,sizeof(cwf),NULL);
 DSCBCAPS dscc;
 dscc.dwSize=sizeof(DSCBCAPS);
 dscapture8->GetCaps(&dscc);
 Init(SAMPFORM(cwf.nChannels,cwf.wBitsPerSample,cwf.nSamplesPerSec),dscc.dwBufferBytes/cwf.nBlockAlign);
 void* plck;
 DWORD lckb;
 if(dscapture8->Lock(0,0,&plck,&lckb,NULL,NULL,DSCBLOCK_ENTIREBUFFER)!=DS_OK)
  return 0;
 CopyMemory(snd,plck,lckb);
 dscapture8->Unlock(plck,lckb,NULL,0);
 return 1;
 }
#endif

// Sound >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>