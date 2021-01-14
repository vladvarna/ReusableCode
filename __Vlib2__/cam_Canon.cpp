//VERSION 1.0.a
#pragma once
#define V_CANON_CAM	//based on CDSDK (Canon Digital SDK)

//#include <vfw.h>
#include <udata.cpp>

#include <cdAPI.h>
#pragma comment(lib,"cdSDK.lib")

cdVersionInfo	CDSDKversion;

#define	VIEWFINDER_WIDTH		320
#define	VIEWFINDER_HEIGHT		240

#define	FILEFORMAT_JPG			0
#define	FILEFORMAT_BMP			1

#define cderror(cderrid,mesaj,arg0) if((cdERROR_ERRORID_MASK&cderrid)!=cdOK){if(exitproc("%I64x\n"mesaj,cderrid,arg0)==IDYES) return;}
#define cderror_(cderrid,mesaj,arg0) if((cdERROR_ERRORID_MASK&cderrid)!=cdOK){if(exitproc("%I64x\n"mesaj,cderrid,arg0)==IDYES) return 0;}

//status
#define CANON_VF					   0x1 //VF is started
#define CANON_VF_LCD 			       0x2 //VF to LCD
//flags
#define CANON_VF_SAVE			0x80000000 //save data VF

class CanonCam
{
public:
 BITS32 flags;
 cdError err;
 cdHSource hSource;
 cdHVolume hVol;
 cdUInt32 maxoptzoom,maxzoom,zoom;  //
 HWND hvfwnd;
 //BMPImage*pbmp;
 
 //////////////////////////////
 CanonCam() { ZEROCLASS(CanonCam); }
 ~CanonCam() { Free(); }
 void Free();
 //remote release
 BOOL SelSource(); //select camera (if more than 1) and enter remote release
 BOOL StartVF(HWND);
 BOOL StopVF();
 void VFtoLCD();
 BOOL Release(LPSTR); //take picture
 BOOL SelMode();
 BOOL SelAFdist();
 BOOL SelAv();
 void AutoEF();
 void SetMode(cdShootingMode);
 void SetAFd(cdAFDistance);
 void SetFlash(cdFlashMode,cdCompensation);
 void GetZoom(cdURational*);
 void SetZoom(cdUInt32);
 void GetSize(cdCompQuality&,cdImageSize&);
 void SetSize(cdCompQuality,cdImageSize);
 //image retrieval
 BOOL SelVolume();
 //callbacks
 static cdUInt32 cdSTDCALL ViewFinderCallBackProc(cdVoid*,cdUInt32,cdUInt32,cdContext);
 void DBG();
};

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//............................................................................................ 
void CanonCam::Free()
{
if(flags&CANON_VF)
 StopVF();
if(hSource)
 {
 err=CDExitReleaseControl(hSource);
 err=CDCloseSource(hSource);
 cderror(err,"CDCloseSource()",0);
 hSource=NULL;
 }
}

//............................................................................................ 
BOOL CanonCam::SelSource()
{
cdHEnum	hEnumDevice;
cdUInt32 count,i;
UDATA ud;
err=CDEnumDeviceReset(1,&hEnumDevice);
cderror_(err,"CDEnumDeviceReset()",0);
err=CDGetDeviceCount(hEnumDevice, &count);
cderror_(err,"CDGetDeviceCount()",0);
exitif_(count<=0,0,"No Canon cameras",0);
ud.init(UDT_BINARY,count,sizeof(cdSourceInfo));
for(i=0;i<count;i++)
 CDEnumDeviceNext(hEnumDevice,(cdSourceInfo*)ud.getit(i));
err=CDEnumDeviceRelease(hEnumDevice);
i=ud.choose("Choose camera",hmwnd,8);
err=CDOpenSource((cdSourceInfo*)ud.getit(i),&hSource);
cderror_(err,"Couldn't open source()",0);
//err=CDEnterReleaseControl(hSource,RelCallBackFunc,(cdContext)this);
err=CDEnterReleaseControl(hSource,NULL,NULL);
cderror_(err,"Couldn't enter release control",0);
err=CDSelectReleaseDataKind(hSource,cdREL_KIND_PICT_TO_PC);
cderror_(err,"Couldn't set release mode",0);
err=CDGetMaximumZoomPos(hSource,&maxzoom,&maxoptzoom);
return 1;
}

//auto exposure, auto focus ............................................................................................ 
BOOL CanonCam::StartVF(HWND hwnd)
{
cdStgMedium	VFMedium;
VFMedium.Type=cdMEMTYPE_STREAM;
if(hwnd) hvfwnd=hwnd;
err=CDStartViewfinder(hSource,FILEFORMAT_BMP,ViewFinderCallBackProc,(cdContext)this);
cderror_(err,"CDStartViewfinder() failed",0);
flags|=CANON_VF;
return 1;
}

//auto exposure, auto focus ............................................................................................ 
BOOL CanonCam::StopVF()
{
err=CDTermViewfinder(hSource);
cderror_(err,"CDTermViewfinder() failed",0);
flags&=~CANON_VF;
return 1;
}

//............................................................................................ 
inline void CanonCam::VFtoLCD()  //send vf to LCD also
{
if(flags&CANON_VF_LCD)
 err=CDSelectViewFinderCameraOutput(hSource,cdREL_FINDER_TO_OFF);
else
 err=CDSelectViewFinderCameraOutput(hSource,cdREL_FINDER_TO_LCD);
flags^=CANON_VF_LCD;
}

//............................................................................................ 
BOOL CanonCam::Release(LPSTR path)
{
cdUInt32 NumData;
cdStgMedium medium;
cdReleaseImageInfo rii;
err=CDRelease(hSource,FALSE,NULL,NULL,cdPROG_NO_REPORT,&NumData);
cderror_(err,"CDRelease() failed",0);
while(NumData)
 {
 medium.Type=cdMEMTYPE_FILE;
 medium.u.lpszFileName=path;
 err=CDGetReleasedData(hSource,NULL,0,cdPROG_NO_REPORT,&rii,&medium);
 cderror_(err,"CDGetReleaseData() failed",0);
 NumData--;
 }
return 1;
}

//............................................................................................ 
BOOL CanonCam::SelMode()
{
cdHEnum	hEnum;
cdUInt32 count,i;
UDATA ud,udn;
cdShootingMode mode;
err=CDEnumShootingModeReset(hSource,&hEnum);
cderror_(err,"CDEnumShootingModeReset()",0);
err=CDGetShootingModeCount(hEnum,&count);
cderror_(err,"CDGetShootingModeCount()",0);
exitif_(count<=0,0,"No shooting modes",0);
ud.init(UDT_NAT,count);
udn.init(UDT_CPSTR,count);
for(i=0;i<count;i++)
 {
 CDEnumShootingModeNext(hEnum,&mode);
 ud.set(i,(__int64)mode);
 switch(mode)
    {
	case cdSHOOTING_MODE_INVALID:		udn.set(i, "(Invalid)" );		break;
	case cdSHOOTING_MODE_AUTO:			udn.set(i, "Auto" );			break;
	case cdSHOOTING_MODE_PROGRAM:		udn.set(i, "Program" );			break;
	case cdSHOOTING_MODE_TV:			udn.set(i, "Tv" );				break;
	case cdSHOOTING_MODE_AV:			udn.set(i, "Av" );				break;
	case cdSHOOTING_MODE_MANUAL:		udn.set(i, "Manual" );			break;
	case cdSHOOTING_MODE_A_DEP:			udn.set(i, "A_DEP" );			break;
	case cdSHOOTING_MODE_M_DEP:			udn.set(i, "M_DEP" );			break;
	case cdSHOOTING_MODE_BULB:			udn.set(i, "Bulb" );			break;
	case cdSHOOTING_MODE_MANUAL_2:		udn.set(i, "Manual" );			break;
	case cdSHOOTING_MODE_FAR_SCENE:		udn.set(i, "Far Scene" );		break;
	case cdSHOOTING_MODE_FAST_SHUTTER:	udn.set(i, "Fast Shutter" );	break;
	case cdSHOOTING_MODE_SLOW_SHUTTER:	udn.set(i, "Slow Shutter" );	break;
	case cdSHOOTING_MODE_NIGHT_SCENE:	udn.set(i, "Night Scene" );		break;
	case cdSHOOTING_MODE_GRAY_SCALE:	udn.set(i, "Gray Scale" );		break;
	case cdSHOOTING_MODE_SEPIA:			udn.set(i, "Sepia" );			break;
	case cdSHOOTING_MODE_PORTRAIT:		udn.set(i, "Portrait" );		break;
	case cdSHOOTING_MODE_SPOT:			udn.set(i, "Spot" );			break;
	case cdSHOOTING_MODE_MACRO:			udn.set(i, "Macro" );			break;
	case cdSHOOTING_MODE_BW:			udn.set(i, "BW" );				break;
	case cdSHOOTING_MODE_PANFOCUS:		udn.set(i, "Panfocus" );		break;
	case cdSHOOTING_MODE_VIVID:			udn.set(i, "Vivid" );			break;
	case cdSHOOTING_MODE_NEUTRAL:		udn.set(i, "Neutral" );			break;
	default:							udn.set(i, "(Error)" );	
	};
 }
err=CDEnumShootingModeRelease(hEnum);
i=udn.choose("Choose shooting mode",hmwnd);
mode=ud.gint(i);
err=CDSetShootingMode(hSource,mode);
cderror_(err,"CDSetShootingMode()",0);
return 1;
}

//............................................................................................ 
BOOL CanonCam::SelAFdist()
{
cdHEnum	hEnum;
cdUInt32 count,i;
UDATA ud,udn;
cdAFDistance dist;
err=CDEnumAFDistanceSettingReset(hSource,&hEnum);
cderror_(err,"CDEnumAFDistanceSettingReset()\nCheck if you are in a mode that supports AFd",0);
err=CDGetAFDistanceSettingCount(hEnum,&count);
cderror_(err,"CDEnumAFDistanceSettingCount()",0);
exitif_(count<=0,0,"No AFd",0);
ud.init(UDT_NAT,count);
udn.init(UDT_CPSTR,count);
for(i=0;i<count;i++)
 {
 CDEnumAFDistanceSettingNext(hEnum,&dist);
 ud.set(i,(__int64)dist);
 switch(dist)
    {
	case cdAF_DISTANCE_MANUAL:			udn.set(i, "Manual" );		break;
	case cdAF_DISTANCE_AUTO:			udn.set(i, "Auto" );		break;
	case cdAF_DISTANCE_UNKNOWN:			udn.set(i, "Unknown" );		break;
	case cdAF_DISTANCE_CLOSE_UP:		udn.set(i, "Close up" );	break;
	case cdAF_DISTANCE_VERY_CLOSE:		udn.set(i, "Very close" );	break;
	case cdAF_DISTANCE_CLOSE:			udn.set(i, "Close" );		break;
	case cdAF_DISTANCE_MIDDLE:			udn.set(i, "Middle" );		break;
	case cdAF_DISTANCE_FAR:				udn.set(i, "Far" );			break;
	case cdAF_DISTANCE_PAN_FOCUS:		udn.set(i, "Pan focus" );	break;
	case cdAF_DISTANCE_SUPER_MACRO:		udn.set(i, "Super macro" );	break;
	case cdAF_DISTANCE_INFINITY:		udn.set(i, "Infinity" );	break;
//	case cdAF_DISTANCE_SUPER_MACRO_0CM:	udn.set(i, "Super macro 0");break;
	case cdAF_DISTANCE_NA:				udn.set(i, "N/A" );			break;
	default:							udn.set(i, "(Error)" );
	};
 }
err=CDEnumAFDistanceSettingRelease(hEnum);
i=udn.choose("Choose AF distance",hmwnd);
dist=ud.gint(i);
err=CDSetAFDistanceSetting(hSource,dist);
cderror_(err,"CDSetAFDistanceSetting()",0);
err=CDActViewfinderAutoFunctions(hSource,0x00000007);
return 1;
}

//............................................................................................ 
inline void CanonCam::AutoEF()  //auto exposure and focus
{
if(!hSource) return;
err=CDActViewfinderAutoFunctions(hSource,0x00000007);
cderror(err,"CDActViewfinderAutoFunctions()",0);
}

//............................................................................................ 
inline void CanonCam::SetMode(cdShootingMode mode=cdSHOOTING_MODE_AUTO)
{
if(!hSource) return;
err=CDSetShootingMode(hSource,mode);
cderror(err,"CDSetShootingMode()",0);
}

//............................................................................................ 
inline void CanonCam::SetAFd(cdAFDistance dist=cdAF_DISTANCE_AUTO)
{
if(!hSource) return;
err=CDSetAFDistanceSetting(hSource,dist);
cderror(err,"CDSetAFDistanceSetting()",0);
}

//............................................................................................ 
inline void CanonCam::SetFlash(cdFlashMode flash=cdFLASH_MODE_OFF,cdCompensation fcomp=0xff)
{
if(!hSource) return;
err=CDSetFlashSetting(hSource,flash,fcomp);
cderror(err,"CDSetFlashSetting()",0);
}

	/*
	cdCOMP_QUALITY_UNKNOWN		0xffff
	cdCOMP_QUALITY_ECONOMY		0x0001
	cdCOMP_QUALITY_NORMAL		0x0002
	cdCOMP_QUALITY_FINE			0x0003
	cdCOMP_QUALITY_LOSSLESS		0x0004
	cdCOMP_QUALITY_RAW			0x0004
	cdCOMP_QUALITY_SUPERFINE	0x0005
	
	cdIMAGE_SIZE_UNKNOWN		0xFFFF
	cdIMAGE_SIZE_LARGE			0x0000
	cdIMAGE_SIZE_MEDIUM			0x0001
	cdIMAGE_SIZE_SMALL			0x0002
	cdIMAGE_SIZE_MEDIUM1		0x0005
	cdIMAGE_SIZE_MEDIUM2		0x0006
	cdIMAGE_SIZE_MEDIUM3		0x0007
	*/
//............................................................................................ 
inline void CanonCam::SetSize(cdCompQuality quality=cdCOMP_QUALITY_NORMAL,cdImageSize imgsz=cdIMAGE_SIZE_LARGE)
{
if(!hSource) return;
err=CDSetImageFormatAttribute(hSource,quality,imgsz);
cderror(err,"Couldn't set image size or quality",0);
}

//............................................................................................ 
inline void CanonCam::GetSize(cdCompQuality&quality,cdImageSize&imgsz)
{
if(!hSource) return;
err=CDGetImageFormatAttribute(hSource,&quality,&imgsz);
cderror(err,"Couldn't get image size or quality",0);
}

//............................................................................................ 
inline void CanonCam::GetZoom(cdURational*pdigzoom=NULL)
{
if(!hSource) return;
if(pdigzoom)
 err=CDGetDZoomMagnification(hSource,pdigzoom);
err=CDGetZoomPos(hSource,&zoom);
}

//............................................................................................ 
inline void CanonCam::SetZoom(cdUInt32 nzoom=0)
{
if(!hSource) return;
err=CDSetZoomPos(hSource,nzoom>maxoptzoom?maxoptzoom:nzoom);
err=CDGetZoomPos(hSource,&zoom);
}


//............................................................................................ 
BOOL CanonCam::SelAv()
{
cdHEnum	hEnum;
cdUInt32 count,i;
UDATA ud;
cdRemoteSetAv av;
err=CDEnumAvValueReset(hSource,&hEnum);
cderror_(err,"CDEnumAvValueReset()\nCheck that you are in Av or Manual shooting mode",0);
err=CDGetAvValueCount(hEnum,&count);
cderror_(err,"CDGetAvValueCount()",0);
exitif_(count<=0,0,"No Av values",0);
ud.init(UDT_NAT,count);
for(i=0;i<count;i++)
 {
 CDEnumAvValueNext(hEnum,&av);
 ud.set(i,(__int64)av);
 }
err=CDEnumAvValueRelease(hEnum);
i=ud.choose("Choose Av value",hmwnd);
av=ud.gint(i);
err=CDSetAvValue(hSource,av);
cderror_(err,"CDSetAvValue()",0);
return 1;
}

//............................................................................................ 
BOOL CanonCam::SelVolume()
{
cdHEnum	hEnum;
cdUInt32 count,i;
UDATA ud;
err=CDEnumVolumeReset(hSource,&hEnum);
cderror_(err,"CDEnumVolumeReset()",0);
err=CDGetVolumeCount(hEnum,&count);
cderror_(err,"CDGetVolumeCount()",0);
exitif_(count<=0,0,"No volumes",0);
ud.init(UDT_STR,count,sizeof(cdVolName));
for(i=0;i<count;i++)
 {
 CDEnumVolumeNext(hEnum,&hVol);
 CDGetVolumeName(hVol,(cdVolName*)ud.getit(i));
 }
err=CDEnumVolumeRelease(hEnum);
i=ud.choose("Choose volume",hmwnd);
return 1;
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* The function which receives the picture from a camera */
cdUInt32 cdSTDCALL CanonCam::ViewFinderCallBackProc
(cdVoid	*pBuf,cdUInt32 Size,cdUInt32 Format,cdContext Context)
{
CanonCam*This;
HDC hldc;
RECT lr;
LPBYTE				bpPixel;
PBITMAPFILEHEADER	pBmpFileheader;
PBITMAPINFOHEADER	pBmpInfoheader;
This=(CanonCam*)Context;
//This->DBG();
if(Format==FILEFORMAT_BMP)
 {
 pBmpFileheader=(PBITMAPFILEHEADER)pBuf;
 pBmpInfoheader=(PBITMAPINFOHEADER)((LPBYTE)pBuf + sizeof(BITMAPFILEHEADER));
 bpPixel=(LPBYTE)pBuf+pBmpFileheader->bfOffBits;
 GetClientRect(This->hvfwnd,&lr);
 hldc=GetDC(This->hvfwnd);
 SetStretchBltMode(hldc,COLORONCOLOR);
 StretchDIBits(hldc,0,0,lr.right,lr.bottom,0,0,pBmpInfoheader->biWidth,pBmpInfoheader->biHeight,
               bpPixel,(PBMI)pBmpInfoheader,DIB_RGB_COLORS,SRCCOPY);
 ReleaseDC(This->hvfwnd,hldc);
 }
return This->err;
}

//----------------------------------------------------------------------------
void InitCanonSDK()
{
ZeroMemory(&CDSDKversion,sizeof(cdVersionInfo));
CDSDKversion.Size=sizeof(cdVersionInfo);
CDSDKversion.MajorVersion=7;
CDSDKversion.MinorVersion=3;
erret=CDStartSDK(&CDSDKversion,0);
cderror(erret,"Failed to init Canon Digital SDK 7.3\nCheck for redistributable %u",erret);
}


//----------------------------------------------------------------------------
void FreeCanonSDK()
{
CDFinishSDK();
}

//............................................................................................ 
inline void CanonCam::DBG()
{
cdURational digzoom;
cdCompQuality quality;
cdImageSize imgsz;
cdShootingMode mode;
cdAFDistance dist;
cdFlashMode flash;
cdCompensation fcomp;
GetZoom(&digzoom);
GetSize(quality,imgsz);
err=CDGetShootingMode(hSource,&mode);
err=CDGetAFDistanceSetting(hSource,&dist);
err=CDGetFlashSetting(hSource,&flash,&fcomp);
printbox("Zoom:%u/%u/%u(%u/%u)\nQuality=%x Size=%x\nMode=%x AFd=%x\nFlash=%x Comp=%x",
         zoom,maxoptzoom,maxzoom,digzoom.Numerator,digzoom.Denominator,
         quality,imgsz,mode,dist,flash,fcomp);
}
