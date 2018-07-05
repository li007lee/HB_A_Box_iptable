/*
 * my_sqlite.h
 *
 *  Created on: Jul 5, 2018
 *      Author: root
 */

#ifndef MY_SQLITE_H_
#define MY_SQLITE_H_

#include "my_include.h"

#include "sqlite3.h"

HB_S32 my_sqlite_open(const HB_CHAR *pDatabaseName, sqlite3 **ppSqliteDbHandle);
HB_S32 my_sqlite_exec(sqlite3 *pSqliteDbHandle, HB_CHAR *pSqlCmd,  HB_S32 (*pCallBack)(HB_VOID*,HB_S32,HB_CHAR**,HB_CHAR**), HB_HANDLE hArgs);
HB_S32 my_sqlite_close(sqlite3 **ppSqliteDbHandle);

#endif /* MY_SQLITE_H_ */
