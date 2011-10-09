all:
	+make CC=$(CAVAN_TARGET_PLAT)-gcc
	+make DESTDIR=$(ROOTFS_PATH) PREFIX=/ install
