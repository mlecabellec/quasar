# $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/Makefile $
# $Rev: 54971 $
# $Date: 2024-08-07 15:59:52 -0500 (Wed, 07 Aug 2024) $

# 16AI64SSA: Device Driver: make file

TARGET		:= 16ai64ssa

.NOTPARALLEL: %.o

OBJ_FILES	:=					\
			close.o				\
			device.o			\
			io.o				\
			ioctl.o				\
			irq.o				\
			open.o				\
			read.o				\
			reg.o				\
								\
			gsc_bar.o			\
			gsc_close.o			\
			gsc_dma.o			\
			gsc_dma_pci.o		\
			gsc_eeprom.o		\
			gsc_endian.o		\
			gsc_endian_pci.o	\
			gsc_init.o			\
			gsc_io.o			\
			gsc_ioctl.o			\
			gsc_irq.o			\
			gsc_irq_pci.o		\
			gsc_open.o			\
			gsc_read.o			\
			gsc_reg.o			\
			gsc_utils.o			\
			gsc_wait.o			\
			gsc_write.o			\
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

# Locate the source from the starting directory.
ifndef GSC_DEV_DIR
GSC_DEV_DIR	:= $(shell pwd)
endif

export GSC_DEV_DIR

include ${GSC_DEV_DIR}/os_makefile.inc
