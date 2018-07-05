#TYPE?=hisi_100
#TYPE?=hisi_200
TYPE?=hisi_300


ifeq ($(TYPE), hisi_100)
CC:=arm-hisiv100nptl-linux-gcc
STRIP:=arm-hisiv100nptl-linux-strip
CFLAGS = -Wall -O2
LIBFLAGS = -Wl,-rpath /ipnc/lib/
INC_DIR := -I./inc
LIB_DIR := -L./lib/hisi100
LIBS := -levent -levent_pthreads -lxml -lpthread -lsqlite3 -lm -lrt -ldl

DEST_DIR=/mnt/hgfs/nfs_dir/share_dir/hb/A_Box_iptables/bin
APPBIN = box_iptable_100
endif

ifeq ($(TYPE), hisi_200)
CC:=arm-hisiv200-linux-gcc
STRIP:=arm-hisiv200-linux-strip
CFLAGS = -Wall -O2
LIBFLAGS = -Wl,-rpath /mnt/ydt_box/lib
INC_DIR := -I./inc
LIB_DIR := -L./lib/hisi200
LIBS := -lremote_debug -lmd5gen -lsqlite3 -lxml -lpthread -lm -lrt -ldl

DEST_DIR=/mnt/hgfs/nfs_dir/share_dir/hb/A_Box_iptables/bin
APPBIN = box_iptable_200
endif

ifeq ($(TYPE), hisi_300)
CC:=arm-hisiv300-linux-gcc
STRIP:=arm-hisiv300-linux-strip
CFLAGS = -Wall -O2 
LIBFLAGS = -Wl,-rpath /ipnc/lib/
INC_DIR := -I./inc
LIB_DIR := -L./lib/hisi300
LIBS := -levent -levent_pthreads -lxml -lpthread -lsqlite3 -lm -lrt -ldl

DEST_DIR=/mnt/hgfs/nfs_dir/share_dir/hb/A_Box_iptables/bin
APPBIN = box_iptable_300
endif


SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

all:
	$(CC) $(SRCS) $(CFLAGS) $(INC_DIR) $(LIBFLAGS) $(LIB_DIR) $(LIBS) -o $(APPBIN)
	$(STRIP) $(APPBIN)
	cp $(APPBIN) $(DEST_DIR)/$(APPBIN)
clean:
	rm -rf $(OBJS) $(APPBIN)
