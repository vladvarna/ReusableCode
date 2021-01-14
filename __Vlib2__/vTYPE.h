#ifndef V_DATA_TYPES
#define V_DATA_TYPES

#define DLL_IMP extern __declspec(dllimport)
#define DLL_EXP extern __declspec(dllexport)

//Mouse info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct MouseStatus
 {
 int dx,dy,dz; //rel coords
 NAT b; //button states; bit: 8-left, 16-right, 24-middle
 int x,y; //abs coords
 int ex,ey; //last event place
 NAT ev; //last event (MB_X)
 float s; //scale factor
 int l,u,r,d; //clip rect
 };
//Mouse buttons
#define MB_L 0x01
#define MB_R 0x02
#define MB_S 0x04 //shift
#define MB_C 0x08 //control
#define MB_M 0x10
#define MB_1 0x20 //X1
#define MB_2 0x40 //X2
#define MB_V 0x80 //vlad win button
#define MB_H 0x8000 //hover

#pragma pack(push,default_pack)
#pragma pack(1) //byte packing

//QuadByte=DoubleWord ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
union QUADB
 {
 int i;
 unsigned u;
 long l;
 unsigned long n;
 float f;
 unsigned short w[2];
 unsigned char b[4];
 };
//QuadWord= 64 bits ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
union QUADW
 {
 __int64 i64;
 __int32 i[2];
 unsigned __int64 u64;
 unsigned __int32 u[2];
 double d;
 float f[2];
 unsigned short w[4];
 unsigned char b[8];
 };
//All types of pointer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
union POINTER
 {
 void *v;
 char *c;
 BYTE *b;
 short *s;
 WORD *w;
 int *i;
 long *l;
 DWORD *dw;
 unsigned *u;
 __int64 *q;
 float *f;
 double *d;
 void**p;
 };

//2D POINT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct VECT2
 {
 union {float X; long x,_1;};
 union {float Y; long y,_2;};
 };

//3D POINT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct VECT3
 {
 union {float X; long x;};
 union {float Y; long y;};
 union {float Z; long z;};
 };
//3+1D POINT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct VECT4
 {
 union {float X; long x,l;};
 union {float Y; long y,u;};
 union {float Z; long z,r,w;};
 union {float W; long w,d,h;};
 };

#pragma pack(pop,default_pack)

#endif