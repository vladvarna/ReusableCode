#ifndef V_TWAIN
#define V_TWAIN

#include <ext/twain.h> //run-time link to TWAIN_32.DLL

DSMENTRYPROC DSMEntry=NULL; //Data Source Manager entry point
HMODULE hTwainDSM=NULL; //handle to TWAIN_32.DLL
TW_UINT16 TWret; //standard return code
TW_IDENTITY TWThisApp; //used as parameter for DSMEntry, identifies the origin as been this application
TW_IDENTITY TWSrc; //Data Source used for acquire
int TWlinks=0; //reference count

//test TWret for failure ---------------------------------------------------------------
int isTWERR(LPSTR errmsg=NULL)
{
if(TWret==TWRC_FAILURE)
 error(errmsg,TWret,"TWAIN ERROR");
return TWret;
}

//loads DLL and opens DSM -----------------------------------------------------------
int InitTWAIN()
{
if(!hTwainDSM) hTwainDSM=LoadLibrary("TWAIN_32.DLL");
if(!hTwainDSM)
 {
 error("Linking to TWAIN_32.DLL failed");
 return 0;
 }
TWlinks++;
if(DSMEntry) return 1; //already initialized
DSMEntry=(DSMENTRYPROC)GetProcAddress(hTwainDSM,MAKEINTRESOURCE(1));
if(!DSMEntry)
 {
 FreeLibrary(hTwainDSM);
 hTwainDSM=NULL;
 error("Can't find entry point in DSM");
 return 0;
 }
TWThisApp.Id=0; // Source Manager will assign real value
TWThisApp.Version.MajorNum=1;
TWThisApp.Version.MinorNum=0;
TWThisApp.Version.Language=TWLG_ENGLISH_USA;
TWThisApp.Version.Country=TWCY_ROMANIA;
sc(TWThisApp.Version.Info,"version 1.0");
TWThisApp.ProtocolMajor=TWON_PROTOCOLMAJOR;
TWThisApp.ProtocolMinor=TWON_PROTOCOLMINOR;
TWThisApp.SupportedGroups=DG_IMAGE|DG_CONTROL;
sc(TWThisApp.Manufacturer,V_COMPANY);
sc(TWThisApp.ProductFamily,"ErgoEd2D");
sc(TWThisApp.ProductName,AppName);
TWret=DSMEntry(&TWThisApp,NULL,DG_CONTROL,DAT_PARENT,MSG_OPENDSM,&hmwnd);
isTWERR("Open DSM");
TWSrc.Id=0;
TWSrc.ProductName[0]='\0';
TWret=DSMEntry(&TWThisApp,NULL,DG_CONTROL,DAT_IDENTITY,MSG_GETDEFAULT,&TWSrc);
return 1;
}

//closes DSM and unloads DLL -------------------------------------------------------------
int FreeTWAIN()
{
TWlinks--;
if(TWlinks) return TWlinks;
if(DSMEntry)
 {
 TWret=DSMEntry(&TWThisApp,NULL,DG_CONTROL,DAT_PARENT,MSG_CLOSEDSM,&hmwnd);
 isTWERR("Close DSM");
 }
if(hTwainDSM)
 {
 if(FreeLibrary(hTwainDSM))
  {
  hTwainDSM=NULL;
  DSMEntry=NULL;
  }
 }
return 0;
}

//displays the TWAIN Source select dialog -------------------------------------------------------
int TWSource()
{
if(!DSMEntry)
 {
 error("TWAIN not initialized\nCall InitTWAIN()");
 return 0;
 }
TWSrc.Id=0;
TWSrc.ProductName[0]='\0';
TWret=DSMEntry(&TWThisApp,NULL,DG_CONTROL,DAT_IDENTITY,MSG_USERSELECT,&TWSrc);
isTWERR("Select DS");
return 1;
}

//reads nri images from DS as memory handles to packed DIBs -----------------------------------------------------
NAT TWGetNative(HGLOBAL*phdib=NULL,NAT nri=1)
{
TW_PENDINGXFERS twpx;
NAT ind=0;
twpx.Count=0;
TWret=DSMEntry(&TWThisApp,&TWSrc,DG_CONTROL,DAT_PENDINGXFERS,MSG_GET,&twpx);
isTWERR("Get pending");
if(!phdib) //alloc memory
 {
 if(twpx.Count>0) nri=twpx.Count;
 phdib=(HGLOBAL*)malloc(nri*sizeof(HGLOBAL));
 }
while(twpx.Count!=0&&ind<nri)
 {
 //TW_IMAGEINFO twii;
 //TWret=DSMEntry(&TWThisApp,&TWSrc,DG_IMAGE,DAT_IMAGEINFO,MSG_GET,&twii);
 //isTWERR("Get image info");
 //dump(TW_IMAGEINFO_MAP,&twii);
 phdib[ind]=NULL;
 TWret=DSMEntry(&TWThisApp,&TWSrc,DG_IMAGE,DAT_IMAGENATIVEXFER,MSG_GET,&phdib[ind]);
 switch(TWret)
  {
  case TWRC_XFERDONE:
   ind++;
   break;
  case TWRC_CANCEL:
   GlobalFree(phdib[ind]);
   phdib[ind]=NULL;
   error("Acquisition canceled",TWret,"TWAIN");
   break;
  case TWRC_FAILURE:
   error("Acquisition failed",TWret,"TWAIN");
   break;
  }
 TWret=DSMEntry(&TWThisApp,&TWSrc,DG_CONTROL,DAT_PENDINGXFERS,MSG_ENDXFER,&twpx);
 if(TWret!=TWRC_SUCCESS)
  twpx.Count=0;
 }
if(twpx.Count) //discard all remaining images
 TWret=DSMEntry(&TWThisApp,&TWSrc,DG_CONTROL,DAT_PENDINGXFERS,MSG_RESET,&twpx);
return ind;
}

//acquires data from DS --------------------------------------------------------------
NAT TWAcquire(HGLOBAL*phdib=NULL,NAT nri=1,int showui=0x1)
{
MSG msg;
TW_EVENT twev;
TW_USERINTERFACE twui;
int Block=1,retv=0;
if(!DSMEntry) return 0; //must have called InitTWAIN() before
if(showui&0x4) TWSource();
TWret=DSMEntry(&TWThisApp,NULL,DG_CONTROL,DAT_IDENTITY,MSG_OPENDS,&TWSrc);
isTWERR("Open DS");
twui.ShowUI=showui&0x1;
twui.ModalUI=showui&0x2;
twui.hParent=hmwnd;
TWret=DSMEntry(&TWThisApp,&TWSrc,DG_CONTROL,DAT_USERINTERFACE,MSG_ENABLEDS,&twui);
isTWERR("Enable DS");
twev.pEvent=&msg;
do{
 if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
  {
  if(msg.message==WM_QUIT)
   {
   Block=0;
   PostQuitMessage(msg.wParam);
   }
  else
   {
   TWret=DSMEntry(&TWThisApp,&TWSrc,DG_CONTROL,DAT_EVENT,MSG_PROCESSEVENT,&twev);
   if(TWret==TWRC_NOTDSEVENT)
    DispatchMessage(&msg);
   else if(twev.TWMessage!=MSG_NULL)
    {
    switch(twev.TWMessage)
     {
     case MSG_XFERREADY:
      retv=TWGetNative(phdib,nri);
     case MSG_CLOSEDSOK: //?
     case MSG_CLOSEDSREQ:
      Block=0;
      break;
     case MSG_DEVICEEVENT:
      break;
     }
    }
   }
  }
}while(Block);
TWret=DSMEntry(&TWThisApp,&TWSrc,DG_CONTROL,DAT_USERINTERFACE,MSG_DISABLEDS,&twui);
isTWERR("Disable DS");
TWret=DSMEntry(&TWThisApp,NULL,DG_CONTROL,DAT_IDENTITY,MSG_CLOSEDS,&TWSrc);
isTWERR("Close DS");
return retv;
}

#endif