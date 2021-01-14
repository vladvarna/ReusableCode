#ifndef V_MISCTOOLS
#define V_MISCTOOLS

#include <gdi.cpp>
#include <vtime.cpp>
#include <mmed.cpp>
#include <advdbg.cpp>

#ifndef V_SUPPORT_WIN95
#pragma comment(lib,"Msimg32.lib")
#endif

//calculates bit usage for [pmem,pmem+nri*Bpi]..................................................................
NAT BitUsage(BYTE*pmem,int Bpi,NAT nri,LPSTR sbits,int*ibits=NULL)
{
int*buse;
NAT el,pos,bpi;
bpi=Bpi<<3; //bpi=Bpi*8;
buse=(int*)calloc(sizeof(int),bpi);
for(el=0;el<nri;el++)
 {
 for(pos=0;pos<bpi;pos++)
  buse[pos]+=(int)GetBit(pmem,pos);
 pmem+=Bpi;
 }
if(sbits)
 {
 el=0;
 for(pos=bpi;pos>0;pos--)
  el+=sprintf(sbits+el,"%5.1f%c",(double)buse[pos-1]/nri*100,(pos-1)&7?' ':'\n');
 }
if(ibits) CopyMemory(ibits,buse,bpi);
free(buse);
return el;
}

#define CPU_TYPE(cpuvers) ((cpuvers>>12)&0x3)
#define CPU_FAM(cpuvers) ((cpuvers>>8)&0xf)
#define CPU_MODEL(cpuvers) ((cpuvers>>4)&0xf)
#define CPU_STEP(cpuvers) (cpuvers&0xf)

//returns CPU feature caps --------------------------------------------------
DWORD __cdecl CPUcaps(LPSTR cpuname,DWORD *cpuvers)
{
__asm
 {
 pushfd
 pop eax
 mov ebx,eax
 xor eax,00200000h
 push eax
 popfd
 pushfd
 pop eax
 cmp eax,ebx
 mov eax,0
 jz CPUID_NA //cpuid not supported
 cpuid
 mov edi,cpuname
 mov [edi],ebx
 add edi,4
 mov [edi],edx
 add edi,4
 mov [edi],ecx
 add edi,4
 mov [edi],0
 mov eax,1
 cpuid
 mov edi,cpuvers
 mov [edi],eax
 mov eax,edx
CPUID_NA:
 }
}

#define FPU_PREC(fpucdw) ((fpucdw>>8)&0x3)
#define FPU_ROUND(fpucdw) ((fpucdw>>10)&0x3)
//returns fpu control(LOW) and status(HIW) words in one dword ----------------------------------------------------
DWORD __cdecl FPUstat()
{
WORD fpuw;
__asm
 {
 fstsw fpuw
 mov ax,fpuw
 shl eax,16
 fstcw fpuw
 mov ax,fpuw
 }
}

#ifndef V_SUPPORT_WIN95
//interface for GradientFill ----------------------------------------------------------------
inline void GradRect(HDC hdc,int l,int u,int r,int d,COLORREF c1,COLORREF c2,int m=1)
{
TRIVERTEX verts[2] ;
GRADIENT_RECT inds;
verts[0].x=l;
verts[0].y=u;
verts[0].Red=(c1&0xff)<<8;
verts[0].Green=c1&0xff00;
verts[0].Blue=(c1&0xff0000)>>8;
verts[0].Alpha=0xff00;
verts[1].x=r;
verts[1].y=d; 
verts[1].Red=(c2&0xff)<<8;
verts[1].Green=c2&0xff00;
verts[1].Blue=(c2&0xff0000)>>8;
verts[1].Alpha=0xff00;
inds.UpperLeft=0;
inds.LowerRight=1;
GradientFill(hdc,verts,2,&inds,1,m?GRADIENT_FILL_RECT_V:GRADIENT_FILL_RECT_H);
}
#endif

//simulates mouse movement ---------------------------------------------------------------------------
void MoveMouseTo(int mx,int my,int vv=7,float vt=100)
{
int vh=vv;
Chronos ltm;
do{
 if(ltm.Sec(vt))
  {
  ltm.Abs();
  GetCursorPos((POINT*)&mouse.x);
  mouse.dx=mx-mouse.x;
  mouse.dy=my-mouse.y;
  while(!(mouse.dx>>vh)&&vh)
   {vh--;}
  while(!(mouse.dy>>vv)&&vv)
   {vv--;}
  mouse.x+=mouse.dx>>vh;
  mouse.y+=mouse.dy>>vv;
  SetCursorPos(mouse.x,mouse.y);
  }
}while(mouse.x!=mx||mouse.y!=my);
}

//tries to grab all the memory -------------------------------------------------------------
NAT VAllocTest(NAT blocsz=0x10000)
{
char lsbuf[256];
struct MemBList
 {
 void*pmem;
 MemBList*next;
 }head,*tail,*last;
Chronos ltm;
NAT bcnt=0,aloctm,freetm;
int go=1;
if(blocsz<1024) blocsz=1024; //no point in test with less than 1 KB
head.next=NULL;
tail=&head;
ltm.Abs();
while(go)
 {
 tail->pmem=VirtualAlloc(NULL,blocsz,MEM_COMMIT,PAGE_READWRITE);
 if(tail->pmem)
  {
  bcnt++;
  tail->next=(MemBList*)malloc(sizeof(MemBList));
  if(tail->next)
   {
   tail=tail->next;
   tail->next=NULL;
   }
  else
   go=0;
  }
 else
  go=0;
 }
aloctm=ltm.Sec(1000.0);
tail=&head;
go=bcnt;
ltm.Abs();
while(tail->pmem)
 {
 VirtualFree(tail->pmem,0,MEM_RELEASE);
 tail=tail->next;
 go--;
 }
freetm=ltm.Sec(1000.0);
tail=head.next;
while(tail)
 {
 last=tail;
 tail=tail->next;
 free(last);
 }
blocsz>>=10;
sprintf(lsbuf,"%u blocks of %u KB = %u MB allocated in %u miliseconds\nand freed in %u miliseconds (%i blocks not freed)",
        bcnt,blocsz,(bcnt*blocsz)>>10,aloctm,freetm,go);
MessageBox(NULL,lsbuf,"VirtualAlloc test results",MB_OK);
return (bcnt*blocsz)>>10;
}

//tries to grab all the memory -------------------------------------------------------------
NAT MAllocTest(NAT blocsz=4096)
{
char lsbuf[256];
struct MemBList
 {
 void*pmem;
 MemBList*next;
 }head,*tail,*last;
Chronos ltm;
NAT bcnt=0,aloctm,freetm;
int go=1;
#ifdef _DEBUG
if(MessageBox(NULL,"In DEBUG builds the malloc test can take very long!\nDo the test anyway?","Warning",MB_YESNO)==IDNO)
 return 0;
#else
if(blocsz<0x10000)
 if(MessageBox(NULL,"Doing the test with block size less than 64KB can take very long!\nKeep this size anyway?","Warning",MB_YESNO)==IDNO)
  blocsz=0x10000;
#endif
if(blocsz<32) blocsz=32;
head.next=NULL;
tail=&head;
ltm.Abs();
while(go)
 {
 tail->pmem=malloc(blocsz);
 if(tail->pmem)
  {
  bcnt++;
  tail->next=(MemBList*)malloc(sizeof(MemBList));
  if(tail->next)
   {
   tail=tail->next;
   tail->next=NULL;
   }
  else
   go=0;
  }
 else
  go=0;
 }
aloctm=ltm.Sec(1000.0);
free(head.pmem);
tail=head.next;
go=bcnt-1;
ltm.Abs();
while(tail->pmem)
 {
 free(tail->pmem);
 last=tail;
 tail=tail->next;
 free(last);
 go--;
 }
free(tail);
freetm=ltm.Sec(1000.0);
sprintf(lsbuf,"%u blocks of %u B = %u MB (+%u KB) allocated in %u miliseconds \nand freed in %u miliseconds (%i blocks not freed)\n%u seconds total time",
        bcnt,blocsz,(bcnt*blocsz)>>20,(sizeof(MemBList)*bcnt)>>10,aloctm,freetm,go,(aloctm+freetm)/1000);
MessageBox(NULL,lsbuf,"malloc test results",MB_OK);
return (bcnt*blocsz)>>20;
}

//changes byte ordering for a structure using a memory map ----------------------------------------------------
void __cdecl BOrder(LPSTR memmap,...)
{
va_list vparam;
va_start(vparam,memmap);
POINTER pmem;
pmem.v=va_arg(vparam,void*);
while(*memmap)
 {
 switch(toupper(*memmap))
  {
  case 'C': //char __int8
  case 'O': //octet
  case 'B': //byte
   pmem.b++;
   memmap++;
   break;
  case 'M': //wchar
  case 'S': //short __int32
  case 'W': //word
   __asm
    {
    mov edx,pmem.w //edx=pmem.w
    mov ax,[edx] //ax=*pmem.w
    xchg al,ah //swap bytes
    mov [edx],ax
    }
   pmem.b+=2;
   memmap++;
   break;
  case 'P': //pointer
  case 'T': //text with null terminator
  case '~': //text with next char as terminator
   error("Warning: byte reordering for pointers may not be done correct!");
  case 'I':
  case 'L': //long
  case 'U':
  case 'N': //NAT 
  case 'F': //float
   __asm
    {
    mov edx,pmem.u //edx=pmem.u
    mov eax,[edx] //eax=*pmem.u
    bswap eax //swap bytes
    mov [edx],eax
    }
   pmem.b+=4;
   memmap++;
   break;
  case 'D': //double
  case 'E': //__int64 ~ 2^60=1 ExaByte
  case 'Q': //quadword
   error("Warning: byte reordering for 8 bytes structs not done!");
   pmem.b+=8;
   memmap++;
   break;
  case '\'': //text as is
  case ',': //label
   memmap=nextch('\'',memmap,2,BUFFSZ);
   break;
  case '$': //move pointer back
  case '<': //move pointer back
   pmem.b-=StoI(memmap+1,10,&memmap);
   break;
  case '>': //move pointer forward
   pmem.b+=StoI(memmap+1,10,&memmap);
   break;
  case '^': //load next pointer
   pmem.v=va_arg(vparam,void*);
   memmap++;
   break;
  case '-': //delete last outputed character
  case '|': //return output
  default: //ignore any unrecognized chars
   memmap++;
  }
 }
va_end(vparam);
}

#endif