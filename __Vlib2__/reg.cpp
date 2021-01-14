#pragma once

#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

//Virtual interface to registry ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class REGISTRY
{
public:
 HKEY root,hkey;
 
 void Close();
 void Root(HKEY);
 BOOL Open(LPSTR,REGSAM); //can be used to test if key exists
 BOOL Key(LPSTR,void*,DWORD,DWORD,REGSAM); //open or create if non existent
 BOOL Val(LPSTR,void*,DWORD,DWORD);
 BOOL SetVal(LPSTR,void*,DWORD,DWORD);
 BOOL GetVal(LPSTR,void*,DWORD);
 BOOL SetStr(LPSTR,LPSTR,DWORD);
 BOOL GetStr(LPSTR,LPSTR,DWORD);
 BOOL DelKey(LPSTR);
 BOOL DelVal(LPSTR);
 BOOL SetDW(LPSTR,DWORD);
 BOOL GetDW(LPSTR,DWORD*);
 BOOL AppKey(REGSAM,LPSTR);
 BOOL AppUserKey(REGSAM,LPSTR);
 REGISTRY();
 ~REGISTRY() { Close(); }
};

// REGISTRY <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//...................................................................................
REGISTRY::REGISTRY()
{
root=HKEY_LOCAL_MACHINE;
hkey=NULL;
}

//...................................................................................
void REGISTRY::Close()
{
RegCloseKey(root);
root=HKEY_LOCAL_MACHINE;
if(hkey) RegCloseKey(hkey);
hkey=NULL;
}

//...................................................................................
void REGISTRY::Root(HKEY hrootkey=NULL)
{
RegCloseKey(root);
if(hrootkey) //set new preopened root
 root=hrootkey;
else if(hkey) //use curent hkey as new root
 {
 root=hkey;
 hkey=NULL;
 }
else
 root=HKEY_LOCAL_MACHINE;
}

//...................................................................................
BOOL REGISTRY::Open(LPSTR kname=NULL,REGSAM sam=KEY_ALL_ACCESS)
{
erret=RegOpenKeyEx(root,kname,0,sam,&hkey);
if(erret==ERROR_SUCCESS) return 1;
return 0;
}

//...................................................................................
inline BOOL REGISTRY::SetVal(LPSTR vname=NULL,void*pvalue=NULL,DWORD valszb=0,DWORD valtype=REG_BINARY)
{
if(!valszb)
 {
 if(valtype==REG_SZ) valszb=sl((LPSTR)pvalue)+1;
 else if(valtype==REG_DWORD) valszb=4;
 else return 0; //
 }
erret=RegSetValueEx(hkey,vname,0,valtype,(BYTE*)pvalue,valszb);
return erret==ERROR_SUCCESS?1:0;
}

//...................................................................................
inline BOOL REGISTRY::GetVal(LPSTR vname=NULL,void*pvalue=NULL,DWORD valszb=0)
{
erret=RegQueryValueEx(hkey,vname,NULL,&REGdump,(BYTE*)pvalue,&valszb);
return erret==ERROR_SUCCESS;
}

//...................................................................................
BOOL REGISTRY::Key(LPSTR kname,void*defval=NULL,DWORD valszb=0,DWORD valtype=REG_SZ,REGSAM sam=KEY_ALL_ACCESS)
{
if(hkey) RegCloseKey(hkey);
hkey=NULL;
erret=RegCreateKeyEx(root,kname,0,NULL,REG_OPTION_NON_VOLATILE,sam,NULL,&hkey,&REGdump);
if(erret!=ERROR_SUCCESS) return 0; //failed
if(defval) SetVal(NULL,defval,valszb,valtype);
return REGdump==REG_CREATED_NEW_KEY?2:1; //created new or opened existing 
}

//...................................................................................
BOOL REGISTRY::Val(LPSTR vname=NULL,void*pvalue=NULL,DWORD valszb=0,DWORD valtype=REG_BINARY)
{
WARN(!valszb,"0 passed as size of buffer in REGISTRY::Val()");
erret=RegQueryValueEx(hkey,vname,NULL,NULL,(BYTE*)pvalue,&valszb);
if(erret==ERROR_SUCCESS) return valszb; //query succeded
SetVal(vname,pvalue,valszb,valtype);
return 0;
}

//...................................................................................
inline BOOL REGISTRY::SetStr(LPSTR vname=NULL,LPSTR lstr=NULL,DWORD valszb=0)
{
if(!valszb) valszb=sl(lstr)+1;
erret=RegSetValueEx(hkey,vname,0,REG_SZ,(BYTE*)lstr,valszb);
return erret==ERROR_SUCCESS?1:0;
}

//...................................................................................
inline BOOL REGISTRY::GetStr(LPSTR vname=NULL,LPSTR lstr=NULL,DWORD valszb=0)
{
WARN(!valszb,"0 passed as size of buffer in REGISTRY::GetStr()");
erret=RegQueryValueEx(hkey,vname,NULL,&REGdump,(BYTE*)lstr,&valszb);
if(erret!=ERROR_SUCCESS) return 0; //query failed
return REGdump==REG_SZ?1:0; //check if it was a string
}

//...................................................................................
inline BOOL REGISTRY::SetDW(LPSTR vname=NULL,DWORD val=0)
{
return RegSetValueEx(hkey,vname,0,REG_DWORD,(BYTE*)&val,4)==ERROR_SUCCESS?1:0;
}

//...................................................................................
inline BOOL REGISTRY::GetDW(LPSTR vname=NULL,DWORD*pval=NULL)
{
DWORD ldwsz=4;
erret=RegQueryValueEx(hkey,vname,NULL,&REGdump,(BYTE*)pval,&ldwsz);
if(erret!=ERROR_SUCCESS) return 0; //query failed
return REGdump==REG_DWORD?1:0; //check if it was a DW
}

//...................................................................................
inline BOOL REGISTRY::DelKey(LPSTR kname=NULL)
{
//return RegDeleteKey(hkey,kname); //doesn't work for WinNT,2000,XP
return SHDeleteKey(hkey,kname);
}

//...................................................................................
inline BOOL REGISTRY::DelVal(LPSTR vname=NULL)
{
return RegDeleteValue(hkey,vname);
}

//...................................................................................
BOOL REGISTRY::AppKey(REGSAM sam=KEY_ALL_ACCESS,LPSTR nameovrd=NULL)
{
int retv;
Close();
smerge(REGs,2,"Software\\" V_COMPANY "\\",nameovrd?nameovrd:AppName);
retv=Key(REGs,NULL,0,0,sam);
if(retv) return retv;
Root(HKEY_CURRENT_USER);
smerge(REGs,2,"Software\\" V_COMPANY "\\",nameovrd?nameovrd:AppName);
return Key(REGs,NULL,0,0,sam);
}

//...................................................................................
BOOL REGISTRY::AppUserKey(REGSAM sam=KEY_ALL_ACCESS,LPSTR nameovrd=NULL)
{
Close();
Root(HKEY_CURRENT_USER);
smerge(REGs,2,"Software\\" V_COMPANY "\\",nameovrd?nameovrd:AppName);
return Key(REGs,NULL,0,0,sam);
}

// REGISTRY >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#ifdef V_THIS_APP
//registers for boot start-up --------------------------------------------------
void AutoStart(int start)
{
REGISTRY rgy;
rgy.Key("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"); //new
if(start)
 {
 GetModuleFileName(GetModuleHandle(NULL),REGs,PATHSZ);
 rgy.SetStr(V_THIS_APP,REGs);
 }
else
 rgy.DelVal(V_THIS_APP);
}
#endif

//creates a file type association --------------------------------------------------
BOOL Associate(LPSTR ext,LPSTR verb=NULL,int mkdef=1,LPSTR icon=NULL,LPSTR progname=NULL)
{
if(*ext!='.')
 {
 error("Associate() called with invalid extension");
 return 0;
 }
char pid[40];
REGISTRY rgy;
rgy.Root(HKEY_CLASSES_ROOT);
if(rgy.Key(ext)==2) //new
 {
 smerge(pid,3,V_COMPANY".",AppName,".1");
 rgy.SetStr(NULL,pid);
 }
else //exist
 rgy.GetStr(NULL,pid,39);
if(progname)
 REGu=sc(strbuf,progname);
else
 REGu=GetModuleFileName(GetModuleHandle(NULL),strbuf,PATHSZ);
if(icon) //new
 {
 smerge(REGs,2,pid,"\\DefaultIcon");
 if(*icon==',') //relative index in file
  {
  rgy.Key(REGs);
  smerge(REGs,2,strbuf,icon);
  rgy.SetStr(NULL,REGs);
  }
 else
  rgy.Key(REGs,icon);
 }
if(!verb) verb="Open";
smerge(REGs,4,pid,"\\Shell\\",verb,"\\command");
if(rgy.Key(REGs)==1&&!progname)//if already exists and not a specific association default verb to AppName
 {
 verb=AppName;
 smerge(REGs,4,pid,"\\Shell\\",verb,"\\command");
 rgy.Key(REGs);
 }
sc(strbuf+REGu," %1");
rgy.SetStr(NULL,strbuf);
if(mkdef)//set new verb as default
 {
 smerge(REGs,2,pid,"\\Shell");
 rgy.Key(REGs,verb);
 }
rgy.Close();
return 1;
//SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST,NULL,NULL);
}

//register exe in AppPaths ()-----------------------------------------------------------
void RegEnvPath(LPSTR dllpath=NULL)
{
REGISTRY rgy;
LPSTR exename;
GetModuleFileName(GetModuleHandle(NULL),strbuf,PATHSZ);
PathNameExt(strbuf,NULL,&exename);
smerge(REGs,2,"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\",exename);
rgy.Key(REGs,strbuf);
if(dllpath) rgy.SetStr("Path",dllpath); //this is used to find DLLs
rgy.Close();
}

//creates in registry the directory structure -------------------------------------
void CreateRegDirStructure(LPSTR shared=NULL,LPSTR useropt=NULL)
{
REGISTRY rgy;
rgy.Key("Software\\" V_COMPANY);
if(shared) rgy.SetStr("GlobalResDir",shared);
rgy.Close();
rgy.Root(HKEY_CURRENT_USER);
rgy.Key("Software\\" V_COMPANY);
if (useropt) rgy.SetStr("UserResDir",useropt);
rgy.Close();
}
