# v1.01

#*===========================================================================*
#* FILE:                 CEIPCC1553 Client Driver Makefile
#*===========================================================================*
#
#      COPYRIGHT (C) 2005 - 2007 BY ABACO SYSTEMS, INC.
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
#===========================================================================*

CC          = gcc
CFLAGS      = -D__KERNEL__ -DMODULE -pipe -I/lib/modules/`uname -r`/build/include 
CLIENT      = cei_pcc1553
CLIENT_DRV  = ${CLIENT}_cs
TARGET      = ${CLIENT_DRV}.o
MODULE_DIR  = "/lib/modules/`uname -r`/pcmcia"
PCMCIA_DIR  = "/etc/pcmcia"


ifeq ($(NO_INSTALL), 1)
all:	build clean
else
all:	build clean install
endif

build:
	${CC} ${CFLAGS} -MD -c ${CLIENT_DRV}.c -o ${TARGET}

install:
	$(shell mkdir -p ${MODULE_DIR})
	chmod 774 ./${TARGET}
	mv -f ./${TARGET} ${MODULE_DIR}/${TARGET}
	chmod 755 ./${CLIENT}
	cp -f ./${CLIENT} ${PCMCIA_DIR}/${CLIENT}
	chmod 644 ./${CLIENT}.conf
	cp -f ./${CLIENT}.conf ${PCMCIA_DIR}/${CLIENT}.conf

clean:
	rm -f core core.* ${CLIENT_DRV}.d	

