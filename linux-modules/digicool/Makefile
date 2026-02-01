# Makefile for Pcig kernel module
obj-m += Pcig.o
Pcig-objs := src/lenval.o src/micro.o src/pcigSkel.o src/pcigWrappers.o src/ports.o

EXTRA_CFLAGS += -I$(PWD)/src

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
