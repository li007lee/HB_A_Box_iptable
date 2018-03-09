/*
 * my_include.h
 *
 *  Created on: 2014年12月11日
 *      Author: root
 */

#ifndef MY_INCLUDE_H_
#define MY_INCLUDE_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/prctl.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <memory.h>
#include <linux/rtc.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <ctype.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/shm.h>


///////////////////////////////////////////////////////////////////////////////////////////
//数据类型定义
///////////////////////////////////////////////////////////////////////////////////////////
typedef unsigned char              HB_U8;
typedef unsigned short            HB_U16;
typedef unsigned int                 HB_U32;
typedef unsigned long long    HB_U64;
typedef signed char                   HB_S8;
typedef short                                HB_S16;
typedef int                                     HB_S32;
typedef long long                        HB_S64;
typedef char                                 HB_CHAR;
typedef float                                 HB_FLOAT;
typedef void                                  HB_VOID;
typedef void *                               HB_HANDLE;
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
//测试服务器
//#define PT_ADDR_IP  "testaegisci.hbydt.cn"
//#define PT_ADDR_IP  "aegiscitest.hbydt.cn"
//#define PT_PORT     80

#define MAX_ERR_CRITICAL	10
#define MAX_ERR_TIMES		3

#define IP_LIST_MAX		5
#define IP_ADDR_NAME	32
#define IP_LEN_MAX	16
#define PORT_LEN	8

#define KEY	0xabcd0acd21ec //用于生成机器码时隐藏真是MAC地址（异或时使用）


#ifdef SMALL_BOX

#define DOUBLE_NET_PORT //定义双网口版本
#define AGRICULTURE //定义农业版本

#define LED_CTRL_SH_PATH	"led_ctrl.sh"
#define KILL_LED_CTRL_SH_PATH	"killall -9 led_ctrl.sh"
#define BOX_VERSION_FILE "/ipnc/config/box_version"
#define BOX_DATA_BASE_NAME    "/home/default/TM_X01_Data.db"
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


typedef enum
{
	NETWORK_ANOMALY,			//网络异常
	NETWORK_REGULAR,
	NO_CONNECT_HF_SRV,			//连接汉邦服务器异常
	DEVICE_MEMORY_FAILED,		//内存创建文件失败
	DEVICE_REGIST_ODD,			//设备注册异常
	DEVICE_REGIST_TOKEN,		//获取令牌异常
	API_CALLBACK_ERR,			//api接口调用异常
	YDT_SERVER_ANOMALY,			//一点通服务异常
	YDT_CHECKIN_ERR,			//一点通密码校验错
	YDT_SERVER_BREAK,			//一点通服务退出
	DEVICE_LOOP_ERR,			//设备自环异常
	DEVICE_MEMFILE_ERR,			//内存创建文件异常
	ERR_DEFINE_END
}enum_err_define;


typedef enum
{
    AEGIS_SERVER, //维护服务器类型
    REGIST_SERVER, //注册服务器类型
    VIDEO_SWITCH_SERVER, //视频转发服务器
    VIDEO_ABSTRACT_SERVER, //视频摘要服务器
    SERVER_IP_CUT //服务器类型截取
}enum_server_type;



typedef struct
{
	HB_CHAR addname[IP_ADDR_NAME];
    HB_CHAR ip[IP_LEN_MAX];
    HB_CHAR port[PORT_LEN];
}stru_ip_addr;

typedef struct
{
	HB_S32 num;
    HB_CHAR ip[IP_LIST_MAX][IP_LEN_MAX];
    HB_CHAR port[IP_LIST_MAX][PORT_LEN];
}SERVER_INFO_STRUCT;


typedef struct
{
	HB_CHAR machine_code[32];
}GLOBLE_MSG_STRUCT;


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
