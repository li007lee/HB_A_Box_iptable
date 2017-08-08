/*
 * getma2c.c
 *
 *  Created on: 2014年12月12日
 *      Author: root
 */
#include "my_include.h"
#include "sqlite3.h"
#include "get_set_config.h"

HB_S32 get_ip_dev(HB_CHAR *eth, HB_CHAR *ipaddr)
{
	struct ifreq req;
	HB_S32 sock;
	HB_CHAR *temp_ip = NULL;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        return -1;
    }
	strncpy(req.ifr_name, eth, IFNAMSIZ);
	if ( ioctl(sock, SIOCGIFADDR, &req) < 0 )
    {
        fprintf(stderr,"ioctl error: %s\n", strerror (errno));
        return -1;
    }
   temp_ip = inet_ntoa(*(struct in_addr *) &((struct sockaddr_in *) &req.ifr_addr)->sin_addr);
   //temp_ip = inet_ntoa(((struct sockaddr_in *) &req.ifr_addr)->sin_addr);
   strcpy(ipaddr,temp_ip);
   close(sock);
   return HB_SUCCESS;

}


//获取网卡序列号
//mac_sn 网卡序列号, dev 网卡名
static HB_S32 get_mac_dev(HB_CHAR *mac_sn, HB_CHAR *dev)
{
    struct ifreq tmp;
    HB_S32 sock_mac;
   // HB_CHAR *tmpflag;
    //HB_CHAR mac_addr[30];
    sock_mac = socket(AF_INET, SOCK_STREAM, 0);
    if( sock_mac == -1)
    {
        perror("### create socket fail\n");
        return -1;
    }
    memset(&tmp,0,sizeof(tmp));
    strncpy(tmp.ifr_name, dev, sizeof(tmp.ifr_name)-1);
    if( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 )
    {
    	close(sock_mac);
    	TRACE_ERR("### mac ioctl error\n");
        return -1;
    }

    close(sock_mac);
    memcpy(mac_sn, tmp.ifr_hwaddr.sa_data, 6);

    return 0;
}


//获取MAC
HB_U64 get_sys_mac()
{
	HB_CHAR mac[32] = {0};
	HB_CHAR get_mac[32] = {0};
	get_mac_dev(get_mac, ETHX);

    sprintf(mac, "0x%02x%02x%02x%02x%02x%02x",
            (HB_U8)get_mac[0],
            (HB_U8)get_mac[1],
            (HB_U8)get_mac[2],
            (HB_U8)get_mac[3],
            (HB_U8)get_mac[4],
            (HB_U8)get_mac[5]
            );

    return strtoull(mac, 0, 16);
}

//获取网卡序列号
HB_S32 get_sys_sn(HB_CHAR *sn, HB_S32 sn_size)
{
	HB_U64 sn_num = 0;
	HB_CHAR sn_mac[32] = {0};
	HB_CHAR mac[32] = {0};
	get_mac_dev(mac, ETHX);
	sprintf(sn_mac, "0x%02x%02x%02x%02x%02x%02x",
			(HB_U8)mac[0],
			(HB_U8)mac[1],
			(HB_U8)mac[2],
			(HB_U8)mac[3],
			(HB_U8)mac[4],
			(HB_U8)mac[5]);
	sn_num = strtoull(sn_mac, 0, 16);
	snprintf(sn, sn_size, "%llu", sn_num);

	printf("%s===%d\t%s==%d\n",sn_mac,strlen(sn_mac), sn, strlen(sn));
	return 0;
}


//用于获取是否开启了广域网
static HB_S32 GetWanConnectionStatusCb( HB_VOID * para, HB_S32 n_column, HB_CHAR ** column_value, HB_CHAR ** column_name )
{
	HB_CHAR *pWanConnection = (HB_CHAR *)para;

	strncpy(pWanConnection, column_value[0], strlen(column_value[0]));

	return 0;
}


//用于获取是否开启了广域网
//返回0 关闭， 返回1 开启
HB_S32 GetWanConnectionStatus()
{
	sqlite3 *db;
	HB_CHAR *errmsg = NULL;
	HB_S32 ret = 0;
	HB_CHAR sql[512] = {0};
	HB_CHAR WanConnection[8] = {0};

	ret = sqlite3_open(BOX_DATA_BASE_NAME, &db);
	if (ret != SQLITE_OK) {
		printf("***************%s:%d***************\nsqlite3_open error[%d]\n", __FILE__, __LINE__, ret);
		return -1;
	}

	strcpy(sql, "select value from system_config_data where key='wan_connection'");
	ret = sqlite3_exec(db, sql, GetWanConnectionStatusCb, (HB_VOID*)WanConnection, &errmsg);
	if (ret != SQLITE_OK) {
		printf("***************%s:%d***************\nsqlite3_exec get wan_connection error[%d]:%s\n", __FILE__, __LINE__, ret, errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return -2;
	}

//	printf("strlen(WanConnection) : [%s],[%d]\n", WanConnection, strlen(WanConnection));

	if ( strlen(WanConnection) <= 0 )
	{
		printf("Instert new value wan_connection\n");
		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "insert into system_config_data (key,value) values ('wan_connection','1')");
		ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
		if (ret != SQLITE_OK) {
			printf("***************%s:%d***************\nsqlite3_exec set wan_connection error[%d]:%s\n", __FILE__, __LINE__, ret, errmsg);
			sqlite3_free(errmsg);
			sqlite3_close(db);
			return -2;
		}
		strcpy(WanConnection, "1");
	}


	sqlite3_free(errmsg);
	sqlite3_close(db);

	if (atoi(WanConnection) == 0)
	{
		printf("wan closed!\n");
		return 0;
	}

	printf("wan started!\n");
	return 1;
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
