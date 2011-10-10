all:
	$(Q)+sb2 make CC=gcc
	$(Q)+sb2 make DESTDIR=$(ROOTFS_PATH) PREFIX=/ install
