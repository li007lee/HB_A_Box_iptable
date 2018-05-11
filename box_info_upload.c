/*
 * box_info_upload.c
 *
 *  Created on: 2018年3月13日
 *      Author: root
 */

#include "box_info_upload.h"

#define UPLOAD_APPID "rWy3-dSGvWhIhhsFSCzKbA"
#define UPLOAD_UNION_ID "GpzXx5cW0AERDuUmseHmkQ"
#define UPLOAD_AUTH_TOKEN	"PvPmWR-f46VKlIsTrT5QnXq80IEhbxPv4wpHy8jhAdnVqN_e"

struct UPLOAD_SERVER_INFO stUploadServerInfo;
struct BOX_INFO stBoxInfo;
struct event evTimer; //定时器事件
pthread_mutex_t lockMutexLock = PTHREAD_MUTEX_INITIALIZER;


/*
 *	Function: 处理与客户端信令交互时产生的异常和错误(第一次信令交互)
 *
 *	@param bev: 异常产生的事件句柄
 *	@param events: 异常事件类型
 *  @parmm args	: 实际为LIBEVENT_ARGS_HANDLE类型的参数结构体
 *
 *	Retrun: 无
 */
static HB_VOID error_cb(struct bufferevent *pConnectUploadServerBev, HB_S16 events, HB_VOID *arg)
{
	HB_S32 err = EVUTIL_SOCKET_ERROR();
//	struct timeval tv = {0, 0};
//	struct event_base *pTimerEventBase = (struct event_base *)arg;

	if (events & BEV_EVENT_EOF)//对端关闭
	{
		TRACE_ERR("######## error_cb BEV_EVENT_EOF(%d) : %s !", err, evutil_socket_error_to_string(err));
	}
	else if (events & BEV_EVENT_ERROR)//错误事件
	{
		TRACE_ERR("######## error_cb BEV_EVENT_ERROR(%d) : %s !", err, evutil_socket_error_to_string(err));
	}
	else if (events & BEV_EVENT_TIMEOUT)//超时事件
	{
		TRACE_ERR("######## error_cb BEV_EVENT_TIMEOUT(%d) : %s !", err, evutil_socket_error_to_string(err));
	}

//	stUploadServerInfo.iUploadInterval = DEFAULT_UPLOAD_INTERVAL;//若获取的数据有误，默认5分钟上报一次
//	printf("stUploadServerInfo.iUploadInterval=%d\n", stUploadServerInfo.iUploadInterval);
//	tv.tv_sec = stUploadServerInfo.iUploadInterval;
//	event_assign(&evTimer, pTimerEventBase, -1, EV_PERSIST, upload_box_info_timer, (HB_VOID*)&evTimer);
//	event_add(&evTimer, &tv);
	bufferevent_free(pConnectUploadServerBev);
}


static HB_VOID *connect_ip_test(void *param)
{
	HB_S32 iSockFd = -1;
	HB_S32 ret = 0;
	struct DEV_STATUS *pDevInfo = (struct DEV_STATUS *)param;
//	ret = connect_ip_port_test(pDevInfo->cDevIp, pDevInfo->iPort1, 2);
	printf("pDevInfo->cDevIp:%s, pDevInfo->iPort:%d\n", pDevInfo->cDevIp, pDevInfo->iPort1);
	ret = create_socket_connect_ipaddr(&iSockFd, pDevInfo->cDevIp, pDevInfo->iPort1, 2);
	if (HB_SUCCESS == ret)
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
		if (HB_SUCCESS == ret)
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
	struct DEV_STATUS *pDevInfo = malloc(sizeof(struct DEV_STATUS));
	memset(pDevInfo, 0, sizeof(struct DEV_STATUS));

	//获取盒子信息
	strncpy(pDevInfo->cDevSn, column_value[0], strlen(column_value[0]));
	strncpy(pDevInfo->cDevIp, column_value[1], strlen(column_value[1]));
	pDevInfo->iPort1 = atoi(column_value[2]);
	pDevInfo->iPort2 = atoi(column_value[3]);
	pDevInfo->iDevStatus = atoi(column_value[4]);

	//插入链表
	list_append(&(stBoxInfo.listYdtDev), (HB_VOID*)pDevInfo);

	return 0;
}

static HB_VOID get_ydt_dev_list()
{
	HB_CHAR *sql = "select dev_id,dev_ip,dev_port,dev_port2,dev_state from dev_add_web_data";

	if (!list_empty(&(stBoxInfo.listYdtDev)))
	{
		//链表不为空，清空链表
		list_clear(&(stBoxInfo.listYdtDev));
	}
	sqlite3 *db;
	sqlite3_open(BOX_DATA_BASE_NAME, &db);
	sqlite3_exec(db, sql, load_ydt_dev_info, NULL, NULL);
	sqlite3_close(db);

	list_iterator_start(&(stBoxInfo.listYdtDev));
	while (list_iterator_hasnext(&(stBoxInfo.listYdtDev)))
	{
		struct DEV_STATUS *pDevInfo = (struct DEV_STATUS*)list_iterator_next(&(stBoxInfo.listYdtDev));
		pthread_attr_t attr;
		pthread_t connect_test_pthread_id;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&connect_test_pthread_id, &attr, connect_ip_test, pDevInfo);
		pthread_attr_destroy(&attr);
	}
	list_iterator_stop(&(stBoxInfo.listYdtDev));
}

//获取一点通盒子列表回调函数
static HB_S32 load_onvif_dev_info(HB_VOID * para, HB_S32 n_column, HB_CHAR ** column_value, HB_CHAR ** column_name)
{
	struct DEV_STATUS *pDevInfo = malloc(sizeof(struct DEV_STATUS));
	memset(pDevInfo, 0, sizeof(struct DEV_STATUS));

	//获取盒子信息
	strncpy(pDevInfo->cDevSn, column_value[0], strlen(column_value[0]));
	strncpy(pDevInfo->cDevIp, column_value[1], strlen(column_value[1]));
	pDevInfo->iPort1 = atoi(column_value[2]);
	pDevInfo->iDevStatus = atoi(column_value[3]);

	//插入链表
	list_append(&(stBoxInfo.listOnvifDev), (HB_VOID*)pDevInfo);

	return 0;
}

static HB_VOID get_onvif_dev_list()
{
	HB_CHAR *sql = "select dev_id,dev_ip,rtsp_port,dev_state from onvif_dev_data";

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
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&connect_test_pthread_id, &attr, connect_ip_test, pDevInfo);
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

		sleep(BOX_INFO_COLLECT_INTERVAL);
	}

	printf("thread_get_box_info exit!\n");
	pthread_exit(NULL);
}


void make_sign(char *pDest, const char *cSrc)
{
	HB_CHAR buf[4096] = {0};

	snprintf(buf, sizeof(buf), "%s%s", cSrc, UPLOAD_AUTH_TOKEN);
//	printf("make md5=[%s]\n", buf);
	md5_packages_string(pDest, buf, strlen(buf));
//	printf("&&&&&&&&&&md5=[%s]\n", pDest);
}


static HB_VOID connect_and_upload_box_info(struct bufferevent *pConnectUploadServerBev, HB_S16 what, HB_VOID *arg)
{
	if (what & BEV_EVENT_CONNECTED)//盒子主动连接rtsp服务器成功
	{
		HB_CHAR sign[64] = {0};
		HB_CHAR cDevInfo[256] = {0};
		HB_CHAR cUrl[512] = {0};
		HB_CHAR cUploadInfo[8192] = {0};
		HB_CHAR cYdtDevStatusList[4096] = {0};
		HB_CHAR cOnvifDevStatusList[4096] = {0};
		HB_CHAR cSendBuff[10240] = {0};
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
		struct timeval tv;
		//由于DVR的NTP校时时会把北京时间校为UTC时间
		gettimeofday(&tv,NULL);
		HB_U64 lluRegTime = (HB_U64)tv.tv_sec*1000 + tv.tv_usec/1000;//取毫秒值
		snprintf(cUploadInfo, sizeof(cUploadInfo), \
			"{\"root\":{\"access_token\":\"%s\",\"stamp\":\"%llu\",\"datas\":{\"boxSn\":\"%s\",\"gnLan\":%d,\"cpu\":%.2f, \"mem\":%.2f, \"recordTime\":\"%llu\",\"ydtDevStatus\":[%s],\"onvifDevStatus\":[%s]}}}", \
			stUploadServerInfo.cAccessToken, lluRegTime, stBoxInfo.cBoxSn, stBoxInfo.iGnLanStatus, stBoxInfo.fCpu, stBoxInfo.fMem, stBoxInfo.lluRecordTime, cYdtDevStatusList, cOnvifDevStatusList);
		//计算签名
		make_sign(sign, cUploadInfo);
		//拼接发送字符串
		snprintf(cUrl, sizeof(cUrl), "/"UPLOAD_UNION_ID"/4QAEAAEBAB4/BoxStatusUp/?app_id="UPLOAD_APPID"&sign=%s", sign);
		snprintf(cSendBuff, sizeof(cSendBuff), "POST %s HTTP/1.1\r\nHost:%s:%d\r\ncontent-type: text/plain; charset=utf-8\r\nContent-Length: %d\r\nConnection:keep-alive\r\n\r\n%s",
				cUrl, stUploadServerInfo.cUploadServerIp, stUploadServerInfo.iUploadServerPort, strlen(cUploadInfo), cUploadInfo);
		printf("upload upload : [%s]\n", cSendBuff);


		bufferevent_write(pConnectUploadServerBev, cSendBuff, strlen(cSendBuff));
	    struct timeval tv_read = {10,0};
	    bufferevent_set_timeouts(pConnectUploadServerBev, &tv_read, NULL);
		bufferevent_setcb(pConnectUploadServerBev, deal_cmd, NULL, error_cb, arg);
		bufferevent_enable (pConnectUploadServerBev, EV_READ);
	}
	else  //盒子connect rtsp服务器失败
	{
		printf("connect to upload server failed!\n");
//		struct timeval tv = {0, 0};
//		struct event_base *pTimerEventBase = (struct event_base *)arg;
//		stUploadServerInfo.iUploadInterval = DEFAULT_UPLOAD_INTERVAL;//若获取的数据有误，默认5分钟上报一次
//		printf("stUploadServerInfo.iUploadInterval=%d\n", stUploadServerInfo.iUploadInterval);
//		tv.tv_sec = stUploadServerInfo.iUploadInterval;
//		event_assign(&evTimer, pTimerEventBase, -1, EV_PERSIST, upload_box_info_timer, (HB_VOID*)&evTimer);
//		event_add(&evTimer, &tv);
		bufferevent_free(pConnectUploadServerBev);
	}
}


//盒子信息上报的定时器
static HB_VOID upload_box_info_timer(evutil_socket_t fd, HB_S16 events, HB_VOID *arg)
{
//	struct event* evTimer = arg;
	struct timeval tv = {0, 0};
	struct event_base *pTimerEventBase = (struct event_base *)arg;
	struct sockaddr_in stServeraddr;
	struct bufferevent *pConnectUploadServerEvent; //连接上报服务器事件

	//连接服务器并上报信息
	if (stUploadServerInfo.iIfUpload)
	{
		pConnectUploadServerEvent = bufferevent_socket_new(pTimerEventBase, -1, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS|BEV_OPT_THREADSAFE);
		bzero(&stServeraddr, sizeof(stServeraddr));
		stServeraddr.sin_family = AF_INET;
		stServeraddr.sin_port = htons(stUploadServerInfo.iUploadServerPort);
		inet_pton(AF_INET, stUploadServerInfo.cUploadServerIp, &stServeraddr.sin_addr);
		bufferevent_setcb(pConnectUploadServerEvent, NULL, NULL, connect_and_upload_box_info, (HB_VOID *)pTimerEventBase);
		bufferevent_socket_connect(pConnectUploadServerEvent, (struct sockaddr*)&stServeraddr, sizeof(struct sockaddr_in));
		bufferevent_enable(pConnectUploadServerEvent, EV_READ|EV_WRITE);
	}
	else
	{
		printf("2***************\n");
		tv.tv_sec = stUploadServerInfo.iUploadInterval;
		event_assign(&evTimer, pTimerEventBase, -1, EV_PERSIST, upload_box_info_timer, (HB_VOID*)pTimerEventBase);
		event_add(&evTimer, &tv);
	}
}


HB_VOID deal_cmd(struct bufferevent *pConnectUploadServerBev, HB_VOID *arg)
{
	static HB_S32 iFlag = 1;
	HB_CHAR *pPos = NULL;
	struct timeval tv = {0, 0};
	struct event_base *pTimerEventBase = (struct event_base *)arg;
	HB_CHAR cRecvBuf[512] = {0};

	bufferevent_read(pConnectUploadServerBev, cRecvBuf, sizeof(cRecvBuf));

	printf("recv buf[%s]\n", cRecvBuf);

	pPos = strstr(cRecvBuf, "{");
	if (pPos == NULL)
	{
		TRACE_ERR("get wrong msg!\n");
		bufferevent_free(pConnectUploadServerBev);
		return;
	}

	if (strstr(pPos, "code") == NULL)
	{
		TRACE_ERR("get wrong msg!\n");
//		stUploadServerInfo.iUploadInterval = DEFAULT_UPLOAD_INTERVAL;//若获取的数据有误，默认5分钟上报一次
	}
	else
	{
		cJSON *pRoot;
		HB_S32 i = 0;
		pRoot = cJSON_Parse(pPos);
		cJSON *pCode = cJSON_GetObjectItem(pRoot, "code");
		if(atoi(pCode->valuestring) == 1)
		{
			cJSON *pDatas = cJSON_GetObjectItem(pRoot, "datas");
			HB_S32 iArrySize = cJSON_GetArraySize(pDatas);
			printf("iArrySize=%d\n", iArrySize);
			for (i=0;i<iArrySize;i++)
			{
				HB_CHAR *pPosStart = NULL;
				HB_CHAR *pPosEnd = NULL;
				cJSON *pArryRoot = cJSON_GetArrayItem(pDatas, i);
				cJSON *pServerUrl = cJSON_GetObjectItem(pArryRoot, "serverUrl");//上报地址
				if (pServerUrl != NULL)
				{
					pPosStart = strstr(pServerUrl->valuestring, "http://");
					if (pPosStart != NULL)
						pPosStart += strlen("http://");
					pPosEnd = strstr(pPosStart, ":");

					strncpy(stUploadServerInfo.cUploadServerDomain, pPosStart, pPosEnd-pPosStart);
					from_domain_to_ip(stUploadServerInfo.cUploadServerIp, stUploadServerInfo.cUploadServerDomain, 2);
					pPosStart = pPosEnd+1;
					stUploadServerInfo.iUploadServerPort = atoi(pPosStart);
//					printf("domain : [%s]\nip : [%s]\nport : [%d]\n", stUploadServerInfo.cUploadServerDomain, stUploadServerInfo.cUploadServerIp, stUploadServerInfo.iUploadServerPort);
				}
				cJSON *pInterval = cJSON_GetObjectItem(pArryRoot, "interval"); //上报时间间隔
				if (pInterval != NULL)
				{
					if (pInterval->valueint <= 10)
					{
						stUploadServerInfo.iUploadInterval = 300;//若获取的数据有误，默认5分钟上报一次
					}
					else
					{
						stUploadServerInfo.iUploadInterval = pInterval->valueint;
					}
				}
				cJSON *pAccessToken = cJSON_GetObjectItem(pArryRoot, "accessToken"); //是否允许上报
				if (pAccessToken != NULL)
				{
					strncpy(stUploadServerInfo.cAccessToken, pAccessToken->valuestring, sizeof(stUploadServerInfo.cAccessToken));
				}
				cJSON *pIf = cJSON_GetObjectItem(pArryRoot, "if"); //是否允许上报
				if (pIf != NULL)
				{
					stUploadServerInfo.iIfUpload = pIf->valueint;
					printf("stUploadServerInfo.iIfUpload=%d\n", stUploadServerInfo.iIfUpload);
				}
			}

			printf("stUploadServerInfo.iUploadInterval=%d\n", stUploadServerInfo.iUploadInterval);
//			printf("pTimerEventBasexxxxxxxxxxx=%p\n", pTimerEventBase);
			tv.tv_sec = stUploadServerInfo.iUploadInterval;
			if (iFlag)
			{
				tv.tv_sec = 10;
				event_assign(&evTimer, pTimerEventBase, -1, EV_PERSIST, upload_box_info_timer, (HB_VOID*)pTimerEventBase);
				iFlag = 0;
			}
			event_add(&evTimer, &tv);
		}
		else
		{
			cJSON *pMsg = cJSON_GetObjectItem(pRoot, "msg");
			TRACE_ERR("recv msg error : [%s]\n", pMsg->valuestring);
//			tv.tv_sec = stUploadServerInfo.iUploadInterval;
//			event_assign(&evTimer, pTimerEventBase, -1, EV_PERSIST, upload_box_info_timer, (HB_VOID*)pTimerEventBase);
//			event_add(&evTimer, &tv);
		}
	}

	bufferevent_free(pConnectUploadServerBev);
}





static HB_VOID connect_to_upload_server_event_cb(struct bufferevent *pConnectUploadServerBev, HB_S16 what, HB_VOID *arg)
{
//	struct event_base *pTimerEventBase = (struct event_base *)arg;
	HB_CHAR cSendbuf[4096] = {0};

	if (what & BEV_EVENT_CONNECTED)//盒子主动连接rtsp服务器成功
	{
		printf("Connect alarm server success!\n");
		HB_CHAR cSign[64] = {0};
		HB_CHAR cBody[512] = {0};
		HB_CHAR cUrlBase[512] = {0};
		HB_CHAR cUrl[4096] = {0};
		struct timeval tv;
		//由于DVR的NTP校时时会把北京时间校为UTC时间
		gettimeofday(&tv,NULL);
		HB_U64 lluRegTime = (HB_U64)tv.tv_sec*1000 + tv.tv_usec/1000;//取毫秒值

		snprintf(cUrlBase, sizeof(cUrlBase), "/%s/4QAEAAEBAB4/BoxRegToken/?app_id=%s", UPLOAD_UNION_ID, UPLOAD_APPID);
		//连接服务器获取上报时间间隔
		snprintf(cBody, sizeof(cBody), \
			"{\"root\":{\"access_token\":\"\",\"stamp\":\"%llu\",\"datas\":{\"sn\":\"%s\",\"type\":\"%s\",\"version\":\"%s\",\"regTime\":\"%llu\"}}}", \
			lluRegTime, stBoxInfo.cBoxSn, stBoxInfo.cBoxType, stBoxInfo.cVersion, lluRegTime);
		make_sign(cSign, cBody);
	    //拼接发送字符串
	    snprintf(cUrl, sizeof(cUrl), "%s&sign=%s", cUrlBase, cSign);
	    snprintf(cSendbuf, sizeof(cSendbuf), "POST %s HTTP/1.1\r\nHost:%s:%d\r\ncontent-type: text/plain; charset=utf-8\r\nContent-Length: %d\r\nConnection:keep-alive\r\n\r\n%s",
	    			cUrl, stUploadServerInfo.cUploadServerIp, stUploadServerInfo.iUploadServerPort, strlen(cBody), cBody);

	    printf("Send Send[%s]\n", cSendbuf);

		bufferevent_write(pConnectUploadServerBev, cSendbuf, strlen(cSendbuf));

	    struct timeval tv_read = {10,0};
	    bufferevent_set_timeouts(pConnectUploadServerBev, &tv_read, NULL);
		bufferevent_setcb(pConnectUploadServerBev, deal_cmd, NULL, error_cb, arg);
		bufferevent_enable (pConnectUploadServerBev, EV_READ);

//		event_assign(&evTimer, pTimerEventBase, -1, EV_PERSIST, upload_box_info_timer, arg);
	}
	else  //盒子connect rtsp服务器失败
	{
		printf("connect to upload server failed!\n");
//		struct timeval tv = {0, 0};
//		struct event_base *pTimerEventBase = (struct event_base *)arg;
//		stUploadServerInfo.iUploadInterval = DEFAULT_UPLOAD_INTERVAL;//若获取的数据有误，默认5分钟上报一次
//		printf("stUploadServerInfo.iUploadInterval=%d\n", stUploadServerInfo.iUploadInterval);
//		tv.tv_sec = stUploadServerInfo.iUploadInterval;
//		event_assign(&evTimer, pTimerEventBase, -1, EV_PERSIST, upload_box_info_timer, (HB_VOID*)&evTimer);
//		event_add(&evTimer, &tv);
		bufferevent_free(pConnectUploadServerBev);
	}
}



HB_VOID *thread_hb_box_info_upload(HB_VOID *arg)
{
	pthread_detach(pthread_self());
	stUploadServerInfo.iThreadStartFlag = 1;
	struct event_base *pTimerEventBase;
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

	from_domain_to_ip(stUploadServerInfo.cUploadServerIp, HB_ALARM_SERVER_IP, 2);
//	strcpy(stUploadServerInfo.cUploadServerIp, "192.168.8.12");
	stUploadServerInfo.iUploadServerPort = HB_ALARM_SERVER_PORT;
//	printf("pTimerEventBase==========%p\n", pTimerEventBase);
	while (1)
	{
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
		sleep(5);
	}

	event_base_free(pTimerEventBase);
	printf("timer thread exit~!\n");
	stUploadServerInfo.iThreadStartFlag = 0;

	pthread_mutex_lock(&lockMutexLock);
//	pthread_cancel(threadGetBoxInfo);
//	pthread_join(threadGetBoxInfo, NULL);
	pthread_mutex_unlock(&lockMutexLock);
	pthread_exit(NULL);
}
