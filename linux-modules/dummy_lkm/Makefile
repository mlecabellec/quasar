# Makefile for dummy kernel module
obj-m += dummy.o
dummy-objs := src/export.o src/initexit.o src/main.o

EXTRA_CFLAGS += -I$(PWD)/src

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
