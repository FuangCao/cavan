BINUTILS_OPTION =	$(CPU_BINUTILS_OPTION) \
					--prefix=$(TOOLCHIAN_PATH) \
					--build=$(CAVAN_BUILD_PLAT) \
					--host=$(CAVAN_HOST_PLAT) \
					--target=$(CAVAN_TARGET_PLAT) \
					--with-sysroot=$(SYSROOT_PATH) \
					--with-mpfr=$(UTILS_PATH)/usr \
					--with-gmp=$(UTILS_PATH)/usr \
					--disable-nls \
					--disable-werror \
					--disable-mutilib

all:
	$(Q)$(SRC_BINUTILS)/configure $(BINUTILS_OPTION)
	$(Q)+make
	$(Q)+make install
	$(Q)cp $(SRC_BINUTILS)/include/libiberty.h $(SYSROOT_PATH)/usr/include -av
