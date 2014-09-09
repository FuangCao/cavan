GLIBC_OPTION =	--prefix=/usr \
				--build=$(CAVAN_BUILD_PLAT) \
				--host=$(CAVAN_TARGET_PLAT) \
				--disable-profile \
				--enable-add-ons \
				--enable-obsolete-rpc \
				--enable-kernel=$(LOWEST_KERNEL_VERSION) \
				--with-tls \
				--with-__thread \
				--with-binutils=$(TOOLCHIAN_PATH) \
				--with-headers=$(SYSROOT_PATH)/usr/include

GLIBC_CONFIGPARMS = "install_root=$(SYSROOT_PATH)"

ifeq ($(GLIBC_VERSION),2.16.0)
$(shell sed -i "s/-lgcc_s//g" $(SRC_GLIBC)/Makeconfig)
endif

all:
	$(Q)echo "$(GLIBC_CONFIGPARMS)" > configparms
	$(Q)libc_cv_forced_unwind=yes \
	libc_cv_c_cleanup=yes \
	libc_cv_gnu99_inline=yes \
	libc_cv_ctors_header=yes \
	libc_cv_slibdir=/lib \
	BUILD_CC=gcc \
	CC="$(CAVAN_TARGET_PLAT)-gcc" \
	AR="$(CAVAN_TARGET_PLAT)-ar" \
	RANLIB="$(CAVAN_TARGET_PLAT)-ranlib" \
	$(SRC_GLIBC)/configure $(GLIBC_OPTION)
	$(Q)+make
	$(Q)+make install
