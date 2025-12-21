# Makefile for ai64ll kernel module

obj-m := ai64ll.o
ai64ll-y := src/ai64ll.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
