#pragma once
#define V_WINDOWS_MMEDIA

#include <mmed.cpp>

void CALLBACK SoundRecwaveINProc(HWAVEIN,UINT,DWORD,DWORD,DWORD); //callback for SoundRec

//tests for a last error --------------------------------------------------------------------------------------
int isMMER(int retv,LPSTR errmsg=NULL)
{
//retv=GetLastError();
if(retv==MMSYSERR_NOERROR) return 0;
LPVOID errbuf;
FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
              NULL,retv,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR)&errbuf,0,NULL);
retv=MessageBox(HWND_DESKTOP,(LPTSTR)errbuf,errmsg,MB_ICONERROR|MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_TOPMOST);
LocalFree(errbuf);
if(retv==IDABORT) exit(1);
if(retv==IDRETRY) DebugBreak();//return 1;
return 0;
}

// WaveformEnv <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
class WaveformEnv
{
public:
 NAT nrO,nrI,nrA,nrM; //number of out,in,aux and mixer devices
 WAVEOUTCAPS*capsO;
 WAVEINCAPS*capsI;
 AUXCAPS*capsA;
 MIXERCAPS*capsM;

 int defO,defI,defM; //default devices
 HWAVEOUT hwavO;
 HWAVEIN hwavI;
 WAVEFORMATEX wfO,wfI; //
 MIXERLINE mixO,mixI;
 
//............................................................................................................
 WaveformEnv()
  {
  NAT d;
  ZEROCLASS(WaveformEnv);
  nrO=waveOutGetNumDevs();
  nrI=waveInGetNumDevs();
  nrA=auxGetNumDevs();
  nrM=mixerGetNumDevs();
  capsO=(WAVEOUTCAPS*)ALLOC(nrO*sizeof(WAVEOUTCAPS));
  capsI=(WAVEINCAPS*)ALLOC(nrI*sizeof(WAVEINCAPS));
  capsA=(AUXCAPS*)ALLOC(nrA*sizeof(AUXCAPS));
  capsM=(MIXERCAPS*)ALLOC(nrM*sizeof(MIXERCAPS));
  for(d=0;d<nrM;d++)
   mixerGetDevCaps(d,capsM+d,sizeof(MIXERCAPS));
  for(d=0;d<nrA;d++)
   auxGetDevCaps(d,capsA+d,sizeof(AUXCAPS));
  for(d=0;d<nrI;d++)
   waveInGetDevCaps(d,capsI+d,sizeof(WAVEINCAPS));
  for(d=0;d<nrO;d++)
   waveOutGetDevCaps(d,capsO+d,sizeof(WAVEOUTCAPS));
  InitMixers();
  }
//...............................................................................................................
 ~WaveformEnv()
  {
  CloseIN();
  CloseOUT();
  FREE(capsO);
  FREE(capsI);
  FREE(capsA);
  FREE(capsM);
  //nrO=nrI=nrA=nrM=0;
  }
//....................................................................................................................
 int Select(int mod=0xf)
  {
  if(mod&0x1) defO=InputChoiceL("Wave form output devices",hmwnd,capsO->szPname,nrO,sizeof(WAVEOUTCAPS));
  if(mod&0x2) defI=InputChoiceL("Wave form input devices",hmwnd,capsI->szPname,nrI,sizeof(WAVEINCAPS));
  if(mod&0x8) defM=InputChoiceL("Mixers",hmwnd,capsM->szPname,nrM,sizeof(MIXERCAPS));
  if(mod&0x4) return InputChoiceL("Wave form auxiliary devices",hmwnd,capsA->szPname,nrA,sizeof(AUXCAPS));
  //InitMixers();
  return 0;
  }
//Updates destination line for default in&out devices ...........................................................................................
 void InitMixers()
  {
  //defM=InputChoiceL("Mixers",hmwnd,capsM->szPname,nrM,sizeof(MIXERCAPS));
  mixO.cbStruct=sizeof(MIXERLINE);
  mixO.Target.dwType=MIXERLINE_TARGETTYPE_WAVEOUT;
  mixO.Target.wMid=capsO[defO].wMid;
  mixO.Target.wPid=capsO[defO].wPid;
  mixO.Target.vDriverVersion=capsO[defO].vDriverVersion;
  CopyMemory(mixO.Target.szPname,capsO[defO].szPname,sizeof(capsO[defO].szPname));
  mixerGetLineInfo((HMIXEROBJ)defM,&mixO,MIXER_GETLINEINFOF_TARGETTYPE|MIXER_OBJECTF_MIXER);
  mixI.cbStruct=sizeof(MIXERLINE);
  mixI.Target.dwType=MIXERLINE_TARGETTYPE_WAVEIN;
  mixI.Target.wMid=capsI[defI].wMid;
  mixI.Target.wPid=capsI[defI].wPid;
  mixI.Target.vDriverVersion=capsI[defI].vDriverVersion;
  CopyMemory(mixI.Target.szPname,capsI[defI].szPname,sizeof(capsI[defI].szPname));
  mixerGetLineInfo((HMIXEROBJ)defM,&mixI,MIXER_GETLINEINFOF_TARGETTYPE|MIXER_OBJECTF_MIXER);
  }
//.............................................................................................................
 void CloseOUT()
  {
  ifn(hwavO) return;
  waveOutReset(hwavO);
  waveOutClose(hwavO);
  hwavO=NULL;
  }
//..............................................................................................................
 BOOL OpenOUT(DWORD sampformat,int mod=0)
  {
  if(mod&0xf) defO=InputChoiceL("Wave form output devices",hmwnd,capsO->szPname,nrO,sizeof(WAVEOUTCAPS));
  CloseOUT();
  wfO=SFtoWF(sampformat);
  if(erret=waveOutOpen(&hwavO,defO,&wfO,NULL,0,0)!=MMSYSERR_NOERROR)
   {
   isMMER(erret,"waveOutOpen() in OpenOUT() failed");
   return 0;
   }
  return 1; //ok 
  }
//...............................................................................................................
 void CloseIN()
  {
  ifn(hwavI) return;
  waveInReset(hwavI);
  waveInClose(hwavI);
  hwavI=NULL;
  }
//...........................................................................................
 BOOL OpenIN(DWORD sampformat,int mod=0,void*context=NULL)
  {
  if(mod&0x2) defI=InputChoiceL("Wave form input devices",hmwnd,capsI->szPname,nrI,sizeof(WAVEINCAPS));
  CloseIN();
  wfI=SFtoWF(sampformat);
  if(erret=waveInOpen(&hwavI,defI,&wfI,(DWORD_PTR)SoundRecwaveINProc,(DWORD_PTR)context,CALLBACK_FUNCTION)!=MMSYSERR_NOERROR)
   {
   isMMER(erret,"waveInOpen() in OpenIN() failed");
   return 0;
   }
  return 1; //ok 
  }
}wfenv;
// WaveformEnv >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// WAudioMixer <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
class WAudioMixer
{
public:
 NAT Lines,Ctrls; //lines, controls
 MIXERLINE*line;
 MIXERCONTROL*ctrl;
 int actL,actC; //active line, active control
 
//...........................................................................................
 WAudioMixer() { ZEROCLASS(WAudioMixer); }
//...........................................................................................
 ~WAudioMixer() { Free(); }
//...........................................................................................
 void Free()
  {
  FREE(ctrl);
  FREE(line);
  ZEROCLASS(WAudioMixer);  
  }
//...........................................................................................
 void EnumDestinations()
  {
  Free();
  Lines=wfenv.capsM[wfenv.defM].cDestinations;
  line=(MIXERLINE*)ALLOC(Lines*sizeof(MIXERLINE));
  for(NAT d=0;d<Lines;d++)
   {
   line[d].cbStruct=sizeof(MIXERLINE);
   line[d].dwDestination=d;
   mixerGetLineInfo((HMIXEROBJ)wfenv.defM,line+d,MIXER_GETLINEINFOF_DESTINATION|MIXER_OBJECTF_MIXER);
   }
  }
//...........................................................................................
 void EnumSources(MIXERLINE ml)
  {
  Free();
  Lines=ml.cConnections;
  line=(MIXERLINE*)ALLOC(Lines*sizeof(MIXERLINE));
  for(NAT d=0;d<Lines;d++)
   {
   line[d].cbStruct=sizeof(MIXERLINE);
   line[d].dwDestination=ml.dwDestination;
   line[d].dwSource=d;
   mixerGetLineInfo((HMIXEROBJ)wfenv.defM,line+d,MIXER_GETLINEINFOF_SOURCE|MIXER_OBJECTF_MIXER);
   }
  }
//...........................................................................................
 void GetLine(DWORD comptype=0)
  {
  Free();
  Lines=1;
  line=(MIXERLINE*)ALLOC(Lines*sizeof(MIXERLINE));
  line->cbStruct=sizeof(MIXERLINE);
  line->dwComponentType=comptype;
  erret=mixerGetLineInfo((HMIXEROBJ)wfenv.defM,line,MIXER_GETLINEINFOF_COMPONENTTYPE|MIXER_OBJECTF_MIXER);
  if(erret!=MMSYSERR_NOERROR)
   {
   #ifdef _DEBUG
   printbox("Mixer line %x not found",comptype);
   #endif
   Free();
   }
  }
//...........................................................................................
 int SelectLine()
  {
  ifn(line) return 0;
  return actL=InputChoiceL("Lines",hmwnd,line->szName,Lines,sizeof(MIXERLINE),actL);
  }
//...........................................................................................
 void EnumControls()
  {
  MIXERLINECONTROLS mlc;
  ifn(line) return;
  FREE(ctrl);
  actC=0;
  mlc.cbStruct=sizeof(MIXERLINECONTROLS);
  mlc.cControls=Ctrls=line[actL].cControls;
  mlc.cbmxctrl=sizeof(MIXERCONTROL);
  mlc.pamxctrl=ctrl=(MIXERCONTROL*)ALLOC(Ctrls*mlc.cbmxctrl);
  mlc.dwLineID=line[actL].dwLineID;
  erret=mixerGetLineControls((HMIXEROBJ)wfenv.defM,&mlc,MIXER_GETLINECONTROLSF_ALL|MIXER_OBJECTF_MIXER);
  }
//...........................................................................................
 BOOL GetControl(DWORD ctrltype=MIXERCONTROL_CONTROLTYPE_MUTE)
  {
  MIXERLINECONTROLS mlc;
  ifn(line) return 0;
  FREE(ctrl);
  actC=0;
  mlc.cbStruct=sizeof(MIXERLINECONTROLS);
  mlc.cControls=Ctrls=1;
  mlc.cbmxctrl=sizeof(MIXERCONTROL);
  mlc.pamxctrl=ctrl=(MIXERCONTROL*)ALLOC(Ctrls*mlc.cbmxctrl);
  mlc.dwLineID=line[actL].dwLineID;
  mlc.dwControlType=ctrltype;
  erret=mixerGetLineControls((HMIXEROBJ)wfenv.defM,&mlc,MIXER_GETLINECONTROLSF_ONEBYTYPE|MIXER_OBJECTF_MIXER);
  if(erret==MMSYSERR_NOERROR)
   return 1;
  else
   FREE(ctrl);
  return 0;
  }
//...........................................................................................
 int SelectControl()
  {
  ifn(line) return 0;
  ifn(ctrl) return 0;
  return actC=InputChoiceL(line[actL].szName,hmwnd,ctrl->szName,Ctrls,sizeof(MIXERCONTROL));
  }
//...........................................................................................
 int ShowControlList()
  {
  ifn(line) return 0;
  ifn(ctrl) return 0;
  int value=0;
  MIXERCONTROLDETAILS mcd;
  MIXERCONTROLDETAILS_LISTTEXT*detail;
  mcd.cbStruct=sizeof(MIXERCONTROLDETAILS);
  mcd.dwControlID=ctrl[actC].dwControlID;
  mcd.cChannels=1;
  mcd.cMultipleItems=ctrl[actC].cMultipleItems;
  mcd.cbDetails=sizeof(MIXERCONTROLDETAILS_LISTTEXT);
  mcd.paDetails=detail=(MIXERCONTROLDETAILS_LISTTEXT*)ALLOC(mcd.cbDetails*(mcd.cMultipleItems?mcd.cMultipleItems:1));
  erret=mixerGetControlDetails((HMIXEROBJ)wfenv.defM,&mcd,MIXER_GETCONTROLDETAILSF_LISTTEXT|MIXER_OBJECTF_MIXER);
  if(erret!=MMSYSERR_NOERROR) 
   {
   FREE(detail);
   return 0;
   }
  value=InputChoiceL(ctrl[actC].szName,hmwnd,detail->szName,mcd.cMultipleItems,sizeof(MIXERCONTROLDETAILS_LISTTEXT),GetControlDetails());
  FREE(detail);
  return value; 
  }
//...........................................................................................
 int GetControlDetails()
  {
  ifn(line) return 0;
  ifn(ctrl) return 0;
  int value=0;
  MIXERCONTROLDETAILS mcd;
  MIXERCONTROLDETAILS_SIGNED*detail;
  mcd.cbStruct=sizeof(MIXERCONTROLDETAILS);
  mcd.dwControlID=ctrl[actC].dwControlID;
  mcd.cChannels=1;
  mcd.cMultipleItems=ctrl[actC].cMultipleItems;
  mcd.cbDetails=sizeof(MIXERCONTROLDETAILS_SIGNED);
  mcd.paDetails=detail=(MIXERCONTROLDETAILS_SIGNED*)ALLOC(mcd.cbDetails*(mcd.cMultipleItems?mcd.cMultipleItems:1));
  erret=mixerGetControlDetails((HMIXEROBJ)wfenv.defM,&mcd,MIXER_GETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_MIXER);
  //isMMER(erret,"mixerGetControlDetails()");
  if(ctrl[actC].dwControlType==MIXERCONTROL_CONTROLTYPE_MUX||ctrl[actC].dwControlType==MIXERCONTROL_CONTROLTYPE_SINGLESELECT) //MUX is an array with the selected line marked by a 1
   {
   for(int i=0;i<mcd.cMultipleItems;i++)
    if(detail[i].lValue) value=i;
   }  
  else
   value=detail->lValue;
  FREE(detail);
  return value; 
  }
//...........................................................................................
 void SetControlDetails(int value)
  {
  ifn(line) return;
  ifn(ctrl) return;
  MIXERCONTROLDETAILS mcd;
  MIXERCONTROLDETAILS_SIGNED*detail;
  mcd.cbStruct=sizeof(MIXERCONTROLDETAILS);
  mcd.dwControlID=ctrl[actC].dwControlID;
  mcd.cChannels=1;
  mcd.cMultipleItems=ctrl[actC].cMultipleItems;
  if(mcd.cMultipleItems>100) return; //extra safety 
  mcd.cbDetails=sizeof(MIXERCONTROLDETAILS_SIGNED);
  mcd.paDetails=detail=(MIXERCONTROLDETAILS_SIGNED*)ALLOC0(mcd.cbDetails*(mcd.cMultipleItems?mcd.cMultipleItems:1));
  if(ctrl[actC].dwControlType==MIXERCONTROL_CONTROLTYPE_MUX)
   detail[value].lValue=1;
  else
   detail->lValue=value;
  erret=mixerSetControlDetails((HMIXEROBJ)wfenv.defM,&mcd,MIXER_SETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_MIXER);
  //isMMER(erret,"mixerSetControlDetails()");
  FREE(detail);
  }
#ifdef _DEBUG
//Enumeraes controls for all posible lines...........................................................................................
 void DBGEnumAll()
  {
  int l;
  for(l=MIXERLINE_COMPONENTTYPE_SRC_FIRST+1;l<=MIXERLINE_COMPONENTTYPE_SRC_LAST;l++) //playback
   {
   GetLine(l);
   EnumControls();
   SelectControl();
   }
  for(l=MIXERLINE_COMPONENTTYPE_DST_FIRST+1;l<=MIXERLINE_COMPONENTTYPE_DST_LAST;l++) //recording
   {
   GetLine(l);
   EnumControls();
   SelectControl();
   }
  }
#endif
};
// WAudioMixer >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//select recording source -------------------------------------------------------------------
int SelectRecSource(int s=-1)
{
WAudioMixer lmux;
lmux.GetLine(MIXERLINE_COMPONENTTYPE_DST_WAVEIN);
lmux.GetControl(MIXERCONTROL_CONTROLTYPE_MUX);
//lmux.SelectControl();
if(s<0) s=lmux.ShowControlList();
lmux.SetControlDetails(s);
return s;
}

//set recording source volume (also get if v<0)-------------------------------------------------------------------
int SetRecSourceVol(int s,int v=-1)
{
WAudioMixer lmux;
lmux.EnumSources(wfenv.mixI);
lmux.actL=lmux.Lines-s-1; //for some reason, lines are enumerated in reverse here
//lmux.SelectLine();
//lmux.EnumControls();
//lmux.SelectControl();
ifn(lmux.GetControl(MIXERCONTROL_CONTROLTYPE_VOLUME))
 return 0; //error
if(v>=0&&v<=0xffff)
 lmux.SetControlDetails(v);
return lmux.GetControlDetails();
}

//prints a WAVEFORMATEX structure -------------------------------------------------------------------
void dumpwf(WAVEFORMATEX dswf)
{
printbox("%s\n%u fps\n%u bits\n%u channels\nBpF %u\nBpS %u\nExtra %u B",dswf.wFormatTag==1?"PCM":"Comp",dswf.nSamplesPerSec,dswf.wBitsPerSample,dswf.nChannels,dswf.nBlockAlign,dswf.nAvgBytesPerSec,dswf.cbSize);
}

/*
#define WHDR_DONE       0x00000001  // done bit
#define WHDR_PREPARED   0x00000002  // set if this header has been prepared
#define WHDR_BEGINLOOP  0x00000004  // loop start block
#define WHDR_ENDLOOP    0x00000008  // loop end block
#define WHDR_INQUEUE    0x00000010  // reserved for driver
*/

//inits a WAVEHFR --------------------------------------------------------------------
inline void InitWAVEHDR(WAVEHDR*wh,void*ldata,NAT datasz,NAT rep=0,void*context=NULL)
{
wh->lpData=(char*)ldata;
wh->dwBufferLength=datasz;
wh->dwBytesRecorded=0;
wh->dwUser=(DWORD_PTR)context;
wh->dwFlags=!rep?0:WHDR_BEGINLOOP|WHDR_ENDLOOP;
wh->dwLoops=rep; //has meaning only for playback 
}