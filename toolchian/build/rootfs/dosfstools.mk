all:
	+sb2 make CC=gcc
	+sb2 make DESTDIR=$(ROOTFS_PATH) PREFIX=/ install
