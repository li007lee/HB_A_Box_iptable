LIBA = libmd5gen.a

CC = /root/work/software/arm-hisiv100nptl-linux/arm-hisiv100-linux/bin/arm-hisiv100-linux-uclibcgnueabi-gcc
AR = /root/work/software/arm-hisiv100nptl-linux/arm-hisiv100-linux/bin/arm-hisiv100-linux-uclibcgnueabi-ar

CFLAGS = -Wall -O2
COMPILE = $(CC) $(CFLAGS) -c
INSTALL	 = install -c


SRCS = $(wildcard *.c) 
OBJS = $(patsubst %.c,%.o,$(SRCS))


all:$(LIBA)  
$(LIBA):$(OBJS)  
	$(AR) -crs $(LIBA) $(OBJS)
	$(INSTALL) $(LIBA) ../lib
	
%.o:%.c  
	$(COMPILE) $< -o $@  
	
	
	
clean:  
	rm -f $(OBJS) $(LIBA) 