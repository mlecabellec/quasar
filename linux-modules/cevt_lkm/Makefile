# Makefile for cevt kernel module
obj-m += cevt.o
cevt-objs := src/cevt.o src/cevtioctl.o

EXTRA_CFLAGS += -I$(PWD)/src

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
