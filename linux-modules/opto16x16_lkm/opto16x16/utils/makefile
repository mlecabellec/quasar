# $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/makefile $
# $Rev: 53726 $
# $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

# OPTO16X16: Utilities: make file

default: all

CC			= gcc
CC_FLAGS	= -c -Wall -I../include
DEP_FILE	= makefile.dep
TARGET		= ../lib/opto16x16_utils.a

.NOTPARALLEL: %.o

ECHO	:= ${shell ls /bin/echo 2>/dev/null | wc -l}
ifeq ("${ECHO}","1")
ECHO	:= /bin/echo
else
ECHO	:= echo
endif

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

OBJ_FILES	:=		\
			close.o	\
			init.o	\
			ioctl.o	\
			open.o	\
			read.o	\
			\
			util_api_listing.o	\
			util_count.o		\
			util_id.o			\
			util_reg.o			\
			\
			util_clock_divider.o	\
			util_cos_polarity.o		\
			util_debounce_ms.o		\
			util_debounce_us.o		\
			util_initialize.o		\
			util_irq_enable.o		\
			util_led.o				\
			util_query.o			\
			util_reg_mod.o			\
			util_reg_read.o			\
			util_reg_write.o		\
			util_rx_data.o			\
			util_rx_event_counter.o	\
			util_tx_data.o			\
			util_wait_cancel.o		\
			util_wait_event.o		\
			util_wait_status.o

include ./gsc_util_makefile.inc
include ./gsc_util_makefile_pci.inc
include ./os_util_makefile.inc

# Sort object file list to compile most recentlty changed sources first.
OBJ_FILES:= $(shell ls -t $(patsubst %.o,%.c,${OBJ_FILES}))
OBJ_FILES:= $(patsubst %.c,%.o,${OBJ_FILES})

.c.o:
	@echo == Compiling: $<  ${GSC_LIB_COMP_FLAGS_MSG}
	@-chmod +rw ${DEP_FILE}
	@# Get the dependency list for this module.
	@-${CC} -MM ${CC_FLAGS} $< > .tmp1
	@# Remove the trailing white space and backslash.
	@-sed -e "s/[ ]*[\\\\]//g" < .tmp1 > .tmp2
	@# Put everything on seperate lines.
	@-tr [:space:] \\n < .tmp2 > .tmp3
	@# Remove all of the system include files.
	@-grep -v "^[ ]*/" < .tmp3 > .tmp4
	@# Remove all empty lines.
	@-grep [[:alnum:]] < .tmp4 > .tmp5
	@# Put everything on the same line.
	@-tr '\n' '\040' < .tmp5 > .tmp6
	@-${ECHO} -e '\012' >> .tmp6
	@# Add all of the other dependencies to the end of this file.
	@-echo >> ${DEP_FILE}
	@-grep -v "^[ ]*$@" < ${DEP_FILE} >> .tmp6
	@# Remove blank lines from the list.
	@-grep "[[:alnum:]]" < .tmp6 > .tmp7
	@# Sort the list and put it in the dependency file.
	@-sort < .tmp7 > ${DEP_FILE}
	@# Cleanup.
	@rm -f ${DEP_FILE}.tmp* .tmp*
	@# Compile the module.
	@${CC} ${CC_FLAGS} $< -o $@ ${GSC_LIB_COMP_FLAGS}

all: ${TARGET}
	@echo ==== All Done

release: ${TARGET}
	@rm -f *.o
	@echo ==== release Done

clean::
	@echo ==== Cleaning ${TARGET} ${GSC_TARGET} ${PLX_TARGET} ${OS_TARGET} ...
	@rm -f *.a *.o ${TARGET} ${GSC_TARGET} ${PLX_TARGET} ${OS_TARGET}
	@-chmod +rw ${DEP_FILE}
	@echo > ${DEP_FILE}

${TARGET}: makefile ${OBJ_FILES} ${GSC_TARGET} ${PLX_TARGET} ${OS_TARGET}
	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${OBJ_FILES} ${GSC_LIB_LINK_FLAGS}

${DEP_FILE}:
	@echo ==== Creating: $@
	@echo > ${DEP_FILE}

include ${DEP_FILE}
