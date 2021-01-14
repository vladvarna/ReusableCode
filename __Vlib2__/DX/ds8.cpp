#ifndef V_IDXSOUND8
#define V_IDXSOUND8

#include <dx8.cpp>
#include <dsound.h> //dsound.lib
#pragma comment(lib,"dsound.lib")

IDirectSound8 *dsobj=NULL; //default DS playback object
IDirectSoundBuffer *dsbprim=NULL; //primary buffer
IDirectSound3DListener8 *dsorig=NULL; //listener's position in 3D space
IDirectSoundCapture8 *dscobj=NULL; //default DS capture object

// DSoundEnv <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
class DSoundEnv
{
public:
 NAT nrO,nrI;
 int defO,defI;
 char**nameO,**nameI;
 GUID**guidO,**guidI;
 
 DSoundEnv();
 ~DSoundEnv()
  {
  int i;
  for(i=0;i<nrO;i++)
   free(nameO[i]);
  for(i=0;i<nrI;i++)
   free(nameI[i]);
  FREE(nameO);
  FREE(nameI);
  FREE(guidO);
  FREE(guidI);
  }
#ifdef V_UNIFACE
 void Select(int mod=0x3)
  {
  if(mod&0x1) defO=uidlg.PickS("DirectSound playback devices",nameO,nrO,1,defO);
  if(mod&0x2) defI=uidlg.PickS("DirectSound recording devices",nameI,nrI,1,defI);
  }
#endif
}dsenv; //default environament
//callback for DS device enum ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
BOOL CALLBACK DSEnumO(GUID*devguid,LPCSTR devname,LPCSTR drvname,void*penv)
{
DSoundEnv*lenv=(DSoundEnv*)penv;
int l;
lenv->nrO++;
lenv->nameO=(char**)realloc(lenv->nameO,sizeof(char*)*lenv->nrO);
lenv->guidO=(GUID**)realloc(lenv->guidO,sizeof(GUID*)*lenv->nrO);
l=sl((char*)devname)+sl((char*)drvname)+3;
lenv->nameO[lenv->nrO-1]=SALLOC(l);
lenv->guidO[lenv->nrO-1]=devguid;
sprintf(lenv->nameO[lenv->nrO-1],"%s <%s>",devname,drvname);
return TRUE;
}
//callback for DS device enum ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
BOOL CALLBACK DSEnumI(GUID*devguid,LPCSTR devname,LPCSTR drvname,void*penv)
{
DSoundEnv*lenv=(DSoundEnv*)penv;
int l;
lenv->nrI++;
lenv->nameI=(char**)realloc(lenv->nameI,sizeof(char*)*lenv->nrI);
lenv->guidI=(GUID**)realloc(lenv->guidI,sizeof(GUID*)*lenv->nrI);
l=sl((char*)devname)+sl((char*)drvname)+3;
lenv->nameI[lenv->nrI-1]=SALLOC(l);
lenv->guidI[lenv->nrI-1]=devguid;
sprintf(lenv->nameI[lenv->nrI-1],"%s <%s>",devname,drvname);
return TRUE;
}
//......................................................................................
DSoundEnv::DSoundEnv()
{
nrO=nrI=0;
nameO=nameI=NULL;
guidO=guidI=NULL;
DirectSoundEnumerate(DSEnumO,this);
DirectSoundCaptureEnumerate(DSEnumI,this);
}
// DSoundEnv >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//inits default DS environament ------------------------------------------------------------
int InitDS(DWORD sampform=0x1100ac44,int use3D=DSBCAPS_CTRL3D,int devid=0)
{
//CoInitialize(NULL);
erret=DirectSoundCreate8(dsenv.guidO[devid],&dsobj,NULL);
isDXOk("Object creation failed");
erret=dsobj->SetCooperativeLevel(hmwnd,DSSCL_PRIORITY);
isDXOk("Cooperative level refused");
DSBUFFERDESC dsbpd;
dsbpd.dwSize=sizeof(DSBUFFERDESC);
dsbpd.dwFlags=DSBCAPS_PRIMARYBUFFER|use3D;
dsbpd.dwBufferBytes=0;
dsbpd.dwReserved=0;
dsbpd.lpwfxFormat=NULL;
dsbpd.guid3DAlgorithm=DS3DALG_DEFAULT;
erret=dsobj->CreateSoundBuffer(&dsbpd,&dsbprim,NULL);
isDXOk("Primary buffer acquisition failed");
if(use3D&DSBCAPS_CTRL3D) //3d support
 {
 sampform=0x10000000|(sampform&0xfffffff); //3D sounds must be mono
 if(dsbprim->QueryInterface(IID_IDirectSound3DListener8,(void**)&dsorig)!=S_OK)
  error("Failed to create 3D listener");
 }
gsndform=SFtoWF(sampform);
dsbprim->SetFormat(&gsndform);
dsbprim->GetFormat(&gsndform,sizeof(WAVEFORMATEX),NULL);
return erret;
}

//inits the default DSCapture environament ----------------------------------------------
int InitDSC(DWORD sampform=0x1100ac44)
{
erret=DirectSoundCaptureCreate8(NULL,&dscobj,NULL);
isDXOk("Capture object creation failed");
return 0;
}

//releases both DS and DSC ----------------------------------------------------------------
void FreeDS()
{
RelCOM(dscobj,"DS capture object");
dscobj=NULL;
RelCOM(dsorig,"DS listener object");
dsorig=NULL;
RelCOM(dsbprim,"DS primary buffer");
dsbprim=NULL;
RelCOM(dsobj,"DS object");
dsobj=NULL;
//CoUninitialize();
}

//creates a ds secondary buffer --------------------------------------------------------
IDirectSoundBuffer8* MakeDSB8(WAVEFORMATEX dswf,DWORD flags=DSBCAPS_CTRL3D,NAT sizeb=0,float sizes=1)
{
IDirectSoundBuffer *dsbuffer=NULL;
IDirectSoundBuffer8 *dsbuffer8=NULL;
DSBUFFERDESC dsbd;
dsbd.dwSize=sizeof(DSBUFFERDESC);
dsbd.dwFlags=flags|DSBCAPS_STICKYFOCUS|DSBCAPS_GETCURRENTPOSITION2;
dsbd.dwBufferBytes=(sizeb>0?sizeb:sizes*dswf.nAvgBytesPerSec)+dswf.nBlockAlign+4;
dsbd.dwReserved=0;
dsbd.lpwfxFormat=&dswf;
dsbd.guid3DAlgorithm=DS3DALG_DEFAULT;
if((erret=dsobj->CreateSoundBuffer(&dsbd,&dsbuffer,NULL))!=DS_OK) 
 {
 //isDXOk("CreateSB");
 DXTrace(__FILE__,__LINE__,erret,"Io MakeDSB",1);
 return NULL;
 }
if(dsbuffer->QueryInterface(IID_IDirectSoundBuffer8,(void**)&dsbuffer8)!=S_OK)
 return NULL;
dsbuffer->Release();
return dsbuffer8;
}

//creates a ds capture buffer -----------------------------------------------------------
IDirectSoundCaptureBuffer8* MakeDSCB8(WAVEFORMATEX dswf,DWORD flags=DSCBCAPS_WAVEMAPPED,
                                      NAT sizeb=0,float sizes=2)
{
IDirectSoundCaptureBuffer *dscapture=NULL;
IDirectSoundCaptureBuffer8 *dscapture8=NULL;
DSCBUFFERDESC dscbd;
dscbd.dwSize=sizeof(DSCBUFFERDESC);
dscbd.dwFlags=flags|DSCBCAPS_WAVEMAPPED;
dscbd.dwBufferBytes=(sizeb>0?sizeb:sizes*dswf.nAvgBytesPerSec)+dswf.nBlockAlign+4;
dscbd.dwReserved=0;
dscbd.lpwfxFormat=&dswf;
dscbd.dwFXCount=0;
dscbd.lpDSCFXDesc=NULL;
erret=dscobj->CreateCaptureBuffer(&dscbd,&dscapture,NULL);
isDXOk("Create CapBuf");
erret=dscapture->QueryInterface(IID_IDirectSoundCaptureBuffer8,(void**)&dscapture8);
isDXOk("Query CapBuf8");
/*if(dscobj->CreateCaptureBuffer(&dscbd,&dscapture,NULL)!=DS_OK) 
 return NULL;
if(dscapture->QueryInterface(IID_IDirectSoundCaptureBuffer8,(void**)&dscapture8)!=S_OK)
 return NULL;*/
dscapture->Release();
return dscapture8;
}

#endif