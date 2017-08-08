/*
 * get_config.h
 *
 *  Created on: 2015年1月5日
 *      Author: root
 */

#ifndef GET_SET_CONFIG_H_
#define GET_SET_CONFIG_H_
HB_S32 update_network_conf(HB_CHAR *str,const HB_CHAR *file);
HB_S32 get_ip_dev(HB_CHAR *eth, HB_CHAR *ipaddr);
HB_S32 get_sys_sn(HB_CHAR *sn, HB_S32 size);

//用于获取是否开启了广域网
//返回0 关闭， 返回1 开启
HB_S32 GetWanConnectionStatus();

//判断进程是否存在
HB_S32 get_ps_status(HB_CHAR *cmd);

//测试天联是否正常，正常返回0,不正常返回-1
HB_S32 test_gnLan_alive();
HB_S32 get_sys_gnLan(HB_VOID);
//获取MAC
HB_U64 get_sys_mac();

#endif /* GET_SET_CONFIG_H_ */
