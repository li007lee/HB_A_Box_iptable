/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2012 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#include "my_include.h"
#include "xml_app.h"
#include "md5gen.h"
#include "hf_plant_api.h"
#include "get_set_config.h"
#include "net_api.h"

#include "box_info_upload.h"
#include "my_sqlite.h"
#include "common_args.h"

extern DEV_PLAT_MESSAGE_OBJ gl_plant_msg;
extern SERVER_INFO_STRUCT	stream_msg;
extern SERVER_INFO_STRUCT	heartbeat_server_msg;

//封装字符串
static HB_S32 make_url(HB_CHAR *pUrlBuf, HB_S32 iUrlBufSize, OPT_TYPE enumOptCmd)
{
    strcpy(gl_plant_msg.route_regist.ietype, "gn");
    if(strlen(gl_plant_msg.route_regist.sn_number) <= 0)
    {
    	strcpy(gl_plant_msg.route_regist.sn_number, glCommonArgs.cBoxSn);
    }

    switch (enumOptCmd) {
    	case REGISTER ://注册
    	{
    		snprintf(pUrlBuf, iUrlBufSize, "http://"PT_ADDR_IP"/b659708496b3b74b9fb0138cd19904c7/010100FF/act93802102/?appid=lt0JIFQ85J3Vl5wCmHTQxA&timestamp=%ld&imei=%s&sn=%s&vntype=%s&type=%s&softver=%s", time(NULL), glCommonArgs.cMachineCode, glCommonArgs.cBoxSn, gl_plant_msg.route_regist.ietype, glCommonArgs.cBoxType, glCommonArgs.cVersion);
    		break;
    	}
    	case GET_TOKEN : //令牌
    		snprintf(pUrlBuf, iUrlBufSize, "http://"PT_ADDR_IP"/b659708496b3b74b9fb0138cd19904c7/010100FF/act387429/?appid=lt0JIFQ85J3Vl5wCmHTQxA&timestamp=%ld&imei=%s&sn=%s", time(NULL), glCommonArgs.cMachineCode, glCommonArgs.cBoxSn);
    		break;
    	case GETSTREAMINFO://获取流媒体服务器ip及端口
    		snprintf(pUrlBuf, iUrlBufSize, "http://"PT_ADDR_IP"/b659708496b3b74b9fb0138cd19904c7/010100FF/act74339820/?appid=lt0JIFQ85J3Vl5wCmHTQxA&timestamp=%ld&imei=%s", time(NULL), glCommonArgs.cMachineCode);
    		break;
    	case GET_HEARTBEAT_SERVER_INFO://获取流媒体服务器ip及端口
    		snprintf(pUrlBuf, iUrlBufSize, "http://"PT_ADDR_IP"/b659708496b3b74b9fb0138cd19904c7/010100FF/act62749329/?appid=lt0JIFQ85J3Vl5wCmHTQxA&timestamp=%ld&imei=%s&sn=%s", time(NULL), glCommonArgs.cMachineCode, glCommonArgs.cBoxSn);
    		break;
    	default :
    		break;
    }

//    printf("pocket_string:[%s]\n", data);

    return 1;
}


static HB_S32 fn_routeregist_fun(xmlDoc *doc, void *param, HB_CHAR *tags, HB_CHAR *values)
{
//    static HB_S32 ifcheck_behind = 0;
//    printf("%s:tags:%s\t\t\t\tvalues:%s\n",__FUNCTION__, tags, values);
	if(strstr(tags, "code") != NULL)
	{
		if(atoi(values) == 0)
		{
//		    ifcheck_behind = 1;
			gl_plant_msg.return_regist = 1; //设备注册成功
		}
		else
		{
			gl_plant_msg.return_regist = 0;//设备注册失败
		}
	}

	return 1;
}


static HB_S32 fn_GetStreamServerIp_fun(xmlDoc *doc, void *param, HB_CHAR *tags, HB_CHAR *values)
{
//	struct hostent *hptr;
	HB_S32 index = stream_msg.num;

//    printf("%s:tags:%s\t\t\tvalues:%s\n",__FUNCTION__, tags, values);
	if(strstr(tags, "HostIP") != NULL)
	{
		if (index >= IP_LIST_MAX)
		{
			return 0;
		}
		memset(stream_msg.ip[index], 0, sizeof(stream_msg.ip[index]));
		if (from_domain_to_ip(stream_msg.ip[index], values, 2) < 0)
		{
			TRACE_ERR("gethostbyname error for host:%s\n", values);
			return -1; /* 如果调用gethostbyname发生错误，返回 */
		}
//		if((hptr = gethostbyname(values)) == NULL)
//		{
//			TRACE_ERR("gethostbyname error for host:%s\n", values);
//			return -1; /* 如果调用gethostbyname发生错误，返回 */
//		}
//		memset(stream_msg.ip[index], 0, sizeof(stream_msg.ip[index]));
//		snprintf(stream_msg.ip[index], sizeof(stream_msg.ip[index]), "%s", inet_ntoa(*((struct in_addr *)hptr->h_addr)));
//
		printf("ipaddr:%s\n", stream_msg.ip[index]);

		gl_plant_msg.return_stream = 2;
	}
	else if (strstr(tags, "HostPort") != NULL)
	{
		if (index >= IP_LIST_MAX)
		{
			return 0;
		}
		memset(stream_msg.port[index], 0, sizeof(stream_msg.port[index]));
		snprintf(stream_msg.port[index], sizeof(stream_msg.port[index]), "%s", values);

		if (gl_plant_msg.return_stream == 2)
		{
			stream_msg.num++;
			gl_plant_msg.return_stream = 1;
		}
	}

	return 1;
}



static HB_S32 fn_get_token_fun(xmlDoc *doc, void *param, HB_CHAR *tags, HB_CHAR *values)
{
    static HB_S32 ifcheck_behind = 0;
//    printf("%s:tags:%s\t\t\t\tvalues:%s\n",__FUNCTION__, tags, values);
    if(doc == NULL || param == NULL || tags == NULL || values == NULL)
    {
    	assert(0);
    	return 0;
    }
	if(strstr(tags, "code") != NULL)
	{
		if(atoi(values) == 0)
		{
		    ifcheck_behind = 1;
		}
		else
		{
			gl_plant_msg.return_token = 0; //令牌获取失败
		}
	}
	if(ifcheck_behind == 1)
	{
		if(strstr(tags, "TokenID") != NULL)
		{
			gl_plant_msg.return_token = 1; //令牌获取成功
		    gl_plant_msg.stru_token.tokenid = atoi(values);
		}
		else if(strstr(tags, "TokenName") != NULL)
		{
		    strcpy(gl_plant_msg.stru_token.tokenname, values);
		}
		else if(strstr(tags, "TokenPassword") != NULL)
		{
		    strcpy(gl_plant_msg.stru_token.tokenpassword, values);
		}
		else if(strstr(tags, "TokenIP") != NULL)
		{
		    strcpy(gl_plant_msg.stru_token.tokenip, values);
		}
//		else
//		{
//			printf("other data\n");
//		}
	}

	return 1;
}



static HB_S32 fn_GetHeartbeatServerIp_fun(xmlDoc *doc, void *param, HB_CHAR *tags, HB_CHAR *values)
{
//	struct hostent *hptr;

//    printf("%s:tags:%s\t\t\tvalues:%s\n",__FUNCTION__, tags, values);
	if(strstr(tags, "hostIP") != NULL)
	{
		memset(heartbeat_server_msg.ip[0], 0, sizeof(heartbeat_server_msg.ip[0]));
		if (from_domain_to_ip(heartbeat_server_msg.ip[0], values, 2) < 0)
		{
			TRACE_ERR("gethostbyname error for host:%s\n", values);
			return -1; /* 如果调用gethostbyname发生错误，返回 */
		}
//		if((hptr = gethostbyname(values)) == NULL)
//		{
//			TRACE_ERR("gethostbyname error for host:%s\n", values);
//			return -1; /* 如果调用gethostbyname发生错误，返回 */
//		}
//		memset(heartbeat_server_msg.ip[0], 0, sizeof(heartbeat_server_msg.ip[0]));
//		snprintf(heartbeat_server_msg.ip[0], sizeof(heartbeat_server_msg.ip[0]), "%s", inet_ntoa(*((struct in_addr *)hptr->h_addr)));

		printf("heartbeat server addr:%s\n", heartbeat_server_msg.ip[0]);
	}
	else if (strstr(tags, "hostPort") != NULL)
	{
		memset(heartbeat_server_msg.port[0], 0, sizeof(heartbeat_server_msg.port[0]));
		snprintf(heartbeat_server_msg.port[0], sizeof(heartbeat_server_msg.port[0]), "%s", values);
		printf("heartbeat server port:%s\n", heartbeat_server_msg.port[0]);
		gl_plant_msg.return_heart_beat = 1;
	}

	return 1;
}

//设备注册以及取令牌接口，成功返回HB_SUCCESS，失败返回HB_FAILURE
HB_S32 hb_box_opt_cmd_exec(HB_S32 *pSockfd, OPT_TYPE enumOptCmd)
{
    HB_CHAR cMsgBuff[1024] = {0};//用于信令通信的缓冲区
    HB_CHAR cTmpCmdUrl[1024] = {0};
	HB_CHAR cCmdUrl[1024] = {0}; //命令url
	HB_CHAR cTmpBuf[1024] = {0};
	HB_CHAR cSign[64] = {0}; //签名值
	HB_CHAR *pPos = NULL;
	xmlDoc *doc = NULL;

	//拼接信令url
	make_url(cTmpCmdUrl, sizeof(cTmpCmdUrl), enumOptCmd);
	//计算签名
	snprintf(cTmpBuf, sizeof(cTmpBuf), "%skJodi-OJNPodnoxTbgZKLr0VdR12xcEySrXDQ2vYZms53BAG", cTmpCmdUrl);
	md5_packages_string(cSign, cTmpBuf, strlen(cTmpBuf));
	//将签名加入url
	snprintf(cCmdUrl, sizeof(cCmdUrl), "%s&sign=%s", cTmpCmdUrl, cSign);

	pPos = strstr(cCmdUrl, PT_ADDR_IP);
	if (pPos == NULL) {
		pPos = cCmdUrl;
	} else {
		pPos+=sizeof(PT_ADDR_IP);
	}

	sprintf(cMsgBuff, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:keep-alive\r\nAccept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUpgrade-Insecure-Requests:1\r\nUser-Agent:Mozilla/5.0\r\nAccept-Encoding:gzip,deflate,sdch\r\nAccept-Language:zh-CN,zh;q=0.8\r\n\r\n",
					pPos, PT_ADDR_IP);

	if(send_data(pSockfd, cMsgBuff, strlen(cMsgBuff), 10) < 0)
	{
		TRACE_ERR("\n#######send failed\n");
		return HB_FAILURE;
	}
	TRACE_DBG("\n============Send Send Send Send============ \n[%s]\n",cMsgBuff);
	memset(cMsgBuff, 0, sizeof(cMsgBuff));
	if(recv_data(pSockfd ,cMsgBuff, sizeof(cMsgBuff), 10) < 0)
	{
		TRACE_ERR("\n#######recv failed\n");
		return HB_FAILURE;
	}
	TRACE_LOG("\n============Recv Recv Recv Recv============ \n[%s]\n",cMsgBuff);
	if((pPos = strstr(cMsgBuff,"<?xml")) != NULL)
	{
		switch(enumOptCmd)
		{
			case REGISTER:gl_plant_msg.return_regist = 0;doc = File2MemxmlParseToDoc(pPos, &pSockfd, fn_routeregist_fun);break;
			case GET_TOKEN:gl_plant_msg.return_token = 0;doc = File2MemxmlParseToDoc(pPos, &pSockfd, fn_get_token_fun);break;
			case GETSTREAMINFO:gl_plant_msg.return_regist = 0;doc = File2MemxmlParseToDoc(pPos, &pSockfd, fn_GetStreamServerIp_fun);break;
			case GET_HEARTBEAT_SERVER_INFO:gl_plant_msg.return_heart_beat = 0;doc = File2MemxmlParseToDoc(pPos, &pSockfd, fn_GetHeartbeatServerIp_fun);break;
			default: break;
		}
		xmlFreeDoc(doc);
	}
	else
	{
		return HB_FAILURE;
	}

    return HB_SUCCESS;
}


//重启网卡重设辅助ip
static HB_S32 SetLanIp( HB_VOID * para, HB_S32 n_column, HB_CHAR ** column_value, HB_CHAR ** column_name )
{
	HB_CHAR cmd[256] = {0};

	//eth0:num ip mask
	snprintf(cmd, sizeof(cmd), "ifconfig %s:%s %s netmask %s", ETHX, column_value[0], column_value[1], column_value[2]);
	printf("[cmd:%s]\n", cmd);
	system(cmd);

	return 0;
}

//重启网卡重设静态路由
static HB_S32 SetStaticRoute( HB_VOID * para, HB_S32 n_column, HB_CHAR ** column_value, HB_CHAR ** column_name )
{
	HB_CHAR cCmd[256] = {0};

	snprintf(cCmd, sizeof(cCmd), "route add -net %s netmask %s gw %s dev %s", column_value[0], column_value[1], column_value[2], ETHX);
	printf("[do cmd:%s]\n", cCmd);
	system(cCmd);

	return 0;
}

HB_S32 set_network(sqlite3 *pSqliteDbHandle)
{
	HB_CHAR cSqlCmd[512] = {0};

	memset(cSqlCmd, 0, sizeof(cSqlCmd));
	strncpy(cSqlCmd, "select net_ethx,net_ipaddr,net_mask from lan_web_ip_data", sizeof(cSqlCmd));
	if (HB_FAILURE == my_sqlite_exec(pSqliteDbHandle, cSqlCmd, SetLanIp, NULL))
	{
		return HB_FAILURE;
	}

	memset(cSqlCmd, 0, sizeof(cSqlCmd));
	strncpy(cSqlCmd, "select segment,mask,gateway from static_route_data", sizeof(cSqlCmd));
	if (HB_FAILURE == my_sqlite_exec(pSqliteDbHandle, cSqlCmd, SetStaticRoute, NULL))
	{
		return HB_FAILURE;
	}

	return HB_SUCCESS;
}


