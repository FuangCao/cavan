BINUTILS_OPTION =	$(CPU_BINUTILS_OPTION) $(TOOLCHIAN_COMMON_CONFIG) \
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
