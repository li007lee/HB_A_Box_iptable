/*
 * getma2c.c
 *
 *  Created on: 2014年12月12日
 *      Author: root
 */
#include "my_include.h"
#include "net_api.h"
#include "get_set_config.h"

extern HB_S32 flag_wan; //用于标记是否启用广域网 1启用 0未启用

//用于获取是否开启了广域网
static HB_S32 GetWanConnectionStatusCb( HB_VOID * para, HB_S32 n_column, HB_CHAR ** column_value, HB_CHAR ** column_name )
{
	HB_CHAR *pWanConnection = (HB_CHAR *)para;

	strncpy(pWanConnection, column_value[0], strlen(column_value[0]));

	return 0;
}


//用于获取是否开启了广域网
//返回0 关闭， 返回1 开启
HB_S32 get_wan_connection_status(sqlite3 *db)
{
	HB_CHAR *errmsg = NULL;
	HB_S32 ret = 0;
	HB_CHAR *pSql = "select wan_connection from system_web_data";
	HB_CHAR cWanConnection[8] = {0};

	ret = sqlite3_exec(db, pSql, GetWanConnectionStatusCb, (HB_VOID*)cWanConnection, &errmsg);
	if (ret != SQLITE_OK) {
		printf("***************%s:%d***************\nsqlite3_exec get wan_connection error[%d]:%s\n", __FILE__, __LINE__, ret, errmsg);
		sqlite3_free(errmsg);
		return HB_FAILURE;
	}
	sqlite3_free(errmsg);

	flag_wan = atoi(cWanConnection);

	return HB_SUCCESS;
}

HB_S32 get_ps_status(HB_CHAR *cmd)
{
	FILE *cmd_fp = NULL;
	HB_CHAR cmd_buf[256] = {0};
	HB_S32 status = 0; //1 进程存在，0不存在
	if((cmd_fp = popen(cmd,"r")) != NULL)
	{
		if( (fgets(cmd_buf, sizeof(cmd_buf), cmd_fp)) != NULL )
		{
			status = atoi(cmd_buf);
			if(status > 0)
			{
				pclose(cmd_fp);
				return 1;
			}
		}
		pclose(cmd_fp);
		cmd_fp = NULL;
	}
	return -1;
}


//测试天联是否正常，正常返回0,不正常返回-1
HB_S32 test_gnLan_alive()
{
	HB_S32 rand1 = -1, rand2 = -1;
	HB_S32 rand_times = 0;
	FILE *p_fd = NULL;
	HB_CHAR out_msg[256] = {0};

	HB_CHAR *arr_Ping[] = {	"ping -I gnLan 10.6.171.2 -c 1 -w 5",
							"ping -I gnLan 10.7.8.48 -c 1 -w 5",
							"ping -I gnLan 10.7.9.164 -c 1 -w 5",
							"ping -I gnLan 10.7.9.231 -c 1 -w 5",
							"ping -I gnLan 10.7.10.197  -c 1 -w 5",
							"ping -I gnLan 10.4.120.242 -c 1 -w 5",
							"ping -I gnLan 10.7.9.119 -c 1 -w 5",
							"ping -I gnLan 10.7.9.228 -c 1 -w 5",
							"ping -I gnLan 10.7.10.175 -c 1 -w 5",
							"ping -I gnLan 10.7.10.72 -c 1 -w 5"};

	while(rand_times < 3)
	{
		srand(time(NULL));
		rand1 = rand()%10;

		if (rand1 == rand2)
		{
			continue;
		}
		rand2 = rand1;

		p_fd = popen(arr_Ping[rand1], "r");
		if(NULL == p_fd)
		{
			rand_times++;
			continue;
		}
		else
		{
			while(fgets(out_msg, 256, p_fd) != NULL)
			{
				if(strstr(out_msg, "ttl=") != NULL)//gnLan正常
				{
					printf("\n#############  gnLan alive!\n");
					pclose(p_fd);
					return 0;
				}
				memset(out_msg, 0, sizeof(out_msg));
			}
			rand_times++;
			pclose(p_fd);
		}

	}

	printf("\n#############  gnLan died!\n");
	return -1;
}

//获取gnLan登录信息
//返回 1 表示gnLan登录成功 , 返回 0 表示gnLan登录失败
HB_S32 get_sys_gnLan(HB_VOID)
{
	HB_CHAR tmp[128] = {0};
	FILE *fp;
	//fp = popen("ifconfig eth0 | awk '/Link encap:/ {print $5}'", "r");
	fp = popen("ifconfig gnLan | grep \"Link\"", "r");

	fgets(tmp, sizeof(tmp), fp);
	pclose(fp);
	TRACE_LOG("\n######  mac: %s\n", tmp);
	if (strlen(tmp)>0 && strstr(tmp, "Link encap:"))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

HB_S32 update_network_conf(HB_CHAR *str,const HB_CHAR *file)
{
	// 更新配置文件,应该有备份，下面的操作会将文件内容清除
    FILE *fp;
    fp = fopen(file, "wb");
    if(fp == NULL)
    {
    	return -1;
    }
    fprintf(fp,"%s",str);
    fflush(fp);
    fclose(fp);
    return 0;
}

HB_S32 get_current_time(HB_CHAR *time_str)
{
	if(NULL == time_str)
	{
		return HB_FAILURE;
	}
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep); /*取得当地时间*/
	sprintf(time_str, "%d-%d-%d %d:%d:%d",(1900+p->tm_year),( 1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec);
	return HB_SUCCESS;
}
