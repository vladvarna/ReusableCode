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

//____________________________________________________________________________________________________________________
struct APP_OPTIONS
{
 DWORD misc;
 char profile[PATHSZ];
 //.....................................................................................................
 APP_OPTIONS()
  {
  misc=0xf0;
  sc(profile,"last.profile");
  }
 
 //.....................................................................................................
 void User1()
  {
  misc=InputFlags(OPT_COUNT,misc,hmwnd,"Options",
                 "Auto start with Windows",OPT_STARTUP);
  AutoStart(misc&OPT_STARTUP);
  }
 //.....................................................................................................
 void User2(HWND hwnd) //wintip
  {
  //InputPaths(3,0,hwnd,"Global paths","TIP",TIP,"OBJ/PAT",OBJ,"Summaries",SUM);
  }
 //.....................................................................................................
 BOOL Restore()
  {
  REGISTRY rgy;
  if(rgy.AppKey()!=1) return 0;
  rgy.GetVal("APP_OPTIONS",this,sizeof(APP_OPTIONS));
  return 1;
  };
 //......................................................................................................
 ~APP_OPTIONS()
  {
  REGISTRY rgy;
  rgy.AppKey();
  rgy.SetVal("APP_OPTIONS",this,sizeof(APP_OPTIONS));
  //AutoStart(misc&OPT_STARTUP);
  }
}opt;
