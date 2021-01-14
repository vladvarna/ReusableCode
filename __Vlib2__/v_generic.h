//Global macros//////////////////////////////////////////////////////////////////////////////////
#define UNS unsigned
#define nat unsigned int
#define NAT unsigned int
#define INT signed int
#define OCTET signed char
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define QWORD unsigned __int64
#define SINGLE float

#define BOOL int
#define FAIL int //function return: 0=succes, P(>0)=failure level, N(<0)=special situation
#define FLAGS unsigned long //DWORD

#define BITS8  unsigned __int8
#define BITS16 unsigned __int16
#define BITS32 unsigned __int32
#define BITS64 unsigned __int64

//#define ASCII unsigned char
#define ASCIZ char* //null terminated string

#define ifn(condition)		if(!(condition))

#include <stdio.h>
#include <malloc.h>

#define PATHSZ 260 //MAX_PATH
#define BUFFSZ 1024
#define SBUFSZ 256
#define NAMESZ 64
#define PATHDEPTH 128 //used as a stack size in some recursive functions

#define ALLOC(bytes)        malloc(bytes)
#define ALLOC0(bytes)       calloc(1,bytes)
#define REALLOC(pmem,bytes) realloc(pmem,bytes)
#define SALLOC(slen)        (LPSTR)malloc((slen)+1)
#define ALLOC_BYTE(bytes)   (BYTE*)malloc(bytes)
#define ALLOC_INT(n)        (int*)malloc((n)*sizeof(int))
#define ALLOC_NAT(n)        (NAT*)malloc((n)*sizeof(NAT))
#define ALLOC_FLOAT(n)      (float*)malloc((n)*sizeof(float))
#define ALLOC_DOUBLE(n)     (double*)malloc((n)*sizeof(double))
#define ALLOC_WORD(n)       (WORD*)malloc((n)*sizeof(WORD))
#define ALLOC_DWORD(n)      (DWORD*)malloc((n)*sizeof(DWORD))
#define ALLOC_POINTER(n)    (void**)malloc((n)*sizeof(void*))
#define MEMSZ(pmem)         _msize(pmem)
#define FREE(pmem)          if(pmem) free(pmem),pmem=NULL
#define ARRAY_DIM(x)        (sizeof((x))/sizeof((x)[0])) //number of elements in an array
#define DEL(obj)            delete obj,obj=NULL
#define ZEROCLASS(class_name)	memset(this,0,sizeof(class_name))  //Zero class
#define ZEROCLASSV(class_name)	memset((char*)this+sizeof(void*),0,sizeof(class_name)-sizeof(void*))  //Zero class with vtable

#define V_CREATOR "Vlad Varna"
#define V_EMAIL "vladvarna@yahoo.com"
#define V_PHONE "+40-(0)21 321 1884"
