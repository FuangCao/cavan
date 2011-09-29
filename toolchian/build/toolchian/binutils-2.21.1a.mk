BINUTILS_OPTION =	$(CPU_BINUTILS_OPTION) \
					--with-sysroot=$(SYSROOT_PATH) \
					--with-mpfr=$(UTILS_PATH)/usr \
					--with-gmp=$(UTILS_PATH)/usr \
					--disable-nls \
					--disable-werror \
					--disable-mutilib

all:
	$(Q)+make -f $(MAKEFILE_TOOLCHIAN_RULE) OPT="$(BINUTILS_OPTION)" SRC="$(SRC_BINUTILS)"
	$(Q)cp $(SRC_BINUTILS)/include/libiberty.h $(SYSROOT_PATH)/usr/include -av
