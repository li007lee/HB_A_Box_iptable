/*
 * box_info_upload.c
 *
 *  Created on: 2018年3月13日
 *      Author: root
 */

#include "event2/event.h"
#include "event2/event_struct.h"
#include "event2/util.h"
#include "event2/thread.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"

#include "cJSON.h"
#include "box_info_upload.h"

struct UPLOAD_SERVER_INFO stUploadServerInfo;
struct BOX_INFO stBoxInfo;
struct event evTimer; //定时器事件
pthread_mutex_t lockMutexLock = PTHREAD_MUTEX_INITIALIZER;


static HB_VOID *connect_ip_test(void *param)
{
	HB_S32 iSockFd = -1;
	HB_S32 ret = 0;
	struct DEV_STATUS *pDevInfo = (struct DEV_STATUS *)param;
//	ret = connect_ip_port_test(pDevInfo->cDevIp, pDevInfo->iPort1, 2);
	printf("pDevInfo->cDevIp:%s, pDevInfo->iPort:%d\n", pDevInfo->cDevIp, pDevInfo->iPort1);
	ret = create_socket_connect_ipaddr(&iSockFd, pDevInfo->cDevIp, pDevInfo->iPort1, 2);
	if (0 == ret)
	{
		pDevInfo->iDevStatus = 1; //能联通，表示在线
	}
	else
	{
		pDevInfo->iDevStatus = 0; //不能联通，表示不在线
	}
	close_sockfd(&iSockFd);
	if (pDevInfo->iPort2 > 0)
	{
		ret = create_socket_connect_ipaddr(&iSockFd, pDevInfo->cDevIp, pDevInfo->iPort2, 2);
		if (0 == ret)
		{
			pDevInfo->iDevStatus = 1; //能联通，表示在线
		}
		else
		{
			pDevInfo->iDevStatus = 0; //不能联通，表示不在线
		}
		close_sockfd(&iSockFd);
	}

	pthread_exit(NULL);
}


//获取一点通盒子列表回调函数
static HB_S32 load_ydt_dev_info(HB_VOID * para, HB_S32 n_column, HB_CHAR ** column_value, HB_CHAR ** column_name)
{
	HB_S32 iDevStatus = 0;
	struct DEV_STATUS *pDevInfo = malloc(sizeof(struct DEV_STATUS));
	memset(pDevInfo, 0, sizeof(struct DEV_STATUS));

	//获取盒子信息
	strncpy(pDevInfo->cDevSn, column_value[0], strlen(column_value[0]));
	strncpy(pDevInfo->cDevIp, column_value[1], strlen(column_value[1]));
	pDevInfo->iPort1 = atoi(column_value[2]);
	pDevInfo->iPort2 = atoi(column_value[3]);
	iDevStatus = atoi(column_value[4]);

	//插入链表
	list_append(&(stBoxInfo.listYdtDev), (HB_VOID*)pDevInfo);

	return 0;
}

static HB_VOID get_ydt_dev_list()
{
	HB_CHAR *sql = "select dev_id,dev_ip,dev_port,dev_port2,dev_state from dev_add_web_data";
	HB_S32 ret = 0;

	if (!list_empty(&(stBoxInfo.listYdtDev)))
	{
		//链表不为空，清空链表
		list_clear(&(stBoxInfo.listYdtDev));
	}
	sqlite3 *db;
	sqlite3_open(BOX_DATA_BASE_NAME, &db);
	sqlite3_exec(db, sql, load_ydt_dev_info, NULL, NULL);
	sqlite3_close(db);
//	SqlOperation(sql, BOX_DATA_BASE_NAME, load_ydt_dev_info, NULL);

	list_iterator_start(&(stBoxInfo.listYdtDev));
	while (list_iterator_hasnext(&(stBoxInfo.listYdtDev)))
	{
		struct DEV_STATUS *pDevInfo = (struct DEV_STATUS*)list_iterator_next(&(stBoxInfo.listYdtDev));
		pthread_attr_t attr;
		pthread_t connect_test_pthread_id;
		ret = pthread_attr_init(&attr);
		ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		ret = pthread_create(&connect_test_pthread_id, &attr, connect_ip_test, pDevInfo);
		pthread_attr_destroy(&attr);
	}
	list_iterator_stop(&(stBoxInfo.listYdtDev));
}

//获取一点通盒子列表回调函数
static HB_S32 load_onvif_dev_info(HB_VOID * para, HB_S32 n_column, HB_CHAR ** column_value, HB_CHAR ** column_name)
{
	HB_S32 iDevStatus = 0;
	struct DEV_STATUS *pDevInfo = malloc(sizeof(struct DEV_STATUS));
	memset(pDevInfo, 0, sizeof(struct DEV_STATUS));

	//获取盒子信息
	strncpy(pDevInfo->cDevSn, column_value[0], strlen(column_value[0]));
	strncpy(pDevInfo->cDevIp, column_value[1], strlen(column_value[1]));
	pDevInfo->iPort1 = atoi(column_value[2]);
	iDevStatus = atoi(column_value[3]);

	//插入链表
	list_append(&(stBoxInfo.listOnvifDev), (HB_VOID*)pDevInfo);

	return 0;
}

static HB_VOID get_onvif_dev_list()
{
	HB_CHAR *sql = "select dev_id,dev_ip,rtsp_port,dev_state from onvif_dev_data";
	HB_S32 ret = 0;

	if (!list_empty(&(stBoxInfo.listOnvifDev)))
	{
		//链表不为空，清空链表
		list_clear(&(stBoxInfo.listOnvifDev));
	}
	sqlite3 *db;
	sqlite3_open(BOX_DATA_BASE_NAME, &db);
	sqlite3_exec(db, sql, load_onvif_dev_info, NULL, NULL);
	sqlite3_close(db);

	list_iterator_start(&(stBoxInfo.listOnvifDev));
	while (list_iterator_hasnext(&(stBoxInfo.listOnvifDev)))
	{
		struct DEV_STATUS *pDevInfo = (struct DEV_STATUS*)list_iterator_next(&(stBoxInfo.listOnvifDev));
		pthread_attr_t attr;
		pthread_t connect_test_pthread_id;
		ret = pthread_attr_init(&attr);
		ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		ret = pthread_create(&connect_test_pthread_id, &attr, connect_ip_test, pDevInfo);
		pthread_attr_destroy(&attr);
	}
	list_iterator_stop(&(stBoxInfo.listOnvifDev));
}


//用于获取盒子信息的线程，设备信息每一分钟采集一次
HB_VOID *thread_get_box_info(HB_VOID *arg)
{
//	pthread_detach(pthread_self());
	while(stUploadServerInfo.iThreadStartFlag)
	{
		struct timeval tv;
		//由于DVR的NTP校时时会把北京时间校为UTC时间
		gettimeofday(&tv,NULL);
		stBoxInfo.lluRecordTime = (HB_U64)tv.tv_sec*1000 + tv.tv_usec/1000;//取毫秒值
		//获取天联在线状态
		if (test_gnLan_alive()!=0)
		{
			stBoxInfo.iGnLanStatus = 0; //天联不在线
		}
		else
		{
			stBoxInfo.iGnLanStatus = 1; //天联在线
		}

		//获取cpu使用率
		get_cpuoccupy(&(stBoxInfo.fCpu));
		//获取内存使用率
		stBoxInfo.fMem = get_memoccupy();

		pthread_mutex_lock(&lockMutexLock);
		get_ydt_dev_list();//获取一点通设备列表

		get_onvif_dev_list();//获取onvif设备列表
		if (!list_empty(&(stBoxInfo.listYdtDev)) || !list_empty(&(stBoxInfo.listOnvifDev)))
		{
//			printf("sleep 3\n");
			sleep(3);
		}
		//探测设备连通性
		pthread_mutex_unlock(&lockMutexLock);

		sleep(10);
	}

	printf("thread_get_box_info exit!\n");
	pthread_exit(NULL);
}


//盒子信息上报的定时器
static void upload_box_info_timer(evutil_socket_t fd, short events, void *arg)
{
	struct event* evTimer = arg;
	struct timeval tv = {0, 0};
	HB_CHAR cDevInfo[256] = {0};
	HB_CHAR cUploadInfo[4096] = {0};
	HB_CHAR cYdtDevStatusList[2048] = {0};
	HB_CHAR cOnvifDevStatusList[2048] = {0};

	pthread_mutex_lock(&lockMutexLock);
	if (!list_empty(&(stBoxInfo.listYdtDev)))
	{
		list_iterator_start(&(stBoxInfo.listYdtDev));
		while (list_iterator_hasnext(&(stBoxInfo.listYdtDev)))
		{
			struct DEV_STATUS *pDevInfo = (struct DEV_STATUS*)list_iterator_next(&(stBoxInfo.listYdtDev));
			snprintf(cDevInfo, sizeof(cDevInfo), "{\"devSn\":\"%s\",\"status\":%d},", pDevInfo->cDevSn, pDevInfo->iDevStatus);
			strncat(cYdtDevStatusList, cDevInfo, sizeof(cYdtDevStatusList)-strlen(cYdtDevStatusList));
//			printf("str1[%s], str2[%s]\n", cYdtDevStatusList, cDevInfo);
		}
		list_iterator_stop(&(stBoxInfo.listYdtDev));
		cYdtDevStatusList[strlen(cYdtDevStatusList)-1] = '\0';
	}

	if (!list_empty(&(stBoxInfo.listOnvifDev)))
	{
		list_iterator_start(&(stBoxInfo.listOnvifDev));
		while (list_iterator_hasnext(&(stBoxInfo.listOnvifDev)))
		{
			struct DEV_STATUS *pDevInfo = (struct DEV_STATUS*)list_iterator_next(&(stBoxInfo.listOnvifDev));
			snprintf(cDevInfo, sizeof(cDevInfo), "{\"devSn\":\"%s\",\"status\":%d},", pDevInfo->cDevSn, pDevInfo->iDevStatus);
			strncat(cOnvifDevStatusList, cDevInfo, sizeof(cOnvifDevStatusList)-strlen(cOnvifDevStatusList));
//			printf("str1[%s], str2[%s]\n", cYdtDevStatusList, cDevInfo);
		}
		list_iterator_stop(&(stBoxInfo.listYdtDev));
		cOnvifDevStatusList[strlen(cOnvifDevStatusList)-1] = '\0';
	}

	pthread_mutex_unlock(&lockMutexLock);
	//拼接消息体
	snprintf(cUploadInfo, sizeof(cUploadInfo), \
		"{\"datas\":{\"boxSn\":\"%s\",\"gnLan\":%d,\"cpu\":%.2f, \"mem\":%.2f, \"recordTime\":%llu,\"ydtDevStatus\":[%s],\"onvifDevStatus\":[%s]}}", \
		stBoxInfo.cBoxSn, stBoxInfo.iGnLanStatus, stBoxInfo.fCpu, stBoxInfo.fMem, stBoxInfo.lluRecordTime, cYdtDevStatusList, cOnvifDevStatusList);

	printf("send buf:[%s]\n", cUploadInfo);
	tv.tv_sec = stUploadServerInfo.iUploadInterval;
	event_add(evTimer, &tv);
}


HB_VOID deal_cmd(struct bufferevent *pConnectUploadServerBev, void *arg)
{

	struct timeval tv = {0, 0};
	struct event_base *pTimerEventBase = (struct event_base *)arg;
	HB_CHAR cRecvBuf[512] = {0};

	bufferevent_read(pConnectUploadServerBev, cRecvBuf, sizeof(cRecvBuf));
	cJSON *pRoot;
	pRoot = cJSON_Parse(cRecvBuf);
	cJSON *pCode = cJSON_GetObjectItem(pRoot, "code");
	if(pCode->valueint == 0)
	{
		cJSON *pDatas = cJSON_GetObjectItem(pRoot, "datas");
		cJSON *pInterval = cJSON_GetObjectItem(pDatas, "interval");
		if (pInterval->valueint <= 10)
		{
			stUploadServerInfo.iUploadInterval = 300;//若获取的数据有误，默认5分钟上报一次
		}
		else
		{
			stUploadServerInfo.iUploadInterval = pInterval->valueint;
		}
	}

	printf("stUploadServerInfo.iUploadInterval=%d\n", stUploadServerInfo.iUploadInterval);
	tv.tv_sec = stUploadServerInfo.iUploadInterval;
	event_assign(&evTimer, pTimerEventBase, -1, EV_PERSIST, upload_box_info_timer, (HB_VOID*)&evTimer);
	event_add(&evTimer, &tv);
	bufferevent_free(pConnectUploadServerBev);
}


static HB_VOID connect_to_upload_server_event_cb(struct bufferevent *pConnectUploadServerBev, HB_S16 what, HB_VOID *arg)
{
	HB_CHAR cSendbuf[128] = {0};

	if (what & BEV_EVENT_CONNECTED)//盒子主动连接rtsp服务器成功
	{
		snprintf(cSendbuf, sizeof(cSendbuf), "{\"datas\":{\"BoxSn\":\"%s\"}}", stBoxInfo.cBoxSn);
		bufferevent_write(pConnectUploadServerBev, cSendbuf, strlen(cSendbuf));

	    struct timeval tv = {10,0};
	    bufferevent_set_timeouts(pConnectUploadServerBev, &tv, NULL);
		bufferevent_setcb(pConnectUploadServerBev, deal_cmd, NULL, NULL, (HB_VOID *)arg);
		bufferevent_enable (pConnectUploadServerBev, EV_READ);

//		bufferevent_free(pConnectUploadServerBev);
//		tv.tv_sec = stUploadServerInfo.iUploadInterval;
//		event_assign(&evTimer, pTimerEventBase, -1, EV_PERSIST, upload_box_info_timer, (HB_VOID*)&evTimer);
//		event_add(&evTimer, &tv);
	}
	else  //盒子connect rtsp服务器失败
	{
//		bufferevent_free(pConnectUploadServerBev);
		printf("connect to upload server failed!\n");
		struct sockaddr_in stServeraddr;
		HB_S32 iServeraddrLen;
//		struct bufferevent *pConnectUploadServerEvent; //连接上报服务器事件
//		pConnectUploadServerEvent = bufferevent_socket_new(pTimerEventBase, -1, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS|BEV_OPT_THREADSAFE);
		bzero(&stServeraddr, sizeof(stServeraddr));
		stServeraddr.sin_family = AF_INET;
		stServeraddr.sin_port = htons(stUploadServerInfo.iUploadServerPort);
		inet_pton(AF_INET, stUploadServerInfo.cUploadServerIp, &stServeraddr.sin_addr);
		iServeraddrLen = sizeof(struct sockaddr_in);

		bufferevent_setcb(pConnectUploadServerBev, NULL, NULL, connect_to_upload_server_event_cb, (HB_VOID *)arg);
		bufferevent_socket_connect(pConnectUploadServerBev, (struct sockaddr*)&stServeraddr, iServeraddrLen);
	}
}



HB_VOID *thread_hb_box_info_upload(HB_VOID *arg)
{
	pthread_detach(pthread_self());
	stUploadServerInfo.iThreadStartFlag = 1;
	struct event_base *pTimerEventBase;

	memset(&stBoxInfo, 0, sizeof(stBoxInfo));
	list_init(&(stBoxInfo.listOnvifDev));
	list_init(&(stBoxInfo.listYdtDev));

	get_sys_sn(stBoxInfo.cBoxSn, sizeof(stBoxInfo.cBoxSn));

	pthread_t threadGetBoxInfo = -1;
	pthread_create(&threadGetBoxInfo, NULL, thread_get_box_info, NULL);

	pTimerEventBase = event_base_new();
	if (!pTimerEventBase)
	{
		perror("cmd_base event_base_new()");
		stUploadServerInfo.iThreadStartFlag = 0;
		pthread_mutex_lock(&lockMutexLock);
		pthread_cancel(threadGetBoxInfo);
		pthread_join(threadGetBoxInfo, NULL);
		pthread_mutex_unlock(&lockMutexLock);
		pthread_exit(NULL);
	}
	//用于线程安全
	if(evthread_make_base_notifiable(pTimerEventBase) != 0)
	{
		TRACE_ERR("###### evthread_make_base_notifiable() err!");
		stUploadServerInfo.iThreadStartFlag = 0;
		pthread_mutex_lock(&lockMutexLock);
		pthread_cancel(threadGetBoxInfo);
		pthread_join(threadGetBoxInfo, NULL);
		pthread_mutex_unlock(&lockMutexLock);
		pthread_exit(NULL);
	}

	strcpy(stUploadServerInfo.cUploadServerIp, "192.168.8.12");
	stUploadServerInfo.iUploadServerPort = 12345;

	struct sockaddr_in stServeraddr;
	HB_S32 iServeraddrLen;
	struct bufferevent *pConnectUploadServerEvent; //连接上报服务器事件
	pConnectUploadServerEvent = bufferevent_socket_new(pTimerEventBase, -1, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS|BEV_OPT_THREADSAFE);
	bzero(&stServeraddr, sizeof(stServeraddr));
	stServeraddr.sin_family = AF_INET;
	stServeraddr.sin_port = htons(stUploadServerInfo.iUploadServerPort);
	inet_pton(AF_INET, stUploadServerInfo.cUploadServerIp, &stServeraddr.sin_addr);
	iServeraddrLen = sizeof(struct sockaddr_in);

	bufferevent_setcb(pConnectUploadServerEvent, NULL, NULL, connect_to_upload_server_event_cb, (HB_VOID *)pTimerEventBase);
	bufferevent_socket_connect(pConnectUploadServerEvent, (struct sockaddr*)&stServeraddr, iServeraddrLen);

	event_base_dispatch(pTimerEventBase);
	event_base_free(pTimerEventBase);
	printf("timer thread exit~!\n");
	stUploadServerInfo.iThreadStartFlag = 0;

	pthread_mutex_lock(&lockMutexLock);
	pthread_cancel(threadGetBoxInfo);
	pthread_join(threadGetBoxInfo, NULL);
	pthread_mutex_unlock(&lockMutexLock);
	pthread_exit(NULL);
}
