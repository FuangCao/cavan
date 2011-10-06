all:
	CC=$(CAVAN_TARGET_PLAT)-gcc AR=$(CAVAN_TARGET_PLAT)-ar ./configure --prefix=/usr --shared
	make -j4
	make DESTDIR=$(SYSROOT_PATH) install
