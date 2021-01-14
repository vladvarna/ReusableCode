//VERSION 1.0.a
#ifndef V_BASICTOOLS
#define V_BASICTOOLS

//inline assembly macros
#define DISCARD_ST0 __asm ffree st(0) __asm fincstp //pops the stack discarding value

#define PADR(p,bas,off) __asm mov eax,bas __asm add eax,off __asm mov p,eax
#define POFF(p,off) __asm mov eax,p __asm add eax,off __asm mov p,eax
#define PADD(p1,s2) __asm mov eax,p1 __asm add eax,p1 __asm mov p1,eax
#define PSUB(p1,s2) __asm mov eax,p1 __asm sub eax,p2 __asm mov p1,eax

#define ROLL(val,cnt) __asm mov ecx,cnt __asm rol val,cl
#define ROLR(val,cnt) __asm mov ecx,cnt __asm ror val,cl

#define ALIGN(number,mask) ((((number)-1)|(mask))+1) //Round up to mask+1
#define TRUNC(number,mask) ((number)&~(mask)) //Round down to mask+1
#define SETBIT(num,bpos) ((num)|(0x1<<(bpos))) //set bit in val
#define RSTBIT(num,bpos) ((num)&~(0x1<<(bpos))) //reset bit in val
#define GETBIT(num,bpos) (((num)>>(bpos))&0x1) //get bit from val
#define XORBIT(num,bpos) ((num)^(0x1<<(bpos))) //set bit in val
#define DW_NIB(dwhex,nibble) (((dwhex)>>((nibble)<<2))&0xf) //get nibble from dword (7...0)
#define FLAG_MASK(flags,mask,onoff) ((onoff)?(flags)|=(mask):(flags)&=~(mask)) //set or reset maks in flags based on onoff

#define TAG4(s4)   (*(DWORD*)s4) //"abcd"->d|c|b|a 
#define FOURCC(s4) (*(DWORD*)s4) //"abcd"->d|c|b|a //for compatibility 
#define TAG4S(dw) ((LPSTR)&dw) //d|c|b|a->"abcd"
#define TAG8(s8) (*(QWORD*)s8) //"abcdefgh"->h|gf|e|d|c|b|a
#define TAG8S(qw) ((LPSTR)&qw) //h|gf|e|d|c|b|a->"abcdefgh"
#define B4(b0,b1,b2,b3) (((b0)&0xff)|(((b1)&0xff)<<8)|(((b2)&0xff)<<16)|(((b3)&0xff)<<24)) //build DWORD from four BYTEs

#define ABS(x) ((x)<0?-(x):(x))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN3(a,b,c) MIN(MIN(a,b),c)
#define MAX3(a,b,c) MAX(MAX(a,b),c)
#define CLAMP(x,a,b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
#define SGN(val) ((signed)val>0?1:(val?-1:0)) //sgn(x)={ -1 if x<0, 0 if x==0, 1 if x>0
#define INT_SGN 0x80000000 //mask for integer sign bit

//---------------------------------------------------------------------
template <typename tip> void swap(tip&v1,tip&v2)
{
tip aux;
aux=v1;
v1=v2;
v2=aux;
}

//sign extends a value ---------------------------------------------------------------------
inline int __cdecl sgnext(NAT val,NAT sgnpos=32)
{
__asm
 {
 mov eax,val
 mov ecx,32
 sub ecx,sgnpos
 shl eax,cl
 sar eax,cl
 }
}

//return bit at [bitbase]+bitoff (bitoff=[-2^31,2^32-1])---------------------------------------------------------
inline int __cdecl GetBit(void*bitbase,int bitoff=0)
{
__asm
 {
 xor eax,eax
 mov ecx,bitoff
 mov edx,bitbase
 bt [edx],ecx
 adc eax,0
 }
}

//return bit and complement -----------------------------------------------------------
inline int __cdecl NegBit(void*bitbase,int bitoff=0)
{
__asm
 {
 xor eax,eax
 mov ecx,bitoff
 mov edx,bitbase
 btc [edx],ecx
 adc eax,0
 }
}

//bit=1 ---------------------------------------------------------------------------
inline void __cdecl SetBit(void*bitbase,int bitoff=0)
{
__asm
 {
 mov ecx,bitoff
 mov edx,bitbase
 bts [edx],ecx
 }
}

//bit=0 ---------------------------------------------------------------------------
inline void __cdecl RstBit(void*bitbase,int bitoff=0)
{
__asm
 {
 mov ecx,bitoff
 mov edx,bitbase
 btr [edx],ecx
 }
}

//returns the number of set bits in a mask ---------------------------------------------
inline NAT __cdecl setbits(DWORD mask)
{
__asm
 {
 xor eax,eax //eax=0
 mov edx,mask
 bsr ecx,edx //ecx=MSB set
 jz LsetbitsRET //if(!mask) return
 xor ebx,ebx
LIfSetNextBit:
 bt edx,ecx //CF=bit(mask,ecx)
 adc eax,ebx //eax+=CF
 dec ecx
 jge LIfSetNextBit
LsetbitsRET:
 }
}

//returns the number of reseted (0) bits in a mask ---------------------------------------------
inline NAT __cdecl zerobits(DWORD mask)
{
__asm
 {
 mov eax,32 //eax=32
 mov edx,mask
 bsr ecx,edx //ecx=MSB set
 jz LzerobitsRET //if(!mask) return
 xor ebx,ebx
LIfZeroNextBit:
 bt edx,ecx //CF=bit(mask,ecx)
 sbb eax,ebx //eax+=CF
 dec ecx
 jge LIfZeroNextBit
LzerobitsRET:
 }
}

//zeroes memory on a DW granularity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline void ZeroMem(void*pmem,NAT memszB=0)
{
__asm
 {
 mov edi,pmem
 mov ecx,memszB
 mov edx,ecx //save ecx
 shr ecx,2
 jz LSkip1
 xor eax,eax
 rep stosd
LSkip1:
 mov ecx,edx
 and ecx,0x3
 jz LSkip2
 rep stosb
LSkip2:
 }
}

//compares to blocks ------------------------------------------------------------------------------
inline BOOL __cdecl CmpMem(void*str1,void*str2,NAT bsz=0)
{
__asm
 {
 xor eax,eax
 mov ecx,bsz
 jecxz LRetTrue
 mov edi,str1
 mov esi,str2
 repz cmpsb
 jnz LRetFalse
LRetTrue:
 mov eax,1
LRetFalse:
 }
}

//compares to blocks and returns address of first difference ------------------------------------------------------------------------------
inline NAT __cdecl MemDif(void*str1,void*str2,NAT bsz=0)
{
__asm
 {
 mov ecx,bsz
 mov edi,str1
 mov esi,str2
 repz cmpsb
 mov eax,edi
 sub eax,str1
 }
}

//returns most significant set bit------------------------------------------------------------
inline NAT __cdecl MSBit(NAT num)
{
__asm
 {
 mov ecx,num
 mov eax,0xffffffff
 jecxz LMSBitRET
 bsr eax,ecx //eax=MSb
LMSBitRET:
 }
}

//returns least significant set bit------------------------------------------------------------
inline NAT __cdecl LSBit(NAT num)
{
__asm
 {
 mov ecx,num
 mov eax,0xffffffff
 jecxz LLSBitRET
 bsf eax,ecx //eax=LSb
LLSBitRET:
 }
}

//creates a mask with bcnt bits set starting at msbo --------------------------------------
inline DWORD __cdecl BitMask(NAT msbo=31,NAT bcnt=32)
{
__asm
 {
 xor eax,eax
 mov ecx,bcnt
 jecxz LReturn //if(bcnt==0) return 0;
 not eax //eax=0xffffffff
 sub ecx,32
 neg ecx //ecx=32-bcnt
 shl eax,cl //put ecx 0s in eax
 mov ecx,31
 sub ecx,msbo //ecx=31-msbo
 ror eax,cl //put mask in position
LReturn:
 }
}

//applies a mask to a mem block using bitwise OR ---------------------------------------
void __cdecl ORmaskU(void*pmem,DWORD ormask=0,int Bpi=1,NAT icnt=1)
{
__asm
 {
 mov edi,pmem
 mov edx,ormask
 mov ebx,Bpi
 mov ecx,icnt
LNextItem:
 mov eax,[edi]
 or eax,edx
 mov [edi],eax
 add edi,ebx
 loop LNextItem
 }
}

//applies a mask to a mem block using bitwise AND ---------------------------------------
void __cdecl ANDmaskU(void*pmem,DWORD andmask=0,int Bpi=1,NAT icnt=1)
{
__asm
 {
 mov edi,pmem
 mov edx,andmask
 mov ebx,Bpi
 mov ecx,icnt
LNextItem:

 mov eax,[edi]
 and eax,edx
 mov [edi],eax
 add edi,ebx
 loop LNextItem
 }
}

//copies memory on a DW granularity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline void CopyMem(void*pdst,void*psrc,NAT memszB=0)
{
__asm
 {
 mov edi,pdst
 mov esi,psrc
 mov ecx,memszB
 mov edx,ecx //save ecx
 shr ecx,2
 jz LSkip1
 rep movsd
LSkip1:
 mov ecx,edx
 and ecx,0x3
 jz LSkip2
 rep movsb
LSkip2:
 }
}

//copy to dest limit or to term ----------------------------------------------------------
inline NAT __cdecl  CopyMemUntil(void*dstr,void*sstr,NAT dsz=0,char term=0)
{
__asm
 {
 mov edi,dstr
 mov esi,sstr
 mov eax,edi
 mov dl,term
 mov ecx,dsz
LNextchar:
  cmp dl,[esi]
  je LBreakOut
  movs
  loop LNextchar
LBreakOut:
 sub eax,edi //dstr-edi
 neg eax //eax=edi-dstr
 } //return count of bytes copied
}

//copy to dest limit and count seps----------------------------------------------------------
inline NAT __cdecl CopyMemCnt(void*dstr,void*sstr,NAT dsz=0,char sep=0)
{
__asm
 {
 xor eax,eax
 mov ecx,dsz
 jecxz LBreakOut
 mov edi,dstr
 mov esi,sstr
 mov dl,sep
LNextchar:
  cmp dl,[esi]
  jne LNoInc
  inc eax
LNoInc:
  movs
  loop LNextchar
LBreakOut:
 }
}

//copies 4 bytes at once and alignes pointers ---------------------------------------------------------------------
inline int __cdecl CopyMemDW(DWORD*pdst,DWORD*psrc,NAT bytes=0)
{
__asm
 {
 mov edi,pdst
 mov esi,psrc
 mov ecx,bytes
 dec ecx
 or ecx,0x3 //round bytes up to DW multiple
 inc ecx
 shr ecx,2
 mov eax,ecx //return number of DWORDs copied
 jz LRet
 rep movsd
LRet:
 }
}

//copy number with sign extension ----------------------------------------------------------------------
void CopyNumber(void*pdst,void*psrc,NAT dszB,NAT sszB,int uns=0)
{
__asm
 {
 mov edi,pdst
 mov esi,psrc
 mov edx,dszB
 mov eax,sszB
 cmp edx,eax
 jbe Lnopad	//dszB<=sszB
 mov ecx,eax
 jecxz Lret
 rep movsb
 sub edx,eax
 mov ecx,edx //dszB-sszB
 cmp uns,0
 jne Lunsigned
 mov al,[esi-1]	//get last byte
 sar al,7  //al=signbit(al)?0xff:0
 jmp Lfill

Lunsigned:
 xor al,al   //fill with 0
Lfill:
 rep stosb //fill dszB-sszB with al
 jmp Lret
Lnopad:
 mov ecx,edx
 jecxz Lret
 rep movsb
Lret:
 }
}

//convert float to double or viceversa ----------------------------------------------------------------------
double CopyFloat(void*pdst,void*psrc,NAT dszB=8,NAT sszB=4)
{
__asm
 {
 mov eax,psrc
 mov edx,sszB
 cmp edx,8
 jb Lfloat
 fld qword ptr[eax]	//double
 jmp Lwrite
Lfloat: 
 cmp edx,4
 jb Linvalid
 fld dword ptr[eax]	//float
 jmp Lwrite
Linvalid:
 fldz
Lwrite:
 mov eax,pdst
 mov edx,dszB
 cmp edx,8
 jb Lwfloat
 fst qword ptr[eax] //double
 jmp Lret
Lwfloat: 
 cmp edx,4
 jb Lret
 fst dword ptr[eax] //float
Lret:
 }
}

//bit block transfer moving 1 quad at a time ----------------------------------------------
void __cdecl BltQ(DWORD*pdst,int dspan,int dstx,int dsty,DWORD*psrc,int sspan,int srcx,int srcy,int lng,int lat)
{
__asm
 {
 mov ecx,dspan
 mov eax,dsty
 imul ecx //eax=dspan*dsty
 mov edi,dstx
 shl edi,2 //edi=dstx*4
 add edi,pdst //edi+=pdst
 add edi,eax //edi=DWORD pointer to dst block's UL corner
 mov ebx,sspan
 mov eax,srcy
 imul ebx
 mov esi,srcx
 shl esi,2 //esi=srcx*4
 add esi,psrc //eax+=psrc
 add esi,eax //esi=DWORD pointer to src block's UL corner
 mov edx,ecx
 mov ecx,lng
 shl ecx,2
 sub edx,ecx //edx=dst extra B
 sub ebx,ecx //ebx=src extra B
 mov eax,lat
LBltQ_NextLine:
  mov ecx,lng
  rep movsd
  add edi,edx //add extra B to obtain span
  add esi,ebx //add extra B to obtain span
  dec eax
  jnz LBltQ_NextLine //loop if eax>0
 }
}

//Copy mem rects with diffrent pitch ---------------------------------------------
void __cdecl BltU(void*pdc,int dspan,void*psc,int sspan,int lng,int lat,int Bpi=1)
{
__asm
 {
 mov eax,lng
 imul Bpi //eax=lng*Bpi
 mov edi,pdc
 mov edx,dspan
 sub edx,eax //edx=dspan-lng*Bpi=extra dBpl
 mov esi,psc
 mov ebx,sspan
 sub ebx,eax //ebx=sspan-lng*Bpi=extra sBpl
 push ebp
 mov ebp,lat //use ebp as outer counter
LNextLine:
  mov ecx,eax
  rep movsb
  add edi,edx       //next d line
  add esi,ebx       //next s line
  dec ebp //lat--
  jnz LNextLine //loop if lat>0
 pop ebp
 }
}

//Stretch src rect buffer to fit in dst rect buffer ---------------------------------------------
void __cdecl ZoomU(void*pdc,int dspan,int dlng,int dlat,void*psc,int sspan,int slng,int slat,int Bpi=4)
{
__asm
 {
 mov edi,pdc
 mov esi,psc          //load pointers
 mov eax,dlng
 imul Bpi
 sub dspan,eax        //dspan-=dlng*Bpi extra B
 mov fpuioi,0         //use fpuioi as vertical decision
 mov eax,dlat
 mov pdc,eax          //use pdc as outer loop counter
LNextDLine:
  mov psc,esi         //save pointer to begining of line
  xor edx,edx         //use EDX as hrap
  mov ebx,dlng        //use EBX as counter for inner loop
LNextDPixel:
   mov ecx,Bpi
   rep movsb
   mov eax,edx
   add eax,slng
   cdq
   idiv dlng
   dec eax            //esi already incresead by Bpi
   imul eax,Bpi       //eax=(edx+slng)/dlng*Bpi
   add esi,eax        //next src X
   dec ebx
   jnz LNextDPixel    //loop if ebx>0
  add edi,dspan       //edi+=extra B
  mov esi,psc         //restore src pointer to begining of line
  mov eax,fpuioi
  add eax,slat
  cdq
  idiv dlat
  mov fpuioi,edx        //fpuioi%=dlng
  imul sspan          //eax=(fpuioi+slat)/dlat*sspan
  add esi,eax         //next src Y
  dec pdc
  jnz LNextDLine      //loop if pdc>0
 }
}

//left shifts a memory block (<--)-----------------------------------------------------------
void* __cdecl ShiftMemL(void*psrc,int delta=1,NAT count=1)
{
__asm
 {
 mov esi,psrc
 mov edi,esi
 sub edi,delta
 mov eax,edi
 mov ecx,count
 rep movsb
 }
}

//right shifts a memory block (-->)-----------------------------------------------------------
void* __cdecl ShiftMemR(void*psrc,int delta=1,NAT count=1)
{
__asm
 {
 mov ecx,count
 mov esi,psrc
 add esi,ecx
 dec esi //esi=psrc+count-1
 mov edi,esi
 add edi,delta
 pushfd
 std
 rep movsb
 popfd
 mov eax,edi
 inc eax
 }
}

//copies Bpi bytes from psrc to pdest itemcnt times --------------------------------------
void __cdecl FillMem(void*pdst,void*psrc,NAT Bpi=1,NAT itemcnt=1)
{
__asm
 {
 mov edi,pdst
 mov edx,psrc
 mov ebx,Bpi
 mov eax,itemcnt
LNextItem:
 mov esi,edx
 mov ecx,ebx
 rep movsb
 dec eax
 jnz LNextItem
 }
}

//copies b in pb count times -----------------------------------------------------------
inline void __cdecl FillB(BYTE*pb,BYTE b=0,NAT count=1)
{
__asm
 {
 mov edi,pb
 mov al,b
 mov ecx,count
 rep stosb
 }
}

//copies dw in pdw count times ----------------------------------------------------------
inline void __cdecl FillDW(DWORD*pdw,DWORD dw=0,NAT count=1)
{
__asm
 {
 mov edi,pdw
 mov eax,dw
 mov ecx,count
 rep stosd
 }
}

//copies f in pf count times ------------------------------------------------------------
inline void __cdecl FillF(float *pf,float f=0.0f,NAT count=1)
{
__asm
 {
 mov edi,pf
 mov eax,f
 mov ecx,count
 rep stosd
 }
}

//--------------------------------------------------------------------------------------------------------------
void __cdecl FillMemU(void*pdst,void*psrc,NAT Bpi=1,NAT count=1,int Bpl=0)
{
__asm
 {
 mov eax,pdst
 mov edx,count
LNextItem:
 mov edi,eax
 mov esi,psrc
 mov ecx,Bpi
 rep movsb
 add eax,Bpl
 dec edx
 jnz LNextItem
 }
}

//numarul de aparitii al sep in pmem --------------------------------------------------------
NAT __cdecl MemCnt(char sep,void*pmem,NAT msz=0)
{
__asm
 {
 xor eax,eax
 mov ecx,msz
 jecxz LBreakOut
 mov bl,sep
 mov edx,pmem
LNextChar:
  cmp bl,[edx]
  jne LNoInc
  inc eax
LNoInc:
  inc edx
  loop LNextChar
LBreakOut:
 }
}

//searches for b in memory [pmem,pmem+szb] and returns pointer ---------------------------
inline void* __cdecl FindMemB(BYTE b,void*pmem,NAT szb=0)
{
__asm
 {
 mov eax,pmem
 mov dl,b
 mov ecx,szb
LNextByte:
  cmp [eax],dl
  je Lreturn
  inc eax
  loop LNextByte
Lreturn:
 }
}

//searches for w in memory [pmem,pmem+szb] and returns pointer ---------------------------
inline void* __cdecl FindMemW(WORD w,void*pmem,NAT szb=0)
{
__asm
 {
 mov eax,pmem
 mov dx,w
 mov ecx,szb
LNextByte:
  cmp [eax],dx
  je Lreturn
  inc eax
  loop LNextByte
Lreturn:
 }
}

//searches for dw in memory [pmem,pmem+szb] and returns pointer ---------------------------
inline void* __cdecl FindMemDW(DWORD dw,void*pmem,NAT szb=0)
{
__asm
 {
 mov eax,pmem
 mov edx,dw
 mov ecx,szb
LNextByte:
  cmp [eax],edx
  je Lreturn
  inc eax
  loop LNextByte
Lreturn:
 }
}

//copies a buffer to delta between B with a specified granularity ---------------------------------------------------------
inline void DataCopyDeltaU(void*pdst,void*psrc,NAT memszB,NAT gran=1)
{
__asm
 {
 mov ecx,gran
 jecxz LJustCopy
 mov ebx,memszB
 cmp ebx,ecx
 jbe LReturn //return if(memszB<=gran)
 add ebx,pdst //ebx=max pointer value
LLoop1:
  mov edi,pdst
  mov esi,psrc
  mov al,[esi] //al=current B
  mov [edi],al
  jmp LStartLoop2
LJustCopy: //this is a separate block put here for optimization
 mov edi,pdst
 mov esi,psrc
 mov ecx,memszB
 rep movsb
 jmp LReturn
Lloop2:
   mov dh,[esi]
   mov dl,dh
   sub dh,al
   mov [edi],dh
   mov eax,edx //al=dl
LStartLoop2:
   add esi,ecx
   add edi,ecx
   cmp edi,ebx
   jb Lloop2 //loop if(edi<pdst+memszB)
  inc psrc
  inc pdst
  dec gran
  jnz Lloop1 //loop if((gran--)>0)
LReturn:
 }
}

//transforms a buffer to delta between B with a specified granularity ---------------------------------------------------------
inline void DataToDeltaU(void*pmem,NAT memszB,NAT gran=1)
{
__asm
 {
 mov ecx,gran
 jecxz LReturn
 mov ebx,memszB
 cmp ebx,ecx
 jbe LReturn //return if(memszB<=gran)
 mov edi,pmem
 add ebx,edi //ebx=max pointer value
LLoop1:
  mov esi,edi
  mov al,[esi] //al=current B
  jmp LStartLoop2
Lloop2:
   mov dl,[esi]
   sub [esi],al
   mov eax,edx //al=dl
LStartLoop2:
   add esi,ecx
   cmp esi,ebx
   jb Lloop2 //loop if(esi<pmem+memszB)
  inc edi
  dec gran
  jnz Lloop1 //loop if((gran--)>0)
LReturn:
 }
}

//transforms a buffer from delta between B with a specified granularity  ---------------------------------------------------------
inline void DataFromDeltaU(void*pmem,NAT memszB,NAT gran=1)
{
__asm
 {
 mov ecx,gran
 jecxz LReturn
 mov edx,memszB
 cmp edx,ecx
 jbe LReturn //return if(memszB<=gran)
 mov edi,pmem
 add edx,edi //edx=max pointer value
LLoop1:
  mov esi,edi
  mov al,[esi] //al=current B
  jmp LStartLoop2
Lloop2:
   add al,[esi]
   mov [esi],al
LStartLoop2:
   add esi,ecx
   cmp esi,edx
   jb Lloop2 //loop if(esi<pmem+memszB)
  inc edi
  dec gran
  jnz Lloop1 //loop if((gran--)>0)
LReturn:
 }
}

//returns str4's 1 based index in list(dwords) (0-if not found) ------------------------------------
int __cdecl s4inlist(void*str4,void*list,NAT nrit)
{
__asm
 {
 mov eax,str4
 mov edx,[eax]
 mov eax,list
 mov ecx,nrit
 jecxz Lreturn0

 sub eax,4
 mov ebx,eax
Lloop1:
  add eax,4
  cmp edx,[eax]
  loopne Lloop1
  je Lreturn //found
Lreturn0:
 mov ebx,eax
Lreturn:
 sub eax,ebx
 shr eax,2
 }
}

//returns the index where entry is in a LUT or -1 if it's not--------------------------------------------------
int __cdecl InLUT(void*lut,NAT nrentries,void*entry,NAT entrysz=1,NAT extra=0)
{
__asm
 {
 xor eax,eax
 mov ebx,lut //ebx=LUT
 mov edx,entrysz
Lloop1:
  mov edi,ebx
  mov esi,entry
  mov ecx,edx
  repe cmpsb //lut[eax]==*entry ?
  je LFound //Yup
  add ebx,edx
  add ebx,extra //LUT+=entrysz+extra
  inc eax
  cmp eax,nrentries
  jb Lloop1  //do while(eax<nrentries)
 mov eax,-1
LFound:
 }
}

//changes byte ordering from intel(little endian) to motorola(big endian) ---------------------
inline DWORD __cdecl BswapDW(DWORD dw)
{
__asm
 {
 mov eax,dw
 bswap eax
 }
}

//changes byte ordering from intel(little endian) to motorola(big endian) ---------------------
inline void __cdecl BswapDW(DWORD*pdw,NAT n)
{
for(int i=0;i<n;i++)
 pdw[i]=BswapDW(pdw[i]);
}


//changes byte ordering from intel(little endian) to motorola(big endian) ---------------------
inline WORD __cdecl BswapW(WORD w)
{

__asm
 {
 mov ax,w
 xchg al,ah
 }
}

//aligns a pointer to memory granularity ------------------------------------------------
void* __cdecl AlignPointer(void*pmem,NAT gran=0x10000)
{
__asm
 {
 mov eax,pmem
 mov ecx,gran
 xor edx,edx
 div ecx
 mul ecx
 }
}

//taie elementele val din vector ----------------------------------------------------------------------------
template <typename tip>NAT TrimVal(tip val,tip*vect,NAT osz)
{
tip*tvect=vect;
NAT nc=0;
while(osz)
 {
 *vect=*tvect;
 if((*tvect!=val))
   {
   vect++;
   nc++;
   }
 tvect++;
 osz--;
 }
return nc;
}

//returns x=[0,max) and a proper overflow --------------------------------------------------------------------------------------------
inline int AddIntCarryOver(int&x,int dx=1,int c=0,int max=0x7fffffff)
{
x+=dx+c;
c=0;
while(x>=max)
 {
 c++;
 x-=max;
 }
while(x<0)
 {
 c--;
 x+=max;
 }
return c; //overflow 
}

#endif