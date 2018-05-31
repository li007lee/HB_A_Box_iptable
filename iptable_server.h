/*
 * iptable_server.h
 *
 *  Created on: 2017年8月4日
 *      Author: root
 */

#ifndef IPTABLE_SERVER_H_
#define IPTABLE_SERVER_H_

#define IPTABLE_SERVER_PORT	7879

//启动一个服务，在每次登陆页面时会收到从loginout.cgi发来的连接，此时进行获取验证服务器ip的操作。
HB_VOID *IptableServer(HB_VOID *args);
//获取验证服务器ip和端口
HB_S32 GetStreamInfo();
//获取心跳服务器ip和端口
HB_S32 GetHeartBeatServerInfo();
//获取验证服务器和心跳服务器ip和端口线程
HB_VOID *GetServer(HB_VOID *args);

#endif /* IPTABLE_SERVER_H_ */
