#ifndef V_DXSHOW8
#define V_DXSHOW8

#include <dshow.h>

#pragma comment(lib,"Strmiids.lib") //GUIDs
#pragma comment(lib,"Strmbasd.lib") //base filter creation classes
#pragma comment(lib,"Quartz.lib") //AMGetErrorText() only

#define WM_DSHOWGRAPH  WM_APP+1 //message used to notify of status change in dhgb

//Note: For DShow (aka Active Movie) the prefix will be "dh" (becouse the "ds" prefix is used for DSound) 
IGraphBuilder* dhgb=NULL; //main graph builder
IVideoWindow*  dhvw=NULL; //main video window
IMediaControl* dhmc=NULL; //main media control
IMediaEventEx* dhme=NULL; //main media event (extended)

//tests erret -----------------------------------------------------------------------------
int ERRDShow(LPSTR errmsg=NULL)
{
if(!HRESULT_SEVERITY(erret)) return 1;
AMGetErrorText(erret,REGs,256);
sprintf(strbuf,"%x:%s\n%s\nContinue ?",erret,REGs,errmsg);
if(MessageBox(hmwnd,strbuf,"DXShow8 ERROR",MB_ICONERROR|MB_YESNO|MB_DEFBUTTON2)==IDNO)
 exit(1);
return 0;
}

//initializes Direct Show engine -------------------------------------------------------
void InitDShow()
{
CoInitialize(NULL);
erret=CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void**)&dhgb);
ERRDShow("Getting main Filter Graph");
erret=dhgb->QueryInterface(IID_IVideoWindow,(void**)&dhvw);
ERRDShow("Getting main Video Window");
erret=dhgb->QueryInterface(IID_IMediaControl,(void**)&dhmc);
ERRDShow("Getting main Media Control");
erret=dhgb->QueryInterface(IID_IMediaEventEx,(void**)&dhme);
ERRDShow("Getting main Media Event");
erret=dhme->SetNotifyWindow((OAHWND)hmwnd,WM_DSHOWGRAPH,0);
ERRDShow("SetNotifyWindow()");
}

//frees Direct Show --------------------------------------------------------------------
void FreeDShow()
{
RelCOM(dhme,"main DShow Media Event");
dhme=NULL;
RelCOM(dhmc,"main DShow Media Control");
dhmc=NULL;
RelCOM(dhvw,"main DShow Video Window");
dhvw=NULL;
RelCOM(dhgb,"main DShow Filter Graph");
dhgb=NULL;
CoUninitialize();
}

#endif