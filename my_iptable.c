/*
 * my_iptable.c
 *
 *  Created on: 2014年11月28日
 *      Author: root
 */
#include "my_include.h"
#include "md5gen.h"
#include "hf_plant_api.h"
#include "get_set_config.h"
#include "sqlite3.h"
#include "ctrl_led.h"
#include "iptable_server.h"
#include "net_api.h"
#include "box_info_upload.h"

DEV_PLAT_MESSAGE_OBJ gl_plant_msg;
GLOBLE_MSG_STRUCT glMsg;
HB_S32 flag_wan = 0; //用于标记是否启用广域网 1启用 0未启用

HB_S32 check_write_token_file()
{
	FILE *fp = NULL;
	HB_CHAR tmp[512];
	HB_CHAR md5_passwd[64];
	static HB_S32 write_flag = 0;
	memset(md5_passwd, 0, sizeof(md5_passwd));
	memset(tmp, 0, sizeof(tmp));

	if (write_flag == 1)
	{
		fp = fopen(TEAMLINK_CONF_FILENAME, "r");
		while (NULL != fgets(tmp, sizeof(tmp), fp))
		{
			if (strstr(tmp, "username="))
			{
				if (strlen(tmp + strlen("username=")) <= 1)
				{
					write_flag = 0;
					break;
				}
				TRACE_LOG("username = %s", tmp + strlen("username="));
			}
			else if (strstr(tmp, "passwd="))
			{
				if (strlen(tmp + strlen("groupname=")) <= 1)
				{
					write_flag = 0;
					break;
				}
				TRACE_LOG("passwd = %s", tmp + strlen("passwd="));
			}
			else
			{
				continue;
			}
		}
		if (fp != NULL)
		{
			fclose(fp);
			fp = NULL;
		}
		if (write_flag == 1)
		{
			return 1;
		}
	}
	if ((fp = fopen(TEAMLINK_CONF_FILENAME, "w+b")) == NULL)
	{
		printf("write gnLan configure failed!\n");
		return -1;
	}

	md5_packages_string(md5_passwd, gl_plant_msg.stru_token.tokenpassword, strlen(gl_plant_msg.stru_token.tokenpassword));
	//	printf("src:%s\tdesc:%s\n",plant_msg.stru_token.tokenpassword, md5_passwd);
	snprintf(tmp, sizeof(tmp), "UserKey1=<;;8;?>8>>8<=@\nUserKey2=1800\ngroupname=bjhbgkyybydt\nusername=%s\npasswd=\x01%s\n",
					gl_plant_msg.stru_token.tokenname, md5_passwd);
	fwrite(tmp, strlen(tmp), 1, fp);
	fflush(fp);
	if (fp != NULL)
	{
		write_flag = 1;
		fclose(fp);
		fp = NULL;
	}

	return 1;
}

void * TimeSync(void *arg)
{
	pthread_detach(pthread_self());
#if 0
	char *time_add[]=
	{	"asia.pool.ntp.org",
		"ntp.api.bz",
		"time.twc.weather.com",
		"swisstime.ethz.ch",
		"ntp3.fau.de",
		"time-a.nist.gov",
		"time-b.nist.gov",
		"s1a.time.edu.cn",
		"s1a.time.edu.cn",
		"s2g.time.edu.cn",
		"time.nist.gov",
		"ntp.fudan.edu.cn",
		"time.windows.com"};
	char uptime[32];
	int i = 0;
	struct timeval now;

	now.tv_usec = 0;
	settimeofday(&now,NULL); //时间初始化

	while (1)
	{
		memset(uptime, '\0', sizeof(uptime));
#ifdef SMALL_BOX
		sprintf(uptime, "ntpdate %s",time_add[i]);
#endif

#ifdef BIG_BOX
		sprintf(uptime, BIN_PATH"ntpdate %s",time_add[i]);
#endif
		system(uptime);

		if(1451577600<time(NULL))  //(1420041600)为北京时间的2016/1/1 00:00:00
		{
			now.tv_sec = time(NULL);
			settimeofday(&now,NULL);
			printf("get_time from \"%s\"***************************cur_time=%ld\n",time_add[i], time(NULL));
			sleep(604800);  //校时成功，每七天校一次时
			continue;
//			pthread_exit(NULL);
			//return 0;
		}
		sleep(5);
		i++;
		if (sizeof(time_add)/4 <= i)
		{
			i=0;
		}
	}
#else
	while (1)
	{
		//	http://alarm.hbydt.cn:8088/OPEN_UNION/4QAEAAEBAB4/AL000?app_id=OPEN_BASE_APP&sign=
		//通过报警服务器进行校时
		HB_S32 iSockFd = -1;
		HB_CHAR *pPos = NULL;
		HB_CHAR pUrlBuf[1024] = { 0 };
		HB_CHAR cMsgBuff[1024] = { 0 };
		//此处死循环，没有注册成功，不能取令牌
		if (create_socket_connect_domain(&iSockFd, HB_ALARM_SERVER_IP, HB_ALARM_SERVER_PORT, 5) != 0)
		{
			//连接服务器失败，5s后重试
			TRACE_ERR("\n########  The HB_BOX connect HbAlarmServer failed !!!\n");
			close_sockfd(&iSockFd);
			sleep(5);
			continue;
		}
		strncpy(pUrlBuf, "OPEN_UNION/4QAEAAEBAB4/AL000/?appid=OPEN_BASE_APP&sign=", sizeof(pUrlBuf));

		sprintf(cMsgBuff,
						"GET /%s HTTP/1.1\r\nHost:%s:%d\r\nConnection:keep-alive\r\nAccept:text/html,application/json;q=0.9,image/webp,*/*;q=0.8\r\nUpgrade-Insecure-Requests:1\r\nUser-Agent:Mozilla/5.0\r\nAccept-Encoding:gzip,deflate,sdch\r\nAccept-Language:zh-CN,zh;q=0.8\r\n\r\n",
						pUrlBuf, HB_ALARM_SERVER_IP, HB_ALARM_SERVER_PORT);

		if (send_data(&iSockFd, cMsgBuff, strlen(cMsgBuff), 10) < 0)
		{
			TRACE_ERR("\n#######send failed\n");
			return NULL;
		}
		TRACE_DBG("\n============Send Send Send Send============ \n[%s]\n", cMsgBuff);
		memset(cMsgBuff, 0, sizeof(cMsgBuff));
		if (recv_data(&iSockFd, cMsgBuff, sizeof(cMsgBuff), 10) < 0)
		{
			TRACE_ERR("\n#######recv failed\n");
			return NULL;
		}
		TRACE_DBG("\n============Recv Recv Recv Recv============ \n[%s]\n", cMsgBuff);
		close_sockfd(&iSockFd);

		pPos = strstr(cMsgBuff, "stamp");
		if (pPos != NULL)
		{
			HB_U64 lluCurTime = ((HB_U64) atoll(pPos + 8)) / 1000;
			printf("cur time : %llu\n", lluCurTime);
			struct timeval tv_now;
			tv_now.tv_sec = lluCurTime;
			tv_now.tv_usec = 0;
			printf("settimeofday:%d\n", settimeofday(&tv_now, NULL)); //时间初始化
			sleep(604800); //校时成功，每七天校一次时
			continue;
		}

		sleep(5);
	}
#endif
	pthread_exit(NULL);
	return 0;
}

static HB_S32 make_machine_code(sqlite3 *db, HB_S32 base)
{
	HB_CHAR *p;
	HB_CHAR *errmsg = NULL;
	HB_S32 ret = 0;
	HB_U64 machine_code;
	HB_U8 retbuf[33] = { 0 };
	HB_CHAR sql[512] = { 0 };

	machine_code = get_sys_mac();
	printf("machine_code:%llu\n", machine_code);
	machine_code ^= KEY;

//	if(base<2 || base>16)
//	{
//		return HB_FAILURE;
//	}
	p = &retbuf[sizeof(retbuf) - 1];
	*p = '\0';
	do
	{
		*--p = "0123456789abcdef"[machine_code % base];
		machine_code /= base;
	} while (machine_code != 0);

	snprintf(glMsg.machine_code, sizeof(glMsg.machine_code), "%s", p);

	memset(sql, 0, sizeof(sql));
	snprintf(sql, sizeof(sql), "update system_web_data set machine_code='%s'", p);
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf("***************%s:%d***************\nsqlite3_exec [%s] error[%d]:%s\n", __FILE__, __LINE__, sql, ret, errmsg);
		sqlite3_free(errmsg);
		return HB_FAILURE;
	}

	return HB_SUCCESS;
}

//设备开机初始化操作
static HB_S32 start_box_init()
{
	sqlite3 *db;
	HB_S32 ret = 0;
	FILE *pFileFp = NULL;
	HB_CHAR *pPos = NULL;

	memset(&stBoxInfo, 0, sizeof(stBoxInfo));
	memset(&stUploadServerInfo, 0, sizeof(stUploadServerInfo));

	//盒子校时操作
	pthread_t thread_time_sync;
	pthread_create(&thread_time_sync, NULL, TimeSync, NULL);

	ret = sqlite3_open(BOX_DATA_BASE_NAME, &db);
	if (ret != SQLITE_OK)
	{
		printf("***************%s:%d***************\nsqlite3_open error[%d]\n", __FILE__, __LINE__, ret);
		return HB_FAILURE;
	}

	if (make_machine_code(db, 16) == HB_FAILURE) //生成机器码
	{
		sqlite3_close(db);
		return HB_FAILURE;
	}

	//设置在数据库中配置的辅助ip和路由
	if (set_network(db) == HB_FAILURE)
	{
		sqlite3_close(db);
		return HB_FAILURE;
	}
	//获取盒子是否开起了广域网
	if (get_wan_connection_status(db) == HB_FAILURE)
	{
		sqlite3_close(db);
		return HB_FAILURE;
	}
	sqlite3_close(db);

	get_sys_sn(stBoxInfo.cBoxSn, sizeof(stBoxInfo.cBoxSn));
	printf("cSn:[%s]\n", stBoxInfo.cBoxSn);

	pFileFp = fopen(BOX_VERSION_FILE, "r");
	if (NULL == pFileFp)
	{
		return HB_FAILURE;
	}
	else
	{
		fgets(stBoxInfo.cBoxType, 32, pFileFp);
		if ((pPos = strchr(stBoxInfo.cBoxType, '\r')) != NULL)
		{
			*pPos = '\0';
		}
		else if ((pPos = strchr(stBoxInfo.cBoxType, '\n')) != NULL)
		{
			*pPos = '\0';
		}
		fgets(stBoxInfo.cVersion, 16, pFileFp);
		if ((pPos = strchr(stBoxInfo.cVersion, '\r')) != NULL)
		{
			*pPos = '\0';
		}
		else if ((pPos = strchr(stBoxInfo.cVersion, '\n')) != NULL)
		{
			*pPos = '\0';
		}
		fclose(pFileFp);
	}
	pthread_t threard_iptable_server;
	pthread_create(&threard_iptable_server, NULL, IptableServer, NULL);

#ifdef DOUBLE_NET_PORT
	//双网口小盒子点灯操作
	pthread_t threard_get_net_status;
	pthread_create(&threard_get_net_status, NULL, CtrlLed, NULL);
#endif

	pthread_t threard_get_stream_server_info;
	pthread_create(&threard_get_stream_server_info, NULL, GetStreamServer, NULL);

	pthread_t threard_get_heartbeat_server_info;
	pthread_create(&threard_get_heartbeat_server_info, NULL, GetHearBeatServer, NULL);


	pthread_t thTimer = -1;
	pthread_create(&thTimer, NULL, thread_hb_box_info_upload, NULL);

	sleep(1);
	return HB_SUCCESS;
}

HB_S32 main(HB_S32 argc, HB_CHAR* argv[])
{
	HB_S32 iSockFd = -1;
	HB_S32 iGnlanExistFlag = 0; //用于标记天联是否已经启动
	HB_CHAR cCmdBuf[64] = { 0 };

	memset(&gl_plant_msg, 0, sizeof(DEV_PLAT_MESSAGE_OBJ));
	signal(SIGPIPE, SIG_IGN);

	system("killall -9 gnLan");
	system(KILL_HEARTBEAT_CLIENT);
	system(KILL_LED_CTRL_SH);

	if (HB_FAILURE == start_box_init())
	{
		return HB_FAILURE;
	}

GET_WAN_STATUS:

	while (!flag_wan)
	{
		sleep(10);
	}

	//盒子注册部分
	for(;;)
	{
		if (flag_wan == 0)
		{
			//广域网为开启或被关闭
			goto GET_WAN_STATUS;
		}
		//此处死循环，没有注册成功，不能取令牌
		if (create_socket_connect_domain(&iSockFd, PT_ADDR_IP, PT_PORT, 5) != 0)
		{
			//连接服务器失败，5s后重试
			TRACE_ERR("\n########  The HB_BOX connect HbServer failed !!!\n");
			close_sockfd(&iSockFd);
			sleep(5);
			continue;
		}
		else
		{
			TRACE_LOG("\n#######  The route connect HbServer success !!!\n");
		}

		//设备注册接口
		hb_box_opt_cmd_exec(&iSockFd, REGISTER);
		if (gl_plant_msg.return_regist == 0)
		{
			TRACE_ERR("\n########  The HB_BOX register HbServer failed !!!\n");
			close_sockfd(&iSockFd);
			sleep(5);
			continue;
		}

		close_sockfd(&iSockFd);
		TRACE_LOG("\n#######  The HB_BOX regist successful !!!\n");
		break;
	}

	//任务扫描线程
//	pthread_t scanning_pthread_id;
//	pthread_create(&scanning_pthread_id, NULL, scanning_task, NULL);

	//设备注册成功,进行取令牌操作
	for (;;)
	{
		if (flag_wan == 0)
		{
			//广域网为开启或被关闭
			goto GET_WAN_STATUS;
		}

		if (get_sys_gnLan() == 0)
		{
			//天联未登录或登陆失败
			if (create_socket_connect_domain(&iSockFd, PT_ADDR_IP, PT_PORT, 5) != 0)
			{
				//连接服务器失败，5s后重试
				TRACE_ERR("\n########  The HB_BOX connect HbServer failed !!!\n");
				close_sockfd(&iSockFd);
				sleep(5);
				continue;
			}

			hb_box_opt_cmd_exec(&iSockFd, GET_TOKEN); //设备令牌获取
			if (gl_plant_msg.return_token != 1)
			{
				TRACE_ERR("\n########  The HB_BOX get token failed !!!\n");
				sleep(20);
				continue;
			}
			TRACE_LOG("\n#######  The HB_BOX get token success !!!\n");
			close_sockfd(&iSockFd);

			//将token信息写入gnLan配置文件是否已经存在
			if (check_write_token_file() > 0)
			{
				FILE *pCmdFp = NULL;
				memset(cCmdBuf, 0, sizeof(cCmdBuf));
				iGnlanExistFlag = 0;
				if ((pCmdFp = popen("ps | grep gnLan | grep -v grep | wc -l", "r")) != NULL)
				{
					if ((fgets(cCmdBuf, sizeof(cCmdBuf), pCmdFp)) != NULL)
					{
						iGnlanExistFlag = atoi(cCmdBuf);
						if (iGnlanExistFlag > 0)
						{
							//gnLan已经存在则杀死
							system("killall -9 gnLan");
							sleep(1);
						}
					}
					pclose(pCmdFp);
					pCmdFp = NULL;
#ifdef SMALL_BOX
					system(GNLAN_START_CMD);
#endif
#ifdef BIG_BOX
					system(BIN_PATH GNLAN_START_CMD);
#endif
					//只要重新启动了天联就1分钟后探测一下天联是否启动成功了
					sleep(60);
					continue;
				}
			}
			else
			{
				TRACE_ERR("\n########  gnLan write token file failed !!!\n");
			}
		}
		else
		{
			//如果天联已经启动，则对天联的连通性进行测试
			if (test_gnLan_alive() != 0)
			{
				//天联不通
				system("killall -9 gnLan");
				sleep(1);
				TRACE_ERR("resart gnLan!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				continue;
			}
			TRACE_LOG("\n########  gnLan start success !!!\n");
			sleep(60);
		}
	}

	return HB_SUCCESS;
}

