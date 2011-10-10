all:
	make defconfig
	sed 's/\s*\(CONFIG_CROSS_COMPILER_PREFIX\)\s*=.*/\1="$(CAVAN_TARGET_PLAT)-"/g' .config -i
	+make
	make install
	cp _install/* $(ROOTFS_PATH) -anv
