#pragma once

#include <ext/sha-1.cpp>
#include <ext/md5.c>
#undef F
#undef G
#undef H
#undef I
#include <ext/zip.h>
#include <ext/unzip.h>
#include <ext/zip.cpp>
#include <ext/unzip.cpp>
#include <vtime.cpp>
#include <shell.cpp>
#include <io.cpp>
#include <wincrypt.h>
//#pragma comment(lib,"advapi32.lib")

#define HASH_SHA1             1
#define HASH_MD5              5
#define HASH_FNV1            11
#define HASH_FNV1a           12
#define HASH_WIN          10000

//adds all dwords returning sum -----------------------------------------------------------------------
DWORD __cdecl CheckSumDW(DWORD*pdw,NAT dwcnt=0)
{
__asm
 {
 xor eax,eax
 mov ecx,dwcnt
 mov edx,pdw
LNextDword:
  add eax,[edx]
  loop LNextDword
 }
}

//see http://www.lammertbies.nl/comm/info/crc-calculation.html
#define GENERATOR_CRC_32        0x04C11DB7 //0xEDB88320
#define GENERATOR_CRC_16        0xA001
#define GENERATOR_CRC_CCITT     0x1021
//#define GENERATOR_CRC_DNP       0xA6BC
//#define GENERATOR_CRC_SICK      0x8005

//generic CRC function -----------------------------------------------------------
unsigned V_CRC(unsigned char*message,unsigned szB=0,int nrbits=32,unsigned rest=0,unsigned generator=GENERATOR_CRC_32)
{
unsigned topbit=1<<(nrbits-1),shift=(nrbits-8);
for(unsigned byte=0;byte<szB;byte++)
 {
 rest^=(message[byte]<<shift);
 for(unsigned char bit=8;bit>0;--bit)
  {
  if(rest&topbit)
   rest=(rest<<1)^generator;
  else
   rest=(rest<<1);
  }
 }
return rest&((unsigned)-1>>((sizeof(unsigned)<<3)-nrbits));
}

#define VCRC32(msg,msgsz) V_CRC((unsigned char*)msg,msgsz,32,0xffffffff,GENERATOR_CRC_32)^0xffffffff

#define FNV32_prime  16777619 //0x1000193
#define FNV32_offset 2166136261
#define FNV64_prime  1099511628211ui64 //0x100000001B3
#define FNV64_offset 14695981039346656037ui64

//Fowler / Noll / Vo (FNV) Hash -----------------------------------------------------------------------
DWORD __cdecl FNV1_32b(char*msg,NAT msgnc=0)
{
if(!msgnc) msgnc=sl(msg);
__asm
 {
 mov eax,FNV32_offset
 mov ecx,msgnc
 mov esi,msg
 jecxz Lret
LNextByte:
  mov edx,eax
  shl edx,1
  add eax,edx
  shl edx,3 //4
  add eax,edx
  shl edx,3 //7
  add eax,edx
  shl edx,1 //8
  add eax,edx
  shl edx,16 //24
  add eax,edx
  xor al,byte ptr [esi]
  dec ecx
  jnz LNextByte
Lret:
 }
}

//Fowler / Noll / Vo (FNV) Hash -----------------------------------------------------------------------
DWORD __cdecl FNV1a_32b(char*msg,NAT msgnc=0)
{
if(!msgnc) msgnc=sl(msg);
__asm
 {
 mov eax,FNV32_offset
 mov ecx,msgnc
 mov esi,msg
 jecxz Lret
LNextByte:
  xor al,byte ptr [esi]
  mov edx,eax
  shl edx,1
  add eax,edx
  shl edx,3 //4
  add eax,edx
  shl edx,3 //7
  add eax,edx
  shl edx,1 //8
  add eax,edx
  shl edx,16 //24
  add eax,edx
  dec ecx
  jnz LNextByte
Lret:
 }
}

//Fowler / Noll / Vo (FNV) Hash -----------------------------------------------------------------------
QWORD __cdecl FNV1_64b(char*msg,NAT msgnc=0)
{
if(!msgnc) msgnc=sl(msg);
QUADW hashval;
hashval.u64=FNV64_offset;
while(msgnc)
 {
 hashval.u64*=FNV64_prime;
 *hashval.b^=*msg;
 msg++;
 msgnc--;
 }
return hashval.u64;
}

//Fowler / Noll / Vo (FNV) Hash -----------------------------------------------------------------------
QWORD __cdecl FNV1a_64b(char*msg,NAT msgnc=0)
{
if(!msgnc) msgnc=sl(msg);
QUADW hashval;
hashval.u64=FNV64_offset;
while(msgnc)
 {
 *hashval.b^=*msg;
 hashval.u64*=FNV64_prime;
 msg++;
 msgnc--;
 }
return hashval.u64;
}

//Hash data (returns size of hash in Bytes)------------------------------------------------------------------------------------------
NAT HashX(char*message,NAT messagenc,BYTE*msgdigest,int method=HASH_WIN,int keysz=0)
{
if(!messagenc) messagenc=strlen(message);
if(method==HASH_SHA1) //160b=20B=5DW
 {
 SHA1Context shac;
 SHA1Reset(&shac);
 SHA1Input(&shac,(unsigned char*)message,messagenc);
 erret=SHA1Result(&shac,msgdigest);
 ifn(erret)
  return 20;
 }
else if(method==HASH_MD5) //128b=16B=4DW
 {
 md5_state_t md5state;
 md5_init(&md5state);
 md5_append(&md5state,(md5_byte_t*)message,messagenc);
 md5_finish(&md5state,msgdigest);
 return 16;
 }
else if(method==HASH_FNV1a) //32b,64b,128b,256b
 {
 if(keysz==4)
  {
  *(DWORD*)msgdigest=FNV1a_32b(message,messagenc);
  return 4;
  }
 else if(keysz==8)
  {
  *(QWORD*)msgdigest=FNV1a_64b(message,messagenc);
  return 8;
  }
 }
else if(method==HASH_FNV1) //32b,64b,128b,256b
 {
 if(keysz==4)
  {
  *(DWORD*)msgdigest=FNV1_32b(message,messagenc);
  return 4;
  }
 else if(keysz==8)
  {
  *(QWORD*)msgdigest=FNV1_64b(message,messagenc);
  return 8;
  }
 }
else if(method==HASH_WIN) //2048b=256B=64DW
 {
 keysz=CLAMP(keysz,1,256); //repeats after 256B (smaller than 256 are just truncated to that size)
 erret=HashData((BYTE*)message,messagenc,(BYTE*)msgdigest,keysz);
 if(erret==S_OK)
  return keysz;
 }
return 0; //failed
}

//uses CryptoAPI ------------------------------------------------------------------------------------
NAT HashW(char*message,NAT messagenc,BYTE*msgdigest,ALG_ID method=CALG_SHA1)
{
HCRYPTPROV hProv=NULL;
HCRYPTHASH hHash=NULL;
NAT mdsz=0;
if(!messagenc) messagenc=strlen(message);
ifn(CryptAcquireContext(&hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT|CRYPT_SILENT))
 return 0;
ifn(CryptCreateHash(hProv,method,0,0,&hHash))
 goto bail;
ifn(CryptHashData(hHash,(BYTE*)message,messagenc,0))
 goto bail;
REGdump=sizeof(mdsz);
CryptGetHashParam(hHash,HP_HASHSIZE,(BYTE*)&mdsz,&REGdump,0);
CryptGetHashParam(hHash,HP_HASHVAL,(BYTE*)msgdigest,(DWORD*)&mdsz,0);
bail:
if(hHash)
 CryptDestroyHash(hHash);
if(hProv)
 CryptReleaseContext(hProv,0);
return mdsz;
}

//---------------------------------------------------------------------------------------------------
BOOL Unzip(LPSTR srczip,LPSTR dstpath=NULL)
{
char lBufDir[PATHSZ];
HZIP hz = OpenZip(srczip,0);
ZIPENTRY ze;
GetZipItem(hz,-1,&ze);
int numitems=ze.index;
if(!dstpath) dstpath=ios.CurDir;
for(int i=0;i<numitems;i++)
 {
 GetZipItem(hz,i,&ze);
 mergepath(lBufDir,dstpath,ze.name);
 UnzipItem(hz,i,lBufDir);
 SetFileAttributes(lBufDir,GetFileAttributes(lBufDir)&(~FILE_ATTRIBUTE_ARCHIVE));
 }
CloseZip(hz);
return 1;
}

//prints a buffer as text ------------------------------------------------------------------------
NAT BufferToText(char*text,BYTE*buffer,NAT buffersz=0)
{
NAT l=0;
*text=0;
for(int i=0;i<buffersz;i++)
 l+=sprintf(text+l,"%02X",(BYTE)buffer[i]);
return l;
}