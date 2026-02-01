PACKAGES=tsync-driver libtsync tsync-utils

all: $(addsuffix -all,$(PACKAGES))
clean: $(addsuffix -clean,$(PACKAGES))
install: $(addsuffix -install,$(PACKAGES))
uninstall: $(addsuffix -uninstall,$(PACKAGES))

libtsync-%: FORCE
	+make CFLAGS=-DCOLDFIRE -C libtsync/src $*

# Driver do not have all target, just use the default one instead.
tsync-driver-all:
	+make -C tsync-driver

tsync-driver-%: FORCE
	+make -C tsync-driver $*

tsync-utils-%: FORCE
	+make CFLAGS=-DCOLDFIRE -C tsync-utils/src $*

tsync-utils-all: libtsync-all

.PHONY: all clean tsync-driver-all
FORCE:;
