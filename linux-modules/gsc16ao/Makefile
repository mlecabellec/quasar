# Makefile for gsc16ao kernel module
obj-m += gsc16ao.o
gsc16ao-objs := src/gsc16ao_ioctl.o src/gsc16ao_main.o src/gsc16ao_util.o

EXTRA_CFLAGS += -I$(PWD)/src

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
