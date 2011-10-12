GCC_COMMON_OPTION = $(CPU_GCC_OPTION) $(TOOLCHIAN_COMMON_CONFIG)
#					--with-gmp=$(UTILS_PATH)/usr \
#					--with-mpfr=$(UTILS_PATH)/usr \
#					--with-mpc=$(UTILS_PATH)/usr

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
					--without-ppl \
					--without-cloog

ifeq ($(CAVAN_HOST_PLAT),$(CAVAN_BUILD_PLAT))
GCC_OPTION2 += --with-system-zlib
else
GCC_OPTION2 += --disable-target-zlib
endif

$(GCC_NAME)-pase1:
	$(Q)$(SRC_GCC)/configure $(GCC_OPTION1)
	$(Q)+make
	$(Q)+make install
	$(Q)ln -vsf libgcc.a $$($(CAVAN_TARGET_PLAT)-gcc -print-libgcc-file-name | sed 's/libgcc/&_eh/')

$(GCC_NAME)-pase2:
	$(Q)$(SRC_GCC)/configure $(GCC_OPTION2)
	$(Q)+make AS_FOR_TARGET="$(CAVAN_TARGET_PLAT)-as" LD_FOR_TARGET="$(CAVAN_TARGET_PLAT)-ld"
	$(Q)+make install
