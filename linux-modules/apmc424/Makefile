# Makefile for apmc424 kernel module
obj-m += apmc424.o

apmc424-y := src/pmc424/dev424/dev424.o

EXTRA_CFLAGS += -I$(PWD)/src/pmc424 -I$(PWD)/src/pmccommon

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
