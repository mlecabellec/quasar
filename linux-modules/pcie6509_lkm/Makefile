# Makefile for pcie6509 kernel module
obj-m += pcie6509.o
pcie6509-objs := \
    src/pcie6509_din.o \
    src/pcie6509_dio.o \
    src/pcie6509_dout.o \
    src/pcie6509_loopback.o \
    src/pcie6509_reg.o \
    src/pcie6509_tst_lib.o \
    src/pcie6509_tst.o \
    src/pcie6509.o

EXTRA_CFLAGS += -I$(PWD)/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
