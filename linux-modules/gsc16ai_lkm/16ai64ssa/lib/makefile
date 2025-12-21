# $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/lib/makefile $
# $Rev: 54955 $
# $Date: 2024-08-07 15:25:47 -0500 (Wed, 07 Aug 2024) $

# 16AI64SSA: Library Directory: make file

default: all

DEV				= 16ai64ssa

TARGET_A		= ${DEV}_main.a
TARGET_M		= ${DEV}_multi.a

TARGET_A_SO		= lib${DEV}_main.so
TARGET_M_SO		= lib${DEV}_multi.so
TARGET_ALL_SO	= lib${DEV}_all.so

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

LIB_FILES_ALL	=				\
				${LIB_FILES_A}	\
				${DEV}_api.a

LIB_FILES_A		=				\
				${LIB_FILES_M}	\
				gsc_utils.a		\
				plx_utils.a		\
				os_utils.a

LIB_FILES_M		=					\
				${DEV}_dsl.a		\
				${DEV}_utils.a

all: ${TARGET_A} ${TARGET_M}
	@echo ==== All Done

release: ${TARGET_A} ${TARGET_M}
	@echo ==== release Done

shared: ${TARGET_A_SO} ${TARGET_M_SO} ${TARGET_ALL_SO}
	@echo ==== shared Done

clean::
	@echo ==== Cleaning ${TARGET_A} ${TARGET_M} ...
	@rm -f ${LIB_FILES_A} ${TARGET_A} ${TARGET_M} ${TARGET_A_SO} ${TARGET_M_SO} ${TARGET_ALL_SO}

${TARGET_A}: ${LIB_FILES_A}
	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${LIB_FILES_A} ${GSC_LIB_LINK_FLAGS}

${TARGET_M}: ${LIB_FILES_M}
	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${LIB_FILES_M} ${GSC_LIB_LINK_FLAGS}

${TARGET_A_SO}: ${LIB_FILES_A}
	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@gcc -shared -o ${TARGET_A_SO} -Wl,--whole-archive ${TARGET_A} -Wl,--no-whole-archive

${TARGET_M_SO}: ${LIB_FILES_M}
	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@gcc -shared -o ${TARGET_M_SO} -Wl,--whole-archive ${TARGET_M} -Wl,--no-whole-archive

${TARGET_ALL_SO}: ${LIB_FILES_ALL}
	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@gcc -shared -o ${TARGET_ALL_SO} -Wl,--whole-archive ${LIB_FILES_ALL} -Wl,--no-whole-archive
