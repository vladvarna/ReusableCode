#ifndef V_COLOR_CONVERTOR
#define V_COLOR_CONVERTOR

//Color Convertor ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct CCONV
{
public: //must keep THIS order, Info is for both modes (Tf|Ed)
 BYTE roll[4]; //B,G,R,A rolls
 DWORD mask[4]; //B,G,R,A masks
 DWORD SBpp,DBpp; //B/pixel (should be signed)
 DWORD Bpi; //B/index
 DWORD Imask; //index mask
 
 void Set(DWORD Bm=0,DWORD Gm=0,DWORD Rm=0,DWORD Am=0,BYTE Br=0,BYTE Gr=0,BYTE Rr=0,BYTE Ar=0,int sbpp=32,int dbpp=32,int bpi=0,DWORD imask=0);
//convert 
 void Tf(DWORD Spixform=0,DWORD Dpixform=0,int bpi=0,DWORD channels=0x3210);//sets up for color conversion
 DWORD BGRA(void*pc,BYTE b=0,BYTE g=0,BYTE r=0,BYTE a=0); //pc must not be NULL, but can be any memory
 void Blt(void*pdc,void*psc,NAT count=1); //pdc,psc must be at least 3B longer than required
 void Cpr(void*pdc,int dspan,void *psc,int sspan,int lng,int lat); //copy pixel rects
 void Spr(void*pdc,int dspan,int dlng,int dlat,void *psc,int sspan,int slng,int slat); //stretch pixel rects
 void Rpr(void*pdc,int dspan,int dlng,int dlat,void *psc,int sspan,int slng,int slat); //reduce pixel rects
 void Ptl(void*pdc,void *pal,void *psi,NAT count=1); //palette translation
//edit
 void Ed(DWORD,int); //sets up for editing and processing
 void Row(int,void*); //selects line
 void Pos(int,int,void*); //selects line
 void Col(BYTE*,BYTE*,BYTE*,BYTE*); //get pixel
 void Pix(BYTE,BYTE,BYTE,BYTE); //put pixel
 void Mip(void*,void*,int,int,int); //halves dimensions
}REGcc; //global register color convertor (use on inner level only)
/*
Tf mode:
 roll[ch] - numarul care trebuie folosit ca argument intr-o instructiune
rol (asm) pentru a aduce msbitul canalului 'ch', de pe pozitia din sursa
pe pozitia din destinatie
 mask[ch] - masca ce trebuie aplicata sursei(DWORD) pentru a obtine
bitii semnificativi ai destinatiei pt. canalul 'ch'
 SBpp,DBpp - Bytes per pixel pt. src & dst
 Bpi - Bytes per index (pt. LUT)
 Imask - Index mask (pt. LUT) masca care obtine indexul dintr-un DWORD

Ed mode:
 "pixel"=packed ARGB value
 roll[ch] - numarul care trebuie folosit ca argument intr-o instructiune
rol (asm) pentru a aduce msbitul canalului 'ch', de pe pozitia din pixel
pe pozitia 7; pt. a extrage valoarea canalului 'ch' intr-un BYTE; folosit
intr-o instructiune ror (asm) aduce msbitul dintr-un BYTE pe pozitia din pixel
 mask[ch] - masca ce trebuie aplicata pixelului pentru a obtine
bitii semnificativi pt. canalul 'ch'
 SBpp - pointer to begining of current line
 DBpp - Bytes per line, (row span)
 Bpi - Bytes per item (pixel)
 Imask - Negative pixel mask, masca care reseteaza bitii pixelului intr-un DWORD
*/

//copies an image optimizing for special cases ----------------------------------------------------
inline void CopyImg(void*pdi,int dspan,int dw,int dh,DWORD dpf,void*psi,int sspan,int sw,int sh,DWORD spf)
{
if(dpf==spf) //same pixel format
 {
 if(dw==sw&&dh==sh) //same size
  BltU(pdi,dspan,psi,sspan,sw,sh,ALIGN(PF_bpix(spf),7)>>3);
 else
  ZoomU(pdi,dspan,dw,dh,psi,sspan,sw,sh,ALIGN(PF_bpix(spf),7)>>3);
 }
else
 {
 REGcc.Tf(spf,dpf);
 if(dw==sw&&dh==sh) //same size
  REGcc.Cpr(pdi,dspan,psi,sspan,sw,sh);
 else
  REGcc.Spr(pdi,dspan,dw,dh,psi,sspan,sw,sh);
 }
}

//Color CONVertor <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
inline void CCONV::Set(DWORD Bm,DWORD Gm,DWORD Rm,DWORD Am,BYTE Br,BYTE Gr,BYTE Rr,BYTE Ar,int sbpp,int dbpp,int bpi,DWORD imask)
{
mask[0]=Bm; mask[1]=Gm; mask[2]=Rm; mask[3]=Am;
roll[0]=Br; roll[1]=Gr; roll[2]=Rr; roll[3]=Ar;
SBpp=ALIGN(sbpp,7)>>3;
DBpp=ALIGN(dbpp,7)>>3;
Bpi=ALIGN(bpi,7)>>3;
Imask=imask;
}

//creates masks'n'rools for color conversion ...........................................
inline void CCONV::Tf(DWORD Spixform,DWORD Dpixform,int bpi,DWORD channels) //channels contains the source channels
{
NAT Sbpc,Dbpc,Smsb,Dmsb; //bits per channel
for(int ch=0;ch<4;ch++)
 {
 Sbpc=PF_bcch(Spixform,channels&0xf);
 Dbpc=PF_bcch(Dpixform,ch);
 if(Sbpc&&Dbpc&&!(channels&0xc)) //channels low nibble < 4
  {
  Smsb=PF_msb(Spixform,channels&0x3);
  Dmsb=PF_msb(Dpixform,ch);
  mask[ch]=BitMask(Smsb,Sbpc<Dbpc?Sbpc:Dbpc);
  roll[ch]=(BYTE)(Smsb<=Dmsb?Dmsb-Smsb:32+Dmsb-Smsb);
  }
 else
  {
  mask[ch]=0;
  roll[ch]=0;
  }
 channels>>=4; //next nibble
 }
SBpp=ALIGN(PF_bpix(Spixform),7)>>3;
DBpp=ALIGN(PF_bpix(Dpixform),7)>>3;
Imask=BitMask(bpi-1,bpi);
Bpi=ALIGN(bpi,7)>>3;
}

//creates masks'n'rolls for image processing ..............................................
inline void CCONV::Ed(DWORD pixform=0,int pitch=0)
{
Bpi=PF_bpix(pixform);
for(int ch=0;ch<4;ch++)
 {
 mask[ch]=PF_mask(pixform,ch);
 if(mask[ch])
  {
  Imask=PF_msb(pixform,ch);
  roll[ch]=(BYTE)(Imask<=7?7-Imask:39-Imask);
  }
 else
  roll[ch]=0;
 }
Imask=BitMask(31,32-Bpi);
Bpi=ALIGN(Bpi,7)>>3;
DBpp=pitch; //Bytes per row
SBpp=0; //NULL
}

//concatenates B,G,R,A to form a color ...................................................
DWORD CCONV::BGRA(void *pc,BYTE b,BYTE g,BYTE r,BYTE a)
{
__asm
   {
   mov edx,[ecx]this.Bpi
   mov dh,a
   mov dl,r
   shl edx,16
   mov dh,g
   mov dl,b //edx=a|r|g|b
   mov esi,ecx //esi=this
   mov ecx,[esi] //cx=rolls
   mov eax,edx
   and eax,[esi+4] //eax&=mask[0]
   rol eax,cl 
   shr ecx,8
   mov ebx,edx
   and ebx,[esi+8] //ebx&=mask[1]
   rol ebx,cl
   shr ecx,8
   or eax,ebx //eax|=G
   mov ebx,edx
   and ebx,[esi+12] //ebx&=mask[2]
   rol ebx,cl
   shr ecx,8
   or eax,ebx //eax|=R
   and edx,[esi+16] //edx&=mask[3]
   rol edx,cl
   or eax,edx //eax|=A
   mov edi,pc
   mov [edi],eax
   }
}

//BitbLockTransfer: converts a linear buffer ...............................................................
void CCONV::Blt(void *pdc,void *psc,NAT count)
{
__asm
 {
 mov ebx,ecx //ebx=this
 mov edi,pdc
 mov esi,psc
 mov ecx,[ebx] //ecx=rolls
LNextPixel:
  mov eax,[esi] //eax=*psc
  and eax,[ebx+4] //eax&=mask[0]
  rol eax,cl //eax=B
  ror ecx,8 //cl=roll[1]
  mov edx,[esi]
  and edx,[ebx+8] //edx&=mask[1]
  rol edx,cl
  or eax,edx //eax|=G
  ror ecx,8 //cl=roll[2]
  mov edx,[esi]
  and edx,[ebx+12] //edx&=mask[2]
  rol edx,cl
  or eax,edx //eax|=R
  ror ecx,8 //cl=roll[3]
  mov edx,[esi]
  and edx,[ebx+16] //edx&=mask[3]
  rol edx,cl
  or eax,edx //eax|=A
  ror ecx,8 //cl=roll[0]
  mov [edi],eax //*pdc=eax
  add esi,[ebx]this.SBpp //psc+=SBpp
  add edi,[ebx]this.DBpp //pdc+=DBpp
  dec count
 jnz LNextPixel //loop if count>0
 }
}

//PaletteTransLate: converts paletted images to ARGB in desired format ...................................
void CCONV::Ptl(void *pdc,void *pal,void *psi,NAT count)
{
__asm
 {
 mov ebx,ecx //ebx=this
 mov edi,pdc
 mov esi,psi
 mov ecx,[ebx] //ecx=rolls
LNextPixel:
  mov eax,[esi] //eax=index
  add esi,[ebx]this.Bpi //next index
  push esi
  and eax,[ebx]this.Imask //mask index
  mul [ebx]this.SBpp //scale index
  mov esi,pal //esi=pal
  add esi,eax //esi=pal+index*SBpp
  mov eax,[esi] //eax=pal[index]
  and eax,[ebx+4] //eax&=mask[0]
  rol eax,cl //eax=B
  ror ecx,8 //cl=roll[1]
  mov edx,[esi] //edx=pal[index]
  and edx,[ebx+8] //edx&=mask[1]
  rol edx,cl
  or eax,edx //eax|=G
  ror ecx,8 //cl=roll[2]
  mov edx,[esi]
  and edx,[ebx+12] //edx&=mask[2]
  rol edx,cl
  or eax,edx //eax|=R
  ror ecx,8 //cl=roll[3]
  mov edx,[esi]
  and edx,[ebx+16] //edx&=mask[3]
  rol edx,cl
  or eax,edx //eax|=A
  ror ecx,8 //cl=roll[0]
  pop esi
  mov [edi],eax //*pdc=eax
  add edi,[ebx]this.DBpp //pdc+=DBpp
  dec count
 jnz LNextPixel //loop if count>0
 }
}

//CopyPixelRects: converts rectangular buffers ........................................
void CCONV::Cpr(void *pdc,int dspan,void *psc,int sspan,int lng,int lat)
{
__asm
 {
 mov ebx,ecx          //ebx=this
 mov ecx,[ebx]        //ecx=rolls
 mov edi,pdc
 mov esi,psc          //load pointers
 mov eax,lng
 mov edx,eax
 mov psc,eax          //use psc to save lng
 imul edx,[ebx]this.DBpp
 sub dspan,edx        //dspan-=lng*DBpp extra dst B
 imul [ebx]this.SBpp
 sub sspan,eax        //sspan-=lng*SBpp extra src B
LNextLinePix:         //label for both inner and outer loop
   mov eax,[esi]      //read pixel 1
   mov edx,eax        //read pixel 2
   and eax,[ebx+4]    //eax&=mask[0]
   rol eax,cl         //eax=B
   ror ecx,8          //cl=roll[1]
   and edx,[ebx+8]    //edx&=mask[1]
   rol edx,cl
   or eax,edx         //eax|=G
   ror ecx,8          //cl=roll[2]
   mov edx,[esi]      //read pixel 3
   and edx,[ebx+12]   //edx&=mask[2]
   rol edx,cl
   or eax,edx         //eax|=R
   ror ecx,8          //cl=roll[3]
   mov edx,[esi]      //read pixel 4
   and edx,[ebx+16]   //edx&=mask[3]
   rol edx,cl
   or eax,edx         //eax|=A
   ror ecx,8          //cl=roll[0]
   mov [edi],eax      //write pixel
   add edi,[ebx]this.DBpp//pdc+=DBpp
   add esi,[ebx]this.SBpp//next src X
   dec lng
   jnz LNextLinePix   //loop if lng>0
  add edi,dspan       //edi+=extra B
  add esi,sspan       //esi+=extra B
  mov eax,psc
  mov lng,eax         //restore lng
  dec lat
  jnz LNextLinePix    //loop if lat>0
 }
}

//StretchPixelRects: converts rectangular buffers ........................................
void CCONV::Spr(void *pdc,int dspan,int dlng,int dlat,void *psc,int sspan,int slng,int slat)
{
int loopi,hrap,vrap=0;
__asm
 {
 mov ebx,ecx          //ebx=this
 mov ecx,[ebx]        //ecx=rolls
 mov edi,pdc
 mov esi,psc          //load pointers
 mov eax,dlng
 imul [ebx]this.DBpp
 sub dspan,eax        //dspan-=dlng*DBpp extra B
 mov eax,dlat
 mov pdc,eax          //use pdc as outer loop counter
LNextDLine:
  mov hrap,0
  mov psc,esi         //save pointer to begining of line
  mov eax,dlng
  mov loopi,eax       //use as counter for inner loop
LNextDPixel:
   mov eax,[esi]      //read pixel 1
   mov edx,eax        //read pixel 2
   and eax,[ebx+4]    //eax&=mask[0]
   rol eax,cl         //eax=B
   ror ecx,8          //cl=roll[1]
   and edx,[ebx+8]    //edx&=mask[1]
   rol edx,cl
   or eax,edx         //eax|=G
   ror ecx,8          //cl=roll[2]
   mov edx,[esi]      //read pixel 3
   and edx,[ebx+12]   //edx&=mask[2]
   rol edx,cl
   or eax,edx         //eax|=R
   ror ecx,8          //cl=roll[3]
   mov edx,[esi]      //read pixel 4
   and edx,[ebx+16]   //edx&=mask[3]
   rol edx,cl
   or eax,edx         //eax|=A
   ror ecx,8          //cl=roll[0]
   mov [edi],eax      //write pixel
   add edi,[ebx]this.DBpp //pdc+=DBpp
   mov eax,hrap
   add eax,slng
   cdq
   idiv dlng
   mov hrap,edx       //hrap%=dlng
   imul [ebx]this.SBpp//eax=(hrap+slng)/dlng*SBpp
   add esi,eax        //next src X
   dec loopi
   jnz LNextDPixel    //loop if loopi>0
  add edi,dspan       //edi+=extra B
  mov esi,psc         //restore src pointer to begining of line
  mov eax,vrap
  add eax,slat
  cdq
  idiv dlat
  mov vrap,edx        //vrap%=dlng
  imul sspan          //eax=(vrap+slat)/dlat*sspan
  add esi,eax         //next src Y
  dec pdc
  jnz LNextDLine      //loop if pdc>0
 }
}

//ReducePixelRects: reduces rectangular buffers (dlng,dlat MUST BE <= slng,slat)........................................
void CCONV::Rpr(void *pdc,int dspan,int dlng,int dlat,void *psc,int sspan,int slng,int slat)
{
int loopi,loopo,hrap,vrap,blat,blng,fio;
__asm
 {
 mov ebx,ecx          //ebx=this
 mov edi,pdc
 mov esi,psc          //load pointers
 mov vrap,0
 mov eax,dlng
 imul [ebx]this.DBpp
 sub dspan,eax        //dspan-=dlng*DBpp extra B
 mov eax,dlat
 mov loopo,eax        //use as outer loop counter
LNextDLine:
  mov eax,vrap
  add eax,slat
  cdq
  idiv dlat
  mov vrap,edx        //vrap%=dlng
  mov blat,eax        //loop counter for box's outer
  fild blat           //st()=blat
  mov psc,esi         //save pointer to begining of next line
  mov hrap,0
  mov eax,dlng
  mov loopi,eax       //use as counter for inner loop
LNextDPixel:
   mov eax,hrap
   add eax,slng
   cdq
   idiv dlng
   mov hrap,edx       //hrap%=dlng
   mov blng,eax       //loop counter for box's inner
   fild blng
   fmul st,st(1)      //st()*=blng
   fldz
   fldz
   fldz
   fldz               //reset all 4 channels
LNextBoxColumn:
    mov pdc,esi       //save esi
    mov ecx,blat
LNextBoxRow:      
     mov eax,[esi]    //read pixel
     mov edx,eax
     and edx,[ebx+16] //edx&=mask[3]
     shr edx,1        //adjust for sign bit lose
     mov fio,edx
     fild fio
     faddp st(4),st   //st()+=A
     mov edx,eax
     and edx,[ebx+12] //edx&=mask[2]
     mov fio,edx
     fild fio
     faddp st(3),st   //st()+=R
     mov edx,eax
     and edx,[ebx+8]  //edx&=mask[1]
     mov fio,edx
     fild fio
     faddp st(2),st   //st()+=G
     and eax,[ebx+4]  //eax&=mask[0]
     mov fio,eax
     fild fio
     faddp st(1),st   //st()+=B
     add esi,sspan    //next row
     loop LNextBoxRow
    mov esi,pdc       //restore esi
    add esi,[ebx]this.SBpp //next col
    dec blng
    jnz LNextBoxColumn
   mov ecx,[ebx]      //ecx=rolls
   fdiv st,st(4)
   fistp fio
   mov eax,fio
   and eax,[ebx+4]    //eax&=mask[0]
   rol eax,cl         //eax=B
   fdiv st,st(3)
   fistp fio
   mov edx,fio
   and edx,[ebx+8]    //edx&=mask[1]
   ror ecx,8          //cl=roll[1]
   rol edx,cl
   or eax,edx         //eax|=G
   fdiv st,st(2)
   fistp fio
   mov edx,fio
   and edx,[ebx+12]   //edx&=mask[2]
   ror ecx,8          //cl=roll[2]
   rol edx,cl
   or eax,edx         //eax|=R
   fdiv st,st(1)
   fistp fio
   mov edx,fio
   shl edx,1          //adjust for sign bit lose
   and edx,[ebx+16]   //edx&=mask[3]
   ror ecx,8          //cl=roll[3]
   rol edx,cl
   or eax,edx         //eax|=A
   fstp fio           //clear reg
   mov [edi],eax       
   add edi,[ebx]this.DBpp //edi+=DBpp
   dec loopi
   jnz LNextDPixel    //loop if loopi>0
  mov esi,psc         //restore src pointer to begining of line
  mov eax,blat
  imul sspan          //
  add esi,eax         //esi+=blat*sspan
  add edi,dspan       //edi+=extra B
  fstp fio            //clear reg
  dec loopo
  jnz LNextDLine      //loop if loopo>0
 }
}

//MipMap: resize 4:1, slng&slat must be even (no extra B, keeps pixel format)........................................
void CCONV::Mip(void *pdc,void *psc,int sspan,int slng,int slat)
{
__asm
 {
 mov edi,pdc
 mov esi,psc          //load pointers
 shr slng,1
 shr slat,1
LNextDLine:
  mov eax,slng
LNextDPixel:
   mov ebx,[esi]      //ebx=ul pixel
   mov edx,ebx        //edx=ul
   and edx,[ecx+4]  
   mov gpr0,edx        //gpr0=Bul
   mov edx,ebx        //edx=ul
   and edx,[ecx+8]  
   mov gpr1,edx        //gpr1=Gul
   mov edx,ebx        //edx=ul
   and edx,[ecx+12] 
   mov gpr2,edx        //gpr2=Rul
   and ebx,[ecx+16]
   ror ebx,2
   mov gpr3,ebx        //gpr3=Aul
   add esi,sspan      //____________________________________
   mov ebx,[esi]      //ebx=dl pixel
   mov edx,ebx        //edx=dl
   and edx,[ecx+4]  
   add gpr0,edx        //gpr0+=Bdl
   mov edx,ebx        //edx=dl
   and edx,[ecx+8]  
   add gpr1,edx        //gpr1+=Gdl
   mov edx,ebx        //edx=dl
   and edx,[ecx+12] 
   add gpr2,edx        //gpr2+=Rdl
   and ebx,[ecx+16] 
   ror ebx,2
   add gpr3,ebx        //gpr3+=Adl
   add esi,[ecx]this.Bpi//____________________________________
   mov ebx,[esi]      //ebx=dr pixel
   mov edx,ebx        //edx=dr
   and edx,[ecx+4]  
   add gpr0,edx        //gpr0+=Bdr
   mov edx,ebx        //edx=dr
   and edx,[ecx+8]  
   add gpr1,edx        //gpr1+=Gdr
   mov edx,ebx        //edx=dr
   and edx,[ecx+12] 
   add gpr2,edx        //gpr2+=Rdr
   and ebx,[ecx+16] 
   ror ebx,2 
   add gpr3,ebx        //gpr3+=Adr
   sub esi,sspan      //____________________________________
   mov ebx,[esi]      //ebx=ur pixel
   mov edx,ebx        //edx=ur
   and edx,[ecx+4]  
   add gpr0,edx        //gpr0+=Bur
   mov edx,ebx        //edx=ur
   and edx,[ecx+8]  
   add gpr1,edx        //gpr1+=Gur
   mov edx,ebx        //edx=ur
   and edx,[ecx+12] 
   add gpr2,edx        //gpr2+=Rur
   and ebx,[ecx+16] 
   ror ebx,2 
   add gpr3,ebx        //gpr3+=Aur
   add esi,[ecx]this.Bpi //next box
   mov edx,gpr3        //rebuild in edx
   and edx,[ecx+16]   //edx=gpr3
   mov ebx,gpr2
   ror ebx,2
   and ebx,[ecx+12]
   or  edx,ebx        //edx|=gpr2
   mov ebx,gpr1
   ror ebx,2
   and ebx,[ecx+8]
   or  edx,ebx        //edx|=gpr1
   mov ebx,gpr0
   ror ebx,2
   and ebx,[ecx+4]
   or  edx,ebx        //edx|=gpr0
   mov [edi],edx      //write pixel
   add edi,[ecx]this.Bpi //edi+=Bpi
   dec eax
   jnz LNextDPixel    //loop if slng>0
  add esi,sspan       //jump an aditional src line
  dec slat
  jnz LNextDLine      //loop if slat>0
 }
}

//moves the row pointer (SBpp) absolute or relative(if !base) ...................................
void CCONV::Row(int row=1,void*base=NULL)
{
__asm
 {
 mov eax,row
 imul [ecx]this.DBpp //eax=row*Bpr
 mov edx,base //must be after the imul !
 test edx,edx
 jnz LAbsolute //if(base) move abs
 mov edx,[ecx]this.SBpp //else if(!base) edx=current row pointer
LAbsolute:
 add edx,eax
 mov [ecx]this.SBpp,edx //SBpp=new row pointer
 }
}

//moves the  pointer (SBpp) absolute or relative(if !base) ...................................
void CCONV::Pos(int x=0,int y=0,void*base=NULL)
{
__asm
 {
 mov eax,x
 imul [ecx]this.Bpi //eax=x*Bpi
 mov edx,y
 imul edx,[ecx]this.DBpp //edx=y*Bpl
 add eax,edx //edx=y*Bpl
 mov edx,base //must be after the imul !
 test edx,edx
 jnz LAbsolute //if(base) eax=base+y*Bpl+x*Bpi
 add eax,[ecx]this.SBpp //else if(!base) eax=cp+y*Bpl+x*Bpi
LAbsolute:
 add eax,edx
 mov [ecx]this.SBpp,eax //SBpp=new row pointer
 }
}

//get pixel .............................................................................
void CCONV::Col(BYTE*b,BYTE*g,BYTE*r,BYTE*a)
{
__asm
 {
 mov esi,ecx //esi=this
 mov ecx,[esi] //ecx=rolls
 mov edx,[esi]this.SBpp //edx=pointer to pixel
 mov eax,[edx] //eax=pixel
 add edx,[esi]this.Bpi //x++
 mov [esi]this.SBpp,edx //save new position
 mov edx,eax //save pixel
 and edx,[esi+4] //mask B
 rol edx,cl //to BYTE
 shr ecx,8 //next roll
 mov edi,b
 mov [edi],edx //*b=edx
 mov edx,eax
 and edx,[esi+8] //mask G
 rol edx,cl //to BYTE
 shr ecx,8 //next roll
 mov edi,g
 mov [edi],edx //*g=edx
 mov edx,eax
 and edx,[esi+12] //mask R
 rol edx,cl //to BYTE
 shr ecx,8 //next roll
 mov edi,r
 mov [edi],edx //*r=edx
 mov edx,eax
 and edx,[esi+16] //mask A
 rol edx,cl //to BYTE
 mov edi,a
 mov [edi],edx //*a=edx
 }
}

//put pixel .............................................................................
void CCONV::Pix(BYTE b=0,BYTE g=0,BYTE r=0,BYTE a=0xff)
{
__asm
 {
 mov esi,ecx //esi=this
 mov ecx,[esi] //ecx=rolls
 mov edi,[esi]this.SBpp //edi=pointer to pixel 'x' on current row
 mov eax,[edi] //eax=pixel
 and eax,[esi]this.Imask //clear this pixel's bits
 mov dl,b
 ror edx,cl
 shr ecx,8
 and edx,[esi+4] //edx&=mask[0]
 or eax,edx
 mov dl,g
 ror edx,cl
 shr ecx,8
 and edx,[esi+8] //edx&=mask[1]
 or eax,edx
 mov dl,r
 ror edx,cl
 shr ecx,8
 and edx,[esi+12] //edx&=mask[2]
 or eax,edx
 mov dl,a
 ror edx,cl
 and edx,[esi+16] //edx&=mask[3]
 or eax,edx
 mov [edi],eax //write pixel
 add edi,[esi]this.Bpi //x++
 mov [esi]this.SBpp,edi //save new position
 }
}
//CCONV >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#endif