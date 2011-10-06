all:
	./configure --prefix=$(SYSROOT_PATH)/usr --host=$(CAVAN_TARGET_PLAT)
	make -j4
	make install-dev
	make install-lib
