#pragma once

#include <vtime.cpp>

class UTREE;

class UTREE_NODE
{
public: 
 char*name;
 int nr; //color,state,flags,etc.
 VTIME tm;
 NAT dtsz; //extra data size (may be used for other purpose if not using extra data)
 void*dt; //extra data
 UTREE*next; //each node may have a whole subtree
 
 void FreeNode();
 void Set(char*,int,VTIME*,NAT,void*);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UTREE
{
public:
 UTREE_NODE*node; //root nodes
 NAT nrnodes; //number of root nodes
 int ienum; //internal enumeration index
 
 UTREE() { ZEROCLASS(UTREE); }
 void Dir(LPSTR,int,LPSTR);
 void MkDir(LPSTR,int); //makes dir structure
 BOOL SaveTXT(LPSTR,int);
 BOOL LoadTXT(LPSTR,int);
 ~UTREE() { Free(); }
 
 //..........................................................................................
 void Free()
  {
  if(node)
   {
   while(nrnodes)
    {
    node[--nrnodes].FreeNode();
    }
   } 
  FREE(node);
  }
 //..........................................................................................
 NAT Add(char*lstr,int lnr=0,VTIME*ptm=NULL,NAT ldtsz=0,void*ldt=NULL)
  {
  nrnodes++;
  node=(UTREE_NODE*)REALLOC(node,nrnodes*sizeof(UTREE_NODE));
  ZeroMemory(&node[nrnodes-1],sizeof(UTREE_NODE));
  node[nrnodes-1].Set(lstr,lnr,ptm,ldtsz,ldt);
  return nrnodes-1; //index
  }
 //creates a child for node index..........................................................................................
 UTREE* Spawn(NAT index=0)
  {
  if(index>=nrnodes) return NULL;
  return node[index].next=(UTREE*)ALLOC0(sizeof(UTREE));
  }
 //............................................................................................ 
 void Del(NAT pos=0,NAT itcnt=1)
 {
 if(pos>=nrnodes) return;
 if(pos+itcnt>=nrnodes) itcnt=nrnodes-pos;
 for(int i=pos;i<pos+itcnt;i++)
  node[i].FreeNode();
 if(itcnt)
  {
  ShiftMemL(node+pos+itcnt,itcnt*sizeof(UTREE_NODE),(nrnodes-pos-itcnt)*sizeof(UTREE_NODE));
  }
 nrnodes-=itcnt;
 node=(UTREE_NODE*)REALLOC(node,nrnodes*sizeof(UTREE_NODE));
 }
 //....................................................................................................... 
 NAT CountLeafs()
 {
 NAT nrleafs=0;
 for(int i=0;i<nrnodes;i++)
  {
  if(node[i].next)
   nrleafs+=node[i].next->CountLeafs();
  else
   nrleafs++; 
  } 
 return nrleafs;
 }
 //....................................................................................................... 
 NAT CountNodes()
 {
 NAT nrnodes=0;
 for(int i=0;i<nrnodes;i++)
  {
  if(node[i].next)
   nrnodes+=1+node[i].next->CountNodes();
  } 
 return nrnodes;
 }
 //delete empty nodes ....................................................................................................... 
 void DelEmpty()
 {
 for(int i=0;i<nrnodes;i++)
  {
  if(node[i].next)
   {
   node[i].next->DelEmpty();
   if(node[i].next->CountLeafs()<1)
    {
    Del(i,1);
    i--;
    }
   }
  } 
 }

 //............................................................................................ 
 void EnumReset()
 {
 ienum=0;
 for(int i=0;i<nrnodes;i++)
  if(node[i].next)
   node[i].next->EnumReset();
 }
 //....................................................................................................... 
 UTREE_NODE* EnumLeafs(char*relpath=NULL) //relpath is valid only if not containing full paths already
 {
 UTREE_NODE*ret;
 NAT l;
 if(ienum>=nrnodes) return NULL;
 if(node[ienum].next)
  {
  if(relpath)
   {
   *relpath='/';
   l=sc(relpath+1,node[ienum].name);
   ret=node[ienum].next->EnumLeafs(relpath+l+1);
   }
  else
   ret=node[ienum].next->EnumLeafs(NULL);
  if(!ret)
   {
   ienum++;
   ret=EnumLeafs(relpath);
   }
  }
 else
  {
  if(relpath)
   {
   *relpath='/';
   l=sc(relpath+1,node[ienum].name);
   }
  ret=node+ienum;
  ienum++;
  }
 return ret;
 }
 //....................................................................................................... 
 UTREE_NODE* EnumNodes(char*relpath=NULL) //relpath is valid only if not containing full paths already
 {
 UTREE_NODE*ret;
 NAT l;
 if(ienum&0x80000000) //enumerate subnodes
  {
  if(relpath)
   {
   *relpath='/';
   l=sc(relpath+1,node[ienum&0x7fffffff].name);
   ret=node[ienum&0x7fffffff].next->EnumNodes(relpath+l+1);
   }
  else
   ret=node[ienum&0x7fffffff].next->EnumNodes(NULL);
  if(!ret) //done enumerating subnodes for current, go to next node
   {
   ienum&=0x7fffffff;
   ienum++;
   ret=EnumNodes(relpath);
   }
  } 
 else
  {
  if(ienum>=nrnodes) return NULL;
  while(!node[ienum].next)
   {
   ienum++;
   if(ienum>=nrnodes) return NULL;
   }
  if(relpath)
   {
   *relpath='/';
   l=sc(relpath+1,node[ienum].name);
   }
  ret=node+ienum;
  ienum|=0x80000000; //enumerate subnodes next
  }
 return ret;
 }
 // DBG ...............................................................................................
 void Show(int shownodes=0)
 {
 UTREE_NODE*pnode;
 VSTR vs;
 EnumReset();
 if(shownodes)
  {
  while(pnode=EnumNodes(NULL))
   {
   vs.apf("\n%x [%s] ",pnode->nr,pnode->name);
   vs+pnode->tm;
   }
  } 
 else
  {
  while(pnode=EnumLeafs(NULL))
   {
   vs.apf("\n%x  %s  ",pnode->nr,pnode->name);
   vs+pnode->tm;
   }
  } 
 vs.show();
 }
 //...............................................................................................
 void HList(HWND hlbox,int flags=0x3)
 { //flags= b0-show leafs, b1-show nodes
 SendMessage(hlbox,LB_RESETCONTENT,0,0); //remove all
 for(int i=0;i<nrnodes;i++)
  if((node[i].next&&flags&0x2)||flags&0x1)
   SendMessage(hlbox,LB_ADDSTRING,0,(LPARAM)node[i].name); //add
 }
 //...............................................................................................
 void HTree(HWND htree,int flags=0x3,HTREEITEM htreeit=NULL)
 {//flags= b0-show leafs, b1-show nodes, b2-show leafs in lowercase, b3-show nodes in uppercase
 HTREEITEM htreesub;
 TVINSERTSTRUCT tvi;
 tvi.hInsertAfter=TVI_SORT;
 tvi.itemex.mask=TVIF_PARAM|TVIF_TEXT;
 tvi.itemex.stateMask=0;
 tvi.hParent=htreeit;//TVI_ROOT
 for(int i=0;i<nrnodes;i++)
  {
  if(node[i].next) //node
   {
   if(flags&0x2) //show nodes
    {
    if(flags&0x8) UPcase(node[i].name,sl(node[i].name));
    tvi.itemex.pszText=node[i].name;
    tvi.itemex.lParam=StrToDW_ID(tvi.itemex.pszText);
    htreesub=(HTREEITEM)SendMessage((HWND)htree,TVM_INSERTITEM,0,(LPARAM)&tvi);
    node[i].next->HTree(htree,flags,htreesub);
    }
   else
    node[i].next->HTree(htree,flags,htreeit);
   }
  else if(flags&0x1)//leaf
   {
   if(flags&0x4) LOcase(node[i].name,sl(node[i].name));
   tvi.itemex.pszText=node[i].name;
   tvi.itemex.lParam=StrToDW_ID(tvi.itemex.pszText);
   SendMessage(htree,TVM_INSERTITEM,0,(LPARAM)&tvi);
   }
  }
 }
#ifdef V_MFC
 //...............................................................................................
 void CTree(CTreeCtrl*ptree,int flags=0x13,HTREEITEM hroot=NULL)
 { //flags= b0-show leafs, b1-show nodes, b2- show leafs in lowercase, b3- show nodes in uppercase, b4-delete all
 if(flags&0x10&&(!hroot)) ptree->DeleteAllItems();
 HTree(ptree->m_hWnd,flags,hroot);
 }
#endif
}; //end of UTREE class

 //..........................................................................................
 void UTREE_NODE::Set(char*lstr,int lnr=0,VTIME*ptm=NULL,NAT ldtsz=0,void*ldt=NULL)
  {
  NAT nc;
  nc=sl(lstr);
  name=SALLOC(nc);
  sc(name,lstr,nc);
  nr=lnr;
  if(ptm)
   CopyMemory(&tm,ptm,sizeof(VTIME));
  else
   tm.Now();
  dtsz=ldtsz;
  if(ldt&&ldtsz)
   {
   dt=ALLOC(dtsz);
   CopyMemory(dt,ldt,dtsz);
   }
  else
   dt=NULL; 
  next=NULL; //! inits without freeing
  }

 //.................................................................................................
 void UTREE_NODE::FreeNode()
 {
 FREE(name);
 nr=0;
 FREE(dt);
 dtsz=0;
 if(next)
  {
  next->Free();
  FREE(next); 
  }
 }

//append dir structure (!Functie recursiva) ......................................................................................
void UTREE::Dir(LPSTR root=NULL,int flags=0,LPSTR filter="*")
{ //b0=fullpath, b1=only folders, b2=only files, b3=skip subfolders
HANDLE hff;
WIN32_FIND_DATA wfd;
char lBufDir[PATHSZ];
VTIME ltm;
ltm.init(VTMF_VLAD);
mergepath(lBufDir,root,filter);
hff=FindFirstFile(lBufDir,&wfd);
if(hff!=INVALID_HANDLE_VALUE)
 {
 do{
  if(wfd.cFileName[0]!='.') //exclude parent and current because they can cause infinite loops
   {
   if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY||!(flags&0x2))
    {
    if(!(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)||!(flags&0x4))
     {
     ltm=wfd.ftCreationTime;
     if(flags&0x1)
      {
      mergepath(lBufDir,root,wfd.cFileName);
      Add(lBufDir,wfd.dwFileAttributes,&ltm);
      }
     else 
      Add(wfd.cFileName,wfd.dwFileAttributes,&ltm);
     } 
    }
   } 
  }while(FindNextFile(hff,&wfd));
 FindClose(hff);
 } 
if(flags&0xC) return;
for(int i=0; i<nrnodes; i++)
 {
 if(node[i].nr&FILE_ATTRIBUTE_DIRECTORY)
  {
  ifn(node[i].next)
   node[i].next=(UTREE*)ALLOC0(sizeof(UTREE)); //=Spawn(i);
  if(flags&0x1)
   node[i].next->Dir(node[i].name,flags,filter);
  else
   {
   mergepath(lBufDir,root,node[i].name);
   node[i].next->Dir(lBufDir,flags,filter);
   }
  }
 }
}//TODO: properly support FILETIME

//make dir structure (!Functie recursiva) ......................................................................................
void UTREE::MkDir(LPSTR newroot,int oldrootnc=-1)
{
UTREE_NODE*pnode;
char lBufDir[PATHSZ];
NAT l;
l=sc(lBufDir,newroot);
EnumReset();
if(oldrootnc>=0) //must have full paths
 {
 if(lBufDir[l-1]!='\\'&&lBufDir[l-1]!='/') //newroot must have trailing sep
  lBufDir[l++]='/'; //append sep
 while(pnode=EnumNodes(NULL))
  {
  sc(lBufDir+l,pnode->name+oldrootnc,PATHSZ-l);
  CreateDirectory(lBufDir,NULL);
  }
 } 
else
 {
 if(lBufDir[l-1]=='\\'||lBufDir[l-1]=='/') l--; //newroot must NOT have trailing sep
 while(pnode=EnumNodes(lBufDir+l))
  CreateDirectory(lBufDir,NULL);
 } 
}

//save as text file (only curent node)......................................................................................
BOOL UTREE::SaveTXT(LPSTR path,int append=0)
{
FILE *fis;
fis=FOPEN(path,append?"a+t":"wt");
if(!fis) return 0;
for(int i=0;i<nrnodes;i++)
 fprintf(fis,"%s\r\n",node[i].name);
fclose(fis); 
return 1;
}

//load from text file (only curent node)......................................................................................
BOOL UTREE::LoadTXT(LPSTR path,int append=0)
{
FILE *fis;
char buf[1024];
fis=FOPEN(path,"rt");
if(!fis) return 0;
ifn(append) Free();
while(!feof(fis))
 {
 if(fscanf(fis,"%s",buf)>0)
  Add(buf);
 }
fclose(fis); 
return 1;
}
