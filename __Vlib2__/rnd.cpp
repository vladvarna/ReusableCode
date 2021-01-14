#ifndef V_RANDOM
#define V_RANDOM

#define DEFAULT_SEED GetTickCount()
#define LCRNG_A 0x15a4e35 //used in BC++3.1

// RNG <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
struct RNG //RN[k]=a*RN[k-1] (LCG)
{
 NAT RN; //M->max value (also mask if M=2^m)

 RNG()
  {
  RN=DEFAULT_SEED;
  }
 //generates one number ...........................................................
 NAT Gen()
  {
  return RN=RN*LCRNG_A;
  }
 //random number between [min,max) .........................................................
 NAT Rnd(int max=0xffffffff)
  {
  RN=RN*LCRNG_A;
  return (__int64)RN*max/0xffffffff;
  }
 //random number between [min,max) .........................................................
 int Rand(int min=0,int max=0x7fffffff)
  {
  RN=RN*LCRNG_A;
  return (__int64)RN*(max-min)/0xffffffff+min;
  }
 //float random number between [min,max) .........................................................
 float Randf(float min=0.f,float max=1.f)
  {
  RN=RN*LCRNG_A;
  return (double)RN*(max-min)/4294967295.+min;
  }
}REGrnd;
// RNG >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#define RND(M)      REGrnd.Rnd(M)
#define RAND(m,M)   REGrnd.Rand(m,M)
#define RANDf(m,M)  REGrnd.Randf(m,M)

//Linear Congruential Random Number Generator <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//RN[k]=(a*RN[k-1]+b)%M
struct LCRNG
{
 NAT RN,a,b,M; //M->max value (also mask if M=2^m)

 LCRNG()
  {
  a=LCRNG_A;
  b=1;
  M=0xffffffff;
  RN=DEFAULT_SEED;
  }
 //sets up the parameters ...........................................................
 void Init(NAT la=16807,NAT lb=0,NAT lM=0xffffffff)
  {
  a=la;
  b=lb;
  M=lM;
  }
 //generates one number ...........................................................
 NAT Gen()
  {
  return RN=(RN*a+b)%M;
  }
 //random number between [min,max) .........................................................
 int Rand(int min=0,int max=0x7fffffff)
  {
  RN=(RN*a+b)%M;
  return (__int64)RN*(max-min)/M+min;
  }
 //float random number between [min,max) .........................................................
 float Randf(float min=0.f,float max=1.f)
  {
  RN=(RN*a+b)%M;
  return (double)RN*(max-min)/M+min;
  }
};
//LCRNG >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>.

//pune count valori intre [min,max] in rndbuf ---------------------------------------------
void fillrnd(BYTE *rndbuf,unsigned count=0,int min=0,int max=0xff)
{
while(count>0)
 {
 *rndbuf=RAND(min,max);
 rndbuf++;
 count--;
 }
}

//also unscramble (because of XOR) -----------------------------------------------------------------------------------------------
void ScramblePassword(char*pwd,NAT pwdnc=0,int key=0)
{
LCRNG rn;
rn.Init(LCRNG_A,1,0xffffffff);
rn.RN=key;
if(!pwdnc) pwdnc=sl(pwd);
for(int i=0;i<pwdnc;i++)
 pwd[i]^=rn.Rand(0,255);
}

#endif