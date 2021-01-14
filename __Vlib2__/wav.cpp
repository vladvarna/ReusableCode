#pragma once
#define V_WAVEFILE

#include <wmm.cpp>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class WAVsound:public UFILEAudio
{
public:
 WAVEFORMATEX wf; //
 FILE *Fwav; //file handle
 DWORD fmtoff,fmtsz; //begining of "fmt " chunk + 8 ,format chunk size
 DWORD dataoff,datasz; //offset of "data" chunk in file + 8,sound size in B
 
 virtual void Set(DWORD,NAT); //inits the object
 virtual FAIL Open(LPSTR,WAVEFORMATEX*); //open
 virtual FAIL Save(LPSTR,WAVEFORMATEX*); //create/overwrite
 virtual void Free();
   
 virtual NAT Seek(NAT); //seeks a frame
 virtual NAT BlocksLeft(); //estimated nr. of blocks left in file
 virtual NAT GetF(void*,NAT); //reads frame
 virtual NAT PutF(void*,NAT); //
 
 WAVsound() { ZEROCLASSV(WAVsound); }
 ~WAVsound() { Free(); }

#ifdef V_IDXSOUND8
 IDirectSoundBuffer8* DSBuffer(IDirectSoundBuffer8* dsbuffer8=NULL,DWORD flags=0);
#endif
};

//..............................................................................................
void WAVsound::Set(DWORD sampform=0x11005622,NAT nrf=0)
{
sf=sampform;
wf=SFtoWF(sampform);
if(nrf>0) NrF=nrf;
fmtoff=20; //12+8
fmtsz=16; //sizeof(WAVEFORMATEX)-2
dataoff=fmtoff+fmtsz+8;
datasz=NrF*wf.nBlockAlign;
cursor=0;
}
 
//..............................................................................................
void WAVsound::Free()
{
FCLOSE(Fwav);
}

//..............................................................................................
FAIL WAVsound::Open(LPSTR filename=NULL,WAVEFORMATEX*psugestedwf=NULL) //psugestedwf is for UFILEAudio
{
if(Fwav) fclose(Fwav);
Fwav=NULL;
ifn(filename) return 0; //close ok
Fwav=FOPEN(filename,"r+b");
ifn(Fwav) return 1; //can't open file
fread(&fmtoff,12,1,Fwav);
if(fmtoff!=FOURCC("RIFF")||dataoff!=FOURCC("WAVE"))
 {
 fclose(Fwav);
 Fwav=NULL;
 return 2; //not a RIFF or WAV
 }
if(!FindRIFFChunk(fmtoff,fmtsz,Fwav,FOURCC("fmt "),0)||
   !FindRIFFChunk(dataoff,datasz,Fwav,FOURCC("data"),0))
 {
 fclose(Fwav);
 Fwav=NULL;
 return 3; //required chunks not found
 }
fmtoff+=8;
dataoff+=8;
fseek(Fwav,fmtoff,SEEK_SET);
fread(&wf,fmtsz,1,Fwav);
if(wf.wFormatTag!=1)
 {
 fclose(Fwav);
 Fwav=NULL;
 return 4; //only PCM supported
 }
sf=WFtoSF(wf);
NrF=datasz/wf.nBlockAlign;
cursor=0;
return 0; //Ok
}

//..............................................................................................
FAIL WAVsound::Save(LPSTR filename=NULL,WAVEFORMATEX*psugestedwf=NULL)
{
if(filename) //create new file
 {
 if(Fwav) fclose(Fwav);
 Fwav=FOPEN(filename,"w+b");
 }
if(!Fwav) return 1; //no file to write to
fseek(Fwav,0,SEEK_SET);
fwrite("RIFF",4,1,Fwav);
fpuioi=fmtsz+datasz+20; //4+8+fmtsz+8+datasz
fwrite(&fpuioi,4,1,Fwav);
fwrite("WAVE",4,1,Fwav);
fwrite("fmt ",4,1,Fwav);
fwrite(&fmtsz,4,1,Fwav);
fwrite(&wf,fmtsz,1,Fwav);
fwrite("data",4,1,Fwav);
fwrite(&datasz,4,1,Fwav);
return 0; //Ok
}

//..............................................................................................
inline NAT WAVsound::Seek(NAT fi=0)
{
cursor=fi;
eof=cursor>NrF;
return cursor;
}

//..............................................................................................
inline NAT WAVsound::BlocksLeft()
{
if(cursor>=NrF)
 return 0;
if(FpB)
 return (NrF-cursor)/FpB+((NrF-cursor)%FpB==0?0:1);
return -1; //undefined numeber left 
}

//..............................................................................................
NAT WAVsound::GetF(void*frmbuf,NAT fcnt=0)
{
ifn(Fwav) return 0;
ifn(fcnt) fcnt=FpB;
if(cursor>=NrF) cursor=0;
if(cursor+fcnt>=NrF)
 {
 eof=1;
 ZeroMemory((BYTE*)frmbuf+(NrF-cursor)*wf.nBlockAlign,(fcnt-NrF+cursor)*wf.nBlockAlign);
 fcnt=NrF-cursor;
 }
fseek(Fwav,dataoff+cursor*wf.nBlockAlign,SEEK_SET);
fread(frmbuf,fcnt*wf.nBlockAlign,1,Fwav);
cursor+=fcnt;
return fcnt;
}

//..............................................................................................
NAT WAVsound::PutF(void*frmbuf,NAT fcnt=0)
{
if(!Fwav||!fcnt) return 0;
fseek(Fwav,dataoff+cursor*wf.nBlockAlign,SEEK_SET);
fwrite(frmbuf,fcnt*wf.nBlockAlign,1,Fwav);
cursor+=fcnt;
if(cursor>NrF) //resize file
 {
 NrF=cursor;
 datasz=NrF*wf.nBlockAlign;
 fseek(Fwav,dataoff-4,SEEK_SET);
 fwrite(&datasz,4,1,Fwav);
 fcnt=fmtsz+datasz+20;
 fseek(Fwav,4,SEEK_SET);
 fwrite(&fcnt,4,1,Fwav);
 }
return fcnt;
}

#ifdef V_IDXSOUND8

//..............................................................................................
 IDirectSoundBuffer8* WAVsound::DSBuffer(IDirectSoundBuffer8* dsbuffer8,DWORD flags)
 {
 if(!Fwav) return dsbuffer8; //no file no change
 DELCOM(dsbuffer8);
 dsbuffer8=MakeDSB8(wf,flags,datasz);
 if(!dsbuffer8) return NULL;
 void* plck;
 DWORD lckb;
 if(dsbuffer8->Lock(0,datasz,&plck,&lckb,NULL,NULL,DSBLOCK_ENTIREBUFFER)!=DS_OK)
  {
  dsbuffer8->Release();
  return NULL;
  }
 fseek(Fwav,dataoff,SEEK_SET);
 fread(plck,datasz,1,Fwav);
 dsbuffer8->Unlock(plck,lckb,NULL,0);
 return dsbuffer8;
 }

#endif