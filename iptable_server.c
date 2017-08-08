/*
 * iptable_server.c
 *
 *  Created on: 2017年8月4日
 *      Author: root
 */

#include "my_include.h"
#include "sqlite3.h"
#include "debug.h"
#include "hf_plant_net.h"
#include "get_set_config.h"

#include "iptable_server.h"

STREAM_INFO_STRUCT	stream_msg = {0};
extern DEV_PLAT_MESSAGE_OBJ  gl_plant_msg;

static HB_VOID GetStreamInfo()
{
//	pthread_detach(pthread_self());
	sqlite3 *db;
	HB_CHAR *errmsg = NULL;
	HB_CHAR sql[512] = {0};
	HB_S32 ret, i;
	HB_S32 sock_stream_fd = -1;
	while(1) //盒子获取流媒体地址及端口
	{
		ret = init_socket2platform(&sock_stream_fd, REGIST_SERVER);
		if(ret != HB_SUCCESS)
		{
			DEBUG_PRINT("\n########  The HB_BOX connect HbServer failed !!!\n");
			TRACE_ERR("\n########  The HB_BOX connect HbServer failed !!!\n");
			sleep(10);
			close(sock_stream_fd);
			sock_stream_fd = -1;
			continue;
		}
		else
		{
			//设备连接服务器成功
			DEBUG_PRINT("\n#######  The route connect HbServer success !!!\n");
			TRACE_LOG("\n#######  The route connect HbServer success !!!\n");
		}

		hb_box_get_streamserver_info(&sock_stream_fd);//设备获取流媒体信息接口
		if(gl_plant_msg.return_stream == 0)
		{
			DEBUG_PRINT("\n########  The HB_BOX get stream server ip from HbServer failed !!!\n");
			TRACE_ERR("\n########  The HB_BOX get stream server ip from HbServer failed !!!\n");
			sleep(10);
			close(sock_stream_fd);
			sock_stream_fd = -1;
			continue;
		}
		DEBUG_PRINT("\n########  The HB_BOX get stream server ip successful !!!\n");
		TRACE_LOG("\n#######  The HB_BOX get stream server ip successful !!!\n");
		close(sock_stream_fd);
		sock_stream_fd = -1;

RE_EXEC_SQL:
		sleep(1); //等待页面读取数据库完毕
		//ip获取成功写入数据库
		ret = sqlite3_open(BOX_DATA_BASE_NAME, &db);
		if (ret != SQLITE_OK) {
			printf("***************%s:%d***************\nsqlite3_open error[%d]\n", __FILE__, __LINE__, ret);
			return;
		}

		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "Delete from stream_server_list_data");
		ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
		if (ret != SQLITE_OK) {
			printf("***************%s:%d***************\nsqlite3_exec error[%d]:%s\n", __FILE__, __LINE__, ret, errmsg);
			sqlite3_free(errmsg);
			sqlite3_close(db);
			sleep(1);
			goto RE_EXEC_SQL;
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

				sleep(1);
				goto RE_EXEC_SQL;
			}
		}

		sqlite3_free(errmsg);
		sqlite3_close(db);
		printf("########################get stream info succeed!\n");
		break;
//		sleep(60*60*2);//每2小时获取一次流媒体ip
	}

//	pthread_exit(NULL);
}

//启动一个服务，在每次登陆页面时会收到从loginout.cgi发来的连接，此时进行获取验证服务器ip的操作。
HB_VOID *IptableServer(HB_VOID *args)
{
	pthread_detach(pthread_self());
	HB_S32 sockServerFd = -1, sockClientFd = -1;
	int ret;
	int opt = 1;
    socklen_t  clilen;
    struct sockaddr_in cliaddr, servaddr;
    char recvBuf[128] = {0};

    sockServerFd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(IPTABLE_SERVER_PORT);

	setsockopt(sockServerFd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	bind(sockServerFd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in));
	listen(sockServerFd, 100);

	clilen = sizeof(struct sockaddr_in);
	while(1)
	{
//		printf("start listen!\n");
		sockClientFd = accept(sockServerFd, (struct sockaddr*)&cliaddr, &clilen);
		if (sockClientFd < 0)
		{
			continue; /* back to for() */
		}

		ret = sock_recv(&sockClientFd, recvBuf, 128, 2);
		if (ret <= 0)
		{
			printf("sock_recv : [%d]\n", ret);
			close(sockClientFd);
			sockClientFd = -1;
			continue;
		}

		if (strncmp(recvBuf, "GetStreamInfo", strlen("GetStreamInfo")) == 0)
		{
			int flag_wan = -1;
	    	//读数据库
	    	flag_wan = GetWanConnectionStatus();
	    	if (flag_wan == 1)
	    	{
	    		//广域网开启
	    		GetStreamInfo();
	    	}
		}
		close(sockClientFd);
		sockClientFd = -1;
	}
	pthread_exit(NULL);
}
