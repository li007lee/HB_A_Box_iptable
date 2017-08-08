//
//  debug.h
//  Mapport
//
//  Created by MoK on 15/3/19.
//  Copyright (c) 2015ๅนด MoK. All rights reserved.
//

#ifndef __Mapport__debug__
#define __Mapport__debug__

#include <stdio.h>
//#include "loopbuf.h"

void *debug_thread_run(void *param);
void create_debug_thread(void * stru);
void debug_err_sys(const char *fmt, ...);
void inmarktime(int sockpt_num, size_t val);

//unsigned  char kfifo_buff[KFIFO_BUF_SIZE];
//size_t  SPEED_UPDATE_TIME;

#define DEBUG_PRINT(fmt,...)    debug_err_sys((const char *)fmt,## __VA_ARGS__)

#endif /* defined(__Mapport__debug__) */
