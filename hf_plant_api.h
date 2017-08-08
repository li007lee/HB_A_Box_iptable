/*
 * hf_plant_api.h
 *
 *  Created on: 2015年1月5日
 *      Author: root
 */

#ifndef HF_PLANT_API_H_
#define HF_PLANT_API_H_


typedef enum {
	REGISTER = 1,
	TOKEN,
	GETSTREAMINFO
} OPT_TYPE;

HB_S32 sleep_appoint_time(HB_S32 times);
//times为次数 从1开始n次结束, base_num为基数
HB_S32 sleep_times(HB_S32 times, HB_S32 base_num);
HB_S32 ODD_CHECK_PUT_MSG(HB_S32 *sockfd, enum_err_define err_index);//异常诊断数据上传
HB_S32 hb_box_get_token(HB_S32 *sockfd);
HB_S32 hb_box_register(HB_S32 *sockfd);
HB_S32 pt_device_register(HB_S32 *sockfd);
//计算MD5
int Calculate_MD5(char *desc, const char *src);

HB_S32 hb_box_get_streamserver_info(HB_S32 *sockfd);

void *scanning_task(void *param);

#endif /* HF_PLANT_API_H_ */
