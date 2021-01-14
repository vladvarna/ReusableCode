#pragma once

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <class StaticStruct> class UARRAY
{
public:
 StaticStruct*item; //items
 NAT nrit; //nr of items
    
 UARRAY() { ZEROCLASS(UARRAY); }
 ~UARRAY() { Free(); }
//............................................................................................ 
 void Free()
 {
 FREE(item);
 nrit=0;
 }
//............................................................................................ 
 NAT dim(NAT lnrit=0)  //ins at end, del from end
 {
 if(lnrit==-1) lnrit=nrit+1; //ins one at end
 item=(StaticStruct*)REALLOC(item,sizeof(StaticStruct)*(lnrit));
 if(lnrit>nrit) ZeroMemory(item+nrit,(lnrit-nrit)*sizeof(StaticStruct));
 nrit=lnrit;
 return nrit-1;
 }
//............................................................................................ 
 void ins(NAT pos=0,NAT itcnt=1)
 {
 item=(StaticStruct*)REALLOC(item,sizeof(StaticStruct)*(nrit+itcnt));
 if(pos<nrit&&itcnt)
  {
  ShiftMemR(item+pos,itcnt*sizeof(StaticStruct),(nrit-pos)*sizeof(StaticStruct));
  ZeroMemory(item+pos,itcnt*sizeof(StaticStruct));
  }
 nrit+=itcnt;
 }
//............................................................................................ 
 void del(NAT pos=0,NAT itcnt=1)
 {
 if(pos>=nrit) return;
 if(pos+itcnt>=nrit) itcnt=nrit-pos;
 if(itcnt)
  {
  ShiftMemL(item+(pos+itcnt),itcnt*sizeof(StaticStruct),(nrit-pos-itcnt)*sizeof(StaticStruct));
  }
 nrit-=itcnt;
 item=(StaticStruct*)REALLOC(item,(nrit-itcnt)*sizeof(StaticStruct));
 }
//............................................................................................ 
 void Zero(NAT pos=0,NAT itcnt=-1)
 {
 if(pos>=nrit) return;
 if(pos+itcnt>=nrit) itcnt=nrit-pos;
 ZeroMemory(item+pos,itcnt*sizeof(StaticStruct));
 }
//............................................................................................ 
 void Clone(UARRAY<StaticStruct>*puarray)
 {
 dim(puarray->nrit);
 CopyMemory(item,puarray->item,nrit*sizeof(StaticStruct));
 }
//can index directly............................................................................................ 
 StaticStruct operator[](NAT ind)
 {
 StaticStruct ls;
 ZeroMemory(&ls,sizeof(StaticStruct));
 if(ind<nrit)
  return item[ind];
 else
  {
  error("Invalid index in UARRAY");
  return ls;
  }
 }
//if used as number return item count............................................................................................ 
 operator NAT() //(NAT)
 {
 return nrit;
 }
//if used as pointer return item count............................................................................................ 
 operator void*() //(void*)
 {
 return item;
 }
//............................................................................................ 
 FAIL SaveStatic(LPSTR path=NULL)
 {
 IOSFile iof;
 if(iof.open(path,FU_WO))
  return 1;
 iof.write(&nrit,sizeof(nrit));
 iof.write(item,nrit*sizeof(StaticStruct));
 return 0;
 }
//............................................................................................ 
 FAIL LoadStatic(LPSTR path=NULL)
 {
 IOSFile iof;
 NAT lnrit;
 if(iof.open(path,FU_R|FF_SILENT))
  return 1;
 Free();
 iof.read(&lnrit,sizeof(lnrit));
 dim(lnrit);
 iof.read(item,nrit*sizeof(StaticStruct));
 return 0;
 }
};

