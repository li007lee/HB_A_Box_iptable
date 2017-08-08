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
#include "hf_plant_net.h"
#include "hf_plant_api.h"
#include "get_set_config.h"

#define IP_VERTICAL 		6
#define IP_HORIZONTAL 		40

static int get_host_name(char *argv, char (*ip)[IP_HORIZONTAL], int ip_vertical)
{
	int i = 0;
	char *ptr,**pptr;
	struct hostent *hptr;
	char str[32];
	/* 取得命令后第一个参数，即要解析的域名或主机名*/
	ptr = argv;
	if(ip == NULL)
	{
		return -1;
	}
	res_init();
	/* 调用gethostbyname()。调用结果都存在hptr中*/
	if((hptr = gethostbyname(ptr)) == NULL)
	//if((hptr = getnameinfo(ptr)) == NULL)
	{
		TRACE_ERR("gethostbyname error for host:%s\n", ptr);
		return -1; /* 如果调用gethostbyname发生错误，返回 */
	}
	/* 将主机的规范名打出来 */
	TRACE_LOG("official hostname:%s\n",hptr->h_name);
	/* 主机可能有多个别名，将所有别名分别打出来 */
	for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
	{
		TRACE_LOG(" alias:%s\n",*pptr);
	}

	/* 根据地址类型，将地址打出来*/
	switch(hptr->h_addrtype)
	{
		case AF_INET:
		case AF_INET6:
		pptr=hptr->h_addr_list;
		/* 将刚才得到的所有地址都打出来。其中调用了inet_ntop()函数 */
		for(; *pptr != NULL; pptr++)
		{
			memset(str, 0, sizeof(str));
//			printf("1: address:%s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
			if(inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)) != NULL)
			{
				if(i < ip_vertical)
				{
//					printf("2: address:%s\tsize = %d\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)), size);
					memcpy(ip, str, strlen(str));
					TRACE_LOG("\nip gethostbyname=[%s]\n", str);
					ip++;
				}
				i++;
			}
		}
		break;
		default:
			TRACE_ERR("unknown address type\n");
		break;
	}
	return i;
}


static int express_ip_addr(xmlDoc *doc, void *param, char *tags, char *values)
{
    static int ri = 0;
    static int ai = 0;
    char *point = NULL;
    char ip_buf[16];
    char port[8];
 	if((point = strstr(tags, "Aegis_Server")) != NULL && ai < IP_LIST_MAX)
	{   
//	    plant_msg.ip_stall.server_type = AEGIS_SERVER;
	    sscanf(values, "%[0-9,.]:%[0-9]",ip_buf,port);
//	    ai++;
//	    if(ai >= IP_LIST_MAX)
	    {
//	        ai = 0;
	    }
	}
	else if((point = strstr(tags, "Regist_Server")) != NULL && ri < IP_LIST_MAX)
	{   
//	    plant_msg.ip_stall.server_type = AEGIS_SERVER;
//	    sscanf(values, "%[0-9,.]:%[0-9]", plant_msg.ip_stall[REGIST_SERVER].ip_list[ai].ip,
//	            plant_msg.ip_stall[REGIST_SERVER].ip_list[ai].port);
//	    ri++;
	}
	else
	{
	}
	return 1;
}


static int read_server_ip_list(char *srv_ip, int *port)//读取服务器列表，预留
{
	int i = 0;
    int size = 0;
	xmlDoc *doc;
    FILE *fp = NULL;
    char *tmp = NULL;
//    char *point = NULL;
    int iprtn_count = 0;
    char ip_str[IP_VERTICAL][IP_HORIZONTAL];
    if(srv_ip == NULL || port == NULL)
    {
    	return -1;
    }
    memset(ip_str, 0, sizeof(ip_str));
    
    fp = fopen(SRV_IP_LIST, "rb");
    if(fp == NULL)
    {
		iprtn_count = get_host_name(PT_ADDR_IP, ip_str, IP_VERTICAL);
		if(iprtn_count <= 0)
		{
			return -1;
		}
		for(i = 0; i< IP_VERTICAL; i++)
			if(strlen(ip_str[i]) > 0)
				TRACE_LOG("ip = %s\n",ip_str[i]);
		TRACE_LOG("%s:\tip get success!ip:[%s],port:[%d]================>\n",__FUNCTION__, ip_str[0], PT_PORT);

		strcpy(srv_ip, ip_str[0]);
		*port = PT_PORT;
    }
    else
    {
        fseek(fp, 0, SEEK_END);/*移动指针到文件尾*/
        size = ftell(fp);        /*取指针的位置来获得长度*/
        tmp = malloc(size *sizeof(char) + 1);
        size = fread(tmp, size * sizeof(char), 1, fp);
    	doc = File2MemxmlParseToDoc(tmp, NULL, express_ip_addr);
    	xmlFreeDoc(doc);
    	if(fp != NULL)
    	{
    	    fclose(fp);
    	}
        free(tmp);
    }

    return iprtn_count;
}

static int connect_noblock(int sockfd, struct sockaddr *addr, socklen_t salen, int nsec)
{
    int flags, n, error;
    socklen_t len;
    fd_set rset, wset;
    struct timeval tval;
    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    error = 0;
    if ((n = connect(sockfd, addr, salen)) < 0)
    {
        if (errno != EINPROGRESS)
            return -1;
    }

    if (0 == n)
    {
        goto done;					// connect completed immediately
    }

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;
    if (0 == (n = select(sockfd + 1, &rset, &wset, NULL, nsec ? &tval : NULL)))	// timeout
    {
        return -1;
    }

    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
    {
        len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {
            return -1;				// solaris pending error
        }
    }
    else
    {
    	TRACE_ERR("select error:sockfd no set!\n");
    }

done:
//    fcntl(sockfd, F_SETFL, flags);	// restrore file status flags
    if (error)
    {
        return -1;
    }

    return 0;
}

static int pt_connect(int *psockfd, char *addr, int port, int waitsec)
{
    struct sockaddr_in server_addr;
//    int retval = -1;
    int trueflag = 1;

	if(*psockfd <= 0 || addr == NULL || port <= 0)
	{
		assert(0);
		return 0;
	}
    setsockopt(*psockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&trueflag, sizeof(trueflag));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(addr);
    server_addr.sin_port = htons(port);
    memset(server_addr.sin_zero, 0, 8);

    if (connect_noblock(*psockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in), waitsec) < 0)
    {
    	return -1;
    }
    return 1;
}


int init_socket2platform(int *psockfd, enum_server_type server_types)
{
    char ipaddr[32];
    int port = 0;
    int ret = -1;
    if(*psockfd <= 0)
    {
	    if((*psockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    {
	        return HB_FAILURE;
	    }
    }
    memset(ipaddr, 0, sizeof(ipaddr));
    if(read_server_ip_list(ipaddr, &port) < 0)
    {
		if(*psockfd > 0)
		{
			close(*psockfd);
		    *psockfd = -1;
		}
    	return HB_FAILURE;
    }
    
    ret = pt_connect(psockfd, ipaddr, port, 10);
    if(ret < 0)
    {
    	if(*psockfd > 0)
    	{
    		close(*psockfd); 
	        *psockfd = -1;
    	}
        return HB_FAILURE;
    }
    else
    {
        return HB_SUCCESS;
    }
}

int send_to_pt(int *sockfd, void *buf, size_t n, int waitsec)
{
	struct timeval wait_time;
    int err = 0;
    fd_set  writeset;

    int len = 0, totalen = n;
	char * tmp = buf;

	if(*sockfd < 0 || NULL == buf || n <= 0)
	{
		assert(0);
		return -1;
	}
	while (n > 0)
    {
        FD_ZERO(&writeset);
        FD_SET(*sockfd, &writeset);
        wait_time.tv_sec = waitsec;
        wait_time.tv_usec = 0;

        err = select(FD_SETSIZE, NULL, &writeset, NULL, &wait_time);
        if (err < 0)
        {
	    	if(*sockfd > 0)
	    	{
	    		close(*sockfd); 
		        *sockfd = -1;
	    	}
	    	TRACE_ERR("select err=%d(%s)\n", errno, strerror(errno));
        	return -1;
        }
        else if (err == 0)
        {
        	TRACE_LOG("select timeout\n");
        	continue;
        }
        if ((len = send(*sockfd, tmp, n, MSG_NOSIGNAL)) <= 0)
        {
            if (errno == EINTR)
            {
            	TRACE_ERR("send err=%d(%s), again\n", errno, strerror(errno));
                continue; /* just an interrupted system call */
            }
            else
            {
		    	if(*sockfd > 0)
		    	{
		    		close(*sockfd); 
			        *sockfd = -1;
		    	}
            	ODD_CHECK_PUT_MSG(sockfd, NETWORK_ANOMALY);
            	TRACE_ERR("send err=%d(%s)\n", errno, strerror(errno));
                return -1;
            }
        }
        tmp += len;
        n -= len;
    }
	return totalen;
}


int sock_recv(int *sockfd, char * buf, int len, int waitsec)
{
    struct timeval timeout1 = {0, 0};
	int retval = 0;
	int ret;
	fd_set fdread;
    timeout1.tv_sec   = waitsec;
    timeout1.tv_usec  = 0;
    if(*sockfd <= 0 || buf == NULL || len <= 0)
    {
    	assert(0);
    	return 0;
    }
    FD_ZERO(&fdread);
    FD_SET(*sockfd, &fdread);
	ret = select(*sockfd + 1, &fdread, NULL, NULL, &timeout1);
    if (0 == ret)		// 超时
    {
    	TRACE_LOG("select timeout!\n");
        return ret;
    }
    else if (0 > ret)	// 出错
    {
    	if(*sockfd > 0)
    	{
    		close(*sockfd); 
	        *sockfd = -1;
    	}
    	TRACE_ERR("select error!\n");
        return ret;
    }
    do
    {
        retval = recv(*sockfd, buf, len, 0);
        if (retval <= 0)
        {
	    	if(*sockfd > 0)
	    	{
	    		close(*sockfd); 
		        *sockfd = -1;
	    	}
        	ODD_CHECK_PUT_MSG(sockfd, NETWORK_ANOMALY);
        	TRACE_ERR("recv error! retval= %d\n", retval);
        }
        usleep(5000);
    }
    while (((-1) == retval) && (EINTR == errno));
    return retval;
}

int close_sockfd(int *sockfd)
{
	if(*sockfd > 0)
	{
		close(*sockfd);
		*sockfd = -1;
	}
	return 1;
}

