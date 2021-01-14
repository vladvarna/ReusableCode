#ifndef V_HUFFMAN
#define V_HUFFMAN

//this also includes the DCPR_HUFF
#define DCPR_HUFF_A         0x0101 //Huffman Byte compression with freq table in header

//Node in the Encode Tree
struct HTreeEnc
{
 WORD parent; //positive relative offset in B to parent node from this node; if(NULL) node is still in list
 WORD flags; //b31-0=left/1=right descendant
 DWORD freq; //nr of occurences; if <=0 node not in list
};

//Node in the Decode Tree
struct HTreeDec
{
 WORD ldesc; //negative relative offset in B to left descendant of this node; if(NULL) node is leaf
 WORD rdesc; //negative relative offset in B to right descendant of this node; if(NULL) node is leaf
 DWORD freq; //nr of occurences; if <=0 node not in list
};

/*
LIMITATIONS:
 -any one char freq can't be more than 2^31-1
 -source (and destination implicitly) data buffer size can't be more than 2^32
 -source must be at least 2 B long 
*/

//counts data freq and records in tree --------------------------------------------------------------------------
//ptree should be 8*511=4088 bytes initialized to 0 for the first call
void HCountFreq1(void*udata,NAT udataB,HTreeEnc*ptree)
{
__asm
 {
 //count frequencies
 mov esi,udata
 mov ecx,udataB
 mov edx,ptree
 add edx,4 //select freq member
Lloop1:
  xor eax,eax
  lodsb	 //al=[esi]
  shl eax,3	//eax*=sizeof(HTreeEnc)
  add eax,edx //eax=&ptree[eax]
  inc dword ptr[eax]
  loop Lloop1
 }
}  

//counts data freq with delta and records in tree --------------------------------------------------------------------------
//ptree should be 8*511=4088 bytes initialized to 0 for the first call
void HCountFreq2(void*udata,NAT udataB,HTreeEnc*ptree,NAT gran=1)
{
__asm
 {
 mov esi,udata
 mov edx,udataB
 mov ecx,edx
 mov edi,ptree
 add edi,4 //select freq member
 mov ebx,gran
 cmp ebx,0
 je Lloop0 //if(gran==0) count udataB with no delta
 cmp edx,ebx
 jbe Lloop0 //if(gran<=udataB) count udataB with no delta
 mov ecx,ebx //else count gran with no delta
Lloop0:  //count first <=gran Bytes
  xor eax,eax
  lodsb	 //al=[esi]
  shl eax,3	//eax*=sizeof(HTreeEnc)
  add eax,edi //eax=&ptree[eax]
  inc dword ptr[eax]
  loop Lloop0
 cmp ebx,0
 je LReturn //if(gran==0) done
 sub edx,ebx //udataB-=gran
 jbe LReturn //if(udataB<=gran) done
 mov ecx,edx
 mov edx,esi
 sub edx,ebx //edx=esi-gran
LLoop1: //count remaining bytes with granularity
  xor eax,eax
  lodsb
  sub al,[edx]
  inc edx
  shl eax,3	//eax*=sizeof(HTreeEnc)
  add eax,edi //eax=&ptree[eax]
  inc dword ptr[eax]
  loop Lloop1
LReturn:
 }
}

//builds a huffman encoding tree from data --------------------------------------------------------------------------
NAT HBuildTreeEnc1(HTreeEnc*ptree)
{
int min2,NrNodes;
__asm
 {
 mov edi,ptree
 add edi,4 //select freq member
 mov NrNodes,256 
Lloop2:
  mov edx,0xffffffff //edx=min1 index
  mov esi,0xffffffff //esi=min1
  mov ebx,0xffffffff //ebx=min2 index
  mov min2,0xffffffff
  xor ecx,ecx //ecx is node counter
  push edi
Lloop3:	//find mins
   mov eax,[edi] //eax=ptree[ecx].freq
   cmp eax,0
   jle Lnextnode //if(<=0)	not in list
   cmp eax,esi
   jb Lnewmin1 	//if(eax<min1)
   cmp eax,min2
   jae Lnextnode //if(eax>=min2)
   mov min2,eax //min2=ptree[ecx].freq
   mov ebx,ecx //min2ind=ecx
   jmp Lnextnode
Lnewmin1:
   mov min2,esi //min2=min1
   mov esi,eax	//min1=ptree[ecx].freq
   mov ebx,edx //min2ind=min1ind
   mov edx,ecx //min1ind=ecx
Lnextnode:
   add edi,8 //edi+=sizeof(HTreeEnc)
   inc ecx
   cmp ecx,NrNodes
   jb Lloop3 //loop if(ecx<NrNodes)
  pop edi
  cmp ebx,0xffffffff
  je Lbreakloop2 //break if not found two mins (only the root left)
  //create node
  add esi,min2 //esi=min1+min2
  shl ecx,3 //*=sizeof(HTreeEnc)
  mov eax,ecx //eax=NrNodes*sizeof(HTreeEnc)
  add eax,edi //eax=&ptree[NrNodes]
  mov [eax],esi	//ptree[NrNodes].freq=min1+min2
  mov esi,ecx
  inc NrNodes //add new node
  //make the created node parent for min1
  shl edx,3 //*=sizeof(HTreeEnc)
  mov eax,edx
  add eax,edi
  or dword ptr[eax],0x80000000	//remove from list
  sub esi,edx //esi=(NrNodes-min1ind)*8
  //and esi,0xffff  //ptree[min1ind].flags=0
  mov [eax-4],esi //add parent and flags members
  //make the created node parent for min2
  shl ebx,3 //*=sizeof(HTreeEnc)
  mov eax,ebx
  add eax,edi
  or dword ptr[eax],0x80000000	//remove from list
  sub ecx,ebx //ecx=(NrNodes-min2ind)*8
  or ecx,0x80000000 //make min2 the right node
  //and ecx,0x8000ffff  //ptree[min2ind].flags=1
  mov [eax-4],ecx //add parent and flags members
  jmp Lloop2 //loop if found mins
Lbreakloop2:
LReturn:
 mov eax,NrNodes //return nr of nodes in tree
 }
}

//builds an encode table, with the nr of bits per char, from an encode tree ---------------------------------------------------------------
NAT HBuildTableEnc1(HTreeEnc*ptree,WORD*ptable1,__int64*pDTPszB)
//ptable should be 512 bytes
{
int contor,totbitsL,totbitsH;
__asm
 {
 mov esi,ptree
 mov edi,ptable1
 mov contor,256
 xor ecx,ecx //ecx will hold max MSB position
 mov totbitsL,ecx //=0
 mov totbitsH,ecx //=0
Lloop1:
  mov edx,[esi] //edx=ptree[ch].flags|parent
  mov ebx,esi
  xor eax,eax //eax=nr of bits
Lloop2:
   and edx,0xffff
   jz LFoundRoot
   inc eax
   add ebx,edx
   mov edx,[ebx] //edx=[ebx]->flags|parent
   jmp Lloop2
LFoundRoot:
  stosw	//ptable[ecx]=ax
  add esi,4	//select .freq member
  and dword ptr[esi],0x7fffffff //restore all frequencies (reset MSBit who is used in tree creation)
  mov ebx,[esi] //edx=ptree[ch].freq
  mov edx,ebx
  mul edx //edx|eax=code_length*frequency
  add totbitsL,eax
  adc totbitsH,edx
  bsr eax,ebx //eax=MSB freq
  jz LIsSmaller	//if(freq==0) skip
  cmp eax,ecx
  jb LIsSmaller	//if(eax<ecx) don't update
  mov ecx,eax
LIsSmaller:
  add esi,4 //next char
  dec contor
  jnz Lloop1
 mov eax,totbitsL
 mov edx,totbitsH  //(totbitsH:totbitsL)= +: 0<=i<=255 {ptable[i]*ptree[i].freq} (a se citi "suma dupa i din {}")
 test eax,0x1f
 jz LExact1 //already divisible by 32
 add eax,32 //inc doesn't update CF
 adc edx,0
 and eax,0xffffffe0 //round_up(totbitsH:totbitsL) to 32 bits
LExact1:
 shrd eax,edx,3
 shr edx,3 //(totbitsH:totbitsL)/=8 to B
 inc ecx //Nr of bits=MSBit position + 1
 mov ebx,ecx
 shl ebx,5 //ebx=freqszb*256/8=freqszb*32 (because is a multiple of 32 is guaranteed to be DWORD aligned)
 add ebx,24 //add space for: TAG2, TAG3, DTB1 size, freqszb and DTB2 size, DTB2 original size(ebx now contains all the overhead)
 add eax,ebx
 adc edx,0 //add it to (totbitsH:totbitsL)
 mov edi,pDTPszB
 mov [edi],eax
 mov [edi+4],edx //return number of B after compression in *pcprszB
 mov eax,ecx //return freqszb=maximum number of bits required to reprezent frequency
 }
}

//builds an encode table, with the code for each char, from an encode tree and a b/char table---------------------------------------------------------------
void HBuildTableEnc2(HTreeEnc*ptree,WORD*ptable1,BYTE*ptable2)
//ptable2 should be 32*256=8192 not necessary initialized to all 0
{
int contor;
__asm
 {
 mov edi,ptable2
 mov esi,ptable1
 mov ebx,ptree
 mov contor,256
Lloop1:
  mov ecx,[esi]	
  and ecx,0xffff //ecx=(WORD)ptable1[contor]=code length in bits
  jecxz LNextChar //ATEN.: this jump could be >128
  mov eax,ebx
Lloop2:
   dec ecx //ecx=MSBit in code
   mov edx,[eax] //next node
   test edx,0x80000000 //mask
   jz LResetIt
   bts [edi],ecx //1
   jmp LNextBit
LResetIt:
   btr [edi],ecx //0
LNextBit:
   and edx,0xffff //mask
   add eax,edx //build pointer
   jecxz LNextChar
   jmp Lloop2 //loop if(ecx>0)
LNextChar:
  add edi,32
  add esi,2
  add ebx,8
  dec contor
  jnz Lloop1
 }
}

//writes freq table in DTB1 -----------------------------------------------------------------
DWORD* HWriteHead1(void*edtb,HTreeEnc*ptree,NAT freqszb)
{
int contor;
__asm
 {
 //write TAGs
 mov edi,edtb
 mov eax,freqszb //this is the bits per item in the frequency table (<=32)
 mov ecx,eax
 shl eax,5 //eax=freqszb*32
 add eax,4 //eax=4+freqszb*32
 stosd //this is DTB1 size
 //write frequency table
 mov [edi],ecx //this is freqszb (currently only the lower 6 bits are used; the rest are reserved for expansion)
 add edi,4
 shl ecx,8 //ch=freqszb
 xor cl,cl //cl is nr of bits currently over a DWORD boundary
 mov esi,ptree
 add esi,4 //select freq member
 xor eax,eax //used to keep significant bits
 xor ebx,ebx //used to keep shifted out significant bits
 mov contor,256
Lloop1:
  mov edx,[esi] //edx=ptree[256-contor].freq
  //and edx,0x7fffffff //the MSBit is used as a validation flag (should already be restored by HBuildTableEnc1(...))
  shld ebx,edx,cl //save in ebx the most significant bits from edx
  shl edx,cl //shift edx in position
  or eax,edx //combine with current
  add cl,ch //cl+=freqszb
  test cl,0xe0
  jz LNotOver //if(!(cl>=32))
  and cl,0x1f  //subtract anything above 31
  stosd //write current DWORD
  mov eax,ebx //use the valid bits saved in ebx
  xor ebx,ebx
LNotOver:
  add esi,8 //next node
  dec contor
  jnz Lloop1
 //it is guaranteed to be DWORD aligned because the byte size is freqszb*32, so cl should be 0 at this point
 mov eax,edi //return (DWORD aligned) pointer to where next DTB should start
 }
}

//encode udata to edtb using an encode tree and table 1 ----------------------------------------------------
DWORD* HEncode1(void*udata,NAT udataB,HTreeEnc*ptree,WORD*ptable1,void*edtb)
{
__asm
 {
 mov esi,udata //this is the uncompressed data
 mov edi,edtb //this is a pointer to DTB2 in the DTP
 add edi,4 //leave room for size
 mov eax,udataB
 stosd //write original size
 xor ecx,ecx //bit offset relative to edi
Lloop1:
  xor eax,eax
  lodsb
  shl eax,1
  mov edx,eax
  add edx,ptable1 //edx=[esi]*2+ptable1
  mov edx,[edx] 
  and edx,0xffff //edx=(WORD)ptable1[[esi]]
  add ecx,edx
  shl eax,2
  add eax,ptree //eax=[esi]*8+ptree
Lloop2:
   mov ebx,[eax]
   test ebx,0xffff
   jz LNextChar //reached the root
   dec ecx
   test ebx,0x80000000
   jz LIsLeft
   bts [edi],ecx
   jmp LNextBit
LIsLeft:
   btr [edi],ecx
LNextBit:
   and ebx,0xffff
   jz LNextChar //reached the root
   add eax,ebx //jump to next node
   jmp Lloop2
LNextChar:
  add ecx,edx
  mov edx,ecx
  and edx,0xffffffe0
  shr edx,3
  add edi,edx
  and ecx,0x1f //keep ecx under 32
  dec udataB
  jnz Lloop1
 test ecx,0xff
 jz LExact
 add edi,4
LExact:
 mov eax,edi  //return (DWORD aligned) pointer to where next DTB should start
 mov esi,edtb
 sub edi,esi
 sub edi,4 //edi=edi-edtb-4
 mov [esi],edi //write DTB size in B
 }
}

//reads freq table in DTB1, also initializes to 0 where needed----------------------------------------------------------
DWORD* HReadHead1(void*edtb,HTreeDec*ptree,NAT*porigszB)
{
NAT lorigszB;
DWORD lmask;
__asm
 {
 mov esi,edtb
 add esi,4 //DTB1 size 
 mov ebx,[esi] //bl=freqszb
 add esi,4
 //and ebx,0x3f //lowest 6 bits = freqszb (nr of bits per frequency)
 mov ecx,ebx
 sub ecx,32
 neg ecx //cl=32-freqszb
 mov bh,cl //bh=32-freqszb
 xor edx,edx
 mov lorigszB,edx //lorigszB=0
 not edx //edx=0xffffffff
 shr edx,cl //leave only freqszb 1's in edx
 mov lmask,edx //bit mask for useful bits (freqszb) starting at b0
 mov edi,ptree
 sub edi,4 //select freq member
 xor ecx,ecx //ch is contor (ch=256%256=0); cl=(nr of bits to shift out)=32-(nr of bits left in eax)
 lodsd
Lloop1:
  add edi,8 //next 
  mov edx,eax
  cmp cl,bh
  jg LLoadNextDW //j(cl>32-freqszb) there is at least one more "freq" packed in this DWORD
   shr edx,cl //shift edx in position
   add cl,bl //cl+=freqszb
   and edx,lmask //apply mask
   add lorigszB,edx
   mov [edi],edx //ptree[256-contor].freq=edx&lmask
   mov dword ptr[edi-4],0 //ptree[256-contor].ldesc:rdesc=0
   dec ch
   jnz Lloop1
  jmp LBreak
LJustCopy:
   mov edx,eax
   jmp LContinue
LLoadNextDW:
   lodsd //eax=next DW
   test ecx,0x20 //if (CL==32) shld will fail so you need to copy mannualy :(
   jnz LJustCopy
   shrd edx,eax,cl
LContinue:
   add cl,bl //cl+=freqszb
   and cl,0x1f //cl=(cl+freqszb)%32
   and edx,lmask //apply mask
   add lorigszB,edx
   mov [edi],edx //ptree[256-contor].freq=edx&lmask
   mov dword ptr[edi-4],0 //ptree[256-contor].ldesc:rdesc=0
   dec ch
   jnz Lloop1
LBreak:
 mov eax,lorigszB
 mov edx,porigszB
 mov [edx],eax //return origszB in *porigszB
 mov eax,esi //return (DWORD aligned) pointer to where next DTB starts
LReturn:
 }
}

//builds a huffman decode tree from frequency data --------------------------------------------------------------------------
//ptree should be 8*511=4088 bytes with the first 256 elements valid
NAT HBuildTreeDec1(HTreeDec*ptree,HTreeDec**proot)
{
int min2,NrNodes;
__asm
 {
  //build tree
  mov edi,ptree
  add edi,4 //select freq member
  mov NrNodes,256
Lloop2:
  mov edx,0xffffffff //edx=min1 index
  mov esi,0xffffffff //esi=min1
  mov ebx,0xffffffff //ebx=min2 index
  mov min2,0xffffffff
  xor ecx,ecx //ecx is node counter
  push edi
Lloop3:	//find mins
   mov eax,[edi] //eax=ptree[ecx].freq
   cmp eax,0
   jle Lnextnode //if(<=0) not in list
   cmp eax,esi
   jb Lnewmin1 	//if(eax<min1)
   cmp eax,min2
   jae Lnextnode //if(eax>=min2)
   mov min2,eax //min2=ptree[ecx].freq
   mov ebx,ecx //min2ind=ecx
   jmp Lnextnode
Lnewmin1:
   mov min2,esi //min2=min1
   mov esi,eax	//min1=ptree[ecx].freq
   mov ebx,edx //min2ind=min1ind
   mov edx,ecx //min1ind=ecx
Lnextnode:
   add edi,8 //edi+=sizeof(HTreeEnc)
   inc ecx
   cmp ecx,NrNodes
   jb Lloop3 //loop if(ecx<NrNodes)
  pop edi
  cmp ebx,0xffffffff
  je Lbreakloop2 //break if not found two mins (only the root left)
  //create node
  add esi,min2 //esi=min1+min2
  shl ecx,3 //*=sizeof(HTreeEnc)
  mov eax,ecx //eax=NrNodes*sizeof(HTreeEnc)
  add eax,edi //eax=&ptree[NrNodes]
  mov [eax],esi	//ptree[NrNodes].freq=min1+min2
  inc NrNodes //add new node
  //make min1 left node of the created node
  shl edx,3 //*=sizeof(HTreeEnc)
  mov esi,edx
  add esi,edi
  or dword ptr[esi],0x80000000	//remove from list
  sub edx,ecx 
  neg edx//edx=&ptree[NrNodes]-&ptree[min1ind]
  //make min2 rigth node of the created node
  shl ebx,3 //*=sizeof(HTreeEnc)
  mov esi,ebx
  add esi,edi
  or dword ptr[esi],0x80000000	//remove from list
  sub ebx,ecx 
  neg ebx//ebx=&ptree[NrNodes]-&ptree[min2ind]
  shl ebx,16
  mov bx,dx //ebx=(LO_WORD(ebx)<<16)|LO_WORD(edx)
  mov [eax-4],ebx //ptree[NrNodes].ldesc=min1ind; ptree[NrNodes].ldesc=min2ind
  jmp Lloop2 //loop if found mins
Lbreakloop2:
LReturn:
 mov eax,NrNodes //return nr of nodes in tree
 mov edx,eax
 shl edx,3
 add edx,edi
 sub edx,12 //edx=&ptree[NrNodes-1]
 mov esi,proot
 mov [esi],edx //return pointer to root node in: *proot=&ptree[NrNodes-1]
 //replace first 256 frequencies with coresponding char for easy reference
 mov ecx,256
 xor edx,edx
Lloop1:
  mov [edi],edx
  inc edx
  add edi,8
  loop Lloop1
 }
}

//decode edtb to udata using a decode tree ---------------------------------------------------------------
DWORD* HDecode1(void*edtb,HTreeDec*proot,void*udata,NAT*pudataB)
{
NAT udataB;
__asm
 {
 mov esi,edtb //this is a pointer to DTB2 in the DTP
 add esi,4 //skip DTB size
 lodsd //read original DTB size
 mov edx,pudataB
 mov [edx],eax //return udataB in *pudataB
 mov udataB,eax
 mov edx,[esi]
 add esi,4
 mov ch,32 //significant bits left in edx
 mov ebx,proot
 mov edi,udata //this is the uncompressed data
Lloop1:
  mov eax,[ebx] //eax=proot->(ldesc:rdesc)
  test eax,0xffffffff
  jz LFoundLeaf
LContinue:
  shr edx,1 //CF=LSBit(edx)
  setc cl //cl=CF
  shl cl,4 //cl=CF*16
  shr eax,cl //eax>>=CF*16
  and eax,0xffff //eax=CF?rdesc:ldesc
  sub ebx,eax //ebx=current node
  dec ch
  jnz Lloop1
  mov edx,[esi] //load next encoded DWORD
  add esi,4
  mov ch,32
  jmp Lloop1
LFoundLeaf:
  mov al,byte ptr[ebx+4] //al=(BYTE)ebx->freq
  stosb
  mov ebx,proot //start from root again
  mov eax,[ebx] //eax=proot->(ldesc:rdesc)
  dec udataB
  jnz LContinue //decode next byte
 mov eax,esi  //return (DWORD aligned) pointer to where next DTB starts
 }
}

#endif