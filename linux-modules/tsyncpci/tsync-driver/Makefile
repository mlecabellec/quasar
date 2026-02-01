KVER := $(shell uname -r)

UDEV_DIR=$(DESTDIR)/lib/udev/rules.d
UDEV_RULE=25-tsyncpci.rules

TOP_LEVEL := $(realpath $(CURDIR)/..)
COMMON_DIR := $(TOP_LEVEL)/common
COMMON := $(foreach file, $(wildcard $(COMMON_DIR)/*.c), $(notdir $(file)))

TSYNC_VERSION ?= $(shell cat version || echo 9.9.9)

PREFIX ?= /usr
INSTALL ?= install

obj-m += tsyncpci.o

tsyncpci-objs := \
	tpro_func.o \
	os_wait.o \
	tpro_func32.o \
	tsync_func.o \
	tsync_nios.o \
	tsync_pcie.o \
	tsync_pps.o \
	tsync_ptp.o \
	tsync_ptp_host_reference.o

tsyncpci-objs += tsync_hw_func.o
tsyncpci-objs += tsync_hw_nonkts_func.o
tsyncpci-objs += tsync_comm_validation.o
tsyncpci-objs += tsync_comm_transaction.o
tsyncpci-objs += tsync_comm_fifo.o
tsyncpci-objs += tsync_gpio_stamp_queue.o
tsyncpci-objs += binary_coded_decimal.o

ifeq ($(TSYNC_DRV_DEBUG), 1)
	DBG_FLAG=-DTPRO_DEBUG
	tsyncpci-objs += tsync_error_codes.o
endif


ccflags-y := \
	-DDRIVER_VERSION=\"$(TSYNC_VERSION)\" \
	-I$(src) \
	-I$(src)/include

KDIR ?= /lib/modules/$(KVER)/build

all: $(if $(TSYNC_DRV_DKMS_BUILD),, modules)

$(COMMON):
	cp $(COMMON_DIR)/$@ .

modules: $(COMMON)
	$(MAKE) -C $(KDIR) TSYNC_VERSION=$(TSYNC_VERSION) M=$$PWD modules CFLAGS="$(DBG_FLAG) $(CFLAGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)"
clean:
	$(RM) $(COMMON)
	$(MAKE) -C $(KDIR) M=$$PWD clean

udev_install:
	install -d $(UDEV_DIR)
	install -m 644 $(UDEV_RULE) $(UDEV_DIR)

modules_install: udev_install
	$(MAKE) -C $(KDIR) M=$$PWD $(if $(DESTDIR),INSTALL_MOD_PATH=$(DESTDIR),) modules_install
	depmod $(if $(DESTDIR), -b $(DESTDIR),) -a $(KVER)

modules_uninstall:
	rm -f /lib/modules/$(KVER)/extra/tsyncpci.ko
	if [ -d $(UDEV_DIR) -a -f $(UDEV_DIR)/$(UDEV_RULE) ] ; then \
		rm -f /lib/udev/rules.d//25-tsyncpci.rules ; \
	fi

# DKMS Build info: https://wiki.kubuntu.org/Kernel/Dev/DKMSPackaging
DKMS_ARGS := -m tsyncpci -v $(TSYNC_VERSION)

dkms: dkms-add dkms-build dkms-install udev_install

dkms-add:
	cp -r ../tsync-driver /usr/src/tsyncpci-$(TSYNC_VERSION)
	/usr/sbin/dkms add $(DKMS_ARGS)

dkms-build:
	/usr/sbin/dkms build $(DKMS_ARGS)

dkms-install:
	/usr/sbin/dkms install $(DKMS_ARGS)

dkms-uninstall:
	/usr/sbin/dkms remove $(DKMS_ARGS) --all
	rm -r /usr/src/tsyncpci-$(TSYNC_VERSION)

dkms-debian:
	/usr/sbin/dkms mkdeb -m tsyncpci -v $(TSYNC_VERSION)
	cp /var/lib/dkms/tsyncpci/$(TSYNC_VERSION)/deb/*.deb .

install: $(if $(TSYNC_DRV_DKMS_BUILD), dkms, modules_install install_headers)

install_headers:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/src/tsyncpci-$(TSYNC_VERSION)/include
	$(INSTALL) -m 644 include/* $(DESTDIR)$(PREFIX)/src/tsyncpci-$(TSYNC_VERSION)/include

uninstall: modules_uninstall
	$(RM) $(DESTDIR)$(PREFIX)/lib/pkgconfig/tsyncpci.pc
	$(RM) -r $(DESTDIR)$(PREFIX)/src/tsyncpci-$(TSYNC_VERSION)

pkgconfig:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	cat > $(DESTDIR)$(PREFIX)/lib/pkgconfig/tsyncpci.pc <<HEREDOC
	prefix=$(PREFIX)
	exec_prefix=\$${prefix}
	includedir=\$${prefix}/src/tsyncpci-$(TSYNC_VERSION)/include

	Name: tsyncpci
	Description: The Tsyncpci driver
	Version: $(TSYNC_VERSION)
	Cflags: -I\$${includedir}
	HEREDOC

.PHONY=	all install uninstall clean \
	modules modules_install modules_uninstall udev_install \
	dkms dkms-add dkms-build dkms-install dkms-uninstall dkms-debian
.ONESHELL:

