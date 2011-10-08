all:
	./configure $(LIBRARY_COMMON_CONFIG) --prefix=$(SYSROOT_PATH)/usr
	make -j4
	make install-dev
	make install-lib
