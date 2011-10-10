BUSYBOX_OPTION = CONFIG_PREFIX=$(ROOTFS_PATH)

all:
	+sb2 make $(BUSYBOX_OPTION) defconfig
	+sb2 make $(BUSYBOX_OPTION) install
