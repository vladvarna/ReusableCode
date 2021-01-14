//VERSION 1.0.a
#ifndef V_STRING
#define V_STRING

#include <bas.cpp>

/* All functions asume the strings are null terminated even if size is given;
also string size is the number of actual characters excluding the terminator.
All functions who return strings also append null terminator
   Most functions who receive a 0 in the size argument will ignore size and stop
only at the terminator (or after 0xffffffff cycles !) */

#define R_NULL   0x80000000 //null result
#define C_NULL   '\255' //null character (never used explicitly as terminator or separator)
#define WC_NULL L'\255' //null character (never used explicitly as terminator or separator)

#define SAFE_STR(s) (s?s:"")

//bitmap cu codurile ASCII ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct ASCIISET
{
 DWORD set[8]; //32B=256b

 ASCIISET() { Zero(); }
 void Zero() { ZeroMemory(this,sizeof(ASCIISET)); } //reset all
 //set al in [a,b] .....................................................................
 void __cdecl Range(char a=32,char b=127)
  {
  __asm
   {
   movzx ecx,a
   movzx eax,b
   mov edx,this
   Lloop1:
    bts [edx],ecx
	inc ecx
	cmp ecx,eax
	jbe	Lloop1   //jmp if ecx<=b
   }
  }
 //return bit c (0,255).....................................................................
 BOOL __cdecl operator[](char c)
  {
  __asm
   {
   mov edx,this
   movzx ecx,c
   xor eax,eax
   bt [edx],ecx
   adc eax,0
   }
  }
 //count set bits.....................................................................
 NAT __cdecl NC()
  {
  __asm
   {
   xor eax,eax //eax=0
   mov edx,this
   mov ecx,256
  Lloop1:
    dec ecx
    bt [edx],ecx //CF=bit(this,ecx)
    adc eax,0 //eax+=CF
    jecxz LRET
    jmp Lloop1
  LRET:
   }
  }
 //enum set ASCII codes.....................................................................
 void DBG()	//show printable range
  {
  char lsbuf[256];
  NAT l;
  l=sprintf(lsbuf,"[%u] ",NC());
  for(int i=32;i<=255;i++)
   if(this->operator[](i))
    lsbuf[l++]=i;
  lsbuf[l]=0;  
  MessageBox(hmwnd,lsbuf,"ASCII set",MB_OK);
  }
};

//converts unicode strings to ascii strings (src and dest can be the same) ---------------
inline NAT __cdecl W_A(char*strascii,WCHAR*strucode)
{
__asm
 {
 mov esi,strucode
 mov edi,strascii
 mov edx,edi
 xor ecx,ecx //cx=null
LW_Aloop:
  cmp [esi],cx
  lodsw
  stosb
  jne LW_Aloop
 mov eax,edi
 sub eax,edx
 dec eax //eax=strlen(strascii)
 }
}

//converts ascii strings to unicode strings (src and dest shouldn't overllapp)----------------------------------------------
inline NAT __cdecl A_W(WCHAR*strucode,char*strascii)
{
__asm
 {
 mov esi,strascii
 mov edi,strucode
 mov edx,esi
 xor eax,eax //ah=null
LW_Aloop:
  cmp [esi],ah
  lodsb
  stosw
  jne LW_Aloop
 mov eax,esi
 sub eax,edx
 dec eax //eax=wcslen(strucode)
 }
}

//merge strings ------------------------------------------------------------------
NAT __cdecl smerge(char*dstr,NAT count,...)
{
__asm
 {
 mov edi,dstr
 mov ecx,count
 mov edx,ebp
 add edx,16
 mov eax,edi
LNextStr:
  mov esi,[edx]
  test esi,esi
  je LNextArg //skip NULL arguments
LNextChar:
   cmp [esi],0
   movsb
   jne LNextChar //loop if [esi]!=term
  dec edi //delete last term
LNextArg:
  add edx,4 //next argument
  loop LNextStr //loop if more arguments
 mov [edi],0  //put terminator
 sub eax,edi
 neg eax //return strlen(dstr)
 }
}

//string length (to terminator) ------------------------------------------------------------
inline NAT __cdecl sl(char*lstr,char term=0)
{
__asm
 {
 mov eax,lstr
 mov ecx,eax
 jecxz LBreakOut //if(!lstr) return 0;
 mov dl,term
LNextchar:
  cmp dl,[eax]
  je LBreakOut
  inc eax
  jmp LNextchar
LBreakOut:
 sub eax,ecx
 }
}

//string length to any of the terminators ----------------------------------------------------------
inline NAT __cdecl sl_any(char*dstr,char*terms,NAT tsz)
{
__asm
 {
 mov eax,dstr
LNextChar:
  mov edx,terms
  mov ecx,tsz
  mov bl,[eax]
LNextTerm:
   cmp bl,[edx]
   je LFoundTerm
   inc edx //next term
   dec ecx
   jnz LNextTerm
  inc eax
  jmp LNextChar
LFoundTerm:
 sub eax,dstr  //return strlen(dstr)
 }
}

//end of string (pointer to NULL terminator) ------------------------------------------------------------
inline char* __cdecl eos(char*lstr)
{
__asm
 {
 mov eax,lstr
 test [eax],0xff
 jz LBreakOut
LNextchar:
  inc eax
  test [eax],0xff
  jnz LNextchar
LBreakOut:
 }
}

//copy string to terminator or to dest limit ----------------------------------------------------------
inline NAT __cdecl sc(char*dstr,char*sstr,NAT dsz=0,char sterm=0,char dterm=0)
{
__asm
 {
 mov edi,dstr
 test edi,-1 //if null
 jz LRet
 mov eax,edi
 mov esi,sstr
 mov dh,dterm
 mov dl,sterm
 mov ecx,dsz
 test esi,-1 //if null
 jz LBreakOut
//!!! jecxz LBreakOut
LNextchar:
  cmp dl,[esi]
  je LBreakOut
  movsb
  loop LNextchar
LBreakOut:
 mov [edi],dh  //put terminator
 sub eax,edi
 neg eax //return strlen(dstr)
LRet:
 }
}

//copy UNICODE string to terminator or to dest limit ----------------------------------------------------------
inline NAT __cdecl scw(WCHAR*dstr,WCHAR*sstr,NAT dcnt=0,WCHAR sterm=0,WCHAR dterm=0)
{
__asm
 {
 mov edi,dstr
 mov esi,sstr
 mov eax,edi
 mov dx,sterm
 mov ecx,dcnt
LNextchar:
  cmp dx,[esi]
  je LBreakOut
  movsw
  loop LNextchar
LBreakOut:
 mov dx,dterm
 mov [edi],dx  //put terminator
 sub eax,edi
 neg eax 
 shr eax,1 //return strlen(dstr)
 }
}

//copy string to terminator and fill to dest limit ----------------------------------------------------------
NAT __cdecl scf(char *dstr,char *sstr,NAT dsz=0,char sterm=0,char dterm=0,char filc=0)
{
__asm
 {
 mov edi,dstr
 mov esi,sstr
 mov ah,sterm
 mov al,filc
 mov ecx,dsz
LNextchar:
  cmp ah,[esi]
  je LBreakOut
  movsb
  loop LNextchar
LBreakOut:
 rep stosb
 mov ah,dterm
 mov [edi],ah  //put dest terminator
 sub edi,dstr
 mov eax,edi  //return strlen(dstr)
 }
}

//copy string to terminator and fill to dest limit don't add terminator ----------------------------------------------------------
NAT __cdecl scfn(char *dstr,char *sstr,NAT dsz=0,char sterm=0,char filc=0)
{
__asm
 {
 mov edi,dstr
 mov esi,sstr
 mov ah,sterm
 mov al,filc
 mov ecx,dsz
LNextchar:
  cmp ah,[esi]
  je LBreakOut
  movsb
  loop LNextchar
LBreakOut:
 rep stosb
 sub edi,dstr
 mov eax,edi  //return strlen(dstr)
 }
}

//copy string to terminator or to dest limit and count----------------------------------------------------------
NAT __cdecl scc(char *dstr,char *sstr,NAT dsz=0,char sterm=0,char dterm=0,char sep=0)
{
__asm
 {
 mov edi,dstr
 mov esi,sstr
 mov dl,sterm
 mov dh,sep
 xor eax,eax
 mov ecx,dsz
LNextchar:
  cmp dl,[esi]
  je LBreakOut
  cmp dh,[esi]
  jne LNoInc
  inc eax
LNoInc:
  movsb
  loop LNextchar
LBreakOut:
 mov dh,dterm
 mov [edi],dh  //put dest terminator
 }
}

//copy string to any of the terminators ----------------------------------------------------------
inline NAT __cdecl sc_any(char*dstr,char*sstr,char*terms,NAT nrt,NAT dsz=0,char dterm=0)
{
__asm
 {
 mov edi,dstr
 mov esi,sstr
 mov ecx,dsz
LNextChar:
  mov edx,terms
  mov ebx,nrt
  mov al,[esi]
LNextTerm:
   cmp al,[edx]
   je LFoundTerm
   inc edx //next term
   dec ebx
   jnz LNextTerm
  movsb
  dec ecx
  jnz LNextChar
LFoundTerm:
 mov dl,dterm
 mov [edi],dl  //put dest terminator
 mov eax,edi
 sub eax,dstr  //return strlen(dstr)
 }
}

//copy expresion enclosed with parens (including embeded parens) ----------------------------------------------------------
inline NAT __cdecl sc_paren(char*dstr,char*sstr,char paren1='(',char paren2=')')
{
__asm
 {
 mov edi,dstr
 mov esi,sstr
 xor ecx,ecx
 mov dl,paren1
 mov dh,paren2
LNextchar:
  mov al,[esi]
  cmp al,dl //'('
  jne LNoInc
  inc ecx
  jmp LIsPar
LNoInc:
  cmp al,dh //')'
  jne LNoDec
  dec ecx
  jmp LIsPar
LNoDec:
  cmp al,0
  je LError //nrof'(' < nrof')'
LIsPar:
  jecxz LBreakOut
  movsb
  jmp LNextchar
LBreakOut:
 movsb
LError: 
 mov [edi],'\0'  //put terminator
 mov eax,edi
 sub eax,dstr  //return strlen(dstr)
 }
}

//(str1==str2) -----------------------------------------------------------------------
inline BOOL __cdecl scmp(LPSTR str1,LPSTR str2,char term=0)
{
__asm
 {
 mov esi,str1
 test esi,-1
 jz LNotEqual
 mov edi,str2
 test edi,-1
 jz LNotEqual
 mov dl,term
 mov eax,1
LNextChar:
 cmp dl,[esi]
 je LBreakOut
 cmpsb
 je LNextChar
 jmp LNotEqual
LBreakOut:
 cmp dl,[edi]
 je LReturn
LNotEqual:
 xor eax,eax
LReturn:
 }
}

//checks if str1 is a root for str2 (ignores case)-----------------------------------------------------------------------
//return <0 : -len(str1) if str1 is found at the beginning of str2
//       =0 : str1 is not found at the beginning of str2
//       >0 : nr. of characters str1 and str2 have in common at the beginning
inline int __cdecl sroot(LPSTR str1,LPSTR str2)
{
__asm
 {
 mov esi,str1
 mov edi,str2
 xor edx,edx  //dh=term
 mov eax,esi
LNextChar:
  cmp dh,[esi]
  je LIsRoot
  cmpsb
  je LNextChar
 mov cl,[esi-1] //cl=*str1
 mov ch,cl //ch=*str1
 sub cl,[edi-1] //cl=*str1-*str2
 cmp cl,32
 je LBigger //*str1>*str2 by 32
 cmp cl,-32
 je LSmaller //*str1<*str2 by 32
LAreDiffrent:
 dec esi
 sub eax,esi
 neg eax
 jmp LReturn //|->
LBigger:
 cmp ch,'a'
 jb LAreDiffrent
 cmp ch,'z'
 ja LAreDiffrent
 jmp LNextChar //if('a'<=*str1<='z') => they are the same letter but different case
LSmaller:
 cmp ch,'A'
 jb LAreDiffrent
 cmp ch,'Z'
 ja LAreDiffrent
 jmp LNextChar //if('A'<=*str1<='Z') => they are the same letter but different case
LIsRoot:
 sub eax,esi
LReturn: //<-|
 }
}

//inlocuieste b cu a in str pana la term sau slen ---------------------------------------------
void __cdecl swapch(char oldc,char newc,LPSTR lstr,NAT slen=0,char term=0)
{
__asm
 {
 mov edx,lstr
 cmp edx,0
 jz LBreakOut
 mov bl,oldc
 mov bh,newc
 mov al,term
 mov ecx,slen
LNextChar:
  cmp al,[edx]
  je LBreakOut
  cmp bl,[edx]
  jne LNoSwap
  mov [edx],bh
LNoSwap:
  inc edx
  dec ecx
  jnz LNextchar
LBreakOut:
 }
}

//inlocuieste characterele intre a si z cu b ---------------------------------------------
void __cdecl FilterStr(char a,char z,char b,LPSTR lstr,NAT slen=0,char term=0)
{
__asm
 {
 mov edx,lstr
 mov bl,term
 mov bh,b
 mov al,a
 mov ah,z
 mov ecx,slen
LNextChar:
  cmp bl,[edx]
  je LBreakOut
  cmp [edx],al
  jb LNoSwap
  cmp [edx],ah
  ja LNoSwap
  mov [edx],bh
LNoSwap:
  inc edx
  dec ecx
  jnz LNextchar
LBreakOut:
 }
}

//taie characterele a din string ----------------------------------------------------------------------------
NAT __cdecl TrimCh(char a,char*lstr,NAT slen=0,char term=0)
{
char*tstr=lstr;
NAT nc=0;
while(slen)
 {
 *lstr=*tstr;
 if((*tstr!=a))
   {
   lstr++;
   nc++;
   }
 tstr++;
 slen--;
 }
*lstr=term;
return nc;
}

//taie characterele a din string, care se repeta ----------------------------------------------------------------------------
NAT __cdecl TrimRep(char a,char*lstr,NAT slen=0,char term=0)
{
char*tstr=lstr;
NAT nc=0;
while(slen)
 {
 *lstr=*tstr;
 if((*tstr!=a)||(*lstr!=*(lstr-1)))
   {
   lstr++;
   nc++;
   }
 tstr++;
 slen--;
 }
*lstr=term;
return nc;
}

//taie characterele a din string de la inceput ----------------------------------------------------------------------------
NAT __cdecl TrimBeg(char a,char*lstr,NAT slen=0,char term=0)
{
NAT l=0;
while(lstr[l]==a&&l<slen) //count them
 l++;
return sc(lstr,lstr+l,slen-l,term,term);
}

//taie characterele a de la sfarsitul string ----------------------------------------------------------------------------
NAT __cdecl TrimEnd(char a,char*lstr,NAT slen=0,char term=0)
{
if(!slen) slen=sl(lstr);
lstr+=slen-1;
while((slen)&&(*lstr==a))
 {
 lstr--;
 slen--;
 }
*(lstr+1)=term;
return slen;
}

//taie characterele intre a si z ----------------------------------------------------------------------------
NAT __cdecl TrimStr(char a,char z,char*lstr,NAT slen=0,char term=0)
{
char*tstr=lstr;
NAT nc=0;
while(slen)
 {
 *lstr=*tstr;
 if(((*tstr)<a)||((*tstr)>z))
   {
   lstr++;
   nc++;
   }
 tstr++;
 slen--;
 }
*lstr=term;
return nc;
}

//numarul de aparitii al ch in str --------------------------------------------------------
inline NAT __cdecl countch(char sep,LPSTR dstr,NAT dsz=0,char term=0)
{
__asm
 {
 mov ebx,dstr
 mov dl,term
 mov dh,sep
 mov ecx,dsz
 xor eax,eax
LNextChar:
  cmp dl,[ebx]
  je LBreakOut
  cmp dh,[ebx]
  jne LNoInc
  inc eax
LNoInc:
  inc ebx
  loop LNextchar
LBreakOut:
 }
}

//returns position of first sep or term in dstr-----------------------------------------------------
NAT __cdecl firstch(char sep,LPSTR dstr,NAT dsz=0,char term=0)
{
__asm
 {
 mov edx,dstr
 mov eax,edx
 mov ecx,dsz
 jecxz LBreakOut
 mov bl,term
 mov bh,sep
LNextchar:
  cmp bl,[eax]
  je LBreakOut
  cmp bh,[eax]
  je LBreakOut
  inc eax
  loop LNextchar
LBreakOut:
 sub eax,edx
 }
}

//returns position+1 of last sep or 0 in str (searches to term)------------------------------------------------------
NAT __cdecl lastch(char sep,LPSTR dstr,char term=0)
{
__asm
 {
 mov edx,dstr
 mov eax,edx
 mov cl,term
 mov ch,sep
LNextchar:
  cmp cl,[edx]
  je LBreakOut
  cmp ch,[edx]
  je LFound
  inc edx
  jmp LNextchar
LFound:
  inc edx
  mov eax,edx
  jmp LNextchar
LBreakOut:
 sub eax,dstr
 }
}

//find the ord-th sep (serching foreward to dest limit) in dstr----------------------------------------------------------
inline char* __cdecl nextch(char sep,char *dstr,NAT ord=1,NAT dsz=0)
{
__asm
 {
 mov eax,dstr
 mov edx,ord
 dec eax
 cmp edx,0
 jz LBreakOut
 mov bl,sep
 mov ecx,dsz
LNextchar:
  inc eax
  cmp bl,[eax]
  loopne LNextchar
  jecxz LBreakOut //if out becouse ecx==0
  dec edx
  jnz LNextchar
LBreakOut:
 inc eax //return position of sep + 1
 }
}

//find the ord-th sep (serching backwards to dest limit ) in dstr----------------------------------------------------------
inline char* __cdecl prevch(char sep,char *dstr,NAT ord=1,NAT dsz=0)
{
__asm
 {
 mov eax,dstr
 mov edx,ord
 cmp edx,0
 jz LBreakOut
 mov bl,sep
 mov ecx,dsz
LNextchar:
  dec eax
  cmp bl,[eax]
  loopne LNextchar
  jecxz LBreakOut //if out becouse ecx==0
  dec edx
  jnz LNextchar
  inc eax //return position of sep + 1
LBreakOut:
 }
}

//to lower case (eng only) ---------------------------------------------------------
NAT __cdecl LOcase(char*lstr,NAT lnc=0,char term=0)
{
__asm
 {
 mov eax,lstr
 mov esi,eax
 mov dl,'A'
 mov dh,'Z'
 mov bl,32
 mov bh,term
 mov ecx,lnc
LNextchar:
  cmp bh,[eax]
  je LBreakOut
  cmp [eax],dh
  ja LSkip
  cmp [eax],dl
  jb LSkip
  add [eax],bl
LSkip:  
  inc eax
  loop LNextchar
LBreakOut:
 sub eax,esi //return strlen(lstr)
 }
}

//to upper case (eng only) ---------------------------------------------------------
NAT __cdecl UPcase(char*lstr,NAT lnc=0,char term=0)
{
__asm
 {
 mov eax,lstr
 mov esi,eax
 mov dl,'a'
 mov dh,'z'
 mov bl,32
 mov bh,term
 mov ecx,lnc
LNextchar:
  cmp bl,[eax]
  je LBreakOut
  cmp [eax],dl
  jb LSkip
  cmp [eax],dh
  ja LSkip
  sub [eax],bl
LSkip:  
  inc eax
  loop LNextchar
LBreakOut:
 sub eax,esi //return strlen(lstr)
 }
}

//find any char in terms or R_NULL if none --------------------------------------------------------------------
inline NAT __cdecl s_anych(char*lstr,char*terms,NAT nrt,NAT maxsz=0)
{
__asm
 {
 mov ebx,lstr
LNextChar:
  mov edx,terms
  mov ecx,nrt
  mov al,[ebx]
LNextTerm:
   cmp al,[edx]
   je LFoundTerm
   inc edx //next term
   dec ecx
   jnz LNextTerm
  inc ebx
  dec maxsz
  jnz LNextChar
 mov eax,R_NULL
 jmp LReturn 
LFoundTerm:
 mov eax,ebx
 sub eax,lstr  //return strlen(dstr)
LReturn:
 }
}

//find seq in lstr or R_NULL if none --------------------------------------------------------------------
inline NAT __cdecl s_seq(char*lstr,char*seq,NAT seqsz,NAT maxsz=0)
{
__asm
 {
 mov edx,lstr
 mov ebx,maxsz
 cmp ebx,seqsz
 jb	LNotFound       //if(seqsz<maxsz) return R_NULL
 dec seqsz
 sub ebx,seqsz		//maxsz-=seqsz-1
 mov esi,seq
 mov al,[esi]
LNextChar:
  cmp al,[edx]
  je LFoundMatch
  inc edx //next
LContinueChar:
  dec ebx
  jnz LNextChar
 jmp LNotFound 
LFoundMatch:
 mov ecx,seqsz
 jecxz LSeqComplete
 mov eax,edx
 inc edx //next
 mov edi,edx
 inc esi
LNextCharInSeq:
  cmpsb
  jne LRestartSeq
  loop LNextCharInSeq
LSeqComplete:
 sub eax,lstr  //return strlen(lstr)
 jmp LReturn 
LRestartSeq:
 mov esi,seq
 mov al,[esi]
 jmp LContinueChar
LNotFound: 
 mov eax,R_NULL
LReturn:
 }
}

//find seq (case sensitive) in lstr or R_NULL if none --------------------------------------------------------------------
inline NAT s_seqS(char*lstr,char*seq,NAT seqsz,NAT maxsz=0)
{
int found;
if(!lstr||!seq) return R_NULL;
for(int p=0;p<=(signed)(maxsz-seqsz);p++)
 {
 found=1;
 for(NAT c=0;c<seqsz;c++)
  {
  if(lstr[p+c]==seq[c])
   continue;
  else
   {
   found=0;
   break;
   }
  }
 if(found) return p;
 }
return R_NULL;
}

//find seq (case insensitive) in lstr or R_NULL if none --------------------------------------------------------------------
inline NAT s_seqI(char*lstr,char*seq,NAT seqsz,NAT maxsz=0)
{
int found;
if(!lstr||!seq) return R_NULL;
for(int p=0;p<=(signed)(maxsz-seqsz);p++)
 {
 found=1;
 for(NAT c=0;c<seqsz;c++)
  {
  if(lstr[p+c]==seq[c])
   continue;
  else if(seq[c]>='A'&&seq[c]<='Z'&&lstr[p+c]>='a'&&lstr[p+c]<='z')
   {
   if(lstr[p+c]-32==seq[c])
    continue;
   else
    {
    found=0;
    break;
    }
   }
  else if(seq[c]>='a'&&seq[c]<='z'&&lstr[p+c]>='A'&&lstr[p+c]<='Z')
   {
   if(lstr[p+c]==seq[c]-32)
    continue;
   else
    {
    found=0;
    break;
    }
   }
  else
   {
   found=0;
   break;
   }
  }
 if(found)   return p;
 }
return R_NULL;
}

//multiplies a string rep times ------------------------------------------------------------
char*ReplicateStr(char*dest,char*kern,NAT rep=2)
{
NAT t=0;
while(rep)
 {
 t+=sc(dest+t,kern);
 rep--;
 }
return dest;
}

//pozitia pe care apare ch in str sau pozitia pe care e NULL-------------------------------------------------------------
inline int chinstr(char ch,LPSTR str,int ord=1,NAT of=0)
{
while(str[of])
 {
 if(str[of]==ch)
  {
  ord--;
  if(!ord) return of;
  }
 of++;
 }
return of;
}

//searches for a block in a string ----------------------------------------------------------------------------------------
char* blkinstr(LPSTR blk,NAT blkl,LPSTR lstr,char term=0)
{
char*retv=NULL;
int e=0;
while(*lstr!=term)
 {
 if(*lstr==blk[e])
  {
  if(e+1==blkl) return retv;
  e++;
  }
 else
  {
  e=0;
  retv=lstr;
  }
 lstr++;
 }
return NULL;
}

//deletes lstr found in text, returns new text size ----------------------------------------------------------------------
int DelStrInBlk(LPSTR text,NAT txtszB,LPSTR lstr,NAT strszB=0)
{
if(!strszB) strszB=sl(lstr);
NAT d,s,i,c;
d=s=i=c=0;
while(s<txtszB)
 {
 for(i=0;i<strszB;i++)
  if(text[s+i]!=lstr[i])
   break;
 if(i==strszB) //found
  {
  s+=strszB;
  c++;
  }
 text[d]=text[s];
 d++;
 s++;
 }
return txtszB-c*strszB;
}

//reads a line of text from a file up to eof,lf,cr or null -------------------------------------------------------------------
inline void fgetstr(FILE* fis,char* lstr)
{
*lstr=fgetc(fis);
if((unsigned)*lstr<' ') lstr--;
do{
 lstr++;
 *lstr=fgetc(fis);
}while(*lstr!=EOF&&*lstr!='\n'&&*lstr!='\r'&&*lstr!='\0');
*lstr='\0';
}

//string to integer ------------------------------------------------------------------------------------------------------
int StoI(LPSTR str,int rad=10,char**rets=NULL,NAT maxnc=80)
{
int val=0,sgn=1;
char max1=47+rad,max2='A';
if(max1>'9')
 {
 max1='9';
 max2='A'+rad-10;
 }
while(*str&&maxnc)
 {
 if(*str>='0'&&*str<=max1)
  val=(val*rad)+*str-48;
 else if(*str>='A'&&*str<max2)
  val=(val*rad)+*str-55;
 else if(*str>='a'&&*str<max2+32)
  val=(val*rad)+*str-87;
 else if(*str=='-')
  sgn=-1;
 else if(val)
  break;
 str++;
 maxnc--;
 }
if(rets) *rets=str;
return sgn*val;
}

//string to unsigned integer ---------------------------------------------------------------------------------------------
int StoU(LPSTR str,int rad=10,char**rets=NULL,NAT maxnc=80)
{
int val=0;
char max1=47+rad,max2='A';
if(max1>'9')
 {
 max1='9';
 max2='A'+rad-10;
 }
while(*str&&maxnc)
 {
 if(*str>='0'&&*str<=max1)
  val=(val*rad)+*str-48;
 else if(*str>='A'&&*str<max2)
  val=(val*rad)+*str-55;
 else if(*str>='a'&&*str<max2+32)
  val=(val*rad)+*str-87;
 else if(val)
  break;
 str++;
 maxnc--;
 }
if(rets) *rets=str;
return val;
}

//integer to string (maxnc doesn't include the terminator) ---------------------------------------------------------------
void ItoS(int val,LPSTR str,int rad=10,int maxnc=80,char term='\0',char fill=' ')
{
int sgn;
str+=maxnc;
if(term!=C_NULL) *str=term;
if(val<0)
 {
 val=-val;
 sgn=-1;
 }
else
 sgn=1; 
do{
 str--;
 *str=val%rad;
 if(*str<10) *str+='0';
 else *str+=55;
 val/=rad;
 maxnc--;
 }while(val&&maxnc);
if(sgn==-1&&maxnc)
 {
 str--;
 *str='-';
 maxnc--;
 }
while(maxnc)
 {
 str--;
 *str=fill;
 maxnc--;
 }
}

//unsigned to string (result is right aligned in maxnc) ---------------------------------------------------------------
void UtoS(UNS val,LPSTR str,int rad=10,int maxnc=80,char term='\0',char fill=' ')
{
str+=maxnc;
if(term!=C_NULL) *str=term;
do{
 str--;
 *str=val%rad;
 if(*str<10) *str+='0';
 else *str+=55;
 val/=rad;
 maxnc--;
 }while(val&&maxnc);
while(maxnc)
 {
 str--;
 *str=fill;
 maxnc--;
 }
}

//string to fixed ----------------------------------------------------------------
__int64 StoF(LPSTR str,int rad=10,int prec=0,int sgn=1,NAT maxnc=80,char**rets=NULL)
{   //sgn=0-unsigned 1-signed
__int64 val=0;
int decs=-1,f=0;
char max1=47+rad,max2='A';
if(max1>'9')
 {
 max1='9';
 max2='A'+rad-10;
 }
while(*str&&maxnc)
 {
 if(*str=='0') //this is separate so you can bail at the right time
  {
  val*=rad;
  f=1;
  }
 else if(*str>'0'&&*str<=max1)
  val=(val*rad)+*str-48;
 else if(*str>='A'&&*str<max2)
  val=(val*rad)+*str-55;
 else if(*str>='a'&&*str<max2+32)
  val=(val*rad)+*str-87;
 else if(*str=='-')
  {
  if(sgn<=0) break;   //2nd '-' breaks	or if unsigned
  sgn=-1;
  }
 else if(*str=='.'||*str==',')
  {
  if(decs>=0) break; //2nd	'.' breaks
  decs=0;
  }
 else if(val||f) //bail
  break;
 str++;
 maxnc--;
 if(decs>=0)
  {
  if(decs>=prec) break; //found required precision
  decs++;
  }
 }
if(rets) *rets=str;
return sgn<0?-val:val;
}

//fixed integer to string (maxnc doesn't include the terminator) ------------------------
NAT FtoS(__int64 num,LPSTR str,int lstrnc=80,int rad=10,int prec=0,int sgn=1,char fill=C_NULL,char term='\0')
{
int maxnc=lstrnc;
unsigned __int64 val;
str+=maxnc;
if(term!=C_NULL) *str=term;
if(sgn&&num<0)
 {
 val=-num;
 sgn=-1;
 }
else
 val=num;
do{
 if((prec==0)&&(maxnc>=2))
  {
  str--;
  *str='.';
  maxnc--;
  }
 prec--;
 str--;
 *str=val%rad;
 if(*str<10) *str+='0';
 else *str+=55;
 val/=rad;
 maxnc--;
 }while(val&&maxnc);
if(sgn<0&&maxnc)
 {
 str--;
 *str='-';
 maxnc--;
 }
if(fill==C_NULL) //right align
 {
 if(maxnc>0) ShiftMemL(str,maxnc,lstrnc-maxnc+1);
 return lstrnc-maxnc;
 }
else while(maxnc)  //left align
 {
 str--;
 *str=fill;
 maxnc--;
 }
return lstrnc;
}

//string to real ----------------------------------------------------------------
double StoR(LPSTR str,double rad=10,NAT maxnc=80,char**rets=NULL)
{
double val=0.,dig=1.0;
int decs=0,sgn=0;
char max1=47+rad,max2='A';
if(max1>'9')
 {
 max1='9';
 max2='A'+rad-10;
 }
while(*str&&maxnc)
 {
 if(*str>='0'&&*str<=max1)
  {
  if(decs)
   val+=dig*(*str-48);
  else
   val=(val*rad)+*str-48;
  }
 else if(*str>='A'&&*str<max2)
  {
  if(decs)
   val+=dig*(*str-55);
  else
   val=(val*rad)+*str-55;
  }
 else if(*str>='a'&&*str<max2+32)
  {
  if(decs)
   val+=dig*(*str-87);
  else
   val=(val*rad)+*str-87;
  }
 else if(*str=='-')
  {
  if(sgn) break;   //2nd '-' breaks
  sgn=1;
  }
 else if(*str=='.'||*str==',')
  {
  if(decs) break; //2nd	'.' breaks
  decs=1;
  }
 else if(val)
  break;
 str++;
 maxnc--;
 if(decs) dig/=rad;
 }
if(rets) *rets=str;
if(sgn)
 return _chgsign(val);
else 
 return val;
}

//real to string (maxnc doesn't include the terminator) ------------------------
NAT RtoS(double val,char*str,int lstrnc=80,int rad=10,int prec=0,char fill=C_NULL,char term='\0')
{
int maxnc=lstrnc,sgn=1;
double intval;
if(maxnc<prec) return 0; //RtoS() needs at least prec digits
str+=maxnc;
if(term!=C_NULL) *str=term;
str-=prec;
maxnc-=prec;
if(val<0)
 {
 val=_chgsign(val);
 sgn=-1;
 }
val=modf(val,&intval);
for(int d=0;d<prec;d++)
 {
 val*=rad;
 str[d]=val;
 val-=str[d];
 if(str[d]<10) str[d]+='0';
 else str[d]+=55;	//'A'-10
 }
if(maxnc!=lstrnc)
 {
 str--;
 *str='.';
 maxnc--;
 }
do{
 str--;
 *str=fmod(intval,rad);
 intval/=rad;
 if(*str<10) *str+='0';
 else *str+=55;
 maxnc--;
 }while((intval>=1.)&&maxnc);
if(sgn<0&&maxnc)
 {
 str--;
 *str='-';
 maxnc--;
 }
if(fill==C_NULL) //right align
 {
 if(maxnc>0) ShiftMemL(str,maxnc,lstrnc-maxnc+1);
 return lstrnc-maxnc;
 }
else while(maxnc)  //left align
 {
 str--;
 *str=fill;
 maxnc--;
 }
return lstrnc;
}

//read string up to terminators ----------------------------------------------------------------------------------------------
NAT file_next_word(char*retv,NAT maxnc,char*buffer,NAT bufsz,char*terms,NAT nrt)
{
NAT ncc=0;
while(*buffer<=' ') //find begining
 buffer++,bufsz--;
while(bufsz&&ncc<maxnc)
 {
 for(int i=0;i<nrt;i++)
  if(*buffer==terms[i])
   return ncc;
 retv[ncc++]=*buffer;
 buffer++,bufsz--;
 }
return ncc;
}

//read value with label 'seq' ----------------------------------------------------------------------------------------------
BOOL find_valueS(char*retv,NAT maxnc,char*buffer,NAT bufsz,char*seq,NAT seqsz=0,char*terms=NULL,NAT nrt=0)
{
NAT p;
if(!seqsz) seqsz=sl(seq);
if(!terms)
 terms="\n\r",nrt=2;
else if(!nrt)
 nrt=sl(terms);
if((p=s_seqI(buffer,seq,seqsz,bufsz))!=R_NULL)
 return file_next_word(retv,maxnc,buffer+p+seqsz,bufsz-p-seqsz,terms,nrt);
return 0; //label not found
}

//pathd=pathd'\'paths ---------------------------------------------------------------------------------------------------
inline char* __cdecl appendpath(char*pathd,char*paths,char term2=0)
{
__asm
 {
 mov edi,pathd
 mov esi,paths
 xor dh,dh
 mov dl,'\\'
 dec edi
LNextchar1: //find eos(pathd)
  inc edi
  cmp dh,[edi] //if null break
  jne LNextchar1
 cmp dl,[edi-1]
 je LFoundSep
 mov [edi],dl  //append '\' if not found at end of pathd
 inc edi
LFoundSep:
 mov eax,edi //return pointer to paths in pathd (nulling this will remove paths)
 cmp dl,[esi] //clip '\' if found at the begining of paths
 mov dl,term2 //optional terminator for paths
 je LClip
 cmp byte ptr[esi],'/' //clip '/' if found at the begining of paths
 jne LNextchar2
LClip:
 inc esi
LNextchar2: //append paths to pathd
  cmp dl,[esi]
  je LFoundTerm
  cmp dh,[esi]
  je LFoundTerm
  movsb
  jmp LNextchar2
LFoundTerm:
 mov [edi],dh  //put terminator
 }
}

//path=path1'\'path2 ------------------------------------------------------------------------------------------------------
inline char* __cdecl mergepath(char*path,char*path1,char*path2,char term2=0,char sep='\\')
{
__asm
 {
 mov edi,path
 mov esi,path1
 xor dh,dh
 mov dl,sep
LNextchar1: //copy path1
  cmp dh,[esi] //if null break
  jz LBreakOut1
  movsb
  jmp LNextchar1
LBreakOut1:
 cmp dl,[edi-1]
 je LFoundSep
 mov [edi],dl  //append sep if not found at end of path1
 inc edi
LFoundSep:
 mov eax,edi //return pointer to path2 in path (nulling this will remove path2)
 mov esi,path2
 mov dl,term2 //optional terminator for path2
 cmp byte ptr[esi],'\\' //clip '\' if found at the begining of path2
 je LClip
 cmp byte ptr[esi],'/' //clip '/' if found at the begining of path2
 jne LNextchar2
LClip:
 inc esi
LNextchar2: //append path2 to path
  cmp dl,[esi]
  je LFoundTerm
  cmp dh,[esi]
  je LFoundTerm
  movsb
  jmp LNextchar2
LFoundTerm:
 mov [edi],dh  //put terminator
 }
}

//copy one path element at a time (permits one '\' as first char then is considered terminator)-----------------------------------------------------------------
inline NAT __cdecl parsepath(char*temppath,char*path)
{
__asm
 {
 mov edi,temppath
 mov esi,path
 mov eax,edi
 xor dh,dh
 cmp dh,[esi]
 je LFoundTerm
 movsb
 mov dl,'\\'
LNextchar:
  cmp dl,[esi] //if '\' break
  je LFoundTerm
  cmp dh,[esi]
  je LFoundTerm
  movsb
  jmp LNextchar
LFoundTerm:
 mov [edi],dh  //put terminator
 sub eax,edi
 neg eax //return nr of chars copied
 }
}

//change (or add file extension) --------------------------------------------------------------------------------------
inline NAT __cdecl changefileext(LPSTR filename,LPSTR newext)
{
__asm
 {
 mov edi,filename
 mov eax,edi
 xor dh,dh
 mov dl,'.'
 dec edi
LNextchar1: //find eos(pathd)
  inc edi
  cmp dl,[edi] //if '.' break
  je LAppendExt
  cmp dh,[edi] //if null break
  jne LNextchar1
 mov [edi],'.'  //put sep
LAppendExt:
 inc edi
 mov esi,newext
LNextchar2:
  cmp dh,[esi]
  je LFoundTerm
  movsb
  jmp LNextchar2
LFoundTerm:
 mov [edi],0  //put terminator
 sub edi,eax
 mov eax,edi //return new path strlen
 }
}

//copies the path from npath to dpath and appends filename from spath --------------------------------------
char* __cdecl pathtransfer(LPSTR dpath,LPSTR spath,LPSTR npath=NULL)
{
__asm
 {
 mov edi,dpath
 mov esi,npath
 xor edx,edx //dh=term
 mov dl,'\\'
 test esi,esi
 jz LFindFile //skip if npath is NULL
 cmp dh,[esi]
 jz LFindFile //skip if npath is empty
LNextChar1: //copy npath
  movsb
  cmp dh,[esi] //if null break
  jne LNextChar1
 cmp dl,[edi-1]
 je LFindFile
 mov [edi],dl //put a '\'
 inc edi
LFindFile:
 mov eax,edi //return a pointer to filename in dpath
 mov ecx,spath
 mov esi,ecx
 dec ecx
LNextChar2:
  inc ecx
  cmp dh,[ecx]
  je LBreakOut1
  cmp dl,[ecx] //if(ecx=='\') esi=ecx+1
  jne LNextChar2
  mov esi,ecx
  inc esi
  jmp LNextChar2
LBreakOut1:
 sub ecx,esi //ecx=len(filename from spath)
 jecxz LBreakOut2
 rep movsb //append filename to dpath
LBreakOut2:
 mov [edi],dh //put terminator
 }
}

//File Four Character Code -----------------------------------------------------------------------
NAT FFCC(LPSTR path)
{
__asm
 {
 mov esi,path
 xor eax,eax
 mov dh,'.'
Lloop1:
  mov cl,[esi]
  inc esi
  test ecx,0xff //cmp cl,0
  jz Lret //if found '\0' return 0;
  cmp cl,dh
  jne Lloop1
 mov dh,'|'
 mov ecx,4
Lloop2: //copy until '\0' or '|'
  mov dl,[esi]
  inc esi
  test edx,0xff //cmp dl,0
  jz Lbreak2
  cmp dl,dh
  je Lbreak2
  mov al,dl
  ror eax,8 //next byte
  dec ecx
  jnz Lloop2
Lbreak2:
 shl ecx,3 //cl*=8
 ror eax,cl
Lret:
 }
}

//gets ext, name and dir from path (sets to NULL if not present) -------------------------------
void __cdecl PathNameExt(char*fpne,char**ppath=NULL,char**pname=NULL,char**pext=NULL)
{
__asm
 {
 mov esi,fpne
 mov edi,esi //name
 mov edx,esi //ext
 mov ebx,esi //path (const)
 test esi,esi
 jz LBreak //if(!fpne) break (return all NULLs)
 mov cl,'\\'
 mov ch,'.'
 xor eax,eax //ah='\0' (term)
LLoop:
  lodsb
  test al,al //if(!*fpne) break
  jz LBreak
  cmp al,cl //if(*fpne=='\\') path=fpne;
  je LSetName
  cmp al,ch  //if(*fpne!='.') loop
  jne LLoop
  mov edx,esi //ext=fpne+1
  jmp LLoop
LSetName:
  mov edi,esi //name=fpne+1
  jmp LLoop
LBreak:
 mov ecx,ppath //if(!ppath) next
 jecxz Lnext1
 cmp ebx,edi   //if(path!=name) *ppath=path
 jne Lif1
 xor ebx,ebx   //else *ppath=NULL
Lif1:
 mov [ecx],ebx
Lnext1:
 mov ecx,pext //if(!pext) next
 jecxz Lnext2
 cmp edx,ebx  //if(ext!=path) *pext=ext
 jne Lif2
 xor edx,edx  //else *pext=NULL
Lif2:
 mov [ecx],edx
Lnext2:
 mov ecx,pname //if(!pname) return
 jecxz Lret
 mov [ecx],edi
Lret:
 }
 //?TODO?: if(name) *(name-1)='\0'; if(ext) *(ext-1)='\0'; - separates path from name and name from extension
}

//gets dir  and file from path --------------------------------------------------------------------
void __cdecl SplitPath(char*path,char*dir,char*file,char sep='\\')
{
NAT l;
if((l=lastch(sep,path))>0)
 sc(dir,path,l-1);
else
 *dir=0;//""
sc(file,path+l);
}

#endif