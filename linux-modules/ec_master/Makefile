# Makefile for ec_master kernel module
obj-m += ec_master.o

ec_master-objs := \
    src/master/cdev.o \
    src/master/coe_emerg_ring.o \
    src/master/datagram.o \
    src/master/datagram_pair.o \
    src/master/device.o \
    src/master/domain.o \
    src/master/fmmu_config.o \
    src/master/foe_request.o \
    src/master/fsm_change.o \
    src/master/fsm_coe.o \
    src/master/fsm_foe.o \
    src/master/fsm_master.o \
    src/master/fsm_pdo.o \
    src/master/fsm_pdo_entry.o \
    src/master/fsm_sii.o \
    src/master/fsm_slave.o \
    src/master/fsm_slave_config.o \
    src/master/fsm_slave_scan.o \
    src/master/fsm_soe.o \
    src/master/ioctl.o \
    src/master/mailbox.o \
    src/master/master.o \
    src/master/module.o \
    src/master/pdo.o \
    src/master/pdo_entry.o \
    src/master/pdo_list.o \
    src/master/reg_request.o \
    src/master/sdo.o \
    src/master/sdo_entry.o \
    src/master/sdo_request.o \
    src/master/slave.o \
    src/master/slave_config.o \
    src/master/soe_errors.o \
    src/master/soe_request.o \
    src/master/sync.o \
    src/master/sync_config.o \
    src/master/voe_handler.o

EXTRA_CFLAGS += -I$(PWD)/src -I$(PWD)/src/master -I$(PWD)/src/devices

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
