
//
//  thread_depend.c
//  MapPort
//
//  Created by MoK on 15/1/21.
//  Copyright (c) 2015年 MoK. All rights reserved.
//

#include "my_include.h"
#include <setjmp.h>
#include "net_api.h"

#define LISTEN_BACKLOG_SIZE (100)

typedef struct _tagDOMAIN_PARSE_ARG
{
	HB_CHAR domain[256];
	HB_S32 pipe_fd;
}DOMAIN_PARSE_ARG_OBJ, *DOMAIN_PARSE_ARG_HANDLE;


#if 0
static sigjmp_buf sig_jmp_buf;

static HB_VOID alarm_func()
{
    siglongjmp(sig_jmp_buf, 1);
}


//timeout秒内，能够通过domain获取相应的ip，则获取ip成功，否则失败
static struct hostent *get_host_by_name_timeout(const HB_CHAR *domain, HB_S32 timeout)
{
    struct hostent *ipHostent = NULL;
    signal(SIGALRM, alarm_func);
    if(sigsetjmp(sig_jmp_buf, 1) != 0)
    {
        alarm(0);//timout
        signal(SIGALRM, SIG_IGN);
        return NULL;
    }
    alarm(timeout);//setting alarm
    ipHostent = gethostbyname(domain);
    signal(SIGALRM, SIG_IGN);
    return ipHostent;
}

//返回值大于0，则解析成功
static HB_S32 get_host_name(HB_CHAR *domain, HB_CHAR (*ip)[IP_HORIZONTAL], HB_S32 ip_vertical, HB_S32 timeout)
{
	if(NULL == domain || NULL == ip)
	{
		return -1;
	}
	HB_S32 i = 0;
	HB_CHAR **pptr;
	struct hostent *hptr;
	HB_CHAR str[32];

	res_init();

	if((hptr = get_host_by_name_timeout(domain, timeout)) == NULL) //在指定的timeout秒内解析ip失败
	{
		printf("gethostbyname error for host:%s\n", domain);
		return -1; // 如果调用gethostbyname发生错误，返回-1
	}
	// 将主机的规范名打出来
	//TRACE_LOG("official hostname:%s\n",hptr->h_name);
	//主机可能有多个别名，将所有别名分别打出来
	//for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
	//{
	//	TRACE_LOG(" alias:%s\n",*pptr);
	//}

	/* 根据地址类型，将地址打出来 */
	switch(hptr->h_addrtype)
	{
		case AF_INET:
		case AF_INET6:
			pptr=hptr->h_addr_list;
			// 将刚才得到的所有地址都打出来。其中调用了inet_ntop()函数
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
						printf("######### gethostbyname=[%s]\n", str);
						ip++;
					}
					i++;
				}
			}
			break;
		default:
			printf("Unknown address type\n");
			break;
	}
	return i;
}


//通过域名解析出相应的ip，超过timeout秒解析不出来，则返回失败-1，成功返回值大于0
static HB_S32 from_domain_to_ip(HB_CHAR *srv_ip, HB_CHAR *srv_domain, HB_S32 timeout)
{
    if(NULL == srv_ip || NULL == srv_domain || timeout <= 0)
    {
    	return -1;
    }

    HB_S32 iprtn_count = 0;
    HB_CHAR ip_str[IP_VERTICAL][IP_HORIZONTAL];

    memset(ip_str, 0, sizeof(ip_str));

	iprtn_count = get_host_name(srv_domain, ip_str, IP_VERTICAL, timeout);
	if(iprtn_count <= 0)
	{
		return -1;
	}
#if 0
	HB_S32 i = 0;
	for(i = 0; i< IP_VERTICAL; i++)
	{
		if(strlen(ip_str[i]) > 0)
		{
			//TRACE_LOG("ip = %s\n",ip_str[i]);
		}
	}
#endif
	//TRACE_LOG("\n#############  tip get success!ip:[%s]================>\n",ip_str[0]);
	strcpy(srv_ip, ip_str[0]);

    return iprtn_count;
}
#endif

static HB_VOID *parse_domain_task(HB_VOID *param)
{
	HB_S32 ret = 0;
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    struct sockaddr_in *addr;
    HB_CHAR srv_ip[16] = {0};
    HB_CHAR ipbuf[16] = {0};
    DOMAIN_PARSE_ARG_HANDLE parse_arg = (DOMAIN_PARSE_ARG_HANDLE)param;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Allow IPv4 */
    hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;

    //printf("\n######  begin parse domain:[%s]\n", parse_arg->domain);

    ret = getaddrinfo(parse_arg->domain, NULL,&hints,&res);
    printf("*************return val = [%d]\n", ret);
    if (ret < 0)
    {
        //perror("#####  parse err getaddrinfo");
        return NULL;
    }

    for (cur = res; cur != NULL; cur = cur->ai_next)
    {
        addr = (struct sockaddr_in *)cur->ai_addr;
        sprintf(srv_ip, "%s", inet_ntop(AF_INET, &addr->sin_addr, ipbuf, 16));
        write(parse_arg->pipe_fd, srv_ip, strlen(srv_ip));
        break;
    }
    freeaddrinfo(res);
    return NULL;
}

//通过域名解析出相应的ip，超过timeout秒解析不出来，则返回失败-1，成功返回值大于0
HB_S32 from_domain_to_ip(HB_CHAR *srv_ip, HB_CHAR *srv_domain, HB_S32 timeout)
{
	HB_S32 ret = 0;
	HB_S32 fd[2] = {0};
	HB_CHAR recv_buf[32] = {0};
	DOMAIN_PARSE_ARG_OBJ domain_arg;
	memset(&domain_arg, 0, sizeof(DOMAIN_PARSE_ARG_OBJ));
	struct timeval tval;
	fd_set rset;

	pipe(fd);
    FD_ZERO(&rset);
    FD_SET(fd[0],&rset);
    tval.tv_sec = timeout;
    tval.tv_usec = 0;

    domain_arg.pipe_fd = fd[1];
    memcpy(domain_arg.domain, srv_domain, strlen(srv_domain));
	pthread_attr_t attr;
	pthread_t parse_domain_pthread_id;
	ret = pthread_attr_init(&attr);
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&parse_domain_pthread_id, &attr, parse_domain_task, (HB_VOID *)(&domain_arg));
	pthread_attr_destroy(&attr);

	ret = select(fd[0] + 1, &rset, NULL, NULL, &tval);
    if (0 == ret) // timeout
    {
    	close(fd[0]);
    	close(fd[1]);
    	printf("Select time out!\n");
        return -1;
    }
    else if (ret < 0)
    {
    	close(fd[0]);
    	close(fd[1]);
    	printf("Select error !\n");
        return -1;
    }
    else
    {
        if (FD_ISSET(fd[0], &rset))
        {
            ret = read(fd[0], recv_buf, sizeof(recv_buf));
            printf("parse domain [%s] to ip [%s]\n", srv_domain, recv_buf);
            strcpy(srv_ip, recv_buf);
        }
    	close(fd[0]);
    	close(fd[1]);
    }

    return 0;
}


static HB_S32 connect_noblock(HB_S32 sockfd, struct sockaddr *addr, socklen_t salen, HB_S32 nsec)
{
    HB_S32 flags, n, error;
    socklen_t len;
    fd_set rset, wset;
    struct timeval tval;
    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    error = 0;
    if ((n = connect(sockfd, addr, salen)) < 0)
    {
        if (errno != EINPROGRESS)
        {
        	printf("connect error[%d]!\n", n);
        	return -1;
        }
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
//    	printf("select error[%d]!\n", n);
        return -1;
    }

    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
    {
        len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {
        	printf("getsockopt error!\n");
            return -1;				// solaris pending error
        }
    }

done:
    if (error)
    {
    	printf("connect error[%d]!\n", n);
    	return -1;
    }

    return 0;
}

static HB_S32 pt_connect(HB_S32 *psockfd, HB_CHAR *addr, HB_S32 port, HB_S32 waitsec)
{
    struct sockaddr_in server_addr;
    HB_S32 trueflag = 1;

	if(*psockfd <= 0 || addr == NULL || port <= 0)
	{
		//assert(0);
		return -2;
	}
    setsockopt(*psockfd, SOL_SOCKET, SO_REUSEADDR, (HB_VOID *)&trueflag, sizeof(trueflag));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(addr);
    server_addr.sin_port = htons(port);
    memset(server_addr.sin_zero, 0, 8);

    if (connect_noblock(*psockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in), waitsec) < 0)
    {
    	return -1;
    }
    return 0;
}



////////////////////////////////////////////////////////////////////////////////
// 函数名：make_socket_non_blocking
// 描  述：设置网络文件描述符为非阻塞模式
// 参  数：[in] sock   网络文件描述符
// 返回值：成功返回1，出错返回0
// 说  明：
////////////////////////////////////////////////////////////////////////////////
static HB_S32 make_socket_non_blocking(HB_S32 sock)
{

  HB_S32 cur_flags = fcntl(sock, F_GETFL, 0);
  return fcntl(sock, F_SETFL, cur_flags|O_NONBLOCK) >= 0;
}

////////////////////////////////////////////////////////////////////////////////
// 函数名：setup_socket
// 描  述：设置流式套接字
// 参  数：[in] port              端口号
//         [in] make_non_blocking 是否是非阻塞模式的标识
// 返回值：成功返回网络文件描述符，出错返回ERR_GENERIC
// 说  明：
////////////////////////////////////////////////////////////////////////////////
static HB_S32 setup_socket(HB_U16 port, HB_S32 make_non_blocking)
{
	HB_S32 new_socket = -1;
    HB_S32 reuse_flag = 1;
    struct sockaddr_in s;

	//创建流式网络文件描述符
    if((new_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
    	//TRACE_ERR( "socket() error");
		return HB_FAILURE;
    }

	//防止端口被占用
    if(setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR,
		(HB_CHAR *)&reuse_flag, sizeof(reuse_flag)) < 0)
    {
    	close(new_socket);
    	//TRACE_ERR("setsockopt() error");
		return HB_FAILURE;
	}

	//设置网络地址
	memset(&s, 0x00, sizeof(struct sockaddr_in));
    s.sin_family = AF_INET;
    s.sin_addr.s_addr = htonl(INADDR_ANY);
    s.sin_port = htons(port);

	//绑定网络地址和网络文件描述符
    if(bind(new_socket, (struct sockaddr*)&s, sizeof(struct sockaddr_in)) != 0)
	{
		close(new_socket);
		//TRACE_ERR("bind() error in setup_stream_socket.");
		return HB_FAILURE;
    }

	//根据非阻塞模式标识判断是否要设置成非阻塞模式
  	if(make_non_blocking)
	{
	    if(!make_socket_non_blocking(new_socket))
		{
			close(new_socket);
			//TRACE_ERR("make_socket_non_blocking() error");
			return HB_FAILURE;
	    }
	}

	//增大网络缓冲区的大小
	#ifdef SOCKET_INCREASE_SEND_BUFFER
	if(!increase_send_buffer_to(new_socket, SOCKET_SEND_BUFFER))
	{
		hb_fnc_log(HB_LOG_ERR, "increase_send_buffer_to() error");
		return ERR_GENERIC;
	}
	#endif

	return new_socket;
}


////////////////////////////////////////////////////////////////////////////////
// 函数名：setup_listen_socket
// 描  述：设置服务监听套接字
// 参  数：[in] port  端口号
// 返回值：成功返回网络文件描述符，出错返回-1
// 说  明：
////////////////////////////////////////////////////////////////////////////////
HB_S32 setup_listen_socket(HB_U16 port)
{
	HB_S32 blocking_flag = 1;
  	HB_S32 socket = -1;

	//设置网络文件描述符以及阻塞模式
    socket = setup_socket(port, blocking_flag);
    if(socket < 0)
	{
    	//TRACE_ERR("\nsetup_listen_socket() error  !\n");
		return HB_FAILURE;
	}

    //监听网络文件描述符
    if(listen(socket, LISTEN_BACKLOG_SIZE) < 0)
	{
    	//TRACE_ERR("\nlisten() error in setup_listen_socket() !\n");
		return HB_FAILURE;
    }
    return socket;
}


////////////////////////////////////////////////////////////////////////////////
// 函数名：create_socket_connect_domain
// 描述：tcp网络连接
// 参数：
//  ［IN］psockfd - 套接字指针。
//  ［IN］domain – 要连接的域名
//    [IN] domain_port - 要连接域名的端口
//   [IN] timeout - 超时时间
// 返回值：-1失败,0成功
// 说明：
//通过域名和端口创建socket并且连接到对端，timeout秒后仍然没有连接上，则返回失败-1，成功返回0
////////////////////////////////////////////////////////////////////////////////
HB_S32 create_socket_connect_domain(HB_S32 *psockfd, HB_CHAR *domain, HB_S32 domain_port, HB_S32 timeout)
{
    HB_S32 ret = -1;
    HB_CHAR ipaddr[16];
    if(*psockfd <= 0)
    {
	    if((*psockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    {
	        return HB_FAILURE;
	    }
    }
    memset(ipaddr, 0, sizeof(ipaddr));
    if(from_domain_to_ip(ipaddr, domain, timeout) < 0)
    {
    	return HB_FAILURE;
    }

    ret = pt_connect(psockfd, ipaddr, domain_port, timeout);
    if(ret < 0)
    {
        return HB_FAILURE;
    }
    else
    {
        return HB_SUCCESS;
    }
}


////////////////////////////////////////////////////////////////////////////////
// 函数名：create_socket_connect_ipaddr
// 描述：tcp网络连接
// 参数：
//  ［IN］psockfd - 套接字指针。
//  ［IN］ipaddr – 要连接的IP地址
//    [IN] port - 要连接的端口
//   [IN] timeout - 超时时间
// 返回值：-1失败,0成功
// 说明：
//通过IP和端口创建socket并且连接到对端，timeout秒后仍然没有连接上，则返回失败
////////////////////////////////////////////////////////////////////////////////
HB_S32 create_socket_connect_ipaddr(HB_S32 *psockfd, HB_CHAR *ipaddr, HB_S32 port, HB_S32 timeout)
{
    HB_S32 ret = -1;

    if(*psockfd <= 0)
    {
	    if((*psockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    {
	    	printf("create socket failed[%d]!\n", *psockfd);
	        return HB_FAILURE;
	    }
    }

    ret = pt_connect(psockfd, ipaddr, port, timeout);
    if(ret < 0)
    {
    	printf("connect to [%s]:[%d] failed!\n", ipaddr, port);
        return HB_FAILURE;
    }
    else
    {
        return HB_SUCCESS;
    }
}


////////////////////////////////////////////////////////////////////////////////
// 函数名：send_data
// 描述：tcp网络数据发送
// 参数：
//  ［IN］psockfd - 套接字指针。
//  ［IN］send_buf -发送缓冲区首地址
//    [IN] data_len - 要发送的数据长度
//   [IN] timeout - 超时时间
// 返回值：-1超时，-2失败，0成功
// 说明：
//发送指定长度datalen的数据，如果timeout秒后还为发送，则返回超时失败-1，
//否则返回发送剩余的字节数，成功表示全部发送完成则返回0，否则失败返回非0
////////////////////////////////////////////////////////////////////////////////
HB_S32 send_data(HB_S32 *psockfd, HB_VOID *send_buf, HB_S32 data_len, HB_S32 timeout)
{
	if(*psockfd < 0 || NULL == send_buf || data_len <= 0)
	{
		return -1;
	}

    HB_S32 err = 0;
	struct timeval wait_time;
    fd_set  writeset;
    HB_S32 len = 0;

	while (data_len > 0)
    {
        FD_ZERO(&writeset);
        FD_SET(*psockfd, &writeset);
        wait_time.tv_sec = timeout;
        wait_time.tv_usec = 0;

        err = select(FD_SETSIZE, NULL, &writeset, NULL, &wait_time);
        if (err < 0)
        {
        	printf("select err=%d(%s)!\n", errno, strerror(errno));
        	return -2;
        }
        else if (err == 0)
        {
        	printf("select timeout\n");
        	return -1;
        }
        if ((len = send(*psockfd, send_buf, data_len, 0)) <= 0)
        {
            if (errno == EINTR)
            {
            	printf("send err=%d(%s), again\n", errno, strerror(errno));
                continue; /* just an interrupted system call */
            }
            else
            {
            	printf("send err=%d(%s)\n", errno, strerror(errno));
                return -2;
            }
        }
        send_buf += len;
        data_len -= len;
    }
	return data_len;
}


////////////////////////////////////////////////////////////////////////////////
// 函数名：recv_data
// 描述：tcp网络数据接收
// 参数：
//  ［IN］psockfd - 套接字指针。
//  ［IN］send_buf -接收缓冲区首地址
//    [IN] data_len - 接收缓冲区长度
//   [IN] timeout - 超时时间
// 返回值：-1超时，-2失败，0对端close，大于0成功
// 说明：
//接收数据，接收缓冲区为recv_buf,缓冲区长度为recv_buf_len,超过timeout秒仍没接收
//到数据，则返回失败(0-超时，小于0-其他错误，大于0-成功接收)
////////////////////////////////////////////////////////////////////////////////
HB_S32 recv_data(HB_S32 *psockfd, HB_VOID *recv_buf, HB_S32 recv_buf_len, HB_S32 timeout)
{
    struct timeval wait_time = {0, 0};
	HB_S32 retval = 0;
	HB_S32 recvCount = 0;
	HB_S32 ret;
	fd_set fdread;
	wait_time.tv_sec   = timeout;
	wait_time.tv_usec  = 0;

    if(*psockfd <= 0 || recv_buf == NULL || recv_buf_len <= 0)
    {
    	assert(0);
    	return 0;
    }
    FD_ZERO(&fdread);
    FD_SET(*psockfd, &fdread);
	ret = select(*psockfd + 1, &fdread, NULL, NULL, &wait_time);
    if (0 == ret)		// 超时
    {
    	printf("recv select timeout!\n");
        return -1;
    }
    else if (ret < 0)	// 出错
    {
    	printf("recv select error!\n");
        return -2;
    }
    do
    {
        retval = recv(*psockfd, recv_buf, recv_buf_len, 0);
        if (0 == retval)
        {
        	printf("Recv Closed! retval= %d\n", retval);
        	break;
        }
        recvCount += retval;
        usleep(5000);
    }
    while ((retval < 0) && (EINTR == errno));

    return recvCount;
}


////////////////////////////////////////////////////////////////////////////////
// 函数名：close_sockfd
// 描述：关闭tcp网络套接字
// 参数：
//  ［IN］psockfd - 套接字指针。
// 返回值：1
// 说明：
//关闭tcp网络套接字，并初始化为-1
////////////////////////////////////////////////////////////////////////////////
HB_S32 close_sockfd(HB_S32 *sockfd)
{
	if(*sockfd > 0)
	{
		close(*sockfd);
	}
	*sockfd = -1;
	return 1;
}




//返回值：HB_SUCCESS-端口未被占用，HB_FAILURE-端口已占用
HB_S32 check_port(HB_S32 port)
{
	HB_S32 test_sock = -1;
	HB_S32 opt = 1;
	struct sockaddr_in  listen_on_addr;
	test_sock = socket(AF_INET, SOCK_STREAM, 0);//主要是探测端口是否可用，别无所用
	setsockopt(test_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&listen_on_addr, sizeof(listen_on_addr));
	listen_on_addr.sin_family = AF_INET;
	listen_on_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_on_addr.sin_port = htons(port);

	if (0 == bind(test_sock, (struct sockaddr *) &listen_on_addr, sizeof(struct sockaddr)))//绑定成功，说明此端口没有被占用
	{
		close(test_sock);
		return HB_SUCCESS;
	}
	close(test_sock);
	return HB_FAILURE;
}

//根据网卡eth获取相应到ip地址
//返回值：-1获取失败，0获取成功
HB_S32 get_dev_ip(HB_CHAR *eth, HB_CHAR *ipaddr)
{
	HB_S32 sock;
    struct ifreq req;
    struct  sockaddr_in my_addr;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        return -1;
    }
    strncpy(req.ifr_name, eth, IFNAMSIZ);
    if ( ioctl(sock, SIOCGIFADDR, &req) < 0 )
    {
        close(sock);
        return -1;
    }
    memcpy(&my_addr, &req.ifr_addr, sizeof(my_addr));
    strcpy(ipaddr, inet_ntoa(my_addr.sin_addr));
    close(sock);
    return 0;
}

//根据网卡eth获取相应到mac地址
static HB_S32 get_mac_dev(HB_CHAR *mac_sn, HB_CHAR *dev)
{
    struct ifreq tmp;
    HB_S32 sock_mac;
   // HB_CHAR *tmpflag;
    //HB_CHAR mac_addr[30];
    sock_mac = socket(AF_INET, SOCK_STREAM, 0);
    if( sock_mac == -1)
    {
        return -1;
    }
    memset(&tmp,0,sizeof(tmp));
    strncpy(tmp.ifr_name, dev, sizeof(tmp.ifr_name)-1 );
    if( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 )
    {
    	close(sock_mac);
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


//获取盒子到序列号
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

	return 0;
}
