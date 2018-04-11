/*
 * box_info_upload.h
 *
 *  Created on: 2018年3月13日
 *      Author: root
 */

#ifndef BOX_INFO_UPLOAD_H_
#define BOX_INFO_UPLOAD_H_

#include "my_include.h"
#include "simclist.h"
#include "get_set_config.h"
#include "net_api.h"
#include "event2/event.h"
#include "event2/event_struct.h"
#include "event2/util.h"
#include "event2/thread.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "md5gen.h"
#include "cJSON.h"

struct UPLOAD_SERVER_INFO {
	HB_CHAR cUploadServerDomain[64]; //上报服务器的域名
	HB_CHAR cUploadServerIp[16]; //上报服务器的ip
	HB_S32	iUploadServerPort;	//上报服务器的端口
	HB_S32	iUploadInterval;	//数据上报的时间间隔
	HB_S32 	iIfUpload;		//是否上报 1上报 0不上报
	HB_S32	iThreadStartFlag;	//上报线程启动标识（1已启动，0未启动）
	HB_CHAR cAccessToken[128];
};

//设备状态结构体
struct DEV_STATUS {
	HB_CHAR cDevIp[16];
	HB_S32	iPort1;
	HB_S32	iPort2;
	HB_CHAR cDevSn[64]; //盒子序列号
	HB_S32	iDevStatus;	//设备在线状态 （1：在线，0：不在线）
};

struct BOX_INFO {
	HB_S32	iGnLanStatus;	//天联在线状态 （1：在线，0：不在线）
	HB_FLOAT	fCpu;	//cpu使用率
	HB_FLOAT	fMem;	//内存使用率
	HB_U64 lluRecordTime;	//数据采集时间
    HB_CHAR cVersion[16];
    HB_CHAR cBoxType[32];
	HB_CHAR cBoxSn[32]; //盒子序列号
	list_t listOnvifDev;
	list_t listYdtDev;
};

HB_VOID *thread_hb_box_info_upload(HB_VOID *arg);
HB_VOID deal_cmd(struct bufferevent *pConnectUploadServerBev, HB_VOID *arg);

extern struct BOX_INFO stBoxInfo;
extern struct UPLOAD_SERVER_INFO stUploadServerInfo;

#endif /* BOX_INFO_UPLOAD_H_ */
