/*
 * iptable_server.c
 *
 *  Created on: 2017年8月4日
 *      Author: root
 */

#include "my_include.h"
#include "sqlite3.h"
#include "get_set_config.h"
#include "hf_plant_api.h"
#include "net_api.h"
#include "box_info_upload.h"

#include "iptable_server.h"

#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "event2/listener.h"
#include "event2/thread.h"

SERVER_INFO_STRUCT	stream_msg = {0};
SERVER_INFO_STRUCT	heartbeat_server_msg = {0};
extern GLOBLE_MSG_STRUCT glMsg;
struct event_base *pEventBase;
extern DEV_PLAT_MESSAGE_OBJ  gl_plant_msg;
extern HB_S32 flag_wan; //用于标记是否启用广域网 1启用 0未启用


/*
 *	Function: 处理与客户端信令交互时产生的异常和错误(第一次信令交互)
 *
 *	@param bev: 异常产生的事件句柄
 *	@param events: 异常事件类型
 *  @parmm args	: 实际为LIBEVENT_ARGS_HANDLE类型的参数结构体
 *
 *	Retrun: 无
 */
static HB_VOID deal_client_cmd_error_cb1(struct bufferevent *bev, short events, void *args)
{
	HB_S32 err = EVUTIL_SOCKET_ERROR();

	if (events & BEV_EVENT_EOF)//对端关闭
	{
		TRACE_ERR("######## deal_client_cmd_error_cb1 BEV_EVENT_EOF(%d) : %s !", err, evutil_socket_error_to_string(err));
	}
	else if (events & BEV_EVENT_ERROR)//错误事件
	{
		TRACE_ERR("######## deal_client_cmd_error_cb1 BEV_EVENT_ERROR(%d) : %s !", err, evutil_socket_error_to_string(err));
	}
	else if (events & BEV_EVENT_TIMEOUT)//超时事件
	{
		TRACE_ERR("######## deal_client_cmd_error_cb1 BEV_EVENT_TIMEOUT(%d) : %s !", err, evutil_socket_error_to_string(err));
	}

	if (bev != NULL)
	{
		bufferevent_free(bev);
		bev = NULL;
	}
}

HB_S32 GetStreamInfo()
{
	sqlite3 *db;
	HB_CHAR *errmsg = NULL;
	HB_CHAR sql[512] = {0};
	HB_S32 ret, i;
	HB_S32 iSockFd = -1;

	//盒子获取流媒体地址及端口
	if (create_socket_connect_domain(&iSockFd, PT_ADDR_IP, PT_PORT, 5)!=0)
	{
		//连接服务器失败，5s后重试
		TRACE_ERR("\n########  The HB_BOX connect HbServer failed !!!\n");
		close_sockfd(&iSockFd);
		return HB_FAILURE;
	}

	hb_box_opt_cmd_exec(&iSockFd, GETSTREAMINFO);
	if(gl_plant_msg.return_stream == 0)
	{
		TRACE_ERR("\n########  The HB_BOX get stream server ip from HbServer failed !!!\n");
		close_sockfd(&iSockFd);
		return HB_FAILURE;
	}
	TRACE_LOG("\n#######  The HB_BOX get stream server ip successful !!!\n");
	close_sockfd(&iSockFd);

	//ip获取成功写入数据库
	ret = sqlite3_open(BOX_DATA_BASE_NAME, &db);
	if (ret != SQLITE_OK) {
		printf("***************%s:%d***************\nsqlite3_open error[%d]\n", __FILE__, __LINE__, ret);
		return HB_FAILURE;
	}

	memset(sql, 0, sizeof(sql));
	snprintf(sql, sizeof(sql), "Delete from stream_server_list_data");
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK) {
		printf("***************%s:%d***************\nsqlite3_exec error[%d]:%s\n", __FILE__, __LINE__, ret, errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return HB_FAILURE;
	}

	for (i=0;i<stream_msg.num;i++) {
		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "insert into stream_server_list_data (stream_server_ip,stream_server_port) values ('%s','%s')", \
						stream_msg.ip[i], stream_msg.port[i]);
		ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
		if (ret != SQLITE_OK) {
			printf("***************%s:%d***************\nsqlite3_exec error[%d]:%s\n", __FILE__, __LINE__, ret, errmsg);
			sqlite3_free(errmsg);
			sqlite3_close(db);
			return HB_FAILURE;
		}
	}

	sqlite3_free(errmsg);
	sqlite3_close(db);
	printf("########################get stream info succeed!\n");

	return HB_SUCCESS;
}



HB_S32 write_xml(HB_CHAR *pHeartbeatIp, HB_S32 iHeartbeatPort, HB_CHAR *pBoxSn)
{
	FILE *fileFp;
	HB_CHAR cFileContent[20480] = {0};
	HB_CHAR cBuff[256] = {0};
	fileFp = fopen(EASYCAMERA_XML, "r+");
	if (fileFp == NULL)
	{
		TRACE_ERR("open %s failed!\n", fileFp);
		return -1;
	}

	while(fgets(cBuff, sizeof(cBuff), fileFp) != NULL)
	{
		if(strstr(cBuff, "easycms_ip") != NULL)
		{
			printf("find str:[%s]\n", cBuff);
			memset(cBuff, 0, sizeof(cBuff));
			snprintf(cBuff, sizeof(cBuff), "\t\t<PREF NAME=\"easycms_ip\" >%s</PREF>\n", pHeartbeatIp);
		}
		else if(strstr(cBuff, "easycms_port") != NULL)
		{
			printf("find str:[%s]\n", cBuff);
			memset(cBuff, 0, sizeof(cBuff));
			snprintf(cBuff, sizeof(cBuff), "\t\t<PREF NAME=\"easycms_port\" TYPE=\"UInt16\" >%d</PREF>\n", iHeartbeatPort);
		}
		else if(strstr(cBuff, "device_serial") != NULL)
		{
			printf("find str:[%s]\n", cBuff);
			memset(cBuff, 0, sizeof(cBuff));
			snprintf(cBuff, sizeof(cBuff), "\t\t<PREF NAME=\"device_serial\" >%s</PREF>\n", pBoxSn);
		}

		strcat(cFileContent, cBuff);
		memset(cBuff, 0, sizeof(cBuff));
	}

	fseeko(fileFp, 0, SEEK_SET);
	fprintf(fileFp, "%s", cFileContent);

	fclose(fileFp);
	fileFp = NULL;

	return 0;
}


HB_S32 GetHeartBeatServerInfo()
{
	HB_S32 iSockFd = -1;
	HB_S32 iRet = -1;

	//盒子获取心跳服务器地址及端口
	if (create_socket_connect_domain(&iSockFd, PT_ADDR_IP, PT_PORT, 5)!=0)
	{
		//连接服务器失败，5s后重试
		TRACE_ERR("\n########  The HB_BOX connect HbServer failed !!!\n");
		close_sockfd(&iSockFd);
		return HB_FAILURE;
	}

	//设备获取心跳服务器信息接口
	hb_box_opt_cmd_exec(&iSockFd, GET_HEARTBEAT_SERVER_INFO);
	if(gl_plant_msg.return_heart_beat == 0)
	{
		TRACE_ERR("\n########  The HB_BOX get heartbeat server ip from HbServer failed !!!\n");
		close_sockfd(&iSockFd);
		return HB_FAILURE;
	}
	printf("heartbeat server ip [%s] port [%s]\n", heartbeat_server_msg.ip[0], heartbeat_server_msg.port[0]);
	TRACE_LOG("\n#######  The HB_BOX get heartbeat server ip successful !!!\n");
	close_sockfd(&iSockFd);

	if ((strcmp(glMsg.cHeartbeatServerIp, heartbeat_server_msg.ip[0])!=0) || (glMsg.iHeartbeatPort != atoi(heartbeat_server_msg.port[0])))
	{
		strncpy(glMsg.cHeartbeatServerIp, heartbeat_server_msg.ip[0], sizeof(glMsg.cHeartbeatServerIp));
		glMsg.iHeartbeatPort = atoi(heartbeat_server_msg.port[0]);
		system(KILL_HEARTBEAT_CLIENT);
		//获取长连接服务器成功
		iRet = write_xml(heartbeat_server_msg.ip[0], atoi(heartbeat_server_msg.port[0]), stBoxInfo.cBoxSn);
		if (iRet < 0)
			return HB_FAILURE;

		system(START_HEARTBEAT_CLIENT);
	}

	return HB_SUCCESS;
}


/*
 *	Function: 读取回调函数,用于处理接从客户端接收到的信令，并根据信令能容做相应处理
 *
 *	@param client_bev: 与客户端信令交互的bufferevent事件。
 *	@param arg: 事件触发时传入的参数。
 *
 *	Retrun: 无
 */
HB_VOID deal_client_cmd(struct bufferevent *pClientBev, void *arg)
{
	HB_CHAR arrc_RecvCmdBuf[1024] = {0};

//	struct evbuffer *src = bufferevent_get_input(pClientBev);//获取输入缓冲区
//	HB_S32 len = evbuffer_get_length(src);//获取输入缓冲区中数据的长度，也就是可以读取的长度。

	bufferevent_read(pClientBev, (HB_VOID*)(arrc_RecvCmdBuf), sizeof(arrc_RecvCmdBuf));
	bufferevent_disable(pClientBev, EV_READ);

	bufferevent_free(pClientBev);
	pClientBev = NULL;

	//解析命令
	if (strstr(arrc_RecvCmdBuf, "GetStreamInfo") != NULL)
	{
		//收到推送视频流信令
		TRACE_LOG("Recv Cmd : [%s]\n", arrc_RecvCmdBuf);
		GetStreamInfo();
		GetHeartBeatServerInfo();
	}
	if (strstr(arrc_RecvCmdBuf, "SetWan") != NULL)
	{
		HB_CHAR *pPos = strstr(arrc_RecvCmdBuf, "Value=");
		flag_wan = atoi(pPos+6);
		if (flag_wan == 0)
		{
			//关闭了广域网
			system("killall -9 gnLan");
		}
		printf("wan flag = %d\n", flag_wan);
	}
}

/*
 *	Function: 接收到客户端连接的回调函数
 *
 *	@param pListener : 监听句柄
 *  @param iAcceptSockfd : 收到连接后分配的文件描述符（与客户端连接的文件描述符）
 *  @param pClientAddr : 客户端地址结构体
 *
 *	Retrun: 无
 */
static HB_VOID accept_client_connect_cb(struct evconnlistener *pListener, evutil_socket_t iAcceptSockfd,
	    struct sockaddr *pClientAddr, int slen, void *arg)
{
	struct timeval tv_read;

#if 1
	//打印对端ip
	struct sockaddr_in *p_PeerAddr = (struct sockaddr_in *)pClientAddr;
	char arrch_PeerIp[16] = {0};
	inet_ntop(AF_INET, &(p_PeerAddr->sin_addr), arrch_PeerIp, sizeof(arrch_PeerIp));
	TRACE_DBG("\nA new Client[%s]:[%d] connect !\n", arrch_PeerIp, ntohs(p_PeerAddr->sin_port));
#endif

    // 为新的连接分配并设置 bufferevent,设置BEV_OPT_CLOSE_ON_FREE宏后，当连接断开时也会通知客户端关闭套接字
    struct bufferevent *accept_sockfd_bev = bufferevent_socket_new(pEventBase, iAcceptSockfd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    //设置低水位，当数据长度大于消息头时才读取数据
//    bufferevent_setwatermark(accept_sockfd_bev, EV_READ, sizeof(BOX_CTRL_CMD_OBJ)+1, 0);
    //设置超时，5秒内未收到对端发来数据则断开连接
    tv_read.tv_sec  = 5;
    tv_read.tv_usec = 0;
	//注意，在盒子连接设备处也设置了超时，此处超时需大于盒子与设备连接时的超时，当前盒子与设备连接超时时间为5s
	bufferevent_set_timeouts(accept_sockfd_bev, &tv_read, NULL);
    bufferevent_setcb(accept_sockfd_bev, deal_client_cmd, NULL, deal_client_cmd_error_cb1, NULL);
    bufferevent_enable(accept_sockfd_bev, EV_READ);

    return;
}

//启动一个服务，在每次登陆页面时会收到从loginout.cgi发来的连接，此时进行获取验证服务器ip的操作。
HB_VOID *IptableServer(HB_VOID *args)
{
	pthread_detach(pthread_self());

	struct evconnlistener *pListener;
	struct sockaddr_in  stListenAddr;

	if(evthread_use_pthreads() != 0)
	{
		TRACE_ERR("########## evthread_use_pthreads() err !");
		pthread_exit(NULL);
	}
	bzero(&stListenAddr, sizeof(stListenAddr));
	stListenAddr.sin_family = AF_INET;
	stListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	stListenAddr.sin_port = htons(IPTABLE_SERVER_PORT);
	pEventBase = event_base_new();
	if (!pEventBase)
	{
		perror("cmd_base event_base_new()");
		pthread_exit(NULL);
	}
	//用于线程安全
	if(evthread_make_base_notifiable(pEventBase) != 0)
	{
		TRACE_ERR("###### evthread_make_base_notifiable() err!");
		pthread_exit(NULL);
	}

	//LEV_OPT_CLOSE_ON_EXEC 用于为套接字设置close-on-exec标志，
	//这个标志符的具体作用在于当开辟其他进程调用exec（）族函数时，在调用exec函数之前为exec族函数释放对应的文件描述符。
	//绑定端口并接收连接
	pListener = evconnlistener_new_bind(pEventBase, accept_client_connect_cb, NULL,
			LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE|LEV_OPT_THREADSAFE,
			1024, (struct sockaddr*)&stListenAddr, sizeof(struct sockaddr_in));
	event_base_dispatch(pEventBase);
	evconnlistener_free(pListener);
	event_base_free(pEventBase);

#if 0

	HB_S32 sockServerFd = -1, sockClientFd = -1;
	HB_S32 ret;
    socklen_t iSockAddrLen;
    struct sockaddr_in stClientAddr;
    HB_CHAR cRecvBuf[128] = {0};

    sockServerFd = setup_listen_socket(IPTABLE_SERVER_PORT);
    iSockAddrLen = sizeof(struct sockaddr_in);
	while(1)
	{
		sockClientFd = accept(sockServerFd, (struct sockaddr*)&stClientAddr, &iSockAddrLen);
		if (sockClientFd < 0)
		{
			perror("accept()");
			continue; /* back to for() */
		}

		ret = recv_data(&sockClientFd, cRecvBuf, 128, 2);
		if (ret <= 0)
		{
			printf("sock_recv : [%d]\n", ret);
			close_sockfd(&sockClientFd);
			continue;
		}

		if (strstr(cRecvBuf, "SetWan")!=NULL)
		{
			HB_CHAR *pPos = strstr(cRecvBuf, "Value=");
			flag_wan = atoi(pPos+6);
			if (flag_wan == 0)
			{
				//关闭了广域网
				system("killall -9 gnLan");
			}
		}

		if (strstr(cRecvBuf, "GetStreamInfo") != NULL)
		{
	    	if (flag_wan == 1)
	    	{
	    		//广域网开启
	    		GetStreamInfo();
	    		sleep(1);
	    		GetHeartBeatServerInfo();
	    	}
		}

		close_sockfd(&sockClientFd);
	}
#endif
	TRACE_ERR("thread IptableServer exit !\n");
	pthread_exit(NULL);
}
