# v1.10

#*=============================================================================*
#* FILE:                  Linux Lowlevel Library Makefile
#*=============================================================================*
#
#      COPYRIGHT (C) 2006 - 2017 BY ABACO SYSTEMS, INC.
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
#   KERNEL=24 - if using a Linux kernel 2.4.x
#  Optional:
#   USB_SUPPORT=1  - enable USB support
#   USB_SERVER_SUPPORT=1  - enable USB support to support two channels
#                           concurrently (R15-USB only)
#   NO_SYSFS=1 - disables support for the SYS file system
#   ARCH= - specify the architecture of the target system
#   32BIT=1 - compiles for 32-bit emulation in a 64-bit system
#   CROSS_COMPILE - specify the toolchain for the target system
#   NO_INSTALL=1 - does not move the library to the /usr/lib(64) directory
#   DEBUG=1 - enables debugging output
#
#==============================================================================*

# NOTE: if using a kernel 2.6.5 or earlier will need to set "-DNO_MSGQ_SUPPORT"
#       to disable message queues since they are not supported in the kernel.

# NOTE: if needing to link to a version of the "libusb" library other than the
#       one located in /usr/lib or /usr/lib64, then uncomment "LIBUSB_PATH" and
#       set the path to the alternative directory (the default is /usr/local/lib).
#LIBUSB_PATH = /usr/local/lib
# NOTE: set the name of the "libusb" library if needed.
LIBUSB = usb-1.0


CC      = ${CROSS_COMPILE}gcc
AR      = ${CROSS_COMPILE}ar
CFLAGS  = -fPIC -c -g -Wall
SRCS    = mem.c config.c ll_posix.c
OBJS    = mem.o config.o ll_posix.o
TLIBS   = -lc -lrt -lpthread
LFLAGS  = -shared -Wl,-soname -Wl,${TARGET}
LIBDIR  = /usr/lib 

ifeq (${USB_SERVER_SUPPORT}, 1)
 override undefine USB_SUPPORT
 USB_SUPPORT = 1
 TARGET = libceill_sc.so
else
 TARGET = libceill.so
endif

ifeq (${KERNEL}, 24)
 CFLAGS += -DKERNEL_24 -DNO_MSGQ_SUPPORT
endif
ifeq (${NO_SYSFS}, 1)
 CFLAGS += -DNO_SYSFS_SUPPORT
endif
ifeq (${DEBUG}, 1)
 CFLAGS += -DLL_DEBUG
endif

MODE = 32
ifeq (${ARCH}, x86_64)
 ifeq (${32BIT}, 1)
  CFLAGS += -D_32BIT -m32 
  LFLAGS += -m32
 else
  CFLAGS += -DLP64 -m64 -mno-red-zone
  LFLAGS += -m64
  LIBDIR  = /usr/lib64
  MODE = 64
 endif
endif

ifeq (${ARCH}, ppc64)
 ifeq (${32BIT}, 1)
  CFLAGS += -D_32BIT -m32 
  LFLAGS += -m32
 else
  CFLAGS += -DLP64 -m64
  LFLAGS += -m64
  LIBDIR  = /usr/lib64
 endif
 # USB not supported with PowerPC
 ifeq (${USB_SUPPORT}, 1)
  exit 1
 endif
endif

ifeq (${USB_SUPPORT}, 1)
 CFLAGS += -DLL_USB
 ifneq (${NO_SYSFS}, 1)
  CFLAGS += -DNO_SYSFS_SUPPORT
 endif
 ifeq (${USB_SERVER_SUPPORT}, 1)
  LIB_CEILL = ceill_usbs${MODE}
  LIB_FW = ceill_7503fw${MODE}
 else 
  LIB_CEILL = ceill_usb${MODE}
  LIB_FW = ceill_7503fw${MODE} -lceill_7505fw${MODE}
 endif
  LFLAGS += -Wl,-L ./USB -Wl,-whole-archive -l${LIB_CEILL} -l${LIB_FW} -Wl,-no-whole-archive
   
 ifneq (${LIBUSB_PATH}, )
  LFLAGS += -Wl,-rpath=${LIBUSB_PATH}
  TLIBS += -L ${LIBUSB_PATH}
 endif
 
 TLIBS += -l${LIBUSB}
endif 

ifneq (${CROSS_COMPILE}, )
 override undefine NO_INSTALL
 NO_INSTALL = 1
endif

#CFLAGS += -D_GNU_SOURCE -D_REENTRANT

ifneq ($(NO_INSTALL), 1)
 INSTALL_CMD = install
endif

all: comp link ${INSTALL_CMD} clean

#ifeq ($(NO_INSTALL), 1)
# all: comp link clean
#else
# all: comp link install clean
#endif	

comp:
	${CC} ${CFLAGS} ${SRCS}

link:
	${CC} ${LFLAGS} -o ${TARGET} ${OBJS} ${TLIBS}
	
install:
	mv ${TARGET} ${LIBDIR}

clean:
	rm -f *.o *~ core
