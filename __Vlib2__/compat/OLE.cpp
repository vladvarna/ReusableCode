#ifndef V_COM_OLE
#define V_COM_OLE

#include <win.cpp>
#include <ole2.h>
#include <olectl.h>
#include <initguid.h>

//import/export macros
#define COMAPI_IMP extern __declspec(dllimport) HRESULT STDAPICALLTYPE
#define COMAPI_EXP extern __declspec(dllexport) HRESULT STDAPICALLTYPE

#define COALLOC(bsize)        CoTaskMemAlloc(bsize)
#define COREALLOC(pmem,bsize) CoTaskMemRealloc(pmem,bsize)
#define COFREE(pmem)          if(pmem) CoTaskMemFree(pmem),pmem=NULL

WCHAR REGsw[SBUFSZ]; //register used for string conversions

//sets up OLE environament ----------------------------------------------------------
inline void InitOLE()
{
OleInitialize(NULL);
}

//releases OLE environament ----------------------------------------------------------
inline void FreeOLE()
{
OleUninitialize();
}

//creates registry infrastructure for a COM object -------------------------------
BOOL RegCOM(LPSTR name,CLSID clsid=CLSID_NULL,int ins=0,int vers=1)
{
if(!name) return 0;
char scid[80],spid[40],svid[40]; //max is 39 for any of this
smerge(svid,2,V_COMPANY".",name);
sprintf(spid,"%s.%u",svid,vers);
StringFromGUID2(clsid,(LPOLESTR)scid,255);
W_A(scid,(WCHAR*)scid);
REGu=GetModuleFileName(appinst,strbuf,PATHSZ);
REGISTRY rgy;

smerge(REGs,2,"Software\\Classes\\",spid);
rgy.Key(REGs,name); //exists
rgy.Root(); //root=HKEY_LOCAL_MACHINE\Software\Classes\<ProgID> 
rgy.Key("CLSID",scid);
if(ins) rgy.Key("Insertable");
rgy.Key("Shell");
rgy.Root(); //root=HKEY_LOCAL_MACHINE\Software\Classes\<ProgID>\Shell
smerge(REGs,2,strbuf," %1");
rgy.Key("Open\\command",REGs);
rgy.Close();

smerge(REGs,2,"Software\\Classes\\",svid);
rgy.Key(REGs,name);
rgy.Root();//root=HKEY_LOCAL_MACHINE\Software\Classes\<VersIndepProgID> 
rgy.Key("CLSID",scid);
rgy.Key("CurVer",spid);
rgy.Close();

if(IsEqualCLSID(clsid,CLSID_NULL)) return 1; //no CLSID entry needed
smerge(REGs,2,"Software\\Classes\\CLSID\\",scid);
rgy.Key(REGs,name);
rgy.Root();//root=HKEY_LOCAL_MACHINE\Software\Classes\CLSID\{CLSID}
rgy.Key("InprocServer32",strbuf);
rgy.Key("ProgID",spid);
if(ins) rgy.Key("Insertable");
rgy.Close();
return 2;
}

//deletes registry infrastructure for a COM object -------------------------------
BOOL UnRegCOM(LPSTR name,CLSID clsid=CLSID_NULL,int vers=1)
{
if(!name) return 0;
char scid[80],spid[40],svid[40]; //max is 39 for any of this
smerge(svid,2,V_COMPANY".",name);
sprintf(spid,"%s.%u",svid,vers);
REGISTRY rgy;
rgy.Key("Software\\Classes");
rgy.DelKey(spid);
rgy.DelKey(svid);
if(IsEqualCLSID(clsid,CLSID_NULL)) return 1; //no CLSID entry needed
StringFromGUID2(clsid,(LPOLESTR)scid,255);
W_A(scid,(WCHAR*)scid);
rgy.Key("Software\\Classes\\CLSID");
rgy.DelKey(scid);
rgy.Close();
return 2;
}

struct OLEComponent
{
 CLSID clsid;
 char name[24]; //max ProgID is 39
 int vers,insert;
 int links,locks;

 OLEComponent(LPSTR lname=NULL,CLSID lclsid=CLSID_NULL)
  {
  sc(name,lname?lname:AppName);
  CopyMemory(&clsid,&lclsid,sizeof(CLSID));
  links=locks=0;
  vers=1;
  insert=0;
  }
 BOOL Reg()
  {
  return RegCOM(name,clsid,insert,vers);
  }
 BOOL UnReg()
  {
  return UnRegCOM(name,clsid,vers);
  }
};


#endif