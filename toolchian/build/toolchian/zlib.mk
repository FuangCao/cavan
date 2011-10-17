all:
	$(Q)CC=$(CAVAN_TARGET_PLAT)-gcc \
	AR=$(CAVAN_TARGET_PLAT)-ar \
	RANLIB=$(CAVAN_TARGET_PLAT)-ranlib \
	./configure --prefix=/usr
	$(Q)+make
	$(Q)+make DESTDIR="$(SYSROOT_PATH)" install
