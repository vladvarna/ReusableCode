//VERSION 1.0.a
#pragma once
//TODO: auto synchronization and auto negociation
//Win95: CancelIO doesn't exist, WriteFile() hangs computer (at least with ovelapped)
#include <mmed.cpp>

//Status
#define COM_IDLE		       0
#define COM_WAIT			 0x1
#define COM_SEND		     0x2
#define COM_FORWARD			 0x4
#define COM_BREAK			 0x8
#define COM_NO_READ_TO		0x10 //timeout reads
#define COM_CAPTURE_TERM   0x100 //capture ends at timeout
#define COM_CAPTURE_SEQ	   0x200 //capture ends at sequence
#define COM_CAPTURE_TO	   0x400 //capture ends at timeout
#define COM_CAPTURE		   0x800 //capture

#define COM_DTR		     0x10000 //Data Terminal Ready
#define COM_RTS		     0x20000 //Request To Send

//Notifications
#define COM_N_EVENT		 0x8000 //includes error, cts, dsr, received chars
#define COM_RCVD_TERM	 0x4000
//#define COM_RCVD_SEQ 	 0x2000
#define COM_RCVD_ANY 	 0x1000
#define COM_N_ERROR		 0x0800 //error
#define COM_N_CAPTURE    0x0400 //end capture
#define COM_N_CAPTURE_TO 0x0200 //capture timed out

int COMBufferTimeOut=500; //in ms

class IOCOM;

typedef int(*IOCOMNotify)(IOCOM*,DWORD);

class IOCOM
{
public:
 HANDLE hcom; //handler to COM port
 char comname[6]; //System name: COM1,COM2
 IOCOM*forward; //forward to this COM
 COMMCONFIG cfg; 
 COMMTIMEOUTS cto;
 COMMPROP prop;
 COMSTAT comstat;
 DWORD eventmask;
 DWORD event,comerr,cts_dsr; //also ring and rlsd
 DWORD stat; //flaguri (vezi mai sus)
 char*combuf,*capbuf; //bufere
 NAT combufsz,capbufsz; //dimensiunile buferelor
 char*terms,*escseq,capterm; //caractere speciale folosite ca separatori
 NAT nrterms,escseqnc;
 int repcapterm; //number of escape sequences to wait before ending capture
 int captmout; //time to wait before ending capture
 OVERLAPPED ovs;
 OVERLAPPED ovr;
 OVERLAPPED ovw;
 MSTimer lastrcvd,lastempty,capto; //last received char time, last empty buffer
 IOCOMNotify Notify;   //callback

 IOCOM() { ZeroMemory(this,sizeof(IOCOM)); }
 ~IOCOM() { Close(); Free(); EndCap(0); }
 BOOL Open(NAT,LPSTR,int);
 void Close();
 BOOL getDCB();
 BOOL setDCB(DWORD,BYTE,BYTE,BYTE);
 BOOL config(int);
 BOOL getTimeOuts();
 BOOL setTimeOuts(int,int,int,int,int);
 BOOL setBuffers(NAT,NAT);
 BOOL Break(int);
 BOOL DTR(int);
 BOOL RTS(int);
 BOOL Purge(DWORD);
 BOOL Transmit(char*,NAT,NAT);
 FAIL Send(void*,NAT,NAT);
 FAIL Monitor(char**,NAT*); //wait for event
 
 void Init(IOCOMNotify,IOCOM*);
 void Forward(int);
 void Termins(LPSTR,NAT);
 void Flush();
 BOOL Converse(char*,NAT,char*,NAT,NAT);
 void StartCap(DWORD,int,char,int,char*,NAT);
 void Capture();
 void EndCap(int);
 void Free(); //discard internal buffer
 FAIL Wait(); //wait for event
 
 //helper print functions; buffers should be static
 NAT Tconfig(char*);
 NAT Tevent(char*);
 NAT Terror(char*);
 NAT Tstatus(char*);
 void DBG();
 
};

#define COM_BUFFER_SZB 1024

// IOCOM <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

LPSTR COM_EVENTS[]={"Received char","Flag char","Tx buffer empty","CTS","DSR","RLSD","BREAK","Line status error",
                    "Ring signal","PERR","RX80FULL","Ev1","Ev2"}; //helper to print events
//#define EV_RXCHAR           0x0001  // Any Character received
//#define EV_RXFLAG           0x0002  // Received certain character
//#define EV_TXEMPTY          0x0004  // Transmitt Queue Empty
//#define EV_CTS              0x0008  // CTS changed state
//#define EV_DSR              0x0010  // DSR changed state
//#define EV_RLSD             0x0020  // RLSD changed state
//#define EV_BREAK            0x0040  // BREAK received
//#define EV_ERR              0x0080  // Line status error occurred
//#define EV_RING             0x0100  // Ring signal detected
//#define EV_PERR             0x0200  // Printer error occured
//#define EV_RX80FULL         0x0400  // Receive buffer is 80 percent full
//#define EV_EVENT1           0x0800  // Provider specific event 1
//#define EV_EVENT2           0x1000  // Provider specific event 2
LPSTR COM_ERRORS[]={"Queue overflow","Overrun","Parity errror","Framing error","BREAK","","","",
                    "TX Queue is full","LPTx Timeout","LPTx I/O Error","LPTx Device not selected",
                    "LPTx Out-Of-Paper","","","Requested mode unsupported"}; //helper to print errors
//#define CE_RXOVER           0x0001  // Receive Queue overflow
//#define CE_OVERRUN          0x0002  // Receive Overrun Error
//#define CE_RXPARITY         0x0004  // Receive Parity Error
//#define CE_FRAME            0x0008  // Receive Framing error
//#define CE_BREAK            0x0010  // Break Detected
//#define CE_TXFULL           0x0100  // TX Queue is full
//#define CE_PTO              0x0200  // LPTx Timeout
//#define CE_IOE              0x0400  // LPTx I/O Error
//#define CE_DNS              0x0800  // LPTx Device not selected
//#define CE_OOP              0x1000  // LPTx Out-Of-Paper
//#define CE_MODE             0x8000  // Requested mode unsupported
LPSTR PARITY_MODE[5]={"no","odd","even","mark","space"}; //helper to print parity
float STOP_BITS[3]={1.,1.5,2.}; //helper to print stop bits

//deschide portul.........................................................................
BOOL IOCOM::Open(NAT lcomnr=0,LPSTR lcomname=NULL,int usesettings=0)
{
char lsbuf[16];
if(!lcomname)
 {
 lcomname=REGs;
 sprintf(lcomname,"COM%u",lcomnr);
 }
hcom=CreateFile(lcomname,GENERIC_READ|GENERIC_WRITE,
                0,    // must be opened with exclusive-access
                NULL, // no security attributes
                OPEN_EXISTING, // must use OPEN_EXISTING
                FILE_FLAG_OVERLAPPED,    // overlapped I/O
                NULL);  // hTemplate must be NULL for comm devices
if(hcom==INVALID_HANDLE_VALUE)
 { 
 hcom=NULL;
 //isERROR(lcomname);
 return 0;
 }
sc(comname,lcomname);
ovs.hEvent=CreateEvent(NULL,TRUE,FALSE,comname);
isNULL(ovs.hEvent,"CreateEvent() failed sync");

smerge(lsbuf,2,comname,"rcvd");
ovr.hEvent=CreateEvent(NULL,TRUE,FALSE,lsbuf);
isNULL(ovr.hEvent,"CreateEvent() failed read");

smerge(lsbuf,2,comname,"send");
ovw.hEvent=CreateEvent(NULL,TRUE,FALSE,lsbuf);
isNULL(ovw.hEvent,"CreateEvent() failed write");

if(usesettings)	//internal DCB is valid
 {
 SetCommConfig(hcom,&cfg,sizeof(cfg));
 }
else //internal DCB is initialized with port current settings
 {
 cfg.dwSize=REGdump=sizeof(cfg);
 GetCommConfig(hcom,&cfg,&REGdump);
 }
GetCommTimeouts(hcom,&cto);
GetCommProperties(hcom,&prop);
SetCommMask(hcom,eventmask=EV_BREAK|EV_ERR|EV_RXCHAR|EV_TXEMPTY|EV_CTS|EV_DSR|EV_RING|EV_RLSD|EV_RXFLAG);
lastrcvd.Abs();
stat=COM_IDLE;
DTR(stat&COM_DTR);
RTS(stat&COM_RTS);
return 1;
}

//inchide portul.........................................................................
void IOCOM::Close()
{
if(!hcom) return; //not opened
PurgeComm(hcom,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
#ifndef V_SUPPORT_WIN95
CancelIo(hcom);
#endif
stat&=~(COM_WAIT|COM_SEND|COM_BREAK);
Free();
CloseHandle(ovs.hEvent);
CloseHandle(ovr.hEvent);
CloseHandle(ovw.hEvent);
CloseHandle(hcom);
hcom=NULL;
}

//.........................................................................
inline BOOL IOCOM::getDCB()
{
if(!hcom) return 0; //not opened
return GetCommState(hcom,&cfg.dcb);
}

//.........................................................................
inline BOOL IOCOM::setDCB(DWORD baud=CBR_9600,BYTE datab=8,BYTE parity=NOPARITY,BYTE stopb=ONESTOPBIT)
{
cfg.dcb.DCBlength=sizeof(DCB);
cfg.dcb.BaudRate=baud;
cfg.dcb.ByteSize=datab;
cfg.dcb.Parity=parity;
cfg.dcb.fParity=(parity!=NOPARITY);
cfg.dcb.StopBits=stopb;
if(!hcom) return 0; //not opened
return SetCommState(hcom,&cfg.dcb);
}

//.........................................................................
inline BOOL IOCOM::config(int mod=0)
{
if(!mod) //display dialog
 {
 if(hcom)
  GetCommConfig(hcom,&cfg,&REGdump);
 else
  GetDefaultCommConfig(comname,&cfg,&REGdump);
 CommConfigDialog(comname,hmwnd,&cfg);
 }
if(hcom) SetCommConfig(hcom,&cfg,sizeof(cfg));
SetDefaultCommConfig(comname,&cfg,sizeof(cfg));
return erret;
}

//.........................................................................
inline BOOL IOCOM::getTimeOuts()
{
if(!hcom) return 0; //not opened
return GetCommTimeouts(hcom,&cto);
}

//.........................................................................
inline BOOL IOCOM::setTimeOuts(int Rinterval=MAXDWORD,int RtotalM=0,int RtotalC=0,int WtotalM=0,int WtotalC=0)
{
if(!hcom) return 0; //not opened
cto.ReadIntervalTimeout=Rinterval;
cto.ReadTotalTimeoutMultiplier=RtotalM;
cto.ReadTotalTimeoutConstant=RtotalC;
cto.WriteTotalTimeoutMultiplier=WtotalM;
cto.WriteTotalTimeoutConstant=WtotalC;
return SetCommTimeouts(hcom,&cto);
}

//.........................................................................
inline BOOL IOCOM::setBuffers(NAT inszB=1024,NAT outszB=1024)
{
return SetupComm(hcom,inszB,outszB); //mostly useless; Windows overrides settings
}

//.........................................................................
inline BOOL IOCOM::Purge(DWORD flags=PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR)
{
return PurgeComm(hcom,flags);
}

//set DTR .........................................................................
inline BOOL IOCOM::DTR(int dtr=-1)
{
if(dtr==-1) //toggle
 stat^=COM_DTR;
else
 stat=dtr?stat|COM_DTR:stat&~COM_DTR;
if(stat&COM_DTR) 
 return EscapeCommFunction(hcom,SETDTR);
else 
 return EscapeCommFunction(hcom,CLRDTR);
}

//set RTS .........................................................................
inline BOOL IOCOM::RTS(int rts=-1)
{
if(rts==-1) //toggle
 stat^=COM_RTS;
else
 stat=rts?stat|COM_RTS:stat&~COM_RTS;
if(stat&COM_RTS) 
 return EscapeCommFunction(hcom,SETRTS);
else 
 return EscapeCommFunction(hcom,CLRRTS);
}

//(seems to hang) .........................................................................
inline BOOL IOCOM::Break(int brk=-1)
{
if(brk==-1) //toggle
 stat^=COM_BREAK;
else
 stat=brk?stat|COM_BREAK:stat&~COM_BREAK;
if(stat&COM_BREAK) 
 return SetCommBreak(hcom);
else 
 return ClearCommBreak(hcom);
}

//transmit a string with high priority.........................................................................
FAIL IOCOM::Transmit(char*buffer,NAT slen=1,NAT delay=0)
{
SetLastError(0);
while(slen)
 {
 if(TransmitCommChar(hcom,*buffer))
  {
  buffer++;
  slen--;
  }
 else  //wait 
  Sleep(delay);
 }
return slen;
}

//write a string to the port.........................................................................
FAIL IOCOM::Send(void*buffer,NAT buffersz,NAT timeout=INFINITE)
{
DWORD B_written=0;
if(!hcom) return 0;
//ResetEvent(ovw.hEvent);
#ifndef V_SUPPORT_WIN95
stat|=COM_SEND;
if(!WriteFile(hcom,buffer,buffersz,&B_written,&ovw))
 {
 if(GetLastError()!=ERROR_IO_PENDING)
  isERROR("WriteFile() in IOCOM::Send()");
 }
erret=WaitForSingleObject(ovw.hEvent,timeout);
if(erret!=WAIT_OBJECT_0&&erret!=WAIT_TIMEOUT)
 isERROR("WaitForSingleObject() in IOCOM::Send()");
#else
 B_written=Transmit((char*)buffer,buffersz,0);
#endif
return B_written;
}

//start or continue monitoring the port .........................................
FAIL IOCOM::Monitor(char**pbuffer,NAT*buffersz)
{
char locbuf[COM_BUFFER_SZB];
DWORD B_read;
*buffersz=0;
if(!hcom) return 0; //not opened
if(!(stat&COM_WAIT)) //start monitoring
 {
 if(!WaitCommEvent(hcom,&event,&ovs))
  {
  if(GetLastError()==ERROR_IO_PENDING)
   stat|=COM_WAIT;
  else
   {
   isERROR("WaitCommEvent(ovs) in IOCOM::Monitor");
   return -1; //can't start monitoring
   }
  }
 else
  SetEvent(ovs.hEvent); //manual signal
 }
erret=WaitForSingleObject(ovs.hEvent,1);
if(erret==WAIT_TIMEOUT)
 return 0; //OK but nothing to process
else if(erret==WAIT_OBJECT_0)
 {
 ResetEvent(ovs.hEvent);
 stat&=~COM_WAIT;
 if(event&(EV_CTS|EV_DSR|EV_RING|EV_RLSD)) //cts, dsr, ring, rlsd
  {
  GetCommModemStatus(hcom,&cts_dsr);
  }
 if(event&EV_RXCHAR) //read
  {
  int to=0;	//manual timeout
  do
   {
   B_read=0;
   ResetEvent(ovr.hEvent);
   if(!ReadFile(hcom,locbuf,sizeof(locbuf),&B_read,&ovr))
    {
    if(GetLastError()==ERROR_IO_PENDING)
     {
     erret=WaitForSingleObject(ovr.hEvent,30000); //wait 30"
   	 if(erret!=WAIT_OBJECT_0&&erret!=WAIT_TIMEOUT)
      isERROR("WaitForSingleObject(ovr) in IOCOM::Send()");
     } 
    else
     isERROR("ReadFile() in IOCOM::Monitor");
    }
   if(B_read)
    {
    *pbuffer=(char*)REALLOC(*pbuffer,*buffersz+B_read);
    CopyMemory(*pbuffer+*buffersz,locbuf,B_read);
    *buffersz+=B_read;
    }
   else
    break; 
   to++;
   }while(to<10);
  return 2; //chars read
  }
 if(event&EV_ERR) //error
  {
  if(!ClearCommError(hcom,&comerr,&comstat))
   isERROR("ClearCommError() in IOCOM::Monitor()");
  return -3;
  }
 else
  {
  return 3; //other event
  }
 }
else
 {
 isERROR("WaitForSingleObject(ovs) failed");
 return -2; //error
 }
}

//setup the callback function and foreward port..................................................................................
inline void IOCOM::Init(IOCOMNotify lNotify=NULL,IOCOM*lforward=NULL)
{
Notify=lNotify;
forward=lforward;
}

//enable/disable forwarding..................................................................................
inline void IOCOM::Forward(int enable=0)
{
if(enable)
 stat|=COM_FORWARD;
else
 stat&=~COM_FORWARD;
}

//set the terminators..................................................................................................
inline void IOCOM::Termins(LPSTR lterms=NULL,NAT lnrterms=0)
{
terms=lterms;
nrterms=lnrterms?lnrterms:sl(terms);
}

//free resources.................................................................................................
inline void IOCOM::Free()
{
FREE(combuf);
combufsz=0;
}

//free resources and flush buffers..........................................................................................................
inline void IOCOM::Flush()
{
if(!combufsz) return;
if(terms)
 {
 if(Notify) Notify(this,COM_RCVD_TERM);
 }
else
 {
 if(Notify) Notify(this,COM_RCVD_ANY);
 }
FREE(combuf); //auto Flush()
combufsz=0;
}

//send message and wait ACK ........................................................................
BOOL IOCOM::Converse(char*sendbuf,NAT sendszB,char*rcvdbuf,NAT rcvdszB,NAT to=0)
{
char locbuf[COM_BUFFER_SZB];
DWORD B_read;
int retv=0;
if(!hcom) return 0;
Purge();
Send(sendbuf,sendszB);
//#ifdef V_SUPPORT_WIN95
//return 1;
//#endif
B_read=0;
ResetEvent(ovr.hEvent);
Sleep(to&0xffff); //max wait ~ 1 min
if(!(erret=ReadFile(hcom,locbuf,rcvdszB,&B_read,&ovr)))
 {
 if(GetLastError()==ERROR_IO_PENDING)
   {
   erret=WaitForSingleObject(ovr.hEvent,5000);   //5s
   if(erret!=WAIT_OBJECT_0&&erret!=WAIT_TIMEOUT)
    isERROR("WaitForSingleObject(ovr) in IOCOM::Send()");
   } 
  else
   isERROR("ReadFile() in IOCOM::Converse()");
  }
if(B_read)
 {
 lastrcvd.Abs();
 //if(forward&&(stat&COM_FORWARD))
 // forward->Send(locbuf,B_read);
 }
if(B_read>=rcvdszB)
 {
 retv=CmpMem(rcvdbuf,locbuf,rcvdszB);
 if(B_read>rcvdszB)
  {
  combuf=(char*)REALLOC(combuf,combufsz+B_read-rcvdszB);
  CopyMemory(combuf+combufsz,locbuf+rcvdszB,B_read-rcvdszB);
  combufsz+=B_read-rcvdszB;
  if(Notify) Notify(this,COM_RCVD_ANY);
  }
 }
else
 {
 retv=0;
 if(B_read)
  {
  combuf=(char*)REALLOC(combuf,combufsz+B_read);
  CopyMemory(combuf+combufsz,locbuf,B_read);
  combufsz+=B_read;
  if(Notify) Notify(this,COM_RCVD_ANY);
  }
 }
return retv;
}

//set up port capture to the internal buffer..............................................................................................................
void IOCOM::StartCap(DWORD captype=COM_CAPTURE_TO,int lcaptmout=0,char lcapterm=0,int lrepcapterm=1,char*lescseq=NULL,NAT lescseqnc=0)
{
Flush();
stat|=COM_CAPTURE|captype;

captmout=lcaptmout;
capterm=lcapterm;
repcapterm=lrepcapterm;

escseq=lescseq;
escseqnc=lescseqnc?lescseqnc:sl(escseq);

FREE(capbuf);
capbufsz=0;
capto.Abs();
}

//begin capture..............................................................................................................
void IOCOM::Capture()
{
NAT B_read;
NAT t;
char*tmpbuf;
if(!(stat&COM_CAPTURE)) return;
capbuf=(char*)REALLOC(capbuf,capbufsz+combufsz);
B_read=combufsz;
tmpbuf=combuf;
if(stat&COM_CAPTURE_TERM) //capture ends after receiving repcapterm of capterm
 {
 while(B_read)
  {
  t=s_anych(tmpbuf,&capterm,1,B_read);
  if(t!=R_NULL)	//found term
   {
   t++;
   CopyMemory(capbuf+capbufsz,tmpbuf,t);
   capbufsz+=t;
   tmpbuf+=t;
   B_read-=t;
   repcapterm--;
   if(repcapterm<=0)
    {
    if(Notify) Notify(this,COM_N_CAPTURE);
    return;
    }
   } 
  else
   {
   CopyMemory(capbuf+capbufsz,tmpbuf,B_read);
   capbufsz+=B_read;
   break;  //loop while found terms
   }
  }
 } 
else //timeout or sequence
 {
 CopyMemory(capbuf+capbufsz,tmpbuf,B_read);
 capbufsz+=B_read;
 if(stat&COM_CAPTURE_SEQ)  //capture ends after receiving 1 of escseq
  {
  t=s_seq(capbuf,escseq,escseqnc,capbufsz);
  if(t!=R_NULL)	//found term
   {
   capbufsz=t;
   if(Notify) Notify(this,COM_N_CAPTURE);
   return;
   } 
  }
 }
}

//stop capturing..............................................................................................................
void IOCOM::EndCap(int exitcode=0)
{
FREE(capbuf);
capbufsz=0;
stat&=~(COM_CAPTURE|COM_CAPTURE_SEQ|COM_CAPTURE_TERM|COM_CAPTURE_TO);
}

//start or continue monitoring port for events........................................................................
FAIL IOCOM::Wait()
{
char locbuf[COM_BUFFER_SZB],*tmpbuf;
DWORD B_read;
NAT t,to;
if(!hcom) return 0; //not opened
if((lastrcvd.Rel()>COMBufferTimeOut)&&!(stat&COM_NO_READ_TO))
 Flush();
if(stat&COM_CAPTURE_TO)
 {
 if(capto.Rel()>captmout)
  {
  if(Notify) Notify(this,COM_N_CAPTURE_TO);
  }
 }
if(!(stat&COM_WAIT)) //start monitoring
 {
 event=0; //clear prev events
 if(!WaitCommEvent(hcom,&event,&ovs))
  {
  if(GetLastError()==ERROR_IO_PENDING)
   stat|=COM_WAIT;
  else
   {
   isERROR("WaitCommEvent(ovs) in IOCOM::Monitor");
   return -1; //can't start monitoring
   }
  }
 else
  SetEvent(ovs.hEvent); //manual signal
 }
erret=WaitForSingleObject(ovs.hEvent,1);
if(erret==WAIT_TIMEOUT)
 return 0; //OK no event to process
else if(erret==WAIT_OBJECT_0)
 {
 ResetEvent(ovs.hEvent);
 stat&=~COM_WAIT;
 if(event&EV_ERR) //error
  {
  if(!ClearCommError(hcom,&comerr,&comstat))
   isERROR("ClearCommError() in IOCOM::Monitor()");
  if(comerr&CE_BREAK)
   stat|=COM_BREAK;
  if(Notify) Notify(this,COM_N_ERROR);
  }
 if(event&(EV_CTS|EV_DSR|EV_RING|EV_RLSD)) //cts, dsr, ring, rlsd
  {
  GetCommModemStatus(hcom,&cts_dsr);
  }
 if(event&EV_TXEMPTY) //finished send
  {
  lastempty.Abs();
  stat&=~COM_SEND;
  }
 if(event&EV_RXCHAR) //read
  {
  to=0;	//manual timeout
  do
   {
   B_read=0;
   ResetEvent(ovr.hEvent);
   if(!ReadFile(hcom,locbuf,sizeof(locbuf),&B_read,&ovr))
    {
    if(GetLastError()==ERROR_IO_PENDING)
     {
     erret=WaitForSingleObject(ovr.hEvent,30000); //wait 30"
   	 if(erret!=WAIT_OBJECT_0&&erret!=WAIT_TIMEOUT)
      isERROR("WaitForSingleObject(ovr) in IOCOM::Send()");
     } 
    else
     isERROR("ReadFile() in IOCOM::Monitor()");
    }
   if(B_read)
    {
    lastrcvd.Abs();
    capto.Abs();
    if(forward&&(stat&COM_FORWARD))
     forward->Send(locbuf,B_read);
    combuf=(char*)REALLOC(combuf,combufsz+B_read);
    tmpbuf=locbuf;
    if(terms)   //search escape sequence
     {
     while(B_read)
      {
      t=s_anych(tmpbuf,terms,nrterms,B_read);
      if(t!=R_NULL)	//found term
       {
       t++;
       CopyMemory(combuf+combufsz,tmpbuf,t);
       combufsz+=t;
       tmpbuf+=t;
       if(stat&COM_CAPTURE) Capture();
       if(Notify) Notify(this,COM_RCVD_TERM);
       B_read-=t;
       if((!combuf)&&B_read) combuf=(char*)ALLOC(B_read);  //flushed by user
       } 
	  else
       break;  //loop while found terms
      }
     }
    if(B_read)
     {
     CopyMemory(combuf+combufsz,tmpbuf,B_read);
     combufsz+=B_read;
     if(Notify) Notify(this,COM_RCVD_ANY);
     }
    }
   else
    break; 
   to++;
   }while(to<10);
  }
 if(Notify) Notify(this,COM_N_EVENT);
 return 2; //event
 }
else
 {
 isERROR("WaitForSingleObject(ovs) failed");
 return -2; //error
 }
}

//get a string description for last event.........................................................................
NAT IOCOM::Tevent(LPSTR evbuf)
{
NAT l;
if(!event) return 0;
l=sprintf(evbuf,"%s:> Event %X=",comname,event);
for(int i=0;i<13;i++)
 {
 if((event>>i)&1)
  l+=sprintf(evbuf+l,"%s|",COM_EVENTS[i]);
 }
evbuf[l--]='\0'; //delete last ','
return l;
}

//get a string description for last error.........................................................................
NAT IOCOM::Terror(LPSTR erbuf)
{
NAT l;
if(!comerr) return 0;
l=sprintf(erbuf,"%s:> ERROR %X=",comname,comerr);
for(int i=0;i<16;i++)
 {
 if((comerr>>i)&1)
  l+=sprintf(erbuf+l,"%s|",COM_ERRORS[i]);
 }
erbuf[l--]='\0'; //delete last ','
event&=~EV_ERR;
return l;
}

//get a string description for last status change.........................................................................
NAT IOCOM::Tstatus(LPSTR stbuf)
{
NAT l=0;
if(event&EV_CTS)
 {
 l=sprintf(stbuf,"%s:> ",comname);
 l+=sprintf(stbuf+l,"CTS %s ",cts_dsr&MS_CTS_ON?"on":"off");
 event&=~EV_CTS;
 }
if(event&EV_DSR)
 {
 if(!l) l=sprintf(stbuf,"%s:> ",comname);
 l+=sprintf(stbuf+l,"DSR %s",cts_dsr&MS_DSR_ON?"on":"off");
 event&=~EV_DSR;
 }
return l;
}

//get a string with port config details.........................................................................
NAT IOCOM::Tconfig(LPSTR stbuf)
{
NAT l=0;
l=sprintf(stbuf,"%s (Baud=%u; b/B=%u+%1.1f; %s parity)",comname,cfg.dcb.BaudRate,cfg.dcb.ByteSize,STOP_BITS[cfg.dcb.StopBits],PARITY_MODE[cfg.dcb.Parity]);
return l;
}

//debug function.........................................................................
//void IOCOM::DBG()
//{
//getDCB();
//dump("#'__________________'");
////dump("#"DCB_MAP,&cfg.dcb);
//dump("#"COMMCONFIG_MAP,&cfg);
//dump("#'__TIMEOUTS__',RI:u,Rm:u,Rc:u,Wm:u,Wc:u",&cto);
////llog("TmOuts: RI=%u Rm=%u Rc=%u Wm=%u Wc=%u",cto.ReadIntervalTimeout,cto.ReadTotalTimeoutMultiplier,cto.ReadTotalTimeoutConstant,cto.WriteTotalTimeoutMultiplier,cto.WriteTotalTimeoutConstant);
//dump("#"COMMPROP_MAP,&prop);
//}
//
// IOCOM >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
