#pragma once

#include <img.cpp>
#include <snd.cpp>
#include <rgb.cpp>
#include <uarray.cpp>
#include <ustruct.cpp>

#define V_AUDIO_DEF_SAMPFREQ  16000

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct SpeakerInfo
 {
 COLOR color;
 char first_name[16],last_name[16];
 unsigned __int64 uid;
 char lab[32];
 
 void Show()
  {
  printbox("%I64x=>%s<\n%s %s\n%x %x %x",uid,lab,first_name,last_name,color.r,color.g,color.b);
  }
 };

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct SpeakerSeg
 {
 double seg;
 char lab[32];
 
 void Show()
  {
  printbox("%f %s",seg,lab);
  }
 };

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SpeakerSet
{
public:
 FLAGS stat; 
 UARRAY <SpeakerInfo> speaker;

 SpeakerSet()  { ZEROCLASS(SpeakerSet); Init(); }
 ~SpeakerSet() { Free(); }
 NAT Add(char*);
 SpeakerInfo*ByID(unsigned __int64);
 SpeakerInfo*ByLAB(char*);
 void Init();
 void SetName(NAT,char*,char*);
#ifdef V_MFC
 void Combo(CComboBox*);
#endif
 FAIL Load(LPSTR);
 FAIL Save(LPSTR);
 void Print(NAT);
 void Free()
  {
  speaker.Free();
  }
}spkset;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SpeakerSegs
{
public:
 FLAGS stat; 
 UARRAY <SpeakerSeg> s;

 SpeakerSegs()  { ZEROCLASS(SpeakerSegs); }
 ~SpeakerSegs() { Free(); }
 NAT Add(NAT,SpeakerInfo*);
 FAIL LoadLAB(LPSTR);
 FAIL SaveLAB(LPSTR);
 int WhichSeg(float,float);
 int Insert(char*,int,Image*,Sound*);
 int Click(int,Image*,Sound*);
 void Drag(int,Image*,Sound*);
 void Draw(Image*,Sound*);
 void Free()
  {
  s.Free();
  }
}spkseg;

// Set de vorbitori <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//.................................................................................
NAT SpeakerSet::Add(char*llab) 
{
NAT ind;
ind=speaker.dim(-1);
speaker.item[ind].uid=VUID;
sc(speaker.item[ind].lab,llab,sizeof(speaker.item[ind].lab));
RandomColor(&speaker.item[ind].color,100,200);
return ind;
}

//.................................................................................
SpeakerInfo* SpeakerSet::ByID(unsigned __int64 luid)
{
for(int i=0;i<speaker.nrit;i++)
 {
 if(speaker.item[i].uid==luid)
  return &speaker.item[i];
 }
return &speaker.item[0];
}

//.................................................................................
SpeakerInfo* SpeakerSet::ByLAB(char*llab)
{
for(int i=0;i<speaker.nrit;i++)
 {
 if(scmp(speaker.item[i].lab,llab))
  return &speaker.item[i];
 }
return &speaker.item[0];
}

//.................................................................................
void SpeakerSet::SetName(NAT ind,char*fname,char*lname)
{
sc(speaker.item[ind].first_name,fname);
sc(speaker.item[ind].last_name,lname);
}

//.................................................................................
void SpeakerSet::Init() 
{
if(speaker.nrit<1)
 {
 Add("n");
 SetName(0,"Unusable","Junk");
 speaker.item[0].color=0xffffff;
 speaker.item[0].uid=0;
 }
}

#ifdef V_MFC
//.................................................................................
void SpeakerSet::Combo(CComboBox*pcb)
{
char lsbuf[256];
pcb->ResetContent();
for(int i=0;i<speaker.nrit;i++)
 {
 sprintf(lsbuf,"%s",speaker.item[i].lab);
 pcb->AddString(lsbuf);
 }
}
#endif

//.................................................................................
inline FAIL SpeakerSet::Save(LPSTR path)
{
return speaker.SaveStatic(path);
}

//.................................................................................
inline FAIL SpeakerSet::Load(LPSTR path)
{
return speaker.LoadStatic(path);
}

//.................................................................................
void SpeakerSet::Print(NAT ind)
{
//printbox("%I64x\n%s %s\n%x %x %x",speaker.item[ind]->uid,speaker.item[ind]->first_name,speaker.item[ind]->last_name,speaker.item[ind]->color.r,speaker.item[ind]->color.g,speaker.item[ind]->color.b);
speaker.item[ind].Show();
}

// End Set de vorbitori >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Segmente <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//.................................................................................
NAT SpeakerSegs::Add(NAT lseg,SpeakerInfo*psi) 
{
NAT ind;
ind=s.dim(-1);
s.item[ind].seg=lseg;
sc(s.item[ind].lab,psi->lab,sizeof(s.item[ind].lab));
return ind;
}

//.................................................................................
inline FAIL SpeakerSegs::SaveLAB(LPSTR path)
{
FILE*fis;
ifn(s.nrit)
 return 0;
ifn(fis=fopen(path,"wt"))
 return 0;
float a,b;
fprintf(fis,"0 %.f %s\xD\xA",s.item[0].seg*10.,s.item[0].lab);
for(int i=1;i<s.nrit;i++)
 fprintf(fis,"%.f %.f %s\xD\xA",s.item[i-1].seg*10.,s.item[i].seg*10.,s.item[i].lab);
fclose(fis);
return 1;
}

//.................................................................................
inline BOOL SpeakerSegs::LoadLAB(LPSTR path)
{
FILE*fis;
ifn(fis=fopen(path,"rt"))
 return 0;
//unsigned __int64 a,b;
float a,b;
char label[64];
Free();
for(;;)
 {
 //erret=fscanf(fis,"%I64u %I64u %s",&a,&b,label);
 erret=fscanf(fis,"%f %f %s",&a,&b,label);
 if(erret==EOF||erret==0||erret==S_FALSE)
  break;
 s.dim(-1);
 s.item[s.nrit-1].seg=b/10; //microsec
 sc(s.item[s.nrit-1].lab,label,sizeof(s.item[s.nrit-1].lab));
 //printbox("%I64u %I64u %s",a,b,label);
 }
fclose(fis);
return 1;
}

//.................................................................................
int SpeakerSegs::Insert(char*llab,int mousex,Image*pimg,Sound*psnd)
{
double t;
//if(!pimg->img||!psnd->snd||!s.nrit)
// return 0;
t=((double)mousex*psnd->NrF/pimg->lng)/psnd->Fps*1000000.;
int i=0;
for(;i<s.nrit;i++)
 {
 if(t<s.item[i].seg)
  break;
 }
s.ins(i,1);
s.item[i].seg=t;
sc(s.item[i].lab,llab,sizeof(s.item[i].lab));
return i;
}

//.................................................................................
int SpeakerSegs::WhichSeg(float a,float b)
{
ifn(s.nrit)
 return 0;
int i=0;
for(;i<s.nrit;i++)
 {
 if(a<s.item[i].seg)
  break;
 }
if(i>=s.nrit-1)
 return s.nrit-1;
if(b<s.item[i].seg)
 return i;
if((s.item[i].seg-a)>(b-s.item[i].seg))
 return i;
else
 return i+1;
}

//.................................................................................
int SpeakerSegs::Click(int mousex,Image*pimg,Sound*psnd)
{
double t;
if(!pimg->img||!psnd->snd||!s.nrit)
 return 0;
t=((double)mousex*psnd->NrF/pimg->lng)/psnd->Fps*1000000.;
for(int i=0;i<s.nrit;i++)
 {
 if(t<s.item[i].seg)
  return i;
 }
return s.nrit-1;
}

//.................................................................................
void SpeakerSegs::Drag(int mousex,Image*pimg,Sound*psnd)
{
double t;
if(!pimg->img||!psnd->snd||!s.nrit)
 return;
t=((double)mousex*psnd->NrF/pimg->lng)/psnd->Fps*1000000.;
int mini=0;
for(int i=1;i<s.nrit;i++)
 {
 if(ABS(t-s.item[i].seg)<ABS(t-s.item[mini].seg))
  mini=i;
 }
s.item[mini].seg=t;
}

//.................................................................................
void SpeakerSegs::Draw(Image*pimg,Sound*psnd)
{
SpeakerInfo*lpsi;
if(!pimg->img||!psnd->snd||!s.nrit)
 {
 pimg->Clear(0);
 return;
 }
RCT lr;
lr(0,0,0,pimg->lat);
for(int i=0;i<s.nrit;i++)
 {
 lpsi=spkset.ByLAB(s.item[i].lab);
 lr.l=i>0?((double)s.item[i-1].seg/1000000.*psnd->Fps)*pimg->lng/psnd->NrF:0;
 lr.r=((double)s.item[i].seg/1000000.*psnd->Fps)*pimg->lng/psnd->NrF;
 pimg->color=lpsi->color.dw;
 pimg->bar(lr.l,lr.u,lr.r,lr.d);
 SetTextColor(pimg->mdc,notRGB(pimg->color));
 DrawText(pimg->mdc,s.item[i].lab,sl(s.item[i].lab),lr,DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS|DT_SINGLELINE);
 }
if(lr.r<pimg->lng)
 {
 lr.l=lr.r;
 lr.r=pimg->lng;
 pimg->color=0;
 pimg->bar(lr.l,lr.u,lr.r,lr.d);
 }
}

// End Segmente >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
