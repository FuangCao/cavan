BUSYBOX_OPTION = CONFIG_PREFIX=$(ROOTFS_PATH)

all:
	$(Q)+sb2 make $(BUSYBOX_OPTION) defconfig
	$(Q)+sb2 make $(BUSYBOX_OPTION) install
	$(Q)rm $(ROOTFS_PATH)/bin/sh -rfv
	$(Q)ln -vsf bash $(ROOTFS_PATH)/bin/sh
