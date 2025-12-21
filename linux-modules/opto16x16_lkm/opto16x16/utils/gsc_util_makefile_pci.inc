# $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/pci_plx/gsc_util_makefile_pci.inc $
# $Rev: 51522 $
# $Date: 2022-10-03 16:31:47 -0500 (Mon, 03 Oct 2022) $

# Linux: Utility: PLX Specific: makefile

.PHONY: all clean default makefile release

ECHO	:= ${shell ls -d ../lib 2>/dev/null | wc -l}
ifeq ("${ECHO}","1")
PLX_TARGET			= ../lib/plx_utils.a
else
PLX_TARGET			= plx_utils.a
endif

PLX_OBJ_FILES		:=							\
					./gsc_util_reg_pex8111.o	\
					./gsc_util_reg_pex8112.o	\
					./gsc_util_reg_plx9056.o	\
					./gsc_util_reg_plx9060es.o	\
					./gsc_util_reg_plx9080.o	\
					./gsc_util_reg_plx9656.o



ECHO	:= ${shell ls /bin/echo 2>/dev/null | wc -l}
ifeq ("${ECHO}","1")
ECHO	:= /bin/echo
else
ECHO	:= echo
endif



${PLX_TARGET}: ${PLX_OBJ_FILES}
	@echo ==== Linking: $@ ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${PLX_OBJ_FILES} ${GSC_LIB_LINK_FLAGS}


