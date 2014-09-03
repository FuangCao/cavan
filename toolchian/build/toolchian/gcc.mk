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
					--disable-libatomic \
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
					--enable-languages=c,c++ \
					--enable-plugins \
					--enable-c99 \
					--enable-long-long
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

ifeq ($(CAVAN_TARGET_EABI),androideabi)
GCC_OPTION2 +=		--with-gnu-as \
					--with-gnu-ld \
					--with-host-libstdcxx='-static-libgcc -Wl,-Bstatic,-lstdc++,-Bdynamic -lm' \
					--enable-threads \
					--disable-libssp \
					--disable-libmudflap \
					--disable-libstdc__-v3 \
					--disable-sjlj-exceptions \
					--disable-shared \
					--disable-tls \
					--disable-libgomp \
					--disable-libatomic \
					--disable-libitm \
					--disable-libsanitizer \
					--disable-libquadmath
else
GCC_OPTION2 +=		--enable-clocale=gnu \
					--enable-shared \
					--enable-libgomp \
					--enable-threads=posix \
					--enable-__cxa_atexit
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
