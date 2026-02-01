# Makefile for asdc kernel module
obj-m += asdc.o
asdc-objs := src/asdc.o src/asdcalloc.o src/asdcioctl.o src/asdcwq.o

EXTRA_CFLAGS += -I$(PWD)/src

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
