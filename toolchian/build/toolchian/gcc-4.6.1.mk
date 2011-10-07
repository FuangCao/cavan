GCC_COMMON_OPTION = $(CPU_GCC_OPTION) \
					--prefix=/usr \
					--build=$(CAVAN_BUILD_PLAT) \
					--host=$(CAVAN_BUILD_PLAT) \
					--target=$(CAVAN_TARGET_PLAT) \
					--with-sysroot=$(SYSROOT_PATH)
					--with-gmp=$(UTILS_PATH)/usr \
					--with-mpfr=$(UTILS_PATH)/usr \
					--with-mpc=$(UTILS_PATH)/usr

GCC_OPTION1 =		$(GCC_COMMON_OPTION) \
					--disable-nls \
					--disable-shared \
					--disable-multilib \
					--disable-decimal-float \
					--disable-threads \
					--disable-libmudflap \
					--disable-libssp \
					--disable-libgomp \
					--disable-libquadmath \
					--disable-target-libiberty \
					--disable-target-zlib \
					--enable-long-long \
					--enable-languages=c \
					--without-ppl \
					--without-cloog \
					--without-headers \
					--with-newlib

GCC_OPTION2 =		$(GCC_COMMON_OPTION) \
					--enable-clocale=gnu \
					--enable-shared \
					--enable-threads=posix \
					--enable-__cxa_atexit \
					--enable-languages=c,c++ \
					--enable-c99 \
					--enable-long-long \
					--disable-nls \
					--disable-multilib \
					--disable-libstdcxx-pch \
					--disable-bootstrap \
					--disable-libgomp \
					--with-system-zlib \
					--without-ppl \
					--without-cloog

$(GCC_NAME)-1:
	$(Q)$(SRC_GCC)/configure $(GCC_OPTION1)
	$(Q)+make
	$(Q)+make DESTDIR=$(SYSROOT_PATH) install
	$(Q)ln -vsf libgcc.a $$($(CAVAN_TARGET_PLAT)-gcc -print-libgcc-file-name | sed 's/libgcc/&_eh/')

$(GCC_NAME)-2:
	$(Q)$(SRC_GCC)/configure $(GCC_OPTION2)
	$(Q)+make AS_FOR_TARGET=$(CAVAN_TARGET_PLAT)-as LD_FOR_TARGET=$(CAVAN_TARGET_PLAT)-ld
	$(Q)+make DESTDIR=$(SYSROOT_PATH) install
