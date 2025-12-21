# $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_makefile.inc $
# $Rev: 52470 $
# $Date: 2023-02-13 09:58:00 -0600 (Mon, 13 Feb 2023) $

# Linux: Device Driver: make file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

default: all

# Identify the /dev/nul(l) device
NULL	:= $(shell ls /dev/nu[l]* | grep -w null | wc -l )
NULL	:= $(shell expr ${NULL})

ifeq ("${NULL}","1")
NULL	:= /dev/null
else
NULL	:= /dev/nul
endif

.PHONY: all clean default makefile release

# Most of this is ignored by the 2.6 (and later) module builder.
CC			= gcc
CC_FLAGS	+= -c -O6 -pipe -fomit-frame-pointer -Wall
CC_FLAGS	+= -D__KERNEL__ -DLINUX
CC_FLAGS	+= -I. -I${KERNELDIR}
DEP_FILE	= ${GSC_DEV_DIR}/makefile.dep
KERNELDIR	=
KERNELVER	= $(shell uname -r | cut -d . -f1-2 )
KERNELMAJOR	= $(shell uname -r | cut -d . -f1 )
RELEASE_RM	= ${OBJ_FILES} .tmp* .*.o.cmd .*.ko.cmd .*.mod.cmd .*.ko.*.cmd *.ko.unsigned *.mod.* *.mod Modules*



# kernel 2.2 ==================================================================
ifeq ("${KERNELVER}","2.2")
KERNELDIR	= /usr/src/linux
MODULE_NAME	= ${TARGET}.o

${MODULE_NAME}: ${OBJ_FILES}
	@echo ==== Linking: $@
	@ld -r -o $@ ${OBJ_FILES}
endif

# kernel 2.4 ==================================================================
ifeq ("${KERNELVER}","2.4")

K_DIR	:= $(shell ls -d /usr/src/linux/include 2>/dev/null | wc -l )
K_DIR	:= $(shell echo $(K_DIR) | sed -e 's/[ \t]*//g')

ifeq ("${K_DIR}","1")
KERNELDIR	= /usr/src/linux/include
else
KERNELDIR	= /usr/src/linux-2.4/include
endif

MODULE_NAME	= ${TARGET}.o

${MODULE_NAME}: ${OBJ_FILES}
	@echo ==== Linking: $@
	@ld -r -o $@ ${OBJ_FILES}
endif

# kernel 2.6 ==================================================================
ifeq ("${KERNELVER}","2.6")

${TARGET}-objs	= ${OBJ_FILES}
KERNELDIR		= /lib/modules/$(shell uname -r)/build
MODULE_NAME		= ${TARGET}.ko
obj-m			= ${TARGET}.o
PWD				= $(shell pwd)
RELEASE_RM		+= ${TARGET}.o Module.symvers

${MODULE_NAME}: $(shell ls ${GSC_DEV_DIR}/*.[ch])
	@-chmod +rw ${DEP_FILE}
	@-echo -n > ${DEP_FILE}
	@make -C ${KERNELDIR} SUBDIRS=${PWD} modules
	@strip -d --strip-unneeded $@
	@rm -f *.mod.c
endif

# kernel 3.x ==================================================================
ifeq ("${KERNELMAJOR}","3")

${TARGET}-objs	= ${OBJ_FILES}
KERNELDIR		= /lib/modules/$(shell uname -r)/build
MODULE_NAME		= ${TARGET}.ko
obj-m			= ${TARGET}.o
PWD				= $(shell pwd)
RELEASE_RM		+= ${TARGET}.o Module.symvers

${MODULE_NAME}: $(shell ls ${GSC_DEV_DIR}/*.[ch])
	@-chmod +rw ${DEP_FILE}
	@-echo -n > ${DEP_FILE}
	@make -C ${KERNELDIR} SUBDIRS=${PWD} modules
	@strip -d --strip-unneeded $@
	@rm -f *.mod.c
endif

# kernel 4.x ==================================================================
ifeq ("${KERNELMAJOR}","4")

${TARGET}-objs	= ${OBJ_FILES}
KERNELDIR		= /lib/modules/$(shell uname -r)/build
MODULE_NAME		= ${TARGET}.ko
obj-m			= ${TARGET}.o
PWD				= $(shell pwd)
RELEASE_RM		+= ${TARGET}.o Module.symvers

${MODULE_NAME}: $(shell ls ${GSC_DEV_DIR}/*.[ch])
	@-chmod +rw ${DEP_FILE}
	@-echo -n > ${DEP_FILE}
	@make -C ${KERNELDIR} SUBDIRS=${PWD} modules
	@strip -d --strip-unneeded $@
	@rm -f *.mod.c
endif

# kernel 5.x ==================================================================
ifeq ("${KERNELMAJOR}","5")

${TARGET}-objs	= ${OBJ_FILES}
KERNELDIR		= /lib/modules/$(shell uname -r)/build
MODULE_NAME		= ${TARGET}.ko
obj-m			= ${TARGET}.o
PWD				= $(shell pwd)
RELEASE_RM		+= ${TARGET}.o Module.symvers

${MODULE_NAME}: $(shell ls ${GSC_DEV_DIR}/*.[ch])
	@-chmod +rw ${DEP_FILE}
	@-echo -n > ${DEP_FILE}
	@make -C ${KERNELDIR} M=${PWD} modules
	@strip -d --strip-unneeded $@
	@rm -f *.mod.c
endif

# kernel 6.x ==================================================================
ifeq ("${KERNELMAJOR}","6")

${TARGET}-objs	= ${OBJ_FILES}
KERNELDIR		= /lib/modules/$(shell uname -r)/build
MODULE_NAME		= ${TARGET}.ko
obj-m			= ${TARGET}.o
PWD				= $(shell pwd)
RELEASE_RM		+= ${TARGET}.o Module.symvers

${MODULE_NAME}: $(shell ls ${GSC_DEV_DIR}/*.[ch])
	@-chmod +rw ${DEP_FILE}
	@-echo -n > ${DEP_FILE}
	@make -C ${KERNELDIR} M=${PWD} modules
	@strip -d --strip-unneeded $@
	@rm -f *.mod.c
endif

# kernel OTHER ================================================================
ifeq ("${KERNELDIR}","")

KERNELDIR	= KERNELDIR_os_makefile.inc_not_known_at_this_time
MODULE_NAME	= MODULE_NAME_os_makefile.inc_not_known_at_this_time

${MODULE_NAME}:
	@echo ERROR: KERNEL ${KERNELVER} IS NOT SUPPORTED BY THIS MAKEFILE.
	@_ERROR_ERROR_ERROR_ERROR_ERROR_ERROR_ERROR_ERROR_ERROR
endif



# COMMON ======================================================================

#This is ignored by the 2.6 (and later) module builder.
.c.o:
	@echo == Compiling: $<
	@-chmod +rw ${DEP_FILE}
	@# Get the dependency list for this module.
	@-${CC} -MM ${CC_FLAGS} $< >  .tmp1
	@# Remove trailing white space and backslash, if present.
	@-sed -e "s/[ ]*[\\\\]//g" < .tmp1 > .tmp2
	@# Put everything on seperate lines.
	@-tr [:space:] \\n < .tmp2 > .tmp3
	@# Remove all of the system include files.
	@-grep -v "^[ ]*/" < .tmp3 > .tmp4
	@# Remove all empty lines.
	@-grep [[:alnum:]] < .tmp4 > .tmp5
	@# Put everything on the same line.
	@-tr '\n' '\040' < .tmp5 > .tmp6
	@-echo -e '\012' >> .tmp6
	@# Add all the other dependencies to the end of this file.
	@-echo >> ${DEP_FILE}
	@-grep -v "^[ ]*$@" < ${DEP_FILE} >> .tmp6
	@# Remove blank lines from the list.
	@-grep "[[:alnum:]]" < .tmp6 > .tmp7
	@# Sort the list and put it in the dependency file.
	@-sort < .tmp7 > ${DEP_FILE}
	@# Cleanup.
	@rm -f .tmp?
	@rm -r -f -d .tmp*\*
	@# Compile the module.
	@${CC} ${CC_FLAGS} $< -o $@



all: ${MODULE_NAME}
	@echo ==== All Done

release: ${MODULE_NAME}
	@rm -rf  ${RELEASE_RM}
	@echo ==== Release Done

clean:
	@echo ==== Cleaning ${MODULE_NAME} ...
	@rm -rf ${RELEASE_RM} *.o *.scc *.ko core Module.markers modules.order .cache.* .Module.* .modules.*
	@echo > ${DEP_FILE}

${DEP_FILE}:
	@echo ==== Creating: $@
	@echo > ${DEP_FILE}
	@-chmod +rw ${DEP_FILE}

include ${DEP_FILE}

