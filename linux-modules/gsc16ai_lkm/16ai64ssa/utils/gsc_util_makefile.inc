# $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/gsc_util_makefile.inc $
# $Rev: 53926 $
# $Date: 2023-11-28 16:56:16 -0600 (Tue, 28 Nov 2023) $

# Linux: Utility: Common makefile

.PHONY: all clean default makefile release

ECHO	:= ${shell ls /bin/echo 2>/dev/null | wc -l}
ifeq ("${ECHO}","1")
ECHO	:= /bin/echo
else
ECHO	:= echo
endif

GSC_TARGET		= ../lib/gsc_utils.a

GSC_OBJ_FILES	:=							\
				./gsc_util_args.o			\
				./gsc_util_buf_man.o		\
				./gsc_util_label.o			\
				./gsc_util_reg.o			\
				./gsc_util_time.o

${GSC_TARGET}: ${GSC_OBJ_FILES}
	@echo ==== Linking: $@ ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${GSC_OBJ_FILES} ${GSC_LIB_LINK_FLAGS}
