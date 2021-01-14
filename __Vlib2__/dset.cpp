#ifndef V_DATASET
#define V_DATASET

///////////////////////////////////////////////////////////////////////////////////////////
class DataSet
{
public:
 HANDLE hDSFile;
 DWORD Tip; //LOW: Id-0x5344="DS", HIW: 0-uncompresed
 NAT NrRec; //Number of data records in set
 NAT Rec,RecOf,RecSz; //curent record info: nr, offset(from begin), size(in B)
 
 int Bind(LPSTR,WORD); //0=Ok
 int Open(NAT,NAT); //must use before any valid get/put (valid records are 1..NrRec)
 int Ins(NAT,NAT); //use Open after
 int Del(NAT); //use Open after
 int Put(BYTE*,NAT,NAT);
 int Get(BYTE*,NAT,NAT);
};
///////////////////////////////////////////////////////////////////////////////////////////

// Data Set <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int DataSet::Bind(LPSTR dfname=NULL,WORD comp=0)
{
if(hDSFile!=INVALID_HANDLE_VALUE) CloseHandle(hDSFile);
hDSFile=INVALID_HANDLE_VALUE;
Rec=0;
NrRec=0;
if(!dfname) return 1; //unbind successful
hDSFile=CreateFile(dfname,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
if(hDSFile==INVALID_HANDLE_VALUE) //create
 {
 hDSFile=CreateFile(dfname,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_NEW,0,NULL);
 if(hDSFile==INVALID_HANDLE_VALUE) return 2; //unable to bind to file (invalid name ?)
 Tip=(comp<<16)|0x5344;
 RecOf=12;
 WriteFile(hDSFile,&Tip,4,(DWORD*)&RecSz,NULL);
 WriteFile(hDSFile,&NrRec,4,(DWORD*)&RecSz,NULL);
 WriteFile(hDSFile,&RecOf,4,(DWORD*)&RecSz,NULL);
 }
else //open
 {
 ReadFile(hDSFile,&Tip,4,(DWORD*)&RecSz,NULL);
 ReadFile(hDSFile,&NrRec,4,(DWORD*)&RecSz,NULL);
 ReadFile(hDSFile,&RecOf,4,(DWORD*)&RecSz,NULL);
 if(Tip&0xffff!=0x5344) return 2; //not a data set
 }
return 0; //Ok
}

int DataSet::Open(NAT recpos=0,NAT newsize=0)
{
if(recpos>NrRec) //create
 {
 if(newsize==0) return 2; //can't create
 Open(NrRec);
 recpos=SetFilePointer(hDSFile,0,NULL,FILE_END);
 SetFilePointer(hDSFile,RecOf,NULL,FILE_BEGIN);
 WriteFile(hDSFile,&recpos,4,(DWORD*)&RecSz,NULL);
 RecOf=SetFilePointer(hDSFile,recpos,NULL,FILE_BEGIN);
 RecSz=0;
 WriteFile(hDSFile,&RecSz,4,(DWORD*)&recpos,NULL);
 RecSz=newsize;
 WriteFile(hDSFile,&RecSz,4,(DWORD*)&recpos,NULL);
 SetFilePointer(hDSFile,RecSz+7,NULL,FILE_CURRENT);
 WriteFile(hDSFile,&RecSz,1,(DWORD*)&recpos,NULL);
 NrRec++;
 SetFilePointer(hDSFile,4,NULL,FILE_BEGIN);
 WriteFile(hDSFile,&NrRec,4,(DWORD*)&recpos,NULL);
 Rec=NrRec;
 }
else //open
 {
 Rec=0;
 RecOf=8;
 while(Rec<recpos)
  {
  SetFilePointer(hDSFile,RecOf,NULL,FILE_BEGIN);
  ReadFile(hDSFile,&RecOf,4,(DWORD*)&RecSz,NULL);
  Rec++;
  }
 if(recpos==0) return 1; //get/put will fail
 SetFilePointer(hDSFile,RecOf+4,NULL,FILE_BEGIN);
 ReadFile(hDSFile,&RecSz,4,(DWORD*)&recpos,NULL);
 if(newsize==0||newsize==RecSz)
  return 0;
 else if(newsize<RecSz) //downsize
  {
  RecSz=newsize;
  SetFilePointer(hDSFile,RecOf+4,NULL,FILE_BEGIN);
  WriteFile(hDSFile,&RecSz,4,(DWORD*)&recpos,NULL);
  }
 else if(newsize>RecSz) //resize
  {
  recpos=Rec;
  Del(recpos);
  if(recpos<=NrRec)
   {
   Ins(recpos,newsize);
   Open(recpos);
   }
  else Open(recpos,newsize);
  }
 }
return 0; //can get/put
}

int DataSet::Ins(NAT recpos,NAT newsize)
{
if(recpos==0||recpos>NrRec||newsize==0) return 1;
Open(recpos-1);
SetFilePointer(hDSFile,RecOf,NULL,FILE_BEGIN);
ReadFile(hDSFile,&Rec,4,(DWORD*)&RecSz,NULL);
recpos=SetFilePointer(hDSFile,0,NULL,FILE_END);
WriteFile(hDSFile,&Rec,4,(DWORD*)&RecSz,NULL);
WriteFile(hDSFile,&newsize,4,(DWORD*)&RecSz,NULL);
SetFilePointer(hDSFile,newsize+7,NULL,FILE_CURRENT);
WriteFile(hDSFile,&newsize,1,(DWORD*)&RecSz,NULL);
SetFilePointer(hDSFile,RecOf,NULL,FILE_BEGIN);
WriteFile(hDSFile,&recpos,4,(DWORD*)&RecSz,NULL);
NrRec++;
SetFilePointer(hDSFile,4,NULL,FILE_BEGIN);
WriteFile(hDSFile,&NrRec,4,(DWORD*)&Rec,NULL);
Open();
return 0;
}

int DataSet::Del(NAT recpos)
{
if(recpos==0||recpos>NrRec) return 1; //doesn't exist
Open(recpos);
SetFilePointer(hDSFile,RecOf,NULL,FILE_BEGIN);
ReadFile(hDSFile,&recpos,4,(DWORD*)&RecSz,NULL);
Open(Rec-1);
SetFilePointer(hDSFile,RecOf,NULL,FILE_BEGIN);
WriteFile(hDSFile,&recpos,4,(DWORD*)&RecSz,NULL);
NrRec--;
SetFilePointer(hDSFile,4,NULL,FILE_BEGIN);
WriteFile(hDSFile,&NrRec,4,(DWORD*)&recpos,NULL);
Open();
return 0;
}

int DataSet::Get(BYTE*recbuf,NAT stb=0,NAT bcnt=0)
{
if(Rec==0||Rec>NrRec) return 0; //invalid record
if(stb>=RecSz) return 0; //out of range
if(bcnt==0||stb+bcnt>RecSz) bcnt=RecSz-stb;
SetFilePointer(hDSFile,RecOf+8+stb,NULL,FILE_BEGIN);
ReadFile(hDSFile,recbuf,bcnt,(DWORD*)&stb,NULL);
return stb; //actual bytes read
}

int DataSet::Put(BYTE*recbuf,NAT stb=0,NAT bcnt=0)
{
if(Rec==0||Rec>NrRec) return 0; //invalid record
if(stb>=RecSz) return 0; //out of range
if(bcnt==0||stb+bcnt>RecSz) bcnt=RecSz-stb;
SetFilePointer(hDSFile,RecOf+8+stb,NULL,FILE_BEGIN);
WriteFile(hDSFile,recbuf,bcnt,(DWORD*)&stb,NULL);
return stb; //actual bytes read
}
// DataSet >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//removes unused file space from set ------------------------------------------------------
int DataSetSort(LPSTR fsrcds,LPSTR fdestds=NULL)
{
DataSet dds,sds;
if(sds.Bind(fsrcds)!=0) return 1; //invalid source
if(fdestds==NULL||strcmp(fsrcds,fdestds)==0)
 {
 if(dds.Bind("_dataset.del")!=0) 
  return 3; //can't create temporary
 }
else 
 {
 if(dds.Bind(fdestds)!=0) 
  return 2; //invalid destination
 }
BYTE *recbuf;
NAT rec;
for(rec=1;rec<=sds.NrRec;rec++)
 {
 sds.Open(rec);
 recbuf=(BYTE*)malloc(sds.RecSz);
 sds.Get(recbuf);
 dds.Open(rec,sds.RecSz);
 dds.Put(recbuf);
 free(recbuf);
 }
sds.Bind(); 
dds.Bind();
if(fdestds==NULL||strcmp(fsrcds,fdestds)==0)
 {
 DeleteFile(fsrcds);
 if(MoveFile("_dataset.del",fsrcds)==0)
  return 4; //can't rename file
 }
return 0; //sort ok
}

#endif