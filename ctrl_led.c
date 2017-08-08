/*
 * ctrl_led.c
 *
 *  Created on: 2017年8月1日
 *      Author: root
 */

#include "my_include.h"
#include "ctrl_led.h"
#include "get_set_config.h"

static void *parse_domain_task(void *param)
{
	int ret = 0;
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    struct sockaddr_in *addr;
    char srv_ip[16] = {0};
    char ipbuf[16] = {0};
    DOMAIN_PARSE_ARG_HANDLE parse_arg = (DOMAIN_PARSE_ARG_HANDLE)param;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Allow IPv4 */
    hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;

    printf("\n######  begin parse domain:[%s]\n", parse_arg->domain);

    ret = getaddrinfo(parse_arg->domain, NULL,&hints,&res);
    if (ret < 0)
    {
        perror("#####  parse err getaddrinfo");
        return NULL;
    }

    for (cur = res; cur != NULL; cur = cur->ai_next)
    {
        addr = (struct sockaddr_in *)cur->ai_addr;
        sprintf(srv_ip, "%s", inet_ntop(AF_INET, &addr->sin_addr, ipbuf, 16));
        ret = write(parse_arg->pipe_fd, srv_ip, strlen(srv_ip));
        break;
    }

    freeaddrinfo(res);
    return NULL;
}

//通过域名解析出相应的ip，超过timeout秒解析不出来，则返回失败-1，成功返回值大于0
int from_domain_to_ip(char *srv_ip, char *srv_domain, int timeout)
{
	int ret = 0;
	int fd[2] = {0};
	char recv_buf[32] = {0};
	DOMAIN_PARSE_ARG_OBJ domain_arg;
	memset(&domain_arg, 0, sizeof(DOMAIN_PARSE_ARG_OBJ));
	struct timeval tval;
	fd_set rset;

	ret = pipe(fd);
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
	ret = pthread_create(&parse_domain_pthread_id, &attr, parse_domain_task, (void *)(&domain_arg));
	pthread_attr_destroy(&attr);
	ret = select(fd[0] + 1, &rset, NULL, NULL, &tval);
    if (0 == ret) // timeout
    {
    	close(fd[0]);
    	close(fd[1]);
        printf("#############time_out!\n");
        return -1;
    }
    else if (ret < 0)
    {
    	close(fd[0]);
    	close(fd[1]);
        printf("#############select error !!\n");
        return -1;
    }
    else
    {
        if (FD_ISSET(fd[0], &rset))
        {
            ret = read(fd[0], recv_buf, sizeof(recv_buf));
            TRACE_DBG("\n############parse domain [%s] to ip [%s]\n", srv_domain, recv_buf);
            strcpy(srv_ip, recv_buf);
        }
    	close(fd[0]);
    	close(fd[1]);
    }
    return 0;
}



HB_VOID * CtrlLed(void *arg)
{
	pthread_detach(pthread_self());
	LED_STATUS cur_status = 0;
	HB_S32	ret = 0;

	while(1)
	{
		HB_S32 flag_wan = 0;
		flag_wan = GetWanConnectionStatus();
		if (flag_wan == 0)
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
