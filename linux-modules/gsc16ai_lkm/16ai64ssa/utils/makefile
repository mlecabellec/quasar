# $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/makefile $
# $Rev: 54951 $
# $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

# 16AI64SSA: Utilities: make file

default: all

CC			= gcc
CC_FLAGS	= -c -Wall -I../include
DEP_FILE	= makefile.dep
TARGET		= ../lib/16ai64ssa_utils.a

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
			util_config_ai.o	\
			util_config_aux.o	\
			util_count.o		\
			util_fsamp_ai.o		\
			util_id.o			\
			util_reg.o			\
			\
			util_ai_buf_clear.o			\
			util_ai_buf_level.o			\
			util_ai_buf_overflow.o		\
			util_ai_buf_thr_lvl.o		\
			util_ai_buf_thr_sts.o		\
			util_ai_buf_underflow.o		\
			util_ai_mode.o				\
			util_ai_range.o				\
			util_autocal.o				\
			util_autocal_status.o		\
			util_aux_0_mode.o			\
			util_aux_1_mode.o			\
			util_aux_2_mode.o			\
			util_aux_3_mode.o			\
			util_aux_in_pol.o			\
			util_aux_noise.o			\
			util_aux_out_pol.o			\
			util_burst_size.o			\
			util_burst_src.o			\
			util_burst_status.o			\
			util_chan_active.o			\
			util_chan_first.o			\
			util_chan_last.o			\
			util_chan_single.o			\
			util_data_format.o			\
			util_data_packing.o			\
			util_ext_sync_enable.o		\
			util_initialize.o			\
			util_input_sync.o			\
			util_irq0_sel.o				\
			util_irq1_sel.o				\
			util_low_lat_doh.o			\
			util_low_lat_hold_chan.o	\
			util_low_lat_read.o			\
			util_low_lat_rel_chan.o		\
			util_query.o				\
			util_rag_enable.o			\
			util_rag_nrate.o			\
			util_rbg_clk_src.o			\
			util_rbg_enable.o			\
			util_rbg_nrate.o			\
			util_reg_mod.o				\
			util_reg_read.o				\
			util_reg_write.o			\
			util_rx_io_abort.o			\
			util_rx_io_mode.o			\
			util_rx_io_overflow.o		\
			util_rx_io_timeout.o		\
			util_rx_io_underflow.o		\
			util_samp_clk_src.o			\
			util_scan_marker.o			\
			util_scan_marker_get.o		\
			util_scan_marker_set.o		\
			util_wait_cancel.o			\
			util_wait_event.o			\
			util_wait_status.o

# Retained for backwards compatibility only:
OBJ_FILES	:=	${OBJ_FILES}		\
			util_auto_cal_sts.o		\
			util_auto_calibrate.o

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

${TARGET}: ${OBJ_FILES} ${GSC_TARGET} ${PLX_TARGET} ${OS_TARGET}
	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${OBJ_FILES} ${GSC_LIB_LINK_FLAGS}

${DEP_FILE}:
	@echo ==== Creating: $@
	@echo > ${DEP_FILE}

include ${DEP_FILE}
