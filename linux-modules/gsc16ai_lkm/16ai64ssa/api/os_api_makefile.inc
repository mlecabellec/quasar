# $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/linux/os_util_makefile.inc $
# $Rev: 40729 $
# $Date: 2017-09-26 18:42:08 -0500 (Tue, 26 Sep 2017) $

# Linux: API Library: make file

.PHONY: all clean default makefile release

GSC_TARGET	= ../lib/gsc_api.a

GSC_FILES	=			\
			gsc_close.o	\
			gsc_init.o	\
			gsc_ioctl.o	\
			gsc_open.o	\
			gsc_read.o	\
			gsc_write.o	\
						\
			os_close.o	\
			os_init.o	\
			os_ioctl.o	\
			os_open.o	\
			os_read.o	\
			os_write.o



ECHO	:= ${shell ls /bin/echo 2>/dev/null | wc -l}
ifeq ("${ECHO}","1")
ECHO	:= /bin/echo
else
ECHO	:= echo
endif



${GSC_TARGET}: ${GSC_FILES}
	@echo ==== Linking: $@
	@ld -r -o $@ ${GSC_FILES}


