# $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/driver/Makefile $
# $Rev: 53732 $
# $Date: 2023-09-14 10:52:04 -0500 (Thu, 14 Sep 2023) $

# OPTO16X16: Device Driver: make file

TARGET		:= opto16x16

.NOTPARALLEL: %.o

OBJ_FILES	:=					\
			close.o				\
			device.o			\
			ioctl.o				\
			irq.o				\
			open.o				\
			reg.o				\
								\
			gsc_bar.o			\
			gsc_close.o			\
			gsc_eeprom.o		\
			gsc_endian.o		\
			gsc_endian_pci.o	\
			gsc_init.o			\
			gsc_ioctl.o			\
			gsc_irq.o			\
			gsc_irq_pci.o		\
			gsc_open.o			\
			gsc_reg.o			\
			gsc_utils.o			\
			gsc_wait.o			\
								\
			os_bar.o			\
			os_close.o			\
			os_event.o			\
			os_init.o			\
			os_ioctl.o			\
			os_irq.o			\
			os_kernel_2_2.o		\
			os_kernel_2_4.o		\
			os_kernel_2_6.o		\
			os_kernel_3.o		\
			os_kernel_4.o		\
			os_kernel_5.o		\
			os_kernel_6.o		\
			os_mem.o			\
			os_metrics.o		\
			os_open.o			\
			os_pci.o			\
			os_proc.o			\
			os_read.o			\
			os_reg.o			\
			os_sem.o			\
			os_spinlock.o		\
			os_time.o			\
			os_write.o

###############################################################################
# Everything here is needed so we can access os_makefile.inc.

.EXPORT_ALL_VARIABLES:

# Locate the source from the starting directory.
ifndef GSC_DEV_DIR
GSC_DEV_DIR	:= $(shell pwd)
endif

include ${GSC_DEV_DIR}/os_makefile.inc
