GCC_COMMON_OPTION = $(CPU_GCC_OPTION) $(TOOLCHIAN_COMMON_CONFIG)

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
					--enable-plugins \
					--enable-libgomp \
					--enable-graphite=yes \
					--enable-gold=default \
					--enable-cloog-backend=isl \
					--enable-target-optspace \
					--enable-initfini-array \
					--disable-nls \
					--disable-bootstrap \
					--disable-ppl-version-check \
					--disable-cloog-version-check \
					--with-binutils-version=$(BINUTILS_VERSION) \
					--with-mpfr-version=$(MPFR_VERSION) \
					--with-mpc-version=$(MPC_VERSION) \
					--with-gmp-version=$(GMP_VERSION)

ifneq ($(filter $(GCC_VERSION),4.8.1 4.8.2 4.9.1),)
GCC_OPTION1 += --disable-libatomic
endif

stage1:
	$(Q)$(SRC_GCC)/configure $(GCC_OPTION1)
	$(Q)+make
	$(Q)+make install
	$(Q)ln -vsf libgcc.a $$($(CAVAN_TARGET_PLAT)-gcc -print-libgcc-file-name | sed 's/libgcc/&_eh/')

stage2:
	$(Q)$(SRC_GCC)/configure $(GCC_OPTION2)
	$(Q)+make
	$(Q)+make install
