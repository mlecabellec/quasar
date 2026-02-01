# v1.06

#*============================================================================*
#* FILE:              BusTools/1553 API Linux Examples Makefile
#*============================================================================*
#
#      COPYRIGHT (C) 2002 - 2015 BY ABACO SYSTEMS, INC.
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
#============================================================================*
#
# Makefile options
#  Required:
#   none
#  Optional:
#   ARCH= - specify the architecture
#   32BIT=1 - builds 32-bit examples to execute on a 64-bit system, only used
#             in conjunction with ARCH=x86_64
#   NO_INSTALL=1 - links to the low level and API library in the indicated
#                   directory paths
#   CROSS_COMPILE= - specify the toolchain compiler for the target system
#   USB_SUPPORT=1 - build examples for the R15-USB
#   USB_SERVER_SUPPORT=1 - build examples for the R15-USB with concurrent two
#                          channel operation
#
#============================================================================*


CC     = ${CROSS_COMPILE}gcc
CFLAGS = -I../Include -I../Source -Wall
TLIB   = -lc -lpthread -lrt -lbusapi

# set to debug
#CFLAGS += -g

ifeq (${ARCH}, x86)
 CFLAGS += -D_LINUX_X86_
endif
ifeq (${ARCH}, x86_64)
 CFLAGS += -D_LINUX_X86_
 ifeq (${32BIT}, 1)
  CFLAGS += -m32
 else
  CFLAGS += -DLP64 -D_64_BIT_ -m64 -mno-red-zone
 endif
endif
ifeq (${ARCH}, ppc)
 CFLAGS += -D_LINUX_PPC_
endif
ifeq (${ARCH}, ppc64)
 CFLAGS += -D_LINUX_PPC_
 ifeq (${32BIT}, 1)
  CFLAGS += -m32
 else
  CFLAGS += -DLP64 -D_64_BIT_ -m64
 endif
endif

ifneq (${CROSS_COMPILE}, )
  NO_INSTALL=1
endif

ifeq (${NO_INSTALL}, 1)
  # link to API and low level libraries in local path
  TLIB += -L./ -Wl,-rpath=./
  # link to API library
  TLIB += -L../Source -Wl,-rpath=../Source
  # link to low level library
  TLIB += -L../../../Lowlevel -Wl,-rpath-link=../../../Lowlevel
endif


APP_NAMES = purge ONE_SHOT STEST TSTALL TSTBC
ifeq (${USB_SERVER_SUPPORT}, 1)
USB_SUPPORT = 1
APP_NAMES += USBSRVR
endif
ifeq (${USB_SUPPORT}, )
APP_NAMES += TSTALL_HWINT
endif


all: ${APP_NAMES} clean

ONE_SHOT:
	${CC} ${CFLAGS} ${TLIB} one_shot.c -o one_shot

PLAYBACK:
	${CC} ${CFLAGS} ${TLIB} pb_test.c -o pbt

STEST:
	${CC} ${CFLAGS} stest.c -o stest

TSTALL:
	${CC} ${CFLAGS} ${TLIB} tst_all.c -o tstall

TSTALL_HWINT:
	${CC} ${CFLAGS} -DTEST_HWINT ${TLIB} tst_all.c -o tstall_hwint

TSTBC:
	${CC} ${CFLAGS} ${TLIB} tst_bc.c -o tstbc

USBSRVR:
	${CC} ${CFLAGS} ${TLIB} -lceill_sc usb_srvr.c -o usb_srvr

purge:
	rm -f ./int ./one_shot ./pbt ./stest ./tstall ./tstbc ./cei1553ud ./tstall_hwint *.o *~ core

clean:
	rm -f *.o *~ core
