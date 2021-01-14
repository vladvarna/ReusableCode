#pragma once
#define V_MYSQL

#include <udb.cpp>

#include <mySQL.h>
#pragma comment(lib,"libmysql.lib")
//#ifndef MYSQL_PATH
// #define MYSQL_PATH "G:/Prog/mySQL/"
//#endif 
//#ifdef _DEBUG
// #pragma comment(lib,MYSQL_PATH"lib/debug/libmysql.lib")
//#else
// #pragma comment(lib,MYSQL_PATH"lib/opt/libmysql.lib")
//#endif

#define MAX_QUERY_NC 10240 //8KB

//Interface class for MySQL 
class VMYSQL:public USQLDB
{
public:
 MYSQL*mySQL;
 int ret; //last mysql_*() return value
 NAT qnc; //last query size in chars
 char myQuery[MAX_QUERY_NC]; //buffer for last query
 
 VMYSQL() { ZEROCLASSV(VMYSQL);  type=USQL_MYSQL; }
 int checkret(int,char*,...);
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
 BOOL E_user(LPSTR);
 void ShowResult(MYSQL_RES*);
 void DiscardResult();
 BOOL Ping();
 void Info();
 void Status();
 ~VMYSQL() { Disconnect(); }
};

// VMYSQL <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//similar to ASSERT.................................................................................................................................
int VMYSQL::checkret(int condition=1,char *formstr=NULL,...)
{
if(!condition) return 0;
char lsbuf[MAX_QUERY_NC];
int retv;
va_list vparam;
va_start(vparam,formstr);
retv=sprintf(lsbuf,"%s\nCodes: %i %i %5s\n",mysql_error(mySQL),condition,mysql_errno(mySQL),mysql_sqlstate(mySQL));
if(formstr)
 retv=vsprintf(lsbuf+retv,formstr,vparam);
retv=MessageBox(HWND_DESKTOP,lsbuf,"MySQL ERROR",MB_ICONWARNING|MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_TOPMOST);
if(retv==IDABORT) exit(condition);
if(retv==IDRETRY) DebugBreak();
va_end(vparam);
return (retv==IDRETRY);	//should retry
}

//convert MySQL field type to internal(UDATA) field type.................................................................................................................................
void VMYSQL::UTYPE(enum_field_types mytype,NAT prec,NAT size,BITS32*utype,NAT*uitsz)
{
*uitsz=0;
switch(mytype)
 {
 case MYSQL_TYPE_VAR_STRING:
 //case MYSQL_TYPE_VARCHAR:
 case MYSQL_TYPE_BLOB:
 case MYSQL_TYPE_ENUM:
 case MYSQL_TYPE_SET:
  *utype=UDT_PSTR;
  break;
 case MYSQL_TYPE_STRING:
  *utype=UDT_STR;
  *uitsz=size;
  break;
 case MYSQL_TYPE_TINY:
  *utype=UDT_INT|UD_PREC(prec);
  *uitsz=1;
  break;
 case MYSQL_TYPE_SHORT:
  *utype=UDT_INT|UD_PREC(prec);
  *uitsz=2;
  break;
 //case MYSQL_TYPE_NEWDECIMAL:
 case MYSQL_TYPE_DECIMAL:
 case MYSQL_TYPE_LONG:
  *utype=UDT_INT|UD_PREC(prec);
  *uitsz=4;
  break;
 case MYSQL_TYPE_INT24:
  *utype=UDT_INT|UD_PREC(prec);
  *uitsz=3;
  break;
 case MYSQL_TYPE_LONGLONG:
  *utype=UDT_INT|UD_PREC(prec);
  *uitsz=8;
  break;
 case MYSQL_TYPE_FLOAT:
  *utype=UDT_REAL;
  *uitsz=4;
  break;
 case MYSQL_TYPE_DOUBLE:
  *utype=UDT_REAL;
  *uitsz=8;
  break;
 case MYSQL_TYPE_DATETIME:
 case MYSQL_TYPE_TIMESTAMP:
  *utype=UDT_DATETIME;
  break;
 case MYSQL_TYPE_YEAR:
 case MYSQL_TYPE_DATE:
 case MYSQL_TYPE_NEWDATE:
  *utype=UDT_DATE;
  break;
 case MYSQL_TYPE_TIME:
  *utype=UDT_TIME;
  break;
 default:
  *utype=0;
 }  
}

//attempt MySQL server logon .................................................................................................................................
BOOL VMYSQL::Connect(LOGON_INFO*li=NULL,char*reserved=NULL)
{
char*defdb=NULL;
connections++;
if(connections>1) return 1;
mySQL=mysql_init(NULL);
if(!mySQL)
 {
 checkret(!mySQL);
 return 0;
 }
if(!li->port)  li->port=MYSQL_PORT;
if(!*li->host)  sc(li->host,"localhost");
if(sl(li->database)) defdb=li->database;
REGdump=5; //sec
mysql_options(mySQL,MYSQL_OPT_CONNECT_TIMEOUT,(char*)&REGdump);
mysql_options(mySQL,MYSQL_OPT_WRITE_TIMEOUT,(char*)&REGdump);
mysql_options(mySQL,MYSQL_OPT_READ_TIMEOUT,(char*)&REGdump);
mysql_options(mySQL,MYSQL_OPT_COMPRESS,NULL);
for(;;)
 {
 if(mysql_real_connect(mySQL,li->host,li->username,li->password,defdb,li->port,NULL,CLIENT_ODBC))
  break;//connected
 if(!li->UI())
  {
  connections=0;
  return 0;
  }
 }
connections=1;
return 1;
}

//disconnect .................................................................................................................................
BOOL VMYSQL::Disconnect()
{
connections--;
if(connections>0) return 1;
mysql_close(mySQL);
mySQL=NULL;
return 1;
}


//select schema, create if it doesn't exist .................................................................................................................................
inline int VMYSQL::Database(LPSTR dbname=NULL)
{
sc(databname,dbname,sizeof(databname));
if(mysql_select_db(mySQL,dbname))
 {	//error (try creating it)
 qnc=sprintf(myQuery,"CREATE SCHEMA %s CHARACTER SET ascii COLLATE ascii_general_ci",dbname); 
 ret=mysql_real_query(mySQL,myQuery,qnc);
 if(mysql_select_db(mySQL,dbname))
  return 0; //error
 else
  return 2; //new
 }
else //ok
 return 1; //existing
}

//select default table.................................................................................................................................
inline int VMYSQL::Table(LPSTR tname=NULL)
{
sc(tablename,tname,sizeof(tablename));
return 1;
}

//test if DB exists.................................................................................................................................
inline BOOL VMYSQL::E_db(LPSTR dbname)
{
MYSQL_RES*lresult;
lresult=mysql_list_dbs(mySQL,dbname);
if(!lresult) return 0;
ret=mysql_num_rows(lresult);
mysql_free_result(lresult);
return ret;
}

//test if table exists.................................................................................................................................
inline BOOL VMYSQL::E_table(LPSTR tname)
{
MYSQL_RES*lresult;
lresult=mysql_list_tables(mySQL,tname);
if(!lresult) return 0;
ret=mysql_num_rows(lresult);
mysql_free_result(lresult);
return ret;
}

//test if field exists in the default table.................................................................................................................................
inline BOOL VMYSQL::E_field(LPSTR fieldname)
{
MYSQL_RES*lresult;
lresult=mysql_list_fields(mySQL,tablename,fieldname);
if(!lresult) return 0;
//ret=mysql_field_count(mySQL);
ret=mysql_num_fields(lresult);
mysql_free_result(lresult);
return ret;
}

//test if a field exists with a certain value, in the default table.................................................................................................................................
inline int VMYSQL::E_value(LPSTR fieldname,LPSTR value,int quote=1)
{
MYSQL_RES*lresult;
if(quote)
 qnc=sprintf(myQuery,"SELECT %s FROM %s WHERE %s='%s'",fieldname,tablename,fieldname,value); 
else
 qnc=sprintf(myQuery,"SELECT %s FROM %s WHERE %s=%s",fieldname,tablename,fieldname,value); 
ret=mysql_real_query(mySQL,myQuery,qnc);
lresult=mysql_store_result(mySQL);
if(!lresult) return 0;
ret=mysql_num_rows(lresult);
mysql_free_result(lresult);
return ret;
}

//test if a user account exists .................................................................................................................................
inline BOOL VMYSQL::E_user(LPSTR username=NULL)
{
MYSQL_RES*lresult;
qnc=sprintf(myQuery,"SELECT User FROM mysql.user WHERE User='%s'",username); 
ret=mysql_real_query(mySQL,myQuery,qnc);
lresult=mysql_store_result(mySQL);
if(!lresult) return 0;
ret=mysql_num_rows(lresult);
mysql_free_result(lresult);
return ret;
}

//delete schema.................................................................................................................................
inline BOOL VMYSQL::DropDB(LPSTR dbname=NULL)
{
qnc=sprintf(myQuery,"DROP SCHEMA IF EXISTS %s",dbname); 
ret=mysql_real_query(mySQL,myQuery,qnc);
return !ret;
}

//delete table.................................................................................................................................
inline BOOL VMYSQL::DropTable(LPSTR tname=NULL)
{
qnc=sprintf(myQuery,"DROP TABLE IF EXISTS %s",tname); 
ret=mysql_real_query(mySQL,myQuery,qnc);
return !ret;
}

//send an SQL query to the server.................................................................................................................................
inline BOOL VMYSQL::Query(char *formstr=NULL,...)
{
va_list vparam;
va_start(vparam,formstr);
qnc=vsprintf(myQuery,formstr,vparam);
ret=mysql_real_query(mySQL,myQuery,qnc);
checkret(ret,myQuery);
return !ret;
}

//start a composite query .................................................................................................................................
inline void VMYSQL::StartQuery()
{
qnc=0;
*myQuery=0;
}

//append a string to the query.................................................................................................................................
inline BOOL VMYSQL::Astr(char *formstr=NULL,...)
{
va_list vparam;
va_start(vparam,formstr);
qnc+=vsprintf(myQuery+qnc,formstr,vparam);
return qnc;
}

//append binary data to the query.................................................................................................................................
inline BOOL VMYSQL::Abin(void*pmem,NAT memsz)
{
qnc+=mysql_real_escape_string(mySQL,myQuery+qnc,(char*)pmem,memsz);
myQuery[qnc]=0;
return qnc;
}

//send the query, built with the prevoius 3 function, to the server.................................................................................................................................
inline BOOL VMYSQL::DoQuery()
{
ret=mysql_real_query(mySQL,myQuery,qnc);
//checkret(ret,myQuery);
return !ret;
}

////.................................................................................................................................
//void VMYSQL::GetRow(MYSQL_RES*myResult=NULL)
//{
//int keep=1,f;
//NAT l=0,nrr,nrf,maxfsz=0;
//MYSQL_FIELD*fields;
//MYSQL_ROW row;
//DWORD*rownc;
//if(!myResult)
// {
// myResult=mysql_store_result(mySQL);
// if(!myResult)
//  {
//  checkret(mysql_field_count(mySQL),"Couldn't get result for: %s",myQuery);
//  return;
//  }
// keep=0;
// }
//nrr=mysql_num_rows(myResult);
//nrf=mysql_num_fields(myResult);
//fields=mysql_fetch_fields(myResult);
//if(fields)
// {
// for(f=0;f<nrf;f++)
//  {
//  l+=sprintf(lsbuf+l,"[%u]%s ",fields[f].length,fields[f].name);
//  maxfsz+=fields[f].max_length;
//  }
// l--;
// l+=sprintf(lsbuf+l,"\n\n");
// } 
//for(;;)
// {
// row=mysql_fetch_row(myResult);
// if(!row) break;
// rownc=mysql_fetch_lengths(myResult);
// for(f=0;f<nrf;f++)
//  l+=sprintf(lsbuf+l,"%s\t|",row[f]?row[f]:"");
// l-=2;
// l+=sprintf(lsbuf+l,"\n");
// }
//if(l) MessageBox(hmwnd,lsbuf,myQuery,MB_OK);
//if(!keep) mysql_free_result(myResult);
//FREE(lsbuf);
//}

//get query results in a UTABLE.................................................................................................................................
BOOL VMYSQL::QResult(UTABLE*ut=NULL)
{
MYSQL_RES*myResult;
MYSQL_FIELD*fields;
MYSQL_ROW row;
NAT nrr,nrf;
int f,r;
DWORD*rownc;
if(!ut) mysql_free_result(mysql_use_result(mySQL));
myResult=mysql_store_result(mySQL);
if(!myResult)
 {
 checkret(mysql_field_count(mySQL),"Couldn't get result for: %s",myQuery);
 return 0;
 }
nrr=mysql_num_rows(myResult);
nrf=mysql_num_fields(myResult);
fields=mysql_fetch_fields(myResult);
BITS32 utype;
NAT uitsz;
if(!fields) return 0;
ut->Free();
for(f=0;f<nrf;f++)
 {
 UTYPE(fields[f].type,fields[f].decimals,fields[f].length,&utype,&uitsz);
 ut->AddField(fields[f].name,utype,nrr,uitsz,fields[f].max_length*12);
 } 
for(r=0;r<nrr;r++)
 {
 row=mysql_fetch_row(myResult);
 if(!row) break;
 rownc=mysql_fetch_lengths(myResult);
 for(f=0;f<nrf;f++)
  {
  if(row[f]) ut->field[f].scan(r,row[f],rownc[f]);
  //ut->field[f].text(r,REGs,256);
  //printbox("%s",REGs);
  }
 }
mysql_free_result(myResult);
return 1;
}

//discard query results.................................................................................................................................
inline void VMYSQL::DiscardResult()
{
mysql_free_result(mysql_use_result(mySQL));
}

//print query results - mainly for debug .................................................................................................................................
void VMYSQL::ShowResult(MYSQL_RES*myResult=NULL)
{
int keep=1,f;
char *lsbuf;
NAT l=0,nrr,nrf,maxfsz=0;
MYSQL_FIELD*fields;
MYSQL_ROW row;
DWORD*rownc;
if(!myResult)
 {
 myResult=mysql_store_result(mySQL);
 if(!myResult)
  {
  checkret(mysql_field_count(mySQL),"Couldn't get result for: %s",myQuery);
  return;
  }
 keep=0;
 }
nrr=mysql_num_rows(myResult);
nrf=mysql_num_fields(myResult);
lsbuf=SALLOC(nrr*(nrf+1)*32+1024); //average size per field = 32B
fields=mysql_fetch_fields(myResult);
if(fields)
 {
 for(f=0;f<nrf;f++)
  {
  l+=sprintf(lsbuf+l,"[%u]%s ",fields[f].length,fields[f].name);
  maxfsz+=fields[f].max_length;
  }
 l--;
 l+=sprintf(lsbuf+l,"\n\n");
 } 
for(;;)
 {
 row=mysql_fetch_row(myResult);
 if(!row) break;
 rownc=mysql_fetch_lengths(myResult);
 for(f=0;f<nrf;f++)
  l+=sprintf(lsbuf+l,"%s\t|",row[f]?row[f]:"");
 l-=2;
 l+=sprintf(lsbuf+l,"\n");
 }
if(l) MessageBox(hmwnd,lsbuf,myQuery,MB_OK);
if(!keep) mysql_free_result(myResult);
FREE(lsbuf);
}

//ping MySQL server.................................................................................................................................
BOOL VMYSQL::Ping()
{
return !mysql_ping(mySQL);
}

//show information.................................................................................................................................
void VMYSQL::Info()
{
char lsbuf[BUFFSZ];
int l=0;
l+=sprintf(lsbuf+l,"Client: %s %u\n",mysql_get_client_info(),mysql_get_client_version());
l+=sprintf(lsbuf+l,"%s protocol:%u \n",mysql_get_host_info(mySQL),mysql_get_proto_info(mySQL));
l+=sprintf(lsbuf+l,"Server: %s %u\n",mysql_get_server_info(mySQL),mysql_get_server_version(mySQL));
l+=sprintf(lsbuf+l,"%s",mysql_character_set_name(mySQL));
//l+=sprintf(lsbuf+l,"%s\n",);
MessageBox(HWND_DESKTOP,lsbuf,"MySQL INFO",MB_ICONINFORMATION|MB_OK);
}

//show status.................................................................................................................................
void VMYSQL::Status()
{
char lsbuf[BUFFSZ];
int l=0;
l+=sprintf(lsbuf+l,"Thread id: %u \n",mysql_thread_id(mySQL));
l+=sprintf(lsbuf+l,"%s\n",mysql_stat(mySQL));
l+=sprintf(lsbuf+l,"Query: %s\nAUTO_INC=%I64u\n",mysql_info(mySQL),mysql_insert_id(mySQL));
l+=sprintf(lsbuf+l,"%I64i affected rows\n",mysql_affected_rows(mySQL));
l+=sprintf(lsbuf+l,"Warnings: %u\n",mysql_warning_count(mySQL));
l+=sprintf(lsbuf+l,"Errors: %u %5s %s\n",mysql_errno(mySQL),mysql_sqlstate(mySQL),mysql_error(mySQL));
MessageBox(HWND_DESKTOP,lsbuf,"MySQL status",MB_ICONINFORMATION|MB_OK);
}

// VMYSQL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
