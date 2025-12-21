# Makefile for vfx70 kernel module
obj-m += vfx70.o
vfx70-objs := \
    src/devvfx.o \
    src/isr_pmcvfx.o \
    src/vfxioctl.o

EXTRA_CFLAGS += -I$(PWD)/src

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
