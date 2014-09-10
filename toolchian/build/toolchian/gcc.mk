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
					--enable-long-long \
					--enable-graphite=yes \
					--enable-gold=default \
					--enable-cloog-backend=isl \
					--enable-target-optspace \
					--enable-initfini-array \
					--disable-nls \
					--disable-bootstrap \
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
					--disable-shared \
					--disable-libgomp \
					--disable-libatomic \
					--disable-libitm \
					--disable-libsanitizer \
					--disable-libstdc__-v3 \
					--disable-libquadmath

ifeq ($(filter 4.9%,$(GCC_VERSION)),)
GCC_OPTION2 +=		--disable-libmudflap
endif
else
GCC_OPTION2 +=		--enable-clocale=gnu \
					--enable-shared \
					--enable-libgomp \
					--enable-threads=posix \
					--enable-__cxa_atexit
endif

ifneq ($(filter i%86 amd64 x86_64,$(CAVAN_TARGET_ARCH)),)
GCC_OPTION2 +=		--enable-objc-gc \
					--enable-checking=release
endif

# ifneq ($(filter amd64 x86_64,$(CAVAN_TARGET_ARCH)),)
# GCC_OPTION2 +=		--disable-multilib
# endif

ifeq ($(GLIBC_LIB_NAME),libx32)
GCC_ENV = libc_cv_x32=yes
endif

stage1:
	$(Q)$(GCC_ENV) $(SRC_GCC)/configure $(GCC_OPTION1)
	$(Q)+make
	$(Q)+make install
	$(Q)ln -vsf libgcc.a $$($(CAVAN_TARGET_PLAT)-gcc -print-libgcc-file-name | sed 's/libgcc/&_eh/')

stage2:
	$(Q)$(SRC_GCC)/configure $(GCC_OPTION2)
	$(Q)+make
	$(Q)+make install
