/*
 * hf_plant_api.h
 *
 *  Created on: 2015年1月5日
 *      Author: root
 */

#ifndef HF_PLANT_API_H_
#define HF_PLANT_API_H_

#include "sqlite3.h"

typedef enum {
	REGISTER = 1,
	GET_TOKEN,
	GETSTREAMINFO,
	GET_HEARTBEAT_SERVER_INFO
} OPT_TYPE;

HB_S32 sleep_appoint_time(HB_S32 times);
//times为次数 从1开始n次结束, base_num为基数
HB_S32 sleep_times(HB_S32 times, HB_S32 base_num);
HB_S32 ODD_CHECK_PUT_MSG(HB_S32 *sockfd, enum_err_define err_index);//异常诊断数据上传
//设备注册以及取令牌接口，成功返回HB_SUCCESS，失败返回HB_FAILURE
HB_S32 hb_box_opt_cmd_exec(HB_S32 *pSockfd, OPT_TYPE enumOptCmd);
HB_S32 pt_device_register(HB_S32 *sockfd);


HB_S32 hb_box_get_streamserver_info(HB_S32 *sockfd);
HB_S32 hb_box_get_heartbeatserver_info(HB_S32 *sockfd);

void *scanning_task(void *param);

//开机配置辅助ip和路由
HB_S32 set_network(sqlite3 *db);

//计算MD5
int calculate_md5(char *pDest, const char *cSrc);

#endif /* HF_PLANT_API_H_ */
