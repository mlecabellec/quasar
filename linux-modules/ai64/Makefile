# Makefile for ai64 kernel module

obj-m := ai64.o
ai64-y := src/ai64.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
