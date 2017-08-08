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
#include "hf_plant_net.h"
#include "get_set_config.h"
#include "debug.h"
#include "sqlite3.h"

extern DEV_PLAT_MESSAGE_OBJ gl_plant_msg;
extern STREAM_INFO_STRUCT	stream_msg;
extern GLOBLE_MSG_STRUCT gl_msg;
//static PUSH_DEV_OBJ push_dev;

HB_S32 random_number(HB_S32 start_num, HB_S32 end_num)
{
	HB_S32 ret_num = 0;
	srand((unsigned)time(0));
	ret_num = rand() % (end_num - start_num) + start_num;
	return ret_num;
}


static HB_S32 my_ldexp(HB_S32 num, HB_S32 times)
{
	HB_S32 ret = 1;
	HB_S32 i = 0;
	if (times <= 0)
	{
		return 1;
	}
	else
	{
		for(i = 0; i < times; i++)
		{
			ret=ret*num;
		}
		return ret;
	}
}


//times为次数 从1开始n次结束, base_num为基数
HB_S32 sleep_times(HB_S32 times, HB_S32 base_num)
{
	HB_S32 start_num = 0;
	HB_S32 end_num = 0;
	start_num = base_num*my_ldexp(2,times-1);
	end_num = base_num*my_ldexp(2,times);
	return random_number(start_num, end_num);
}

//对src进行MD5加密，desc为加密后的结果，但在src的基础上增加了sn码，放在了desc的前端
//实际加密后的密文是sn+src的集合
static HB_S32 get_box_authcode(HB_CHAR *desc, HB_CHAR *src)
{
    HB_CHAR data[64];
    HB_CHAR str[128];
    if(desc == NULL || src == NULL)
    {
    	assert(0);
    	return -1;
    }
    memset(str, 0, sizeof(str));
    memset(data, 0, sizeof(data));
    if(strlen(gl_plant_msg.route_regist.sn_number) <= 0)
    {
    	get_sys_sn(data, sizeof(data));
    }
    else
    {
    	strcpy(data, gl_plant_msg.route_regist.sn_number);
    }
    md5_packages_string(desc, data);
//    printf("randtxt1:%s,str:%s\n",desc, src);
    snprintf(str, sizeof(str), "%s%s", desc, src);
    md5_packages_string(desc, str);
//    printf("randtxt2:%s\n",desc);
    return 1;
}


static HB_S32 api_getserver_randstr(HB_S32 *sockfd, HB_CHAR *buff, HB_S32 size)
{
    HB_S32 ret = -1;
    HB_CHAR authstr[1024];
    HB_CHAR *point = NULL;
    if(*sockfd <= 0 || buff == NULL || size == 0)
    {
       	assert(0);
    	return -1;
    }
    memset(authstr, 0, sizeof(authstr));
    memset(buff, 0, size);
    sprintf(buff, "GET /?act=getrand HTTP/1.1\r\nHost:%s:%d\r\nConnection: keep-alive\r\nUser-Agent: Mozilla/5.0\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n", PT_ADDR_IP, PT_PORT);
    ret = send_to_pt(sockfd, buff, strlen(buff), 5);
    if(ret < 0)
    {
        return HB_FAILURE;
    }
    memset(buff, 0, size);
    if(sock_recv(sockfd ,buff, size, 5) < 0)
    {
        return HB_FAILURE;
    }
    if ((point = strstr(buff, "APIResult:")) == NULL)
    {
        return HB_FAILURE;
    }
    memcpy(authstr, point + strlen("APIResult:"), strlen(point) - strlen("APIResult:"));
    memset(buff, 0, size);
    memcpy(buff, authstr, strlen(authstr));
    return HB_SUCCESS;
}


//封装字符串
static HB_S32 packaging_str(HB_CHAR *data, HB_S32 size, OPT_TYPE flag)
{
    if(data == NULL || size == 0)
    {
    	assert(0);
    	return 0;
    }

    HB_CHAR *ps1 = NULL;
    HB_CHAR mac_sn[32] = {0};

    HB_CHAR box_type[32] = {0};
    HB_CHAR version_str[16] = {0};

	FILE *fp_t = NULL;
	fp_t = fopen(BOX_VERSION_FILE, "r");
	if (NULL == fp_t)
	{
		return 0;
	}
	else
	{
		fgets(box_type, 32, fp_t);
	    if ((ps1=strchr(box_type,'\r')) != NULL)
	    {
	        *ps1 = '\0';
	    }
	    else if ((ps1=strchr(box_type,'\n')) != NULL)
	    {
	        *ps1 = '\0';
	    }
		fgets(version_str, 16, fp_t);
	    if ((ps1=strchr(version_str,'\r')) != NULL)
	    {
	        *ps1 = '\0';
	    }
	    else if ((ps1=strchr(version_str,'\n')) != NULL)
	    {
	        *ps1 = '\0';
	    }
		fclose(fp_t);
	}

    get_sys_sn(mac_sn,sizeof(mac_sn));

    printf("mac_sn:[%s]\n", mac_sn);

    strcpy(gl_plant_msg.route_regist.ietype, "gn");
    if(strlen(gl_plant_msg.route_regist.sn_number) <= 0)
    {
    	strcpy(gl_plant_msg.route_regist.sn_number, mac_sn);
    }

    switch (flag) {
    	case REGISTER ://注册
    		snprintf(data, size, "http://"PT_ADDR_IP"/b659708496b3b74b9fb0138cd19904c7/010100FF/act93802102/?appid=lt0JIFQ85J3Vl5wCmHTQxA&timestamp=%ld&imei=%s&sn=%s&vntype=%s&type=%s&softver=%s", time(NULL), gl_msg.machine_code, mac_sn, gl_plant_msg.route_regist.ietype, box_type, version_str);
    		break;
    	case TOKEN : //令牌
    		snprintf(data, size, "http://"PT_ADDR_IP"/b659708496b3b74b9fb0138cd19904c7/010100FF/act387429/?appid=lt0JIFQ85J3Vl5wCmHTQxA&timestamp=%ld&imei=%s&sn=%s", time(NULL), gl_msg.machine_code, mac_sn);
    		break;
    	case GETSTREAMINFO://获取流媒体服务器ip及端口
    		snprintf(data, size, "http://"PT_ADDR_IP"/b659708496b3b74b9fb0138cd19904c7/010100FF/act74339820/?appid=lt0JIFQ85J3Vl5wCmHTQxA&timestamp=%ld&imei=%s", time(NULL), gl_msg.machine_code);
    		break;
    	default :
    		break;
    }

//    printf("pocket_string:[%s]\n", data);

    return 1;
}


static HB_S32 fn_routeregist_fun(xmlDoc *doc, void *param, HB_CHAR *tags, HB_CHAR *values)
{
	HB_S32 sockfd = -1;
    static HB_S32 ifcheck_behind = 0;
    printf("%s:tags:%s\t\t\t\tvalues:%s\n",__FUNCTION__, tags, values);
	if(strstr(tags, "code") != NULL)
	{
		if(atoi(values) == 0)
		{
		    ifcheck_behind = 1;
			gl_plant_msg.return_regist = 1; //设备注册成功
		}
		else
		{
			gl_plant_msg.return_regist = 0;//设备注册失败
			ODD_CHECK_PUT_MSG(&sockfd, DEVICE_REGIST_ODD);//�쳣�������ϴ�
		}
	}
	else
	{
		ODD_CHECK_PUT_MSG(&sockfd, DEVICE_REGIST_ODD);
	}
	return 1;
}


static HB_S32 fn_GetStreamServerIp_fun(xmlDoc *doc, void *param, HB_CHAR *tags, HB_CHAR *values)
{
	struct hostent *hptr;
	HB_S32 index = stream_msg.num;

    printf("%s:tags:%s\t\t\tvalues:%s\n",__FUNCTION__, tags, values);
	if(strstr(tags, "HostIP") != NULL)
	{
		if((hptr = gethostbyname(values)) == NULL)
		{
			TRACE_ERR("gethostbyname error for host:%s\n", values);
			return -1; /* 如果调用gethostbyname发生错误，返回 */
		}
		if (index >= IP_LIST_MAX)
		{
			return 0;
		}
		memset(stream_msg.ip[index], 0, sizeof(stream_msg.ip[index]));
		snprintf(stream_msg.ip[index], sizeof(stream_msg.ip[index]), "%s", inet_ntoa(*((struct in_addr *)hptr->h_addr)));

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
	HB_S32 sockfd = -1;
    static HB_S32 ifcheck_behind = 0;
    printf("%s:tags:%s\t\t\t\tvalues:%s\n",__FUNCTION__, tags, values);
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
			gl_plant_msg.return_token = 0; //�Ӻ�����ȡ����ʧ��
		}
	}
	if(ifcheck_behind == 1)
	{
		if(strstr(tags, "TokenID") != NULL)
		{
			gl_plant_msg.return_token = 1; //�Ӻ�����ȡ���Ƴɹ�
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
		else
		{
			printf("other data\n");
		}
	}
	else
	{
		ODD_CHECK_PUT_MSG(&sockfd, DEVICE_REGIST_TOKEN);
	}
	return 1;
}

static HB_S32 fn_send_device_odd_fun(xmlDoc *doc, void *param, HB_CHAR *tags, HB_CHAR *values)
{
    if(doc == NULL || param == NULL || tags == NULL || values == NULL)
    {
    	assert(0);
    	return 0;
    }
	if(strstr(tags, "CheckInterval") != NULL)
	{
	    gl_plant_msg.checkinterval = atoi(values);
	}
	if(strstr(tags, "RadixVal") != NULL)
	{
	    gl_plant_msg.radixval = atoi(values);
	}

    gl_plant_msg.return_push = 1;
	printf("tags=%s\t\tvalues=%s\n",tags, values);
	return 1;
}


static HB_S32 api_get_token(HB_S32 *sockfd, HB_CHAR *buff, HB_S32 size)
{
	HB_CHAR strtmp[1024] = {0};
	HB_CHAR sendstr[1024] = {0};
	HB_CHAR desc[64];
	HB_CHAR *pos = NULL;
   	xmlDoc *doc;
    HB_CHAR *point = NULL;
    if(*sockfd <= 0 || buff == NULL || size == 0)
    {
    	assert(0);
    	return -1;
    }

    //拼接字符串,用于计算MD5值
    packaging_str(strtmp, sizeof(strtmp), TOKEN);
    //计算MD5值
    Calculate_MD5(desc, strtmp);
    //拼接发送字符串
    snprintf(sendstr, sizeof(sendstr), "%s&sign=%s", strtmp, desc);


    pos = strstr(sendstr, PT_ADDR_IP);
    if (pos == NULL) {
    	pos = strtmp;
    } else {
    	pos+=sizeof(PT_ADDR_IP);
    }
    printf("\nsend_str=%s\r\n", pos);

    sprintf(buff, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:keep-alive\r\nAccept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUpgrade-Insecure-Requests:1\r\nUser-Agent:Mozilla/5.0\r\nAccept-Encoding:gzip,deflate,sdch\r\nAccept-Language:zh-CN,zh;q=0.8\r\n\r\n",
        		pos, PT_ADDR_IP);

   	if(send_to_pt(sockfd, buff, strlen(buff), 10) < 0)
    {
       return -1;
    }
	printf("\n###### api_get_token() send: %s\n",buff);
	memset(buff, 0, sizeof(buff));
    if(sock_recv(sockfd ,buff, size, 10) < 0)
    {
        return -1;
    }
	printf("\n######## api_get_token() recv: %s\n",buff);
    if((point = strstr(buff,"<?xml")) != NULL)
    {
	    doc = File2MemxmlParseToDoc(point, &sockfd, fn_get_token_fun);
	    xmlFreeDoc(doc);
	}
	else
	{
		return -1;
	}
    return 1;
}

HB_S32 pkg_odd_msg(HB_CHAR *buff, HB_S32 length, enum_err_define index)
{
    if(buff == NULL || length <= 0)
    {
    	assert(0);
        return 0;
    }
    snprintf(buff, sizeof(buff), "%d&%s&%d&%d&%s&%s",index, "", MAX_ERR_CRITICAL*3,
    		MAX_ERR_CRITICAL * 3, "10%","%10");

	return 1;
}

static HB_S32 api_send_device_odd_msg(HB_S32 *sockfd, HB_CHAR *authcode, HB_CHAR *errmsg)
{
//    HB_S32 i = 0;
    HB_CHAR buff[1024];
   	xmlDoc *doc;
    HB_CHAR *point = NULL;
    memset(buff, 0, sizeof(buff));
    if(*sockfd <= 0 || authcode == NULL || errmsg == NULL)
    {
    	assert(0);
    	return -1;
    }
    if(gl_plant_msg.deviceid <= 0)
    {
    	return -1;
    }
    sprintf(buff, "GET /?act=devicestatuspush&ietype=hanbangdevice&deviceid=%d&errtype=%s&authcode=%s HTTP/1.1\r\nHost:%s:%d\r\nConnection: keep-alive\r\nUser-Agent: Mozilla/5.0\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n",
    gl_plant_msg.deviceid, errmsg, authcode, PT_ADDR_IP, PT_PORT);
    if(send_to_pt(sockfd, buff, strlen(buff), 10) < 0)
    {
        return -1;
    }
	printf("send:%s",buff);
    if(sock_recv(sockfd ,buff, sizeof(buff), 10) < 0)
    {
        return -1;
    }
	printf("recv:%s",buff);
    if((point = strstr(buff,"<?xml")) != NULL)
    {
	    doc = File2MemxmlParseToDoc(point, &sockfd, fn_send_device_odd_fun);
	    xmlFreeDoc(doc);
	}
	else
	{
		return -1;
	}
    return 1;
}


static HB_S32 push_err_to_server(HB_S32 *sockfd, HB_CHAR * errmsg)
{
	HB_CHAR buff[1024];
    HB_CHAR authcode[256];

	if(errmsg == NULL)
	{
		assert(0);
		return 0;
	}
	memset(buff, 0, sizeof(buff));
	memset(authcode, 0, sizeof(authcode));

    if(*sockfd <= 0)
    {
        if(init_socket2platform(sockfd, REGIST_SERVER) < 0)
        {
           return -1;
        }
    }
    if(api_getserver_randstr(sockfd, buff, sizeof(buff)) < 0)//从服务器获取随机码
    {
        return -1;
    }
    get_box_authcode(authcode, buff);
	if(api_send_device_odd_msg(sockfd, authcode, errmsg) < 0)
	{
		return -1;
	}
	return 1;
}

//计算MD5
int Calculate_MD5(char *desc, const char *src)
{
	char buf[1024] = {0};

	snprintf(buf, sizeof(buf), "%skJodi-OJNPodnoxTbgZKLr0VdR12xcEySrXDQ2vYZms53BAG", src);
	//printf("mk_md5_old = [%s]\r\n", buf);
	md5_packages_string(desc, buf);
	//printf("&&&&&&&&&&md5=[%s]", desc);

	return 1;
}


static HB_S32 api_route_regist(HB_S32 *sockfd, HB_CHAR *buff, HB_S32 size)
{
    HB_CHAR strtmp[1024] = {0};
    HB_CHAR sendstr[1024] = {0};
    HB_CHAR desc[64];
    HB_CHAR *pos = NULL;
   	xmlDoc *doc;
    HB_CHAR *point = NULL;
    memset(strtmp, 0, sizeof(strtmp));
    if(*sockfd <= 0 || buff == NULL || size == 0)
    {
    	assert(0);
    	return -1;
    }
    //拼接字符串,用于计算MD5值
    packaging_str(strtmp, sizeof(strtmp), REGISTER);
    //计算MD5值
    Calculate_MD5(desc, strtmp);
    //拼接发送字符串
    snprintf(sendstr, sizeof(sendstr), "%s&sign=%s", strtmp, desc);

    pos = strstr(sendstr, PT_ADDR_IP);
    if (pos == NULL) {
    	pos = sendstr;
    } else {
    	pos+=sizeof(PT_ADDR_IP);
    }
    //printf("\nsend_str=%s\r\n", strtmp);

    sprintf(buff, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:keep-alive\r\nAccept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUpgrade-Insecure-Requests:1\r\nUser-Agent:Mozilla/5.0\r\nAccept-Encoding:gzip,deflate,sdch\r\nAccept-Language:zh-CN,zh;q=0.8\r\n\r\n",
        		pos, PT_ADDR_IP);

    printf("\nRRRRR [%s]\n", buff);
    if(send_to_pt(sockfd, buff, strlen(buff), 10) < 0)
    {
        return -1;
    }
    TRACE_LOG("\n#######send: %s\n",buff);
    memset(buff, 0, size);
    if(sock_recv(sockfd ,buff, size, 10) < 0)
    {
        return -1;
    }
    TRACE_LOG("\nrecv: %s\n",buff);
    if((point = strstr(buff,"<?xml")) != NULL)
    {
	    doc = File2MemxmlParseToDoc(point, &sockfd, fn_routeregist_fun);
	    xmlFreeDoc(doc);
	}
	else
	{
		return -1;
	}

    return 1;
}


HB_S32 sleep_appoint_time(HB_S32 times)
{
	static HB_S32 prev_time_value;
    HB_S32 num;
    time_t my_time = 0;
    while(1)
    {
	    do
	    {
	    	usleep(1000 * 100);
	        srand((HB_U32)time(&my_time));
	        num = rand() % ((times + 1) * (gl_plant_msg.checkinterval + 1) * (1 + gl_plant_msg.deviceattr) * 10);
	    }
	    while(num == 0);

	    if(num > gl_plant_msg.checkinterval + prev_time_value / 3)
	    {
	    	break;
	    }
	}
	prev_time_value = num;
	TRACE_LOG("sleep time = [%d],times = [%d], base = [%d]=========>\n",num, times,((times + 1) * (gl_plant_msg.checkinterval + 1) * (1 + gl_plant_msg.deviceattr) * 10));
    sleep(num);
    return 1;
}


HB_S32 ODD_CHECK_PUT_MSG(HB_S32 *sockfd, enum_err_define err_index)//异常诊断数据上传
{
	HB_S32 index = 0;
	static HB_CHAR buff[1024];
	static HB_S32 err_total[ERR_DEFINE_END];
	for(index = 0; index < ERR_DEFINE_END;index++)
	{
		if(index == err_index)
		{
			err_total[index]++;
			break;
		}
	}
	printf("ODD_CHECK_PUT_MSG Running=[%d]....!\n",err_index + 1);
	for(index = 0; index < ERR_DEFINE_END; index++)
	{
		if(err_total[index] > MAX_ERR_CRITICAL * (gl_plant_msg.radixval > 0 ? gl_plant_msg.radixval : 1))
		{
			if(strlen(buff) > 0)
			{
				snprintf(buff + strlen(buff), sizeof(buff),",");
			}
			snprintf(buff + strlen(buff), sizeof(buff),"%d", index + 1);
			TRACE_LOG("now push_err_to_server running,err_total[%d]=%d\n",index, err_total[index]);
		}
	}
	if((strlen(buff) > 0) && push_err_to_server(sockfd, buff) > 0)
	{
		while(gl_plant_msg.return_push != 1)
		{
			sleep(2);
		}
		for(index = 0; index < ERR_DEFINE_END; index++)
		{
			if(err_total[index] > MAX_ERR_CRITICAL * (gl_plant_msg.radixval > 0 ? gl_plant_msg.radixval : 1))
			{
				err_total[index] = 0;
			}
		}
		memset(err_total, 0, sizeof(err_total));
		memset(buff, 0 ,sizeof(buff));
	}
	return 1;
}


//设备令牌获取
HB_S32 hb_box_get_token(HB_S32 *sockfd)
{
    HB_CHAR buff[1024];
   	HB_S32 sleep_num = 0;
	HB_S32 connect_times = 0;
	HB_S32 get_token_times = 0;
	HB_S32 pt_get_token_flag = 0;

    while (1)
    {
		if(connect_times > MAX_ERR_TIMES || get_token_times > MAX_ERR_TIMES)
		{
			pt_get_token_flag = -1;
			break;
		}

        if (*sockfd <= 0)
        {
           if (init_socket2platform(sockfd, REGIST_SERVER) < 0)
            {
        	   connect_times++;
               sleep_num = sleep_times(connect_times, 60);
               sleep(sleep_num);
               //sleep_appoint_time(connect_faile_times++);
                continue;
            }
        }

        memset(buff, 0, sizeof(buff));
        pt_get_token_flag = api_get_token(sockfd, buff, sizeof(buff));//从服务器获取令牌
        if (pt_get_token_flag < 0)
        {
            //ODD_CHECK_PUT_MSG(sockfd, DEVICE_REGIST_TOKEN);
        	get_token_times++;
            sleep_num = sleep_times(get_token_times, 60);
            sleep(sleep_num);
           // sleep_appoint_time(connect_faile_times++);
            continue;
        }
        else
        {
        	break;
        }

    }
    return pt_get_token_flag;
}


HB_S32 hb_box_register(HB_S32 *sockfd)
{
    HB_S32 pt_register_flag = 0;
    HB_S32 connect_times = 0;
	HB_S32 dev_regist_times = 0;
	HB_S32 sleep_num = 0;
    HB_CHAR buff[1024] = {0};

   	TRACE_LOG("\n###########pt_register() !!\n");
    while(1)
    {
		if(connect_times > MAX_ERR_TIMES || dev_regist_times > MAX_ERR_TIMES)
		{
			gl_plant_msg.return_regist = 0;
			pt_register_flag = -1;
			break;
		}

        if(*sockfd <= 0)
        {
            if(init_socket2platform(sockfd, REGIST_SERVER) <  0)
            {
            	connect_times++;
                sleep_num = sleep_times(connect_times, 60);
                sleep(sleep_num);
               	//sleep_appoint_time(connect_faile_times++);
                continue;
            }
        }

		memset(buff, 0, sizeof(buff));
        pt_register_flag = api_route_regist(sockfd, buff, sizeof(buff));//设备注册接口
        if (pt_register_flag < 0)
        {
           // ODD_CHECK_PUT_MSG(sockfd, DEVICE_REGIST_ODD);
        	dev_regist_times++;
            sleep_num = sleep_times(dev_regist_times, 60);
            sleep(sleep_num);
           // sleep_appoint_time(connect_faile_times++);
            continue;
        }
        else
        {
        	//gl_plant_msg.return_regist = 1;
        	break;
        }
    }

    return pt_register_flag;
}


HB_S32 hb_box_get_streamserver_info(HB_S32 *sockfd)
{
	HB_CHAR buff[1024] = {0};
	HB_CHAR strtmp[1024] = {0};
	HB_CHAR sendstr[1024] = {0};
	HB_CHAR desc[64] = {0};
	HB_CHAR *pos = NULL;
	xmlDoc *doc;
	HB_CHAR *point = NULL;

	//拼接字符串,用于计算MD5值
	packaging_str(strtmp, sizeof(strtmp), GETSTREAMINFO);
	//计算MD5值
	Calculate_MD5(desc, strtmp);
	//拼接发送字符串
	snprintf(sendstr, sizeof(sendstr), "%s&sign=%s", strtmp, desc);
//	printf("str*************************[%s]\n", strtmp);
	pos = strstr(sendstr, PT_ADDR_IP);
	if (pos == NULL)
	{
		pos = sendstr;
	}
	else
	{
		pos+=sizeof(PT_ADDR_IP);
	}

	sprintf(buff, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:keep-alive\r\nAccept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUpgrade-Insecure-Requests:1\r\nUser-Agent:Mozilla/5.0\r\nAccept-Encoding:gzip,deflate,sdch\r\nAccept-Language:zh-CN,zh;q=0.8\r\n\r\n",
				pos, PT_ADDR_IP);

	if(send_to_pt(sockfd, buff, strlen(buff), 10) < 0)
	{
		return -1;
	}
	TRACE_LOG("\n#######send: %s\n",buff);
	memset(buff, 0, sizeof(buff));
	if(sock_recv(sockfd ,buff, sizeof(buff), 10) < 0)
	{
		return -1;
	}
	TRACE_LOG("\nrecv: %s\n",buff);
	gl_plant_msg.return_stream = 0;
	stream_msg.num = 0;
	if((point = strstr(buff,"<?xml")) != NULL)
	{
		doc = File2MemxmlParseToDoc(point, &sockfd, fn_GetStreamServerIp_fun);
		xmlFreeDoc(doc);
	}
	else
	{
		return -1;
	}

	return 1;
}


void *scanning_task(void *param)
{
	HB_S32  ret;
	HB_CHAR tmp_buf[64] = {0};
	FILE *fp;
	//fp = popen("ifconfig eth0 | awk '/Link encap:/ {print $5}'", "r");
	while (1)
	{
		ret = get_sys_gnLan();
		if (ret) //gnlan成功登录后 修改mtu值
		{
			gl_plant_msg.gnlan_flag = 1;
			memset(tmp_buf, 0, 64);
			fp = popen("ifconfig gnLan | grep \"UP BROADCAST\" | awk '{print $4}'", "r");

			fgets(tmp_buf, sizeof(tmp_buf), fp);
			pclose(fp);
			if (NULL == strstr(tmp_buf, "1000"))
			{
				system("ifconfig gnLan mtu 1000");
			}
		}
		else
		{
			gl_plant_msg.gnlan_flag = 0;
		}
		sleep(120);

#if 0
		if (gl_plant_msg.gnlan_flag)//gnlan成功登录后检测要推送到设备
		{
			ret = check_normal_xml_file(XML_FILE_NAME);
			if (1 == ret)//文件状态发生变化
			{
				ret = push_dev_to_server(XML_FILE_NAME);
				if (ret != HB_SUCCESS)
				{
					sleep(120);
				}
			}
		}
#endif
		//sleep(30);
#if 0
		int ttt=0;
		while(1)
		{
			sleep(1);
			TRACE_LOG("\n### sleep %d second\n", ttt);
			ttt++;
			if(ttt>10)
			{
				break;
			}
		}
#endif
	}
	return NULL;
}

