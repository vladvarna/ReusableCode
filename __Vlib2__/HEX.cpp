#pragma once

#include <ustruct.cpp>

//INHX32 is the only one suppoarted yet
#define HEX_FORMAT_INHX32           1
//#define HEX_FORMAT_INHX8M           2 //duplicated address
//#define HEX_FORMAT_INHX8S           3 //split HXL/HXH
//#define HEX_FORMAT_INHX16           4 //

#define HEX_VERIFY_CHECKSUM         0x10000 //when reading, do check sum

#define HEX_MARK_LINE              ':' //marks start of record

#define HEX_REC_DATA                0 //data
#define HEX_REC_EOF                 1 //eof
#define HEX_REC_ESAR                2 //Extended Segment Address Record = SEG (b19-b4 for address)
#define HEX_REC_SSAR                3 //Start Segment Address Record = CS:IP
#define HEX_REC_ELAR                4 //Extended Linear Address Record = b31-b16 for address
#define HEX_REC_SLAR                5 //Start Linear Address Record = EIP

struct HEXfile_record //type is by default 0=data
 {
 DWORD szB;   //size in Bytes
 DWORD adr;   //address(offset)
 DWORD chk; //checksum
 BYTE data[];
 };

//Intel hex file ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`
class HEXfile
{
public:
 FILE* fhex; //handle to file
 FLAGS stat; 
 USTRUCT <HEXfile_record> recs; //array of records found in file

 HEXfile() { ZEROCLASS(HEXfile); }
 ~HEXfile() { Free(); }
 void Add(NAT,NAT,BYTE*);
 BYTE Checksum1(NAT);
 NAT MaxAddress();
 FAIL Load(LPSTR,FLAGS);
 FAIL Save(LPSTR,FLAGS);
 FAIL ParseIHX32();
 FAIL DumpIHX32();
 void Free()
  {
  recs.Free();
  FCLOSE(fhex);
  }
};

//append a record.............................................................................................
inline void HEXfile::Add(NAT adr,NAT szB,BYTE*pdata=NULL)
{
NAT ind=recs.nrit;
recs.dimit(ind,szB);
recs.item[ind]->adr=adr;
recs.item[ind]->szB=szB;
if(!pdata) return;
CopyMemory(recs.item[ind]->data,pdata,szB);
}

//compute checksum.............................................................................................
inline BYTE HEXfile::Checksum1(NAT ind)
{
if(ind>=recs.nrit) return 0;
recs.item[ind]->chk=(recs.item[ind]->szB&0xff)+(recs.item[ind]->adr&0xff)+((recs.item[ind]->adr>>8)&0xff);
for(int b=0;b<recs.item[ind]->szB;b++)
 recs.item[ind]->chk+=recs.item[ind]->data[b];
return recs.item[ind]->chk=(-recs.item[ind]->chk)&0xff;
}

//find maximum address.............................................................................................
inline NAT HEXfile::MaxAddress()
{
NAT maxadr=0;
for(int r=0;r<recs.nrit;r++)
 if(recs.item[r]->adr>maxadr) 
  maxadr=recs.item[r]->adr;
return maxadr;  
}

//load and parse file.............................................................................................
FAIL HEXfile::Load(LPSTR path,FLAGS flags=HEX_FORMAT_INHX32)
{
FCLOSE(fhex);
fhex=FOPEN(path,"r+b");
ifn(fhex) return 1; //can't open file
ParseIHX32();
return 0; //ok
}

//save to file.............................................................................................
FAIL HEXfile::Save(LPSTR path,FLAGS flags=HEX_FORMAT_INHX32)
{
FCLOSE(fhex);
fhex=FOPEN(path,"wb");
ifn(fhex) return 1; //can't open file
DumpIHX32();
return 0; //ok
}

//parse file.............................................................................................
FAIL HEXfile::ParseIHX32()
{ //carefull !!! when reading with fscanf you must provide at least a WORD at the address
NAT reclen,address,rectype,csum,tdat,ladr,nrr;
char c=0;
reclen=address=rectype=csum=tdat=ladr=nrr=0;
recs.Free();
fseek(fhex,0,SEEK_SET);
while(c!=EOF)
 {
 do{ 
  c=fgetc(fhex);
  if(c==EOF)
   return 0; //done
  }while(c!=HEX_MARK_LINE); //search next record
 if(fscanf(fhex,"%2hx%4hx%2hx",&reclen,&address,&rectype)==EOF)
  return 1; //possible incomplete or truncated file
 if(rectype==HEX_REC_DATA)
  {
  Add(address+ladr,reclen);
  for(int b=0;b<reclen;b++)
   {
   if(fscanf(fhex,"%2hx",&tdat)==EOF)
    return 2; //possible incomplete or truncated file
   recs.item[nrr]->data[b]=tdat;
   }
  //printbox("%u: &%x [%u]",nrr,address,reclen);
  nrr++;
  ladr=0; //reset address
  }
 else if(rectype==HEX_REC_EOF)
  return 0; //done
 else if(rectype==HEX_REC_ESAR)
  {
  if(fscanf(fhex,"%2hx",&ladr)==EOF)
   return 4; //possible incomplete or truncated file
  ladr<<=4;
  }
 else if(rectype==HEX_REC_ELAR)
  {
  if(fscanf(fhex,"%2hx",&ladr)==EOF)
   return 5; //possible incomplete or truncated file
  ladr<<=16;
  }
 else
  error("Unsupported record type in HEXfile");
 if(fscanf(fhex,"%2hx",&csum)==EOF)
  return 3; //possible incomplete or truncated file
 if(stat&HEX_VERIFY_CHECKSUM)
  {
  if(csum!=(tdat=Checksum1(recs.nrit-1)))
   printbox("Check sum (read)%x!=%x(calc) ",csum,tdat);
  } 
 }
return 0; 
}

//save.............................................................................................
FAIL HEXfile::DumpIHX32()
{
NAT csum,tdat;
fseek(fhex,0,SEEK_SET);
//fprintf(fhex,":020000040000FA\r\n"); //start address 0
for(int r=0;r<recs.nrit;r++)
 {
 if(recs.item[r]->adr&0xffff0000)
  printbox("Large address found"); //TODO: generate HEX_REC_ELAR 
 if(recs.item[r]->szB&0xffffff00)
  printbox("Record is larger than 256B; must split"); //TODO: split it
 fprintf(fhex,":%02X%04X00",recs.item[r]->szB&0xff,recs.item[r]->adr&0xffff); //start address
 csum=(recs.item[r]->szB&0xff)+(recs.item[r]->adr&0xff)+((recs.item[r]->adr>>8)&0xff);
 for(int b=0;b<(recs.item[r]->szB);b++)
  {
  csum+=tdat=recs.item[r]->data[b];
  fprintf(fhex,"%02X",tdat);
  }
 fprintf(fhex,"%02X\r\n",(-csum)&0xff);
 }
fprintf(fhex,":00000001FF"); //EOF
return 0; 
}


