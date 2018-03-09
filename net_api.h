//
//  thread_depend.h
//  MapPort
//
//  Created by MoK on 15/1/21.
//  Copyright (c) 2015ๅนด MoK. All rights reserved.
//

#ifndef __MapPort__thread_depend__
#define __MapPort__thread_depend__

//通过域名解析出相应的ip，超过timeout秒解析不出来，则返回失败-1，成功返回值大于0
HB_S32 from_domain_to_ip(HB_CHAR *srv_ip, HB_CHAR *srv_domain, HB_S32 timeout);
//通过域名和端口创建socket并且连接到对端，timeout秒后仍然没有连接上，则返回失败-1，成功返回0
HB_S32 create_socket_connect_domain(HB_S32 *psockfd, HB_CHAR *domain, HB_S32 domain_port, HB_S32 timeout);
//通过IP和端口创建socket并且连接到对端，timeout秒后仍然没有连接上，则返回失败
HB_S32 create_socket_connect_ipaddr(HB_S32 *psockfd, HB_CHAR *ipaddr, HB_S32 port, HB_S32 timeout);
//发送指定长度datalen的数据，如果timeout秒后还为发送，则返回超时失败-1，否则返回发送剩余的字节数，成功表示全部发送完成则返回0，否则失败返回非0
HB_S32 send_data(HB_S32 *psockfd, HB_VOID *send_buf, HB_S32 data_len, HB_S32 timeout);
//接收数据，接收缓冲区为recv_buf,缓冲区长度为recv_buf_len,超过timeout秒仍没接收到数据，则返回失败(-1超时，-2失败，0对端close，大于0成功)
HB_S32 recv_data(HB_S32 *psockfd, HB_VOID *recv_buf, HB_S32 recv_buf_len, HB_S32 timeout);
//关闭tcp网络套接字，并初始化为-1
HB_S32 close_sockfd(HB_S32 *sockfd);
HB_S32 setup_listen_socket(HB_U16 port);
HB_S32 check_port(HB_S32 port);
HB_S32 get_dev_ip(HB_CHAR *eth, HB_CHAR *ipaddr);
HB_S32 get_dev_mac(HB_CHAR *eth, HB_CHAR *mac);
//获取MAC
HB_U64 get_sys_mac();
//获取盒子序列号
HB_S32 get_sys_sn(HB_CHAR *sn, HB_S32 sn_size);


#endif /* defined(__MapPort__thread_depend__) */
