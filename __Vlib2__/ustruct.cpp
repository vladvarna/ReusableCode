#pragma once

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <class DynStruct> class USTRUCT
{
public:
 DynStruct**item; //items
 NAT nrit; //nr of items
    
 USTRUCT() { ZEROCLASS(USTRUCT); }
 ~USTRUCT() { Free(); }
//............................................................................................ 
 void Free()
 {
 for(int i=0;i<nrit;i++)
  FREE(item[i]);
 FREE(item);
 nrit=0;
 }
//............................................................................................ 
 NAT dim(NAT lnrit=0)  //ins at end, del from end
 {
 if(lnrit==-1) lnrit=nrit+1; //ins one at end
 item=(DynStruct**)REALLOC(item,sizeof(DynStruct*)*(lnrit));
 if(lnrit>nrit) ZeroMemory(item+nrit,(lnrit-nrit)*sizeof(DynStruct*));
 nrit=lnrit;
 return nrit-1;
 }
//............................................................................................ 
 void ins(NAT pos=0,NAT itcnt=1)
 {
 item=(DynStruct**)REALLOC(item,sizeof(DynStruct*)*(nrit+itcnt));
 if(pos<nrit&&itcnt)
  {
  ShiftMemR(item+pos,itcnt*sizeof(DynStruct*),(nrit-pos)*sizeof(DynStruct*));
  ZeroMemory(item+pos,itcnt*sizeof(DynStruct*));
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
  ShiftMemL(item+(pos+itcnt),itcnt*sizeof(DynStruct*),(nrit-pos-itcnt)*sizeof(DynStruct*));
  }
 nrit-=itcnt;
 item=(DynStruct**)REALLOC(item,(nrit-itcnt)*sizeof(DynStruct*));
 }
//............................................................................................ 
 void freeit(NAT itind,NAT szB=0)
 {
 if(itind<nrit)
  FREE(item[itind]);
 }
//(re)size item...............................................................................
 NAT dimit(NAT itind,NAT dinamicsz=0)
 {
 if(itind>=nrit)
  {
  itind=nrit;
  dim(nrit+1);
  }
 else
  FREE(item[itind]);
 item[itind]=(DynStruct*)ALLOC0(sizeof(DynStruct)+dinamicsz);
 return itind;
 }
//............................................................................................ 
 FAIL SaveStatic(LPSTR path=NULL)
 {
 IOSFile iof;
 if(iof.open(path,FU_WO))
  return 1;
 iof.write(&nrit,sizeof(nrit));
 for(int i=0;i<nrit;i++)
  iof.write(item[i],sizeof(DynStruct));
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
 //TODO: recover number of items
 for(int i=0;i<nrit;i++)
  {
  dimit(i,0);
  iof.read(item[i],sizeof(DynStruct));
  }
 return 0;
 }
};

