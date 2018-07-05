/*
 * my_sqlite.c
 *
 *  Created on: Jul 5, 2018
 *      Author: root
 */

#include "my_sqlite.h"

HB_S32 my_sqlite_open(const HB_CHAR *pDatabaseName, sqlite3 **ppSqliteDbHandle)
{
	HB_S32 iRet = 0;
	iRet = sqlite3_open(pDatabaseName, ppSqliteDbHandle);
	if (iRet != SQLITE_OK)
	{
		printf("***************%s:%d***************\nsqlite3_open [%s] failed err_code[%d]\n", __FILE__, __LINE__, pDatabaseName, iRet);
		return HB_FAILURE;
	}

	return HB_SUCCESS;
}

static HB_S32 callback_busy(HB_HANDLE hPtr,HB_S32 iCount)
{
	usleep(500000);   //如果获取不到锁，等待0.5秒
	printf("\n************  database is locak now,can not write/read.\n");  //每次执行一次回调函数打印一次该信息
	return 1;    //回调函数返回值为1，则将不断尝试操作数据库。
}

HB_S32 my_sqlite_exec(sqlite3 *pSqliteDbHandle, HB_CHAR *pSqlCmd,  HB_S32 (*pCallBack)(HB_VOID*,HB_S32,HB_CHAR**,HB_CHAR**), HB_HANDLE hArgs)
{
	HB_S32 iRet = 0;
	HB_CHAR *pErrMsg = NULL;

	sqlite3_busy_handler(pSqliteDbHandle, callback_busy,(HB_VOID *)pSqliteDbHandle);
	printf("\n####my_sqlite_exec() Sqlite SQL [%s]\n", pSqlCmd);

	if(0 == pCallBack)
	{
		iRet = sqlite3_exec(pSqliteDbHandle, pSqlCmd, 0, NULL, &pErrMsg);
	}
	else
	{
		iRet = sqlite3_exec(pSqliteDbHandle, pSqlCmd, pCallBack, hArgs, &pErrMsg);
	}
	if (iRet != SQLITE_OK)
	{
		printf("***************%s:%d***************\n my_sqlite_exec [%s] error[%d]:%s\n", __FILE__, __LINE__, pSqlCmd, iRet, pErrMsg);
    	sqlite3_free(pErrMsg);
		return HB_FAILURE;
	}
	sqlite3_free(pErrMsg);

	return HB_SUCCESS;
}

HB_S32 my_sqlite_close(sqlite3 **ppSqliteDbHandle)
{
	sqlite3_close(*ppSqliteDbHandle);
	*ppSqliteDbHandle = NULL;

	return HB_SUCCESS;
}

