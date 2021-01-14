//Copy HBITMAP to clipboard
// Get a handle for a 32bpp DIB from gdiplus
HBITMAP hBitmap = NULL;
if(GDIPlusBmp.GetHBITMAP(Gdiplus::Color(255,0,0,0), &hBitmap) == Gdiplus::Ok)
	{
	//DIB to DDB
	DIBSECTION ds;
	GetObject( hBitmap, sizeof(ds), &ds );
	ds.dsBmih.biCompression = BI_RGB; // change compression fromBI_BITFIELDS to BI_RGB
	// Convert the DIB to a device dependent bitmap(i.e., DDB)
	HDC hDC = GetDC(NULL);
	HBITMAP hDDB = CreateDIBitmap( hDC, &ds.dsBmih, CBM_INIT,	ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS );
	ReleaseDC(NULL, hDC);
	// Open the clipboard
	OpenClipboard(hWnd);
	EmptyClipboard();
	// Place the handle to the DDB on the clipboard
	SetClipboardData(CF_BITMAP, hDDB);
	// Do not delete the hDDB handle, the clipboard owns it
	CloseClipboard();
	}
DeleteObject(hBitmap);


#ifdef API_EXPORTS
	#define API extern "C" __declspec(dllexport)
#else
	#define API extern "C" __declspec(dllimport)
#endif

//____________________________________________________________________________________________________________________
NAT tid=START_UI_THREAD(&Thread_UI_Marquee,_T("...."),NULL,0/*timeout in seconds*/,0,NULL);
CLOSE_UI_THREAD(tid);

//____________________________________________________________________________________________________________________
InputNumber("Recording volume",hmwnd,prof.recvol,0,0xffff,0,10)

//____________________________________________________________________________________________________________________
StoI(textline,radix,NULL,maxnc);
StoR(textline,radix,maxnc,NULL);

//____________________________________________________________________________________________________________________
sprintf(REGs,"%u",prof.sampfreq);
prof.sampfreq=InputBox(REGs,"Frames per second [Hz]",6,hmwnd,1);
    
//____________________________________________________________________________________________________________________
sc(REGs,gopt.SUM);
ifn(BrowsePath(REGs,"Import all files in folder",BIF_NONEWFOLDERBUTTON,NULL,hmwnd))
 return 0;

//____________________________________________________________________________________________________________________
*REGs=0;
ifn(GetFileName(REGs,0,hmwnd,"Zip\0*.zip\0","Import zipped summaries","zip",NULL,NULL,NULL,'O',opt.FTP_IN))
 return 0;

//____________________________________________________________________________________________________________________
InputStrings(3,hmwnd,"File name decorations",
 "Category(subfolder)",prof.categ,sizeof(prof.categ)-1,
 "Prefix",prof.pref,sizeof(prof.pref)-1,
 "Sufix",prof.suf,sizeof(prof.suf)-1);

