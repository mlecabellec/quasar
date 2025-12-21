# Makefile for ec_generic kernel module
obj-m += ec_generic.o
ec_generic-y := src/generic.o

EXTRA_CFLAGS += -I$(PWD)/src

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
