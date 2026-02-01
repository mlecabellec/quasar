# $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/linux/os_util_makefile.inc $
# $Rev: 53059 $
# $Date: 2023-06-01 08:24:17 -0500 (Thu, 01 Jun 2023) $

# Linux: Utility: Common makefile

.PHONY: all clean default makefile release

ECHO	:= ${shell ls -d ../lib 2>/dev/null | wc -l}
ifeq ("${ECHO}","1")
OS_TARGET			= ../lib/os_utils.a
else
OS_TARGET			= os_utils.a
endif

OS_OBJ_FILES		:=							\
					./os_util_count.o			\
					./os_util_id.o				\
					./os_util_kbd.o				\
					./os_util_sem.o				\
					./os_util_sleep.o			\
					./os_util_thread.o			\
					./os_util_time.o



ECHO	:= ${shell ls /bin/echo 2>/dev/null | wc -l}
ifeq ("${ECHO}","1")
ECHO	:= /bin/echo
else
ECHO	:= echo
endif



${OS_TARGET}: ${OS_OBJ_FILES}
	@echo ==== Linking: $@ ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${OS_OBJ_FILES} ${GSC_LIB_LINK_FLAGS}


