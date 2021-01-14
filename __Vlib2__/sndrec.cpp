#pragma once
#define V_SOUND_REC

#include <wmm.cpp>
#include <wav.cpp>

#define V_SNDR_REC                  0x4000000 //is playing

class SoundRec;
typedef int(*SoundRecCallBack)(SoundRec*,WAVEHDR*);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SoundRec
{
public:
 DWORD sf; //sample format tag
 NAT BpS,BpF,Bps; //Bytes per: Sample, Frame, sec
 NAT NrC,Fps; //Nr channels, Frames per sec
 NAT NrF,szB; //Frames per buffer, Bytes in each buffer
 FLAGS stat; //mode=PLAY
 WAVEHDR whA,whB; //buffers
 UFILEAudio*precfile; //recorded file
 SoundRecCallBack Notify;
 
 SoundRec() { ZEROCLASS(SoundRec); }
 ~SoundRec() { Free(); }

//..............................................................................................
 FAIL Prepare(LPSTR path,DWORD sampform=0x1100ac44,NAT nrf=44100)
 {
 if(stat&V_SNDR_REC) return 1; //already recording
 if(sampform) sf=sampform;
 BpS=SF_BpS(sf);
 BpF=SF_BpF(sf);
 Bps=SF_Bps(sf);
 NrC=SF_nrC(sf);
 Fps=SF_Fps(sf);
 if(nrf) NrF=nrf;
 szB=NrF*BpF;
 ifn(wfenv.hwavI)
  wfenv.OpenIN(sf,0,this);
 InitWAVEHDR(&whA,ALLOC(szB),szB,BpF,this);
 InitWAVEHDR(&whB,ALLOC(szB),szB,BpF,this);
 waveInPrepareHeader(wfenv.hwavI,&whA,sizeof(WAVEHDR));
 waveInPrepareHeader(wfenv.hwavI,&whB,sizeof(WAVEHDR));
 waveInAddBuffer(wfenv.hwavI,&whA,sizeof(WAVEHDR));
 waveInAddBuffer(wfenv.hwavI,&whB,sizeof(WAVEHDR));
 precfile=new WAVsound();
 precfile->Set(sf,NrF);
 if(precfile->Save(path,NULL))
  return 2; //can't create file
 return 0; //ok
 }
//..............................................................................................
 void StartRec()
 {
 if(wfenv.hwavI&&!(stat&V_SNDR_REC))
  {
  waveInStart(wfenv.hwavI);
  stat|=V_SNDR_REC;
  }
 }
//..............................................................................................
 void StopRec()
 {
 if(wfenv.hwavI&&(stat&V_SNDR_REC))
  {
  stat&=~V_SNDR_REC;
  waveInStop(wfenv.hwavI);
  waveInReset(wfenv.hwavI);
  precfile->Free();
  waveInUnprepareHeader(wfenv.hwavI,&whA,sizeof(WAVEHDR));
  waveInUnprepareHeader(wfenv.hwavI,&whB,sizeof(WAVEHDR));
  }
 }
//Energy ..............................................................................................
 double Energy(int bi=0) //bi= buffer A or B
 {
 if(BpS==2)
  return SquaresSum((WORD*)(bi?whB.lpData:whA.lpData),(bi?whB.dwBytesRecorded:whA.dwBytesRecorded)>>1);
 else //if(BpS==1)
  return SquaresSum((BYTE*)(bi?whB.lpData:whA.lpData),bi?whB.dwBytesRecorded:whA.dwBytesRecorded);
 }
//Full Scale dB ..............................................................................................
 double dBFS(int bi=0)
 {
 double n;
 if(BpS==2) //max=-96.33 dBFS
  {
  n=(bi?whB.dwBytesRecorded:whA.dwBytesRecorded)/2;
  return log10(AbsolutesSum((WORD*)(bi?whB.lpData:whA.lpData),n)/(n*65536.))*20.;
  }
 else //if(BpS==1) //max=-48.1648 dBFS
  {
  n=(bi?whB.dwBytesRecorded:whA.dwBytesRecorded);
  return log10(AbsolutesSum((BYTE*)(bi?whB.lpData:whA.lpData),n)/(n*255.))*20.;
  }
 }
//..............................................................................................
 void Free()
 {
 StopRec();
 wfenv.CloseIN();
 }
};

//default callback for waveIO (auto unprepare and close) -----------------------------
void CALLBACK SoundRecwaveINProc(HWAVEIN hwave,UINT msg,DWORD inst,DWORD param1,DWORD param2)
{
WAVEHDR*pwh;
SoundRec*psrec;
if(msg==WIM_DATA)
 {
 pwh=(WAVEHDR*)param1;
 psrec=(SoundRec*)pwh->dwUser;
 if(pwh->dwBytesRecorded)
  psrec->precfile->PutF(pwh->lpData,pwh->dwBytesRecorded/pwh->dwLoops);
 if(pwh->dwBytesRecorded==pwh->dwBufferLength&&psrec->stat&V_SNDR_REC)//need more buffers
  waveInAddBuffer(hwave,pwh,sizeof(WAVEHDR));
 if(psrec->Notify)
  psrec->Notify(psrec,pwh);
 }
//else if(msg==WIM_OPEN)
//else if(msg==WIM_CLOSE)
}
