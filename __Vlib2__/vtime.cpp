//VERSION 1.0.a
#pragma once

#include <rnd.cpp>

#define REGTMSZ				24
char REGtm[REGTMSZ];//YYYY-MM-DD HH:mm:ss:_ms
int MonthDays[12]={31,28,31,30,31,30,31,31,30,31,30,31};
LPSTR MonthName[12]={"January","February","March","April","May","June","July","August","September","October","November","December"};
LPSTR DayName[7]={"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
#ifdef V_ROMANA
 LPSTR NumeLuna[12]={"Ianuarie","Februarie","Martie","Aprilie","Mai","Iunie","Iulie","August","Septembrie","Octombrie","Noiembrie","Decembrie"};
 LPSTR NumeZi[7]={"Luni","Marti","Miercuri","Joi","Vineri","Sâmbata","Duminica"};
#endif

#define MINUTES_IN_DAY                  1440 //24*60
#define SECONDS_IN_DAY                 86400 //24*60*60
#define MILISECONDS_IN_DAY          86400000 //24*60*60*1000=0x5265C00

//Formats:
#define VTMF_VLAD 		0 //used to be 3
#define VTMF_JDMS		1 //date=JD, time=ms
//#define VTMF_UTC   		2
#define VTMF_STR   		4
#define VTMF_FILE  		5 //FILETIME (unsupported)

//Calendar types
#define VTMC_AUTO		0xF0 //choose between JULIAN and GREGORIAN (15-Oct-1582)
#define VTMC_GREGORIAN	0x00
#define VTMC_JULIAN		0x10
#define VTMC_HERSCHEL	0x20 //4000 not leap year (TODO)

#define VUID_JD_BASE 	2451576 //2000-1(January)-1
//Julian Day adjustments
int JDBaseAdj[16]={0,2400000,2299160,VUID_JD_BASE,0,0,0,0,0,0,0,0,0,0,0,0}; //JD,MJD,Lilian,VUID

#define VTM_ASPECT_USE_T            0x0000000080000000i64 //separates date and time with 'T' rather than space
#define VTM_ASPECT_ISO_DATETIME_T		0x14404548cb4e113fi64 //YYYY-MM-DDTHH:mm:ss
#define VTM_ASPECT_ISO_DATETIME			0x144045484b4e113fi64 //YYYY-MM-DD HH:mm:ss
#define VTM_ASPECT_ISO_DATE					0x144045083f3f3f3fi64 //YYYY-MM-DD
#define VTM_ASPECT_ISO_TIME					0x103f3f3f4043063fi64 //HH:mm:ss
#define VTM_ASPECT_ISO_SHORTTIME		0x103f3f3f40033f3fi64 //HH:mm
#define VTM_ASPECT_ISO_FULLTIME     0x103f3f3f40434649i64 //HH:mm:ss:ms
//Aspects: Prototype: 0xFLYYMMDDhhmmssmsi64 
BITS64 VTMAspect[]={0,0x140206083f3f3f3fi64}; //when adding, dont' forget to update ASPECT_INDEX_MASK
#define ASPECT_INDEX_MASK 0x1  //maximum ASPECT indexes -1

#define VTM_DAYLIGHTSAVING 0x000100000 //daylight change in effect
//Virtual Time Stamp ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class VTIME
{
public:
struct
 {
 union
  {
  BITS32 tm0;
  struct{ char b0,b1,ch0,ch1; };
  };
 union
  {
  struct{ BITS32 tm1,tm2; };
  FILETIME filtm;
  BITS64 tm;
  double utc;
  };
 }; 	           

 VTIME() { tm0=VTMF_VLAD; tm1=tm2=0; };
 void init(DWORD);
 void clone(VTIME*,DWORD);
 BOOL conv(NAT); //convert internal format (by Vlad date&time)
 void setTime(NAT,NAT,NAT,NAT); //h,m,s,ms
 void getTime(NAT&,NAT&,NAT&,NAT&); //h,m,s,ms
 void setms(NAT);
 NAT  getms();
 void setTZ(int);
 int  getTZ();
 void setDate(int,NAT,NAT,int); //Y,M,D,c
 void getDate(int&,NAT&,NAT&,int&); //Y,M,D,c
 void setJD(NAT,int); //number of days since 1/Jan/-4712
 NAT  getJD(); //number of days since 1/Jan/-4712
 NAT  getWeek(); //0-53
 NAT  getWeekDay(); //0-L,1-M,2-M,3-J,4-V,5-S,6-D
 QWORD miliseconds(int);
 void add(int,int,int,int,int,int,int); //h,m,s,ms
 void setStr(LPSTR,BITS64,NAT);
 void getStr(LPSTR,BITS64,NAT);
 
 char* to_str(BITS64);
 char* filepath();
 operator char*();
 operator SYSTEMTIME();
 operator FILETIME();
 VTIME operator=(VTIME);
 VTIME operator=(VTIME*);
 VTIME operator=(SYSTEMTIME);
 VTIME operator=(SYSTEMTIME*);
 VTIME operator=(FILETIME);
 VTIME operator=(FILETIME*);
 BOOL operator==(VTIME);
 BOOL operator<(VTIME);
 BOOL operator>(VTIME);
 void SysTm(); //
 void Now(); //
 int D_ms(VTIME*);
 int setTimer(HWND,int); //MUST use KillTimer()
 void fromDTPicker(HWND);
 void toDTPicker(HWND);
 void Schedule(int,int,int,int,int,int,int); //h,m,s,ms
 void User(char*,char*,HWND,DWORD);
 void show();
}gtm; //global time

//determine if a year is leap in Julian or Gregorian calendar --------------------------------
inline BOOL IsLeapYear(int year,int cal=VTMC_GREGORIAN)
{
if(cal==VTMC_GREGORIAN) //Gregorian
 return year%400?(year%100?(year&3?0:1):0):1; //divisible by 4, not divisible by 100 or divisible by 400
else if(cal==VTMC_HERSCHEL) //John Herschel
 return year%4000?(year%400?(year%100?(year&3?0:1):0):1):0; //divisible by 4, not divisible by 100 or divisible by 400 but not divisible by 4000
else  //Julian
 return !(year&3); //divisible by 4
}

//determine if a year is leap in Julian or Gregorian calendar --------------------------------
inline int CorectMonthDays(int month,int year,int cal=VTMC_GREGORIAN)
{
month%=12;
if(month<0) month+=12;
if(month!=1) return MonthDays[month];
if(cal==VTMC_GREGORIAN) //Gregorian
 return year%400?(year%100?(year&3?28:29):28):29; //divisible by 4, not divisible by 100 or divisible by 400
else if(cal==VTMC_HERSCHEL) //John Herschel
 return year%4000?(year%400?(year%100?(year&3?28:29):28):29):28; //divisible by 4, not divisible by 100 or divisible by 400 but not divisible by 4000
else  //Julian
 return year&3?28:29; //divisible by 4
}

//returns x=[0,max) and a proper overflow (special version) --------------------------------------------------------------------------------------------
inline int AddIntCarryOverDays(int&day,int dx=1,int c=0,int month=0,int year=1,int cal=0)
{ //day=[1,MD]
day+=dx+c;
c=0;
while(day>(dx=CorectMonthDays(month+c,year,cal)))
 {
 c++;
 day-=dx;
 }
while(day<1)
 {
 c--; //! this order matters
 day+=CorectMonthDays(month+c,year,cal);
 }
return c; //overflow 
}

//converts julian day number to date ------------------------------------------------------------
void DateFromJD(int &year,NAT &month,NAT &day,NAT jd=0,int cal=VTMC_GREGORIAN)
{
int a,b,c,d,e,m;
if(cal==VTMC_JULIAN) //Julian
 {
 b=0;
 c=jd+32082;
 }
else //if(cal==VTMC_GREGORIAN) //Gregorian
 {
 a=jd+32044;
 b=(4*a+3)/146097;
 c=a-146097*b/4;
 }
d=(4*c+3)/1461;
e=c-(1461*d/4);
m=(5*e+2)/153;
day=e-(153*m+2)/5+1;
month=m+2-12*(m/10);
year=100*b+d-4800+m/10;
}

//converts date to julian day number ------------------------------------------------------------
NAT DateToJD(int year,NAT month,NAT day,int cal=VTMC_GREGORIAN)
{
int y,m;
NAT a;
a=(13-month)/12;
y=year+4800-a;
m=month+12*a-2;
if(cal==VTMC_GREGORIAN) //Gregorian
 return day+(153*m+2)/5+365*y+y/4-y/100+y/400-32045;
else if(cal==VTMC_JULIAN)//Julian
 return day+(153*m+2)/5+365*y+y/4-32083;
else
 return 0; 
}

//get week day (0-L,1-M,2-M,3-J,4-V,5-S,6-D) from date ------------------------------------------
NAT WeekDay(int year,NAT month,NAT day,int cal=VTMC_GREGORIAN)
{
int y,m;
NAT a;
a=(13-month)/12;
y=year-a;
m=month+12*a-1;
if(cal==VTMC_GREGORIAN) //Gregorian
 return (day+y-1+y/4-y/100+y/400+31*m/12)%7; //day advances 1 for each year, 2 for leap years
else if(cal==VTMC_JULIAN)//Julian
 return (day+4+y+y/4+31*m/12)%7; //day advances 1 for each year, 2 for leap years
else
 return 0; 
}

//convert speed in B/s to a printable string ----------------------------------------------------
NAT NanosecToStr(char*buffer,double nanosec)
{
char*suf;
if(nanosec>1000.)
 {
 nanosec/=1000.; //miliseconds
 if(nanosec>1000.)
  {
  nanosec/=1000.; //seconds
  if(nanosec>60.)
   {
   nanosec/=60.; //minutes
   if(nanosec>60.)
    {
    nanosec/=60.; //hours
    if(nanosec>24.)
     {
     nanosec/=24.; //days
     suf="days";
     }
    else
     suf="h";
    }
   else
    suf="min";
   }
  else
   suf="s";
  } 
 else
  suf="ms";
 }
else
 suf="ns";
return sprintf(buffer,"%.1f%s",nanosec,suf);
}

//Extract time&date from string based on aspect ------------------------------------------------
BOOL TimeFromAspect(LPSTR dts,BITS64 asp,int&Y,NAT&MW,NAT&DW,NAT&h,NAT&m,NAT&s,NAT&ms,int&W,NAT dtsnc=256)
{
int aw=0;
if((asp>>60)==1) //ISO8601 -fixed
 {
 if(((asp>>48)&0x3F)!=0x3F) //Year
  Y=StoI(dts+((asp>>48)&0x3F),10,NULL,(asp>>56)&0xF);
 if(asp&0x80000000000000i64) //BC
  {
  if(*(dts+((asp>>48)&0x3F)+((asp>>56)&0xF))=='B')
   Y=-Y;
  }
 if(asp&0x800000000000i64) //AW
  aw=W=(*(dts+((asp>>40)&0x3F))=='W');
 else
  W=((asp&0x8000000000i64)!=0); //W
 if(((asp>>40)&0x3F)!=0x3F) //Month / Week
  {
  MW=StoU(dts+((asp>>40)&0x3F)+aw,10,NULL,2);
  MW=W?(MW-1)%53:(MW-1)%12;
  }
 if(((asp>>32)&0x3F)!=0x3F) //Day / WeekDay
  {
  DW=StoU(dts+((asp>>32)&0x3F)+aw,10,NULL,W?1:2);
  DW=W?(DW-1)%7:(DW%32);
  }
 if(((asp>>24)&0x3F)!=0x3F)
  h=StoU(dts+((asp>>24)&0x3F),10,NULL,2)%24;
 if(((asp>>16)&0x3F)!=0x3F)
  m=StoU(dts+((asp>>16)&0x3F),10,NULL,2)%60;
 if((((asp>>8)&0x3F)!=0x3F)&&(((asp>>8)&0x3F)<dtsnc-1))
  s=StoU(dts+((asp>>8)&0x3F),10,NULL,2)%60;
 if(((asp&0x3F)!=0x3F)&&((asp&0x3F)<dtsnc-2))
  ms=StoU(dts+(asp&0x3F),10,NULL,3)%1000;
 }
else
 error("Unsupported ASPECT in TimeFromAspect()");
return 0;
}

//Print time&date to string based on aspect ------------------------------------------------
BOOL TimeToAspect(LPSTR dts,BITS64 asp,int Y,NAT MW=0,NAT DW=0,NAT h=0,NAT m=0,NAT s=0,NAT ms=0)
{
int aw;
if((asp>>60)==1) //ISO8601 -fixed
 {
 if(((asp>>48)&0x3F)!=0x3F) //Year
  ItoS(Y,dts+((asp>>48)&0x3F),10,(asp>>56)&0xF,asp&0x40000000000000i64?'-':C_NULL,'0');
 aw=((asp&0x8000000000i64)!=0); //W
 if(((asp>>40)&0x3F)!=0x3F) //Month / Week
  {
  if(aw) *(dts+((asp>>40)&0x3F))='W';
  UtoS(MW+1,dts+((asp>>40)&0x3F)+aw,10,2,asp&0x400000000000i64?'-':C_NULL,'0');
  }
 if(((asp>>32)&0x3F)!=0x3F) //Day / WeekDay
  UtoS(DW+aw,dts+((asp>>32)&0x3F)+aw,10,aw?1:2,asp&0x4000000000i64?(asp&VTM_ASPECT_USE_T?'T':' '):C_NULL,'0');
 if(((asp>>24)&0x3F)!=0x3F)
  UtoS(h,dts+((asp>>24)&0x3F),10,2,asp&0x40000000i64?':':C_NULL,'0');
 if(((asp>>16)&0x3F)!=0x3F)
  UtoS(m,dts+((asp>>16)&0x3F),10,2,asp&0x400000i64?':':C_NULL,'0');
 if(((asp>>8)&0x3F)!=0x3F)
  UtoS(s,dts+((asp>>8)&0x3F),10,2,asp&0x4000i64?':':C_NULL,'0');
 if((asp&0x3F)!=0x3F)
  UtoS(ms,dts+(asp&0x3F),10,3,asp&0x40i64?'\0':C_NULL,'0');
 }
else
 error("Unsupported ASPECT in TimeToAspect()");
return 0;
}

//VTIME <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//............................................................................................
void VTIME::init(DWORD fmt=VTMF_VLAD)
{
tm0=fmt;
switch(tm0&0xF) //internal format
 {
 case VTMF_VLAD:
 case VTMF_JDMS:
 case VTMF_FILE:
  tm1=tm2=0;
  return;
 case VTMF_STR:
  tm0=(tm0&0xffff00ff)|0x100; //ASPECT 1
  tm1=tm2=0;
  return;
 }
}

//............................................................................................
void VTIME::setTime(NAT h=0,NAT m=0,NAT s=0,NAT ms=0)
{
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
  tm2=((h%24)*3600000)+((m%60)*60000)+((s%60)*1000)+(ms%1000); //miliseconds
  return;
 case VTMF_VLAD:
  tm2=((h%24)<<27)|((m%60)<<16)|((s%60)<<10)|(ms%1000);
  return;
 default:
  error("VTIME not initialized");
 }
}

//............................................................................................
void VTIME::getTime(NAT &h,NAT &m,NAT &s,NAT &ms)
{
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
  ms=tm2&0x07ffffff; //miliseconds
  h=(ms/3600000)%24;
  ms%=3600000;
  m=ms/60000;
  ms%=60000;
  s=ms/1000;
  ms%=1000;
  return;
 case VTMF_VLAD:
  ms=(tm2&0x3ff)%1000;
  s=((tm2>>10)&0x3f)%60;
  m=((tm2>>16)&0x3f)%60;
  h=((tm2>>27)&0x1f)%24;
  return;
 default:
  error("VTIME not initialized");
 }
}

//............................................................................................
void VTIME::setms(NAT ms=0)
{
NAT h,m,s;
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
  tm2=ms&0x07ffffff; //miliseconds
  return;
 case VTMF_VLAD:
  ms&=0x07ffffff; //miliseconds
  h=(ms/3600000)%24;
  ms%=3600000;
  m=ms/60000;
  ms%=60000;
  s=ms/1000;
  ms%=1000;
  tm2=((h%24)<<27)|((m%60)<<16)|((s%60)<<10)|(ms%1000);
  return;
 default:
  error("VTIME not initialized");
 }
}

//............................................................................................
NAT VTIME::getms()
{
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
  return tm2&0x07ffffff; //miliseconds
 case VTMF_VLAD:
  return (((tm2>>27)%24)*3600000)+((((tm2>>16)&0x3f)%60)*60000)+((((tm2>>10)&0x3f)%60)*1000)+((tm2&0x3ff)%1000); //miliseconds
 default:
  error("VTIME not initialized");
 }
return 0;
}

//............................................................................................
void VTIME::setTZ(int tz=0)
{
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
 case VTMF_VLAD:
  tm0=((tz)<<21)|(tm0&0xFFFFF); //set time zone (keep the rest)
 default:
  error("VTIME not initialized");
 }
}

//............................................................................................
int VTIME::getTZ()
{
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
 case VTMF_VLAD:
  return ((int)tm0)>>21; //get time zone
 default:
  error("VTIME not initialized");
 }
return 0;
}

//............................................................................................
void VTIME::setDate(int y=0,NAT m=0,NAT d=0,int cal=VTMC_AUTO)
{
NAT nz;
if(cal==VTMC_AUTO) //determine calendar based on switch date (15Oct1582)
 cal=y>1582?VTMC_GREGORIAN:VTMC_JULIAN; //TODO: compare with month=9 and day=15 
m%=12;
if(d<1)
 d=1; //no 0 day
else if(d>(nz=CorectMonthDays(m,y,cal)))
 d=nz;
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
  tm0=(tm0&0xffffff0f)|cal;
  tm1=DateToJD(y,m,d,cal)-JDBaseAdj[(tm0>>8)&0xf];
  return;
 case VTMF_VLAD:
  tm0=(tm0&0xffffff0f)|cal;
  tm1=(y<<9)|(m<<5)|d;
  return;
 case VTMF_STR:
  tm0=(tm0&0xffffff0f)|cal;
  TimeToAspect(&ch0,VTMAspect[(tm0>>8)&ASPECT_INDEX_MASK],y,m,d);
  return;
 default:
  error("VTIME not initialized");
 }
}

//............................................................................................
void VTIME::getDate(int &y,NAT &m,NAT &d,int &cal)
{
int W;
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
  cal=tm0&0xf0;
  DateFromJD(y,m,d,tm1+JDBaseAdj[(tm0>>8)&0xf],cal);
  return;
 case VTMF_VLAD:
  cal=tm0&0xf0;
  y=((int)tm1)>>9;
  m=(tm1>>5)&0xf;
  d=tm1&0x1f;
  return;
 case VTMF_STR:
  cal=tm0&0xf0;
  TimeFromAspect(&ch0,VTMAspect[(tm0>>8)&ASPECT_INDEX_MASK],y,m,d,d,d,d,d,W);
  WARN(W,"YYYY-Www-WD format not supported in VTIME");
  return;
 default:
  error("VTIME not initialized");
 }
}

//............................................................................................
void VTIME::setJD(NAT jd=0,int cal=VTMC_GREGORIAN)
{
int y;
NAT m,d;
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
  tm0=(tm0&0xffffff0f)|cal;
  tm1=jd-JDBaseAdj[(tm0>>8)&0xf];
  return;
 case VTMF_VLAD:
  DateFromJD(y,m,d,jd,cal);
  tm0=(tm0&0xffffff0f)|cal;
  tm1=(y<<9)|(m<<5)|d;
  return;
 default:
  error("VTIME not initialized");
 }
}

//............................................................................................
NAT VTIME::getJD()
{
switch(tm0&0xF) //internal format
 {
 case VTMF_JDMS:
  return tm1+JDBaseAdj[(tm0>>8)&0xf];
 case VTMF_VLAD:
  return DateToJD(((int)tm1)>>9,(tm1>>5)&0xf,tm1&0x1f,tm0&0xf0);
 default:
  error("VTIME not initialized");
 }
return 0;
}

//............................................................................................
inline NAT VTIME::getWeekDay() //0-L,1-M,2-M,3-J,4-V,5-S,6-D
{
int y,cal;
NAT m,d;
getDate(y,m,d,cal);
return WeekDay(y,m,d,cal);
}

//............................................................................................
inline NAT VTIME::getWeek() //0-53
{
NAT JD,d4,d1,L;
JD=getJD();
d4 = (JD+31741-(JD%7))%146097%36524%1461;
L=d4/1460;
d1=((d4-L)%365)+L;
return d1/7+1;
}

//date and time as number of milisesc since base ............................................................................................
QWORD VTIME::miliseconds(int base=0)
{//base is just for future reference (0=1-ian-1601)
FILETIME ft;
QUADW ns;
ft=*this;
ns.u[0]=ft.dwLowDateTime;
ns.u[1]=ft.dwHighDateTime;
return ns.u64/10000;
}

//............................................................................................
inline VTIME VTIME::operator=(VTIME lvtm)
{
tm0=lvtm.tm0;
tm=lvtm.tm;
return *this;
}

//............................................................................................
inline VTIME VTIME::operator=(VTIME*pvtm)
{
CopyMemory(this,pvtm,sizeof(VTIME));
return *this;
}

//............................................................................................
inline VTIME VTIME::operator=(SYSTEMTIME st)
{
setTime(st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
setDate(st.wYear,st.wMonth-1,st.wDay);
return *this;
}

//............................................................................................
inline VTIME VTIME::operator=(SYSTEMTIME*pst)
{
setTime(pst->wHour,pst->wMinute,pst->wSecond,pst->wMilliseconds);
setDate(pst->wYear,pst->wMonth-1,pst->wDay);
return *this;
}

//............................................................................................
inline VTIME VTIME::operator=(FILETIME ft)
{
SYSTEMTIME st;
FileTimeToSystemTime(&ft,&st);
setTime(st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
setDate(st.wYear,st.wMonth-1,st.wDay);
return *this;
}

//............................................................................................
inline VTIME VTIME::operator=(FILETIME*pft)
{
SYSTEMTIME st;
FileTimeToSystemTime(pft,&st);
setTime(st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
setDate(st.wYear,st.wMonth-1,st.wDay);
return *this;
}

//............................................................................................
inline VTIME::operator SYSTEMTIME()
{
SYSTEMTIME st;
NAT h=0,m=0,s=0,ms=0,M=0,d=0;
int y=0,c=0;
getTime(h,m,s,ms);
getDate(y,M,d,c);
st.wHour=h;
st.wMinute=m;
st.wSecond=s;
st.wMilliseconds=ms;
st.wDay=d;
st.wDayOfWeek=getWeekDay();
st.wMonth=M+1;
st.wYear=y;
return st;
}

//100-nanosecond (=10^-7seconds=1/10 microseconds=1/10000miliseconds)  intervals since January 1, 1601 (UTC) ............................................................................................
inline VTIME::operator FILETIME()
{
SYSTEMTIME st;
FILETIME ft;
st=*this;
SystemTimeToFileTime(&st,&ft);
return ft;
}

//............................................................................................
inline BOOL VTIME::operator==(VTIME lvtm)
{
NAT h1=0,m1=0,s1=0,ms1=0,M1=0,d1=0,h2=0,m2=0,s2=0,ms2=0,M2=0,d2=0;
int y1=0,y2=0,c;
getDate(y1,M1,d1,c);
lvtm.getDate(y2,M2,d2,c);
if(y1!=y2) return 0;
if(M1!=M2) return 0;
if(d1!=d2) return 0;
getTime(h1,m1,s1,ms1);
lvtm.getTime(h2,m2,s2,ms2);
if(h1!=h2) return 0;
if(m1!=m2) return 0;
if(s1!=s2) return 0;
if(ms1!=ms2) return 0;
return 1;
}

//............................................................................................
inline BOOL VTIME::operator<(VTIME lvtm)
{
NAT h1=0,m1=0,s1=0,ms1=0,M1=0,d1=0,h2=0,m2=0,s2=0,ms2=0,M2=0,d2=0;
int y1=0,y2=0,c;
getDate(y1,M1,d1,c);
lvtm.getDate(y2,M2,d2,c);
if(y1>y2) return 0;
else if(y1<y2) return 1;
if(M1>M2) return 0;
else if(M1<M2) return 1;
if(d1>d2) return 0;
else if(d1<d2) return 1;
getTime(h1,m1,s1,ms1);
lvtm.getTime(h2,m2,s2,ms2);
if(h1>h2) return 0;
else if(h1<h2) return 1;
if(m1>m2) return 0;
else if(m1<m2) return 1;
if(s1>s2) return 0;
else if(s1<s2) return 1;
if(ms1>ms2) return 0;
else if(ms1<ms2) return 1;
return 1;
}

//............................................................................................
inline BOOL VTIME::operator>(VTIME lvtm)
{
NAT h1=0,m1=0,s1=0,ms1=0,M1=0,d1=0,h2=0,m2=0,s2=0,ms2=0,M2=0,d2=0;
int y1=0,y2=0,c;
getDate(y1,M1,d1,c);
lvtm.getDate(y2,M2,d2,c);
if(y1>y2) return 1;
else if(y1<y2) return 0;
if(M1>M2) return 1;
else if(M1<M2) return 0;
if(d1>d2) return 1;
else if(d1<d2) return 0;
getTime(h1,m1,s1,ms1);
lvtm.getTime(h2,m2,s2,ms2);
if(h1>h2) return 1;
else if(h1<h2) return 0;
if(m1>m2) return 1;
else if(m1<m2) return 0;
if(s1>s2) return 1;
else if(s1<s2) return 0;
if(ms1>ms2) return 1;
else if(ms1<ms2) return 0;
return 1;
}

//............................................................................................
BOOL VTIME::conv(NAT nfmt)
{
return 0; 
}

//UTC ............................................................................................
inline void VTIME::SysTm()
{
SYSTEMTIME st;
GetSystemTime(&st);
setTime(st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
setDate(st.wYear,st.wMonth-1,st.wDay);
}

//local time ............................................................................................
inline void VTIME::Now()
{
SYSTEMTIME st;
GetLocalTime(&st);
setTime(st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
setDate(st.wYear,st.wMonth-1,st.wDay);
}

//............................................................................................
inline void VTIME::setStr(LPSTR fstr,BITS64 aspect,NAT fstrnc=256)
{
NAT h=0,m=0,s=0,ms=0,M=0,d=0;
int y=0,w=0; //w ignored
TimeFromAspect(fstr,aspect,y,M,d,h,m,s,ms,w,fstrnc);
setTime(h,m,s,ms);
setDate(y,M,d);
}

//............................................................................................
inline void VTIME::getStr(LPSTR fstr,BITS64 aspect,NAT fstrnc=256)
{
NAT h=0,m=0,s=0,ms=0,M=0,d=0;
int y=0,c=0; //c ignored
getTime(h,m,s,ms);
getDate(y,M,d,c);
ZeroMemory(fstr,fstrnc);
TimeToAspect(fstr,aspect,y,M,d,h,m,s,ms);
}

//............................................................................................
inline void VTIME::add(int dy=1,int dM=0,int dd=0,int dh=0,int dm=0,int ds=0,int dms=0)
{
int y=0,M=0,d=0,h=0,m=0,s=0,ms=0,c;
getTime((NAT&)h,(NAT&)m,(NAT&)s,(NAT&)ms);
getDate(y,(NAT&)M,(NAT&)d,c);
//ms+=dms; s+=ds; m+=dm; h+=dh; d+=dd; M+=dM; y+=dy;
dms=AddIntCarryOver(s,dms,0,1000);
ds=AddIntCarryOver(s,ds,dms,60);
dm=AddIntCarryOver(m,dm,ds,60);
dh=AddIntCarryOver(h,dh,dm,24);
dd=AddIntCarryOverDays(d,dd,dh,M,y,c);
dM=AddIntCarryOver(M,dM,dd,12);
y+=dy+dM;
setDate(y,M,d,c);
setTime(h,m,s,ms);
}

//............................................................................................
inline void VTIME::Schedule(int dy=1,int dM=0,int dd=0,int dh=0,int dm=0,int ds=0,int dms=0)
{
VTIME ltm;
ltm.Now();
while(*this<ltm)
 {
 add(dy,dM,dd,dh,dm,ds,dms);
 ltm.Now();
 }
}
 
//based on a REG ! ............................................................................................
inline VTIME::operator char*()
{
getStr(REGtm,VTM_ASPECT_ISO_DATETIME,REGTMSZ);
return REGtm;
}

//based on a REG ! ............................................................................................
inline char* VTIME::to_str(BITS64 asp=VTM_ASPECT_ISO_DATETIME)
{
getStr(REGtm,asp,REGTMSZ);
return REGtm;
}

//based on a REG ! generates a safe name for a file............................................................................................
inline char* VTIME::filepath()
{
NAT h=0,m=0,s=0,ms=0,M=0,d=0;
int y=0,c=0;
getTime(h,m,s,ms);
getDate(y,M,d,c);
sprintf(REGtm,"%04i-%02i-%02i %02i~%02i~%02i~%02i",y,M+1,d,h,m,s,ms);
return REGtm;
}

//............................................................................................
inline int VTIME::D_ms(VTIME*pvtm)
{
return (signed)getms()-(signed)pvtm->getms();
}

//set a timer based on current time ............................................................................................
inline int VTIME::setTimer(HWND hwnd=NULL,int identifier=0)
{
QWORD then,now;
VTIME ctm;
then=miliseconds();
ctm.Now();
now=ctm.miliseconds();
if(then<now)
 return SetTimer(hwnd,identifier,1,NULL); //if setting it in the past, run now
return SetTimer(hwnd,identifier,then-now,NULL);
}

//............................................................................................
void VTIME::fromDTPicker(HWND hdtpicker)
{
SYSTEMTIME st;
SendMessage(hdtpicker,DTM_GETSYSTEMTIME,0,(LPARAM)&st);
setTime(st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
setDate(st.wYear,st.wMonth-1,st.wDay);
}

//............................................................................................
void VTIME::toDTPicker(HWND hdtpicker)
{
SYSTEMTIME st;
st=*this;
SendMessage(hdtpicker,DTM_SETSYSTEMTIME,0,(LPARAM)&st);
}

//............................................................................................
void VTIME::User(char*title,char*format,HWND parent=HWND_DESKTOP,DWORD style=0)
{//DTS_SHOWNONE|DTS_UPDOWN|DTS_TIMEFORMAT|DTS_LONGDATEFORMAT|DTS_SHORTDATE
HWND hdlg,hdt,hok,hcancel;
SIZE z;
HDC hdc;
int block;
if(format)
 {
 hdc=GetDC(parent);
 GetTextExtentPoint32(hdc,format,sl(format),&z);
 ReleaseDC(parent,hdc);
 z.cy+=4; //add border
 }
else
 {
 z.cx=200;
 z.cy=20;
 } 
hdlg=CreateWindowEx(0,WC_DEFAULT_PROC,title,WS_POPUPWINDOW|WS_CAPTION,
		    0,0,z.cx+100,z.cy*6,parent,NULL,appinst,NULL);
hdt=CreateWindowEx(WS_EX_CLIENTEDGE,DATETIMEPICK_CLASS,"",WS_VISIBLE|WS_CHILD|style,
                   50,z.cy,z.cx,z.cy,hdlg,(HMENU)EDIT(0),appinst,NULL);
hok=CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
                  ((z.cx+100)>>2)-30,z.cy*3,60,z.cy,hdlg,(HMENU)BUTT(0),appinst,NULL);
hcancel=CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_TABSTOP|WS_CHILD,
                      ((z.cx+100)>>2)*3-30,z.cy*3,60,z.cy,hdlg,(HMENU)BUTT(1),appinst,NULL);
if(format)
 SendMessage(hdt,DTM_SETFORMAT,0,(LPARAM)format);
if(parent)
 {
 CenterDlg(hdlg,parent);
 EnableWindow(parent,FALSE);
 }
else
 CenterOnScreen (hdlg);
toDTPicker(hdt);
ShowWindow(hdlg,SW_SHOW);
SetFocus(hdt);
block=TRUE;
MSG message;
while(block)
 {
 if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
  {
  if(message.message==WM_KEYDOWN)
   {
   if(WK_SCANCODE(message.lParam)==28) //ENTER
    SendMessage(hok,BM_SETSTATE,BST_PUSHED,0);
   else if(WK_SCANCODE(message.lParam)==1) //ESC
    SendMessage(hcancel,BM_SETSTATE,BST_PUSHED,0);
   }
  if(!IsDialogMessage(hdlg,&message))
   {
   TranslateMessage(&message);
   DispatchMessage(&message);
   }
  }
 if(SendMessage(hok,BM_GETSTATE,0,0)&BST_PUSHED) //Ok
  {
  fromDTPicker(hdt);
  block=FALSE;
  }
 if(SendMessage(hcancel,BM_GETSTATE,0,0)&BST_PUSHED) //Cancel
  block=FALSE;
 if(!IsWindow(hdlg)) block=FALSE;
 }
EnableWindow(parent,TRUE);
DestroyWindow(hdlg);
}

//............................................................................................
void VTIME::show()
{
NAT h=0,m=0,s=0,ms=0,M=0,d=0;
int y=0,c=0;
getTime(h,m,s,ms);
getDate(y,M,d,c);
printbox("%02i:%02i:%02i:%02i + %i = %u ms\n%04i-%02i(%s)/W%i-%02i(%s) JD=%u\nis %sleap year (%s)",
          h,m,s,ms,getTZ(),getms(),y,M+1,MonthName[M],getWeek(),d,DayName[getWeekDay()],getJD(),IsLeapYear(y,c)?"":"NOT ",c?"Julian":"Gregorian");
}

//VTIME >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//generates a unique id (accurate above 8 nsec) ----------------------------------------------------------------------
unsigned __int64 GenVUID1()
{
SYSTEMTIME st;
unsigned __int64 jd,jh,rn;
GetSystemTime(&st);
jh=(st.wHour*3600000)+(st.wMinute*60000)+(st.wSecond*1000)+st.wMilliseconds; //0x07ffffff (27)
jd=DateToJD(st.wYear,st.wMonth-1,st.wDay)-VUID_JD_BASE; //0xfffff (20) (wraps in 4870-11-26)
rn=RND(0x1ffff); //(17)
return ((jd&0xfffff)<<44)|((jh&0x7ffffff)<<17)|rn;	//20|27|17
}

#define VUID GenVUID1()

