//VERSION 1.0.a
#pragma once
#define V_WIA

#include <comutil.h>
#include <wia.h>
#include <sti.h>
#include <Wiavideo.h>

#pragma comment(lib,"Wiaguid.lib")
#pragma comment(lib,"comsupp.lib")

IWiaDevMgr*wiaman=NULL;
IWiaItem*wiadev;
//IWiaVideo*wiavid=NULL;


//---------------------------------------------------------------------------------------------
BOOL InitWIA()
{
CoInitialize(NULL);
erret=S_OK;
erret=CoCreateInstance(CLSID_WiaDevMgr,NULL,CLSCTX_LOCAL_SERVER,IID_IWiaDevMgr,(void**)&wiaman);
isNULL(wiaman,"!wiaman");
return 1;
}

//---------------------------------------------------------------------------------------------
void WIA_UISel()
{
_bstr_t devid;
erret=wiaman->SelectDeviceDlg(hmwnd,StiDeviceTypeDefault,WIA_SELECT_DEVICE_NODEFAULT,devid.GetAddress(),&wiadev);
status("%s",(char*)devid);
isCOERR("WIA UI");

}

//---------------------------------------------------------------------------------------------
void WIA_UI()
{
_bstr_t devid;
devid="c:\\temp\\wia.bmp";
GUID format=CLSID_NULL;
erret=wiaman->GetImageDlg(hmwnd,StiDeviceTypeDefault,WIA_SELECT_DEVICE_NODEFAULT,WIA_INTENT_IMAGE_TYPE_COLOR,wiadev,devid,&format);
status("%s",(char*)devid);
isCOERR("WIA UI");
}

//---------------------------------------------------------------------------------------------
void WIA_Prev()
{
IEnumWIA_DEV_INFO*wiaenumdev;
    IWiaVideo           *pWiaVideo          = NULL;
    IWiaItem            *pRootItem          = wiadev;
    IWiaPropertyStorage *pIWiaPropStorage   = NULL;
    ULONG               ulFetched           = NULL;
    BSTR                DeviceID            = NULL;
    BSTR                ImagesDirectory     = NULL;
    BSTR                FileName            = NULL;
    BOOL                bFound              = FALSE;

erret=CoCreateInstance(CLSID_WiaVideo,NULL,CLSCTX_INPROC_SERVER,IID_IWiaVideo,(void**)&pWiaVideo);
//isNULL(wiavid,"!wiavid");

erret=wiaman->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL,&wiaenumdev);
if(SUCCEEDED(erret))
 {
 erret=wiaenumdev->Reset();
 }
while((SUCCEEDED(erret))&&(!bFound))
 {
 IWiaPropertyStorage*pIWiaPropStg=NULL;
 erret=wiaenumdev->Next(1,&pIWiaPropStg,&ulFetched);
 if(erret==S_OK)
  {
  PROPSPEC    PropSpec[2];
  PROPVARIANT PropVar[2];
  memset(PropVar,0,sizeof(PropVar));
  PropSpec[0].ulKind = PRSPEC_PROPID;
  PropSpec[0].propid = WIA_DIP_DEV_ID;
  PropSpec[1].ulKind = PRSPEC_PROPID;
  PropSpec[1].propid = WIA_DIP_DEV_TYPE;
  erret=pIWiaPropStg->ReadMultiple(sizeof(PropSpec)/sizeof(PROPSPEC),PropSpec,PropVar);
  if (GET_STIDEVICE_TYPE(PropVar[1].lVal) == StiDeviceTypeStreamingVideo)
   {
   DeviceID = PropVar[0].bstrVal;
   bFound = TRUE;
   }
  }
 if (pIWiaPropStg)
  {
  pIWiaPropStg->Release();
  pIWiaPropStg = NULL;
  }
 }
if(!bFound)
 {
 error("Didn't find any video capable");
 }
HRESULT hr=erret; 
if (SUCCEEDED(hr))
 {
 hr = wiaman->CreateDevice(DeviceID,&pRootItem);
 if (SUCCEEDED(hr))
  {
  hr = pRootItem->QueryInterface(IID_IWiaPropertyStorage,(void**)&pIWiaPropStorage);
  }
 }
   if (SUCCEEDED(hr))
    {
        PROPSPEC    PropSpec2[1];
        PROPVARIANT PropVar2[1];
        memset(PropVar2, 0, sizeof(PropVar2));
        PropSpec2[0].ulKind = PRSPEC_PROPID;
        PropSpec2[0].propid = WIA_DPV_IMAGES_DIRECTORY;
        hr = pIWiaPropStorage->ReadMultiple(sizeof(PropSpec2)/sizeof(PROPSPEC),PropSpec2,PropVar2);
        if (SUCCEEDED(hr))
        {
         ImagesDirectory = PropVar2[0].bstrVal;
		 printbox("%s",(char*)ImagesDirectory);
        }
    }
   if (SUCCEEDED(hr))
    {
    hr = pWiaVideo->put_ImagesDirectory(ImagesDirectory);
    }
    if (SUCCEEDED(hr))
    {
        hr = pWiaVideo->CreateVideoByWiaDevID(DeviceID, hmwnd, FALSE, TRUE);
    }
    if (SUCCEEDED(hr))
    {
        hr = pWiaVideo->TakePicture(&FileName);
    }
    if (pWiaVideo)
    {
        pWiaVideo->DestroyVideo();
    }
}

//---------------------------------------------------------------------------------------------
void FreeWIA()
{
//RelCOM(wiavid,"wiavid");
RelCOM(wiadev,"wiadev");
RelCOM(wiaman,"wiaman");
CoUninitialize();
}