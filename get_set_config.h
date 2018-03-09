/*
 * get_config.h
 *
 *  Created on: 2015年1月5日
 *      Author: root
 */

#ifndef GET_SET_CONFIG_H_
#define GET_SET_CONFIG_H_

#include "sqlite3.h"

HB_S32 update_network_conf(HB_CHAR *str,const HB_CHAR *file);

//用于获取是否开启了广域网
//返回0 关闭， 返回1 开启
HB_S32 get_wan_connection_status(sqlite3 *db);

//判断进程是否存在
HB_S32 get_ps_status(HB_CHAR *cmd);

//测试天联是否正常，正常返回0,不正常返回-1
HB_S32 test_gnLan_alive();
//获取gnLan登录信息
HB_S32 get_sys_gnLan(HB_VOID);


#endif /* GET_SET_CONFIG_H_ */
