#pragma once
#define V_MULTIMEDIA

#include <mmreg.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

WAVEFORMATEX gsndform; //global sound format

#define FOURCC_DW(c1,c2,c3,c4) (DWORD)((c4<<24)|(c3<<16)|(c2<<8)|c1) 
#define SAMPFORM(nrc,bps,fps) (DWORD)((nrc<<28)|(bps<<20)|(fps&0xfffff))
#define SF_DEF(hz)  (DWORD)(0x11000000|(hz&0xfffff)) //default to: mono, 16bps
#define SF_KHZ(khz) (DWORD)(0x11000000|(((DWORD)(khz*1000))&0xfffff)) //mono, 16bps, sampling frequncy in KHz
#define SF_nrC(sf) (sf>>28)
#define SF_Fps(sf) (sf&0xfffff)
#define SF_BpS(sf) ((sf>>23)&0x1f)
#define SF_BpF(sf) (((sf>>23)&0x1f)*(sf>>28)) //BlockAlign=BytesPerFrame
#define SF_Bps(sf) (((sf>>23)&0x1f)*(sf>>28)*(sf&0xfffff)) //AvgBytesPerSec=BytesPerSecond

/*
0x01F40 =      8 KHz
0x02B11 =  11025 Hz
0x03E80 =     16 KHz
0x05622 =  22050 Hz
0x0AC44 =  44100 Hz
0x0BB80 =     48 KHz
0x15888 =  88200 Hz
0x17700 =     96 KHz
0x186A0 =    100 KHz
0x2EE00 =    192 KHz
*/

//WAVEFORMATEX to SF -------------------------------------------------------------------------
#define WFtoSF(swf) (DWORD)((swf.nChannels&0xf)<<28)|((swf.wBitsPerSample&0xff)<<20)|(swf.nSamplesPerSec&0xfffff)

//initializes a WAVEFORMATEX structure ------------------------------------------------------
inline WAVEFORMATEX SFtoWF(DWORD sampform=0)
{
WAVEFORMATEX swf;
swf.wFormatTag=WAVE_FORMAT_PCM;
swf.nChannels=sampform>>28;
swf.nSamplesPerSec=sampform&0xfffff;
swf.wBitsPerSample=(sampform&0xff00000)>>20;
swf.nBlockAlign=swf.nChannels*(swf.wBitsPerSample>>3);
swf.nAvgBytesPerSec=swf.nSamplesPerSec*swf.nBlockAlign;
swf.cbSize=0;
return swf;
}

//calculates maximum dBFS for a number of bits/sample ------------------------------------------------------
inline double bps_dBFS(double bps)
{
return 20.*log10(pow(2.,-bps)); //=20*lg(2^(-bps))
}


//multimedia timer in miliseconds (needs: winmm.lib) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct MSTimer
{
NAT mark; //in msec
MSTimer()
 {
 mark=timeGetTime();
 }
NAT Abs()
 {
 return mark=timeGetTime();
 }
NAT Rel()
 {
 return timeGetTime()-mark;
 }
};

//frames per 1000 msecs counter (needs: winmm.lib) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct FPS
{
unsigned fps,mark,ms,fs;
int Count()
 {
 ms=timeGetTime()-mark;
 if(ms>1000) //1s elapsed
  {
  mark+=ms;
  fps=fs;
  fs=0;
  return 1;
  }
 fs++;
 return 0;
 }
};

//returns the offset and size of a chunk in a RIFF ----------------------------------------------
int FindRIFFChunk(DWORD &chkoff,DWORD &chksz,FILE *riff,DWORD chkid=0,NAT chkparent=0)
{
DWORD chk4cc;
fseek(riff,chkparent,0);
fread(&chk4cc,4,1,riff); //parent chunk id
fread(&chksz,4,1,riff); //parent chunk sz
chkoff=chkparent+8;
if(chk4cc==FOURCC_DW('R','I','F','F')||chk4cc==FOURCC_DW('L','I','S','T'))
 chkoff+=4; //this contain an aditional field
chkparent+=chksz+8;
while(chkoff<chkparent)
 {
 fseek(riff,chkoff,0);
 fread(&chk4cc,4,1,riff); //chunk id
 fread(&chksz,4,1,riff); //chunk sz
 if(chk4cc==chkid)
  return 1; //found
 chkoff+=ALIGN(chksz,1)+8;
 }
chkoff=chksz=0;
return 0; //not found
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UFILEAudio
{
public:
 NAT NrF; //total number of frames
 NAT FpB; //
 NAT cursor; //current frame file pointer
 DWORD sf; //internal sound format
 BOOL eof; //end of file reached
  
 virtual void Set(DWORD,NAT)=0; //inits the object
 virtual FAIL Open(LPSTR,WAVEFORMATEX*)=0; //open
 virtual FAIL Save(LPSTR,WAVEFORMATEX*)=0; //create/overwrite
 virtual void Free()=0;
   
 virtual NAT Seek(NAT)=0; //seeks a frame
 virtual NAT BlocksLeft()=0; //estimated nr. of blocks left in file
 virtual NAT GetF(void*,NAT)=0; //reads frame
 virtual NAT PutF(void*,NAT)=0; //
};