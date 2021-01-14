//VERSION 1.0.a
#pragma once

class USQLDB;

#include <net.cpp>
#include <udata.cpp>
#include <utable.cpp>

//USQLDB types
#define USQL_ODBC		1
#define USQL_MYSQL		2
//#define USQL_DBF		4

//Polymorphic class used to describe a generic database ~~~~~~~~~~~~~~~~~~~~~~`
class USQLDB
{
public:
 DWORD type;
 int connections;
 char databname[PATHSZ];
 char tablename[NAMESZ];

 virtual BOOL Connect(LOGON_INFO*,char*)=0;
 virtual BOOL Disconnect()=0;
 virtual int Table(LPSTR)=0;
 virtual BOOL DropTable(LPSTR)=0;  //drop table
 virtual BOOL E_db(LPSTR)=0;
 virtual BOOL E_table(LPSTR)=0;
 virtual BOOL E_field(LPSTR)=0;
 virtual int E_value(LPSTR,LPSTR,int)=0;
 virtual BOOL Query(char*,...)=0;
 virtual void StartQuery()=0;
 virtual BOOL Astr(char*,...)=0;
 virtual BOOL Abin(void*,NAT)=0;
 virtual BOOL DoQuery()=0;
 virtual BOOL QResult(UTABLE*)=0;
 //not garanted to be universally supported (mostly dependent on SQL grammar)
 virtual int Database(LPSTR)=0;
 virtual BOOL DropDB(LPSTR)=0;  //drop database
 
};