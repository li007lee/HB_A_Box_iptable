/*
 * ctrl_led.c
 *
 *  Created on: 2017年8月1日
 *      Author: root
 */

#include "my_include.h"
#include "ctrl_led.h"
#include "net_api.h"
#include "get_set_config.h"
#include "common_args.h"

#ifdef DOUBLE_NET_PORT
HB_VOID * CtrlLed(void *arg)
{
	pthread_detach(pthread_self());
	LED_STATUS cur_status = 0;
	HB_S32	ret = 0;

	while(1)
	{
		if (glCommonArgs.iWanOpenFlag == 0)
		{
			//如果广域网未开启，直接返回成功，无需检测网络
			if (get_ps_status("ps | grep led_ctrl.sh | grep -v grep | wc -l") > 0)
			{
				system(KILL_LED_CTRL_SH_PATH);
			}

			if (cur_status != GREEN_ON)
			{
				system(LED_CTRL_SH_PATH" green_on &");
				cur_status = GREEN_ON;
			}
			sleep(10);
			continue;
		}

		HB_CHAR srv_ip[16] = {0};
		//判断网络状态
		ret = from_domain_to_ip(srv_ip, "www.baidu.com", 5);
		if (ret < 0)
		{
			ret = from_domain_to_ip(srv_ip, "www.taobao.com", 5);
			if (ret < 0)
			{
				//网络异常(红灯闪)
				if ((get_ps_status("ps | grep led_ctrl.sh | grep -v grep | wc -l") > 0) && (cur_status != RED_FLICKER))
				{
					system(KILL_LED_CTRL_SH_PATH);
				}

				if (cur_status != RED_FLICKER)
				{
					system(LED_CTRL_SH_PATH" red_flicker &");
					cur_status = RED_FLICKER;
				}
			}
			sleep(10);
			continue;
		}

		//判断天联状态
		ret = get_sys_gnLan();
		if (0 == ret) //gnLan没有登陆
		{
			printf("天联未启动");
			//点灯(绿灯闪)
			if ((get_ps_status("ps | grep led_ctrl.sh | grep -v grep | wc -l") > 0) && (cur_status != GREEN_FLICKER))
			{
				system(KILL_LED_CTRL_SH_PATH);
			}

			if (cur_status != GREEN_FLICKER)
			{
				system(LED_CTRL_SH_PATH" green_flicker &");
				cur_status = GREEN_FLICKER;
			}

			sleep(10);
			continue;
		}

		ret = test_gnLan_alive();
		if (ret < 0)
		{
			//天联已经启动，但是不通服务器(绿灯闪)
			if ((get_ps_status("ps | grep led_ctrl.sh | grep -v grep | wc -l") > 0) && (cur_status != GREEN_FLICKER))
			{
				system(KILL_LED_CTRL_SH_PATH);
			}

			if (cur_status != GREEN_FLICKER)
			{
				system(LED_CTRL_SH_PATH" green_flicker &");
				cur_status = GREEN_FLICKER;
			}

			sleep(10);
			continue;
		}

		//设备运行正常(绿灯常亮)
		if (get_ps_status("ps | grep led_ctrl.sh | grep -v grep | wc -l") > 0)
		{
			system(KILL_LED_CTRL_SH_PATH);
		}

		if (cur_status != GREEN_ON)
		{
			system(LED_CTRL_SH_PATH" green_on &");
			cur_status = GREEN_ON;
		}
		sleep(300);//如果盒子正常5分钟轮询一次
	}

	pthread_exit(NULL);
}
#endif
