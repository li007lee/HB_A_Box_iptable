/*
 * my_include.h
 *
 *  Created on: 2014年12月11日
 *      Author: root
 */

#ifndef MY_INCLUDE_H_
#define MY_INCLUDE_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <termios.h>
#include <semaphore.h>
#include <errno.h>
#include <resolv.h>
#include <dirent.h>
#include <stdarg.h>
#include <memory.h>
#include <time.h>
#include <netdb.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <linux/rtc.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>



///////////////////////////////////////////////////////////////////////////////////////////
//数据类型定义
///////////////////////////////////////////////////////////////////////////////////////////
typedef unsigned char	HB_U8;
typedef unsigned short	HB_U16;
typedef unsigned int	HB_U32;
typedef unsigned long long	HB_U64;
typedef signed char	HB_S8;
typedef short	HB_S16;
typedef int		HB_S32;
typedef long long	HB_S64;
typedef char	HB_CHAR;
typedef float	HB_FLOAT;
typedef void	HB_VOID;
typedef void *	HB_HANDLE;
typedef enum _tagHB_BOOL
{
    HB_FALSE = 0,
    HB_TRUE  = 1
}HB_BOOL;

#define SMALL_BOX
//#define BIG_BOX_SINGLE_PORT
//#define BIG_BOX_MULTIPLE_PORT
//#define BIG_BOX_ELEVATOR


#ifndef NULL
#define NULL  0L
#endif

#define HB_NULL     0L
#define HB_SUCCESS  0
#define HB_FAILURE  -1

//生产服务器
#define PT_ADDR_IP  "aegisci.ivview.com"
#define PT_PORT     80
#define HB_ALARM_SERVER_IP "alarm.hbydt.cn"
#define HB_ALARM_SERVER_PORT 8088
//测试服务器
//#define PT_ADDR_IP  "testaegisci.hbydt.cn"
//#define PT_ADDR_IP  "aegiscitest.hbydt.cn"
//#define PT_PORT     80

#define BOX_INFO_COLLECT_INTERVAL	60//盒子信息采集时间间隔 单位 秒
#define DEFAULT_UPLOAD_INTERVAL	300	//盒子信息默认上报时间间隔	单位 秒

#define MAX_ERR_CRITICAL	10
#define MAX_ERR_TIMES		3

#define IP_LIST_MAX		5
#define IP_LEN_MAX	16
#define PORT_LEN	8

#define KEY	0xabcd0acd21ec //用于生成机器码时隐藏真是MAC地址（异或时使用）
//#define KEY	0xabcd0a //用于生成机器码时隐藏真是MAC地址（异或时使用）


#ifdef SMALL_BOX

#define DOUBLE_NET_PORT //定义双网口版本

#define LED_CTRL_SH_PATH	"led_ctrl.sh"
#define KILL_LED_CTRL_SH_PATH	"killall -9 led_ctrl.sh"
#define BOX_VERSION_FILE "/ipnc/config/box_version"
#define BOX_DATA_BASE_NAME    "/home/default/TM_X01_Data.db"
#define KILL_LED_CTRL_SH	"killall -9 led_ctrl.sh"
#define KILL_HEARTBEAT_CLIENT "killall -9 easycamera"
#define EASYCAMERA_XML "/home/default/easycamera.xml"
#define START_HEARTBEAT_CLIENT	"/ipnc/ydt/easycamera -c /home/default/easycamera.xml > /dev/null &"
//#define START_HEARTBEAT_CLIENT	"/tmp/nfs_dir/share_dir/hb/EasyDarwin-easycamera/easycamera -c /home/default/easycamera.xml > /dev/null &"
#define SRV_IP_LIST	"/tmp/srv_ip_list.conf"
#define TEAMLINK_CONF_FILENAME "/tmp/stun_client.conf"
#define NTPDATE_FILEPATH	"/ipnc/ydt/ntpdate"
#define GNLAN_START_CMD	"/ipnc/ydt/gnLan -f /tmp/stun_client.conf > /dev/null &"
#define ETHX  "eth0"
#define BIN_PATH	"/ipnc/ydt/"

#endif


#ifdef BIG_BOX_SINGLE_PORT

#define BIG_BOX
#define BOX_VERSION_FILE "/mnt/ydt_box/box_version"
#define BOX_DATA_BASE_NAME    "/mnt/mtd/ydt_box/default/TM_X02_Data.db"
#define SRV_IP_LIST		"/tmp/srv_ip_list.conf"
#define TEAMLINK_CONF_FILENAME "/mnt/mtd/ydt_box/default/stun_client.conf"
#define GNLAN_START_CMD	"gnLan -f /mnt/mtd/ydt_box/default/stun_client.conf > /dev/null &"
#define BIN_PATH	"/mnt/ydt_box/bin/"
#define ETHX  "eth1"

#endif


#ifdef BIG_BOX_MULTIPLE_PORT

#define BIG_BOX
#define BOX_VERSION_FILE "/mnt/ydt_box/box_version"
#define BOX_DATA_BASE_NAME    "/mnt/mtd/ydt_box/default/TM_X03_Data.db"
#define SRV_IP_LIST		"/tmp/srv_ip_list.conf"
#define TEAMLINK_CONF_FILENAME "/mnt/mtd/ydt_box/default/stun_client.conf"
#define GNLAN_START_CMD	"gnLan -f /mnt/mtd/ydt_box/default/stun_client.conf > /dev/null &"
#define BIN_PATH	"/mnt/ydt_box/bin/"
#define ETHX  "eth1"

#endif

#ifdef BIG_BOX_ELEVATOR
#define BOX_VERSION_FILE "/ipnc/default/box_version"
#define BOX_DATA_BASE_NAME    "/home/default/TM_X01_Data.db"
#define SRV_IP_LIST	"/tmp/srv_ip_list.conf"
#define TEAMLINK_CONF_FILENAME "/mnt/mtd/stun_client.conf"
#define GNLAN_START_CMD	"gnLan -f /mnt/mtd/stun_client.conf > /dev/null &"
#define ETHX  "eth0"
#define BIN_PATH	"/ipnc/ydt/"

#endif


typedef struct
{
	HB_S32 num;
    HB_CHAR ip[IP_LIST_MAX][IP_LEN_MAX];
    HB_CHAR port[IP_LIST_MAX][PORT_LEN];
}SERVER_INFO_STRUCT;


/////////////////////////////////////////////////////////////////////////////////
// 路由设备注册信息结构体
/////////////////////////////////////////////////////////////////////////////////
typedef struct _tagROUTE_MSG_REGIST
{
	HB_CHAR ietype[16];
	HB_CHAR sn_number[48]; //盒子序列号
}ROUTE_MSG_REGIST_OBJ, *ROUTE_MSG_REGIST_HANDLE;


typedef struct _tagYDT_TOKEN//一点通令牌结构体
{
    HB_S32 tokenid;
    HB_CHAR tokenname[64];
    HB_CHAR tokenpassword[64];
    HB_CHAR tokenip[16];
    HB_S32 groupid;
    HB_S32 userid;
}YDT_TOKEN_OBJ, *YDT_TOKEN_HANDLE;


typedef struct _tagDEV_PLAT_MESSAGE
{
    HB_U32 deviceid;
    HB_U32 deviceattr;		//迭代倍数	默认值为3，详见时间间隔要求
    HB_U32 checkinterval;  //时间间隔	默认值为1-10分钟内的随机值
    HB_U32 radixval;
	HB_U32 return_from_ipc;
	HB_U32 gnlan_flag;   //天联登录成功标志，登录成功:1 登录失败:0
    HB_U32 return_regist; //设备注册成功标志，注册成功:1 注册失败:0
    HB_U32 return_stream; //设备获取流媒体信息成功标志，成功:1 失败:0
    HB_U32 return_heart_beat; //设备获取流媒体信息成功标志，成功:1 失败:0
    HB_U32 return_token;
    HB_U32 return_push;
    ROUTE_MSG_REGIST_OBJ route_regist;
    //stru_device_msg_regist stru_regist;
    YDT_TOKEN_OBJ stru_token;
}DEV_PLAT_MESSAGE_OBJ, *DEV_PLAT_MESSAGE_HANDLE;



#define DEBUG
#ifdef DEBUG
#define COLOR_STR_NONE          "\033[0m"
#define COLOR_STR_RED              "\033[1;31m"
#define COLOR_STR_GREEN         "\033[1;32m"
#define COLOR_STR_YELLOW      "\033[1;33m"
#define COLOR_STR_BLUE		     "\033[0;32;34m"

#define TRACE_LOG(str, args...)  printf(COLOR_STR_GREEN  "\n########   FILE:%s  FUNCTION: %s, "str COLOR_STR_NONE,__FILE__, __FUNCTION__,## args);
#define TRACE_ERR(str, args...)   printf(COLOR_STR_RED "\n########   FILE:%s  FUNCTION: %s, "str COLOR_STR_NONE,__FILE__, __FUNCTION__,## args);
#define TRACE_DBG(str, args...)  printf(COLOR_STR_YELLOW  str COLOR_STR_NONE, ## args);
#else
#define TRACE_LOG(str, args...)   do{} while(0)
#define TRACE_ERR(str, args...)    do{} while(0)
#define TRACE_DBG(str, args...)   do{} while(0)
#endif /* ERR_DEBUG */


#endif /* MY_INCLUDE_H_ */
