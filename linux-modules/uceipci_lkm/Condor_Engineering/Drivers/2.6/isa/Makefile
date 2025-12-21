# v1.03

#*============================================================================*
#* FILE:                 UCEIISA Device Driver Makefile
#*============================================================================*
#
#      COPYRIGHT (C) 2005 - 2008 BY ABACO SYSTEMS, INC.
#      ALL RIGHTS RESERVED.
#
#      THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
#      COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
#      THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
#      OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
#      AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
#      SOFTWARE IS HEREBY TRANSFERRED.
#
#      THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
#      NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ABACO SYSTEMS.
#
#=============================================================================*
#
# Makefile options
#  Required:
#   NONE
#  Optional:
#   MODULE_PATH= - specify the path of the target system's kernel source
#                    for building the device driver
#   NO_HW_INTERRUPT=1 - disables support for hardware interrupts
#   NO_HW_INTERRUPT_SIGNAL=1 - disables support for using POSIX RT signals
#                              to notify the API when a hardware interrupt occurs
#
#=============================================================================*


CEI_MODULE = uceiisa

ifeq (${MODULE_PATH},)
  MODULE_PATH = /lib/modules/$(shell uname -r)/build
endif

ifeq (${NO_HW_INTERRUPT}, 1)
  CFLAGS_${CEI_MODULE}.o += -DNO_HW_INTERRUPTS
endif
ifeq (${NO_HW_INTERRUPT_SIGNAL},)
  # use POSIX RT signals in the ISR to notify the API that a hardware interrupt occured
  CFLAGS_${CEI_MODULE}.o += -DHW_INTERRUPTS_SIGNAL
endif


ifneq ($(KERNELRELEASE),)
	obj-m := ${CEI_MODULE}.o
else
	KDIR := ${MODULE_PATH}
	PWD  := $(shell pwd)

all: build install clean

build:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

install:
#	chmod 744 ./${CEI_MODULE}.ko
#	cp ${CEI_MODULE}.ko ${MODULE_DIR}
endif

clean:
	$(RM) *.o *.mod.c *.cmd
