#pragma once

#define V_WINET_SYNC

#include <net.cpp>
#include <io.cpp>
#include <utree.cpp>
#include <wininet.h>
#pragma comment(lib,"wininet.lib")

HINTERNET hinet=NULL;

//FLAGS
//#define FTP_RESERVED             0x1
#define FTP_DL_SUBDIRS           0x2
#define FTP_DL_DEL_AFTER         0x4
#define FTP_DL_FAIL_IF_EXIST     0x8
#define FTP_DL_ABORT             0x10000 //abort after current file
#define FTP_UL_ABORT             0x20000

//----------------------------------------------------------------------------------------------
BOOL InitWinINet()
{
if(!hinet)
 {
 hinet=InternetOpen(V_COMPANY" FTP client",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);  //INTERNET_FLAG_ASYNC
 isERROR("InternetOpen() failed");
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class IOINet
{
public:
 HINTERNET hftp; //or gopher
 HINTERNET hhttp;
 char CurDir[MAX_PATH];
 DWORD CurDirNC;
 FLAGS flags;
  
 //............................................................................................... 
 IOINet() { ZEROCLASS(IOINet); InitWinINet(); }
 BOOL GetTree(UTREE*,FLAGS);
 BOOL DlTree(UTREE*,char*,HWND);
 void Download();
 BOOL Upload(LPSTR);
 void dir();
 //............................................................................................... 
 BOOL Connect(LOGON_INFO*li,DWORD service=INTERNET_SERVICE_FTP)
  {
  hftp=InternetConnect(hinet,li->host,li->port,li->username,li->password,service,0,(DWORD_PTR)this);
  if(!hftp)
   {
   #ifdef _DEBUG
   isERROR("InternetOpen() failed");
   #endif
   return 0;
   }
  sync();
  return 1;
  }
 //...............................................................................................
 BOOL Disconnect()
  {
  erret=InternetCloseHandle(hftp);
  hftp=NULL;
  return erret;
  }
 //...............................................................................................
 void sync()
  {
  CurDirNC=MAX_PATH;
  FtpGetCurrentDirectory(hftp,CurDir,&CurDirNC);
  isERROR("FtpGetCurrentDirectory() failed");
  //MessageBox(hmwnd,CurDir,"Directory",MB_OK);
  }
 //change dir ...................................................................................................
 BOOL exist(LPSTR path)
  {
  HINTERNET hff;
  WIN32_FIND_DATA wfd;
  hff=FtpFindFirstFile(hftp,path,&wfd,0,(DWORD_PTR)this);
  if(!hff) return 0; //doesn't exist
  InternetCloseHandle(hff);
  return 1; //exist (is file or directory)
  }
 //.................................................................................................................
 BOOL cd(LPSTR path)
  {
  erret=FtpSetCurrentDirectory(hftp,path);
  #ifdef _DEBUG
  isERROR("FtpSetCurrentDirectory()");
  #endif
  sync();
  return erret; //Ok
  }
 //make dir ...................................................................................................
 BOOL md(LPSTR path)
  {
  return FtpCreateDirectory(hftp,path);
  }
 //delete file or directory...................................................................................................
 BOOL del(LPSTR path)
  {
  return FtpDeleteFile(hftp,path);
  }
 //del dir (only empty)...................................................................................................
 BOOL rd(LPSTR path)
  {
  return FtpRemoveDirectory(hftp,path);
  }
 //...............................................................................................
 ~IOINet() { Disconnect(); }
};

//gets current FTP directory structure (w/o full path, w/o subfolders)...............................................................................................
BOOL IOINet::GetTree(UTREE*pt,FLAGS lflags=0)
{ //flags: 0x1=fullpath 0x2=subdirs
WIN32_FIND_DATA wfd;
HINTERNET hff;
char lBufDir[PATHSZ];
VTIME ltm;
if(!hftp||!pt) return 0;
hff=FtpFindFirstFile(hftp,"*",&wfd,INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,NULL);
ifn(hff) return 0;
 do{
  if(!(lflags&0x2)&&(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) 
   continue;
  ltm=wfd.ftCreationTime;
  if(lflags&0x1)
   {
   mergepath(lBufDir,CurDir,wfd.cFileName,0,'/');
   pt->Add(lBufDir,wfd.dwFileAttributes,&ltm);
   }
  else 
   pt->Add(wfd.cFileName,wfd.dwFileAttributes,&ltm);
 }while(InternetFindNextFile(hff,&wfd));
InternetCloseHandle(hff);
hff=NULL;
ifn(lflags&0x2) return 1; //done
for(int i=0; i<pt->nrnodes; i++)
 {
 if(pt->node[i].nr&FILE_ATTRIBUTE_DIRECTORY&&!pt->node[i].next)
  {
  cd(pt->node[i].name);
  pt->node[i].next=(UTREE*)ALLOC0(sizeof(UTREE)); //=Spawn(i);
  GetTree(pt->node[i].next,lflags); //recurse
  cd("..");
  }
 }
return 1;
}

//download tree with relative paths from ftp to local...............................................................................................
BOOL IOINet::DlTree(UTREE*pt,char*localpath,HWND hwndprogress=NULL)
{
UTREE_NODE*pnode;
NAT nrfiles;
char lBufDir[PATHSZ];
//pt->MkDir(localpath,);
if(hwndprogress)
 {
 nrfiles=pt->CountLeafs();
 SendMessage(hwndprogress,PBM_SETRANGE32,0,nrfiles);
 SendMessage(hwndprogress,PBM_SETPOS,nrfiles,0);
 }
pt->EnumReset();
while((pnode=pt->EnumLeafs(NULL))&&!(flags&FTP_DL_ABORT))
 {
 mergepath(lBufDir,localpath,pnode->name);
 //printbox(lBufDir);
 ifn(FtpGetFile(hftp,pnode->name,lBufDir,flags&FTP_DL_FAIL_IF_EXIST,pnode->nr,FTP_TRANSFER_TYPE_BINARY|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,NULL))
  {
  erret=GetLastError();
  if(erret!=0x50) //file already exists
   {
   isERROR("IOINet::DlTree() FtpGetFile() failed");
   continue;
   }
  }
 if(hwndprogress)
  SendMessage(hwndprogress,PBM_SETPOS,--nrfiles,0);
 if(flags&FTP_DL_DEL_AFTER)
  del(pnode->name);
 }
return 1;
}

//download to current dir from current ftp dir (without subdirectories)...............................................................................................
void IOINet::Download()
{
HINTERNET hff;
WIN32_FIND_DATA wfd;
NAT count=0;
hff=FtpFindFirstFile(hftp,"*",&wfd,0,(DWORD_PTR)this);
if(!hff) return;
 do{
  //printbox("%x %s",wfd.dwFileAttributes,wfd.cFileName);
  if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
   {
   cd(wfd.cFileName);
   ios.md(wfd.cFileName);
   ios.cd(wfd.cFileName);
   Download();
   cd("..");
   ios.cd("..");
   }
  else
   {
   if(!FtpGetFile(hftp,wfd.cFileName,wfd.cFileName,flags&FTP_DL_FAIL_IF_EXIST,wfd.dwFileAttributes,FTP_TRANSFER_TYPE_BINARY|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD|INTERNET_FLAG_RESYNCHRONIZE,NULL))
    isERROR("Download file");
   }
  count++;
  }while(InternetFindNextFile(hff,&wfd));
InternetCloseHandle(hff);
}

//(!Atentie: functie recursiva) upload dir to current ftp dir ...............................................................................................
BOOL IOINet::Upload(char*localpath)
{
HANDLE hff;
WIN32_FIND_DATA wfd;
NAT count=0;
char lBufDir[PATHSZ];
mergepath(lBufDir,localpath,"*",0,'/');
//printbox(">%s<",lBufDir);
hff=FindFirstFile(lBufDir,&wfd);
if(hff==INVALID_HANDLE_VALUE) return -1; //no files
 do{
  //printbox("%x %s",wfd.dwFileAttributes,wfd.cFileName);
  mergepath(lBufDir,localpath,wfd.cFileName);
  if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
   {
   if(wfd.cFileName[0]!='.') //avoid infinte loop
    {
    md(wfd.cFileName);
    cd(wfd.cFileName);
    Upload(lBufDir);
    cd("..");
    }
   }
  else
   {
   if(!FtpPutFile(hftp,lBufDir,wfd.cFileName,FTP_TRANSFER_TYPE_BINARY,(DWORD_PTR)this))
    isERROR("Upload file");
   }
  count++;
 }while(FindNextFile(hff,&wfd));
FindClose(hff);
return count;
}

//...............................................................................................
void IOINet::dir()
{
HINTERNET hff;
WIN32_FIND_DATA wfd;
NAT count=0,bufnc=0;
char buffer[10100],lBufDir[PATHSZ];
mergepath(lBufDir,CurDir,"*",0,'/');
hff=FtpFindFirstFile(hftp,lBufDir,&wfd,0,(DWORD_PTR)this);
ifn(hff) return;
 do{
  count++;
  if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
   bufnc+=sprintf(buffer+bufnc,"[%s], ",wfd.cFileName);
  else
   bufnc+=sprintf(buffer+bufnc,"%s, ",wfd.cFileName);
  if(bufnc>10000) break;
  }while(InternetFindNextFile(hff,&wfd));
InternetCloseHandle(hff);
bufnc-=2;
bufnc+=sprintf(buffer+bufnc,"\n%u files",count);
MessageBox(hmwnd,buffer,lBufDir,MB_OK);
}

