#pragma once
#define V_FILE_IO_INTERFACE

//File Usage:
#define FU_RN        0x04000000 //auto rename file if exist
#define FU_O         0x08000000 //delete file if exist
#define FU_W         0x40000000 //==GENERIC_WRITE (fail if exist) CREATE_NEW
#define FU_R         0x80000000 //==GENERIC_READ (must exist) OPEN_EXISTING
#define FU_RW        0xC0000000 //read and write (open existing or create new) OPEN_ALWAYS
#define FU_WO        0x48000000 //write (del if exist) CREATE_ALWAYS
#define FU_WA        0x44000000 //write (auto rename if exist) CREATE_ALWAYS
#define FU_RWO       0xC8000000 //read and write (del if exist) CREATE_ALWAYS
//File Flags
#define FF_WAIT      0x00040000 //wait until file is available
#define FF_CRITICAL  0x00020000 //return handle or exit program
#define FF_SILENT    0x00010000 //no user input

//Virtual interface to file ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class IOSFile
{
public:
 HANDLE hfile;
 char filepath[PATHSZ];
 NAT fp; //file pointer
 NAT amask; //align mask
 
 IOSFile();
 FAIL open(LPSTR,FLAGS,DWORD);
 void close();
 void setfp(int);
 void skip(int);
 void resize(NAT); //set eof
 void cut(NAT,NAT);
 void ins(NAT,NAT);
 NAT size(); //get eof
 NAT read(void*,NAT);
 NAT write(void*,NAT);
 NAT scantext(char*,NAT,char); //read filtered (>=' ') chars
 NAT scanline(char*,NAT,char);
 NAT printf(char*,...);
 void wdw(DWORD); //write DW
 DWORD rdw(); //read DW
 //functions for block access mode
 NAT put(void*,NAT); //R5
 NAT get(void**); //R1
 //helpers
 void* loadfile(LPSTR,NAT*); //must FREE memory
 ~IOSFile();
};

//Virtual interface to permanent storage system ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class IOSystem
{
public:
 char CurDir[PATHSZ]; //current directory
 char AppDir[PATHSZ]; //directory where exe is
 char ResDir[PATHSZ]; //global resources
 char OptDir[PATHSZ]; //user settings
 NAT CurDirNC,AppDirNC,ResDirNC,OptDirNC;

 IOSystem();
 //~IOSystem();
 void sync();
 BOOL exist(LPSTR);
 BOOL cd(LPSTR); //change dir
 BOOL md(LPSTR); //make dir
 FAIL rd(LPSTR,int); //remove dir
 FAIL del(LPSTR,int); //delete file
 void envpath(LPSTR);
 void dir(); //display content
 void dir_combo(HWND,LPSTR,int);
 FAIL copy(LPSTR,LPSTR,int); //copy file or directory
 //FAIL move(LPSTR,LPSTR,int);
 FAIL ren(LPSTR,LPSTR);
}ios; //default file system manager

//returns a/several Windows usable path(s) from a Vlad path --------------------------------------------------
inline BOOL MetaPath(LPSTR origpath,LPSTR filepath,int step=0)
{
if(*origpath==':') //use predefined search scheme based on second char
 {
 origpath++;
 if(*origpath=='>') //OptDir,ResDir,AppDir
  {
  if(step==0)
   {
   mergepath(filepath,ios.OptDir,origpath+1,'|'); //User
   return 1;
   }
  else if(step==1)
   {
   mergepath(filepath,ios.ResDir,origpath+1,'|'); //Shared
   return 2;
   }
  else
   mergepath(filepath,ios.AppDir,origpath+1,'|'); //App
  }
 else if(*origpath=='<') //ResDir,AppDir
  {
  if(step==0)
   {
   mergepath(filepath,ios.ResDir,origpath+1,'|'); //Shared
   return 1;
   }
  else
   mergepath(filepath,ios.AppDir,origpath+1,'|'); //App
  }
 else if(*origpath=='.') //CurDir
  mergepath(filepath,ios.CurDir,origpath+1,'|');
 else if(*origpath==':') //default R&D directory
  mergepath(filepath,V_DEF_RES,origpath+1,'|');
 else if(*origpath=='#') //Windows directory
  {
  GetWindowsDirectory(filepath,PATHSZ);
  appendpath(filepath,origpath+1,'|');
  }
 }
else if(*origpath=='|') //prefix the application directory (where the exe is)
 mergepath(filepath,ios.AppDir,origpath+1,'|');
else if(*origpath=='<') //prefix the shared resources directory
 mergepath(filepath,ios.ResDir,origpath+1,'|');
else if(*origpath=='>') //prefix the user resources directory
 mergepath(filepath,ios.OptDir,origpath+1,'|');
else //as is
 sc(filepath,origpath,PATHSZ-1,'|');
return 0; //no more options available
}

#define FCLOSE(fis)  if(fis) fclose(fis),fis=NULL //macro to close FILE

//opens a file using predefined paths --------------------------------------------------------------------------------------------
inline FILE* FOPEN(char*path,char*rwamode=NULL)
{
FILE*hfile;
int more=0;
if(!path)
 {
 path=strbuf;
 if(!GetFileName(path,0,hmwnd,NULL,"FOPEN")) return NULL; //canceled by user
 }
do
 {
 more=MetaPath(path,strbuf,REGi);
 if(hfile=fopen(strbuf,rwamode?rwamode:"r+"))
  return hfile;//Ok
 }while(more);
//error(strbuf,0,"Unable to open file");
return NULL; //no file found
}

//returns an unused name in a dir --------------------------------------------------------------------------------------------
int AutoPath(LPSTR unusedpath,LPSTR origpath,LPSTR pref=NULL,NAT maxi=100)
{
int fext=-1;
for(int l=0;l<PATHSZ;l++)
 {
 ifn(origpath[l])
  break;
 if(origpath[l]=='.')
  fext=l;
 } 
if(fext<0) fext=sl(origpath);
sc(unusedpath,origpath,PATHSZ);
erret=GetFileAttributes(unusedpath);
if(erret==-1) return 0; //doesn't exist
for(NAT i=0;i<maxi;i++)
 {
 sprintf(unusedpath+fext,"%s%02u%s",pref?pref:" ",i,origpath+fext);
 erret=GetFileAttributes(unusedpath);
 if(erret==-1) return 1; //doesn't exist
 }
return -1;
}

//returns an unused name in a dir --------------------------------------------------------------------------------------------
int AutoPathSuf(LPSTR unusedpath,LPSTR origpath,LPSTR pref=NULL,NAT maxi=100,NAT sti=0)
{
int fext;
fext=firstch('.',origpath,PATHSZ);
sc(unusedpath,origpath,PATHSZ);
for(NAT i=sti;i<maxi;i++)
 {
 sprintf(unusedpath+fext,"%s%02u%s",pref?pref:" ",i,origpath+fext);
 erret=GetFileAttributes(unusedpath);
 if(erret==-1) return i; //doesn't exist
 }
return -1;
}

//-----------------------------------------------------------------------------------------------
inline void TempPath(LPSTR temppath,LPSTR filename=NULL)
{
GetWindowsDirectory(temppath,PATHSZ);
appendpath(temppath,"TEMP"); //c:\\windows\\temp
if(filename)
 appendpath(temppath,filename);
}

//-----------------------------------------------------------------------------------------------
inline QWORD FileSizeB(LPSTR path)
{
IOSFile iof;
LARGE_INTEGER fszB;
if(iof.open(path,FU_R|FF_SILENT,0))
 return 0;
fszB.LowPart=GetFileSize(iof.hfile,(DWORD*)&fszB.HighPart);
return fszB.QuadPart;
}


// IOSystem <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//init all dirs ..............................................................................................
IOSystem::IOSystem()
{
GetCurrentDirectory(PATHSZ,CurDir);
CurDirNC=sl(CurDir);
GetModuleFileName(GetModuleHandle(NULL),AppDir,PATHSZ);
AppDirNC=lastch('\\',AppDir)-1;
AppDir[AppDirNC]='\0';
REGISTRY rgy;
rgy.Key("Software\\" V_COMPANY);
if(!rgy.GetStr("GlobalResDir",ResDir,PATHSZ))
 sc(ResDir,AppDir);
rgy.Close();
rgy.Root(HKEY_CURRENT_USER);
rgy.Key("Software\\" V_COMPANY);
if(!rgy.GetStr("UserResDir",OptDir,PATHSZ))
 sc(OptDir,AppDir);
rgy.Close();
ResDirNC=sl(ResDir);
OptDirNC=sl(OptDir);
}

//..............................................................................................
void IOSystem::envpath(LPSTR path)
{
char*PATH;
NAT PATHnc,pathnc;
if(!path) return;
pathnc=strlen(path);
PATHnc=GetEnvironmentVariable("PATH",NULL,0);
PATH=SALLOC(PATHnc+pathnc+1);
PATHnc=GetEnvironmentVariable("PATH",PATH,PATHnc);
if(PATHnc>0) PATH[PATHnc++]=';';
CopyMemory(PATH+PATHnc,path,pathnc+1);
erret=SetEnvironmentVariable("PATH",PATH);
FREE(PATH);
}

//sync CurDir ..............................................................................................
inline void IOSystem::sync()
{
GetCurrentDirectory(PATHSZ,CurDir);
CurDirNC=sl(CurDir);
}

//check if exists ...............................................................................................
inline BOOL IOSystem::exist(LPSTR path)
{
erret=GetFileAttributes(path);
if(erret==INVALID_FILE_ATTRIBUTES) return 0; //doesn't exist
if(erret&FILE_ATTRIBUTE_DIRECTORY) return 2; //exist (is directory)
return 1; //exist (is file)
}

//change dir ...................................................................................................
inline BOOL IOSystem::cd(LPSTR path)
{
char lpath[PATHSZ];
REGi=0;
do
 {
 REGi=MetaPath(path,lpath,REGi);
 if(erret=SetCurrentDirectory(lpath)) break;
 }while(REGi);
sync();
return erret; //Ok
}

//make dir ......................................................................................................
BOOL IOSystem::md(LPSTR path)
{
char lBufDir[PATHSZ],lpath[PATHSZ];
DWORD checked=0;
MetaPath(path,lpath,0);
while(lpath[checked])
 {
 checked+=parsepath(lBufDir+checked,lpath+checked);
 if(GetFileAttributes(lBufDir)==-1) //doesn't exist
  {
  if(!CreateDirectory(lBufDir,NULL)) return 0; //failed
  }
 }
return 1; //Ok
}

//delete file ...................................................................................................
/*confirm: b0=confirm dir delete
           b1=confirm file delete
           b2=confirm read only/hidden file deletion 
*/           
inline FAIL IOSystem::del(LPSTR filepath,int confirm=0x7)
{
if(confirm&2)
 {
 REGi=MessageBox(hmwnd,filepath,"Delete file ?",MB_YESNOCANCEL);
 if(REGi==IDNO) return -2; //user denied
 if(REGi==IDCANCEL) return -1; //user cancel
 }
erret=DeleteFile(filepath);
if(erret) return 0; //succes
REGu=GetFileAttributes(filepath);
if(REGu==-1) return 2; //probably doesn't exist
if((REGu&FILE_ATTRIBUTE_DIRECTORY)||(REGu&FILE_ATTRIBUTE_SYSTEM)) return 1; //failed (is dir or system file)
if((REGu&FILE_ATTRIBUTE_READONLY)||(REGu&FILE_ATTRIBUTE_HIDDEN))
 {
 if(confirm&4)
  {
  REGi=MessageBox(hmwnd,filepath,"File is read-only or hidden. Still delete ?",MB_YESNOCANCEL);
  if(REGi==IDNO) return -3; //user denied r-o/h delete
  if(REGi==IDCANCEL) return -1; //user cancel
  }
 REGu&=~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN); //reset r-o and hidden atribs
 if(!SetFileAttributes(filepath,REGu)) return 3; //failed to change atribs
 erret=DeleteFile(filepath);
 if(erret) return 0; //succes
 }
return 4; //failed even after atrib change
}

//remove dir ......................................................................................
//Atentie: functie recursiva !
FAIL IOSystem::rd(LPSTR path,int confirm=0x7)
{
HANDLE hff;
WIN32_FIND_DATA wfd;
NAT lpathnc;
char lBufDir[PATHSZ];
if(confirm&1)
 {
 REGi=MessageBox(hmwnd,path,"Delete directory ?",MB_YESNOCANCEL);
 if(REGi==IDNO) return -2; //user denied
 if(REGi==IDCANCEL) return -1; //user cancel
 }
lpathnc=smerge(lBufDir,2,path,"\\*")-1;
hff=FindFirstFile(lBufDir,&wfd);
if(hff!=INVALID_HANDLE_VALUE)
 {
 do
  {
  sc(lBufDir+lpathnc,wfd.cFileName,PATHSZ);
  if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) //dir
   {
   if(wfd.cFileName[0]!='.') //previne recursivitate infinita
    {
    if(rd(lBufDir,confirm)==-1)
     {
     FindClose(hff);
     return -2; //user cancel
     }
    }
   }
  else //file
   {
   if(del(lBufDir,confirm)==-1)
    {
    FindClose(hff);
    return -2; //user cancel
    }
   }
  }while(FindNextFile(hff,&wfd));
 FindClose(hff);
 }
erret=RemoveDirectory(path);
return !erret; //0-Succes
}

//dir ......................................................................................
void IOSystem::dir()
{
HANDLE hff;
WIN32_FIND_DATA wfd;
NAT count=0,bufnc=0;
char buffer[10100],lBufDir[PATHSZ];
mergepath(lBufDir,CurDir,"*");
hff=FindFirstFile(lBufDir,&wfd);
if(hff==INVALID_HANDLE_VALUE) return;
do{
  count++;
  if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
   bufnc+=sprintf(buffer+bufnc,"[%s], ",wfd.cFileName);
  else
   bufnc+=sprintf(buffer+bufnc,"%s, ",wfd.cFileName);
  if(bufnc>10000) break;
  }while(FindNextFile(hff,&wfd));
FindClose(hff);
bufnc-=2;
bufnc+=sprintf(buffer+bufnc,"\n%u files",count);
MessageBox(hmwnd,buffer,lBufDir,MB_OK);
}

//dir ......................................................................................
void IOSystem::dir_combo(HWND hcbox,LPSTR filter="*",int attribs=0)
{
HANDLE hff;
WIN32_FIND_DATA wfd;
char lBufDir[PATHSZ];
mergepath(lBufDir,CurDir,filter);
hff=FindFirstFile(lBufDir,&wfd);
SendMessage(hcbox,CB_RESETCONTENT,0,0); //remove all
if(hff==INVALID_HANDLE_VALUE) return;
do{
 if(attribs)
  {
  if((wfd.dwFileAttributes&attribs)&&(wfd.cFileName[0]!='.'))
   SendMessage(hcbox,CB_ADDSTRING,0,(LPARAM)wfd.cFileName); //add
  } 
 else
  SendMessage(hcbox,CB_ADDSTRING,0,(LPARAM)wfd.cFileName); //add
 }while(FindNextFile(hff,&wfd));
FindClose(hff);
}

//copy file or directory ..................................................................................
//Atentie: functie recursiva !
FAIL IOSystem::copy(LPSTR spath,LPSTR dpath,int confirm=0x1)
//confirm b0=confirm overwrite
{
HANDLE hff;
WIN32_FIND_DATA wfd;
//NAT lpathnc;
char lSBufDir[PATHSZ],lDBufDir[PATHSZ];
erret=GetFileAttributes(spath);
if(erret==-1) return 2; //source inexistent
pathtransfer(lDBufDir,spath,dpath);
if(erret&FILE_ATTRIBUTE_DIRECTORY)
 {
 if(sroot(spath,dpath)<0) //destination is a subdirectory of source directory
  {
  MessageBox(hmwnd,"Cannot copy a directory to a subdirectory","ERROR",MB_OK);
  return 4; //copying a directory to a subdirectory will cause infinite recursion
  }
 erret=md(lDBufDir);
 mergepath(REGs,spath,"*");
 hff=FindFirstFile(REGs,&wfd);
 if(hff!=INVALID_HANDLE_VALUE)
  {
  do
   {
   if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) //dir
    {
    if(wfd.cFileName[0]!='.') //previne recursivitate infinita
     {
     mergepath(lSBufDir,spath,wfd.cFileName);
     if(copy(lSBufDir,lDBufDir,confirm)==-1)
      {
      FindClose(hff);
      return -1; //user cancel
      }
     }
    }
   else //file
    {
    mergepath(lSBufDir,spath,wfd.cFileName);
    mergepath(REGs,lDBufDir,wfd.cFileName);
    if(!CopyFile(lSBufDir,REGs,confirm&0x1)) //failed
     {
     if((confirm&0x1)&&(GetFileAttributes(lDBufDir)!=-1)) //assume failed because already exists
      {
      REGi=MessageBox(hmwnd,REGs,"File already exists. Overwrite ?",MB_YESNOCANCEL);
      if(REGi==IDNO) continue;
      else if(REGi==IDCANCEL)
       {
       FindClose(hff);
       return -1; //user cancel
       }
      }
     erret=CopyFile(lSBufDir,REGs,0);
     }
    }
   }while(FindNextFile(hff,&wfd));
  FindClose(hff);
  }
 else
  return 3; //couldn't enum dir contents
 }
else //file
 {
 erret=CopyFile(spath,lDBufDir,confirm&0x1);
 if(!erret) //failed
  {
  if((confirm&0x1)&&(GetFileAttributes(lDBufDir)!=-1)) //assume failed because already exists
   {
   REGi=MessageBox(hmwnd,lDBufDir,"File already exists. Overwrite ?",MB_YESNOCANCEL);
   if(REGi==IDNO) return -2; //user denied
   if(REGi==IDCANCEL) return -1; //user cancel
   erret=CopyFile(spath,lDBufDir,0);
   }
  else //assume failed because dpath doesn't exist
   {
   md(dpath);
   erret=CopyFile(spath,lDBufDir,0);
   }
  }
 return !erret; //1=CopyFile failed
 }
return 0; //success
}

//move file or directory ..................................................................................
//FAIL IOSystem::move(LPSTR dpath,LPSTR spath,int confirm)
//{
//return 0;
//}
// IOSystem >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// IOSFile <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//.............................................................................................................
FAIL IOSFile::open(LPSTR path,FLAGS flags=0,DWORD latrib=FILE_ATTRIBUTE_NORMAL)
{
DWORD dispos;
char lpbuf[PATHSZ];
if(hfile&&(hfile!=INVALID_HANDLE_VALUE)) CloseHandle(hfile);
fp=0;
amask=3; //currently all file operations default to DWORD allignement
if(flags&FU_W)
 {
 if((flags&FU_O)&&!(flags&FU_RN)) //WO,RWO
  dispos=CREATE_ALWAYS;
 else if(flags&FU_R) //RW
  dispos=OPEN_ALWAYS;
 else //W
  dispos=CREATE_NEW;
 }
else //R
 dispos=OPEN_EXISTING;
if(!path)
 {
 path=filepath;
 if(!GetFileName(path,0,hmwnd)) return -2; //canceled by user
 }
REGi=0;
do
 {
 REGi=MetaPath(path,filepath,REGi);
 if((hfile=CreateFile(filepath,flags&FU_RW,FILE_SHARE_READ,NULL,dispos,latrib,NULL))!=INVALID_HANDLE_VALUE)
  return 0;//Ok
 }while(REGi);
//error handling __________________________________________________________________________________________
char *fname,*fext;
PathNameExt(filepath,&path,&fname,&fext);
erret=GetFileAttributes(filepath); //-1 => doesn't exists
if(flags&FU_W) //ensure directory exists
 {
 sc(lpbuf,filepath,fname-path-1); //extract path
 ios.md(lpbuf);
 if((hfile=CreateFile(filepath,flags&FU_RW,FILE_SHARE_READ,NULL,dispos,latrib,NULL))!=INVALID_HANDLE_VALUE) return 0;//Ok
 }
if(flags&FF_SILENT) return 1; //caller is responsible for processing error; FF_CRITICAL ignored
if(erret!=-1) //file exists
 {
 if(erret&(FILE_ATTRIBUTE_DEVICE|FILE_ATTRIBUTE_DIRECTORY)) return error("Cannot open a directory",2,"File access",flags&FF_CRITICAL);
 if(flags&FU_W)
  {
  if(flags&FU_RN)
   {
   sc(lpbuf,filepath,PATHSZ);
   AutoPath(filepath,lpbuf,"~");
   hfile=CreateFile(filepath,flags&FU_RW,FILE_SHARE_READ,NULL,CREATE_NEW,latrib,NULL);
   if(hfile!=INVALID_HANDLE_VALUE) return 0;//Ok
   return error("Cannot create",3,"File write access",flags&FF_CRITICAL);
   }
  else
   {
   sprintf(lpbuf,"File %s %s%s exists\nOverwrite ?",filepath,erret&FILE_ATTRIBUTE_READONLY?"(READONLY)":"",erret&FILE_ATTRIBUTE_SYSTEM?"(SYSTEM)":"");
   if(MessageBox(hmwnd,lpbuf,"File write access",MB_ICONWARNING|MB_TASKMODAL|MB_YESNO)==IDYES)
    {
    SetFileAttributes(filepath,latrib);
    hfile=CreateFile(filepath,flags&FU_RW,FILE_SHARE_READ,NULL,CREATE_ALWAYS,latrib,NULL);
    if(hfile!=INVALID_HANDLE_VALUE) return 0;//Ok
    return error("Cannot overwrite",3,"File write access",flags&FF_CRITICAL);
    }
   else if(flags&FF_CRITICAL) exit(-1);
   else return -1; //overwrite denied by user
   }
  }
 sprintf(lpbuf,"File %s %s%s exists, but cannot be opened!",filepath,erret&FILE_ATTRIBUTE_READONLY?"(READONLY)":"",erret&FILE_ATTRIBUTE_SYSTEM?"(SYSTEM)":"");
 return error(lpbuf,4,"File read access",flags&FF_CRITICAL);
 }
else //file doesn't exist
 {
 if(!(flags&FU_O)&&(flags&FU_R)) //file with data 
  {
  if(flags&FF_WAIT)
   {
   NAT k=0;
   do{ //TODO: add a timeout
    Sleep(100);
    hfile=CreateFile(filepath,flags&FU_RW,FILE_SHARE_READ,NULL,dispos,0,NULL);
    k++;
    }while(hfile==INVALID_HANDLE_VALUE&&k<100);
   return 0;//ok
   }
  else
   {
   smerge(lpbuf,2,"Locate file: ",fname);
   if(GetFileName(filepath,OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST,hmwnd,NULL,lpbuf))
    {
    hfile=CreateFile(filepath,flags&FU_RW,FILE_SHARE_READ,NULL,dispos,0,NULL);
    if(hfile!=INVALID_HANDLE_VALUE) return 0;//Ok
    sprintf(lpbuf,"Cannot open file %s",filepath);
    return error(lpbuf,5,"File read access",flags&FF_CRITICAL);
    }
   else if(flags&FF_CRITICAL) exit(-1);
   else return -1; //user canceled open
   }
  }
 sprintf(lpbuf,"Cannot create file %s",filepath);
 return error(lpbuf,6,"File write access",flags&FF_CRITICAL);
 }
}

//.........................................................................................................
inline void IOSFile::close()
{
if(hfile&&(hfile!=INVALID_HANDLE_VALUE)) CloseHandle(hfile);
hfile=INVALID_HANDLE_VALUE;
}

//...........................................................................................................
inline IOSFile::IOSFile() //constructor
{
hfile=INVALID_HANDLE_VALUE;
*filepath='\0';
}

//.............................................................................................................
inline IOSFile::~IOSFile() //destructor
{
if(hfile&&(hfile!=INVALID_HANDLE_VALUE)) CloseHandle(hfile);
}

//.........................................................................................................
inline void IOSFile::setfp(int nfp=0)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
fp=SetFilePointer(hfile,ALIGN(nfp,amask),NULL,FILE_BEGIN);
if(fp==INVALID_SET_FILE_POINTER)
 error("Failed to seek",nfp,filepath);
}

//..........................................................................................................
inline void IOSFile::skip(int Bcount=0)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
fp=SetFilePointer(hfile,ALIGN(Bcount,amask),NULL,FILE_CURRENT);
}

//...........................................................................................................
inline NAT IOSFile::size()
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
return GetFileSize(hfile,NULL);
}

//truncate file............................................................................................................
inline void IOSFile::resize(NAT neof=0)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
if(SetFilePointer(hfile,neof,NULL,FILE_BEGIN)==INVALID_SET_FILE_POINTER)
 error("Failed to seek",neof,filepath);
erret=SetEndOfFile(hfile);
SetFilePointer(hfile,fp,NULL,FILE_BEGIN);
}

//...........................................................................................................
inline void IOSFile::cut(NAT beg=0,NAT end=0)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
BYTE*tempbuf;
NAT l;
l=GetFileSize(hfile,NULL)-end;
tempbuf=(BYTE*)ALLOC(l);
setfp(end);
read(tempbuf,l);
setfp(beg);
write(tempbuf,l);
resize(l+beg+1);
}

//...........................................................................................................
inline void IOSFile::ins(NAT pos=0,NAT cnt=0)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
BYTE*tempbuf;
NAT l;
l=GetFileSize(hfile,NULL)-pos;
tempbuf=(BYTE*)ALLOC(l);
setfp(pos);
read(tempbuf,l);
setfp(pos+cnt);
write(tempbuf,l);
}

//...........................................................................................................
inline NAT IOSFile::read(void*readbuf,NAT Bcount=4)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
erret=ReadFile(hfile,readbuf,Bcount,&REGdump,NULL);
if((fp+=REGdump)&amask)
 SetFilePointer(hfile,ALIGN(fp,amask),NULL,FILE_BEGIN);
//if(REGdump!=Bcount) error("Difference between B read and B requested",Bcount-REGdump,filepath);
return REGdump;
}

//...........................................................................................................
inline NAT IOSFile::write(void*writebuf,NAT Bcount=4)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
erret=WriteFile(hfile,writebuf,Bcount,&REGdump,NULL);
if((fp+=REGdump)&amask)
 SetFilePointer(hfile,ALIGN(fp,amask),NULL,FILE_BEGIN);
//if(REGdump!=Bcount) error("Difference between B written and B sent",Bcount-REGdump,filepath);
return REGdump;
}

//...........................................................................................................
inline NAT IOSFile::scantext(char*readbuf,NAT Bcount=4,char boundary=' ')
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
char lc;
NAT Bread=0;
amask=0; //automatically char align
do{	//skip starting chars
 if(!ReadFile(hfile,&lc,1,&REGdump,NULL)) return 0;
 if(!REGdump) return 0;
 fp++;
 }while(lc<boundary);
*readbuf=lc;
do{	//read
 Bread++;
 if(!ReadFile(hfile,readbuf+Bread,1,&REGdump,NULL)) break;
 }while(REGdump&&readbuf[Bread]>=boundary&&Bread<Bcount);
Bread;
fp+=Bread-1;
return Bread;
}

//...........................................................................................................
inline NAT IOSFile::scanline(char*readbuf,NAT Bcount=4,char termin='\xD')
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
NAT Bread=0;
amask=0; //automatically char align
do{	//read
 if(!ReadFile(hfile,readbuf+Bread,1,&REGdump,NULL)) break;
 if(!REGdump) break;
 Bread++;
 if(readbuf[Bread-1]==termin) break;
 }while(Bread<Bcount);
Bread;
fp+=Bread-1;
return Bread;
}

//...........................................................................................................
inline NAT IOSFile::printf(char *formstr,...)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
NAT len;
char buffer[BUFFSZ];
va_list vparam;
va_start(vparam,formstr);
len=vsprintf(buffer,formstr,vparam);
amask=0; //automatically char align
len=write(buffer,len);
va_end(vparam);
return len;
}

//...........................................................................................................
inline DWORD IOSFile::rdw()
{
DWORD retv;
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
erret=ReadFile(hfile,&retv,4,&REGdump,NULL);
if((fp+=REGdump)&amask)
 SetFilePointer(hfile,ALIGN(fp,amask),NULL,FILE_BEGIN);
return retv;
}

//...........................................................................................................
inline void IOSFile::wdw(DWORD dw)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
erret=WriteFile(hfile,&dw,4,&REGdump,NULL);
if((fp+=REGdump)&amask)
 SetFilePointer(hfile,ALIGN(fp,amask),NULL,FILE_BEGIN);
}

//...........................................................................................................
inline NAT IOSFile::get(void**readbuf)
{
NAT Bcount;
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
ReadFile(hfile,&Bcount,4,&REGdump,NULL);
FREE(*readbuf);
*readbuf=ALLOC(Bcount);
erret=ReadFile(hfile,*readbuf,Bcount,&REGdump,NULL);
if((fp+=Bcount+4)&amask)
 SetFilePointer(hfile,ALIGN(fp,amask),NULL,FILE_BEGIN);
return REGdump;
}

//...........................................................................................................
inline NAT IOSFile::put(void*writebuf,NAT Bcount=4)
{
WARN(!hfile||(hfile==INVALID_HANDLE_VALUE),"Invalid file handle used")
WriteFile(hfile,&Bcount,4,&REGdump,NULL);
erret=WriteFile(hfile,writebuf,Bcount,&REGdump,NULL);
if((fp+=Bcount+4)&amask)
 SetFilePointer(hfile,ALIGN(fp,amask),NULL,FILE_BEGIN);
return REGdump;
}

//...........................................................................................................
inline void*IOSFile::loadfile(LPSTR path,NAT*pBcount=NULL)
{
NAT filesize;
void*filebuffer;
if(open(path,FU_R)) return NULL;
filesize=size();
if(!filesize) return NULL;
filebuffer=ALLOC(filesize+4);
if(!filebuffer) return NULL;
read(filebuffer,filesize);
if(pBcount) *pBcount=filesize;
return filebuffer;
}

// IOSFile >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>