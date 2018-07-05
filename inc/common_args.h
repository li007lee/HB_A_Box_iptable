/*
 * common_args.h
 *
 *  Created on: Jul 5, 2018
 *      Author: root
 */

#ifndef INC_COMMON_ARGS_H_
#define INC_COMMON_ARGS_H_

#include "my_include.h"

typedef struct
{
	HB_S32 iWanOpenFlag; //用于标记是否启用广域网 1启用 0未启用
	sqlite3 *pSqliteDbHandle;
	HB_CHAR cHeartbeatServerIp[16];
	HB_S32	iHeartbeatPort;
	HB_CHAR cVersion[16];	//盒子版本号
	HB_CHAR cBoxType[32];	//盒子类型
	HB_CHAR cBoxSn[32];		//盒子序列号
	HB_CHAR cMachineCode[32];
}GLOBLE_ARGS_STRUCT;

extern GLOBLE_ARGS_STRUCT glCommonArgs; //全局通用结构体

#endif /* INC_COMMON_ARGS_H_ */
