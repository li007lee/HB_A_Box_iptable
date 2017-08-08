/*
 * hf_plant_net.h
 *
 *  Created on: 2015Äê1ÔÂ5ÈÕ
 *      Author: root
 */

#ifndef HF_PLANT_NET_H_
#define HF_PLANT_NET_H_

int init_socket2platform(int *psockfd, enum_server_type server_types);

int send_to_pt(int *sockfd, void *buf, size_t n, int waitsec);

int sock_recv(int *sockfd, char * buf, int len, int waitsec);

int close_sockfd(int *sockfd);


#endif /* HF_PLANT_NET_H_ */
