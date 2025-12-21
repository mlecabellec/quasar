# v1.03

#*=============================================================================*
#* FILE:                 CEIPCC1553 Client Driver Makefile
#*=============================================================================*
#
#      COPYRIGHT (C) 2005 - 2010 BY ABACO SYSTEMS, INC.
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
#   NO_SYSFS=1 - disable support for the SYS file system
#   NO_INSTALL=1 - does not copy the client driver and associated files into
#                   the identified directory
#
#==============================================================================*


CLIENT_DRV = cei_pcc1553_cs
TARGET = ${CLIENT_DRV}.o
PCMCIA_DIR = /etc/pcmcia
MODULE_DIR = /lib/modules/`uname -r`

# file permissions for the device driver
#DRV_PERM = 744

# if the kernel is 2.6.11 or later, then the low level will use sysfs instead of the ceidev.conf file
ifeq (${NO_SYSFS}, 1)
  CFLAGS_${TARGET} += -DNO_SYSFS_SUPPORT
endif

ifneq (${KERNELRELEASE},)
	obj-m := ${TARGET}
else
	KDIR := ${MODULE_DIR}/build
	PWD  := $(shell pwd)

ifeq ($(NO_INSTALL), 1)
all:	build clean
else
all:	build clean install
endif

build:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 

# copy necessary PCC1553 client files
install:
#	chmod ${DRV_PERM} ./${CLIENT}.ko
	$(shell mkdir -p ${MODULE_DIR}/pcmcia)
	mv ./${CLIENT_DRV}.ko ${MODULE_DIR}/pcmcia/${TARGET}
	chmod 755 ./${CLIENT_DRV}
	cp -f ./${CLIENT_DRV} ${PCMCIA_DIR}/${CLIENT_DRV}
	chmod 644 ./${CLIENT_DRV}.conf
	cp -f ./${CLIENT_DRV}.conf ${PCMCIA_DIR}/${CLIENT_DRV}.conf

clean:
	rm -f *.o *.mod.c *.d
endif	
