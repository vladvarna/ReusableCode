//VERSION 1.0.a

#ifndef V_MATHS
#define	V_MATHS

#include <math.h>
#include <float.h>

//Constante matematice
#define PI  3.1415926535897932384626433832795
#define _2PI 6.28318530717958647692528676655901 //2*PI
#define PI2 1.57079632679489661923132169163975 //PI/2
#define _1_PI .31830988618379067153776752674503 //1/PI
#define SQRTPI 1.77245385 //sqrt(pi)

#define RAD(u) u*.01745329251994329576923690768 //u*PI/180
#define GRD(u) u*57.2957795130823208767981548141 //u*180/PI

#define EXP 2.718281828459045
#define LN10 2.3025850929940456840179914546844   //ln(10)
//Unitati de masura (mul first with define to obtain second)
#define INCH_M .0254 //1"=0.0254 m PPM_DPI
#define M_INCH 39.37 //1 m=39.37" DPI_PPM
#define INCH_CM 2.54 //1"=2.54 cm
#define CM_INCH .3937 //1 cm=0.3937"
#define INCH_MM 25.4 //1"=25.4 mm
#define MM_INCH .03937 //1 mm=0.03937"

#define FOOT_M .3048 //1 ft= 0.3048 m
#define M_FOOT 3.28084 //1 m= 3.28084 ft
//ounce avoirdupois
#define OZ_G 28.35 //g = 1 oz
#define G_OZ .03527337 //oz = 1 g
//pound avoirdupois (livra engleza)
#define LB_KG .453592 //kg = 1 lb
#define KG_LB 2.20462442 //lb = 1 kg

#define Kelv(T) (T+273.15)
#define Cels(T) (T-273.15)

char KMGT[]=" KMGTPEZY"; //multipli (Kilo,Mega,Giga,Tera,Peta,Exa,Zetta,Yotta)

//Macros
#define SQRT(p) sqrt(p)

// Fixed 16.16 fraction where the HIWORD is the signed mantissa(asm way) and the LOWORD is an unsigned fraction ~~~~~~
struct F16_16
{
 union
  {
  LONG l;
  WORD w[2];
  };
 F16_16(long dw=0)
  {
  l=dw;
  }
 operator double();
 long operator=(double);
 long operator=(float);
 long operator+();
};
// F16_16 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//use (double)F16_16 to transform to double.............................................
F16_16::operator double()
{
__asm
 {
 mov fpuioi,-16
 fild fpuioi //push -16
 fild [ecx]this.l //push l value
 fscale //st=st*2^-16=st/65536
 fstp st(1) //st(1)=st(0), return st(0)
 }
}
//initialize from float (can be used for conversion).............................................................
long F16_16::operator=(float fval)
{
__asm
 {
 mov fpuioi,16
 fild fpuioi //push 16
 fld fval //push float value
 fscale //st=st*2^16=st*65536
 fistp [ecx]this.l //l=st
 fstp fpuiof //clear reg
 mov eax,[ecx]this.l //return l
 }
}
//initialize from double (can be used for conversion).............................................................
long F16_16::operator=(double dval)
{
__asm
 {
 mov fpuioi,16
 fild fpuioi //push 16
 fld dval //push float value
 fscale //st=st*2^16=st*65536
 fistp [ecx]this.l //l=st
 fstp fpuiof //clear reg
 mov eax,[ecx]this.l //return l
 }
}
//use +F16_16 to return mantissa (rounded to nearest).....................................................................
long F16_16::operator+()
{
__asm
 {
 mov eax,[ecx]this.l
 sar eax,16
 adc eax,0
 }
}
// F16_16 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//returns index of max (to get value do: maxval=vector[findmax(vector,n)] )...........................................................................
template <typename vector> NAT findmax(vector*v,NAT n)
{
NAT m=0;
for(NAT i=1;i<n;i++)
 if(v[i]>v[m]) m=i;
return m;
}

//returns index of max (to get value do: maxval=vector[findmax(vector,n)] )...........................................................................
template <typename vector> NAT findmin(vector*v,NAT n)
{
NAT m=0;
for(NAT i=1;i<n;i++)
 if(v[i]<v[m]) m=i;
return m;
}

// TFI1 Transform integer between liniar spaces ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct TFI1
{
 int x0,xm,y0,ym; //src & dst interval (x0 maps to y0, x0+xm maps to y0+ym)
 double dps; //=ym/xm

 void Tf(int,int,int,int);
 int y(int);
 int f(int);
};
// TFI1 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//sets up for tf..................................................................
inline void TFI1::Tf(int s1=0,int s2=1,int d1=0,int d2=1)
 {
 x0=s1;
 y0=d1;
 xm=s2-s1;
 ym=d2-d1;
 dps=(double)ym/xm;
 }
//map x to y=(x-x0)*ym/xm+y0 (integer).....................................................
int TFI1::y(int x)
{
__asm
 {
 mov eax,x
 sub eax,[ecx]this.x0
 imul [ecx]this.ym
 idiv [ecx]this.xm
 add eax,[ecx]this.y0
 }
}
//map x to f(x)=(x-x0)*dps+y0 (real).....................................................
int TFI1::f(int x)
{
__asm
 {
 mov edx,[ecx]this.x0
 sub x,edx //x-=x0
 fild x
 fmul [ecx]this.dps //x*=dps
 fistp x
 mov eax,x
 add eax,[ecx]this.y0 //x+=y0
 }
}
// TFI1 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//round float using mode (0-nearest,1-down,2-up,3-chop) -----------------------------------
inline int __cdecl RoundF(float fio,DWORD rmod=0)
{
__asm
 {
 fld fio        //load float
 mov edx,rmod
 shl edx,10     //create round mask
 fstcw rmod      
 mov eax,rmod   //eax=cw
 and eax,0xf3ff
 or eax,edx     //eax=new cw
 mov fio,eax
 fldcw fio      //set rounding mode
 fistp fio      //float to int
 mov eax,fio    //read integer
 fldcw rmod     //restore rounding mode
 }
}

//number of times val can be divided by base ------------------------------------------
inline NAT __cdecl LogN(NAT base,NAT val)
{
__asm
 {
 xor ecx,ecx
 mov eax,val
 mov ebx,base
LDigitsLoop:
  xor edx,edx //cdq
  div ebx
  inc ecx
  cmp eax,0
  jnz LDigitsLoop
 mov eax,ecx
 }
}

//round down to a power of 2 -----------------------------------------------------------------
inline NAT __cdecl CutDPow2(NAT num)
{
__asm
 {
 mov ecx,num
 mov eax,1
 jecxz LCutDPow2RET
 bsr ecx,ecx //ecx=MSb
 shl eax,cl
LCutDPow2RET:
 }
}

//round up to a power of 2 ----------------------------------------------------------------
inline NAT __cdecl CutUPow2(NAT num)
{
__asm
 {
 mov ecx,num
 mov eax,1
 jecxz LCutUPow2RET
 bsf ebx,ecx //ebx=LSb
 bsr ecx,ecx //ecx=MSb
 cmp ebx,ecx //CF=LSb<MSb
 adc ecx,0   //if(LSb<MSb) cl++
 shl eax,cl
LCutUPow2RET:
 }
}

//snap num to grid by subtracting -----------------------------------------------------------------
inline int __cdecl CutD(int num,NAT grid=1)
{
__asm
 {
 mov eax,num
 mov ecx,grid
 cdq//xor edx,edx
 idiv ecx
 imul ecx
 }
}

//snap num to grid by rounding to nearest -----------------------------------------------------------------
inline int __cdecl CutM(int num,NAT grid=1)
{
__asm
 {
 mov eax,num
 mov ecx,grid
 mov edx,ecx
 shr edx,1
 add eax,edx //eax=num+grid/2
 cdq
 idiv ecx
 imul ecx
 }
}

//snap num to grid by addition-----------------------------------------------------------------
inline int __cdecl CutU(int num,NAT grid=1)
{
__asm
 {
 mov eax,num
 mov ecx,grid
 cdq
 idiv ecx
 cmp edx,0
 jz LReturn
 or edx,0x40000000 //set bit 30
 sar edx,30 //edx=(edx<0)?-1:1
 add eax,edx
LReturn:
 imul ecx
 }
}

//modulul vectorului in plan -----------------------------------------------------------------------
inline float Seg2(float ABx,float ABy)
{
return (float)SQRT(ABx*ABx+ABy*ABy);
}

//modulul vectorului in spatiu ---------------------------------------------------------------------
inline float Seg3(float ABx,float ABy,float ABz)
{
return (float)SQRT(ABx*ABx+ABy*ABy+ABz*ABz);
}

//calculeaza ecuatia unei drepte (det de 2 puncte A si B) intr-un punct P --------------------------
inline float PonL2(float *P,float *A,float *B)
{
return P[0]*(B[1]-A[1])+P[1]*(A[0]-B[0])+B[0]*A[1]-A[0]*B[1]; //if 0 P apartine AB
}

//verifica daca P este in int ABC ------------------------------------------------------------------
int PinT2(float *P,float *A,float *B,float *C)
{
if(PonL2(P,A,B)>0)
 {
 if(PonL2(P,B,C)>0)
  {
  if(PonL2(P,C,A)>0) return 1;
  }
 }
else
 {
 if(PonL2(P,B,C)<=0)
  {
  if(PonL2(P,C,A)<=0) return 1;
  }
 }
return 0;
}

//normalizes ( SUM(v[i])=1 ) a vector (or matrix) ------------------------------------------------------------------
template <typename tip> inline void NormVect(tip*v,NAT n)
{
NAT i;
double magn=v[0];
for(i=1;i<n;i++)
 magn+=v[i];
if(magn==0.||magn==1.) return;
for(i=0;i<n;i++)
 v[i]/=magn;
}

//media aritmetica ----------------------------------------------------------------------------
template <typename tip> inline double Mean(tip*v,NAT n)
{
double mean=v[0];
for(NAT i=1;i<n;i++)
 mean+=v[i];
return mean/n;
}

//varianta (squared std dev)----------------------------------------------------------------------------
template <typename tip> inline double Variance(tip*v,NAT n,tip*pmean=NULL)
{
double mean=0.,var=0.;
for(NAT i=0;i<n;i++)
 {
 mean+=v[i];
 var+=v[i]*v[i];
 }
mean/=n;
if(pmean) *pmean=mean;   //return mean
return var/n-mean*mean;	//D(v)=M(v^2)-M^2(v)
}

//cumulative moment 0th order ----------------------------------------------------------------------------
template <typename tip> inline double Moment0(tip*v,NAT n)
{
double miu=v[0];
for(NAT i=1;i<n;i++)
 miu+=v[i];
return miu;
}

//cumulative moment 1st order ----------------------------------------------------------------------------
template <typename tip> inline double Moment1(tip*v,NAT n)
{
double miu=v[0];
for(NAT i=1;i<n;i++)
 miu+=(i+1)*v[i];
return miu;
}

//median smooths data to remove noise (n>=W)--------------------------------------
template <typename tip> void DataSmooth(tip*d,tip*s,NAT n,NAT W=3)
{
NAT i,j,w=W>>1;
//beginning
for(i=0;i<w;i++)
 {
 d[i]=0;
 for(j=0;j<=i+w;j++)
  d[i]+=s[j];
 d[i]/=(i+w+1); 
 }
//middle
for(i=w;i<n-w;i++)
 {
 d[i]=0;
 for(j=i-w;j<=i+w;j++)
  d[i]+=s[j];
 d[i]/=W; 
 }
//ending
for(i=n-w;i<n;i++)
 {
 d[i]=0;
 for(j=i-w;j<n;j++)
  d[i]+=s[j];
 d[i]/=(n-i+w); 
 }
}

//universal sum of squares -----------------------------------------------------------------------
template <typename tip> double SquaresSum(tip*v,NAT n)
{
double sqrsum=0.;
for(NAT i=0;i<n;i++)
 sqrsum+=v[i]*v[i];
return sqrsum;
}

//universal sum of absolutes -----------------------------------------------------------------------
template <typename tip> double AbsolutesSum(tip*v,NAT n)
{
double abssum=0.;
for(NAT i=0;i<n;i++)
 abssum+=(v[i]>=0?v[i]:-v[i]);
return abssum;
}

//universal histogram -----------------------------------------------------------------------
template <typename tip> NAT* Histogram(tip*v,NAT n,tip start,tip stop,tip step)
{
NAT *hist,l;
l=(stop-start)/step+1;
hist=ALLOC_NAT(l);
ZeroMemory(hist,l*sizeof(NAT));
for(NAT i=0;i<n;i++)
 if(v[i]>=start&&v[i]<=stop)
  {
  l=(v[i]-start)/step;
  hist[l]++;
  }
return hist;
}

//Find global peaks [min/max(d)=global min/max(s)]-------------------------------------------------------------------------
template <typename tip> void MaxSmooth
(tip*d,tip*s,NAT n,int w=30)
{
NAT i;
int j,k;
for(i=0;i<n;i++)
 {
 d[i]=s[i]*n;
 k=n;
 for(j=i-1;j>=0;j--)
  {
  k-=w;
  if(k<(signed)(-n)) k=n;
  d[i]+=s[j]*ABS(k);
  }
 k=n;
 for(j=i+1;j<n;j++)
  {
  k-=w;
  if(k<(signed)(-n)) k=n;
  d[i]+=s[j]*ABS(k);
  }
 //ShowArray(d,n,hdbgwnd,0xff);
 //WaitVKey();
 }
}

#endif