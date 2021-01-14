#pragma once

#define V_WINET_ASYNC

#include <net.cpp>
#include <io.cpp>
#include <utree.cpp>
#include <shell.cpp>
#include <wininet.h>
#pragma comment(lib,"wininet.lib")

#define FTP_SLEEP_TIME 100

HINTERNET hinet=NULL;
INTERNET_STATUS_CALLBACK previnetcallback=NULL;
void CALLBACK IOINetCallback(HINTERNET,DWORD_PTR,DWORD,LPVOID,DWORD);
typedef void(*IOINetNotify)(DWORD,DWORD,char*,char*,DWORD);

//----------------------------------------------------------------------------------------------
BOOL InitWinINet()
{
if(!hinet)
 {
 hinet=InternetOpen(V_COMPANY" FTP client",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,INTERNET_FLAG_ASYNC);  //INTERNET_FLAG_ASYNC
 isERROR("InternetOpen() failed");
 previnetcallback=InternetSetStatusCallback(hinet,(INTERNET_STATUS_CALLBACK)IOINetCallback);
 }
return hinet!=NULL;
}

//----------------------------------------------------------------------------------------------
BOOL FreeWinINet()
{
if(!hinet) return 1;
erret=InternetCloseHandle(hinet);
hinet=NULL;
return erret;
}

//FLAGS
//#define FTP_RESERVED             0x1
#define FTP_DL_SUBDIRS           0x2
#define FTP_DL_DEL_AFTER         0x4
#define FTP_DL_FAIL_IF_EXIST     0x8

//This values are used in Notify()
#define FTP_MSG             127 
#define FTP_MSG_CONNECTED   128 
#define FTP_MSG_CHDIR       129 
#define FTP_MSG_DL_FILE     130 
#define FTP_MSG_UL_FILE     131 
#define FTP_MSG_DL_EMPTY    132 
#define FTP_MSG_UL_EMPTY    133 
#define FTP_MSG_DL_BEGIN    134 
#define FTP_MSG_UL_BEGIN    135 
#define FTP_VERBOSE         253 
#define FTP_DBG             254 
#define FTP_ERR             255 

//Context values
#define FTP_OP_CONNCO         4 //connect and company
#define FTP_OP_FIND	          8 
#define FTP_OP_GET           12
#define FTP_OP_PUT           16
#define FTP_OP_OPEN          20

//for FTP_OP_CONNCO
#define FTP_OP_CONNECT              0x1
#define FTP_OP_GETCD                0x2
#define FTP_OP_CD  		            0x4
#define FTP_OP_MD        		    0x8
#define FTP_OP_RD  		           0x10
#define FTP_OP_DEL 		           0x20
#define FTP_OP_REN 		           0x40
#define FTP_OP_MISC 		       0xFF  //mask for wait
#define FTP_OP_ENUM_FILES         0x100  //create and return extern UTREE
#define FTP_OP_ENUM_NEXT          0x200  //FindFirstFile result should be added to dl list
#define FTP_OP_ENUM_NO_MORE       0x800 
#define FTP_OP_ENUM		          0x700  //mask
#define FTP_OP_DL		         0x1000 
#define FTP_OP_UL	             0x2000
#define FTP_OP_ANY	            (0x3000|FTP_OP_ENUM|FTP_OP_MISC)  //mask

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class IOINet
{
public:
 DWORD op; //operations in progress
 //should keep order, because this pointer is calculated in callback by constant subtraction
 //this values contain the pointer adjustments to calculate this (in B)
 DWORD op_misc; //overlapped Connect, Get/SetCD, CreateDir, 
 DWORD op_find; //overlapped FindFirstFile
 DWORD op_get;  //overlapped GetFile
 DWORD op_put;  //overlapped PutFile
 HINTERNET hftp;//or gopher
 HINTERNET hff;	//handle to find first file
 char CurDir[MAX_PATH];
 DWORD CurDirNC;
 FLAGS flags;
 char dllast[PATHSZ]; //last dl file
 char ullast[PATHSZ]; //last ul file
 char dlroot[PATHSZ]; //path where to download
 char ulroot[PATHSZ]; //path from where to upload
 NAT dlsrcnc,dldstnc;
 NAT ulsrcnc,uldstnc;
 UTREE dlq; //download queue
 UTREE ulq; //upload queue
 IOINetNotify Notify;
 
 //............................................................................................... 
 IOINet()
  {
  ZEROCLASS(IOINet);
  InitWinINet();
  op_misc=FTP_OP_CONNCO; op_find=FTP_OP_FIND; op_get=FTP_OP_GET; op_put=FTP_OP_PUT;
  }
 //...............................................................................................
 ~IOINet() { Disconnect(); }
 BOOL dir();
 BOOL GetTree(UTREE*,int);
 void MkDir(UTREE*,LPSTR,int);
 BOOL Download(LPSTR);
 BOOL Upload(LPSTR);
 //............................................................................................... 
 void WaitConnect()
  {
  while((op&FTP_OP_CONNECT)&&!hftp)
   SleepEx(FTP_SLEEP_TIME,1);
  }
 //............................................................................................... 
 void WaitRes()
  {
  if(hftp)
   while(op&(FTP_OP_MISC|FTP_OP_ENUM))
    SleepEx(FTP_SLEEP_TIME,1);
  }
 //............................................................................................... 
 void WaitDone()
  {
  if(hftp)
   while(op&FTP_OP_ANY)
    SleepEx(FTP_SLEEP_TIME,1);
  }
 //............................................................................................... 
 BOOL Connect(LOGON_INFO*li,DWORD service=INTERNET_SERVICE_FTP,DWORD flags=INTERNET_FLAG_PASSIVE)
  {
  if(hftp)
   {
   if(Notify) Notify(FTP_MSG_CONNECTED,0,"Connected",NULL,0xff00);
   return 1;
   }
  ifn(hinet)
   {
   error("Must call InitWinINet()");
   return 0;
   }
  if(li->flags&ODBC_LOGON_DIRECTORY&&sl(li->directory))
   CurDirNC=sc(CurDir,li->directory,sizeof(CurDir));
  else
   CurDirNC=0;
  op|=FTP_OP_CONNECT;
  InternetConnect(hinet,li->host,li->port,li->username,li->password,service,flags,(DWORD_PTR)&op_misc);
  return 1;
  }
 //...............................................................................................
 BOOL Disconnect()
  {
  if(op&FTP_OP_ANY)
   {
   erret=MessageBox(hmwnd,"Connection is in use.\nAre you sure you want to disconnect ?\nAbort=disconnect Retry=wait Ignore=cancel","IOINet::Disconnect()",MB_ICONWARNING|MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_TOPMOST);
   if(erret==IDIGNORE) return 0;
   if(erret==IDRETRY) WaitDone();
   }
  if(hftp)
   {
   erret=InternetCloseHandle(hftp);
   hftp=NULL;
   }
  op=0;
  dlq.Free();
  ulq.Free();
  return erret;
  }
 //...............................................................................................
 BOOL sync()
  {
  if(!hftp) return 0;
  while(op&FTP_OP_MISC) SleepEx(FTP_SLEEP_TIME,1);
  op|=FTP_OP_GETCD;
  CurDirNC=MAX_PATH;
  return FtpGetCurrentDirectory(hftp,CurDir,&CurDirNC);
  }
 //test existance ...................................................................................................
 BOOL exist(LPSTR path)
  {
  HINTERNET hlff;
  WIN32_FIND_DATA wfd;
  if(!hftp) return 0;
  while(op&FTP_OP_ENUM) SleepEx(FTP_SLEEP_TIME,1);
  hlff=FtpFindFirstFile(hftp,path,&wfd,INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,NULL);
  if(!hlff) return 0; //doesn't exist
  InternetCloseHandle(hlff);
  return 1; //exist (is file or directory)
  }
 //change dir ...................................................................................................
 BOOL cd(LPSTR path)
  {
  if(!hftp) return 0;
  while(op&FTP_OP_MISC) SleepEx(FTP_SLEEP_TIME,1);
  op|=FTP_OP_CD;
  return FtpSetCurrentDirectory(hftp,path);
  }
 //make dir ...................................................................................................
 BOOL md(LPSTR path)
  {
  if(!hftp) return 0;
  while(op&FTP_OP_MISC) SleepEx(FTP_SLEEP_TIME,1);
  op|=FTP_OP_MD;
  return FtpCreateDirectory(hftp,path);
  }
 //del dir (only empty)...................................................................................................
 BOOL rd(LPSTR path)
  {
  if(!hftp) return 0;
  while(op&FTP_OP_MISC) SleepEx(FTP_SLEEP_TIME,1);
  op|=FTP_OP_CD;
  return FtpRemoveDirectory(hftp,path);
  }
 //delete file or directory...................................................................................................
 BOOL del(LPSTR path)
  {
  if(!hftp) return 0;
  while(op&FTP_OP_MISC) SleepEx(FTP_SLEEP_TIME,1);
  op|=FTP_OP_DEL;
  return FtpDeleteFile(hftp,path);
  }
};

//gets current FTP directory structure (w/o full path, w/o subfolders)...............................................................................................
BOOL IOINet::GetTree(UTREE*pt,int lflags=0)
{ //flags: 0x1=fullpath 0x2=subdirs
WIN32_FIND_DATA wfd;
char lBufDir[PATHSZ];
VTIME ltm;
ltm.init(VTMF_VLAD);
if(!hftp||!pt) return 0;
while(op&(FTP_OP_ENUM|FTP_OP_MISC)) SleepEx(FTP_SLEEP_TIME,1);
op|=FTP_OP_ENUM_FILES;
op&=~FTP_OP_ENUM_NO_MORE;
FtpFindFirstFile(hftp,"*",&wfd,INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,(DWORD_PTR)&op_find);
while(op&FTP_OP_ENUM) SleepEx(FTP_SLEEP_TIME,1);
ifn(hff) return 0;
 do{
  ltm=wfd.ftCreationTime;
  if(lflags&0x1)
   {
   mergepath(lBufDir,CurDir,wfd.cFileName,0,'/');
   pt->Add(lBufDir,wfd.dwFileAttributes,&ltm);
   }
  else 
   pt->Add(wfd.cFileName,wfd.dwFileAttributes,&ltm);
  op|=FTP_OP_ENUM_NEXT;
  InternetFindNextFile(hff,&wfd);
  while(op&FTP_OP_ENUM) SleepEx(FTP_SLEEP_TIME,1);
 }while(!(op&FTP_OP_ENUM_NO_MORE));
InternetCloseHandle(hff);
hff=NULL;
ifn(lflags&0x2) return 1; //done
for(int i=0; i<pt->nrnodes; i++)
 {
 if(pt->node[i].nr&FILE_ATTRIBUTE_DIRECTORY&&!pt->node[i].next)
  {
  cd(pt->node[i].name);
  pt->node[i].next=(UTREE*)ALLOC0(sizeof(UTREE)); //=Spawn(i);
  WaitRes();
  GetTree(pt->node[i].next,lflags); //recurse
  cd("..");
  }
 }
return 1;
}

//make dir structure (!Functie recursiva) ......................................................................................
void IOINet::MkDir(UTREE*pt,LPSTR newroot,int oldrootnc=-1)
{
UTREE_NODE*pnode;
char lBufDir[PATHSZ];
NAT l;
l=sc(lBufDir,newroot);
swapch('\\','/',lBufDir,l);
if(lBufDir[l-1]=='/') l--; //newroot must NOT have trailing sep
pt->EnumReset();
if(oldrootnc>=0) //must have full paths
 {
 while(pnode=pt->EnumNodes(NULL))
  {
  sc(lBufDir+l,pnode->name+oldrootnc,PATHSZ-l);
  swapch('\\','/',lBufDir+l);
  md(lBufDir);
  }
 } 
else
 {
 while(pnode=pt->EnumNodes(lBufDir+l))
  md(lBufDir);
 } 
while(op&FTP_OP_MISC) SleepEx(FTP_SLEEP_TIME,1);
}

//download to current dir from current ftp dir ...............................................................................................
BOOL IOINet::Download(char*localpath)
{
UTREE_NODE*pnode;
if(op&FTP_OP_DL) return 0; //dl in progress
dlq.Free();
while(op&(FTP_OP_ENUM|FTP_OP_MISC)) SleepEx(FTP_SLEEP_TIME,1);
dlsrcnc=CurDirNC;
GetTree(&dlq,(flags&FTP_DL_SUBDIRS)|1);
dlq.MkDir(localpath,dlsrcnc);
if(Notify) Notify(FTP_MSG_DL_BEGIN,0,"Starting download to: ",localpath,0xffff);
dlq.EnumReset();
if(pnode=dlq.EnumLeafs(NULL))
 {
 op|=FTP_OP_DL;
 dldstnc=sc(dlroot,localpath);
 if(dlroot[dldstnc-1]=='\\'||dlroot[dldstnc-1]=='/') //dlroot must NOT have trailing sep
  dldstnc--; //trim sep
 sc(dlroot+dldstnc,pnode->name+dlsrcnc);
 sc(dllast,pnode->name);
 FtpGetFile(hftp,pnode->name,dlroot,flags&FTP_DL_FAIL_IF_EXIST,pnode->nr,FTP_TRANSFER_TYPE_BINARY|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,(DWORD_PTR)&op_get);
 }
else
 {
 if(Notify) Notify(FTP_MSG_DL_EMPTY,0,"Download complete",NULL,0xffff);
 return 0; //no files
 }
return 1;
}

//upload dir to current ftp dir ...............................................................................................
BOOL IOINet::Upload(char*localpath)
{
UTREE_NODE*pnode;
if(op&FTP_OP_UL) return 0; //ul in progress
ulq.Free();
ulsrcnc=sl(localpath);
ulq.Dir(localpath,1);
while(op&(FTP_OP_ENUM|FTP_OP_MISC)) SleepEx(FTP_SLEEP_TIME,1);
uldstnc=sc(ulroot,CurDir);
MkDir(&ulq,ulroot,ulsrcnc);
//ulq.Show();
if(Notify) Notify(FTP_MSG_UL_BEGIN,0,"Starting upload from: ",localpath,0xffff);
ulq.EnumReset();
if(pnode=ulq.EnumLeafs(NULL))
 {
 op|=FTP_OP_UL;
 sc(ulroot+uldstnc,pnode->name+ulsrcnc);
 swapch('\\','/',ulroot);
 sc(ullast,pnode->name);
 FtpPutFile(hftp,pnode->name,ulroot,FTP_TRANSFER_TYPE_BINARY|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,(DWORD_PTR)&op_put);
 }
else
 {
 if(Notify) Notify(FTP_MSG_UL_EMPTY,0,"Upload complete",NULL,0xffff);
 return 0; //no files
 }
return 1;
}

//...............................................................................................
BOOL IOINet::dir()
{
WIN32_FIND_DATA wfd;
NAT count=0,bufnc=0;
char buffer[10100];
if(!hftp) return 0;
while(op&FTP_OP_ENUM) SleepEx(FTP_SLEEP_TIME,1);
op|=FTP_OP_ENUM_FILES;
op&=~FTP_OP_ENUM_NO_MORE;
FtpFindFirstFile(hftp,"*",&wfd,INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,(DWORD_PTR)&op_find);
while(op&FTP_OP_ENUM) SleepEx(FTP_SLEEP_TIME,1);
ifn(hff) return 0;
 do{
  count++;
  if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
   bufnc+=sprintf(buffer+bufnc,"[%s], ",wfd.cFileName);
  else
   bufnc+=sprintf(buffer+bufnc,"%s, ",wfd.cFileName);
  if(bufnc>10000) break;
  op|=FTP_OP_ENUM_NEXT;
  InternetFindNextFile(hff,&wfd);
  while(op&FTP_OP_ENUM) SleepEx(FTP_SLEEP_TIME,1);
 }while(!(op&FTP_OP_ENUM_NO_MORE));
InternetCloseHandle(hff);
hff=NULL;
bufnc-=2;
bufnc+=sprintf(buffer+bufnc,"\n%u files",count);
return MessageBox(hmwnd,buffer,CurDir,MB_OK);
}

//------------------------------------------------------------------------------------------------
void CALLBACK IOINetCallback(HINTERNET hInternet,DWORD_PTR context,DWORD status,LPVOID statinfo,DWORD sz)
{
IOINet*_this;
INTERNET_ASYNC_RESULT*res;
DWORD opcode;
if(!context) return;
opcode=*((DWORD*)context);
_this=(IOINet*)(((char*)context)-opcode);
switch (status)
 {
 case INTERNET_STATUS_REQUEST_COMPLETE:
 case INTERNET_STATUS_HANDLE_CREATED:
  res=(INTERNET_ASYNC_RESULT*)statinfo;
  if(res->dwError&&_this->Notify)
   {
   REGu=sprintf(REGs,"CONTEXT=%x OP=%x ResER=%u, ",opcode,_this->op,res->dwError);
   if(LastErrorMsg(REGs+REGu,sizeof(REGs)-REGu))
    {
    TrimStr(0,31,REGs,sl(REGs));
    if(status==INTERNET_STATUS_REQUEST_COMPLETE)
     _this->Notify(FTP_ERR,res->dwError,"REQUEST_COMPLETE ",REGs,0xff);
    else
     _this->Notify(FTP_DBG,res->dwError,"HANDLE_CREATED ",REGs,0x7f);
    } 
   }
  if(status==INTERNET_STATUS_REQUEST_COMPLETE) //
   {
   if(opcode==_this->op_misc)
    {
    if(_this->op&FTP_OP_CONNECT)
     {
     _this->hftp=(HINTERNET)res->dwResult;
     _this->op&=~FTP_OP_CONNECT; //reset
     if(_this->CurDirNC) _this->cd(_this->CurDir);
     if(_this->Notify) _this->Notify(FTP_MSG_CONNECTED,res->dwError,res->dwError?"Failed to connect":"Connected",NULL,0xff00);
     }
    else if(_this->op&FTP_OP_GETCD)
     {
     //if(_this->Notify) _this->Notify(FTP_MSG_CHDIR,res->dwError,"CD: ",_this->CurDir,0xffff);
     }
    else if(_this->op&FTP_OP_CD&&!res->dwError) //if succesfull ask for current dir
     {
     _this->op&=~FTP_OP_CD;
     _this->sync();
     return;
     }
    else if(_this->op&FTP_OP_MD)
     {
     //if(_this->Notify) _this->Notify(FTP_DBG,res->dwError,"MD",NULL,0xffff);
     }
    else if(_this->op&FTP_OP_DEL)
     {
     //if(_this->Notify) _this->Notify(FTP_DBG,res->dwError,"DEL",NULL,0xffff);
     }
    _this->op&=~FTP_OP_MISC; //reset all misc ops
    } 
   else if(opcode==_this->op_find)
    {
    if(_this->op&FTP_OP_ENUM_FILES)
     {
     _this->hff=(HINTERNET)res->dwResult;
     _this->op&=~FTP_OP_ENUM; //reset all enums
     //if(_this->Notify) _this->Notify(FTP_DBG,res->dwError,res->dwError?"Failed to create handle":"Handle created",NULL,0x7f7f7f);
     }
    else if(_this->op&FTP_OP_ENUM_NEXT)
     {
     _this->op&=~FTP_OP_ENUM; //reset all enums
     if(res->dwError)
      {
      _this->op|=FTP_OP_ENUM_NO_MORE;
      //if(_this->Notify) _this->Notify(FTP_DBG,0,"ENUM_NO_MORE",NULL,0x7f7f7f);
      }
     }
    }
   else if(opcode==_this->op_get)
    {
    if(_this->Notify) _this->Notify(FTP_MSG_DL_FILE,res->dwError,"Downloaded: ",_this->dlroot,0xff00);
    if(_this->flags&FTP_DL_DEL_AFTER)
     {
     _this->del(_this->dllast);
     if(_this->Notify) _this->Notify(FTP_MSG,0,"Deleted: ",_this->dllast,0xff00ff);
     }
    UTREE_NODE*pnode;
    if(pnode=_this->dlq.EnumLeafs(NULL))
     {
     sc(_this->dlroot+_this->dldstnc,pnode->name+_this->dlsrcnc);
     sc(_this->dllast,pnode->name);
     FtpGetFile(_this->hftp,pnode->name,_this->dlroot,_this->flags&FTP_DL_FAIL_IF_EXIST,pnode->nr,FTP_TRANSFER_TYPE_BINARY|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,context);
     }
    else
     {
     _this->op&=~FTP_OP_DL; //dl complete
     _this->dlq.Free();
     if(_this->Notify) _this->Notify(FTP_MSG_DL_EMPTY,0,"Download complete",NULL,0xffff);
     }
    }
   else if(opcode==_this->op_put)
    {
    if(_this->Notify) _this->Notify(FTP_MSG_UL_FILE,res->dwError,"Uploaded: ",_this->ulroot,0xff0000);
    UTREE_NODE*pnode;
    if(pnode=_this->ulq.EnumLeafs(NULL))
     {
     sc(_this->ulroot+_this->uldstnc,pnode->name+_this->ulsrcnc);
     swapch('\\','/',_this->ulroot);
     sc(_this->ullast,pnode->name);
     FtpPutFile(_this->hftp,pnode->name,_this->ulroot,FTP_TRANSFER_TYPE_BINARY|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,context);
     }
    else
     {
     _this->op&=~FTP_OP_UL; //ul complete
     _this->ulq.Free();
     if(_this->Notify) _this->Notify(FTP_MSG_UL_EMPTY,0,"Upload complete",NULL,0xffff);
     }
    }
   }
  break; 
 case INTERNET_STATUS_RESPONSE_RECEIVED:
  sprintf(REGs,"%uB",*((DWORD*)statinfo));
  if(_this->Notify) _this->Notify(FTP_VERBOSE,0,"Received: ",REGs,0x7f00);
  break;
 case INTERNET_STATUS_REQUEST_SENT:
  sprintf(REGs,"%uB",*((DWORD*)statinfo));
  if(_this->Notify) _this->Notify(FTP_VERBOSE,0,"Sent: ",REGs,0x7f7f00);
  break;
#ifdef _DEBUG
 case INTERNET_STATUS_RECEIVING_RESPONSE:
  if(_this->Notify) _this->Notify(FTP_DBG,0,"RECEIVING_RESPONSE",NULL,0x3f3f3f);
  break;
 case INTERNET_STATUS_SENDING_REQUEST:
  if(_this->Notify) _this->Notify(FTP_DBG,0,"SENDING_REQUEST",NULL,0x3f3f3f);
  break;
 case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
  if(_this->Notify) _this->Notify(FTP_DBG,0,"INTERMEDIATE_RESPONSE",NULL,0x3f3f3f);
  break;
 case INTERNET_STATUS_CONNECTING_TO_SERVER:
  if(_this->Notify) _this->Notify(FTP_DBG,0,"CONNECTING_TO_SERVER",NULL,0x3f3f3f);
  break;
 case INTERNET_STATUS_CONNECTED_TO_SERVER:
  if(_this->Notify) _this->Notify(FTP_DBG,0,"CONNECTED_TO_SERVER",NULL,0x3f3f3f);
  break;
 case INTERNET_STATUS_CLOSING_CONNECTION:
  if(_this->Notify) _this->Notify(FTP_DBG,0,"CLOSING_CONNECTION",NULL,0x3f3f3f);
  break;
 case INTERNET_STATUS_HANDLE_CLOSING:
  if(_this->Notify) _this->Notify(FTP_DBG,0,"HANDLE_CLOSING",NULL,0x3f3f3f);
  break;
 case INTERNET_STATUS_STATE_CHANGE:
  if(_this->Notify) _this->Notify(FTP_VERBOSE,0,"Status changed: ",NULL,0xff00ff);
  break;
 default:
  if(_this->Notify) _this->Notify(FTP_ERR,0,"Unknown status received",(char*)statinfo,0xff);
  break;
#endif
 case INTERNET_STATUS_CONNECTION_CLOSED:
  if(_this->Notify) _this->Notify(FTP_MSG,0,"Disconnected",NULL,0xff00ff);
  break;
 case INTERNET_STATUS_RESOLVING_NAME:
  if(_this->Notify) _this->Notify(FTP_MSG,0,"Resolving: ",(char*)statinfo,0x7f7f);
  break;
 case INTERNET_STATUS_NAME_RESOLVED:
  if(_this->Notify) _this->Notify(FTP_MSG,0,"Resolved: ",(char*)statinfo,0xff00);
  break;
 case INTERNET_STATUS_REDIRECT:
  if(_this->Notify) _this->Notify(FTP_MSG,0,"Redirected to: ",(char*)statinfo,0xff00ff);
  break;
 }
}

