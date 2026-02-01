# Makefile for pcie6509_simusol kernel module
obj-m += pcie6509_simusol.o
pcie6509_simusol-objs := \
    src/accesPorts.o \
    src/logMsg.o \
    src/pcie6509init.o \
    src/pcie6509ioctl.o

EXTRA_CFLAGS += -I$(PWD)/src

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
