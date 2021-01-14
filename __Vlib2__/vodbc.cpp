#pragma once
#define V_MYSQL

#include <sql.h>
#include <sqlext.h>
#include <ODBCinst.h>
#include <udb.cpp>

#define MAX_ODBC_QUERY_NC 8192 //8KB

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
class VODBC:public USQLDB
{
public:
 //FLAGS stat;
 SQLHENV henv;
 SQLHDBC hdbc;
 SQLHSTMT hstmt; //default statement
 SQLRETURN ret;  //last return value
 char Qbuf[MAX_ODBC_QUERY_NC]; //buffer for last query
 NAT Qnc; //last query size in chars
 
 VODBC() { ZEROCLASSV(VODBC);  type=USQL_ODBC; }
 int ifer(int,char*,...);
 int checkret(char*,...);
 void UTYPE(enum_field_types,NAT,NAT,BITS32*,NAT*);
 //USQLDB
 virtual BOOL Connect(LOGON_INFO*,char*);
 virtual BOOL Disconnect();
 virtual int Table(LPSTR);
 virtual BOOL E_db(LPSTR);
 virtual BOOL E_table(LPSTR);
 virtual BOOL E_field(LPSTR);
 virtual BOOL E_value(LPSTR,LPSTR,int);
 virtual BOOL DropTable(LPSTR);  //drop table
 virtual BOOL Query(char*,...);
 virtual void StartQuery();
 virtual BOOL Astr(char*,...);
 virtual BOOL Abin(void*,NAT);
 virtual BOOL DoQuery();
 virtual BOOL QResult(UTABLE*);
 virtual int Database(LPSTR);
 virtual BOOL DropDB(LPSTR);  //drop database
 //~USQLDB
 void ShowResult(SQLHSTMT);
 void Info();
 void Status();
 ~VODBC() { Disconnect(); }
};
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
#define SQL_SUCCESS                0
#define SQL_SUCCESS_WITH_INFO      1
#define SQL_NO_DATA              100
#define SQL_ERROR                 (-1)
#define SQL_INVALID_HANDLE        (-2)
#define SQL_STILL_EXECUTING        2
#define SQL_NEED_DATA             99
*/
// [vendor][ODBC-component][data-source]message-text .................................................................................................................................
int VODBC::checkret(char *formstr=NULL,...)
{
if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO) return 0;
char lsbuf[MAX_QUERY_NC];
SQLCHAR state[6];
SQLINTEGER native;
SQLSMALLINT w;
int retv,i;
NAT l=0;
va_list vparam;
va_start(vparam,formstr);
if(formstr)
 l=vsprintf(lsbuf,formstr,vparam);
va_end(vparam);
l+=sprintf(lsbuf+l," returned %i\n",ret);
retv=1;
while(SQLGetDiagRec(SQL_HANDLE_STMT,hstmt,retv,state,&native,(SQLCHAR*)(lsbuf+l),MAX_QUERY_NC-l,&w)==SQL_SUCCESS)
 {
 retv++;
 l+=w;
 lsbuf[l++]='\n';
 }
lsbuf[l]=0;
retv=MessageBox(HWND_DESKTOP,lsbuf,"ODBC ERROR",MB_ICONWARNING|MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_TOPMOST);
if(retv==IDABORT) exit(1);
if(retv==IDRETRY) DebugBreak();
return (retv==IDRETRY);	//should retry
}

//.................................................................................................................................
void VODBC::UTYPE(enum_field_types mytype,NAT prec,NAT size,BITS32*utype,NAT*uitsz)
{
}

//.................................................................................................................................
BOOL VODBC::Connect(LOGON_INFO*li=NULL,char*connopts=NULL)
{
char dir[PATHSZ],file[PATHSZ];
SQLSMALLINT rl;
VSTR connstr;
connections++;
if(connections>1) return 1;
ret=SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv);
ret=SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(void*)SQL_OV_ODBC3,0); 
ret=SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc); 
for(;;)
 {
 if(li->flags&ODBC_LOGON_DRIVER)
  connstr.apf("DRIVER={%s};",li->DRIVER);
 else if(li->flags&ODBC_LOGON_FILEDSN)
  connstr.apf("FILEDSN=%s;",li->DSN);
 else if(li->flags&ODBC_LOGON_DSN)
  connstr.apf("DSN=%s;",li->DSN);  //have DSN="" to prompt
 else if(li->flags&ODBC_LOGON_DATABASE)
  connstr.apf("DATABASE=%s;",li->database);
 
 if(li->flags&ODBC_LOGON_HOST)
  connstr.apf("SERVER=%s;",li->host);
 else if(li->flags&ODBC_LOGON_ADDRESS)
  connstr.apf("ADDRESS=%s;",li->host);
 else if(li->flags&ODBC_LOGON_PATH)
  {
  SplitPath(li->path,dir,file);
  connstr.apf("DEFAULTDIR=%s;DBQ=%s;",dir,file);
  }
 
 if(li->flags&ODBC_LOGON_PORT)
  connstr.apf("PORT=%u;",li->port);
 else if(li->flags&ODBC_LOGON_DRIVERID)
  connstr.apf("DRIVERID=%u;",li->port);
 
 if(li->flags&ODBC_LOGON_NET)
  connstr.apf("NETWORK=(default);");
 if(li->flags&ODBC_LOGON_UID)
  connstr.apf("UID=%s;",li->username);
 if(li->flags&ODBC_LOGON_PWD)
  connstr.apf("PWD=%s;",li->password);
 
 connstr+connopts;
 //connstr.show();
 ret=SQLDriverConnect(hdbc,hmwnd,(SQLCHAR*)connstr.pc,connstr.nc,(SQLCHAR*)Qbuf,sizeof(Qbuf),&rl,SQL_DRIVER_COMPLETE_REQUIRED);
 //printbox(Qbuf);
 if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO)
  break; //connected
 if(!li->UI())
  {
  connections=0;
  return 0; //failed
  }
 }
ret=SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt);
connections=1;
return 1; //connected
}

//.................................................................................................................................
BOOL VODBC::Disconnect()
{
connections--;
if(connections>0) return 1;
if(hstmt)
 {
 SQLFreeHandle(SQL_HANDLE_STMT,hstmt);
 hstmt=NULL;
 }
if(hdbc)
 {
 ret=SQLDisconnect(hdbc);
 //checkret("Disconnect");
 SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
 hdbc=NULL;
 }
if(henv)
 {
 SQLFreeHandle(SQL_HANDLE_ENV,henv);
 henv=NULL;
 }
return ret;
}


//.................................................................................................................................
inline int VODBC::Table(LPSTR ltname=NULL)
{
sc(tablename,ltname,sizeof(tablename));
return 1;
}

//.................................................................................................................................
inline BOOL VODBC::DropTable(LPSTR tname=NULL)
{
Qnc=sprintf(Qbuf,"DROP TABLE %s",tname);
ret=SQLExecDirect(hstmt,(SQLCHAR*)Qbuf,Qnc);
return ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO;
}

//.................................................................................................................................
inline BOOL VODBC::E_db(LPSTR dbname)
{
SQLTables(hstmt,(SQLCHAR*)dbname,SQL_NTS,NULL,0,NULL,0,NULL,0);
//ShowResult(NULL);
ret=SQLFetch(hstmt);
SQLCloseCursor(hstmt);
return ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO;
}

//.................................................................................................................................
inline BOOL VODBC::E_table(LPSTR tname)
{
SQLTables(hstmt,NULL,0,NULL,0,(SQLCHAR*)tname,SQL_NTS,NULL,0);
//ShowResult(NULL);
ret=SQLFetch(hstmt);
SQLCloseCursor(hstmt);
return ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO;
}

//.................................................................................................................................
inline BOOL VODBC::E_field(LPSTR fieldname)
{
SQLColumns(hstmt,(SQLCHAR*)databname,SQL_NTS,NULL,0,(SQLCHAR*)tablename,SQL_NTS,(SQLCHAR*)fieldname,SQL_NTS);
//ShowResult(NULL);
ret=SQLFetch(hstmt);
SQLCloseCursor(hstmt);
return ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO;
}

//.................................................................................................................................
inline int VODBC::E_value(LPSTR fieldname,LPSTR value,int quote=1)
{
int match=0;
if(quote) 
 Qnc=sprintf(Qbuf,"SELECT %s FROM %s WHERE %s='%s'",fieldname,tablename,fieldname,value); 
else
 Qnc=sprintf(Qbuf,"SELECT %s FROM %s WHERE %s=%s",fieldname,tablename,fieldname,value); 
ret=SQLExecDirect(hstmt,(SQLCHAR*)Qbuf,Qnc);
do{
 ret=SQLFetch(hstmt);
 match++;
}while(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO);
return match-1;
}

//.................................................................................................................................
inline BOOL VODBC::Query(char *formstr=NULL,...)
{
va_list vparam;
va_start(vparam,formstr);
Qnc=vsprintf(Qbuf,formstr,vparam);
ret=SQLExecDirect(hstmt,(SQLCHAR*)Qbuf,Qnc);
checkret("Query");
return 1;
}

//.................................................................................................................................
inline void VODBC::StartQuery()
{
Qnc=0;
*Qbuf=0;
}

//.................................................................................................................................
inline BOOL VODBC::Astr(char *formstr=NULL,...)
{
va_list vparam;
va_start(vparam,formstr);
Qnc+=vsprintf(Qbuf+Qnc,formstr,vparam);
return Qnc;
}

//.................................................................................................................................
inline BOOL VODBC::Abin(void*pmem,NAT memsz)
{
//qnc+=mysql_real_escape_string(mySQL,myQuery+qnc,(char*)pmem,memsz);
error("VODBC::Abin() not functional");
CopyMemory(Qbuf+Qnc,pmem,memsz); //TODO: add a proper way here
Qnc+=memsz;
Qbuf[Qnc]=0;
return Qnc;
}

//.................................................................................................................................
inline BOOL VODBC::DoQuery()
{
ret=SQLExecDirect(hstmt,(SQLCHAR*)Qbuf,Qnc);
checkret("Query");
return 1;
}

//.................................................................................................................................
BOOL VODBC::QResult(UTABLE*ut=NULL)
{
//MYSQL_RES*myResult;
//MYSQL_FIELD*fields;
//MYSQL_ROW row;
//NAT nrr,nrf;
//int f,r;
//DWORD*rownc;
//if(!ut) mysql_free_result(mysql_use_result(mySQL));
//myResult=mysql_store_result(mySQL);
//if(!myResult)
// {
// ifer(mysql_field_count(mySQL),"Couldn't get result for: %s",myQuery);
// return 0;
// }
//nrr=mysql_num_rows(myResult);
//nrf=mysql_num_fields(myResult);
//fields=mysql_fetch_fields(myResult);
//BITS32 utype;
//NAT uitsz;
//if(!fields) return 0;
//ut->Free();
//for(f=0;f<nrf;f++)
// {
// UTYPE(fields[f].type,fields[f].decimals,fields[f].length,&utype,&uitsz);
// ut->AddField(fields[f].name,utype,nrr,uitsz,fields[f].max_length*12);
// } 
//for(r=0;r<nrr;r++)
// {
// row=mysql_fetch_row(myResult);
// if(!row) break;
// rownc=mysql_fetch_lengths(myResult);
// for(f=0;f<nrf;f++)
//  {
//  if(row[f]) ut->field[f].scan(r,row[f],rownc[f]);
//  //ut->field[f].text(r,REGs,256);
//  //printbox("%s",REGs);
//  }
// }
//mysql_free_result(myResult);
return 1;
}

/*Issues:
 MySQL:
  > SQLTables() doesn't enumerate empty databases (you must have at least a table in it)
 Ms drivers for xls,dbf,etc.
  > They don't support CREATE DATABASE
*/
//.................................................................................................................................
inline int VODBC::Database(LPSTR ldbname=NULL)
{
SQLTables(hstmt,(SQLCHAR*)ldbname,SQL_NTS,NULL,0,NULL,0,NULL,0);
//ShowResult(NULL);
ret=SQLFetch(hstmt);
SQLCloseCursor(hstmt);
if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO)
 {
 Qnc=sprintf(Qbuf,"USE %s",ldbname); 
 ret=SQLExecDirect(hstmt,(SQLCHAR*)Qbuf,Qnc);
 if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO)
  {
  sc(databname,ldbname,sizeof(databname));
  return 1; //already exist
  }
 else
  return 0; //failed to select it thou it exists 
 }
Qnc=sprintf(Qbuf,"CREATE DATABASE %s",ldbname); 
ret=SQLExecDirect(hstmt,(SQLCHAR*)Qbuf,Qnc);
if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO)
 {
 sc(databname,ldbname,sizeof(databname));
 return 2; //created new
 }
return 0; //failed to create it
}

//.................................................................................................................................
inline BOOL VODBC::DropDB(LPSTR dbname=NULL)
{
Qnc=sprintf(Qbuf,"DROP DATABASE %s",dbname); 
ret=SQLExecDirect(hstmt,(SQLCHAR*)Qbuf,Qnc);
return ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO;
}


//(mainly for debug) .................................................................................................................................
void VODBC::ShowResult(SQLHSTMT hlstmt=NULL)
{
int f,l=0;
char *lsbuf,cname[32];
SQLSMALLINT cnnc,dtype,cdec,cnull,nrf=0;
SQLUINTEGER csize;
SQLINTEGER vsz;
NAT nrr=0,maxfsz=0;
if(!hlstmt) hlstmt=hstmt;
ret=SQLNumResultCols(hlstmt,(SQLSMALLINT*)&nrf);
if(!nrf)
 {
 error("No results");
 return;
 }
lsbuf=SALLOC(100*(nrf+1)*32+1024); //average size per field = 32B, nr rows=100
for(f=1;f<=nrf;f++)
 {
 SQLDescribeCol(hlstmt,f,(SQLCHAR*)cname,32,&cnnc,&dtype,&csize,&cdec,&cnull);
 l+=sprintf(lsbuf+l,"%s\t| ",cname);
 maxfsz+=csize;
 }
l-=3;
l+=sprintf(lsbuf+l,"\n\n");
for(;;)
 {
 ret=SQLFetch(hlstmt);
 if(ret!=SQL_SUCCESS&&ret!=SQL_SUCCESS_WITH_INFO)
  break;
 nrr++;
 for(f=1;f<=nrf;f++)
  {
  ret=SQLGetData(hlstmt,f,SQL_C_CHAR,lsbuf+l,MEMSZ(lsbuf)-l,&vsz);
  checkret("GetData");
  if(vsz>0)
   {
   l+=vsz;
   l+=sprintf(lsbuf+l,"\t| ");
   }
  else
   l+=sprintf(lsbuf+l,"<NULL>\t| ");
  }
 l-=3;
 l+=sprintf(lsbuf+l,"\n");
 }
if(l) MessageBox(hmwnd,lsbuf,Qbuf,MB_OK);
FREE(lsbuf);
}

//.................................................................................................................................
void VODBC::Info()
{
//char lsbuf[BUFFSZ];
//int l=0;
//l+=sprintf(lsbuf+l,"Client: %s %u\n",mysql_get_client_info(),mysql_get_client_version());
//l+=sprintf(lsbuf+l,"%s protocol:%u \n",mysql_get_host_info(mySQL),mysql_get_proto_info(mySQL));
//l+=sprintf(lsbuf+l,"Server: %s %u\n",mysql_get_server_info(mySQL),mysql_get_server_version(mySQL));
//l+=sprintf(lsbuf+l,"%s",mysql_character_set_name(mySQL));
////l+=sprintf(lsbuf+l,"%s\n",);
//MessageBox(HWND_DESKTOP,lsbuf,"MySQL INFO",MB_ICONINFORMATION|MB_OK);
}

//.................................................................................................................................
void VODBC::Status()
{
//char lsbuf[BUFFSZ];
//int l=0;
//l+=sprintf(lsbuf+l,"Thread id: %u \n",mysql_thread_id(mySQL));
//l+=sprintf(lsbuf+l,"%s\n",mysql_stat(mySQL));
//l+=sprintf(lsbuf+l,"Query: %s\nAUTO_INC=%I64u\n",mysql_info(mySQL),mysql_insert_id(mySQL));
//l+=sprintf(lsbuf+l,"%I64i affected rows\n",mysql_affected_rows(mySQL));
//l+=sprintf(lsbuf+l,"Warnings: %u\n",mysql_warning_count(mySQL));
//l+=sprintf(lsbuf+l,"Errors: %u %5s %s\n",mysql_errno(mySQL),mysql_sqlstate(mySQL),mysql_error(mySQL));
//MessageBox(HWND_DESKTOP,lsbuf,"MySQL status",MB_ICONINFORMATION|MB_OK);
}