/*
 * ctrl_led.h
 *
 *  Created on: 2017年8月1日
 *      Author: root
 */

#ifndef CTRL_LED_H_
#define CTRL_LED_H_

typedef struct _tagDOMAIN_PARSE_ARG
{
	char domain[256];
	int pipe_fd;
}DOMAIN_PARSE_ARG_OBJ, *DOMAIN_PARSE_ARG_HANDLE;


typedef enum _LED_STATUS {
	RED_ON = 1, //红灯常亮
	GREEN_ON,	//绿灯常亮
	RED_FLICKER, //红灯闪
	GREEN_FLICKER //绿灯闪
}LED_STATUS;

//盒子信号灯控制线程
HB_VOID * CtrlLed(void *arg);

#endif /* CTRL_LED_H_ */
