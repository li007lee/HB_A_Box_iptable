/*
 * my_iptable.c
 *
 *  Created on: 2014年11月28日
 *      Author: root
 */
#include "my_include.h"
#include "md5gen.h"
#include "hf_plant_api.h"
#include "hf_plant_net.h"
#include "get_set_config.h"
#include "debug.h"
#include "sqlite3.h"
#include "ctrl_led.h"
#include "iptable_server.h"


DEV_PLAT_MESSAGE_OBJ  gl_plant_msg;
GLOBLE_MSG_STRUCT gl_msg;


HB_S32 check_write_token_file()
{
	HB_S32 ret = 0;
    HB_S32 fd = 0;
    FILE *fp = NULL;
    HB_CHAR tmp[512];
    HB_CHAR md5_passwd[64];
    static HB_S32 write_flag = 0;
    memset(md5_passwd, 0, sizeof(md5_passwd));
    memset(tmp, 0, sizeof(tmp));

    if(write_flag == 1)
    {
        fp = fopen(TEAMLINK_CONF_FILENAME, "r");
        while(NULL != fgets(tmp, sizeof(tmp), fp))
        {
            if(strstr(tmp, "username="))
            {
                if(strlen(tmp + strlen("username=")) <= 1)
                {
                    write_flag = 0;
                    break;
                }
                TRACE_LOG("username = %s", tmp + strlen("username="));
            }
            else if(strstr(tmp, "passwd="))
            {
                if(strlen(tmp + strlen("groupname=")) <= 1)
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
        if(fp != NULL)
        {
            fclose(fp);
            fp = NULL;
        }
        if(write_flag == 1)
        {
            return 1;
        }
    }
    if((fp = fopen(TEAMLINK_CONF_FILENAME, "w+b")) == NULL)
    {
        ODD_CHECK_PUT_MSG(&fd, DEVICE_MEMFILE_ERR);
        return -1;
    }

    md5_packages_string(md5_passwd, gl_plant_msg.stru_token.tokenpassword);
    //	printf("src:%s\tdesc:%s\n",plant_msg.stru_token.tokenpassword, md5_passwd);
    snprintf(tmp, sizeof(tmp), "UserKey1=<;;8;?>8>>8<=@\nUserKey2=1800\ngroupname=bjhbgkyybydt\nusername=%s\npasswd=\x01%s\n",
             gl_plant_msg.stru_token.tokenname, md5_passwd);
    ret = fwrite(tmp, strlen(tmp), 1, fp);
    fflush(fp);
    if(fp != NULL)
    {
        write_flag = 1;
        fclose(fp);
        fp = NULL;
    }

    return 1;
}



#if 1
void * TimeSync(void *arg)
{
	pthread_detach(pthread_self());
	char *time_add[]={"asia.pool.ntp.org",
                      "ntp.api.bz",
                      "time.twc.weather.com",
                      "swisstime.ethz.ch",
                      "ntp3.fau.de",
                      "time-a.nist.gov",
                      "time-b.nist.gov" ,
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
			pthread_exit(NULL);
			//return 0;
		}
		sleep(1);
		i++;
		if (sizeof(time_add)/4 <= i)
		{
			i=0;
		}
	}

	pthread_exit(NULL);
	return 0;
}
#endif



//获取数据条数回调函数
static HB_S32 CheckMachineCodeRecall( HB_VOID * para, HB_S32 n_column, HB_CHAR ** column_value, HB_CHAR ** column_name )
{
	HB_CHAR *buf = (HB_CHAR *)para;

	strncpy(buf, column_value[0], strlen(column_value[0]));

	return 0;
}

static HB_S32 CheckMachineCode()
{
	sqlite3 *db;
	HB_CHAR *errmsg = NULL;
	HB_CHAR sql[512] = {0};
	HB_S32 ret = 0;
	HB_CHAR retbuf[32] = {0};

	ret = sqlite3_open(BOX_DATA_BASE_NAME, &db);
	if (ret != SQLITE_OK) {
		printf("***************%s:%d***************\nsqlite3_open error[%d]\n", __FILE__, __LINE__, ret);
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	snprintf(sql, sizeof(sql), "select value from system_config_data where key='machine_code'");
	ret = sqlite3_exec(db, sql, CheckMachineCodeRecall, (HB_VOID *)retbuf, &errmsg);
	if (ret != SQLITE_OK) {
		printf("***************%s:%d***************\nget machine_code error[%d]:%s\n", __FILE__, __LINE__, ret, errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return -2;
	}

	printf("sql:[%s]\n", sql);
	sqlite3_free(errmsg);
	sqlite3_close(db);

	return 0;
}


static HB_CHAR * MakeMachineCode(HB_S32 base)
{
	HB_CHAR *p;
	sqlite3 *db;
	HB_CHAR *errmsg = NULL;
	HB_S32 ret = 0;
	HB_U64 machine_code;
	HB_U8 retbuf[33] = {0};
	HB_CHAR sql[512] = {0};
	static HB_S32 retry_num = 0;

	machine_code = get_sys_mac();
	printf("machine_code:%lld\n", machine_code);
Retry:
	memset(retbuf, 0, sizeof(retbuf));
	if (CheckMachineCode(retbuf) <  0)
	{
		retry_num++;
		if (retry_num > 10)
			return NULL;
		sleep(1);
		goto Retry;
	}

	machine_code ^= KEY;

	if(base<2 || base>16)
	{
		return NULL;
	}
	p = &retbuf[sizeof(retbuf)-1];
	*p = '\0';
	do
	{
		*--p = "0123456789abcdef"[machine_code%base];
		machine_code /= base;
	}
	while(machine_code != 0);

	snprintf(gl_msg.machine_code, sizeof(gl_msg.machine_code), "%s", p);

	ret = sqlite3_open(BOX_DATA_BASE_NAME, &db);
	if (ret != SQLITE_OK) {
		printf("***************%s:%d***************\nsqlite3_open error[%d]\n", __FILE__, __LINE__, ret);
		return NULL;
	}

	memset(sql, 0, sizeof(sql));
	snprintf(sql, sizeof(sql), "update system_config_data set value='%s' where key='machine_code'", p);
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK) {
		printf("***************%s:%d***************\nsqlite3_exec set machine_code error[%d]:%s\n", __FILE__, __LINE__, ret, errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return NULL;
	}

	printf("sql:[%s]\n", sql);
	sqlite3_free(errmsg);
	sqlite3_close(db);

	return p;
}

////测试天联是否正常，正常返回0,不正常返回-1
//static int test_gnLan_alive()
//{
//	int i = 0;
//	int gnLan_flag = -1;
//	FILE *p_fd = NULL;
//	HB_CHAR out_msg[256] = {0};
//
//	HB_CHAR *arr_Ping[] = {	"ping -I gnLan 10.6.171.2 -c 1 -w 5",
//							"ping -I gnLan 10.7.8.48 -c 1 -w 5",
//							"ping -I gnLan 10.7.9.164 -c 1 -w 5",
//							"ping -I gnLan 10.7.9.231 -c 1 -w 5",
//							"ping -I gnLan 10.7.10.197  -c 1 -w 5"};
//
//	for (i = 0; i<5 ; i++)
//	{
//		p_fd = popen(arr_Ping[i], "r");
//		if(NULL == p_fd)
//		{
//			continue;
//		}
//		else
//		{
//			while(fgets(out_msg, 256, p_fd) != NULL)
//			{
//				if(strstr(out_msg, "ttl=") != NULL)//gnLan正常
//				{
//					gnLan_flag = 1;
//					printf("\n#############  gnLan alive!\n");
//					pclose(p_fd);
//					return gnLan_flag;
//				}
//				memset(out_msg, 0, sizeof(out_msg));
//			}
//			pclose(p_fd);
//		}
//	}
//
//	printf("\n#############  gnLan died!\n");
//	return gnLan_flag;
//}




HB_S32 main(HB_S32 argc, HB_CHAR* argv[])
{
	HB_S32 ret = 0;
	HB_S32 sockfd = -1;
	HB_S32 flag_wan = 0; //用于标记是否启用广域网 1启用 0未启用
	FILE* cmd_fp;
	HB_CHAR cmd_buf[36] = {0};
	HB_S32 gnlan_count = 0;
	pthread_t thread_time_sync;
    pthread_attr_t attr;
    memset(&gl_plant_msg, 0, sizeof(DEV_PLAT_MESSAGE_OBJ));
    signal(SIGPIPE, SIG_IGN);

#ifndef BIG_BOX_ELEVATOR
	MakeMachineCode(16);
	pthread_create(&thread_time_sync, NULL, TimeSync, NULL);
#endif

	/***************************************************************************/
	/******************************盒子获取流媒体信息******************************/
	/***************************************************************************/
	pthread_t threard_get_stream_server_info;
	pthread_create(&threard_get_stream_server_info, NULL, IptableServer, NULL);

	/***************************************************************************/
	/*****************************盒子获取流媒体信息End*****************************/
	/***************************************************************************/

	/***************************************************************************/
	/******************************盒子获取网络状态信息******************************/
	/***************************************************************************/
	/////////////////memset(stream_msg, 0, sizeof(stream_msg));
#ifdef DOUBLE_NET_PORT
	pthread_t threard_get_net_status;
	ret = pthread_create(&threard_get_net_status, NULL, CtrlLed, NULL);
#endif
	/***************************************************************************/
	/*****************************盒子获取网络状态信息End*****************************/
	/***************************************************************************/

GET_WAN_STATUS:
    while(1)
    {
    	//读数据库
    	flag_wan = GetWanConnectionStatus();
    	if (flag_wan == 1)
    	{
    		//广域网开启
    		break;
    	}
    	sleep(10);
    }

	/***************************************************************************/
	/**********************************盒子注册**********************************/
	/***************************************************************************/
	while(1) //盒子注册部分
	{
#if 1
    	flag_wan = GetWanConnectionStatus();
    	if (flag_wan <= 0)
    	{
    		//广域网未开启
    		goto GET_WAN_STATUS;
    	}
		ret = init_socket2platform(&sockfd, REGIST_SERVER);
        if(ret != HB_SUCCESS)
        {
        	DEBUG_PRINT("\n########  The HB_BOX connect HbServer failed !!!\n");
        	TRACE_ERR("\n########  The HB_BOX connect HbServer failed !!!\n");
        	sleep(10);
    		close(sockfd);
    		sockfd = -1;
        	continue;
        }
        else
        {
			//设备连接服务器成功
        	DEBUG_PRINT("\n#######  The route connect HbServer success !!!\n");
        	TRACE_LOG("\n#######  The route connect HbServer success !!!\n");
        }

        hb_box_register(&sockfd);//设备注册接口
		if(gl_plant_msg.return_regist == 0)
		{
			DEBUG_PRINT("\n########  The HB_BOX register HbServer failed !!!\n");
			TRACE_ERR("\n########  The HB_BOX register HbServer failed !!!\n");
			sleep(10);
			close(sockfd);
			sockfd = -1;
			continue;
		}
		DEBUG_PRINT("\n########  The HB_BOX regist successful !!!\n");
		TRACE_LOG("\n#######  The HB_BOX regist successful !!!\n");
		close(sockfd);
		sockfd = -1;
		break;
	}
	/***************************************************************************/
	/*********************************盒子注册End*********************************/
	/***************************************************************************/

	//任务扫描线程
	pthread_t scanning_pthread_id;
	ret = pthread_attr_init(&attr);
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&scanning_pthread_id, &attr, scanning_task, NULL);
	pthread_attr_destroy(&attr);

	while(1)//设备令牌获取以及gnLan启动部分
	{
    	flag_wan = GetWanConnectionStatus();
    	if (flag_wan <= 0)
    	{
    		//广域网未开启
    		goto GET_WAN_STATUS;
    	}

		ret = get_sys_gnLan();
		if (0 == ret) //gnLan没有登陆
		{
			ret = init_socket2platform(&sockfd, REGIST_SERVER);
	        if(ret != HB_SUCCESS)
	        {
	        	DEBUG_PRINT("\n########  (get IP token)The HB_BOX connect HbServer failed !!!\n");
	        	TRACE_ERR("\n########  (get IP token)The HB_BOX connect HbServer failed !!!\n");
	        	sleep(20);
	        	continue;
	        }
			hb_box_get_token(&sockfd);//设备令牌获取
			if(gl_plant_msg.return_token != 1)
			{
				//sleep_appoint_time(connect_faile_times++);
				DEBUG_PRINT("\n########  The HB_BOX get token failed !!!\n");
				TRACE_ERR("\n########  The HB_BOX get token failed !!!\n");
				sleep(20);
				continue;
			}
			close(sockfd);
			sockfd = -1;

	        if(check_write_token_file() > 0)
	        {
	        	memset(cmd_buf, 0, sizeof(cmd_buf));
	        	gnlan_count = 0;
	        	if((cmd_fp = popen("ps | grep gnLan | grep -v grep | wc -l","r")) != NULL)
	        	{
	        		if( (fgets(cmd_buf, sizeof(cmd_buf), cmd_fp)) != NULL )
	        		{
	        			pclose(cmd_fp);
	        			cmd_fp = NULL;
	        			gnlan_count = atoi(cmd_buf);
						if(gnlan_count > 0)
						{
							system("killall gnLan");
							sleep(1);
						}
	        		}
	        		pclose(cmd_fp);
	        		cmd_fp = NULL;
#ifdef SMALL_BOX
	        		system(GNLAN_START_CMD);
#endif
#ifdef BIG_BOX
	        		system(BIN_PATH GNLAN_START_CMD);
#endif
	        	}
	        }
	        else
	        {
	        	DEBUG_PRINT("\n########  gnLan check token file failed !!!\n");
	        }
		}
		else
		{
			if (test_gnLan_alive() < 0)
			{
				system("killall -9 gnLan");
				sleep(1);
				printf("resart gnLan!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				continue;
			}
			DEBUG_PRINT("\n########  gnLan start success !!!\n");
		}
#endif
        int sleep_times = 0;//计数，用于计算总共睡眠时间
        while (1)
        {
        	sleep(10);
        	sleep_times ++;
        	if (sleep_times >= 60)
        	{
        		//sleep(600);//10分钟检测一次
        		break;
        	}

        	flag_wan = GetWanConnectionStatus();
        	if (flag_wan <= 0)
        	{
        		//广域网未开启
        		goto GET_WAN_STATUS;
        	}
        	else
        	{
	        	memset(cmd_buf, 0, sizeof(cmd_buf));
	        	gnlan_count = 0;
        		//如果开启广域网，则需要测试天联是否登陆，若不通需要重新获取令牌，防止用户在系统设置页面频繁点击开启和关闭广域网按钮
	        	if((cmd_fp = popen("ps | grep gnLan | grep -v grep | wc -l","r")) != NULL)
	        	{
	        		if( (fgets(cmd_buf, sizeof(cmd_buf), cmd_fp)) != NULL )
	        		{
						pclose(cmd_fp);
						cmd_fp = NULL;
						gnlan_count = atoi(cmd_buf);
						if(gnlan_count < 1)
						{
							//天联未启动
							break;
						}
	        		}
	        		else
	        		{
						pclose(cmd_fp);
						cmd_fp = NULL;
	        		}
	        	}
        	}
        }
	}

    return HB_SUCCESS;
}

