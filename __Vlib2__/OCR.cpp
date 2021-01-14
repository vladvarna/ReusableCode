//VERSION 1.0.a
#pragma once

#include <img.cpp>

#define SYNAPSE_PREC float   //data type used for neuron weights
#define OCRSOM_SAVECOMMON		48  //4*4+4*8= how much to save from OCRSOM
#define OCRSOM_SAVENEURON		28  //4+3*8= how much to save from OCR_NEURON_O

//status
#define OCRSOM_EXTERNALDRAW  			0x1 //window is used by external
#define OCRSOM_SHOWSCORES    			0x2 //scores for activ

//OCR Self Organizing Map (Kohonen)
class OCRSOM
{
public:
 ASCIISET chset; //dynamic mask
 DWORD inputw,inputh; //input image width and height
 DWORD mincode,maxcode; //ASCII range
 NAT synapses,neurons; //synapses=inputw*inputh, neurons=maxcode-mincode+1
 NAT nrchars;  //=maxcode-mincode+1
 char wild_d,wild_l,wild_a,warn_c;
 struct OCR_NEURON_I
  {
  NAT windot,windif; //last winning neuron (not ASCII)
  double score,dot,dif; //last scores
  char estim,mask; //estimated char
  SYNAPSE_PREC*w; //synapses
  Image img;
  }*input;
 struct OCR_NEURON_O
  {
  DWORD trained; //how many times was trained
  double failed,totalruns; //total no. of failures/runs 
  double weight; //abs sum of elements
  double dot,dif;
  SYNAPSE_PREC*w;
  int i,j; //coordinates in nnimg
  }*output;
 BITS32 stat;
 HWND ownd;
 RCT owr; //ocr window client rect
 Image nnimg;
 int ncols,nrows; //neural image number of columns and rows
 float cellw,cellh;
 RCT nnr; //rect for nnimg
 HFONT hfnt1,hfnt2; 
 int activ;
 
OCRSOM() { ZEROCLASS(OCRSOM); }
void Init(NAT,NAT,char,char);
void Set(char,char,char,char);
void Save(LPSTR);  //save weights to file
BOOL Load(LPSTR);  //load weights to file
void Inputs(Image*,RCT*,char*,NAT,COLOR,int,int);
void NeuronFromImg(SYNAPSE_PREC*,Image*,Image*,RCT,COLOR,int,int);
char RunDot(NAT);
char RunDif(NAT);
void RunAll();
void GetStr(char*);
void Train(NAT,NAT,double);
void TrainAll(char*,double);
void Free();
void DrawInImage(SYNAPSE_PREC*,Image*,int,int);
void ToImage(Image*,int);
void Resize();
void wmChar(char);
void wmMouse(int,int,WPARAM);
void Draw(HDC);
~OCRSOM() { Free(); }
void Hwnd(HWND);	//draw weights to image
//debug
void ShowNeuron(HWND,SYNAPSE_PREC*);	//draw weights to image
//void FontToW(HFONT);  //ideal weights based on font
};

//..................................................................................................
void OCRSOM::Free()
{
NAT n;
DeleteObject(hfnt1);
DeleteObject(hfnt2);
for(n=0;n<nrchars;n++)
 {
 FREE(input[n].w);
 input[n].img.Free();
 }
FREE(input);
for(n=0;n<neurons;n++)
 {
 FREE(output[n].w);
 }
FREE(output);
nrchars=0;
neurons=0;
nnimg.Free();
}

//.........................................................................
void OCRSOM::Init(NAT w,NAT h,char a=' ',char b='\127')
{
Free();
//error("OCRSOM::Init()");
inputw=w;
inputh=h;
mincode=a;
maxcode=b;
synapses=inputw*inputh;
neurons=maxcode-mincode+1;
output=(OCR_NEURON_O*)ALLOC(neurons*sizeof(OCR_NEURON_O));
ZeroMemory(output,neurons*sizeof(OCR_NEURON_O));
for(NAT n=0;n<neurons;n++)
 {
 ifn(chset[n+mincode])
  continue;
 output[n].w=(SYNAPSE_PREC*)ALLOC(synapses*sizeof(SYNAPSE_PREC));
 ZeroMemory(output[n].w,synapses*sizeof(SYNAPSE_PREC));
 }
ToImage(&nnimg,0);
InvalidateRect(ownd,NULL,0);
}

//......................................................................................
void OCRSOM::Set(char wd='#',char wl='?',char wa='@',char wc='!')
{
wild_d=wd;
wild_l=wl;
wild_a=wa;
warn_c=wc;
}

//Save weights ......................................................................................
void OCRSOM::Save(LPSTR path)
{
IOSFile iof;
changefileext(path,"nnt");
if(iof.open(path,FU_WO))
 return;
iof.write(this,OCRSOM_SAVECOMMON);
for(NAT n=0;n<neurons;n++)
 {
 ifn(chset[n+mincode])
  continue;
 iof.write(output+n,OCRSOM_SAVENEURON);
 iof.write(output[n].w,synapses*sizeof(SYNAPSE_PREC));
 }
}

//Save weights .......................................................................................
BOOL OCRSOM::Load(LPSTR path)
{
IOSFile iof;
NAT n;
changefileext(path,"nnt");
if(iof.open(path,FU_R))
 return 0;
Free();
iof.amask=1;
iof.read(this,OCRSOM_SAVECOMMON);
synapses=inputw*inputh;
neurons=maxcode-mincode+1;
//chset.DBG();
output=(OCR_NEURON_O*)ALLOC(neurons*sizeof(OCR_NEURON_O));
ZeroMemory(output,neurons*sizeof(OCR_NEURON_O));
for(n=0;n<neurons;n++)
 {
 ifn(chset[n+mincode])
  continue;
 iof.read(output+n,OCRSOM_SAVENEURON);
 output[n].w=(SYNAPSE_PREC*)ALLOC(synapses*sizeof(SYNAPSE_PREC));
 iof.read(output[n].w,synapses*sizeof(SYNAPSE_PREC));
 }
ToImage(&nnimg,0);
InvalidateRect(ownd,NULL,0);
return 1;
}

//(pimg should be 0x32108888).........................................................................
void OCRSOM::NeuronFromImg(SYNAPSE_PREC*layer,Image*pdst,Image*psrc,RCT cell,COLOR thres,int flags,int ch=0)
{
NAT s,l;
pdst->CloneCh(psrc,cell.l,cell.u,cell.r,cell.d,ch);
pdst->Resize(inputw,inputh,0);
if(flags&1)
 pdst->Mod(IMG_MOD_AUTO_CONTRAST,0.,0);
//img.FitWindow(hdbgwnd);
//HDC htdc=GetDC(hdbgwnd);
//img.DrawDC(htdc);
//ReleaseDC(hdbgwnd,htdc);
//WaitVKey();

NAT OT=pdst->Threshold(IMG_THRESHOLD_OTSU);
thres.c1=0;
thres.c2=OT;
thres.c3=OT+1;
thres.c4=255;

l=0;
for(s=0;s<synapses;s++)
 {
 if(pdst->imgB[l]<=thres.c1)		  //background
  layer[s]=-1.;
 else if(pdst->imgB[l]<=thres.c2)   //background
  layer[s]=((double)(pdst->imgB[l]-thres.c1)/(thres.c2-thres.c1+1))-1.;
 else if(pdst->imgB[l]>=thres.c4)   //foreground
  layer[s]=1.;
 else if(pdst->imgB[l]>=thres.c3)   //foreground
  layer[s]=(double)(pdst->imgB[l]-thres.c3)/(thres.c4-thres.c3+1);
 else
  layer[s]=0.;
 l++;
 }
}

//................................................................................................
void OCRSOM::Inputs(Image*psrc,RCT*cells,char*lmask,NAT nc,COLOR thres,int flags,int ch=0)
{
NAT c;
for(c=0;c<nrchars;c++)
 {
 FREE(input[c].w);
 input[c].img.Free();
 }
FREE(input);
nrchars=nc;
input=(OCR_NEURON_I*)ALLOC(nrchars*sizeof(OCR_NEURON_I));
ZeroMemory(input,nrchars*sizeof(OCR_NEURON_I));
for(c=0;c<nrchars;c++)
 {
 input[c].w=(SYNAPSE_PREC*)ALLOC(synapses*sizeof(SYNAPSE_PREC));
 NeuronFromImg(input[c].w,&input[c].img,psrc,cells[c],thres,flags,ch);
 input[c].mask=lmask[c];
 //ShowNeuron(hdbgwnd,input[c].w);
 //WaitVKey();
 }
activ=-1;
Resize();
InvalidateRect(ownd,NULL,0);
}

//.................................................................................................
void OCRSOM::DrawInImage(SYNAPSE_PREC*layer,Image*pdst,int i,int j)
{
NAT x,y,p,s=0;
if(!pdst->img) return;
if(i>pdst->lng-inputw) return;
if(j>pdst->lat-inputh) return;
for(y=0;y<inputh;y++)
 {
 p=(y+j)*pdst->Bpl+i*pdst->Bpp;
 for(x=0;x<inputw;x++)
  {
  if(layer[s]>0.) //green
   {
   pdst->imgB[p]=0;
   pdst->imgB[p+1]=layer[s]*255;
   pdst->imgB[p+2]=0;
   }
  else if(layer[s]<0.) //red
   {
   pdst->imgB[p]=0;
   pdst->imgB[p+1]=0;
   pdst->imgB[p+2]=-layer[s]*255;
   }
  else //blue
   {
   pdst->imgB[p]=0xff;
   pdst->imgB[p+1]=0;
   pdst->imgB[p+2]=0;
   }
  p+=pdst->Bpp;
  s++;
  }
 }
}
   
//.................................................................................................
void OCRSOM::ToImage(Image*pdst,int nrc=0)
{
NAT n;
int i,j;
if(nrc)
 ncols=nrc;
else
 ncols=ceil(SQRT(2.*chset.NC()));
nrows=ceil((float)chset.NC()/ncols);
pdst->Init(ncols*(inputw+1),nrows*(inputh+1),0x32100888,0,NULL,0,0,"neural image");
pdst->Clear(0xff); //Blue
i=j=0;
for(n=0;n<neurons;n++)
 {
 ifn(chset[n+mincode])
  continue;
 output[n].i=i*(inputw+1);
 output[n].j=j*(inputh+1);
 DrawInImage(output[n].w,pdst,output[n].i,output[n].j);
 i++;
 if(i>=ncols)
  {
  i=0;
  j++;
  }
 }
}

//should call InputImg first .......................................................................
char OCRSOM::RunDot(NAT in=0)
{
NAT n,s,winner=0;
if(in>=nrchars) return warn_c;
for(n=0;n<neurons;n++)
 {
 ifn(chset[n+mincode])
  continue;
 output[n].totalruns++;
 output[n].dot=0.;
 for(s=0;s<synapses;s++)
  {
  output[n].dot+=input[in].w[s]*output[n].w[s];
  }
 if(output[n].dot>output[winner].dot)
  winner=n;
 }
input[in].windot=winner;
input[in].dot=output[winner].dot;
return winner+mincode;
}

//should call InputImg first ......................................................................
char OCRSOM::RunDif(NAT in)
{
NAT n,s,winner=0;
if(in>=nrchars) return warn_c;
for(n=0;n<neurons;n++)
 {
 ifn(chset[n+mincode])
  continue;
 output[n].dif=0.;
 for(s=0;s<synapses;s++)
  {
  output[n].dif+=(input[in].w[s]-output[n].w[s])*(input[in].w[s]-output[n].w[s]);
  }
 if(output[n].dif<output[winner].dif)
  winner=n;
 }
input[in].windif=winner;
input[in].dif=output[winner].dif;
return winner+mincode;
}

//should call InputImg first (n should be the winner)..............................................
void OCRSOM::RunAll()
{
NAT c;
for(c=0;c<nrchars;c++)
 {
 RunDot(c);
 RunDif(c);
 if(input[c].windot!=input[c].windif)
  {
  input[c].estim=input[c].windif+mincode;
  input[c].score=-1.;
  }
 else
  {
  input[c].estim=input[c].windot+mincode;
  input[c].score=(1.-input[c].dif/synapses)*100.;
  //input[c].score=input[c].dot*100/output[input[c].windot].weight;
  //if(input[c].score<0.) input[c].score=0.;
  //if(input[c].score>100.) input[c].score=100.;
  }
 if(input[c].mask==wild_d)
  {
  if(input[c].estim>'9'||input[c].estim<'0')
   {
   input[c].estim=wild_d;
   input[c].score=-2.;
   }
  } 
 else if(input[c].mask==wild_l)
  {
  if(input[c].estim>'Z'||input[c].estim<'A')
   {
   input[c].estim=wild_l;
   input[c].score=-3.;
   }
  } 
 else if(input[c].mask!=wild_a)
  {
  input[c].estim=input[c].mask;
  input[c].score=101.;
  }
 }
}

//should call InputImg first (n should be the winner)..............................................
void OCRSOM::GetStr(char*estimat)
{
NAT c;
for(c=0;c<nrchars;c++)
 estimat[c]=input[c].estim;
estimat[c]=0;
}

//should call Inputs first (n should be the winner)..............................................
void OCRSOM::Train(NAT in,NAT out,double rate=1.)
{
NAT s;
ifn(chset[out])
 return;
if(out>=mincode&&out<=maxcode)
 out-=mincode;
else
 return;
if(in>=nrchars)
 return;
if(input[in].windot!=out)
 output[input[in].windot].failed++;
output[out].trained++;
if(rate<0)
 rate=1./(output[out].trained);
output[out].weight=0.;
for(s=0;s<synapses;s++)
 {
 output[out].w[s]+=rate*(input[in].w[s]-output[out].w[s]);
 output[out].weight+=ABS(output[out].w[s]);
 }
DrawInImage(output[out].w,&nnimg,output[out].i,output[out].j);
InvalidateRect(ownd,NULL,0);
}

//should call InputImg first (n should be the winner)..............................................
void OCRSOM::TrainAll(char*corect,double rate=-1.)
{
for(int c=0;c<nrchars;c++)
 {
 if(chset[corect[c]])
  {
  Train(c,corect[c],rate);
  //ShowNeuron(hdbgwnd,output[corect[c]-mincode].w);
  //WaitVKey();
  }
 }
}

#ifdef _DEBUG
//..................................................................................................
void OCRSOM::ShowNeuron(HWND hwnd,SYNAPSE_PREC*layer)
{
NAT s=0;
HDC hldc;
if(!layer) return;
AdjustClientRect(hdbgwnd,inputw,inputh);
hldc=GetDC(hwnd);
for(int y=0;y<inputh;y++)
 for(int x=0;x<inputw;x++)
  {
  if(layer[s]>0.)
   SetPixelV(hldc,x,y,RGB(0,layer[s]*255,0));
  else if(layer[s]<0.)
   SetPixelV(hldc,x,y,RGB(-layer[s]*255,0,0));
  else
   SetPixelV(hldc,x,y,RGB(0,0,255));
  s++;
  }
ReleaseDC(hwnd,hldc);
}

#endif

//score should be [0%,100%] -----------------------------------------------------------------
inline DWORD OCRColorFromScore(double score)
{
if(score<0.)
 return 0xff;  //RED
if(score>100.)
 return 0xffff00; //CYAN
score=CLAMP(score/100.,0.,1.); 
if(score>0.5)
 return ((NAT)(score*255)<<8);
else
 return ((NAT)((1.-score)*255)<<8)|((NAT)((1.-score)*255)); 
}

//.................................................................................................
void OCRSOM::Resize()
{
cellw=(float)(owr.r-owr.l)/nrchars;
cellh=cellw*inputh/inputw; //keep aspect ratio
DeleteObject(hfnt1);
DeleteObject(hfnt2);
hfnt1=CreateFont(-cellh,cellw,0,0,800,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FF_DONTCARE|FIXED_PITCH,NULL);
hfnt2=CreateFont(cellh/4,cellw/6,0,0,300,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FF_DONTCARE|DEFAULT_PITCH,"Arial Narrow");
nnr(owr.l,owr.u+cellh*3,owr.r,owr.d); //neural network rectangle
}

//.................................................................................................
void OCRSOM::wmChar(char key)
{
if(activ>=0&&activ<nrchars)
 {
 input[activ].estim=key;
 input[activ].score=103.;
 for(;activ<nrchars;activ++) //select next invalid
  {
  if(input[activ].score<50.)
   break;
  } 
 InvalidateRect(ownd,NULL,0);
 }
}

//.................................................................................................
void OCRSOM::wmMouse(int mx,int my,WPARAM wparam)
{
int i,j,t;
NAT n;
char lsBuf[16];
double learnrate;
if(nnr.inc(mx,my)) 
 {
 i=(mx-nnr.l)*ncols/(nnr.r-nnr.l);
 j=(my-nnr.u)*nrows/(nnr.d-nnr.u);
 t=j*ncols+i;
 for(n=0;n<=t;n++)
  {
  ifn(chset[n+mincode]) t++;
  if(t>=neurons) return;
  }
 if(wparam&MK_LBUTTON)
  {
  if(activ<0) return;
  input[activ].estim=t+mincode;
  input[activ].score=102.;
  InvalidateRect(ownd,NULL,0);
  }
 else if(wparam&MK_RBUTTON)
  {
  sc(lsBuf,"-1");
  ifn(InputBox(lsBuf,"Learn rate:",0,ownd,0))
   return;
  learnrate=atof(lsBuf);
  if(wparam&MK_CONTROL)
   {
   GetStr(lsBuf);
   TrainAll(lsBuf);
   }
  else
   {  
   if(activ<0) return;
   Train(activ,t+mincode,learnrate);
   }
  RunAll();
  InvalidateRect(ownd,NULL,0);
  }
 else if(wparam&MK_MBUTTON)
  {
  stat^=OCRSOM_SHOWSCORES;
  InvalidateRect(ownd,NULL,0);
  }
 }
else 
 {
 activ=(mx-owr.l)/cellw;
 RunDot(activ);
 RunDif(activ);
 InvalidateRect(ownd,NULL,0);
 }
}

//.................................................................................................
void OCRSOM::Draw(HDC hdc)
{
//owr=ownd;
NAT c,l,n;
char lsbuf[256];
RCT lr;
HFONT hfnt;
float i,j,cx,cy;
if(!nrchars) return;
SetStretchBltMode(hdc,HALFTONE);
nnimg.ShowDC(hdc,owr.l,owr.u+cellh*3,owr.r,owr.d);
SetBkMode(hdc,TRANSPARENT);
SetTextAlign(hdc,TA_CENTER|TA_TOP);
for(c=0;c<nrchars;c++)
 {
 input[c].img.ShowDC(hdc,c*cellw,0,(c+1)*cellw,cellh);
 }
BitBlt(hdc,owr.l,owr.u+cellh,owr.r-owr.l,cellh*2,NULL,0,0,BLACKNESS);
if(activ>=0&&GetFocus()==ownd)
 {
 SelectObject(hdc,GetStockObject(DKGRAY_BRUSH));
 RoundRect(hdc,activ*cellw,cellh+1,(activ+1)*cellw,cellh*2,cellw/3,cellh/3);
 }
hfnt1=(HFONT)SelectObject(hdc,hfnt1);
for(c=0;c<nrchars;c++)
 {
 SetTextColor(hdc,OCRColorFromScore(input[c].score));
 TextOut(hdc,(c+0.5)*cellw,cellh,(char*)&input[c].estim,1);
 }
hfnt1=(HFONT)SelectObject(hdc,hfnt1);
lr(0,cellh*2,cellw,cellh*3);
SetTextAlign(hdc,TA_LEFT|TA_TOP|TA_NOUPDATECP);
hfnt2=(HFONT)SelectObject(hdc,hfnt2);
for(c=0;c<nrchars;c++)
 {
 l=sprintf(lsbuf,"%.f%%\n%c\n%c",input[c].score,input[c].windot+mincode,input[c].windif+mincode);
 SetTextColor(hdc,OCRColorFromScore(input[c].score));
 DrawText(hdc,lsbuf,l,lr,DT_CENTER);
 lr>>cellw;
 }
hfnt2=(HFONT)SelectObject(hdc,hfnt2);
if(stat&OCRSOM_SHOWSCORES)
 {
 cx=nnr.W()/(float)ncols;
 cy=nnr.H()/(float)nrows;
 hfnt=CreateFont(cy/4,cx/8,0,0,300,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FF_DONTCARE|DEFAULT_PITCH,"Arial Narrow");
 hfnt=(HFONT)SelectObject(hdc,hfnt);
 SetBkMode(hdc,OPAQUE);
 SetBkColor(hdc,0x3f3f3f);
 SetTextColor(hdc,0xffffff);
 lr(0,nnr.u,cx,nnr.u+cy);
 c=0;
 for(n=0;n<neurons;n++)
  {
  ifn(chset[n+mincode])
   continue;
  l=sprintf(lsbuf,"%.f\n%.f",output[n].dot,output[n].dif);
  DrawText(hdc,lsbuf,l,lr,DT_LEFT);
  lr>>cx;
  c++;
  if(c>=ncols)
   {
   c=0;
   lr.l=0;
   lr.r=cx;
   lr^(-cy);
   }
  }
 DeleteObject(SelectObject(hdc,hfnt));
 }
}

//.................................................................................................
void OCRSOM::Hwnd(HWND hwnd)
{
ownd=hwnd;
SetWindowLong(ownd,GWL_USERDATA,(long)this);
owr=ownd;
}

///////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK OCRWndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{                         
OCRSOM*pOCR;
HDC hldc;
RCT lr;
pOCR=(OCRSOM*)GetWindowLong(hwnd,GWL_USERDATA);
if(!pOCR)
 return DefWindowProc(hwnd,message,wparam,lparam);
switch(message)
 {
 case WM_CHAR:
  UPcase((char*)&wparam,1);
  pOCR->wmChar(wparam);
  return 0;
 case WM_MBUTTONDOWN:
 case WM_RBUTTONDOWN:
 case WM_LBUTTONDOWN:
  pOCR->wmMouse(LOWORD(lparam),HIWORD(lparam),wparam);
  return 0;
 case WM_MOUSEMOVE:
  SetFocus(hwnd);
  break; 
 case WM_PAINT:
   if(pOCR->stat&OCRSOM_EXTERNALDRAW) break;
   hldc=GetDC(hwnd);
   pOCR->Draw(hldc);
   ReleaseDC(hwnd,hldc);
  //ValidateRect(hwnd,NULL);
  break;
//_______________________________________________________________________________________________
 case WM_ERASEBKGND:
  return 0;
 case WM_SIZE:
  pOCR->Resize();
  break;
 //case WM_DESTROY:
 // return 0;
 }
return DefWindowProc(hwnd,message,wparam,lparam);
}

