#v1.03

#*===========================================================================*
#* FILE:                 UCEIPCI Device Driver Makefile
#*===========================================================================*
#
#      COPYRIGHT (C) 2005 - 2011 BY ABACO SYSTEMS, INC.
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

CC = gcc
TARGET = uceipci.o
CFLAGS = -D__KERNEL__ -DMODULE -DNEW_CONDOR_PCI -Wall -g -O2 -I/lib/modules/`uname -r`/build/include
#-Wstrict-prototypes -fomit-frame-pointer -fno-strength-reduce -DCPU=386
MODULE_PATH = /lib/modules/`uname -r`/Condor_Modules

# remove if not required
ifeq ($(shell getconf LONG_BIT), 64)
 CFLAGS += -mcmodel=kernel
endif


all: .depend $(TARGET) clean
	
install:
	# create the Condor_Modules directory if non-existant
#	$(shell mkdir -p ${MODULE_PATH})
#	mv -f $(TARGET) ${MODULE_PATH}/$(TARGET)

clean:
	rm -f *~ core .depend

.depend:
	$(CC) $(CFLAGS) -M *.c > $@


ifeq (.depend,$(wildcard .depend))
 include .depend
endif
