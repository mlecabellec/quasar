# Makefile for eabi kernel module
obj-m += eabi.o

eabi-objs := \
    src/ASDC/asdc.o \
    src/ASDC/asdcalloc.o \
    src/ASDC/asdcioctl.o \
    src/ASDC/asdcwq.o \
    src/ASDC/coco.o \
    src/ASDC/do_acces.o \
    src/ASDC/do_ASDC_ABO_GF.o \
    src/ASDC/do_ASDC_ABO_IVVV.o \
    src/ASDC/do_ASDC_ABO_LIBERER.o \
    src/ASDC/do_ASDC_ABO_MF.o \
    src/ASDC/do_ASDC_CC_DATA.o \
    src/ASDC/do_ASDCACCES.o \
    src/ASDC/do_ASDCALLOUE.o \
    src/ASDC/do_ASDCAVORTE.o \
    src/ASDC/do_ASDCDEF.o \
    src/ASDC/do_ASDCECR.o \
    src/ASDC/do_ASDCEVT_ABO_AJOUTER.o \
    src/ASDC/do_ASDCEVT_ABO_SUPPRIMER.o \
    src/ASDC/do_ASDCEVT_CC_AJOUTER.o \
    src/ASDC/do_ASDCEVT_CC_SUPPRIMER.o \
    src/ASDC/do_ASDCLEC.o \
    src/ASDC/do_ASDCLIBERE.o \
    src/ASDC/do_ASDCLIREMODE.o \
    src/ASDC/do_ASDCLVER.o \
    src/ASDC/do_ASDCMODE.o \
    src/ASDC/do_ASDCNBM.o \
    src/ASDC/do_ASDCRAZ.o \
    src/ASDC/do_ASDCSTS.o \
    src/ASDC/do_ASDCTREP.o \
    src/ASDC/do_ES_IMAGE.o \
    src/ASDC/do_ES_PARAM.o \
    src/ASDC/do_ES_RAM.o \
    src/ASDC/do_GOSTOP.o \
    src/ASDC/do_LECMEMO.o \
    src/ASDC/do_SA_DELIER.o \
    src/ASDC/do_SA_LIER.o \
    src/ASDC/isr_asdc.o \
    src/ASDC/msg_debug.o \
    src/ASDC/tampons.o \
    src/CEVT/cevt.o \
    src/CEVT/cevtioctl.o \
    src/VFX70/devvfx.o \
    src/VFX70/isr_pmcvfx.o \
    src/VFX70/vfx_ES_PCIe.o \
    src/VFX70/vfxioctl.o

EXTRA_CFLAGS += -I$(PWD)/src/ASDC -I$(PWD)/src/CEVT -I$(PWD)/src/VFX70

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
