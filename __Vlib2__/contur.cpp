#ifndef V_CONTUR
#define V_CONTUR

#include <img.cpp>

float ConturQual=12.0f; //12(fast)-18(qual)

struct CONTUR
{
int dir; //<0 - up, >0 - down
NAT nrd; //nr drepte = nr points-1
VECT2f*pts;
};

class Contur
{
public:
 Image*plan;
 NAT nrp,nrc;
 NAT*plg;
 CONTUR*ctr;
 float minx,miny,maxx,maxy;
 int cx,cy;
 
 Contur();
 ~Contur();
 void Init(Image*);
 void Free();
 void start(float,float); //start new poly
 void lineto(float,float);
 void spline2(float,float,float,float);
 void close(); //close current poly
 void Draw(DWORD,float,float);
 void Fill(DWORD,float,float,int,int,int);
 short*Trace(int,float,float);
};

// Contur <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//...........................................................................................
Contur::Contur()
{
ZeroMemory(this,sizeof(Contur));
minx=miny=FLT_MAX;
maxx=maxy=-FLT_MAX;
}

//...........................................................................................
Contur::~Contur()
{
Free();
}

//...........................................................................................
void Contur::Init(Image*img)
{
plan=img;
}

//...........................................................................................
void Contur::Free()
{
for(int c=0;c<nrc;c++)
 FREE(ctr[c].pts);
FREE(ctr);
FREE(plg);
nrp=nrc=0;
minx=miny=FLT_MAX;
maxx=maxy=-FLT_MAX;
}

//...........................................................................................
inline void Contur::start(float x,float y)
{
close(); //make sure previous poly is closed
nrp++;
plg=(NAT*)realloc(plg,nrp*sizeof(NAT));
plg[nrp-1]=nrc;
nrc++;
ctr=(CONTUR*)realloc(ctr,nrc*sizeof(CONTUR));
ctr[nrc-1].pts=(VECT2f*)malloc(2*sizeof(VECT2f));
ctr[nrc-1].nrd=0;
ctr[nrc-1].dir=0;
ctr[nrc-1].pts[0].x=x;
ctr[nrc-1].pts[0].y=y;
if(x<minx) minx=x;
if(x>maxx) maxx=x;
if(y<miny) miny=y;
if(y>maxy) maxy=y;
cx=x;
cy=y;
}

//...........................................................................................
inline void Contur::close()
{
if(!nrp) return; //nothing to close
if(!(ctr[nrc-1].pts[ctr[nrc-1].nrd].x==ctr[plg[nrp-1]].pts[0].x&&ctr[nrc-1].pts[ctr[nrc-1].nrd].y==ctr[plg[nrp-1]].pts[0].y))
 lineto(ctr[plg[nrp-1]].pts[0].x,ctr[plg[nrp-1]].pts[0].y); //auto close
if(nrc<=1) return; //nothing to concatenate
if(ctr[nrc-1].dir==ctr[plg[nrp-1]].dir&&(plg[nrp-1]!=nrc-1)) //concatenate
 {
 ctr[nrc-1].pts=(VECT2f*)realloc(ctr[nrc-1].pts,(ctr[nrc-1].nrd+ctr[plg[nrp-1]].nrd+1)*sizeof(VECT2f));
 CopyMemory(ctr[nrc-1].pts+ctr[nrc-1].nrd+1,ctr[plg[nrp-1]].pts+1,ctr[plg[nrp-1]].nrd*sizeof(float)*2);
 ctr[nrc-1].nrd+=ctr[plg[nrp-1]].nrd;
 nrc--;
 ctr[plg[nrp-1]]=ctr[nrc];
 }
}

//...........................................................................................
void Contur::lineto(float x,float y)
{
if(x==cx&&y==cy) return; //don't add points
int dir=SGN(y-cy);
if(!ctr[nrc-1].dir) //no direction defined
 {
 ctr[nrc-1].dir=dir;
 ctr[nrc-1].nrd++;
 ctr[nrc-1].pts=(VECT2f*)realloc(ctr[nrc-1].pts,(ctr[nrc-1].nrd+1)*sizeof(VECT2f));
 ctr[nrc-1].pts[ctr[nrc-1].nrd].x=x;
 ctr[nrc-1].pts[ctr[nrc-1].nrd].y=y;
 }
else
 {
 if(ctr[nrc-1].dir==dir||!dir) //add to curent contur (same direction or no direction)
  {
  ctr[nrc-1].nrd++;
  ctr[nrc-1].pts=(VECT2f*)realloc(ctr[nrc-1].pts,(ctr[nrc-1].nrd+1)*sizeof(VECT2f));
  ctr[nrc-1].pts[ctr[nrc-1].nrd].x=x;
  ctr[nrc-1].pts[ctr[nrc-1].nrd].y=y;
  }
 else
  {
  nrc++;
  ctr=(CONTUR*)realloc(ctr,nrc*sizeof(CONTUR));
  ctr[nrc-1].nrd=1;
  ctr[nrc-1].dir=dir;
  ctr[nrc-1].pts=(VECT2f*)malloc(2*sizeof(VECT2f));
  ctr[nrc-1].pts[0].x=cx;
  ctr[nrc-1].pts[0].y=cy;
  ctr[nrc-1].pts[1].x=x;
  ctr[nrc-1].pts[1].y=y;
  }
 }
if(x<minx) minx=x;
if(x>maxx) maxx=x;
if(y<miny) miny=y;
if(y>maxy) maxy=y;
cx=x;
cy=y;
}

//contour quadratic bezier spline (x1,y1)=(cx,cy) .....................................................................
void Contur::spline2(float x2,float y2,float x3,float y3)
{
int np;
float x,y,dx,dy,d2x,d2y;
np=(fabs(x3-x2)+fabs(x2-cx)+fabs(y3-y2)+fabs(y2-cy))/ConturQual;
if(!(cx*y2+x3*cy+x2*y3-x3*y2-x2*cy-cx*y3)) //all 3 points are coliniar
 {
 lineto(x3,y3);
 return;
 }
fpuiof=(float)1.0/(np*np);
d2x=(2*cx-4*x2+2*x3)*fpuiof;
d2y=(2*cy-4*y2+2*y3)*fpuiof;
dx=(-2*cx+2*x2)/np+d2x/2;
dy=(-2*cy+2*y2)/np+d2y/2;
x=cx;
y=cy;
for(int n=1;n<np;n++)
 {
 x+=dx;
 y+=dy;
 dx+=d2x;
 dy+=d2y;
 lineto(x,y);
 }
lineto(x3,y3);
}

//...........................................................................................
void Contur::Fill(DWORD cc=0,float hz=1.0f,float vz=1.0f,int maxh=0,int l=0,int u=0)
{
if(!nrc||!plan) return; //must use Init() before drawing
NAT nrh,o,p;
int j,i,min1,min2;
float *hit,x,y,a,b,c;
hit=(float*)malloc(nrc*sizeof(float));
if(!maxh) maxh=maxy*vz;
plan->color=cc;
for(j=0;j<maxh;j++)
 {
 y=(float)j/vz;
 nrh=0;
 for(o=0;o<nrc;o++)
  {
  //find intersections (scanline)
  //up
  if(ctr[o].dir==-1)
   {
   if(y>ctr[o].pts[0].y||y<ctr[o].pts[ctr[o].nrd].y) continue; //contur doesn't intersect scanline
   for(p=1;p<=ctr[o].nrd;p++)
    {
    if(y>=ctr[o].pts[p].y&&y<=ctr[o].pts[p-1].y) //intersect with [p-1,p]
     {
     a=ctr[o].pts[p-1].y-ctr[o].pts[p].y;
     if(a)
      {
      b=ctr[o].pts[p].x-ctr[o].pts[p-1].x;
      c=ctr[o].pts[p-1].x*ctr[o].pts[p].y-ctr[o].pts[p].x*ctr[o].pts[p-1].y;
      x=-(y*b+c)/a;
      }
     else //hline
      {
      x=ctr[o].pts[p].x;
      i=x*hz;
      plan->at(l+i,u+j);
      plan->linh(ctr[o].pts[p-1].x*hz-i+1);
      }
     hit[nrh]=x;
     nrh++;
     break;
     }
    }
   }
  //down
  else if(ctr[o].dir==1)
   {
   if(y<ctr[o].pts[0].y||y>ctr[o].pts[ctr[o].nrd].y) continue; //contur doesn't intersect scanline
   for(p=1;p<=ctr[o].nrd;p++)
    {
    if(y<=ctr[o].pts[p].y&&y>=ctr[o].pts[p-1].y) //intersect with [p-1,p]
     {
     a=ctr[o].pts[p-1].y-ctr[o].pts[p].y;
     if(a)
      {
      b=ctr[o].pts[p].x-ctr[o].pts[p-1].x;
      c=ctr[o].pts[p-1].x*ctr[o].pts[p].y-ctr[o].pts[p].x*ctr[o].pts[p-1].y;
      x=-(y*b+c)/a;
      }
     else //hline
      {
      x=ctr[o].pts[p].x;
      i=x*hz;
      plan->at(l+i,u+j);
      plan->linh(ctr[o].pts[p-1].x*hz-i+1);
      }
     hit[nrh]=x;
     nrh++;
     break;
     }
    }
   }
  else
   error("X undef dir");
  }
 //sort and draw
 while(nrh>=2)
  {
  //2 min sort
  min1=hit[0]<hit[1]?0:1;
  min2=1-min1;
  for(p=2;p<nrh;p++)
   {
   if(hit[p]<hit[min2])
    {
    if(hit[p]<hit[min1])
     {
     min2=min1;
     min1=p;
     }
    else
     min2=p;
    }
   }
  //draw
  i=hit[min1]*hz;
  plan->at(l+i,u+j);
  plan->linh(hit[min2]*hz-i+1);
  //remove mins
  o=min1<min2?min1:min2;
  p=min1<min2?min2:min1;
  MoveMemory(hit+p,hit+p+1,(nrh-p-1)*sizeof(float));
  nrh--;
  MoveMemory(hit+o,hit+o+1,(nrh-o-1)*sizeof(float));
  nrh--;
  }
 }
free(hit);
}

//...........................................................................................
void Contur::Draw(DWORD cc,float hz=1.0f,float vz=1.0f)
{
if(!plan) return;
plan->color=cc;
for(int c=0;c<nrc;c++)
 {
 plan->at(ctr[c].pts[0].x*hz,ctr[c].pts[0].y*vz);
 for(int p=1;p<=ctr[c].nrd;p++)
  plan->lin(ctr[c].pts[p].x*hz,ctr[c].pts[p].y*vz);
 }
}

//...........................................................................................
short* Contur::Trace(int maxh,float hz=1.0f,float vz=1.0f)
{
if(!nrc) return NULL;
NAT nrh,o,p,tsz,ltsz,pcnt; //pcnt=packet counter
int j,i,min1,min2,flo,llo; //flo/llo=first/last line offset
float *hit,x,y,a,b,c;
short*trace=NULL;
hit=(float*)malloc(nrc*sizeof(float));
flo=llo=0;
tsz=5;
trace=(short*)malloc((5+(nrc+1)*maxh)*sizeof(short)); //aloc maximum posible
for(j=0;j<maxh;j++)
 {
 y=(float)j/vz;
 nrh=0;
 for(o=0;o<nrc;o++)
  {
  //find intersections (scanline)
  //up
  if(ctr[o].dir==-1)
   {
   if(y>ctr[o].pts[0].y||y<ctr[o].pts[ctr[o].nrd].y) continue; //contur doesn't intersect scanline
   for(p=1;p<=ctr[o].nrd;p++)
    {
    if(y>=ctr[o].pts[p].y&&y<=ctr[o].pts[p-1].y) //intersect with [p-1,p]
     {
     a=ctr[o].pts[p-1].y-ctr[o].pts[p].y;
     if(a)
      {
      b=ctr[o].pts[p].x-ctr[o].pts[p-1].x;
      c=ctr[o].pts[p-1].x*ctr[o].pts[p].y-ctr[o].pts[p].x*ctr[o].pts[p-1].y;
      x=-(y*b+c)/a;
      }
     else //hline
      {
      x=ctr[o].pts[p].x;
      }
     hit[nrh]=x;
     nrh++;
     break;
     }
    }
   }
  //down
  else if(ctr[o].dir==1)
   {
   if(y<ctr[o].pts[0].y||y>ctr[o].pts[ctr[o].nrd].y) continue; //contur doesn't intersect scanline
   for(p=1;p<=ctr[o].nrd;p++)
    {
    if(y<=ctr[o].pts[p].y&&y>=ctr[o].pts[p-1].y) //intersect with [p-1,p]
     {
     a=ctr[o].pts[p-1].y-ctr[o].pts[p].y;
     if(a)
      {
      b=ctr[o].pts[p].x-ctr[o].pts[p-1].x;
      c=ctr[o].pts[p-1].x*ctr[o].pts[p].y-ctr[o].pts[p].x*ctr[o].pts[p-1].y;
      x=-(y*b+c)/a;
      }
     else //hline
      {
      x=ctr[o].pts[p].x;
      }
     hit[nrh]=x;
     nrh++;
     break;
     }
    }
   }
  else
   WARN(1,"Undefined direction in contour");
  }
 //sort and trace
 if(!nrh&&!llo) //empty begining line
  flo++;
 else //trace line
  {
  pcnt=0;
  i=0;
  while(nrh>=2)
   {
   //2 min sort
   min1=hit[0]<hit[1]?0:1;
   min2=1-min1;
   for(p=2;p<nrh;p++)
    {
     if(hit[p]<hit[min2])
     {
     if(hit[p]<hit[min1])
      {
      min2=min1;
      min1=p;
      }
     else
      min2=p;
     }
    }
   //trace
   trace[tsz+1+pcnt]=hit[min1]*hz-i;
   if(trace[tsz+1+pcnt]<0) trace[tsz+1+pcnt]=0;
   i+=trace[tsz+1+pcnt];
   trace[tsz+2+pcnt]=hit[min2]*hz-i+1;
   if(trace[tsz+2+pcnt]<1) trace[tsz+2+pcnt]=1;
   i+=trace[tsz+2+pcnt];
   pcnt+=2;
   //remove mins
   o=min1<min2?min1:min2;
   p=min1<min2?min2:min1;
   MoveMemory(hit+p,hit+p+1,(nrh-p-1)*sizeof(float));
   nrh--;
   MoveMemory(hit+o,hit+o+1,(nrh-o-1)*sizeof(float));
   nrh--;
   }
  //finish up
  trace[tsz]=pcnt;
  tsz+=pcnt+1;
  if(pcnt)
   {
   llo=j;
   ltsz=tsz;
   }
  if(i>trace[2]) trace[2]=i; //find max width
  }//end else trace
 }//end for each line
free(hit);
if(llo-flo>0)
 {
 tsz=ltsz*sizeof(short);
 *((DWORD*)trace)=tsz; //first 4B (trace[0]\trace[1]) are the total size in bytes
 //trace[2]++; //ensure maxw is enough
 trace[3]=llo-flo+1; //=max height
 trace[4]=flo;
 trace=(short*)realloc(trace,tsz);
 return trace;
 }
else
 {
 free(trace);
 return NULL;
 }
}
// Contur >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif