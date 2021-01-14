//VERSION 1.0.a
#pragma once

#ifdef WIN32_LEAN_AND_MEAN
 #include <winsock2.h>
#endif
#include <mat.cpp>
#include <str.cpp>
#include <reg.cpp>
#include <rnd.cpp>
#include <winter.cpp>

#define IPSIZE 32 //255.255.255.255.255.255
#define NETPATHSZ 260 //

//even if the name says ODBC this are universal
#define ODBC_LOGON_DRIVER		0x40000000
#define ODBC_LOGON_DSN   		0x20000000
#define ODBC_LOGON_FILEDSN		0x10000000
#define ODBC_LOGON_HOST  		0x08000000
#define ODBC_LOGON_PATH  		0x04000000
#define ODBC_LOGON_ADDRESS		0x02000000
#define ODBC_LOGON_NET   		0x01000000
#define ODBC_LOGON_DRIVERID		0x00800000
#define ODBC_LOGON_PORT  		0x00400000
#define ODBC_LOGON_UID   		0x00200000
#define ODBC_LOGON_PWD   		0x00100000
#define ODBC_LOGON_DIRECTORY    0x00080000
#define ODBC_LOGON_DATABASE		0x00040000  

#define ODBC_LOGON_MYSQL (ODBC_LOGON_DRIVER|ODBC_LOGON_UID|ODBC_LOGON_PWD|ODBC_LOGON_HOST|ODBC_LOGON_PORT)
#define ODBC_LOGON_MSSQL (ODBC_LOGON_DRIVER|ODBC_LOGON_HOST|ODBC_LOGON_NET)	//must use "Trusted_Connection=Yes"
#define ODBC_LOGON_EXCEL (ODBC_LOGON_DRIVER|ODBC_LOGON_PATH|ODBC_LOGON_DRIVERID)
#define MYSQL_LOGON      (ODBC_LOGON_UID|ODBC_LOGON_PWD|ODBC_LOGON_HOST|ODBC_LOGON_PORT|ODBC_LOGON_DATABASE)
#define FTP_LOGON        (ODBC_LOGON_UID|ODBC_LOGON_PWD|ODBC_LOGON_HOST|ODBC_LOGON_PORT|ODBC_LOGON_DIRECTORY)

struct LOGON_INFO
{
 char username[32];
 char password[32];
 NAT port;   //also driver ID
 union {
  char host[NETPATHSZ];	//SERVER URL
  char path[NETPATHSZ]; //local file
  };
 union {
  char DSN[64]; //DSN or FILEDSN
  char DRIVER[64]; //DRIVER
  char database[64]; //def db
  char directory[64]; //def sub directory
  };
 DWORD flags;
 char title[16]; //unique id (MUST be at end for proper saveing)
 
 LOGON_INFO() { ZeroMemory(this,sizeof(LOGON_INFO)); }
 ~LOGON_INFO() { if(*title) Save(); }
 //.............................................................................................
 void Set(char*ltitle=NULL,LPSTR lusername=NULL,LPSTR lpassword=NULL,LPSTR lhost=NULL,NAT lport=0,LPSTR ldsn=NULL,DWORD lflags=0)
 {
 if(ltitle)
  sc(title,ltitle,sizeof(title)-1);
 else
  *title=0; //="" 
 ifn(Load())
  {
  flags=lflags;
  if(lusername) sc(username,lusername,sizeof(username)-1);
  if(lpassword) sc(password,lpassword,sizeof(password)-1);
  if(lhost) sc(host,lhost,sizeof(host)-1);
  port=lport;
  if(ldsn) sc(DSN,ldsn,sizeof(DSN)-1);	//or default database
  }
 }
 //.............................................................................................
 BOOL Load()	//from registry
 {
 REGISTRY rgy;
 //rgy.Key("Software\\"V_COMPANY"\\LOGON_INFO");
 rgy.AppKey(KEY_READ,"LOGON_INFO");
 erret=rgy.GetVal(title,this,sizeof(LOGON_INFO)-sizeof(title));
 if(erret) ScramblePassword(password,sizeof(password),*title); //unscramble
 return erret;
 }
 //.............................................................................................
 BOOL Save()	//to registry
 {
 REGISTRY rgy;
 //rgy.Key("Software\\"V_COMPANY"\\LOGON_INFO");
 rgy.AppKey(KEY_WRITE,"LOGON_INFO");
 ScramblePassword(password,sizeof(password),*title);
 erret=rgy.SetVal(title,this,sizeof(LOGON_INFO)-sizeof(title));
 ScramblePassword(password,sizeof(password),*title); //unscramble
 return erret;
 }
 //.............................................................................................
 BOOL UI()
 {
 int retv;
 char lport[8];
 itoa(port,lport,10);
 if(flags&ODBC_LOGON_DRIVER)
  retv=InputStrings(5,hmwnd,title,"HOST",host,sizeof(host)-1,"PORT/Driver ID",lport,sizeof(lport)-1,
                   "USERNAME",username,sizeof(username)-1,"PASSWORD",password,sizeof(password)-1,
                   "DRIVER",DRIVER,sizeof(DRIVER)-1);
 else if(flags&(ODBC_LOGON_DSN|ODBC_LOGON_FILEDSN))
  retv=InputStrings(5,hmwnd,title,"DSN",DSN,sizeof(DSN)-1,"HOST",host,sizeof(host)-1,"Driver ID",lport,sizeof(lport)-1,
                   "USERNAME",username,sizeof(username)-1,"PASSWORD",password,sizeof(password)-1);
 else if(flags&ODBC_LOGON_DATABASE)
  retv=InputStrings(5,hmwnd,title,"HOST",host,sizeof(host)-1,"PORT",lport,sizeof(lport)-1,
                   "USERNAME",username,sizeof(username)-1,"PASSWORD",password,sizeof(password)-1,
                   "DataBase",DSN,sizeof(DSN)-1);
 else if(flags&ODBC_LOGON_DIRECTORY)
  retv=InputStrings(5,hmwnd,title,"HOST",host,sizeof(host)-1,"PORT",lport,sizeof(lport)-1,
                   "USERNAME",username,sizeof(username)-1,"PASSWORD",password,sizeof(password)-1,
                   "Directory",directory,sizeof(directory)-1);
 else
  retv=InputStrings(4,hmwnd,title,"HOST",host,sizeof(host)-1,"PORT",lport,sizeof(lport)-1,
                   "USERNAME",username,sizeof(username)-1,"PASSWORD",password,sizeof(password)-1);
 port=atoi(lport);
 return retv;
 }
};

//convert speed in B/s to a printable string ----------------------------------------------------
NAT NetSpeedToStr(char*buffer,double speed)
{
int m=0;
while(speed>1024.)
 {
 speed/=1024.;
 m++;
 }
return sprintf(buffer,"%.1f%cB/s",speed,KMGT[m]);
}

//dl/ul speeds ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class TraficMeter
{
public:
 double res;
 QWORD last;
 QWORD bytes;
 double seconds;
 double speed; //last speed (for one period) in B/s

 TraficMeter() { Reset(); }
//......................................................................
 void Reset() //reset
  {
  QWORD freq;
  ifn(QueryPerformanceFrequency((LARGE_INTEGER*)&freq))
   res=0.;
  else
   res=(double)1./freq; 
  speed=seconds=0.;
  bytes=0;
  QueryPerformanceCounter((LARGE_INTEGER*)&last);
  }
//......................................................................
 void Begin()
  {
  QueryPerformanceCounter((LARGE_INTEGER*)&last);
  }
//......................................................................
 void Finish(QWORD lbytes=0)
  {
  QWORD lnow;
  double lsec;
  QueryPerformanceCounter((LARGE_INTEGER*)&lnow);
  lsec=(double)(lnow-last)*res;
  speed=lsec?lbytes/lsec:-1.;
  seconds+=lsec;
  bytes+=lbytes;
  last=lnow;
  }
//......................................................................
 double AvgSpeed()
  {
  return seconds?(double)bytes/seconds:-1.;
  }
//......................................................................
 void Show()
  {
  REGu=NetSpeedToStr(REGs,speed);
  NetSpeedToStr(REGs+REGu,AvgSpeed());
  MessageBox(hmwnd,REGs,"TraficMeter",MB_OK);
  }
};

