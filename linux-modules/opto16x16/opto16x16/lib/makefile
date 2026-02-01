# $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/lib/makefile $
# $Rev: 53727 $
# $Date: 2023-09-14 10:46:13 -0500 (Thu, 14 Sep 2023) $

# OPTO16X16: Library Directory: make file

default: all

TARGET_A	= opto16x16_main.a
TARGET_M	= opto16x16_multi.a

.NOTPARALLEL: %.a

ifeq ("${GSC_LIB_COMP_FLAGS}","")
GSC_LIB_COMP_FLAGS_MSG=
else
GSC_LIB_COMP_FLAGS_MSG="  (added '${GSC_LIB_COMP_FLAGS}')"
endif

ifeq ("${GSC_LIB_LINK_FLAGS}","")
GSC_LIB_LINK_FLAGS_MSG=
else
GSC_LIB_LINK_FLAGS_MSG="  (added '${GSC_LIB_LINK_FLAGS}')"
endif

LIB_FILES_A	=				\
			${LIB_FILES_M}	\
			gsc_utils.a		\
			plx_utils.a		\
			os_utils.a

LIB_FILES_M	=					\
			opto16x16_dsl.a		\
			opto16x16_utils.a

all: ${TARGET_A} ${TARGET_M}
	@echo ==== All Done

release: ${TARGET_A} ${TARGET_M}
	@echo ==== release Done

clean::
	@echo ==== Cleaning ${TARGET_A} ${TARGET_M} ...
	@rm -f ${LIB_FILES_A} ${TARGET_A} ${TARGET_M}

${TARGET_A}: makefile ${LIB_FILES_A}
	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${LIB_FILES_A} ${GSC_LIB_LINK_FLAGS}

${TARGET_M}: makefile ${LIB_FILES_M}
	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${LIB_FILES_M} ${GSC_LIB_LINK_FLAGS}
